#ifndef OPTION_H
#define OPTION_H

#include "math.h"
#include "types.h"

#ifndef UTILS_IMPLEMENTATION
# define UTILS_IMPLEMENTATION
# include "utils.h"
#endif // UTILS_IMPLEMENTATION

#ifndef CSTRING_IMPLEMENTATION
# define CSTRING_IMPLEMENTATION
# include "cstring.h"
#endif // CSTRING_IMPLEMENTATION

#ifndef OPTION_MAX_LEVEL
# define OPTION_MAX_LEVEL 3
#endif // OPTION_MAX_LEVEL

#include <assert.h>
#include <math.h>

// todo: handle negatable options in optionFind() and optionPrint()

// ==== Typedefs ==============================================================/

typedef enum OptionFlag {
    OPTION_NO_VALUE        = 1 << 0,
    OPTION_VALUE_REQUIRED  = 1 << 1,
    OPTION_VALUE_OPTIONAL  = 1 << 2,
    OPTION_VALUE_ARRAY     = 1 << 3,
    OPTION_VALUE_NEGATABLE = 1 << 4,
    OPTION_LEVELS          = 1 << 5,
} OptionFlag;

typedef enum OptionMetaFlag {
    OPTION_META_CLEANUP_VALUE       = 1 << 0,
    OPTION_META_CLEANUP_VALUE_ARRAY = 1 << 1,
    // OPTION_META_TYPE_LEVEL          = 1 << 2,
    // OPTION_META_TYPE_BOOL           = 1 << 3,
    // OPTION_META_TYPE_STRING         = 1 << 4,
    // OPTION_META_TYPE_ARRAY          = 1 << 5,
} OptionMetaFlag;

// typedef union OptionValue {
//     uint    levelValue;
//     string  stringValue;
//     boolean boolValue;
//     string *stringArray;
// } OptionValue;

typedef struct Option {
    cstring name;
    cstring shortcut;
    cstring desc;
    boolean provided;
    uint    levelValue; // todo: try out using a union
    string  stringValue;
    boolean boolValue;
    string *stringArray;
    usize   arraySize;
    uint    __flags;
    uint    __meta;
} Option;

// ==== Definitions ===========================================================/
void    optionInit(Option *opt, cstring name, cstring shortcut, cstring desc);
Option  optionCreate(cstring name, cstring shortcut, cstring desc); // todo: change arg order
Option  optionCreateStringOpt(cstring name, cstring shortcut, cstring desc, string defaultVal, boolean required);
Option  optionCreateBoolOpt(cstring name, cstring shortcut, cstring desc, boolean defaultVal, boolean negatable);
Option  optionCreateArrayOpt(cstring name, cstring shortcut, cstring desc);
void    optionSetFlag(Option *opt, uint flag);
void    optionAddFlags(Option *opt, uint flag);
int     optionIndexOf(Option **opts, usize optsCount, cstring name, cstring shortcut);
Option *optionFind(Option **opts, usize optsCount, cstring name, cstring shortcut);
Option *optionFindByShortcut(Option **opts, usize optsCount, char shortcut);
Option *optionFindByLongName(Option **opts, usize optsCount, cstring name);
boolean optionGetBool(Option **opts, usize optsCount, cstring name);
string  optionGetString(Option **opts, usize optsCount, cstring name);
string *optionGetStrings(Option **opts, usize optsCount, cstring name);
uint    optionGetPrintWidth(Option **opts, usize optsCount);
void    optionPrintTag(Option *opt);
void    optionPrint(Option *opt, uint width);
void    optionPrintAll(Option **opts, usize optsCount, uint width);
void    optionSort(Option **opts, usize optsCount);
boolean optionIsShort(cstring name);
boolean optionIsLong(cstring name);
boolean optionHasValue(Option *opt);
boolean optionIsBool(Option *opt);
boolean optionIsNegatable(Option *opt);
boolean optionIsLevel(Option *opt);
boolean optionIsString(Option *opt);
boolean optionIsArray(Option *opt);
boolean optionExpectsValue(Option *opt);
boolean optionWasProvided(Option *opt);

// ==== Implementations =======================================================/
void optionInit(Option *opt, cstring name, cstring shortcut, cstring desc) {
    assert(opt != NULL);
    assert(name != NULL);

    opt->shortcut    = shortcut;
    opt->name        = name;
    opt->desc        = desc;
    opt->provided    = false;
    opt->boolValue   = false;
    opt->levelValue  = 0;
    opt->stringValue = NULL;
    opt->stringArray = NULL;
    opt->arraySize   = 0;
    opt->__flags     = OPTION_NO_VALUE;
    opt->__meta      = 0;
}

Option optionCreate(cstring name, cstring shortcut, cstring desc) {
    Option opt;
    optionInit(&opt, name, shortcut, desc);
    return opt;
}

Option optionCreateStringOpt(cstring name, cstring shortcut, cstring desc, string defaultVal, boolean required) {
    Option opt;
    optionInit(&opt, name, shortcut, desc);
    optionSetFlag(&opt, required ? OPTION_VALUE_REQUIRED : OPTION_VALUE_OPTIONAL);
    opt.stringValue = defaultVal;
    return opt;
}

Option optionCreateBoolOpt(cstring name, cstring shortcut, cstring desc, boolean defaultVal, boolean negatable) {
    Option opt;
    optionInit(&opt, name, shortcut, desc);
    if (negatable) {
        optionAddFlags(&opt, OPTION_VALUE_NEGATABLE);
    }
    opt.boolValue = defaultVal;
    return opt;
}

Option optionCreateArrayOpt(cstring name, cstring shortcut, cstring desc) {
    Option opt;
    optionInit(&opt, name, shortcut, desc);
    optionSetFlag(&opt, OPTION_VALUE_ARRAY);
    return opt;
}

void optionSetFlag(Option *opt, uint flag) {
    if (opt) {
        opt->__flags = flag;
    }
}

void optionAddFlags(Option *opt, uint flag) {
    if (opt) {
        opt->__flags |= flag;
    }
}

Option *optionFind(Option **opts, usize optsCount, cstring name, cstring shortcut) {
    int index = optionIndexOf(opts, optsCount, name, shortcut);
    return index == -1 ? NULL : opts[index];
}

Option *optionFindByShortcut(Option **opts, usize optsCount, char shortcut) {
    char __short[2];
    __short[0] = shortcut;
    __short[1] = '\0';
    return optionFind(opts, optsCount, NULL, __short);
}

Option *optionFindByLongName(Option **opts, usize optsCount, cstring name) {
    return optionFind(opts, optsCount, name, NULL);
}

int optionIndexOf(Option **opts, usize optsCount, cstring name, cstring shortcut) {
    if (opts == NULL) {
        return -1;
    }

    assert(name != NULL || shortcut != NULL);

    boolean mightBeNegating = cstringStartsWith(name, "no-");

    for (usize i = 0; i < optsCount; ++i) {
        Option *opt = opts[i];
        assert(opt != NULL);
        assert(opt->name != NULL);

        if (name) {
            if (cstringEquals(opt->name, name)) {
                return i;
            }

            if (mightBeNegating && optionIsNegatable(opt)) {
                char negatingName[strlen(opt->name) + 1 + 3];
                strcpy(negatingName, "no-");
                strcat(negatingName, opt->name);

                if (cstringEquals(negatingName, name)) {
                    return i;
                }
            }
        } else if (opt->shortcut && shortcut && cstringEquals(opt->shortcut, shortcut)) {
            return i;
        }
    }

    return -1;
}

boolean optionGetBool(Option **opts, usize optsCount, cstring name) {
    Option *opt = optionFind(opts, optsCount, name, NULL);
    return opt ? opt->boolValue : false;
}

uint optionGetLevel(Option **opts, usize optsCount, cstring name) {
    Option *opt = optionFind(opts, optsCount, name, NULL);
    return opt ? opt->levelValue : 0;
}

string optionGetString(Option **opts, usize optsCount, cstring name) {
    Option *opt = optionFind(opts, optsCount, name, NULL);
    return opt ? opt->stringValue : NULL;
}

string *optionGetStrings(Option **opts, usize optsCount, cstring name) {
    Option *opt = optionFind(opts, optsCount, name, NULL);
    return opt ? opt->stringArray : NULL;
}

void optionPrintTag(Option *opt) {
    assert(opt != NULL);
    assert(opt->name != NULL);

    printf(" [");
    if (opt->shortcut) {
        printf("-%c|", opt->shortcut[0]);
    }
    printf("--%s", opt->name);
    if (optionIsNegatable(opt)) {
        printf("|no-%s", opt->name);
    } else if (optionHasValue(opt)) {
        char capitalized[strlen(opt->name) + 1];
        printf(" %s", stringToCstring(stringToUpper(stringFrom(opt->name), capitalized)));
    }
    printf("]");
}

void optionPrint(Option *opt, uint width) {
    assert(opt != NULL);
    assert(opt->name != NULL);
    char name[width + 1];

    if (opt->shortcut) {
        strcpy(name, "-");
        if (optionIsLevel(opt)) {
            for (usize i = 0; i < OPTION_MAX_LEVEL; ++i) {
                boolean hasNext = i + 1 < OPTION_MAX_LEVEL;

                for (usize j = 0; j < i + 1; ++j) {
                    char shortStr[2];
                    shortStr[0] = opt->shortcut[0];
                    shortStr[1] = '\0';
                    strcat(name, shortStr);
                }

                if (hasNext) {
                    strcat(name, "|");
                }
            }
        } else {
            char shortStr[2];
            shortStr[0] = opt->shortcut[0];
            shortStr[1] = '\0';
            strcat(name, shortStr);
        }

        strcat(name, ", ");
    } else {
        strcpy(name, "    ");
    }

    strcat(name, "--");
    strcat(name, opt->name);

    // print long
    usize nameLen = strlen(opt->name);

    if (optionHasValue(opt)) {
        char capitalized[nameLen + 1];
        strcat(name, "=");
        strcat(name, stringToCstring(stringToUpper(stringFrom(opt->name), capitalized)));
    } else if (optionIsNegatable(opt)) {
        strcat(name, "|--no-");
        strcat(name, opt->name);
    }

    pcolorf(GREEN, "  %-*s", width + 2, name);

    // desc
    if (opt->desc) {
        printf("%s", opt->desc);
    }

    // default value
    if (opt->stringValue) {
        pcolorf(YELLOW, " [default: %s]", opt->stringValue);
    }

    printf("\n");
}

uint optionGetPrintWidth(Option **opts, usize optsCount) {
    assert(opts != NULL);

    uint width = 4; // base for short options ("-X, ")
    for (usize i = 0; i < optsCount; ++i) {
        Option *opt = opts[i];
        assert(opt != NULL);
        assert(opt->name != NULL);
        uint base = 4;

        if (optionIsLevel(opt)) {
            // e.g. "-v|vv|vvv, "
            u32 chars = (u32) ceil(((OPTION_MAX_LEVEL / 2) + 0.5) * OPTION_MAX_LEVEL) + 1;
            base      = chars + (OPTION_MAX_LEVEL - 1) + 3;
        }

        usize len = strlen(opt->name);

        if (optionHasValue(opt)) {
            // e.g. "name=NAME"
            len = ((len * 2) + 1);
        } else if (optionIsNegatable(opt)) {
            // e.g. "--name|--no-name"
            len = ((len * 2) + 6);
        }

        uint requiredWidth = len + base + 2; // add 2 for the prefixing "--"
        width              = max(width, requiredWidth);
    }

    return width;
}

void optionPrintAll(Option **opts, usize optsCount, uint width) {
    assert(opts != NULL);
    for (usize i = 0; i < optsCount; ++i) {
        optionPrint(opts[i], width);
    }
}

void optionSort(Option **opts, usize optsCount) {
    assert(opts != NULL);
    Option *tmp = NULL;
    for (usize i = 0; i < optsCount; ++i) {
        for (usize j = 0; j < optsCount - 1 - i; ++j) {
            if (strcmp(opts[j]->name, opts[j + 1]->name) > 0) {
                tmp         = opts[j];
                opts[j]     = opts[j + 1];
                opts[j + 1] = tmp;
            }
        }
    }
}

boolean optionIsShort(cstring name) {
    assert(name != NULL);
    usize len = strlen(name);

    if (len < 2) {
        return false;
    }

    return name[0] == '-' && ((name[1] >= 'a' && name[1] <= 'z') || (name[1] >= 'A' && name[1] <= 'Z'));
}

boolean optionIsLong(cstring name) {
    assert(name != NULL);
    usize len = strlen(name);

    if (len < 3) {
        return false;
    }

    return name[0] == '-' && name[1] == '-' &&
           ((name[2] >= 'a' && name[2] <= 'z') || (name[2] >= 'A' && name[2] <= 'Z'));
}

boolean optionIsBool(Option *opt) {
    assert(opt != NULL);
    return opt->__flags & OPTION_NO_VALUE;
}

boolean optionHasValue(Option *opt) {
    assert(opt != NULL);
    if (opt->__flags & OPTION_NO_VALUE) {
        return false;
    }
    if (opt->__flags & OPTION_LEVELS) {
        return false;
    }
    return opt->__flags & OPTION_VALUE_ARRAY || opt->__flags & OPTION_VALUE_REQUIRED ||
           opt->__flags & OPTION_VALUE_OPTIONAL;
}

boolean optionIsLevel(Option *opt) {
    assert(opt != NULL);
    return opt->__flags & OPTION_LEVELS;
}

boolean optionIsNegatable(Option *opt) {
    assert(opt != NULL);
    return opt->__flags & OPTION_VALUE_NEGATABLE;
}

boolean optionIsString(Option *opt) {
    assert(opt != NULL);
    return !optionIsArray(opt) && !optionIsLevel(opt) && optionHasValue(opt);
}

boolean optionIsArray(Option *opt) {
    assert(opt != NULL);
    return opt->__flags & OPTION_VALUE_ARRAY;
}

boolean optionExpectsValue(Option *opt) {
    assert(opt != NULL);
    return opt->__flags & OPTION_VALUE_REQUIRED;
}

boolean optionWasProvided(Option *opt) {
    assert(opt != NULL);
    return opt->provided;
}

#endif // OPTION_H
