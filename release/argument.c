#include "colors.h"
#include <argument.h>
#include <assert.h>
#include <colors.h>
#include <cstring.h>
#include <math-utils.h>
#include <output.h>
#include <stddef.h>
#include <string.h>

void argument_init(Argument *arg, const char *name, const char *desc) {
    assert(arg != NULL);
    assert(name != NULL);

    arg->name      = name;
    arg->desc      = desc;
    arg->value     = NULL;
    arg->valuev    = NULL;
    arg->valuec    = 0;
    arg->min_count = 0;
    arg->flag      = ARGUMENT_OPTIONAL;
}

void argument_set_flag(Argument *arg, int flag) {
    if (arg) {
        arg->flag = flag;
    }
}

void argument_add_flag(Argument *arg, int flag) {
    if (arg) {
        arg->flag |= flag;
    }
}

char *argument_get(Argument **argv, size_t argc, const char *name) {
    Argument *arg = argument_find(argv, argc, name);
    return arg ? arg->value : NULL;
}

Argument argument_create(const char *name, const char *desc) {
    Argument arg;
    argument_init(&arg, name, desc);
    return arg;
}

int argument_index_of(Argument **argv, size_t argc, const char *name) {
    if (!argv || !name) {
        return -1;
    }

    for (size_t i = 0; i < argc; ++i) {
        Argument *arg = argv[i];

        if (arg && name && str_equals_case_insensitive(arg->name, name)) {
            return i;
        }
    }

    return -1;
}

Argument *argument_find(Argument **argv, size_t argc, const char *name) {
    int i = argument_index_of(argv, argc, name);
    return i == -1 ? NULL : argv[i];
}

void argument_print_tag(Argument *arg) {
    assert(arg != NULL);
    assert(arg->name != NULL);
    bool optional = argument_is_optional(arg);
    bool multi    = argument_is_array(arg);
    printf(" ");
    if (optional) {
        printf("[");
    }
    printf("<%s>", arg->name);
    if (multi) {
        printf("...");
    }
    if (optional) {
        printf("]");
    }
}

void argument_print(Argument *arg, int width) {
    assert(arg != NULL);

    pcolorf(GREEN, "  %-*s", width + 2, arg->name);
    printf("%s", arg->desc);

    if (argument_is_optional(arg) && arg->value) {
        printf("%s [default: %s]%s", YELLOW, arg->value, NO_COLOUR);
    }

    printf("\n");
}

void argument_print_all(Argument **argv, size_t argc, int width) {
    for (size_t i = 0; i < argc; ++i) {
        argument_print(argv[i], width);
    }
}

int argument_get_print_width(Argument **argv, size_t argc) {
    int width = 0;
    for (size_t i = 0; i < argc; ++i) {
        width = max(width, (int) strlen(argv[i]->name));
    }
    return width;
}

void argument_validate_order(Argument **argv, size_t argc) {
    if (argc > 0) {
        assert(argv != NULL);
    }

    int state = ARGUMENT_REQUIRED;

    for (size_t i = 0; i < argc; ++i) {
        Argument *arg = argv[i];

        // array type argument that is not last
        if (argument_is_array(arg)) {
            assert(i + 1 >= argc);
        }

        // optional argument that is not last
        if (argument_is_optional(arg)) {
            assert(i + 1 >= argc);
        }

        // ensure required args follow required args,
        // or an optional arg follows a required arg
        assert(arg->flag >= state);

        state = max(state, arg->flag);
    }
}

bool argument_is_required(Argument *arg) {
    assert(arg != NULL);
    return arg->flag & ARGUMENT_REQUIRED;
}

bool argument_is_optional(Argument *arg) {
    return !argument_is_required(arg);
}

bool argument_is_array(Argument *arg) {
    assert(arg != NULL);
    return arg->flag & ARGUMENT_ARRAY;
}

void argument_make_required(Argument *arg) {
    if (arg) {
        argument_set_flag(arg, ARGUMENT_REQUIRED);
    }
}

void argument_set_default_value(Argument *arg, char *val) {
    if (arg) {
        arg->value = val;
    }
}

void argument_make_array(Argument *arg, size_t min) {
    if (arg) {
        argument_set_flag(arg, ARGUMENT_ARRAY);
        argument_add_flag(arg, min > 0 ? ARGUMENT_REQUIRED : ARGUMENT_OPTIONAL);
        arg->min_count = min;
    }
}
