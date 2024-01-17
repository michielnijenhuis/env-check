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

#define MALLOC_ERR printError("Memory allocation failed.")

/*
TODOS:
* TODO: Implement Compare cmd opts
*/

typedef struct EnvVar {
    string name;
    string value;
} EnvVar;

typedef void(EnvVarHandler)(void *, EnvVar *);

void    freeEnvVar(void *var);
EnvVar *createEnvVarFromLine(cstring line);
void    pushEnvVarIntoArray(void *array, EnvVar *var);
void    insertEnvVarIntoHashTable(void *hashTable, EnvVar *var);
int     readEnvFile(void *source, cstring path, EnvVarHandler *handler, cstring ignore, cstring focus);
int     readEnvFileIntoArray(ArrayList *variables, cstring path, cstring ignore, cstring focus);
int     readEnvFileIntoHashTable(HashTable *table, cstring path, cstring ignore, cstring focus);
usize   findMaxEnvFileKeyWidth(ArrayList *variables);
void    trimString(string input);
void    printEnvVar(EnvVar *var, int maxLength, boolean colored, boolean showValue);
void    sortEnvVarsArray(ArrayList *array);
boolean matchPattern(cstring str, cstring pattern);
int     list(Command *self);
int     compare(Command *self);

/**
 * Main
 */
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
    Option  cmp__printOpt     = createBoolOption("p", "print", "Print the keys and values side by side.");
    Option *cmd__opts[]       = {&cmp__targetOpt,
                                 &cmp__sourceOpt,
                                 &cmp__ignoreOpt,
                                 &cmp__keyOpt,
                                 &cmp__missingOpt,
                                 &cmp__undefinedOpt,
                                 &cmp__divergentOpt,
                                 &cmp__valuesOpt,
                                 &cmp__exitOpt,
                                 &cmp__coloredOpt,
                                 &cmp__printOpt};
    compareCmd.opts           = cmd__opts;
    compareCmd.optsCount      = ARRAY_LEN(cmd__opts);

    /**
     * List
     */
    Command listCmd = createCommand("list", "Lists all variables in the target env file, sorted alphabetically.", list);
    // opts
    Option list__pathOpt = createStringOption("p", "path", "Path to the .env file", "./.env.example");
    Option list__ignoreOpt =
        createStringOption("i", "ignore", "Comma separated list of variable name patterns to ignore", NULL);
    Option list__keyOpt =
        createStringOption("k", "key", "Comma separated list of variable name patterns to focus on", NULL);
    Option  list__valuesOpt  = createBoolOption("v", "values", "Include values in the list");
    Option  list__coloredOpt = createBoolOption("c", "colored", "Print the list colored");
    Option *list__opts[]     = {&list__pathOpt, &list__ignoreOpt, &list__keyOpt, &list__valuesOpt, &list__coloredOpt};
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

/**
 * Helpers
 */
void freeEnvVar(void *var) {
    free(((EnvVar *) var)->name);
    free(((EnvVar *) var)->value);
    free(var);
}

void trimString(string input) {
    if (input == NULL) {
        return;
    }

    // Trim line breaks at the end
    usize length = strlen(input);
    while (length > 0 && (input[length - 1] == '\n' || input[length - 1] == '\r')) {
        input[--length] = '\0';
    }

    // Remove quotes if present
    if ((input[0] == '\'' && input[length - 1] == '\'') || (input[0] == '"' && input[length - 1] == '"')) {
        memmove(input, input + 1, length - 2);
        input[length - 2] = '\0';
    }
}

// uses bubble sort
void sortEnvVarsArray(ArrayList *array) {
    if (array == NULL || array->items == NULL) {
        return;
    }

    u32   len = array->length;
    void *tmp = NULL;
    for (u32 i = 0; i < len; ++i) {
        for (u32 j = 0; j < len - 1 - i; ++j) {
            EnvVar *a = arrayGet(array, j);
            EnvVar *b = arrayGet(array, j + 1);

            if (strcmp(a->name, b->name) > 0) {
                // swap array[j] and array[j + 1]
                tmp = arrayGet(array, j);
                arraySet(array, j, arrayGet(array, j + 1));
                arraySet(array, j + 1, tmp);
            }
        }
    }
}

usize findMaxEnvFileKeyWidth(ArrayList *variables) {
    usize maxLength = 0;
    arrayForEach(variables, EnvVar * var) {
        usize nameLength = strlen(var->name);
        maxLength        = max(maxLength, nameLength);
    }
    return maxLength;
}

void printEnvVar(EnvVar *var, int maxLength, boolean colored, boolean showValue) {
    cstr   name      = var->name;
    string nameColor = colored ? GREEN : NO_COLOUR;

    if (!showValue) {
        printf("  %s%-30.30s%s\n", nameColor, name, NO_COLOUR);
        return;
    }

    char    value[1024];
    string  varColor = NO_COLOUR;
    boolean isEmpty  = cstringIsEmpty(var->value);
    boolean isBool = !isEmpty && (cstringEquals(var->value, "true") || cstringEquals(var->value, "false"));

    if (isEmpty || cstringEquals(var->value, "null")) {
        sprintf(value, "(NULL)");
        varColor = RED;
    } else if (!isEmpty) {
        sprintf(value, "%s", var->value);
    }

    if (isBool) {
        varColor = CYAN;
    }

    printf("  %s%-*.*s%s%s%s\n", nameColor, maxLength + 4, maxLength, name, varColor, value, NO_COLOUR);
}

EnvVar *createEnvVarFromLine(cstring line) {
    if (line == NULL) {
        return NULL;
    }

    if (cstringStartsWith(line, "#") || cstringIsEmpty(line)) {
        return NULL;
    }

    string name;
    string value;
    string delimPos = strchr(line, '=');

    if (delimPos == NULL) {
        return NULL;
    }

    usize nameLength = delimPos - line;
    name             = (string) malloc((nameLength + 1) * sizeof(char));

    if (name == NULL) {
        MALLOC_ERR;
        exit(EXIT_FAILURE);
        /* NOT REACHED */
    }

    strncpy(name, line, nameLength);
    name[nameLength]  = '\0';

    usize valueLength = strlen(delimPos + 1);
    value             = (string) malloc((valueLength + 1) * sizeof(char));

    if (value == NULL) {
        free(name);
        MALLOC_ERR;
        exit(EXIT_FAILURE);
        /* NOT REACHED */
    }

    strcpy(value, delimPos + 1);
    value[valueLength] = '\0';
    trimString(value);

    EnvVar *var = malloc(sizeof(*var));

    if (var == NULL) {
        free(name);
        free(value);
        MALLOC_ERR;
        exit(EXIT_FAILURE);
        /* NOT REACHED */
    }

    var->name  = name;
    var->value = value;

    return var;
}

int readEnvFile(void *source, cstring path, EnvVarHandler *handler, cstring ignore, cstring focus) {
    if (source == NULL || path == NULL || handler == NULL) {
        return EXIT_FAILURE;
    }

    if (!fileExists(path)) {
        return fileNotFoundError(path);
    }

    if (ignore != NULL && focus != NULL) {
        printError("Cannot read env file while taking both ignore and focus arguments.");
        return EXIT_FAILURE;
    }

    File  *file;
    string buffer = NULL;
    usize  len    = 0;
    ssize  read;
    file = fopen(path, "r");

    if (file == NULL) {
        return failedToOpenFileError(path);
    }

    // read file line by line and create env var struct from each line
    while ((read = getline(&buffer, &len, file)) != -1) {
        EnvVar *var = createEnvVarFromLine(buffer);

        if (var == NULL) {
            continue;
        }

        if (ignore != NULL && matchPattern(var->name, ignore)) {
            continue;
        }

        if (focus != NULL && !matchPattern(var->name, focus)) {
            continue;
        }

        handler(source, var);
    }

    // cleanup
    fclose(file);
    return EXIT_SUCCESS;
}

void pushEnvVarIntoArray(void *array, EnvVar *var) {
    arrayPushBack((ArrayList *) array, var);
}

int readEnvFileIntoArray(ArrayList *variables, cstring path, cstring ignore, cstring focus) {
    return readEnvFile(variables, path, pushEnvVarIntoArray, ignore, focus);
}

void insertEnvVarIntoHashTable(void *hashTable, EnvVar *var) {
    hashTablePut((HashTable *) hashTable, var->name, var);
}

int readEnvFileIntoHashTable(HashTable *table, cstring path, cstring ignore, cstring focus) {
    return readEnvFile(table, path, insertEnvVarIntoHashTable, ignore, focus);
}

boolean matchPattern(cstring str, cstring pattern) {
    // Base case: both strings are empty, pattern matches
    if (cstringIsEmpty(str) && cstringIsEmpty(pattern)) {
        return true;
    }

    // If pattern has '*', try matching with and without consuming a character in the string
    if (*pattern == '*') {
        return matchPattern(str, pattern + 1) || (*str != '\0' && matchPattern(str + 1, pattern));
    }

    // If pattern has '?' or characters match, move to the next character in both strings
    if (*pattern == '?' || (*str != '\0' && *str == *pattern)) {
        return matchPattern(str + 1, pattern + 1);
    }

    // None of the conditions match
    return false;
}

/**
 * List command impl
 */
int list(Command *self) {
    string  path       = getStringOpt(self, "path");
    string  ignore     = getStringOpt(self, "ignore");
    string  key        = getStringOpt(self, "key");
    boolean showValues = getBoolOpt(self, "values");
    boolean colored    = getBoolOpt(self, "colored");

    if (path == NULL || !fileExists(path)) {
        return fileNotFoundError(path);
    }

    // read env file
    ArrayList variables = arrayCreate(sizeof(struct EnvVar), 50, freeEnvVar);
    int       success   = readEnvFileIntoArray(&variables, path, ignore, key);

    // sort env vars array
    sortEnvVarsArray(&variables);

    if (success > EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    // find max name width
    usize maxLength = findMaxEnvFileKeyWidth(&variables);

    // create title underline
    usize pathLength = strlen(path);
    char  underline[pathLength + 2];
    for (usize i = 0; i < pathLength + 1; ++i) {
        underline[i] = '-';
    }
    underline[pathLength + 1] = '\0';

    // print title and underline
    string titleColor = colored ? YELLOW : NO_COLOUR;
    printf("%s%s:%s\n", titleColor, path, NO_COLOUR);
    printf("%s%s%s\n", titleColor, underline, NO_COLOUR);

    // print the sorted env vars
    arrayForEach(&variables, EnvVar * var) {
        printEnvVar(var, maxLength, colored, showValues);
    }

    // cleanup
    arrayFree(&variables);

    return EXIT_SUCCESS;
}

/**
 * Compare command impl
 */
int compare(Command *self) {
    string target = getStringOpt(self, "target");
    string source = getStringOpt(self, "source");
    string ignore = getStringOpt(self, "ignore");
    string key    = getStringOpt(self, "key");
    // boolean missing        = getBoolOpt(self, "missing");
    // boolean undefined      = getBoolOpt(self, "undefined");
    // boolean divergent      = getBoolOpt(self, "divergent");
    // boolean showValues     = getBoolOpt(self, "values");
    // boolean exitOnMismatch = getBoolOpt(self, "exit");
    // boolean colored        = getBoolOpt(self, "colored");
    // boolean print = getBoolOpt(self, "print");

    HashTableConfig  config = hashTableCreateConfig(true, false, false, free, freeEnvVar, NULL);
    HashTableBucket *targetBuckets[50];
    hashTableInitBuckets(targetBuckets, 50);
    HashTable        targetTable = hashTableCreate(50, targetBuckets, &config);
    HashTableBucket *sourceBuckets[50];
    hashTableInitBuckets(sourceBuckets, 50);
    HashTable sourceTable = hashTableCreate(50, sourceBuckets, &config);

    int success = readEnvFileIntoHashTable(&targetTable, target, ignore, key);

    if (success > 0) {
        hashTableDestroy(&targetTable);
        return EXIT_FAILURE;
    }

    success = readEnvFileIntoHashTable(&sourceTable, source, ignore, key);

    if (success > 0) {
        hashTableDestroy(&sourceTable);
        return EXIT_FAILURE;
    }

    // TODO: run comparison based on provided flags
    usize   targetBufferSize = targetTable.size;
    usize   sourceBufferSize = sourceTable.size;
    cstring targetBuffer[targetBufferSize];
    cstring sourceBuffer[sourceBufferSize];
    hashTableKeysBuffer(&targetTable, targetBuffer, targetBufferSize);
    hashTableKeysBuffer(&sourceTable, sourceBuffer, sourceBufferSize);

    for (usize i = 0; i < targetBufferSize; i++) {
        printf("Target var: %s\n", targetBuffer[i]);
    }

    for (usize i = 0; i < sourceBufferSize; i++) {
        printf("Source var: %s\n", sourceBuffer[i]);
    }

    hashTableDestroy(&targetTable);
    hashTableDestroy(&sourceTable);

    return EXIT_FAILURE;
}
