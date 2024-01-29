#ifndef CLI_H
#define CLI_H

#include "argument.h"
#include "command.h"
#include "input.h"
#include "option.h"
#include "output.h"
#include "program.h"

#ifndef CLIDEF
# define CLIDEF
#endif // CLIDEF

#include <assert.h>

// TODO: add usage cmd in case of errors (just a more detailed usage, not the list of opts and args etc)
// TODO: add more DX fns
// TODO: add autocompletion (see readline/readline.h and readline/history.h
// TODO: add optional ascii art
// TODO: add command name aliases
// TODO: add option to register more global opts
// TODO: implement more assertions
// TODO: handle negatable options (find, print)
// TODO: --ansi|--no-ansi option
// TODO: add interactivity fns (open question, yes/no question, multiple choice)
// TODO: -n, --no-interaction option
// TODO: --profile option

// ==== Definitions ===========================================================/
int        runApplication(Program *program, int argc, string argv[]);
int        runCommand(Command *cmd, int argc, string argv[]);

static int usage(Program *program, boolean versionOnly);
static int usageCmd(Command *cmd, Option **defaultOpts, usize defaultOptsCount);
static int
doRunCommand(Command *cmd, int argc, string argv[], uint offset, Option **defaultOpts, usize defaultOptsCount);
static void cleanup(Command *cmd);

#endif // CLI_H

// dev
#define CLI_IMPLEMENTATION

// ==== Implementation ========================================================/
#ifdef CLI_IMPLEMENTATION

int runApplication(Program *program, int argc, string argv[]) {
    assert(program != NULL);

    // add global opts and arg to program
    Option helpOpt      = optionCreate("help", "h", "Display help.");
    Option quietOpt     = optionCreate("quiet", "q", "Do not output any message.");
    Option versionOpt   = optionCreate("version", "V", "Show the current version of the program.");
    Option verbosityOpt = optionCreate("verbose", "v", "Set the log level for the program.");
    optionSetFlag(&verbosityOpt, OPTION_LEVELS);

    usize optsCount = 3;
    if (program->version) {
        ++optsCount;
    }

    Option *defaultOpts[optsCount];
    defaultOpts[0] = &helpOpt;
    defaultOpts[1] = &quietOpt;
    if (program->version) {
        defaultOpts[2] = &versionOpt;
    }
    defaultOpts[optsCount - 1] = &verbosityOpt;

    Argument cmdArg            = argumentCreate("cmd", "The command to call.");
    argumentMakeRequired(&cmdArg, NULL);
    Argument *args[]   = {&cmdArg};

    program->opts      = defaultOpts;
    program->optsCount = ARRAY_LEN(defaultOpts);
    program->args      = args;
    program->argsCount = ARRAY_LEN(args);

    // program invoked without any args, print usage
    if (argc <= 1) {
        return usage(program, false);
    }

    // parse input
    usize argvOffset = 1;
    argumentValidateOrder(program->args, program->argsCount);
    string error =
        parseArgs(program->opts, program->optsCount, program->args, program->argsCount, argc, argv, argvOffset);

    // handle global opts
    boolean wantsHelp   = optionGetBool(program->opts, program->optsCount, "help");
    boolean showVersion = optionGetBool(program->opts, program->optsCount, "version");
    boolean beQuiet     = optionGetBool(program->opts, program->optsCount, "quiet");
    uint    verbosity   = optionGetLevel(program->opts, program->optsCount, "verbose");

    if ((argc == 2 && wantsHelp) || showVersion) {
        return usage(program, showVersion);
    }

    if (error) {
        printError(error);
        free(error);
        return EXIT_FAILURE;
    }

    if (verbosity > 0) {
        ++argvOffset;
        switch (verbosity + 1) {
            case LOG_LEVEL_ERROR:
                setLogLevel(LOG_LEVEL_ERROR);
                break;
            case LOG_LEVEL_WARNING:
                setLogLevel(LOG_LEVEL_WARNING);
                break;
            case LOG_LEVEL_INFO:
                setLogLevel(LOG_LEVEL_INFO);
                break;
            case LOG_LEVEL_DEBUG:
                setLogLevel(LOG_LEVEL_DEBUG);
                break;
            case LOG_LEVEL_QUIET:
            default:
                break;
        }
    }

    if (beQuiet) {
        setLogLevel(LOG_LEVEL_QUIET);
        ++argvOffset;
    }

    // find and run command
    string   cmd     = argumentGet(program->args, program->argsCount, "cmd");
    Command *command = commandFind(program->commands, program->commandsCount, cmd);

    if (!command) {
        printError("Unknown command: %s", cmd);
        return 1;
    } else {
        ++argvOffset;
    }

    return doRunCommand(command, argc, argv, argvOffset, program->opts, program->optsCount);
}

int runCommand(Command *cmd, int argc, string argv[]) {
    return doRunCommand(cmd, argc, argv, 1, NULL, 0); // todo: append default opts
}

static int usage(Program *program, boolean versionOnly) {
    printf(GREEN "%s" NO_COLOUR, program->name);
    if (program->version != NULL) {
        printf(" version %s%s%s", YELLOW, program->version, NO_COLOUR);
    }
    printf("\n");

    if (versionOnly) {
        return 1;
    }

    printf("\n");
    printf(YELLOW "Usage:\n" NO_COLOUR);
    printf("  command [options] [arguments]\n");

    uint width = max(optionGetPrintWidth(program->opts, program->optsCount),
                     commandGetPrintWidth(program->commands, program->commandsCount));

    printf("\n");
    printf(YELLOW "Options:\n" NO_COLOUR);
    optionPrintAll(program->opts, program->optsCount, width);

    printf("\n");
    printf(YELLOW "Available commands:\n" NO_COLOUR);
    commandPrintAll(program->commands, program->commandsCount, width);

    return 1;
}

static int usageCmd(Command *cmd, Option **defaultOpts, usize defaultOptsCount) {
    printf(YELLOW "Description:\n" NO_COLOUR);
    printf("  %s\n", cmd->description);

    printf("\n");
    printf(YELLOW "Usage:\n" NO_COLOUR);
    printf("  ");
    printf("%s [options]", cmd->name);

    if (cmd->argsCount > 0) {
        printf(" [--] ");

        boolean hasRequiredArgs = argumentIsRequired(cmd->args[0]);

        if (!hasRequiredArgs) {
            printf("[");
        }

        for (usize i = 0; i < cmd->argsCount; ++i) {
            Argument *arg      = cmd->args[i];
            boolean   required = argumentIsRequired(arg);
            boolean   multi    = argumentIsArray(arg);
            printf("%s<%s>%s", !required && i > 0 ? "[" : "", cmd->args[i]->name, !required && i > 0 ? "]" : "");

            if (multi) {
                printf("...");
            }
        }

        if (!hasRequiredArgs) {
            printf("]");
        }

    }
    printf("\n");

    uint optWidth =
        max(optionGetPrintWidth(cmd->opts, cmd->optsCount), optionGetPrintWidth(defaultOpts, defaultOptsCount));
    uint argWidth = argumentGetPrintWidth(cmd->args, cmd->argsCount);
    uint width    = max(optWidth, argWidth);

    printf("\n");
    printf(YELLOW "Options:\n" NO_COLOUR);
    if (cmd->optsCount > 0) {
        optionPrintAll(cmd->opts, cmd->optsCount, width);
    }
    optionPrintAll(defaultOpts, defaultOptsCount, width);

    if (cmd->argsCount > 0) {
        printf("\n");
        printf(YELLOW "Arguments:\n" NO_COLOUR);
        argumentPrintAll(cmd->args, cmd->argsCount, width);
    }

    if (cmd->additionalInfo) {
        printf("\n");
        printf(YELLOW "Help:\n" NO_COLOUR);
        printf("  %s\n", cmd->additionalInfo);
    }

    return 1;
}

static int
doRunCommand(Command *cmd, int argc, string argv[], uint offset, Option **defaultOpts, usize defaultOptsCount) {
    assert(cmd != NULL);
    assert(defaultOpts != NULL);

    // merge opts
    usize   optsCount = cmd->optsCount + defaultOptsCount;
    Option *opts[optsCount];
    for (usize i = 0; i < defaultOptsCount; ++i) {
        opts[i] = defaultOpts[i];
    }
    for (usize i = 0; i < cmd->optsCount; ++i) {
        opts[i + defaultOptsCount] = cmd->opts[i];
    }

    // parse input
    argumentValidateOrder(cmd->args, cmd->argsCount);
    string err = parseArgs(opts, optsCount, cmd->args, cmd->argsCount, argc, argv, offset);

    // handle opts
    boolean wantsHelp = optionGetBool(opts, optsCount, "help");
    if (wantsHelp) {
        usageCmd(cmd, defaultOpts, defaultOptsCount);
        cleanup(cmd);
        return EXIT_FAILURE;
    }

    // args error, exit
    if (err) {
        printError(err);
        free(err);
        cleanup(cmd);
        return EXIT_FAILURE;
    }

    // run command
    int code = cmd->operation(cmd);
    cleanup(cmd);
    return code;
}

static void cleanup(Command *cmd) {
    assert(cmd != NULL);

    for (usize i = 0; i < cmd->optsCount; ++i) {
        Option *opt = cmd->opts[i];
        assert(opt != NULL);

        if (opt->__meta & OPTION_META_CLEANUP_VALUE) {
            free(opt->stringValue);
            opt->stringValue = NULL;
        }

        if (opt->__meta & OPTION_META_CLEANUP_VALUE_ARRAY) {
            for (usize i = 0; i < opt->arraySize; ++i) {
                free(opt->stringArray[i]);
                opt->stringArray[i] = NULL;
            }
            free(opt->stringArray);
            opt->stringArray = NULL;
            opt->arraySize   = 0;
        }
    }

    for (usize i = 0; i < cmd->argsCount; ++i) {
        Argument *arg = cmd->args[i];
        assert(arg != NULL);

        free(arg->values);
        arg->values      = NULL;
        arg->valuesCount = 0;
    }
}

#endif // CLI_IMPLEMENTATION
