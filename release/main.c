//=== Includes ===============================================================//
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

//=== TODOS ==================================================================//
// implement multiple ignore/focus strings (seperate by ,)

//=== Defines ================================================================//
#define ENVC_EXECUTABLE "envc"
#define ENVC_NAME "Env Check"
#define ENVC_VERSION "1.0.0"
#define MALLOC_ERR printError("Memory allocation failed.")

//=== Typedefs ===============================================================//
typedef enum EnvVarStatus {
    OK        = 0,
    MISSING   = 1,
    DIVERGENT = 2,
    UNDEFINED = 3,
} EnvVarStatus;

typedef struct EnvVar {
    string       name;
    string       value;
    string       compareValue;
    EnvVarStatus status;
    usize        nameLength;
    usize        valueLength;
    usize        compareValueLength;
    boolean      isDouble;
} EnvVar;

//=== Defs ===================================================================//
void    freeEnvVar(void *var);
void    freeStrings(String *strings, usize len);
void    createEnvVarFromLine(HashTable *table,
                             cstring    line,
                             String    *ignore,
                             usize      ignoreSize,
                             String    *focus,
                             usize      focusSize,
                             boolean    comparing);
void    setEnvVarStatus(EnvVar *var);
int     readEnvFile(HashTable *table,
                    cstring    path,
                    String    *ignore,
                    usize      ignoreSize,
                    String    *focus,
                    usize      focusSize,
                    boolean    comparing);
usize   findMaxWidthInArray(EnvVar **variables, usize variablesSize, boolean name);
void    trimString(string input);
boolean isNumeric(cstring str);
void    printEnvVar(const EnvVar *var,
                    uint          firstColumnWidth,
                    uint          secondColumnWidth,
                    uint          thirdColumnWidth,
                    boolean       comparing);
void    sortEnvVarsArray(cstring *keys, usize size);
boolean matchPattern(cstring str, cstring pattern);
void    printTitle(cstring fileName);
int     list(Command *self);
int     compare(Command *self);

//=== Main ===================================================================//
int main(int argc, string argv[]) {
    //=== Shared options =====================================================//
    Option ignoreOpt =
        createStringOption("i", "ignore", "Comma seperated list of variable name patterns to ignore", NULL);
    Option keyOpt = createStringOption("k", "key", "Comma seperated list of variable name patterns to focus on", NULL);
    Option truncateOpt = createStringOption("T", "truncate", "The amount of chars to truncate keys or values to", "40");

    //=== Compare ============================================================//
    Command compareCmd     = createCommand("cmp", "Compares two env files files.", compare);
    Option  cmp__targetOpt = createStringOption("t", "target", "Path to the .env file to compare with", "./.env");
    Option  cmp__sourceOpt = createStringOption("s", "source", "Path to the .env file to compare to", "./.env.example");
    Option  cmp__missingOpt = createBoolOption("m", "missing", "Show missing and empty variables");
    Option  cmp__undefinedOpt =
        createBoolOption("u", "undefined", "Show variables in the target that aren't in the source file");
    Option  cmp__divergentOpt = createBoolOption("d", "divergent", "Show variables with diverging values");
    Option *cmd__opts[]       = {&cmp__targetOpt,
                                 &cmp__sourceOpt,
                                 &ignoreOpt,
                                 &keyOpt,
                                 &truncateOpt,
                                 &cmp__missingOpt,
                                 &cmp__undefinedOpt,
                                 &cmp__divergentOpt};
    compareCmd.opts           = cmd__opts;
    compareCmd.optsCount      = ARRAY_LEN(cmd__opts);

    //=== List ===============================================================//
    Command listCmd = createCommand("list", "Lists all variables in the target env file, sorted alphabetically.", list);
    Option  list__pathOpt = createStringOption("p", "path", "Path to the .env file", "./.env.example");
    Option *list__opts[]  = {&list__pathOpt, &ignoreOpt, &keyOpt, &truncateOpt};
    listCmd.opts          = list__opts;
    listCmd.optsCount     = ARRAY_LEN(list__opts);

    //=== Env Check ==========================================================//
    Command *commands[] = {&compareCmd, &listCmd};
    Program  program    = {
            .executableName = ENVC_EXECUTABLE,
            .name           = ENVC_NAME,
            .version        = ENVC_VERSION,
            .commands       = commands,
            .commandsCount  = ARRAY_LEN(commands),
    };

    return runApplication(&program, argc, argv);
}

//=== Helpers ================================================================//
void freeEnvVar(void *var) {
    free(((EnvVar *) var)->name);
    free(((EnvVar *) var)->value);
    free(var);
}

void freeStrings(String *strings, usize len) {
    for (usize i = 0; i < len; ++i) {
        free((string) strings[i].data);
    }
}

boolean isNumeric(cstring str) {
    if (str == NULL || *str == '\0') {
        return false;
    }

    while (*str != '\0') {
        if (*str < '0' || *str > '9') {
            return false;
        }
        str++;
    }

    return true;
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

void sortEnvVarsArray(cstring *keys, usize size) {
    if (keys == NULL) {
        return;
    }

    cstring tmp = NULL;
    for (u32 i = 0; i < size; ++i) {
        for (u32 j = 0; j < size - 1 - i; ++j) {
            if (strcmp(keys[j], keys[j + 1]) > 0) {
                // swap
                tmp         = keys[j];
                keys[j]     = keys[j + 1];
                keys[j + 1] = tmp;
            }
        }
    }
}

usize findMaxWidthInArray(EnvVar **variables, usize variablesSize, boolean name) {
    usize maxLength = 0;
    for (usize i = 0; i < variablesSize; ++i) {
        EnvVar *var = variables[i];
        usize   len = 0;

        if (!name && var->value != NULL) {
            len = strlen(var->value);
        } else if (name && var->name != NULL) {
            len = strlen(var->name);
        }
        maxLength = max(maxLength, len);
    }
    return maxLength;
}

void printEnvVar(const EnvVar *var,
                 uint          firstColumnWidth,
                 uint          secondColumnWidth,
                 uint          thirdColumnWidth,
                 boolean       comparing) {
    string  keyColor      = WHITE_BOLD;
    string  boolColor     = CYAN;
    string  numColor      = EMERALD;
    string  strColor      = NO_COLOUR;
    boolean valueAIsEmpty = cstringIsEmpty(var->value);
    boolean valueAIsBool =
        cstringEqualsCaseInsensitive(var->value, "true") || cstringEqualsCaseInsensitive(var->value, "false");
    boolean valueAIsNumeric = !valueAIsBool && isNumeric(var->value);
    string  valueAColor     = valueAIsBool ? boolColor : valueAIsNumeric ? numColor : strColor;
    usize   valueASize      = max(secondColumnWidth + 1, 8);
    boolean valueBIsEmpty   = comparing ? cstringIsEmpty(var->compareValue) : true;
    boolean valueBIsBool    = comparing ? cstringEqualsCaseInsensitive(var->compareValue, "true") ||
                                           cstringEqualsCaseInsensitive(var->compareValue, "false")
                                        : false;
    boolean valueBIsNumeric = !valueBIsBool && isNumeric(var->compareValue);
    string  valueBColor     = valueBIsBool ? boolColor : valueBIsNumeric ? numColor : strColor;
    usize   valueBSize      = comparing ? max(thirdColumnWidth + 1, 8) : 0;
    char    __valueA[valueASize];
    char    __valueB[valueBSize];

    if (valueAIsEmpty) {
        sprintf(__valueA, "(NULL)");
        valueAColor = DARK_GRAY;
    } else if (!valueAIsEmpty) {
        if (var->valueLength > secondColumnWidth) {
            strncpy(__valueA, var->value, secondColumnWidth - 3);
            strcat(__valueA, "...");
            __valueA[secondColumnWidth] = '\0';
        } else {
            sprintf(__valueA, "%s", var->value);
        }
    }

    if (comparing) {
        if (valueBIsEmpty) {
            sprintf(__valueB, "(NULL)");
            valueBColor = RED;
        } else if (!valueBIsEmpty) {
            if (var->compareValueLength > thirdColumnWidth) {
                strncpy(__valueB, var->compareValue, thirdColumnWidth - 4);
                strcat(__valueB, "...");
                __valueB[thirdColumnWidth] = '\0';
            } else {
                sprintf(__valueB, "%s", var->compareValue);
            }
        }
    }

    string status;
    string statusColor = NO_COLOUR;

    if (comparing) {
        switch (var->status) {
            case MISSING:
                status      = "x";
                statusColor = RED_BOLD;
                keyColor    = RED_BOLD;
                break;
            case UNDEFINED:
                status      = "?";
                statusColor = MAGENTA_LIGHT;
                keyColor    = MAGENTA_LIGHT;
                break;
            case DIVERGENT:
                status      = "!";
                statusColor = YELLOW_BOLD;
                keyColor    = YELLOW_BOLD;
                break;
            case OK:
                status = " ";
        }
    }

    printf("  "); // leading spaces
    if (comparing) {
        printf("%s%s%s ", statusColor, status, NO_COLOUR);
    }
    printf("%s%-*.*s%s", keyColor, firstColumnWidth + 4, firstColumnWidth, var->name, NO_COLOUR);     // column 1
    printf("%s%-*.*s%s", valueAColor, secondColumnWidth + 3, secondColumnWidth, __valueA, NO_COLOUR); // column 2
    if (comparing) {
        printf("%s%-*.*s%s", valueBColor, thirdColumnWidth, thirdColumnWidth, __valueB, NO_COLOUR); // column 3
    }
    printf("\n"); // line break
}

void createEnvVarFromLine(HashTable *table,
                          cstring    line,
                          String    *ignore,
                          usize      ignoreSize,
                          String    *focus,
                          usize      focusSize,
                          boolean    comparing) {
    if (table == NULL || line == NULL) {
        return;
    }

    if (cstringStartsWith(line, "#") || cstringIsEmpty(line)) {
        return;
    }

    string delimPos = strchr(line, '=');

    if (delimPos == NULL) {
        return;
    }

    usize nameLength = delimPos - line;
    char  name[nameLength + 1];
    strncpy(name, line, nameLength);
    name[nameLength] = '\0';

    if (ignoreSize > 0) {
        for (usize i = 0; i < ignoreSize; i++) {
            if (matchPattern(name, stringToCstring(ignore[i]))) {
                return;
            }
        }
    }

    if (focusSize > 0) {
        boolean shouldIgnore = true;
        for (usize i = 0; i < focusSize; i++) {
            if (matchPattern(name, stringToCstring(focus[i]))) {
                shouldIgnore = false;
                break;
            }
        }

        if (shouldIgnore) {
            return;
        }
    }

    usize  valueLength = strlen(delimPos + 1);
    string value       = (string) malloc((valueLength + 1) * sizeof(char));

    if (value == NULL) {
        MALLOC_ERR;
        exit(EXIT_FAILURE);
        /* NOT REACHED */
    }

    char trimmed[valueLength + 1];
    strcpy(trimmed, delimPos + 1);
    trimString(trimmed);
    trimmed[valueLength] = '\0';
    if (!cstringEqualsCaseInsensitive(trimmed, "null") && !cstringEqualsCaseInsensitive(trimmed, "(null)")) {
        strcpy(value, trimmed);
    } else {
        value[0]    = '\0';
        valueLength = 0;
    }

    EnvVar *var = hashTableGet(table, name);
    if (var != NULL) {
        if (!comparing || var->compareValue != NULL) {
            var->isDouble = true;
        }

        if (comparing && var->compareValue == NULL) {
            var->compareValue       = value;
            var->compareValueLength = valueLength;
            setEnvVarStatus(var);
        }
    } else {
        var = malloc(sizeof(*var));

        if (var == NULL) {
            free(value);
            MALLOC_ERR;
            exit(EXIT_FAILURE);
            /* NOT REACHED */
        }

        string pName = (string) malloc((nameLength + 1) * sizeof(char));

        if (pName == NULL) {
            free(value);
            free(var);
            MALLOC_ERR;
            exit(EXIT_FAILURE);
            /* NOT REACHED */
        }

        strcpy(pName, name);

        var->compareValue       = comparing ? value : NULL;
        var->name               = pName;
        var->nameLength         = nameLength;
        var->status             = OK;
        var->value              = comparing ? NULL : value;
        var->valueLength        = comparing ? 0 : valueLength;
        var->compareValueLength = comparing ? valueLength : 0;
        hashTablePut(table, name, var);

        setEnvVarStatus(var);
    }
}

void setEnvVarStatus(EnvVar *var) {
    boolean valueIsEmpty        = cstringIsEmpty(var->value);
    boolean compareValueIsEmpty = cstringIsEmpty(var->compareValue);

    if (var->value != NULL && compareValueIsEmpty) {
        var->status = MISSING;
        return;
    }

    if (var->value == NULL) {
        if (var->compareValue != NULL) {
            var->status = UNDEFINED;
        } else {
            var->status = OK;
        }
        return;
    }

    if (!valueIsEmpty && !cstringEquals(var->value, var->compareValue)) {
        var->status = DIVERGENT;
        return;
    }

    var->status = OK;
}

int readEnvFile(HashTable *table,
                cstring    path,
                String    *ignore,
                usize      ignoreSize,
                String    *focus,
                usize      focusSize,
                boolean    comparing) {
    if (table == NULL || path == NULL) {
        return EXIT_FAILURE;
    }

    if (!fileExists(path)) {
        return fileNotFoundError(path);
    }

    if (ignoreSize > 0 && focusSize > 0) {
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

    while ((read = getline(&buffer, &len, file)) != -1) {
        createEnvVarFromLine(table, buffer, ignore, ignoreSize, focus, focusSize, comparing);
    }

    fclose(file);
    return EXIT_SUCCESS;
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

void printTitle(cstring fileName) {
    if (fileName == NULL) {
        return;
    }

    // create title underline
    usize pathLength = strlen(fileName);
    char  underline[pathLength + 1];
    for (usize i = 0; i < pathLength; ++i) {
        underline[i] = '=';
    }
    underline[pathLength] = '\0';

    // print title and underline
    string titleColor = NO_COLOUR;
    printf("%s%s%s\n", titleColor, fileName, NO_COLOUR);
    printf("%s%s%s\n", titleColor, underline, NO_COLOUR);
}

//=== List ===================================================================//
int list(Command *self) {
    string          path       = getStringOpt(self, "path");
    string          ignore     = getStringOpt(self, "ignore");
    string          key        = getStringOpt(self, "key");
    String          strIgnore  = stringFrom(ignore);
    String          strKey     = stringFrom(key);
    string          truncate   = getStringOpt(self, "truncate");
    int             truncateTo = truncate ? atoi(truncate) : 0;
    HashTableConfig config     = hashTableCreateConfig(true, false, false, free, freeEnvVar, NULL);

    usize           ignoreSize = ignore ? stringCharOccurrences(strIgnore, ',') + 1 : 0;
    String          ignoreArr[ignoreSize + 1];
    stringSplitByDelim(strIgnore, ',', ignoreArr, ignoreSize);
    usize  focusSize = key ? stringCharOccurrences(strKey, ',') + 1 : 0;
    String focusArr[focusSize];
    stringSplitByDelim(strKey, ',', focusArr, focusSize);

    HashTableBucket *buckets[50];
    hashTableInitBuckets(buckets, 50);
    HashTable table = hashTableCreate(50, buckets, &config);

    if (readEnvFile(&table, path, ignoreArr, ignoreSize, focusArr, focusSize, false) > 0) {
        freeStrings(ignoreArr, ignoreSize);
        freeStrings(focusArr, focusSize);
        hashTableDestroy(&table);
        return EXIT_FAILURE;
    }

    freeStrings(ignoreArr, ignoreSize);
    freeStrings(focusArr, focusSize);

    usize   vars = table.size;
    cstring keys[vars];
    hashTableKeysBuffer(&table, keys, vars);
    sortEnvVarsArray(keys, vars);

    printTitle(path);

    usize firstColumnWidth  = 7;
    usize secondColumnWidth = 7;

    for (usize i = 0; i < vars; ++i) {
        EnvVar *var       = hashTableGet(&table, keys[i]);
        firstColumnWidth  = max(firstColumnWidth, var->nameLength);
        secondColumnWidth = max(secondColumnWidth, var->valueLength);
    }

    if (truncateTo > 0) {
        firstColumnWidth  = min(firstColumnWidth, truncateTo);
        secondColumnWidth = min(secondColumnWidth, truncateTo);
    }

    for (usize i = 0; i < vars; ++i) {
        EnvVar *var = hashTableGet(&table, keys[i]);
        printEnvVar(var, firstColumnWidth, secondColumnWidth, 0, false);
    }

    hashTableDestroy(&table);

    return EXIT_SUCCESS;
}

//=== Compare ================================================================//
int compare(Command *self) {
    string          target     = getStringOpt(self, "target");
    string          source     = getStringOpt(self, "source");
    string          ignore     = getStringOpt(self, "ignore");
    string          key        = getStringOpt(self, "key");
    String          strIgnore  = stringFrom(ignore);
    String          strKey     = stringFrom(key);
    string          truncate   = getStringOpt(self, "truncate");
    int             truncateTo = truncate ? atoi(truncate) : 0;
    boolean         missing    = getBoolOpt(self, "missing");
    boolean         undefined  = getBoolOpt(self, "undefined");
    boolean         divergent  = getBoolOpt(self, "divergent");
    boolean         selective  = missing || undefined || divergent;
    HashTableConfig config     = hashTableCreateConfig(true, false, false, free, freeEnvVar, NULL);

    usize           ignoreSize = ignore ? stringCharOccurrences(strIgnore, ',') + 1 : 0;
    String          ignoreArr[ignoreSize];
    stringSplitByDelim(strIgnore, ',', ignoreArr, ignoreSize);
    usize  focusSize = key ? stringCharOccurrences(strKey, ',') + 1 : 0;
    String focusArr[focusSize];
    stringSplitByDelim(strKey, ',', focusArr, focusSize);

    HashTableBucket *buckets[50];
    hashTableInitBuckets(buckets, 50);
    HashTable table = hashTableCreate(50, buckets, &config);

    if (readEnvFile(&table, source, ignoreArr, ignoreSize, focusArr, focusSize, false) > 0) {
        freeStrings(ignoreArr, ignoreSize);
        freeStrings(focusArr, focusSize);
        hashTableDestroy(&table);
        return EXIT_FAILURE;
    }

    if (readEnvFile(&table, target, ignoreArr, ignoreSize, focusArr, focusSize, true) > 0) {
        freeStrings(ignoreArr, ignoreSize);
        freeStrings(focusArr, focusSize);
        hashTableDestroy(&table);
        return EXIT_FAILURE;
    }

    freeStrings(ignoreArr, ignoreSize);
    freeStrings(focusArr, focusSize);

    usize   vars = table.size;
    cstring keys[vars];
    hashTableKeysBuffer(&table, keys, vars);
    sortEnvVarsArray(keys, vars);

    char title[FILENAME_MAX];
    sprintf(title, "Comparing '%s' to '%s'", source, target);
    printTitle(title);

    usize firstColumnWidth  = 7;
    usize secondColumnWidth = 7;
    usize thirdColumnWidth  = 7;

    for (usize i = 0; i < vars; ++i) {
        EnvVar *var         = hashTableGet(&table, keys[i]);
        boolean shouldPrint = !selective || (var->status == MISSING && missing) ||
                              (var->status == UNDEFINED && undefined) || (var->status == DIVERGENT && divergent);

        if (!shouldPrint) {
            continue;
        }

        firstColumnWidth  = max(firstColumnWidth, var->nameLength);
        secondColumnWidth = max(secondColumnWidth, var->valueLength);
        thirdColumnWidth  = max(thirdColumnWidth, var->compareValueLength);
    }

    if (truncateTo > 0) {
        firstColumnWidth  = min(firstColumnWidth, truncateTo);
        secondColumnWidth = min(secondColumnWidth, truncateTo);
        thirdColumnWidth  = min(thirdColumnWidth, truncateTo);
    }

    for (usize i = 0; i < vars; ++i) {
        EnvVar *var         = hashTableGet(&table, keys[i]);
        boolean shouldPrint = !selective || (var->status == MISSING && missing) ||
                              (var->status == UNDEFINED && undefined) || (var->status == DIVERGENT && divergent);

        if (shouldPrint) {
            printEnvVar(var, firstColumnWidth, secondColumnWidth, thirdColumnWidth, true);
        }
    }

    hashTableDestroy(&table);

    return EXIT_SUCCESS;
}
