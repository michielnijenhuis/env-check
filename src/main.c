#define UTILS_IMPLEMENTATION
#include "lib/utils.h"
#define ARRAY_LIST_IMPLEMENTATION
#include "lib/array-list.h"
#define CLI_IMPLEMENTATION
#include "lib/cli.h"
#include "lib/types.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
TODOS:
* TODO: Implement Compare cmd opts
* TODO: Cleanup allocated memory
* TODO: Improve error messages
* TODO: Add coloring to usage
* FIX: Compare cmd results
* FIX: Print env var bug where color stm is printed if no value is printed
*/

#define PROGRAM_NAME    "envcheck"
#define PROGRAM_VERSION "0.0.0"

typedef struct EnvVar {
    cstr   name;
    string value;
} EnvVar;

// def
ArrayList *readEnvFile(cstr path);
void       sortEnvVariables(ArrayList *vars);
void       compareEnvVariables(ArrayList *exampleEnv, ArrayList *env);
boolean    isSameEnvVar(const EnvVar *a, const EnvVar *b);
boolean    hasValue(const EnvVar *var);
void       printEnvVar(const EnvVar *var, boolean withValue, boolean colored);
int        list(Command *self);
int        compare(Command *self);

// main
int main(int argc, string argv[]) {
    /**
     * Compare
     */
    Command compareCmd;
    initCommand(&compareCmd, "cmp", "Compares two env files files.", compare);
    // opts
    Option cmp__targetOpt, cmp__sourceOpt, cmp__ignoreOpt, cmp__keyOpt, cmp__missingOpt, cmp__undefinedOpt,
        cmp__divergentOpt, cmp__valuesOpt, cmp__exitOpt, cmp__coloredOpt;
    initStringOption(&cmp__targetOpt, "t", "target", "Path to the .env file to compare with (default: .env)");
    initStringOption(&cmp__sourceOpt, "s", "source", "Path to the .env file to compare to (default: .env.example)");
    initStringOption(&cmp__ignoreOpt, "i", "ignore", "Comma seperated list of variable name patterns to ignore");
    initStringOption(&cmp__keyOpt, "k", "key", "Comma seperated list of variable name patterns to focus on");
    initBoolOption(&cmp__missingOpt, "m", "missing", "Show missing and empty variables");
    initBoolOption(&cmp__undefinedOpt, "u", "undefined", "Show variables in the target that aren't in the source file");
    initBoolOption(&cmp__divergentOpt, "d", "divergent", "Show variables with diverging values");
    initBoolOption(&cmp__valuesOpt, "v", "values", "Include values in the results");
    initBoolOption(&cmp__exitOpt, "e", "exit", "Exit the script if the target file has missing values");
    initBoolOption(&cmp__coloredOpt, "c", "colored", "Print the results colored");
    Option *cmd__opts[]  = {&cmp__targetOpt,
                            &cmp__sourceOpt,
                            &cmp__ignoreOpt,
                            &cmp__keyOpt,
                            &cmp__missingOpt,
                            &cmp__undefinedOpt,
                            &cmp__divergentOpt,
                            &cmp__valuesOpt,
                            &cmp__exitOpt,
                            &cmp__coloredOpt};
    compareCmd.opts      = cmd__opts;
    compareCmd.optsCount = ARRAY_LEN(cmd__opts);

    /**
     * List
     */
    Command listCmd;
    initCommand(&listCmd, "list", "Lists all variables in the target env file, sorted alphabetically.", list);
    // opts
    Option list__pathOpt, list__valuesOpt, list__coloredOpt;
    initStringOption(&list__pathOpt, "p", "path", "Path to the .env file (default: .env.example)");
    initBoolOption(&list__valuesOpt, "v", "values", "Include values in the list");
    initBoolOption(&list__coloredOpt, "c", "colored", "Print the list colored");
    Option *list__opts[] = {&list__pathOpt, &list__valuesOpt, &list__coloredOpt};
    listCmd.opts         = list__opts;
    listCmd.optsCount    = ARRAY_LEN(list__opts);

    /**
     * Env Check
     */
    Command *commands[] = {&compareCmd, &listCmd};
    Program  program    = {
            .name          = PROGRAM_NAME,
            .version       = PROGRAM_VERSION,
            .commands      = commands,
            .commandsCount = 2,
    };

    return runApplication(&program, argc, argv);
}

void printEnvVar(const EnvVar *var, boolean withValue, boolean colored) {
    cstr   name      = var->name;
    string nameColor = colored ? GREEN : NO_COLOUR;

    if (!withValue) {
        printf(" %s%-30.30s%s\n", nameColor, name, NO_COLOUR);
        return;
    }

    char   value[1024];
    string varColor = NO_COLOUR;

    if (var->value == NULL || stringEquals(var->value, "null")) {
        sprintf(value, "NULL");
        varColor = RED;
    } else if (var->value != NULL) {
        sprintf(value, "%s", var->value);
    }

    printf(" %s%-30.30s%s%s%s\n", nameColor, name, varColor, value, NO_COLOUR);
}

int list(Command *self) {
    cstr    path       = getStringOpt(self->opts, self->optsCount, "path", "./.env.example");
    boolean showValues = getBoolOpt(self->opts, self->optsCount, "values", false);
    boolean colored    = getBoolOpt(self->opts, self->optsCount, "colored", false);

    printf("Path: %s\n", path);
    return 0;

    if (!fileExists(path)) {
        char msg[256];
        sprintf(msg, "File '%s' does not exist.", path);
        panic(msg);
    }

    ArrayList *variables = readEnvFile(path);

    if (variables == NULL) {
        panic("Couldn't read environment variables.");
    }

    arrayForEach(variables, EnvVar * var) {
        printEnvVar(var, showValues, colored);
    }

    arrayFree(variables);

    return EXIT_SUCCESS;
}

int compare(Command *self) {
    Option    *targetOpt           = self->opts[0];
    Option    *sourceOpt           = self->opts[1];

    string     examplePath         = sourceOpt->stringValue ? sourceOpt->stringValue : "./.env.example";
    string     envPath             = targetOpt->stringValue ? targetOpt->stringValue : "./.env";

    ArrayList *envExampleVariables = readEnvFile(examplePath);
    ArrayList *envVariables        = readEnvFile(envPath);

    if (envExampleVariables == NULL) {
        panic("Couldn't read .env.example variables.");
    }

    if (envVariables == NULL) {
        panic("Couldn't read .env variables.");
    }

    compareEnvVariables(envExampleVariables, envVariables);
    arrayFree(envExampleVariables);
    arrayFree(envVariables);

    return EXIT_SUCCESS;
}

ArrayList *readEnvFile(cstr path) {
    ArrayList *variables = newArray(sizeof(struct EnvVar), 50, free);

    if (variables == NULL) {
        panic("Failed to create Variables array.");
    }

    File  *file;
    string buffer = NULL;
    usize  len    = 0;
    ssize  read;

    file = fopen(path, "r");
    if (file == NULL) {
        char msg[256];
        sprintf(msg, "Failed to open file '%s'.\n", path);
        panic(msg);
    }

    while ((read = getline(&buffer, &len, file)) != -1) {
        if (stringStartsWith(buffer, "#") || stringIsEmpty(buffer)) {
            continue;
        }

        string *fragments       = stringSplit(buffer, "=");
        usize   fragmentsLength = ARRAY_LEN(fragments);

        if (fragments != NULL) {
            string name  = NULL;
            string value = NULL;

            if (fragments[0] != NULL) {
                name = stringCopy(fragments[0]);
                name = stringTrim(name);
            }

            if (!stringIsEmpty(name)) {
                if (fragmentsLength >= 1) {
                    usize            i = 1;
                    struct ArrayList values;
                    arrayInit(&values, sizeof(string), 5, free);
                    string current = fragments[i];

                    while (current != NULL) {
                        arrayPush(&values, current);
                        current = fragments[++i];
                    }

                    if (arrayLength(&values) > 0) {
                        value = arrayJoin(&values, "=");
                        if (stringIsEmpty(value)) {
                            free(value);
                            value = NULL;
                        }
                    }
                }

                EnvVar *envVar = malloc(sizeof(*envVar));
                if (envVar != NULL) {
                    envVar->name  = name;
                    envVar->value = value;
                    arrayPush(variables, envVar);
                }
            }
        }
    }

    fclose(file);
    sortEnvVariables(variables);

    return variables;
}

void sortEnvVariables(ArrayList *variables) {
    for (usize i = 0, n = arrayLength(variables); i < n; ++i) {
        for (usize j = 0; j < n - 1 - i; ++j) {
            EnvVar *a = arrayGet(variables, j);
            EnvVar *b = arrayGet(variables, j + 1);

            if (strcmp(a->name, b->name) > 0) {
                arraySet(variables, j, b);
                arraySet(variables, j + 1, a);
            }
        }
    }
}

boolean isSameEnvVar(const EnvVar *a, const EnvVar *b) {
    if (a == b) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    return stringEquals(a->name, b->name);
}

boolean hasValue(const EnvVar *var) {
    if (var == NULL) {
        return false;
    }

    return var->value != NULL && !stringIsEmpty(var->value);
}

EnvVar *findVar(ArrayList *variables, cstr name) {
    usize start = 0;
    usize end   = arrayLength(variables) - 1;

    while (start <= end) {
        usize   middle = start + (end - start) / 2;
        EnvVar *var    = arrayGet(variables, middle);

        if (var == NULL) {
            break;
        }

        if (stringEquals(var->name, name)) {
            return var;
        }

        if (strcmp(var->name, name) > 0) {
            start = middle + 1;
        } else {
            end = middle - 1;
        }
    }

    return NULL;
}

boolean hasVar(ArrayList *variables, cstr name) {
    EnvVar *var = findVar(variables, name);

    return var != NULL;
}

void printCompareResults(ArrayList *variables, cstr title) {
    printf("%s\n", title);
    printf("---------------\n");
    arrayForEach(variables, EnvVar * var) {
        printEnvVar(var, false, true);
        // printf("  * %s\n", Var->Name);
    }
    printf("\n");
}

void compareEnvVariables(ArrayList *exampleEnv, ArrayList *env) {
    usize      aIndex    = 0;
    usize      bIndex    = 0;
    usize      aLen      = arrayLength(exampleEnv);
    usize      bLen      = arrayLength(env);
    ArrayList *missing   = newArray(sizeof(struct EnvVar), 10, free);
    ArrayList *divergent = newArray(sizeof(struct EnvVar), 10, free);
    ArrayList *empty     = newArray(sizeof(struct EnvVar), 10, free);
    ArrayList *undefined = newArray(sizeof(struct EnvVar), 10, free);

    while (aIndex < aLen || bIndex < bLen) {
        EnvVar *a = arrayGet(exampleEnv, aIndex);
        EnvVar *b = arrayGet(env, bIndex);

        if (a == NULL) {
            if (b == NULL) {
                break;
            }

            arrayPush(undefined, b);
            bIndex++;
        } else if (b == NULL) {
            arrayPush(missing, a);
            aIndex++;
        } else if (isSameEnvVar(a, b)) {
            if (!hasValue(b)) {
                arrayPush(empty, b);
            } else if (hasValue(a) && !stringEquals(a->value, b->value)) {
                arrayPush(divergent, b);
            }
            aIndex++;
            bIndex++;
        } else if (!hasVar(exampleEnv, b->name)) {
            arrayPush(undefined, b);
            bIndex++;
        } else if (!hasVar(exampleEnv, a->name)) {
            arrayPush(missing, a);
            aIndex++;
        }
    }

    if (arrayLength(missing)) {
        printCompareResults(missing, "Missing");
    }
    arrayFree(missing);

    if (arrayLength(empty)) {
        printCompareResults(empty, "Empty");
    }
    arrayFree(empty);

    if (arrayLength(undefined)) {
        printCompareResults(undefined, "Undefined");
    }
    arrayFree(undefined);

    if (arrayLength(divergent)) {
        printCompareResults(divergent, "Divergent");
    }
    arrayFree(divergent);
}
