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

// TODO: add option to register more global opts
// TODO: add interactivity fns (open question, yes/no question, multiple choice)
// TODO: -n, --no-interaction option
// TODO: --profile option

// ==== Definitions ===========================================================/
int        runApplication(Program *program, int argc, string argv[]);
int        runCommand(Command *cmd, int argc, string argv[]);

static int usage(Program *program, boolean versionOnly);
static int usageCmd(Command *cmd, Option **defaultOpts, usize defaultOptsCount);
static int usageStringCmd(Command *cmd);
static int
doRunCommand(Command *cmd, int argc, string argv[], uint offset, Option **defaultOpts, usize defaultOptsCount);
static void cleanup(Command *cmd);
static boolean handleAnsiOpt(Option **opts, usize optsCount);
static boolean handleVerbosityOpt(Option **opts, usize optsCount);
static boolean handleQuietOpt(Option **opts, usize optsCount);
static uint handleDefaultOpts(Option **opts, usize optsCount);

#endif // CLI_H

// dev
#define CLI_IMPLEMENTATION

// ==== Implementation ========================================================/
#ifdef CLI_IMPLEMENTATION

int runApplication(Program *program, int argc, string argv[]) {
    assert(program != NULL);

    // create global opts that are added to program and running command
    Option helpOpt      = optionCreate("help", "h", "Display help.");
    Option quietOpt     = optionCreate("quiet", "q", "Do not output any message.");
    Option versionOpt   = optionCreate("version", "V", "Show the current version of the program.");
    Option ansiOpt      = optionCreateBoolOpt("ansi", NULL, "Force (or disable) ANSI output.", true, true);
    Option verbosityOpt = optionCreate("verbose", "v", "Set the log level for the program.");
    optionSetFlag(&verbosityOpt, OPTION_LEVELS);

    usize optsCount = 4;
    if (program->version) {
        ++optsCount;
    }

    // configure default opts array
    Option *defaultOpts[optsCount];
    defaultOpts[0] = &helpOpt;
    defaultOpts[1] = &quietOpt;
    defaultOpts[2] = &ansiOpt;
    if (program->version) {
        defaultOpts[optsCount - 2] = &versionOpt;
    }
    defaultOpts[optsCount - 1] = &verbosityOpt;

    // create command arg to catch the command the user wants to run
    Argument  cmdArg = argumentCreate("cmd", "The command to call.");
    Argument *args[] = {&cmdArg};

    // bind default opts and args to program
    programSetOpts(program, defaultOpts, ARRAY_LEN(defaultOpts));
    programSetArgs(program, args, ARRAY_LEN(args));

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
    argvOffset += handleDefaultOpts(program->opts, program->optsCount);

    // usage (help or version)
    boolean wantsHelp   = optionGetBool(program->opts, program->optsCount, "help");
    boolean showVersion = optionGetBool(program->opts, program->optsCount, "version");
    if ((argc == 2 && wantsHelp) || showVersion) {
        return usage(program, showVersion);
    }

    // exit if input error
    if (error) {
        printErrorLarge(error);
        free(error);
        return EXIT_FAILURE;
    }

    // get provided command
    string cmd = argumentGet(program->args, program->argsCount, "cmd");

    // if no command is given, print usage
    if (!cmd) {
        return usage(program, false);
    }

    // find and run command
    Command *command = commandFind(program->commands, program->commandsCount, cmd);

    if (!command) {
        printErrorLarge("Unknown command: %s", cmd);
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
    if (program->art) {
        printf("%s\n", program->art);
    }

    pcolorf(GREEN, "%s", program->name);
    if (program->version != NULL) {
        printf(" version %s%s%s", COLOR(YELLOW), program->version, NO_COLOUR);
    }
    printf("\n");

    if (versionOnly) {
        return 1;
    }

    printf("\n");
    pcolor(YELLOW, "Usage:\n");
    printf("  command [options] [arguments]\n");

    uint width = max(optionGetPrintWidth(program->opts, program->optsCount),
                     commandGetPrintWidth(program->commands, program->commandsCount));

    printf("\n");
    pcolor(YELLOW, "Options:\n");
    optionPrintAll(program->opts, program->optsCount, width);

    printf("\n");
    pcolor(YELLOW, "Available commands:\n");
    commandPrintAll(program->commands, program->commandsCount, width);

    return 1;
}

static int usageCmd(Command *cmd, Option **defaultOpts, usize defaultOptsCount) {
    pcolor(YELLOW, "Description:\n");
    printf("  %s\n", cmd->description);

    printf("\n");
    pcolor(YELLOW, "Usage:\n");
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
            assert(arg != NULL);
            assert(arg->name != NULL);

            boolean   required = argumentIsRequired(arg);
            boolean   multi    = argumentIsArray(arg);

            if (required && multi && arg->minCount > 1) {
                for (usize j = 0; j < arg->minCount - 1; ++j) {
                    printf("<%s> ", arg->name);
                }
            }

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

    if (cmd->aliases) {
        for (usize i = 0; i < cmd->aliasesCount; ++i) {
            printf("  %s\n", cmd->aliases[i]);
        }
    }

    uint optWidth =
        max(optionGetPrintWidth(cmd->opts, cmd->optsCount), optionGetPrintWidth(defaultOpts, defaultOptsCount));
    uint argWidth = argumentGetPrintWidth(cmd->args, cmd->argsCount);
    uint width    = max(optWidth, argWidth);

    printf("\n");
    pcolor(YELLOW, "Options:\n");
    if (cmd->optsCount > 0) {
        optionPrintAll(cmd->opts, cmd->optsCount, width);
    }
    optionPrintAll(defaultOpts, defaultOptsCount, width);

    if (cmd->argsCount > 0) {
        printf("\n");
        pcolor(YELLOW, "Arguments:\n");
        argumentPrintAll(cmd->args, cmd->argsCount, width);
    }

    if (cmd->additionalInfo) {
        printf("\n");
        pcolor(YELLOW, "Help:\n");
        printf("  %s\n", cmd->additionalInfo);
    }

    return 1;
}

static int usageStringCmd(Command *cmd) {
    printf(COLOR(GREEN));
    printf("%s", cmd->name);

    for (usize i = 0; i < cmd->optsCount; ++i) {
        optionPrintTag(cmd->opts[i]);
    }

    if (cmd->argsCount > 0) {
        printf(" [--]");
        for (usize i = 0; i < cmd->argsCount; ++i) {
            argumentPrintTag(cmd->args[i]);
        }
    }

    printf("%s\n", NO_COLOUR);

    return EXIT_FAILURE;
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

    // handle default opts
    handleDefaultOpts(defaultOpts, defaultOptsCount);

    boolean wantsHelp = optionGetBool(opts, optsCount, "help");
    if (wantsHelp) {
        usageCmd(cmd, defaultOpts, defaultOptsCount);
        cleanup(cmd);
        return EXIT_FAILURE;
    }

    // args error, exit
    if (err) {
        printErrorLarge(err);
        free(err);
        printNewLine();
        usageStringCmd(cmd);
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

static boolean handleAnsiOpt(Option **opts, usize optsCount) {
    Option *opt = optionFind(opts, optsCount, "ansi", NULL);

    if (!opt) {
        return false;
    }

    if (opt->provided) {
        if (opt->boolValue) {
            enableAnsi();
        } else {
            disableAnsi();
        }
    }

    return opt->provided;
}

static boolean handleVerbosityOpt(Option **opts, usize optsCount) {
    Option *opt = optionFind(opts, optsCount, "verbose", NULL);

    if (!opt || !opt->provided || opt->levelValue <= 0) {
        return false;
    }

    uint verbosity = opt->levelValue;

    if (verbosity > 0) {
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

    return true;
}

static boolean handleQuietOpt(Option **opts, usize optsCount) {
    Option *opt = optionFind(opts, optsCount, "quiet", "q");

    if (!opt) {
        return false;
    }

    if (opt->boolValue) {
        setLogLevel(LOG_LEVEL_QUIET);
    }

    return opt->provided;
}

static uint handleDefaultOpts(Option **opts, usize optsCount) {
    uint offset = 0;

    if (handleAnsiOpt(opts, optsCount)) {
        ++offset;
    }

    if (handleVerbosityOpt(opts, optsCount)) {
        ++offset;
    }

    if (handleQuietOpt(opts, optsCount)) {
        ++offset;
    }

    return offset;
}

#endif // CLI_IMPLEMENTATION
