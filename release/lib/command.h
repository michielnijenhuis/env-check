#ifndef COMMAND_H
#define COMMAND_H

#include "math.h"
#include "types.h"
#include "argument.h"
#include "option.h"

#include <assert.h>

// ==== Typedefs ==============================================================/
typedef struct Command Command;

typedef int(Operation)(Command *self);

typedef struct Command {
    cstring    name;
    cstring    description;
    cstring    additionalInfo;
    Operation *operation;
    Option   **opts;
    Argument **args;
    usize      optsCount;
    usize      argsCount;
    Option   **__defaultOpts;
    usize      __defaultOptsCount;
} Command;

// ==== Definitions ===========================================================/
void     commandInit(Command *cmd, cstring name, cstring description, Operation *op);
void     commandAddAdditionalInfo(Command *cmd, cstring additionalInfo);
Command  commandCreate(cstring name, cstring description, Operation *op);
int      commandIndexOf(Command **cmds, usize commandsSize, cstring name);
Command *commandFind(Command **cmds, usize commandsCount, cstring name);
void     commandPrint(Command *cmd, uint width);
void     commandPrintAll(Command **commands, usize commandsCount, uint width);
uint     commandGetPrintWidth(Command **commands, usize commandsCount);

// ==== Implementations =======================================================/
void commandInit(Command *cmd, cstring name, cstring description, Operation *op) {
    assert(cmd != NULL);

    cmd->name           = name;
    cmd->description    = description;
    cmd->additionalInfo = NULL;
    cmd->operation      = op;
    cmd->opts           = NULL;
    cmd->optsCount      = 0;
    cmd->args           = NULL;
    cmd->argsCount      = 0;
}

Command commandCreate(cstring name, cstring description, Operation *op) {
    Command cmd;
    commandInit(&cmd, name, description, op);
    return cmd;
}

void commandAddAdditionalInfo(Command *cmd, cstring additionalInfo) {
    if (cmd) {
        cmd->additionalInfo = additionalInfo;
    }
}

int commandIndexOf(Command **cmds, usize commandsCount, cstring name) {
    if (!cmds || !name) {
        return -1;
    }

    for (usize i = 0; i < commandsCount; ++i) {
        const Command *cmd = cmds[i];

        if (cmd && cstringEquals(cmd->name, name)) {
            return i;
        }
    }

    return -1;
}

Command *commandFind(Command **cmds, usize commandsCount, cstring name) {
    int index = commandIndexOf(cmds, commandsCount, name);
    return index == -1 ? NULL : cmds[index];
}

void commandPrint(Command *cmd, uint width) {
    assert(cmd != NULL);
    printf("  " GREEN "%-*s" NO_COLOUR "%s\n", width + 2, cmd->name, cmd->description);
}

void commandPrintAll(Command **commands, usize commandsCount, uint width) {
    assert(commands != NULL);
    for (usize i = 0; i < commandsCount; ++i) {
        commandPrint(commands[i], width);
    }
}

uint commandGetPrintWidth(Command **commands, usize commandsCount) {
    assert(commands != NULL);
    uint width = 0;
    for (usize i = 0; i < commandsCount; ++i) {
        width = max(width, strlen(commands[i]->name));
    }
    return width;
}

#endif // COMMAND_H
