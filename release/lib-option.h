#ifndef OPTION_H
#define OPTION_H

#include <lib-types.h>

#ifndef OPTION_MAX_LEVEL
# define OPTION_MAX_LEVEL 3
#endif // OPTION_MAX_LEVEL

#include <assert.h>
#include <math.h>

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

#endif // OPTION_H
