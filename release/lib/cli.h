#ifndef CLI_H
#define CLI_H

#define UTILS_IMPLEMENTATION
#include "utils.h"
#define CSTRING_IMPLEMENTATION
#include "cstring.h"
#include "types.h"

#ifndef CLIDEF
# define CLIDEF
#endif // CLIDEF

// TODO: add global options (verbose, silenced, ...)
// TODO: add command name aliases
// TODO: make application run like a command itself
// TODO: use String struct instead of cstring/string
// TODO: add multiple logging/verbosity levels
// TODO: add interactivity fns (open question, yes/no question, multiple choice)
// TODO: add printErr, printInfo, printWarn, printDebug fns -> global print fn that accepts log level and app config (silenced?)
// TODO: add type cast/assert fns. add types to options/args ?
// TODO: add progress bar

typedef struct Option {
    cstring shortName;
    cstring longName;
    cstring description;
    string  stringValue;
    boolean isBool;
    boolean boolValue;
} Option;

typedef struct Argument {
    cstring name;
    cstring description;
    string  value;
} Argument;

typedef struct Command Command;

typedef int(Operation)(Command *self);

typedef struct Command {
    cstring    name;
    cstring    description;
    cstring    additionalInfo;
    Operation *operation;
    Option   **opts;
    Argument **args;
    usize      optsCount;
    usize      argsCount;
} Command;

typedef struct Program {
    cstring   name;
    cstring   executableName;
    cstring   version;
    Command **commands;
    usize     commandsCount;
} Program;

int      usage(Program *program, cstring commandName);
int      usageCmd(Command *cmd);
int      initOption(Option *opt, cstring shortName, cstring longName, cstring description, boolean boolean);
int      initBoolOption(Option *opt, cstring shortName, cstring longName, cstring description);
int      initStringOption(Option *opt, cstring shortName, cstring longName, cstring description, string defaultValue);
int      initArgument(Argument *arg, cstring name, cstring description);
int      initCommand(Command *cmd, cstring name, cstring description, Operation *op);
int      initProgram(Program *program, cstring name, cstring executableName, cstring version);
int      addAdditionalInfoToCommand(Command *cmd, cstring additionalInfo);
Option   createOption(cstring shortName, cstring longName, cstring description, boolean boolean);
Option   createBoolOption(cstring shortName, cstring longName, cstring description);
Option   createStringOption(cstring shortName, cstring longName, cstring description, string defaultValue);
Argument createArgument(cstring name, cstring description);
Command  createCommand(cstring name, cstring description, Operation *op);
Program  createProgram(cstring name, cstring executableName, cstring version);
boolean  getBoolOpt(Command *cmd, cstring longName);
string   getStringOpt(Command *cmd, cstring longName);
string   getArg(Command *cmd, cstring name);
int      runApplication(Program *program, int argc, string argv[]);
int      runCommand(Command *cmd, int argc, string argv[]);

static boolean cli__wantsHelp(cstring arg);
static usize   cli__parseArgs(Command *cmd, int argc, string argv[], int offset);
static int     cli__runCommand(Command *cmd, int argc, string argv[], int offset);
static int     cli__cmdUsage(Command *cmd, cstring programName);
static void    cli__printOption(const Option *opt, int width);
static int     cli__printVersion(Program *program);

#endif // CLI_H

#ifdef CLI_IMPLEMENTATION

int runApplication(Program *program, int argc, string argv[]) {
    string executableCommand = argv[1];

    // ./bin/main (-h|--help)
    if (argc == 1 || cli__wantsHelp(executableCommand)) {
        return usage(program, NULL);
    }

    if (program->version != NULL &&
        (cstringEquals(executableCommand, "-v") || cstringEquals(executableCommand, "--version"))) {
        return cli__printVersion(program);
    }

    // ./bin/main <command> -h|--help
    if (cli__wantsHelp(argv[2])) {
        return usage(program, executableCommand);
    }

    int index = -1;
    for (usize i = 0; i < program->commandsCount; ++i) {
        if (cstringEquals(program->commands[i]->name, executableCommand)) {
            index = i;
            break;
        }
    }

    if (index < 0) {
        char errorMsg[128];
        sprintf(errorMsg, "Unknown command: %s.", executableCommand);
        panic(errorMsg);
    }

    Command *cmd = program->commands[index];

    return cli__runCommand(cmd, argc, argv, 2);
}

int runCommand(Command *cmd, int argc, string argv[]) {
    // ./bin/main (-h|--help)
    if (cli__wantsHelp(argv[1])) {
        return cli__cmdUsage(cmd, NULL);
    }

    return cli__runCommand(cmd, argc, argv, 1);
}

static int cli__runCommand(Command *cmd, int argc, string argv[], int offset) {
    if (cmd == NULL) {
        return 1;
    }

    usize args = cli__parseArgs(cmd, argc, argv, offset);

    if (args < cmd->argsCount) {
        char errorMsg[128];
        sprintf(errorMsg, "Too few arguments. Expected %zu arguments, but received %zu.", cmd->argsCount, args);
        panic(errorMsg);
    }

    return cmd->operation(cmd);
}

static usize cli__parseArgs(Command *cmd, int argc, string argv[], int offset) {
    boolean parsingArguments = false;
    usize   currentArgIndex  = 0;

    for (int i = offset; i < argc; ++i) {
        string currentArg = argv[i];

        if (currentArg == NULL) {
            if (currentArgIndex < cmd->argsCount) {
                char errorMsg[128];
                sprintf(errorMsg,
                        "Too few arguments. Expected %zu arguments, but received %zu.",
                        cmd->argsCount,
                        currentArgIndex);
                panic(errorMsg);
            }

            break;
        }

        boolean isLongOption  = cstringStartsWith(currentArg, "--");
        boolean isShortOption = !isLongOption && cstringStartsWith(currentArg, "-");

        if (isLongOption || isShortOption) {
            if (parsingArguments) {
                Argument *expectedArg = cmd->args[currentArgIndex];
                char      errorMsg[128];
                sprintf(errorMsg,
                        "Invalid args. Expected argument '%s', but received option '%s'.",
                        expectedArg->name,
                        currentArg);
                panic(errorMsg);
            }

            Option *currentOption = NULL;
            usize   argLen        = strlen(currentArg);

            char    nameBuffer[64];
            cstr name = stringToCstring(stringSlice(stringFrom(currentArg), isLongOption ? 2 : 1, argLen, nameBuffer));

            for (usize j = 0; j < cmd->optsCount; ++j) {
                Option *opt = cmd->opts[j];
                if (isLongOption && cstringEquals(opt->longName, name)) {
                    currentOption = opt;
                    break;
                } else if (isShortOption && cstringEquals(opt->shortName, name)) {
                    currentOption = opt;
                    break;
                }
            }

            if (currentOption == NULL) {
                char errorMsg[128];
                sprintf(errorMsg, "Received unknown option: %s.", name);
                panic(errorMsg);
            }

            if (currentOption->isBool) {
                currentOption->boolValue = true;
            } else {
                string nextArg = argv[++i];

                if (nextArg == NULL || cstringStartsWith(nextArg, "--") || cstringStartsWith(nextArg, "-")) {
                    char errorMsg[128];
                    sprintf(errorMsg, "Expected value for option '%s', but received none.", name);
                    panic(errorMsg);
                }

                currentOption->stringValue = nextArg;
            }
        } else {
            parsingArguments = true;

            if (cmd->args == NULL || currentArgIndex >= cmd->argsCount) {
                char errorMsg[128];
                sprintf(errorMsg,
                        "Too many arguments. Expected %zu arguments, but received %zu.",
                        cmd->argsCount,
                        currentArgIndex + 1);
                panic(errorMsg);
            }

            cmd->args[currentArgIndex++]->value = currentArg;
        }
    }

    return currentArgIndex;
}

static boolean cli__wantsHelp(cstring Arg) {
    return cstringEquals(Arg, "-h") || cstringEquals(Arg, "--help");
}

int initOption(Option *opt, cstring shortName, cstring longName, cstring description, boolean boolean) {
    if (opt == NULL) {
        return -1;
    }

    opt->shortName   = shortName;
    opt->longName    = longName;
    opt->isBool      = boolean;
    opt->boolValue   = false;
    opt->stringValue = NULL;
    opt->description = description;

    return 0;
}

int initBoolOption(Option *opt, cstring shortName, cstring longName, cstring description) {
    return initOption(opt, shortName, longName, description, true);
}

int initStringOption(Option *opt, cstring shortName, cstring longName, cstring description, string defaultValue) {
    int success = initOption(opt, shortName, longName, description, false);

    if (success == 0) {
        opt->stringValue = defaultValue;
    }

    return success;
}

int initArgument(Argument *arg, cstring name, cstring description) {
    if (arg == NULL) {
        return -1;
    }

    arg->name        = name;
    arg->description = description;
    arg->value       = NULL;

    return 0;
}

int initCommand(Command *cmd, cstring name, cstring description, Operation *op) {
    if (cmd == NULL) {
        return -1;
    }

    cmd->name           = name;
    cmd->description    = description;
    cmd->additionalInfo = NULL;
    cmd->operation      = op;
    cmd->opts           = NULL;
    cmd->optsCount      = 0;
    cmd->args           = NULL;
    cmd->argsCount      = 0;

    return 0;
}

int addAdditionalInfoToCommand(Command *cmd, cstring additionalInfo) {
    if (cmd == NULL) {
        return -1;
    }

    cmd->additionalInfo = additionalInfo;
    return 0;
}

int initProgram(Program *program, cstring name, cstring executableName, cstring version) {
    if (program == NULL) {
        return -1;
    }

    program->name           = name;
    program->executableName = executableName;
    program->version        = version;
    program->commands       = NULL;
    program->commandsCount  = 0;

    return 0;
}

static void cli__printOption(const Option *opt, int width) {
    boolean isBool = opt->isBool;
    char    name[100];
    boolean hasShort = opt->shortName != NULL;

    if (isBool) {
        sprintf(name,
                "%s%s%s --%s",
                hasShort ? "-" : " ",
                hasShort ? opt->shortName : " ",
                hasShort ? "," : " ",
                opt->longName);
        printf("  %s%-*.*s%s %s\n", GREEN, width + 4 + 7, width + 7, name, NO_COLOUR, opt->description);
    } else {
        char    nameBuffer[64];
        cstring nameUpper = stringToCstring(stringToUpper(stringFrom(opt->longName), nameBuffer));
        sprintf(name,
                "%s%s%s --%s=%s",
                hasShort ? "-" : " ",
                hasShort ? opt->shortName : " ",
                hasShort ? "," : " ",
                opt->longName,
                nameUpper);
        printf("  %s%-*.*s%s %s", GREEN, width + 4 + 7, width + 7, name, NO_COLOUR, opt->description);

        if (opt->stringValue != NULL) {
            printf(" %s[default: \"%s\"]%s", YELLOW, opt->stringValue, NO_COLOUR);
        }

        printf("\n");
    }
}

static int cli__cmdUsage(Command *cmd, cstring programName) {
    printf("%sDescription:%s\n", YELLOW, NO_COLOUR);
    printf("  %s\n\n", cmd->description);

    printf("%sUsage:%s\n", YELLOW, NO_COLOUR);
    boolean hasOpts = cmd->opts != NULL;
    boolean hasArgs = cmd->args != NULL;
    cstring opts    = hasOpts ? " [opts]" : "";
    cstring args    = hasArgs ? " [args...]" : "";

    if (programName != NULL) {
        printf("  %s %s%s%s\n", programName, cmd->name, opts, args);
    } else {
        printf("  %s%s%s\n", cmd->name, opts, args);
    }

    printf("\n%sOptions:%s\n", YELLOW, NO_COLOUR);

    uint maxOptionNameLength = 4;
    for (usize i = 0; i < cmd->optsCount; ++i) {
        Option *opt         = cmd->opts[i];
        usize   len         = strlen(cmd->opts[i]->longName);
        maxOptionNameLength = max(maxOptionNameLength, opt->isBool ? len : ((len * 2) + 1));
    }

    printf("  %s%-*.*s %s%s\n",
           GREEN,
           maxOptionNameLength + 4 + 7,
           maxOptionNameLength + 7,
           "-h, --help",
           NO_COLOUR,
           "Display help.");

    if (hasOpts) {
        for (usize i = 0; i < cmd->optsCount; ++i) {
            Option *opt = cmd->opts[i];
            cli__printOption(opt, maxOptionNameLength);
        }
    }

    if (hasArgs) {
        uint maxArgNameLength = 0;
        for (usize i = 0; i < cmd->argsCount; ++i) {
            maxArgNameLength = max(maxArgNameLength, strlen(cmd->args[i]->name));
        }

        printf("\n%sArguments:%s\n", YELLOW, NO_COLOUR);

        for (usize i = 0; i < cmd->argsCount; ++i) {
            Argument *arg = cmd->args[i];
            printf("  %s%-*.*s %s%s\n",
                   GREEN,
                   maxArgNameLength + 4,
                   maxArgNameLength,
                   arg->name,
                   NO_COLOUR,
                   arg->description);
        }
    }

    if (cmd->additionalInfo != NULL) {
        String cmdName = stringFrom(cmd->name);
        char   buffer[stringLength(cmdName) + 1];
        String capitalizedName = stringCapitalize(cmdName, buffer);
        printf("\n%s%s:%s\n", YELLOW, stringToCstring(capitalizedName), NO_COLOUR);
        printf("  %s\n", cmd->additionalInfo);
    }

    return EXIT_FAILURE;
}

int usage(Program *program, cstring commandName) {
    if (commandName == NULL) {
        if (program->version != NULL) {
            cli__printVersion(program);
        } else {
            printf("%s%s%s\n", GREEN, program->name, NO_COLOUR);
        }

        printf("%s\nUsage:%s\n", YELLOW, NO_COLOUR);
        printf("  %s <command> [opts] [args...]\n", program->executableName);

        printf("\n%sOptions:%s\n", YELLOW, NO_COLOUR);
        printf("  %s-h, --help%s       Display help for the given command.\n", GREEN, NO_COLOUR);
        if (program->version != NULL) {
            printf("  %s-v, --version%s    Show the current version of the program.\n", GREEN, NO_COLOUR);
        }
        printf("\n");

        printf("%sAvailable commands:%s\n", YELLOW, NO_COLOUR);

        uint maxCommandNameLength = 0;
        for (usize i = 0; i < program->commandsCount; ++i) {
            maxCommandNameLength = max(maxCommandNameLength, strlen(program->commands[i]->name));
        }

        for (usize i = 0; i < program->commandsCount; ++i) {
            Command *cmd = program->commands[i];
            printf("  %s%-*s%s%s\n", GREEN, maxCommandNameLength + 4, cmd->name, NO_COLOUR, cmd->description);
        }

        return EXIT_FAILURE;
    }

    for (usize i = 0; i < program->commandsCount; ++i) {
        Command *cmd = program->commands[i];
        if (cstringEquals(cmd->name, commandName)) {
            return cli__cmdUsage(cmd, program->executableName);
        }
    }

    char errorMsg[128];
    sprintf(errorMsg, "Unknown command: %s.", commandName);
    panic(errorMsg);
    return EXIT_FAILURE;
}

int usageCmd(Command *cmd) {
    return cli__cmdUsage(cmd, NULL);
}

boolean getBoolOpt(Command *cmd, cstring longName) {
    for (usize i = 0; i < cmd->optsCount; ++i) {
        Option *opt = cmd->opts[i];

        if (cstringEquals(opt->longName, longName) && opt->isBool) {
            return opt->boolValue;
        }
    }

    return false;
}

string getStringOpt(Command *cmd, cstring longName) {
    for (usize i = 0; i < cmd->optsCount; ++i) {
        Option *opt = cmd->opts[i];

        if (cstringEquals(opt->longName, longName) && !opt->isBool) {
            return opt->stringValue;
        }
    }

    return NULL;
}

string getArg(Command *cmd, cstring name) {
    for (u32 i = 0; i < cmd->argsCount; ++i) {
        Argument *arg = cmd->args[i];

        if (cstringEquals(arg->name, name)) {
            return arg->value;
        }
    }

    return NULL;
}

Option createOption(cstring shortName, cstring longName, cstring description, boolean isBool) {
    Option opt;
    initOption(&opt, shortName, longName, description, isBool);
    return opt;
}

Option createBoolOption(cstring shortName, cstring longName, cstring description) {
    Option opt;
    initBoolOption(&opt, shortName, longName, description);
    return opt;
}

Option createStringOption(cstring shortName, cstring longName, cstring description, string defaultValue) {
    Option opt;
    initStringOption(&opt, shortName, longName, description, defaultValue);
    return opt;
}

Argument createArgument(cstring name, cstring description) {
    Argument arg;
    initArgument(&arg, name, description);
    return arg;
}

Command createCommand(cstring name, cstring description, Operation *op) {
    Command cmd;
    initCommand(&cmd, name, description, op);
    return cmd;
}

Program createProgram(cstring name, cstring executableName, cstring version) {
    Program program;
    initProgram(&program, name, executableName, version);
    return program;
}

int cli__printVersion(Program *program) {
    printf("%s%s %sversion %s%s%s\n", GREEN, program->name, NO_COLOUR, YELLOW, program->version, NO_COLOUR);
    return 1;
}

#endif // CLI_IMPLEMENTATION
