#ifndef OPTION_H
#define OPTION_H

#include <stdbool.h>
#include <stddef.h>

#ifndef OPTION_MAX_LEVEL
# define OPTION_MAX_LEVEL 3
#endif // OPTION_MAX_LEVEL

typedef enum OptionFlag {
    OPTION_NO_VALUE        = 1 << 0,
    OPTION_VALUE_REQUIRED  = 1 << 1,
    OPTION_VALUE_OPTIONAL  = 1 << 2,
    OPTION_VALUE_ARRAY     = 1 << 3,
    OPTION_VALUE_NEGATABLE = 1 << 4,
    OPTION_LEVELS          = 1 << 5,
} OptionFlag;

typedef enum OptionMetaFlag {
    OPTION_META_CLEANUP_OPT         = 1 << 0,
    OPTION_META_CLEANUP_VALUE       = 1 << 1,
    OPTION_META_CLEANUP_VALUE_ARRAY = 1 << 2,
    // OPTION_META_TYPE_LEVEL          = 1 << 2,
    // OPTION_META_TYPE_BOOL           = 1 << 3,
    // OPTION_META_TYPE_STRING         = 1 << 4,
    // OPTION_META_TYPE_ARRAY          = 1 << 5,
} OptionMetaFlag;

// typedef union OptionValue {
//     int    levelValue;
//     char * stringValue;
//     bool boolValue;
//     char **stringArray;
// } OptionValue;

typedef struct Option {
    const char *name;
    const char *shortcut;
    const char *desc;
    bool        provided;
    int         levelval; // todo: try out using a union
    char       *stringval;
    bool        boolval;
    char      **stringv;
    size_t      stringc;
    int         __flag;
    int         __meta;
} Option;

void    option_init(Option *opt, const char *name, const char *shortcut, const char *desc);
Option  option_create(const char *name, const char *shortcut, const char *desc);
Option  option_create_string_opt(const char *name, const char *shortcut, const char *desc, char *defval, bool required);
Option  option_create_bool_opt(const char *name, const char *shortcut, const char *desc, bool defval, bool negatable);
Option  option_create_array_opt(const char *name, const char *shortcut, const char *desc);
void    option_set_flag(Option *opt, int flag);
void    option_add_flag(Option *opt, int flag);
int     option_index_of(Option **optv, size_t optsc, const char *name, const char *shortcut);
Option *option_find(Option **optv, size_t optc, const char *name, const char *shortcut);
Option *option_find_by_shortcut(Option **optv, size_t optc, char shortcut);
Option *option_find_by_long_name(Option **optv, size_t optc, const char *name);
bool    option_get_bool(Option **optv, size_t optc, const char *name);
char   *option_get_string(Option **optv, size_t optc, const char *name);
char  **option_get_strings(Option **optv, size_t optc, const char *name);
int     option_get_print_width(Option **optv, size_t optc);
void    option_print_tag(Option *opt);
void    option_print(Option *opt, int width);
void    option_print_all(Option **optv, size_t optc, int width);
void    option_sort(Option **optv, size_t optc);
bool    option_is_short(const char *name);
bool    option_is_long(const char *name);
bool    option_has_value(Option *opt);
bool    option_is_bool(Option *opt);
bool    option_is_negatable(Option *opt);
bool    option_is_level(Option *opt);
bool    option_is_string(Option *opt);
bool    option_is_array(Option *opt);
bool    option_expects_value(Option *opt);
bool    option_was_provided(Option *opt);

#endif // OPTION_H
