#ifndef INPUT_H
#define INPUT_H

#include "argument.h"
#include "command.h"
#include "option.h"
#include "types.h"

#include <assert.h>

// ==== Definitions ===========================================================/
string parseArgs(Option **opts, usize optsCount, Argument **args, usize argsCount, int argc, string argv[], int offset);
string getArg(Command *cmd, cstring name);
boolean       getBoolOpt(Command *cmd, cstring name);
string        getStringOpt(Command *cmd, cstring name);
string       *getStringArrayOpt(Command *cmd, cstring name);
static string handleUnknownOption(cstring name);
static string handleOptValue(Option *opt, string name, string value);

// ==== Implementations =======================================================/
string
parseArgs(Option **opts, usize optsCount, Argument **args, usize argsCount, int argc, string argv[], int offset) {
    usize   currentArgIndex = 0;
    boolean parsingArgs     = false;

    for (int i = offset; i < argc; ++i) {
        string currentArg = argv[i];

        if (cstringEquals(currentArg, "--")) {
            parsingArgs = true;
            continue;
        }

        boolean isLongOption  = optionIsLong(currentArg);
        boolean isShortOption = !isLongOption && optionIsShort(currentArg);

        if (!parsingArgs && (isLongOption || isShortOption)) {
            if (currentArgIndex > 0) {
                if (currentArgIndex < argsCount) {
                    Argument *expectedArg = args[currentArgIndex];
                    char      errorMsg[128];
                    sprintf(errorMsg,
                            "Invalid args. Expected argument '%s', but received option '%s'",
                            expectedArg->name,
                            currentArg);
                    return strdup(errorMsg);
                } else {
                    return NULL;
                }
            }

            // sanitize name (remove leading hyphens)
            int  argOffset = isLongOption ? 2 : 1;
            int  argLength = strlen(currentArg);
            char sanitizedArg[argLength];
            strncpy(sanitizedArg, currentArg + argOffset, argLength);

            if (isShortOption) {
                int shortLength = argLength - argOffset;

                // chained short options
                // currently short options with a value expect a space
                if (shortLength == 1) {
                    Option *opt = optionFind(opts, optsCount, NULL, sanitizedArg);

                    if (!opt) {
                        return handleUnknownOption(sanitizedArg);
                    }

                    string value = NULL;

                    if (optionExpectsValue(opt)) {
                        if (i + 1 < argc) {
                            value = argv[i + 1];
                            ++i;
                        }
                    }

                    string err = handleOptValue(opt, sanitizedArg, value);
                    if (err) {
                        return err;
                    }
                } else {
                    for (int j = 0; j < shortLength; j++) {
                        Option *opt = optionFindByShortcut(opts, optsCount, sanitizedArg[j]);
                        if (opt) {
                            if (optionIsLevel(opt)) {
                                opt->levelValue = min(opt->levelValue + 1, OPTION_MAX_LEVEL);
                            } else {
                                opt->boolValue = true;
                            }

                            opt->provided = true;
                        } else {
                            char strName[2];
                            strName[0] = sanitizedArg[j];
                            strName[1] = '\0';
                            return handleUnknownOption(strName);
                        }
                    }
                }

                continue;
            }

            // extract value from option, if given
            string name  = NULL;
            string value = NULL;
            if (strchr(sanitizedArg, '=') != NULL) {
                name  = strtok(sanitizedArg, "=");
                value = strtok(NULL, "=");
            } else {
                name = sanitizedArg;
            }

            Option *currentOpt = optionFindByLongName(opts, optsCount, name);

            if (currentOpt == NULL) {
                return handleUnknownOption(name);
            }

            string err = handleOptValue(currentOpt, name, value);

            if (err) {
                return err;
            }
        } else {
            parsingArgs = true;

            if (currentArgIndex >= argsCount) {
                return NULL;
            }

            Argument *arg = args[currentArgIndex];

            if (argumentIsArray(arg)) {
                usize newCount = arg->valuesCount + 1;

                if (!arg->values) {
                    arg->values = malloc(sizeof(char *));
                } else {
                    arg->values = realloc(arg->values, newCount * sizeof(char *));
                }

                arg->valuesCount          = newCount;
                arg->values[newCount - 1] = currentArg;
                continue;
            }

            arg->value = currentArg;

            if (currentArgIndex + 1 >= argsCount) {
                return NULL;
            }

            currentArgIndex++;
        }
    }

    for (usize i = 0; i < argsCount; ++i) {
        Argument *arg = args[i];

        // not required, no need to check anything
        if (argumentIsOptional(arg)) {
            continue;
        }

        // array: ensure 1+ values
        if (argumentIsArray(arg)) {
            if (arg->valuesCount < arg->minCount) {
                char errorMsg[128];
                sprintf(errorMsg, "Argument '%s' requires at least %zu %s", arg->name, arg->minCount, arg->minCount == 1 ? "value" : "values");
                return strdup(errorMsg);

            }

            continue;
        }

        // required arg: ensure value was given
        if (arg->value == NULL) {
            char errorMsg[128];
            sprintf(errorMsg, "Argument '%s' is required, but no value was provided", arg->name);
            return strdup(errorMsg);
        }
    }

    return NULL;
}

string getArg(Command *cmd, cstring name) {
    Argument *arg = argumentFind(cmd->args, cmd->argsCount, name);
    if (!arg) {
        return NULL;
    }
    return arg->value;
}

boolean getBoolOpt(Command *cmd, cstring name) {
    Option *opt = optionFind(cmd->opts, cmd->optsCount, name, NULL);

    if (!opt) {
        return false;
    }

    return opt->boolValue;
}

string getStringOpt(Command *cmd, cstring name) {
    Option *opt = optionFind(cmd->opts, cmd->optsCount, name, NULL);

    if (!opt) {
        return NULL;
    }

    return opt->stringValue;
}

string *getStringArrayOpt(Command *cmd, cstring name) {
    Option *opt = optionFind(cmd->opts, cmd->optsCount, name, NULL);

    if (!opt) {
        return NULL;
    }

    return opt->stringArray;
}

static string handleUnknownOption(cstring name) {
    char errorMsg[128];
    sprintf(errorMsg, "Received unknown option: %s", name);
    return strdup(errorMsg);
}

static string handleOptValue(Option *opt, string name, string value) {
    opt->provided = true;

    if (optionIsLevel(opt)) {
        opt->levelValue = min(opt->levelValue + 1, OPTION_MAX_LEVEL);
    } else if (optionIsBool(opt) && !value) {
        if (optionIsNegatable(opt) && cstringStartsWith(name, "no-")) {
            opt->boolValue = false;
        } else {
            opt->boolValue = true;
        }
    } else if (optionExpectsValue(opt) && !value) {
        char errorMsg[256];
        sprintf(errorMsg, "Missing required value for option %s", name);
        return strdup(errorMsg);
    } else if (optionHasValue(opt) && value) {
        string pValue = malloc(sizeof(char) * (strlen(value) + 1));
        assert(pValue != NULL);
        strcpy(pValue, value);

        if (optionIsArray(opt)) {
            usize newCount = opt->arraySize + 1;
            if (!opt->stringArray) {
                opt->stringArray = malloc(sizeof(char *));
            } else {
                opt->stringArray = realloc(opt->stringArray, newCount * sizeof(char *));
            }

            assert(opt->stringArray != NULL);
            opt->arraySize                 = newCount;
            opt->stringArray[newCount - 1] = pValue;
            opt->__meta |= OPTION_META_CLEANUP_VALUE_ARRAY;
        } else {
            opt->stringValue = pValue;
            opt->__meta |= OPTION_META_CLEANUP_VALUE;
        }
    } else {
        #ifndef NDEBUG
         printf("[DEBUG] Unknown handleOpt() case:\n");
         printf("  name: %s\n", name);
         printf("  value: %s\n", value);
         printf("  flags: %d\n", opt->__flags);
         printf("\n");
        #endif
    }

    return NULL;
}

#endif // INPUT_H
