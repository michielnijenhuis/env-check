#ifndef COMMAND_H
#define COMMAND_H

#include <lib-argument.h>
#include <lib-option.h>
#include <lib-types.h>

#include <assert.h>

// ==== Typedefs ==============================================================/
typedef struct Command Command;

typedef int(Operation)(Command *self);

typedef struct Command
{
    cstring name;
    cstring description;
    cstring additionalInfo;
    cstring *aliases;
    usize aliasesCount;
    Operation *operation;
    Option **opts;
    Argument **args;
    usize optsCount;
    usize argsCount;
    Option **__defaultOpts;
    usize __defaultOptsCount;
} Command;

// ==== Definitions ===========================================================/
void commandInit(Command *cmd, cstring name, cstring description, Operation *op);
void commandAddAdditionalInfo(Command *cmd, cstring additionalInfo);
void commandSetArgs(Command *cmd, Argument **args, usize argsCount);
void commandSetOpts(Command *cmd, Option **opts, usize optsCount);
void commandSetAliases(Command *cmd, cstring *aliases, usize aliasesCount);
Command commandCreate(cstring name, cstring description, Operation *op);
int commandIndexOf(Command **cmds, usize commandsSize, cstring name);
Command *commandFind(Command **cmds, usize commandsCount, cstring name);
uint commandFindLoose(Command **commands, usize commandsCount, cstring input, cstring *buffer, usize bufferSize);
void commandPrint(Command *cmd, uint width);
void commandPrintAll(Command **commands, usize commandsCount, uint width);
uint commandGetPrintWidth(Command **commands, usize commandsCount);

#endif // COMMAND_H
