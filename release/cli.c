#include <argument.h>
#include <array.h>
#include <assert.h>
#include <cli.h>
#include <colors.h>
#include <command.h>
#include <cstring.h>
#include <input.h>
#include <math-utils.h>
#include <option.h>
#include <output.h>
#include <program.h>
#include <usage.h>

// TODO: add option to register more global opts
// TODO: add interactivity fns (open question, yes/no question, multiple choice)
// TODO: -n, --no-interaction option
// TODO: --profile option

static option_t **global_optv = NULL;
static size_t     global_optc = 0;

static int        do_run_command(command_t *cmd, int argc, char **argv, int offset);
static void       cleanup(command_t *cmd);
static void       free_global_opts(void);
bool              register_global_opt(option_t *opt);
static void       create_global_opts(bool has_version);
static void       command_append_global_opts(command_t *cmd, option_t **buf, size_t buflen);
static bool       handle_ansi_opt(void);
static bool       handle_verbosity_opt(void);
static bool       handle_quiet_opt(void);
static void       handle_global_optv(void);

int               run_application(program_t *program, int argc, char **argv) {
    assert(program != NULL);

    create_global_opts(program->version != NULL);

    // create command arg to catch the command the user wants to run
    argument_t  cmd_arg = argument_create("cmd", "The command to call.");
    argument_t *args[]  = {&cmd_arg};

    // bind default opts and args to program
    program_set_opts(program, global_optv, global_optc);
    program_set_args(program, args, ARRAY_LEN(args));

    // program invoked without any args, print usage
    if (argc <= 1) {
        int code = usage(program, false);
        free_global_opts();
        return code;
    }

    // parse input
    char errbuf[1024];
    errbuf[0] = '\0';
    input_parser_t parser = input_parser_create(program->argv, program->argc, program->optv, program->optc, errbuf, 1);
    argument_validate_order(program->argv, program->argc);
    input_parse(&parser, argc, argv);

    // handle global opts
    handle_global_optv();

    // usage (help or version)
    bool wants_help   = option_get_bool(program->optv, program->optc, "help");
    bool show_version = option_get_bool(program->optv, program->optc, "version");
    if (wants_help || show_version) {
        int code = usage(program, show_version);
        free_global_opts();
        return code;
    }

    // exit if input error
    if (!str_is_empty(errbuf)) {
        erro(errbuf);
        new_line();
        usage_string_cmd("command", program->optv, program->optc, NULL, 0);
        free_global_opts();
        return EXIT_FAILURE;
    }

    // get provided command
    char *cmd = argument_get(program->argv, program->argc, "cmd");

    // if no command is given, print usage
    if (!cmd) {
        int code = usage(program, false);
        free_global_opts();
        return code;
    }

    // find and run command
    const char *matches[program->cmdc];
    int         found = command_find_loose(program->cmdv, program->cmdc, cmd, matches, program->cmdc);

    if (found == 0) {
        free_global_opts();
        panicf("Unknown command: \"%s\"", cmd);
    }

    if (found > 1) {
        free_global_opts();
        char errbuf[1024];
        sprintf(errbuf,
                "command_t \"%s\" is ambigious.\n"
                              "Did you mean one of these?",
                cmd);
        panic(errbuf);
    }

    command_t *command  = NULL;

    size_t     cmdlen   = strlen(cmd);
    size_t     matchlen = strlen(matches[0]);
    // TODO: move to sep fn?
    if (cmdlen > matchlen || (cmdlen == matchlen && !str_equals(cmd, matches[0]))) {
        log_level_t log_level = get_log_level();
        if (log_level == LOG_LEVEL_QUIET) {
            free_global_opts();
            return EXIT_FAILURE;
        }

        errof("Unknown command: \"%s\"", cmd);
        new_line();
        char question[1024];
        char input[4];
        sprintf(question, "%sDo you want to run \"%s\" instead? %s", GREEN, matches[0], NO_COLOR);
        sprintf(question, "[");
        sprintf(question, "%sy/N%s", YELLOW, NO_COLOR);
        sprintf(question, "]:\n");
        sprintf(question, "> ");
        writeln(question);
        fgets(input, sizeof(input), stdin);
        char answer[4];
        str_trim(input, answer);

        if (str_starts_with(answer, "y")) {
            command = command_find(program->cmdv, program->cmdc, matches[0]);
            new_line();
        } else {
            free_global_opts();
            return 1;
        }
    } else {
        command = command_find(program->cmdv, program->cmdc, matches[0]);
    }

    assert(command != NULL);

    if (command->handler != NULL) {
        return command->handler(command, argc, argv);
    }

    return do_run_command(command, argc, argv, parser.offset + parser.parsed);
}

int run_command(command_t *cmd, int argc, char *argv[]) {
    return do_run_command(cmd, argc, argv, cmd->handler ? 2 : 1);
}

static void create_global_opts(bool has_version) {
    if (global_optv != NULL) {
        return;
    }

    option_t *help_opt      = malloc(sizeof(*help_opt));
    option_t *quiet_opt     = malloc(sizeof(*help_opt));
    option_t *ansi_opt      = malloc(sizeof(*help_opt));
    option_t *version_opt   = NULL;
    option_t *verbosity_opt = malloc(sizeof(*help_opt));

    assert(help_opt != NULL);
    assert(quiet_opt != NULL);
    assert(ansi_opt != NULL);
    assert(verbosity_opt != NULL);

    option_init(help_opt, "help", "h", "Display help.");
    option_init(quiet_opt, "quiet", "q", "Do not output any message.");
    option_init(ansi_opt, "ansi", NULL, "Force (or disable) ANSI output.");
    option_add_flag(ansi_opt, OPTION_VALUE_NEGATABLE);
    option_set_boolval(ansi_opt, true);
    option_init(verbosity_opt, "verbose", "v", "Set the log level for the program.");
    option_set_flag(verbosity_opt, OPTION_LEVELS);

    global_optc = 4;
    if (has_version) {
        version_opt = malloc(sizeof(*help_opt));
        assert(version_opt != NULL);
        option_init(version_opt, "version", "V", "Show the current version of the program.");
        ++global_optc;
    }

    global_optv = malloc((global_optc + 1) * sizeof(option_t *));
    assert(global_optv != NULL);
    global_optv[0] = help_opt;
    global_optv[1] = quiet_opt;
    global_optv[2] = ansi_opt;
    if (has_version) {
        global_optv[global_optc - 2] = version_opt;
    }
    global_optv[global_optc - 1] = verbosity_opt;

    for (size_t i = 0; i < global_optc; ++i) {
        option_t *opt = global_optv[i];
        opt->__meta |= OPTION_META_CLEANUP_OPT;
    }
}

static int do_run_command(command_t *cmd, int argc, char **argv, int offset) {
    assert(cmd != NULL);
    assert(cmd->operation != NULL);

    create_global_opts(false);
    size_t    optc = global_optc + cmd->optc;
    option_t *optv[optc];
    command_append_global_opts(cmd, optv, optc);

    // parse input
    char errbuf[1024];
    errbuf[0]             = '\0';
    input_parser_t parser = input_parser_create(cmd->argv, cmd->argc, cmd->optv, cmd->optc, errbuf, offset);
    argument_validate_order(cmd->argv, cmd->argc);
    input_parse(&parser, argc, argv);

    // handle default opts
    handle_global_optv();

    bool wants_help = option_get_bool(cmd->optv, cmd->optc, "help");
    if (wants_help) {
        usage_cmd(cmd);
        cleanup(cmd);
        return EXIT_FAILURE;
    }

    // args error, exit
    if (errbuf[0] != '\0') {
        erro(errbuf);
        new_line();
        usage_string_cmd(cmd->name, cmd->optv, cmd->optc, cmd->argv, cmd->argc);
        cleanup(cmd);
        return EXIT_FAILURE;
    }

    // run command
    int code = cmd->operation(cmd);
    cleanup(cmd);
    return code;
}

static void command_append_global_opts(command_t *cmd, option_t **buf, size_t buflen) {
    assert(buflen == cmd->optc + global_optc);

    size_t i = 0;
    if (cmd->optv) {
        for (; i < cmd->optc; ++i) {
            buf[i] = cmd->optv[i];
        }
    }

    if (global_optv) {
        for (size_t j = 0; i < buflen; ++i, ++j) {
            buf[i] = global_optv[j];
        }
    }

    cmd->optv = buf;
    cmd->optc = buflen;
}

static void free_global_opts(void) {
    if (global_optv) {
        for (size_t i = 0; i < global_optc; ++i) {
            free(global_optv[i]);
        }
        free(global_optv);
        global_optv = NULL;
    }
}

static void cleanup(command_t *cmd) {
    assert(cmd != NULL);

    // global opts should be merged here so be included in the cleanup
    for (size_t i = 0; i < cmd->optc; ++i) {
        option_t *opt = cmd->optv[i];
        if (opt == NULL) {
            continue;
        }

        if (opt->__meta & OPTION_META_CLEANUP_VALUE) {
            free(opt->stringval);
            opt->stringval = NULL;
        }

        if (opt->__meta & OPTION_META_CLEANUP_VALUE_ARRAY) {
            for (size_t i = 0; i < opt->stringc; ++i) {
                free(opt->stringv[i]);
                opt->stringv[i] = NULL;
            }
            free(opt->stringv);
            opt->stringv = NULL;
            opt->stringc = 0;
        }

        if (opt->__meta & OPTION_META_CLEANUP_OPT) {
            free(opt);
        }

        cmd->optv[i] = NULL;
    }

    for (size_t i = 0; i < cmd->argc; ++i) {
        argument_t *arg = cmd->argv[i];
        assert(arg != NULL);

        free(arg->valuev);
        arg->valuev = NULL;
        arg->valuec = 0;
    }

    if (global_optv) {
        free(global_optv);
        global_optv = NULL;
    }
}

static bool handle_ansi_opt(void) {
    option_t *opt = option_find(global_optv, global_optc, "ansi", NULL);

    if (!opt) {
        return false;
    }

    if (opt->provided) {
        if (opt->boolval) {
            enable_ansi();
        } else {
            disable_ansi();
        }
    }

    return opt->provided;
}

static bool handle_verbosity_opt(void) {
    option_t *opt = option_find(global_optv, global_optc, "verbose", NULL);

    if (!opt || !opt->provided || opt->levelval <= 0) {
        return false;
    }

    int verbosity = opt->levelval;

    if (verbosity > 0) {
        switch (verbosity + 1) {
            case LOG_LEVEL_ERROR:
                set_log_level(LOG_LEVEL_ERROR);
                break;
            case LOG_LEVEL_WARNING:
                set_log_level(LOG_LEVEL_WARNING);
                break;
            case LOG_LEVEL_INFO:
                set_log_level(LOG_LEVEL_INFO);
                break;
            case LOG_LEVEL_DEBUG:
                set_log_level(LOG_LEVEL_DEBUG);
                break;
            case LOG_LEVEL_QUIET:
            default:
                break;
        }
    }

    return true;
}

static bool handle_quiet_opt(void) {
    option_t *opt = option_find(global_optv, global_optc, "quiet", "q");

    if (!opt) {
        return false;
    }

    if (opt->boolval) {
        set_log_level(LOG_LEVEL_QUIET);
    }

    return opt->provided;
}

static void handle_global_optv(void) {
    handle_ansi_opt();
    handle_verbosity_opt();
    handle_quiet_opt();
}
