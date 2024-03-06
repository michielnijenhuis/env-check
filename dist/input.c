#include <argument.h>
#include <assert.h>
#include <command.h>
#include <cstring.h>
#include <input.h>
#include <math-utils.h>
#include <option.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARG_PARSER_ERR_SIZE 1024

static int handle_opt_value(option_t *opt, char *name, char *value, input_parser_t *parser);

// ==== Implementations =======================================================/
input_parser_t
input_parser_create(argument_t **argv, size_t argc, option_t **optv, size_t optc, char *errbuf, unsigned int offset) {
    input_parser_t parser = {
        .argv   = argv,
        .argc   = argc,
        .errbuf = errbuf,
        .offset = offset,
        .optv   = optv,
        .optc   = optc,
        .parsed = 0,
    };

    return parser;
}

void input_parse(input_parser_t *parser, int argc, char **argv) {
    size_t current_arg_index = 0;
    bool   parsing_args      = false;

    for (int i = parser->offset; i < argc; ++i) {
        char *current_arg = argv[i];
        parser->parsed += 1;

        if (str_equals(current_arg, "--")) {
            parsing_args = true;
            continue;
        }

        bool is_long_opt  = option_is_long(current_arg);
        bool is_short_opt = !is_long_opt && option_is_short(current_arg);
        bool is_option    = is_long_opt || is_short_opt;

        if (!parsing_args && is_option) {
            if (current_arg_index > 0) {
                if (current_arg_index < parser->argc) {
                    argument_t *expected_arg = parser->argv[current_arg_index];
                    sprintf(parser->errbuf,
                            "Invalid args. Expected argument '%s', but received option '%s'",
                            expected_arg->name,
                            current_arg);
                }

                return;
            }

            // sanitize name (remove leading hyphens)
            int  offset = is_long_opt ? 2 : 1;
            int  len    = strlen(current_arg);
            char sanitized[len];
            strncpy(sanitized, current_arg + offset, len);

            if (is_short_opt) {
                int shortlen = len - offset;

                // chained short options
                // currently short options with a value expect a space
                if (shortlen == 1) {
                    option_t *opt = option_find(parser->optv, parser->optc, NULL, sanitized);

                    if (!opt) {
                        sprintf(parser->errbuf, "Received unknown option: %s", sanitized);
                        return;
                    }

                    char *value = NULL;

                    if (option_has_value(opt) && i + 1 < argc) {
                        value = argv[i + 1];
                        ++i;
                    }

                    if (handle_opt_value(opt, sanitized, value, parser) > 0) {
                        return;
                    }
                } else {
                    for (int j = 0; j < shortlen; j++) {
                        option_t *opt = option_find_by_shortcut(parser->optv, parser->optc, sanitized[j]);
                        if (opt) {
                            if (option_is_level(opt)) {
                                opt->levelval = min(opt->levelval + 1, OPTION_MAX_LEVEL);
                            } else {
                                opt->boolval = true;
                            }

                            opt->provided = true;
                        } else {
                            sprintf(parser->errbuf, "Received unknown option: %c", sanitized[j]);
                            return;
                        }
                    }
                }

                continue;
            }

            // extract value from option, if given
            char *name  = NULL;
            char *value = NULL;
            if (strchr(sanitized, '=') != NULL) {
                name  = strtok(sanitized, "=");
                value = strtok(NULL, "=");
            } else {
                name = sanitized;
            }

            option_t *current_opt = option_find_by_long_name(parser->optv, parser->optc, name);

            if (current_opt == NULL) {
                sprintf(parser->errbuf, "Received unknown option: %s", name);
                return;
            }

            if (option_has_value(current_opt) && value == NULL) {
                value = i + 1 < argc ? argv[++i] : NULL;
            }

            if (handle_opt_value(current_opt, name, value, parser) > 0) {
                return;
            }
        } else {
            parsing_args = true;

            if (current_arg_index >= parser->argc) {
                break;
            }

            argument_t *arg = parser->argv[current_arg_index];

            if (argument_is_array(arg)) {
                size_t new_count = arg->valuec + 1;

                if (!arg->valuev) {
                    arg->valuev = malloc(sizeof(char *));
                } else {
                    arg->valuev = realloc(arg->valuev, new_count * sizeof(char *));
                }

                arg->valuec                = new_count;
                arg->valuev[new_count - 1] = current_arg;
                continue;
            }

            arg->value = current_arg;

            if (current_arg_index + 1 >= parser->argc) {
                break;
            }

            current_arg_index++;
        }
    }

    for (size_t i = 0; i < parser->argc; ++i) {
        argument_t *arg = parser->argv[i];

        // not required, no need to check anything
        if (argument_is_optional(arg)) {
            continue;
        }

        // array: ensure 1+ values
        if (argument_is_array(arg)) {
            if (arg->valuec < arg->min_count) {
                sprintf(parser->errbuf,
                        "Argument '%s' requires at least %zu %s",
                        arg->name,
                        arg->min_count,
                        arg->min_count == 1 ? "value" : "values");
                return;
            }

            continue;
        }

        // required arg: ensure value was given
        if (arg->value == NULL) {
            sprintf(parser->errbuf, "Argument '%s' is required, but no value was provided", arg->name);
            return;
        }
    }
}

char *get_arg(command_t *cmd, const char *name) {
    argument_t *arg = argument_find(cmd->argv, cmd->argc, name);
    return arg ? arg->value : NULL;
}

bool get_bool_opt(command_t *cmd, const char *name) {
    option_t *opt = option_find(cmd->optv, cmd->optc, name, NULL);
    return opt ? opt->boolval : false;
}

char *get_string_opt(command_t *cmd, const char *name) {
    option_t *opt = option_find(cmd->optv, cmd->optc, name, NULL);
    return opt ? opt->stringval : NULL;
}

char **get_string_array_opt(command_t *cmd, const char *name) {
    option_t *opt = option_find(cmd->optv, cmd->optc, name, NULL);
    return opt ? opt->stringv : NULL;
}

static int handle_opt_value(option_t *opt, char *name, char *value, input_parser_t *parser) {
    opt->provided = true;

    if (option_is_level(opt)) {
        opt->levelval = min(opt->levelval + 1, OPTION_MAX_LEVEL);
    } else if (option_is_bool(opt) && !value) {
        if (option_is_negatable(opt) && str_starts_with(name, "no-")) {
            opt->boolval = false;
        } else {
            opt->boolval = true;
        }
    } else if (option_expects_value(opt) && !value) {
        sprintf(parser->errbuf, "Missing required value for option %s", name);
        return 1;
    } else if (option_has_value(opt) && value) {
        char *p = malloc(sizeof(char) * (strlen(value) + 1));
        assert(p != NULL);
        strcpy(p, value);

        if (option_is_array(opt)) {
            size_t newCount = opt->stringc + 1;
            if (!opt->stringv) {
                opt->stringv = malloc(sizeof(char *));
            } else {
                opt->stringv = realloc(opt->stringv, newCount * sizeof(char *));
            }

            assert(opt->stringv != NULL);
            opt->stringc               = newCount;
            opt->stringv[newCount - 1] = p;
            opt->__meta |= OPTION_META_CLEANUP_VALUE_ARRAY;
        } else {
            opt->stringval = p;
            opt->__meta |= OPTION_META_CLEANUP_VALUE;
        }
    }

    return 0;
}
