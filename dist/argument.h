#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <stdbool.h>
#include <stddef.h>

typedef enum ArgumentFlag {
    ARGUMENT_REQUIRED = 1 << 0,
    ARGUMENT_OPTIONAL = 1 << 1,
    ARGUMENT_ARRAY    = 1 << 2,
} argument_flag_t;

typedef struct argument_t {
    const char *name;
    const char *desc;
    char       *value;
    char      **valuev;
    size_t      valuec;
    size_t      min_count;
    int         flag;
} argument_t;

void      argument_init(argument_t *arg, const char *name, const char *desc);
argument_t  argument_create(const char *name, const char *desc);
void      argument_set_flag(argument_t *arg, int flag);
void      argument_add_flag(argument_t *arg, int flag);
void      argument_make_required(argument_t *arg);
void      argument_make_array(argument_t *arg, size_t min);
void      argument_set_default_value(argument_t *arg, char *val);
char     *argument_get(argument_t **argv, size_t argc, const char *name);
int       argument_index_of(argument_t **argv, size_t argc, const char *name);
argument_t *argument_find(argument_t **argv, size_t argc, const char *name);
void      argument_print_tag(argument_t *arg);
void      argument_print(argument_t *arg, int width);
void      argument_print_all(argument_t **argv, size_t argcs, int width);
int       argument_get_print_width(argument_t **argv, size_t argc);
void      argument_validate_order(argument_t **argv, size_t argc);
bool      argument_is_required(argument_t *arg);
bool      argument_is_optional(argument_t *arg);
bool      argument_is_array(argument_t *arg);

#endif // ARGUMENT_H
