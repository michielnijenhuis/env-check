#define UTILS_IMPLEMENTATION
#include "lib/utils.h"
#define ARRAY_LIST_IMPLEMENTATION
#include "lib/array-list.h"
#define HASH_TABLE_IMPLEMENTATION
#include "lib/hash-table.h"
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

typedef struct EnvVar {
    string name;
    string value;
} EnvVar;

// def
HashTable *readEnvFile(cstr path);
void       sortEnvVariables(ArrayList *vars);
void       compareEnvVariables(ArrayList *exampleEnv, ArrayList *env);
boolean    isSameEnvVar(const EnvVar *a, const EnvVar *b);
boolean    hasValue(const EnvVar *var);
void       printEnvVar(const EnvVar *var, boolean withValue, boolean colored);
void       freeEnvVar(void *var);
int        list(Command *self);
int        compare(Command *self);

// main
int main(int argc, string argv[]) {
    /**
     * Compare
     */
    Command compareCmd     = createCommand("cmp", "Compares two env files files.", compare);
    Option  cmp__targetOpt = createStringOption("t", "target", "Path to the .env file to compare with", "./.env");
    Option  cmp__sourceOpt = createStringOption("s", "source", "Path to the .env file to compare to", "./.env.example");
    Option  cmp__ignoreOpt =
        createStringOption("i", "ignore", "Comma seperated list of variable name patterns to ignore", NULL);
    Option cmp__keyOpt =
        createStringOption("k", "key", "Comma seperated list of variable name patterns to focus on", NULL);
    Option cmp__missingOpt = createBoolOption("m", "missing", "Show missing and empty variables");
    Option cmp__undefinedOpt =
        createBoolOption("u", "undefined", "Show variables in the target that aren't in the source file");
    Option  cmp__divergentOpt = createBoolOption("d", "divergent", "Show variables with diverging values");
    Option  cmp__valuesOpt    = createBoolOption("v", "values", "Include values in the results");
    Option  cmp__exitOpt      = createBoolOption("e", "exit", "Exit the script if the target file has missing values");
    Option  cmp__coloredOpt   = createBoolOption("c", "colored", "Print the results colored");
    Option *cmd__opts[]       = {&cmp__targetOpt,
                                 &cmp__sourceOpt,
                                 &cmp__ignoreOpt,
                                 &cmp__keyOpt,
                                 &cmp__missingOpt,
                                 &cmp__undefinedOpt,
                                 &cmp__divergentOpt,
                                 &cmp__valuesOpt,
                                 &cmp__exitOpt,
                                 &cmp__coloredOpt};
    compareCmd.opts           = cmd__opts;
    compareCmd.optsCount      = ARRAY_LEN(cmd__opts);

    /**
     * List
     */
    Command listCmd = createCommand("list", "Lists all variables in the target env file, sorted alphabetically.", list);
    // opts
    Option  list__pathOpt    = createStringOption("p", "path", "Path to the .env file", "./.env.example");
    Option  list__valuesOpt  = createBoolOption("v", "values", "Include values in the list");
    Option  list__coloredOpt = createBoolOption("c", "colored", "Print the list colored");
    Option *list__opts[]     = {&list__pathOpt, &list__valuesOpt, &list__coloredOpt};
    listCmd.opts             = list__opts;
    listCmd.optsCount        = ARRAY_LEN(list__opts);

    /**
     * Env Check
     */
    Command *commands[] = {&compareCmd, &listCmd};
    Program  program    = {
            .executableName = "envc",
            .name           = "Env Check",
            .version        = "0.0.0",
            .commands       = commands,
            .commandsCount  = 2,
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

    if (var->value == NULL || cstringEquals(var->value, "null")) {
        sprintf(value, "NULL");
        varColor = RED;
    } else if (var->value != NULL) {
        sprintf(value, "%s", var->value);
    }

    printf(" %s%-30.30s%s%s%s\n", nameColor, name, varColor, value, NO_COLOUR);
}

void freeEnvVar(void *var) {
    if (var == NULL) {
        return;
    }

    free(((EnvVar *) var)->value);
    free(((EnvVar *) var)->name);
    free(var);
}

int list(Command *self) {
    cstr    path       = getStringOpt(self, "path");
    boolean showValues = getBoolOpt(self, "values");
    boolean colored    = getBoolOpt(self, "colored");

    if (!fileExists(path)) {
        char errorMsg[FILENAME_MAX];
        sprintf(errorMsg, "File '%s' does not exist.", path);
        printError(errorMsg);
        return EXIT_FAILURE;
    }

    HashTable *variables = readEnvFile(path);

    if (variables == NULL) {
        printError("Couldn't read environment variables.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int compare(Command *self) {
    Option    *targetOpt           = self->opts[0];
    Option    *sourceOpt           = self->opts[1];

    string     examplePath         = sourceOpt->stringValue ? sourceOpt->stringValue : "./.env.example";
    string     envPath             = targetOpt->stringValue ? targetOpt->stringValue : "./.env";

    HashTable *envExampleVariables = readEnvFile(examplePath);
    HashTable *envVariables        = readEnvFile(envPath);

    if (envExampleVariables == NULL) {
        panic("Couldn't read .env.example variables.");
    }

    if (envVariables == NULL) {
        panic("Couldn't read .env variables.");
    }

    return EXIT_SUCCESS;
}

HashTable *readEnvFile(cstr path) {
    HashTable *variables = hashTableCreateMalloc(50, NULL, freeEnvVar);

    if (variables == NULL) {
        panic("Failed to create Variables hash table.");
    }

    File  *file;
    string buffer = NULL;
    usize  len    = 0;
    ssize  read;

    file = fopen(path, "r");
    if (file == NULL) {
        char errorMsg[FILENAME_MAX];
        sprintf(errorMsg, "Failed to open file '%s'.\n", path);
        panic(errorMsg);
    }

    while ((read = getline(&buffer, &len, file)) != -1) {
        if (cstringStartsWith(buffer, "#") || cstringIsEmpty(buffer)) {
            continue;
        }

        printf("Buffer: %s", buffer);
        string name = strtok(buffer, "=");

        if (name == NULL || cstringIsEmpty(name)) {
            continue;
        }

        printf("Name: %s\n", name);
        printf("Value: %s\n", buffer);

        continue;
    }

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

    return cstringEquals(a->name, b->name);
}

boolean hasValue(const EnvVar *var) {
    if (var == NULL) {
        return false;
    }

    return var->value != NULL && !cstringIsEmpty(var->value);
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

        if (cstringEquals(var->name, name)) {
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
    ArrayList *missing   = arrayMalloc(sizeof(struct EnvVar), 10, free);
    ArrayList *divergent = arrayMalloc(sizeof(struct EnvVar), 10, free);
    ArrayList *empty     = arrayMalloc(sizeof(struct EnvVar), 10, free);
    ArrayList *undefined = arrayMalloc(sizeof(struct EnvVar), 10, free);

    while (aIndex < aLen || bIndex < bLen) {
        EnvVar *a = arrayGet(exampleEnv, aIndex);
        EnvVar *b = arrayGet(env, bIndex);

        if (a == NULL) {
            if (b == NULL) {
                break;
            }

            arrayPushBack(undefined, b);
            bIndex++;
        } else if (b == NULL) {
            arrayPushBack(missing, a);
            aIndex++;
        } else if (isSameEnvVar(a, b)) {
            if (!hasValue(b)) {
                arrayPushBack(empty, b);
            } else if (hasValue(a) && !cstringEquals(a->value, b->value)) {
                arrayPushBack(divergent, b);
            }
            aIndex++;
            bIndex++;
        } else if (!hasVar(exampleEnv, b->name)) {
            arrayPushBack(undefined, b);
            bIndex++;
        } else if (!hasVar(exampleEnv, a->name)) {
            arrayPushBack(missing, a);
            aIndex++;
        }
    }

    if (arrayLength(missing)) {
        printCompareResults(missing, "Missing");
    }
    arrayFree(missing);
    free(missing);

    if (arrayLength(empty)) {
        printCompareResults(empty, "Empty");
    }
    arrayFree(empty);
    free(empty);

    if (arrayLength(undefined)) {
        printCompareResults(undefined, "Undefined");
    }
    arrayFree(undefined);
    free(undefined);

    if (arrayLength(divergent)) {
        printCompareResults(divergent, "Divergent");
    }
    arrayFree(divergent);
    free(divergent);
}
