#ifndef OPTION_H
#define OPTION_H

#include <stdbool.h>
#include <stddef.h>

#define OPTION_MAX_LEVEL 3

typedef enum OptionFlag {
    OPTION_NO_VALUE        = 1 << 0,
    OPTION_VALUE_REQUIRED  = 1 << 1,
    OPTION_VALUE_OPTIONAL  = 1 << 2,
    OPTION_VALUE_ARRAY     = 1 << 3,
    OPTION_VALUE_NEGATABLE = 1 << 4,
    OPTION_LEVELS          = 1 << 5,
} option_flag_t;

typedef enum OptionMetaFlag {
    OPTION_META_CLEANUP_OPT         = 1 << 0,
    OPTION_META_CLEANUP_VALUE       = 1 << 1,
    OPTION_META_CLEANUP_VALUE_ARRAY = 1 << 2,
} option_meta_flag_t;

typedef struct option_t {
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
} option_t;

void     option_init(option_t *opt, const char *name, const char *shortcut, const char *desc);
option_t option_create(const char *name, const char *shortcut, const char *desc);
option_t
option_create_string_opt(const char *name, const char *shortcut, const char *desc, char *defval, bool required);
option_t  option_create_bool_opt(const char *name, const char *shortcut, const char *desc, bool defval, bool negatable);
option_t  option_create_array_opt(const char *name, const char *shortcut, const char *desc);
void      option_set_flag(option_t *opt, int flag);
void      option_add_flag(option_t *opt, int flag);
int       option_index_of(option_t **optv, size_t optsc, const char *name, const char *shortcut);
option_t *option_find(option_t **optv, size_t optc, const char *name, const char *shortcut);
option_t *option_find_by_shortcut(option_t **optv, size_t optc, char shortcut);
option_t *option_find_by_long_name(option_t **optv, size_t optc, const char *name);
bool      option_get_bool(option_t **optv, size_t optc, const char *name);
char     *option_get_string(option_t **optv, size_t optc, const char *name);
char    **option_get_strings(option_t **optv, size_t optc, const char *name);
int       option_get_print_width(option_t **optv, size_t optc);
void      option_print_tag(option_t *opt);
void      option_print(option_t *opt, int width);
void      option_print_all(option_t **optv, size_t optc, int width);
void      option_sort(option_t **optv, size_t optc);
bool      option_is_short(const char *name);
bool      option_is_long(const char *name);
bool      option_has_value(option_t *opt);
bool      option_is_bool(option_t *opt);
bool      option_is_negatable(option_t *opt);
bool      option_is_level(option_t *opt);
bool      option_is_string(option_t *opt);
bool      option_is_array(option_t *opt);
bool      option_expects_value(option_t *opt);
bool      option_was_provided(option_t *opt);
void      option_set_boolval(option_t *opt, bool val);
void      option_set_strval(option_t *opt, char *val);

#endif // OPTION_H
