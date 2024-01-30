#ifndef ARGUMENT_H
#define ARGUMENT_H

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

#include <assert.h>

// ==== Typedefs ==============================================================/
typedef enum ArgumentFlag {
    ARGUMENT_REQUIRED = 1 << 0,
    ARGUMENT_OPTIONAL = 1 << 1,
    ARGUMENT_ARRAY    = 1 << 2,
} ArgumentFlag;

typedef struct Argument {
    cstring name;
    cstring description;
    string  value;
    string *values;
    usize   valuesCount;
    usize   minCount;
    int     flags;
} Argument;

// ==== Definitions ===========================================================/
void      argumentInit(Argument *arg, cstring name, cstring description);
void      argumentSetFlag(Argument *arg, int flag);
void      argumentAddFlags(Argument *arg, int flags);
void      argumentMakeRequired(Argument *arg);
void      argumentMakeArray(Argument *arg, usize min);
void      argumentSetDefaultValue(Argument *arg, string defaultValue);
Argument  argumentCreate(cstring name, cstring description);
string    argumentGet(Argument **args, usize argsCount, cstring name);
int       argumentIndexOf(Argument **args, usize argsSize, cstring name);
Argument *argumentFind(Argument **args, usize argsSize, cstring name);
void      argumentPrintTag(Argument *arg);
void      argumentPrint(Argument *arg, uint width);
void      argumentPrintAll(Argument **args, usize argsCount, uint width);
uint      argumentGetPrintWidth(Argument **args, usize argsCount);
void      argumentValidateOrder(Argument **args, usize argsCount);
boolean   argumentIsRequired(Argument *arg);
boolean   argumentIsOptional(Argument *arg);
boolean   argumentIsArray(Argument *arg);

// ==== Implementations =======================================================/
void argumentInit(Argument *arg, cstring name, cstring description) {
    assert(arg != NULL);
    assert(name != NULL);

    arg->name        = name;
    arg->description = description;
    arg->value       = NULL;
    arg->values      = NULL;
    arg->valuesCount = 0;
    arg->minCount    = 0;
    arg->flags       = ARGUMENT_OPTIONAL;
}

void argumentSetFlag(Argument *arg, int flag) {
    if (arg) {
        arg->flags = flag;
    }
}

void argumentAddFlags(Argument *arg, int flags) {
    if (arg) {
        arg->flags |= flags;
    }
}

string argumentGet(Argument **args, usize argsCount, cstring name) {
    Argument *arg = argumentFind(args, argsCount, name);
    return arg ? arg->value : NULL;
}

Argument argumentCreate(cstring name, cstring description) {
    Argument arg;
    argumentInit(&arg, name, description);
    return arg;
}

int argumentIndexOf(Argument **args, usize argsSize, cstring name) {
    if (!args || !name) {
        return -1;
    }

    for (usize i = 0; i < argsSize; ++i) {
        Argument *arg = args[i];

        if (arg && name && cstringEquals(arg->name, name)) {
            return i;
        }
    }

    return -1;
}

Argument *argumentFind(Argument **args, usize argsSize, cstring name) {
    int index = argumentIndexOf(args, argsSize, name);
    return index == -1 ? NULL : args[index];
}

void argumentPrintTag(Argument *arg) {
    assert(arg != NULL);
    assert(arg->name != NULL);
    boolean optional = argumentIsOptional(arg);
    boolean multi    = argumentIsArray(arg);
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

void argumentPrint(Argument *arg, uint width) {
    assert(arg != NULL);

    pcolorf(GREEN, "  %-*s", width + 2, arg->name);
    printf("%s", arg->description);

    if (argumentIsOptional(arg) && arg->value) {
        pcolorf(YELLOW, " [default: %s]", arg->value);
    }

    printf("\n");
}

void argumentPrintAll(Argument **args, usize argsCount, uint width) {
    assert(args != NULL);
    for (usize i = 0; i < argsCount; ++i) {
        argumentPrint(args[i], width);
    }
}

uint argumentGetPrintWidth(Argument **args, usize argsCount) {
    assert(args != NULL);
    uint width = 0;
    for (usize i = 0; i < argsCount; ++i) {
        width = max(width, strlen(args[i]->name));
    }
    return width;
}

void argumentValidateOrder(Argument **args, usize argsCount) {
    if (argsCount > 0) {
        assert(args != NULL);
    }
    
    int state = ARGUMENT_REQUIRED;

    for (usize i = 0; i < argsCount; ++i) {
        Argument *arg = args[i];

        // array type argument that is not last
        if (argumentIsArray(arg)) {
            assert(i + 1 >= argsCount);
        }

        // optional argument that is not last
        if (argumentIsOptional(arg)) {
            assert(i + 1 >= argsCount);
        }

        // ensure required args follow required args,
        // or an optional arg follows a required arg
        assert(arg->flags >= state);

        state = max(state, arg->flags);
    }
}

boolean argumentIsRequired(Argument *arg) {
    assert(arg != NULL);
    return arg->flags & ARGUMENT_REQUIRED;
}

boolean argumentIsOptional(Argument *arg) {
    return !argumentIsRequired(arg);
}

boolean argumentIsArray(Argument *arg) {
    assert(arg != NULL);
    return arg->flags & ARGUMENT_ARRAY;
}

void argumentMakeRequired(Argument *arg) {
    if (arg) {
        argumentSetFlag(arg, ARGUMENT_REQUIRED);
    }
}

void argumentSetDefaultValue(Argument *arg, string defaultValue) {
    if (arg) {
        arg->value = defaultValue;
    }
}

void argumentMakeArray(Argument *arg, usize min) {
    if (arg) {
        argumentSetFlag(arg, ARGUMENT_ARRAY);
        argumentAddFlags(arg, min > 0 ? ARGUMENT_REQUIRED : ARGUMENT_OPTIONAL);
        arg->minCount = min;
    }
}

#endif // ARGUMENT_H
