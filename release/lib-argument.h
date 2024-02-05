#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <lib-argument.h>
#include <lib-math.h>
#include <lib-types.h>
#include <lib-utils.h>
#include <lib-cstring.h>

#include <assert.h>

// ==== Typedefs ==============================================================/
typedef enum ArgumentFlag
{
    ARGUMENT_REQUIRED = 1 << 0,
    ARGUMENT_OPTIONAL = 1 << 1,
    ARGUMENT_ARRAY = 1 << 2,
} ArgumentFlag;

typedef struct Argument
{
    cstring name;
    cstring description;
    string value;
    string *values;
    usize valuesCount;
    usize minCount;
    int flags;
} Argument;

// ==== Definitions ===========================================================/
void argumentInit(Argument *arg, cstring name, cstring description);
void argumentSetFlag(Argument *arg, int flag);
void argumentAddFlags(Argument *arg, int flags);
void argumentMakeRequired(Argument *arg);
void argumentMakeArray(Argument *arg, usize min);
void argumentSetDefaultValue(Argument *arg, string defaultValue);
Argument argumentCreate(cstring name, cstring description);
string argumentGet(Argument **args, usize argsCount, cstring name);
int argumentIndexOf(Argument **args, usize argsSize, cstring name);
Argument *argumentFind(Argument **args, usize argsSize, cstring name);
void argumentPrintTag(Argument *arg);
void argumentPrint(Argument *arg, uint width);
void argumentPrintAll(Argument **args, usize argsCount, uint width);
uint argumentGetPrintWidth(Argument **args, usize argsCount);
void argumentValidateOrder(Argument **args, usize argsCount);
boolean argumentIsRequired(Argument *arg);
boolean argumentIsOptional(Argument *arg);
boolean argumentIsArray(Argument *arg);

#endif // ARGUMENT_H
