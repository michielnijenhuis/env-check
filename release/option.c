#include <assert.h>
#include <colors.h>
#include <cstring.h>
#include <math-utils.h>
#include <math.h>
#include <option.h>
#include <output.h>
#include <string.h>

// ==== Implementations =======================================================/
void option_init(option_t *opt, const char *name, const char *shortcut, const char *desc) {
    assert(opt != NULL);
    assert(name != NULL);

    opt->shortcut  = shortcut;
    opt->name      = name;
    opt->desc      = desc;
    opt->provided  = false;
    opt->boolval   = false;
    opt->levelval  = 0;
    opt->stringval = NULL;
    opt->stringv   = NULL;
    opt->stringc   = 0;
    opt->__flag    = OPTION_NO_VALUE;
    opt->__meta    = 0;
}

option_t option_create(const char *name, const char *shortcut, const char *desc) {
    option_t opt;
    option_init(&opt, name, shortcut, desc);
    return opt;
}

option_t
option_create_string_opt(const char *name, const char *shortcut, const char *desc, char *defval, bool required) {
    option_t opt;
    option_init(&opt, name, shortcut, desc);
    option_set_flag(&opt, required ? OPTION_VALUE_REQUIRED : OPTION_VALUE_OPTIONAL);
    opt.stringval = defval;
    return opt;
}

option_t option_create_bool_opt(const char *name, const char *shortcut, const char *desc, bool defval, bool negatable) {
    option_t opt;
    option_init(&opt, name, shortcut, desc);
    if (negatable) {
        option_add_flag(&opt, OPTION_VALUE_NEGATABLE);
    }
    opt.boolval = defval;
    return opt;
}

option_t option_create_array_opt(const char *name, const char *shortcut, const char *desc) {
    option_t opt;
    option_init(&opt, name, shortcut, desc);
    option_set_flag(&opt, OPTION_VALUE_ARRAY);
    return opt;
}

void option_set_flag(option_t *opt, int flag) {
    if (opt) {
        opt->__flag = flag;
    }
}

void option_add_flag(option_t *opt, int flag) {
    if (opt) {
        opt->__flag |= flag;
    }
}

void option_set_boolval(option_t *opt, bool val) {
    if (opt) {
        opt->boolval = val;
    }
}

void option_set_strval(option_t *opt, char *val) {
    if (opt) {
        opt->stringval = val;
    }
}

option_t *option_find(option_t **optv, size_t optc, const char *name, const char *shortcut) {
    int i = option_index_of(optv, optc, name, shortcut);
    return i == -1 ? NULL : optv[i];
}

option_t *option_find_by_shortcut(option_t **optv, size_t optc, char shortcut) {
    char __short[2];
    __short[0] = shortcut;
    __short[1] = '\0';
    return option_find(optv, optc, NULL, __short);
}

option_t *option_find_by_long_name(option_t **optv, size_t optc, const char *name) {
    return option_find(optv, optc, name, NULL);
}

int option_index_of(option_t **optv, size_t optc, const char *name, const char *shortcut) {
    if (optv == NULL) {
        return -1;
    }

    assert(name != NULL || shortcut != NULL);

    bool might_be_negating = str_starts_with(name, "no-");

    for (size_t i = 0; i < optc; ++i) {
        option_t *opt = optv[i];
        assert(opt != NULL);
        assert(opt->name != NULL);

        if (name) {
            if (str_equals_case_insensitive(opt->name, name)) {
                return i;
            }

            if (might_be_negating && option_is_negatable(opt)) {
                char negating_name[strlen(opt->name) + 1 + 3];
                strcpy(negating_name, "no-");
                strcat(negating_name, opt->name);

                if (str_equals_case_insensitive(negating_name, name)) {
                    return i;
                }
            }
        } else if (opt->shortcut && shortcut && str_equals(opt->shortcut, shortcut)) {
            return i;
        }
    }

    return -1;
}

bool option_get_bool(option_t **optv, size_t optc, const char *name) {
    option_t *opt = option_find(optv, optc, name, NULL);
    return opt ? opt->boolval : false;
}

int option_get_level(option_t **optv, size_t optc, const char *name) {
    option_t *opt = option_find(optv, optc, name, NULL);
    return opt ? opt->levelval : 0;
}

char *option_get_string(option_t **optv, size_t optc, const char *name) {
    option_t *opt = option_find(optv, optc, name, NULL);
    return opt ? opt->stringval : NULL;
}

char **option_get_strings(option_t **optv, size_t optc, const char *name) {
    option_t *opt = option_find(optv, optc, name, NULL);
    return opt ? opt->stringv : NULL;
}

void option_print_tag(option_t *opt) {
    assert(opt != NULL);
    assert(opt->name != NULL);

    writef(" [");
    if (opt->shortcut) {
        writef("-%c|", opt->shortcut[0]);
    }
    writef("--%s", opt->name);
    if (option_is_negatable(opt)) {
        writef("|no-%s", opt->name);
    } else if (option_has_value(opt)) {
        char upper[strlen(opt->name) + 1];
        strcpy(upper, opt->name);
        str_to_upper(upper);
        writef(" %s", upper);
    }
    writef("]");
}

void option_print(option_t *opt, int width) {
    assert(opt != NULL);
    assert(opt->name != NULL);
    char name[width + 1];

    if (opt->shortcut) {
        strcpy(name, "-");
        if (option_is_level(opt)) {
            for (size_t i = 0; i < OPTION_MAX_LEVEL; ++i) {
                bool has_next = i + 1 < OPTION_MAX_LEVEL;

                for (size_t j = 0; j < i + 1; ++j) {
                    char shortstr[2];
                    shortstr[0] = opt->shortcut[0];
                    shortstr[1] = '\0';
                    strcat(name, shortstr);
                }

                if (has_next) {
                    strcat(name, "|");
                }
            }
        } else {
            char shortstr[2];
            shortstr[0] = opt->shortcut[0];
            shortstr[1] = '\0';
            strcat(name, shortstr);
        }

        strcat(name, ", ");
    } else {
        strcpy(name, "    ");
    }

    strcat(name, "--");
    strcat(name, opt->name);

    // print long
    size_t namelen = strlen(opt->name);

    if (option_has_value(opt)) {
        char upper[namelen + 1];
        strcpy(upper, opt->name);
        str_to_upper(upper);
        strcat(name, "=");
        strcat(name, upper);
    } else if (option_is_negatable(opt)) {
        strcat(name, "|--no-");
        strcat(name, opt->name);
    }

    pcolorf(GREEN, "  %-*s", width + 2, name);

    // desc
    if (opt->desc) {
        writef("%s", opt->desc);
    }

    // default value
    if (opt->stringval) {
        writef("%s%s[default: %s]%s", YELLOW, (opt->desc ? " " : ""), opt->stringval, NO_COLOR);
    }

    writef("\n");
}

int option_get_print_width(option_t **optv, size_t optc) {
    assert(optv != NULL);

    int width = 4; // base for short options ("-X, ")
    for (size_t i = 0; i < optc; ++i) {
        option_t *opt = optv[i];
        assert(opt != NULL);
        assert(opt->name != NULL);
        int base = 4;

        if (option_is_level(opt)) {
            // e.g. "-v|vv|vvv, "
            u_int32_t chars = (u_int32_t) ceil(((OPTION_MAX_LEVEL / 2) + 0.5) * OPTION_MAX_LEVEL) + 1;
            base            = chars + (OPTION_MAX_LEVEL - 1) + 3;
        }

        size_t len = strlen(opt->name);

        if (option_has_value(opt)) {
            // e.g. "name=NAME"
            len = ((len * 2) + 1);
        } else if (option_is_negatable(opt)) {
            // e.g. "--name|--no-name"
            len = ((len * 2) + 6);
        }

        int req_width = len + base + 2; // add 2 for the prefixing "--"
        width         = max(width, req_width);
    }

    return width;
}

void option_print_all(option_t **optv, size_t optc, int width) {
    assert(optv != NULL);
    for (size_t i = 0; i < optc; ++i) {
        option_print(optv[i], width);
    }
}

void option_sort(option_t **optv, size_t optc) {
    assert(optv != NULL);
    option_t *tmp = NULL;
    for (size_t i = 0; i < optc; ++i) {
        for (size_t j = 0; j < optc - 1 - i; ++j) {
            if (strcmp(optv[j]->name, optv[j + 1]->name) > 0) {
                tmp         = optv[j];
                optv[j]     = optv[j + 1];
                optv[j + 1] = tmp;
            }
        }
    }
}

bool option_is_short(const char *name) {
    assert(name != NULL);
    size_t len = strlen(name);

    if (len < 2) {
        return false;
    }

    return name[0] == '-' && ((name[1] >= 'a' && name[1] <= 'z') || (name[1] >= 'A' && name[1] <= 'Z'));
}

bool option_is_long(const char *name) {
    assert(name != NULL);
    size_t len = strlen(name);

    if (len < 3) {
        return false;
    }

    return name[0] == '-' && name[1] == '-' &&
           ((name[2] >= 'a' && name[2] <= 'z') || (name[2] >= 'A' && name[2] <= 'Z'));
}

bool option_is_bool(option_t *opt) {
    assert(opt != NULL);
    return opt->__flag & OPTION_NO_VALUE;
}

bool option_has_value(option_t *opt) {
    assert(opt != NULL);
    if (opt->__flag & OPTION_NO_VALUE) {
        return false;
    }
    if (opt->__flag & OPTION_LEVELS) {
        return false;
    }

    return opt->__flag & OPTION_VALUE_ARRAY || opt->__flag & OPTION_VALUE_REQUIRED ||
           opt->__flag & OPTION_VALUE_OPTIONAL;
}

bool option_is_level(option_t *opt) {
    assert(opt != NULL);
    return opt->__flag & OPTION_LEVELS;
}

bool option_is_negatable(option_t *opt) {
    assert(opt != NULL);
    return opt->__flag & OPTION_VALUE_NEGATABLE;
}

bool option_is_string(option_t *opt) {
    assert(opt != NULL);
    return !option_is_array(opt) && !option_is_level(opt) && option_has_value(opt);
}

bool option_is_array(option_t *opt) {
    assert(opt != NULL);
    return opt->__flag & OPTION_VALUE_ARRAY;
}

bool option_expects_value(option_t *opt) {
    assert(opt != NULL);
    return opt->__flag & OPTION_VALUE_REQUIRED;
}

bool option_was_provided(option_t *opt) {
    assert(opt != NULL);
    return opt->provided;
}
