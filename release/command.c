#include <argument.h>
#include <assert.h>
#include <colors.h>
#include <command.h>
#include <cstring.h>
#include <math-utils.h>
#include <math.h>
#include <option.h>
#include <output.h>
#include <string.h>

static bool        compare_command_name(const char *name, const char *input);
static size_t      calculate_similarity(const char *a, const char *b);
static const char *find_closest_matching_command(const char *input, command_t **cmdv, size_t cmdc);

void               command_init(command_t *cmd, const char *name, const char *desc, cmd_op_func *op) {
    assert(cmd != NULL);
    assert(name != NULL);

    cmd->name           = name;
    cmd->desc           = desc;
    cmd->info           = NULL;
    cmd->aliasv         = NULL;
    cmd->aliasc         = 0;
    cmd->operation      = op;
    cmd->optv           = NULL;
    cmd->optc           = 0;
    cmd->argv           = NULL;
    cmd->argc           = 0;
    cmd->__default_optc = 0;
    cmd->__default_optv = NULL;
    cmd->handler        = NULL;
}

command_t command_create(const char *name, const char *desc, cmd_op_func *op) {
    command_t cmd;
    command_init(&cmd, name, desc, op);
    return cmd;
}

command_t command_define(const char *name, const char *desc, cmd_handler_func *handler, cmd_op_func *op) {
    command_t cmd;
    command_init(&cmd, name, desc, op);
    cmd.handler = handler;
    return cmd;
}

void command_add_info(command_t *cmd, const char *info) {
    if (cmd) {
        cmd->info = info;
    }
}

void command_set_args(command_t *cmd, argument_t **argv, size_t argc) {
    if (cmd) {
        cmd->argv = argv;
        cmd->argc = argc;
    }
}

void command_set_opts(command_t *cmd, option_t **optv, size_t optc) {
    if (cmd) {
        cmd->optv = optv;
        cmd->optc = optc;
    }
}

int command_index_of(command_t **cmdv, size_t cmdc, const char *name) {
    if (!cmdv || !name) {
        return -1;
    }

    for (size_t i = 0; i < cmdc; ++i) {
        const command_t *cmd = cmdv[i];
        assert(cmd != NULL);

        if (str_equals_case_insensitive(cmd->name, name)) {
            return i;
        }

        if (cmd->aliasv) {
            for (size_t j = 0; j < cmd->aliasc; ++j) {
                if (str_equals_case_insensitive(cmd->aliasv[j], name)) {
                    return i;
                }
            }
        }
    }

    return -1;
}

command_t *command_find(command_t **cmdv, size_t cmdc, const char *name) {
    int i = command_index_of(cmdv, cmdc, name);
    return i == -1 ? NULL : cmdv[i];
}

void command_print(command_t *cmd, int width) {
    assert(cmd != NULL);
    pcolorf(GREEN, "  %-*s", width + 2, cmd->name);

    if (cmd->aliasv) {
        writef("[");
        for (size_t i = 0; i < cmd->aliasc; ++i) {
            writef("%s", cmd->aliasv[i]);
            if (i + 1 < cmd->aliasc) {
                writef("|");
            }
        }
        writef("] ");
    }

    writef("%s\n", cmd->desc);
}

void command_print_all(command_t **cmdv, size_t cmdc, int width) {
    assert(cmdv != NULL);
    for (size_t i = 0; i < cmdc; ++i) {
        command_print(cmdv[i], width);
    }
}

int command_get_print_width(command_t **cmdv, size_t cmdc) {
    assert(cmdv != NULL);
    int width = 0;
    for (size_t i = 0; i < cmdc; ++i) {
        width = max(width, (int) strlen(cmdv[i]->name));
    }
    return width;
}

void command_set_aliases(command_t *cmd, const char **aliasv, size_t aliasc) {
    if (cmd) {
        cmd->aliasv = aliasv;
        cmd->aliasc = aliasc;
    }
}

int command_find_loose(command_t **cmdv, size_t cmdc, const char *input, const char **buffer, size_t buffersize) {
    assert(cmdv != NULL);
    assert(input != NULL);
    assert(buffer != NULL);

    size_t ptr = 0;
    for (size_t i = 0; i < cmdc && i < buffersize; ++i) {
        command_t *cmd = cmdv[i];
        assert(cmd != NULL);
        if (compare_command_name(cmd->name, input)) {
            buffer[ptr++] = cmd->name;
        } else {
            for (size_t j = 0; j < cmd->aliasc; ++j) {
                if (compare_command_name(cmd->aliasv[j], input)) {
                    buffer[ptr++] = cmd->aliasv[j];
                    break;
                }
            }
        }
    }
    if (ptr == 0 && ptr < buffersize) {
        const char *suggestion = find_closest_matching_command(input, cmdv, cmdc);
        if (suggestion) {
            buffer[ptr++] = suggestion;
        }
    }
    return ptr;
}

static bool compare_command_name(const char *name, const char *input) {
    if (str_equals_case_insensitive(name, input)) {
        return true;
    }

    if (str_starts_with(name, input)) {
        return true;
    }

    return false;
}

static size_t calculate_similarity(const char *input, const char *name) {
    size_t inputlen   = strlen(input);
    size_t namelen    = strlen(name);
    size_t minlen     = (inputlen < namelen) ? inputlen : namelen;

    size_t similarity = 0;
    for (size_t i = 0; i < minlen; i++) {
        if (input[i] == name[i]) {
            similarity++;
        } else if (similarity < namelen - 1) {
            break;
        }
    }

    if (similarity < namelen - 1) {
        return 0;
    }

    return similarity;
}

static const char *find_closest_matching_command(const char *input, command_t **cmdv, size_t cmdc) {
    int         max_similarity = 0;
    const char *closest_match  = NULL;

    for (size_t i = 0; i < cmdc; ++i) {
        command_t *cmd        = cmdv[i];
        int        similarity = calculate_similarity(input, cmd->name);

        if (similarity > max_similarity) {
            max_similarity = similarity;
            closest_match  = cmd->name;
        } else {
            for (size_t j = 0; j < cmd->aliasc; ++j) {
                similarity = calculate_similarity(input, cmd->aliasv[j]);

                if (similarity > max_similarity) {
                    max_similarity = similarity;
                    closest_match  = cmd->aliasv[j];
                }
            }
        }
    }

    return closest_match;
}
