#include "lib/array-list.h"
#include "lib/hash-table.h"
#include "lib/str.h"
#include "lib/types.h"
#include "lib/utils.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
TODOS:
* TODO: Implement Compare cmd opts
* TODO: Move run program logic into seperate lib
* TODO: Cleanup allocated memory
* TODO: Improve error messages
* TODO: Add coloring to usage
* FIX: Compare cmd results
* FIX: Print env var bug where color stm is printed if no value is printed
*/

#define PROGRAM_NAME ".env"
#define PROGRAM_VERSION "0.1.0"

#define ASCII_ART \
    "\
       _______  __    _  __   __ \n\
      |       ||  |  | ||  | |  |\n\
      |    ___||   |_| ||  |_|  |\n\
      |   |___ |       ||       |\n\
 ___  |    ___||  _    ||       |\n\
|   | |   |___ | | |   | |     | \n\
|___| |_______||_|  |__|  |___|  "

typedef struct Option
{
    string shortName;
    string longName;
    string description;
    string stringValue;
    boolean isBool;
    boolean boolValue;
} Option;

typedef struct Argument
{
    string name;
    string description;
    string value;
} Argument;

typedef int (*Operation)(Option *opts, Argument *args);

typedef struct Command
{
    string name;
    string description;
    Operation operation;
    Option *opts;
    Argument *args;
    usize optsCount;
    usize argsCount;
} Command;

typedef struct EnvVar
{
    cstr name;
    string value;
} EnvVar;

int usage(string command, boolean versionOnly);
void printOption(const Option *opt);
Option *createOption(string shortName, string longName, string description, boolean boolean);
boolean wantsHelp(string arg);
boolean getBoolOpt(const Option *opts, string longNme, u32 optsCount, boolean fallback);
string getStringOpt(const Option *opts, string longName, u32 optsCount, string Fallback);
string getArg(const Argument *args, string name, u32 argsCount);
boolean fileExists(string fileName);
ArrayList *readEnvFile(cstr path);
void sortEnvVariables(ArrayList *vars);
void compareEnvVariables(ArrayList *exampleEnv, ArrayList *env);
boolean isSameEnvVar(const EnvVar *a, const EnvVar *b);
boolean hasValue(const EnvVar *var);
void printEnvVar(const EnvVar *var, boolean withValue, boolean colored);
int list(Option *opts, Argument *args);
int compare(Option *opts, Argument *argss);

static Option listCommandOptions[3] = {
    {"p", "path", "Path to the .env file (default: .env.example)", false, false, NULL},
    {"v", "values", "Include values in the list", true, false, NULL},
    {"c", "colored", "Print the list colored", true, false, NULL},
};

static Option compareCommandOptions[10] = {
    {"t", "target", "Path to the .env file to compare with (default: .env)", false, false, NULL},
    {"s", "source", "Path to the .env file to compare to (default: .env.example)", false, false, NULL},
    {"i", "ignore", "Comma seperated list of variable name patterns to ignore", false, false, NULL},
    {"k", "key", "Comma seperated list of variable name patterns to focus on", false, false, NULL},
    {"m", "missing", "Show missing and empty variables", true, false, NULL},
    {"u", "undefined", "Show variables in the target that aren't in the source file", true, false, NULL},
    {"d", "divergent", "Show variables with diverging values", true, false, NULL},
    {"v", "values", "Include values in the results", true, false, NULL},
    {"e", "exit", "Exit the script if the target file has missing values", true, false, NULL},
    {"c", "colored", "Print the results colored", true, false, NULL},
};

static const Command commands[2] = {
    {
        .name = "cmp",
        .description = "Compares two env files files.",
        .operation = compare,
        .opts = compareCommandOptions,
        .optsCount = ARRAY_LEN(compareCommandOptions),
        .args = NULL,
        .argsCount = 0,
    },
    {
        .name = "list",
        .description = "Lists all variables in the target env file, sorted alphabetically.",
        .operation = list,
        .opts = listCommandOptions,
        .optsCount = ARRAY_LEN(listCommandOptions),
        .args = NULL,
        .argsCount = 0,
    }};

int main(int argc, string argv[])
{
    argc++;
    string executableCommand = *argv++;

    // .env (-h|--help)
    if (argc == 1 || wantsHelp(executableCommand))
    {
        return usage(NULL, false);
    }

    if (stringEquals(executableCommand, "-v") || stringEquals(executableCommand, "--version"))
    {
        return usage(NULL, true);
    }

    // .env <command> -h|--help
    if (wantsHelp(*argv))
    {
        return usage(executableCommand, false);
    }

    ssize index = -1;
    usize i = 0;
    for (usize i = 0; i < ARRAY_LEN(commands); ++i)
    {
        if (stringEquals(commands[i].name, executableCommand))
        {
            index = i;
            break;
        }
        ++i;
    }

    if (index < 0)
    {
        panic("Unknown command.");
    }

    Command cmd = commands[index];
    boolean parsingArguments = false;
    usize currentArgIndex = 0;

    while (true)
    {
        string currentArg = *argv;

        if (currentArg == NULL)
        {
            if (currentArgIndex < cmd.argsCount)
            {
                panic("Missing args.");
            }

            break;
        }

        boolean isLongOption = stringStartsWith(currentArg, "--");
        boolean isShortOption = !isLongOption && stringStartsWith(currentArg, "-");

        if (isLongOption || isShortOption)
        {
            if (parsingArguments)
            {
                panic("Invalid args.");
            }

            Option *currentOption = NULL;
            usize argLen = stringLength(currentArg);
            string nameLong = NULL;
            string nameShort = NULL;

            if (isLongOption)
            {
                nameLong = stringSlice(currentArg, 2, argLen);
            }
            else if (isShortOption)
            {
                nameShort = stringSlice(currentArg, 1, argLen);
            }

            for (usize i = 0; i < cmd.optsCount; ++i)
            {
                Option *opt = &cmd.opts[i];
                if (isLongOption && stringEquals(opt->longName, nameLong))
                {
                    currentOption = opt;
                    break;
                }
                else if (isShortOption && stringEquals(opt->shortName, nameShort))
                {
                    currentOption = opt;
                    break;
                }
            }

            if (currentOption == NULL)
            {
                fprintf(stderr, "Unknown option: %s%s\n", nameLong ? nameLong : "", nameShort ? nameShort : "");
                exit(EXIT_FAILURE);
            }

            if (currentOption->isBool)
            {
                currentOption->boolValue = true;
                argv++;
            }
            else
            {
                argv++;
                string nextArg = *argv;

                if (nextArg == NULL || stringStartsWith(nextArg, "--") || stringStartsWith(nextArg, "-"))
                {
                    panic("Invalid args.");
                }

                currentOption->stringValue = stringCopy(nextArg);
                argv++;
            }

            free(nameLong);
            free(nameShort);
        }
        else
        {
            parsingArguments = true;

            if (cmd.args == NULL || currentArgIndex >= cmd.argsCount)
            {
                panic("Too many args.");
            }

            cmd.args[currentArgIndex++].value = stringCopy(currentArg);
            argv++;
        }
    }

    int code = cmd.operation(cmd.opts, cmd.args);

    for (usize i = 0; i < cmd.optsCount; ++i)
    {
        Option opt = cmd.opts[i];
        if (!opt.isBool)
        {
            free(opt.stringValue);
        }
    }

    for (usize i = 0; i < cmd.argsCount; ++i)
    {
        Argument arg = cmd.args[i];
        free(arg.value);
    }

    // Cleanup: String values on Options and Arguments

    return code;
}

boolean wantsHelp(string Arg)
{
    return stringEquals(Arg, "-h") || stringEquals(Arg, "--long");
}

Option *createOption(string shortName, string longName, string description, boolean boolean)
{
    struct Option *opt = malloc(sizeof(*opt));

    if (opt == NULL)
    {
        return NULL;
    }

    opt->shortName = shortName;
    opt->longName = longName;
    opt->isBool = boolean;
    opt->boolValue = false;
    opt->stringValue = NULL;
    opt->description = description;

    return opt;
}

void printOption(const Option *opt)
{
    boolean isBool = opt->isBool;
    char name[100];

    if (isBool)
    {
        sprintf(name, "-%s, --%s", opt->shortName, opt->longName);
        printf("  %s%-25.25s%s %s\n", GREEN, name, NO_COLOUR, opt->description);
    }
    else
    {
        string nameUpper = stringToUppercase(opt->longName);
        sprintf(name, "-%s, --%s=%s", opt->shortName, opt->longName, nameUpper);
        printf("  %s%-25.25s%s %s\n", GREEN, name, NO_COLOUR, opt->description);
        free(nameUpper);
    }
}

int usage(string commandName, boolean versionOnly)
{
    if (commandName == NULL)
    {
        if (!versionOnly)
        {
            printf("%s\n\n", ASCII_ART);
        }

        printf("%s%s %sversion %s%s%s\n%s",
               GREEN,
               PROGRAM_NAME,
               NO_COLOUR,
               YELLOW,
               PROGRAM_VERSION,
               NO_COLOUR,
               versionOnly ? "" : "\n");

        if (versionOnly)
        {
            return EXIT_FAILURE;
        }

        printf("%sUsage:%s\n", YELLOW, NO_COLOUR);
        printf("  %s <command> [opts] [args...]\n", PROGRAM_NAME);

        printf("\n%sOptions:%s\n", YELLOW, NO_COLOUR);
        printf("  %s%s, %s%s\t\tDisplay help for the given command.\n", GREEN, "-h", "--long", NO_COLOUR);
        printf("  %s-v, --version%s\t\tShow the current version of the program.\n", GREEN, NO_COLOUR);
        printf("\n");

        printf("%sAvailable commands:%s\n", YELLOW, NO_COLOUR);

        for (usize i = 0; i < ARRAY_LEN(commands); ++i)
        {
            Command cmd = commands[i];
            printf("  %s%-5.5s%s\t\t%s\n", GREEN, cmd.name, NO_COLOUR, cmd.description);
        }

        return EXIT_FAILURE;
    }

    for (usize i = 0; i < ARRAY_LEN(commands); ++i)
    {
        Command cmd = commands[i];
        if (stringEquals(cmd.name, commandName))
        {
            printf("%sDescription:%s\n", YELLOW, NO_COLOUR);
            printf("  %s\n\n", cmd.description);

            printf("%sUsage:%s\n", YELLOW, NO_COLOUR);
            boolean hasOpts = cmd.opts != NULL;
            boolean hasArgs = cmd.args != NULL;
            string opts = hasOpts ? " [opts]" : "";
            string args = hasArgs ? " [args...]" : "";
            printf("  %s %s%s%s\n", PROGRAM_NAME, cmd.name, opts, args);

            if (hasOpts)
            {
                printf("\n%sOptions:%s\n", YELLOW, NO_COLOUR);

                for (usize i = 0; i < cmd.optsCount; ++i)
                {
                    Option cur = cmd.opts[i];
                    printOption(&cur);
                }

                printf("\n");
            }

            if (hasArgs)
            {
                printf("%sArguments:%s\n", YELLOW, NO_COLOUR);

                for (usize i = 0; i < cmd.argsCount; ++i)
                {
                    Argument cur = cmd.args[i];
                    printf("  %s%-25.25s %s%s\n", GREEN, cur.name, cur.description, NO_COLOUR);
                }
            }

            return EXIT_FAILURE;
        }
    }

    panic("Unknown command.");
    return EXIT_FAILURE;
}

boolean getBoolOpt(const Option *opts, string longName, u32 optsCount, boolean fallback)
{
    for (usize i = 0; i < optsCount; ++i)
    {
        Option cur = opts[i];

        if (stringEquals(cur.longName, longName) && cur.isBool)
        {
            return cur.boolValue;
        }
    }

    return fallback;
}

string getStringOpt(const Option *opts, string longName, u32 optsCount, string fallback)
{
    for (usize i = 0; i < optsCount; ++i)
    {
        Option cur = opts[i];

        if (stringEquals(cur.longName, longName) && !cur.isBool && cur.stringValue != NULL)
        {
            return cur.stringValue;
        }
    }

    return fallback;
}

string getArg(const Argument *args, string name, u32 argsCount)
{
    for (u32 i = 0; i < argsCount; ++i)
    {
        Argument cur = args[i];

        if (stringEquals(cur.name, name))
        {
            return cur.value;
        }
    }

    return NULL;
}

void printEnvVar(const EnvVar *var, boolean withValue, boolean colored)
{
    string name = var->name;
    string nameColor = colored ? GREEN : NO_COLOUR;

    if (!withValue)
    {
        printf(" %s%-30.30s%s\n", nameColor, name, NO_COLOUR);
        return;
    }

    char value[1024];
    string varColor = NO_COLOUR;

    if (var->value == NULL || stringEquals(var->value, "null"))
    {
        sprintf(value, "NULL");
        varColor = RED;
    }
    else if (var->value != NULL)
    {
        sprintf(value, "%s", var->value);
    }

    printf(" %s%-30.30s%s%s%s\n", nameColor, name, varColor, value, NO_COLOUR);
}

int list(Option *opts, Argument *args __attribute__((unused)))
{
    u32 optsCount = ARRAY_LEN(listCommandOptions);
    cstr path = getStringOpt(opts, "path", optsCount, "./.env.example");
    boolean showValues = getBoolOpt(opts, "values", optsCount, false);
    boolean colored = getBoolOpt(opts, "colored", optsCount, false);

    if (!fileExists(path))
    {
        char msg[256];
        sprintf(msg, "File '%s' does not exist.", path);
        panic(msg);
    }

    ArrayList *variables = readEnvFile(path);

    if (variables == NULL)
    {
        panic("Couldn't read environment variables.");
    }

    arrayForEach(variables, EnvVar * var)
    {
        printEnvVar(var, showValues, colored);
    }

    arrayFree(variables);

    return EXIT_SUCCESS;
}

int compare(Option *opts, Argument *args __attribute__((unused)))
{
    Option targetOpt = opts[0];
    Option sourceOpt = opts[1];

    string examplePath = sourceOpt.stringValue ? sourceOpt.stringValue : "./.env.example";
    string envPath = targetOpt.stringValue ? targetOpt.stringValue : "./.env";

    ArrayList *envExampleVariables = readEnvFile(examplePath);
    ArrayList *envVariables = readEnvFile(envPath);

    if (envExampleVariables == NULL)
    {
        panic("Couldn't read .env.example variables.");
    }

    if (envVariables == NULL)
    {
        panic("Couldn't read .env variables.");
    }

    printf("Comparing %s to %s\n\n", envPath, examplePath);
    compareEnvVariables(envExampleVariables, envVariables);
    arrayFree(envExampleVariables);
    arrayFree(envVariables);

    return EXIT_SUCCESS;
}

ArrayList *readEnvFile(cstr path)
{
    ArrayList *variables = newArray(sizeof(struct EnvVar), 50, free);

    if (variables == NULL)
    {
        panic("Failed to create Variables array.");
    }

    File *file;
    string buffer = NULL;
    usize len = 0;
    ssize read;

    file = fopen(path, "r");
    if (file == NULL)
    {
        char msg[256];
        sprintf(msg, "Failed to open file '%s'.\n", path);
        panic(msg);
    }

    while ((read = getline(&buffer, &len, file)) != -1)
    {
        if (stringStartsWith(buffer, "#") || stringIsEmpty(buffer))
        {
            continue;
        }

        string *fragments = stringSplit(buffer, "=");
        usize fragmentsLength = ARRAY_LEN(fragments);
        ArrayList *values = NULL;
        string name = NULL;
        string value = NULL;
        EnvVar *envVar = NULL;

        if (fragments != NULL)
        {
            if (fragments[0] != NULL)
            {
                name = stringCopy(fragments[0]);
                name = stringTrim(name);
            }

            if (!stringIsEmpty(name))
            {
                if (fragmentsLength >= 1)
                {
                    usize i = 1;
                    values = newArray(sizeof(string), 5, free);
                    string current = fragments[i];

                    while (current != NULL)
                    {
                        arrayPush(values, current);
                        current = fragments[++i];
                    }

                    if (arrayLength(values) > 0)
                    {
                        value = arrayJoin(values, "=");
                        value = stringTrim(value);
                        if (stringIsEmpty(value))
                        {
                            free(value);
                            value = NULL;
                        }
                    }
                }

                envVar = malloc(sizeof(*envVar));
                if (envVar != NULL)
                {
                    envVar->name = name;
                    envVar->value = value;
                    arrayPush(variables, envVar);
                }
            }
        }

        for (usize i = 0; i < ARRAY_LEN(fragments); ++i)
        {
            free(fragments[i]);
        }
        arrayFree(values);
    }

    fclose(file);
    free(buffer);

    sortEnvVariables(variables);

    return variables;
}

void sortEnvVariables(ArrayList *variables)
{
    for (usize i = 0, n = arrayLength(variables); i < n; ++i)
    {
        for (usize j = 0; j < n - 1 - i; ++j)
        {
            EnvVar *a = arrayGet(variables, j);
            EnvVar *b = arrayGet(variables, j + 1);

            if (strcmp(a->name, b->name) > 0)
            {
                arraySet(variables, j, b);
                arraySet(variables, j + 1, a);
            }
        }
    }
}

boolean isSameEnvVar(const EnvVar *a, const EnvVar *b)
{
    if (a == b)
    {
        return true;
    }

    if (a == NULL || b == NULL)
    {
        return false;
    }

    return stringEquals(a->name, b->name);
}

boolean hasValue(const EnvVar *var)
{
    if (var == NULL)
    {
        return false;
    }

    return var->value != NULL && !stringIsEmpty(var->value);
}

EnvVar *findVar(ArrayList *variables, cstr name)
{
    usize start = 0;
    usize end = arrayLength(variables) - 1;

    while (start <= end)
    {
        usize middle = start + (end - start) / 2;
        EnvVar *var = arrayGet(variables, middle);

        if (var == NULL)
        {
            break;
        }

        if (stringEquals(var->name, name))
        {
            return var;
        }

        if (strcmp(var->name, name) > 0)
        {
            start = middle + 1;
        }
        else
        {
            end = middle - 1;
        }
    }

    return NULL;
}

boolean hasVar(ArrayList *variables, cstr name)
{
    EnvVar *var = findVar(variables, name);

    return var != NULL;
}

void printCompareResults(ArrayList *variables, cstr title)
{
    printf("%s\n", title);
    printf("---------------\n");
    arrayForEach(variables, EnvVar * var)
    {
        printEnvVar(var, false, true);
        // printf("  * %s\n", Var->Name);
    }
    printf("\n");
}

void compareEnvVariables(ArrayList *exampleEnv, ArrayList *env)
{
    usize aIndex = 0;
    usize bIndex = 0;
    usize aLen = arrayLength(exampleEnv);
    usize bLen = arrayLength(env);
    ArrayList *missing = newArray(sizeof(struct EnvVar), 10, free);
    ArrayList *divergent = newArray(sizeof(struct EnvVar), 10, free);
    ArrayList *empty = newArray(sizeof(struct EnvVar), 10, free);
    ArrayList *undefined = newArray(sizeof(struct EnvVar), 10, free);

    while (aIndex < aLen || bIndex < bLen)
    {
        EnvVar *a = arrayGet(exampleEnv, aIndex);
        EnvVar *b = arrayGet(env, bIndex);

        if (a == NULL)
        {
            if (b == NULL)
            {
                break;
            }

            arrayPush(undefined, b);
            bIndex++;
        }
        else if (b == NULL)
        {
            arrayPush(missing, a);
            aIndex++;
        }
        else if (isSameEnvVar(a, b))
        {
            if (!hasValue(b))
            {
                arrayPush(empty, b);
            }
            else if (hasValue(a) && !stringEquals(a->value, b->value))
            {
                arrayPush(divergent, b);
            }
            aIndex++;
            bIndex++;
        }
        else if (!hasVar(exampleEnv, b->name))
        {
            arrayPush(undefined, b);
            bIndex++;
        }
        else if (!hasVar(exampleEnv, a->name))
        {
            arrayPush(missing, a);
            aIndex++;
        }
    }

    if (arrayLength(missing))
    {
        printCompareResults(missing, "Missing");
    }
    arrayFree(missing);

    if (arrayLength(empty))
    {
        printCompareResults(empty, "Empty");
    }
    arrayFree(empty);

    if (arrayLength(undefined))
    {
        printCompareResults(undefined, "Undefined");
    }
    arrayFree(undefined);

    if (arrayLength(divergent))
    {
        printCompareResults(divergent, "Divergent");
    }
    arrayFree(divergent);
}
