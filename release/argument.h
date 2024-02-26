#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <stdbool.h>
#include <stddef.h>

typedef enum ArgumentFlag {
    ARGUMENT_REQUIRED = 1 << 0,
    ARGUMENT_OPTIONAL = 1 << 1,
    ARGUMENT_ARRAY    = 1 << 2,
} ArgumentFlag;

typedef struct Argument {
    const char *name;
    const char *desc;
    char       *value;
    char      **valuev;
    size_t      valuec;
    size_t      min_count;
    int         flag;
} Argument;

void      argument_init(Argument *arg, const char *name, const char *desc);
Argument  argument_create(const char *name, const char *desc);
void      argument_set_flag(Argument *arg, int flag);
void      argument_add_flag(Argument *arg, int flag);
void      argument_make_required(Argument *arg);
void      argument_make_array(Argument *arg, size_t min);
void      argument_set_default_value(Argument *arg, char *val);
char     *argument_get(Argument **argv, size_t argc, const char *name);
int       argument_index_of(Argument **argv, size_t argc, const char *name);
Argument *argument_find(Argument **argv, size_t argc, const char *name);
void      argument_print_tag(Argument *arg);
void      argument_print(Argument *arg, int width);
void      argument_print_all(Argument **argv, size_t argcs, int width);
int       argument_get_print_width(Argument **argv, size_t argc);
void      argument_validate_order(Argument **argv, size_t argc);
bool      argument_is_required(Argument *arg);
bool      argument_is_optional(Argument *arg);
bool      argument_is_array(Argument *arg);

#endif // ARGUMENT_H
