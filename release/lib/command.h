#ifndef COMMAND_H
#define COMMAND_H

#include "argument.h"
#include "math.h"
#include "option.h"
#include "types.h"

#include <assert.h>

// ==== Typedefs ==============================================================/
typedef struct Command Command;

typedef int(Operation)(Command *self);

typedef struct Command {
    cstring    name;
    cstring    description;
    cstring    additionalInfo;
    cstring   *aliases;
    usize      aliasesCount;
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
void     commandSetArgs(Command *cmd, Argument **args, usize argsCount);
void     commandSetOpts(Command *cmd, Option **opts, usize optsCount);
void     commandSetAliases(Command *cmd, cstring *aliases, usize aliasesCount);
Command  commandCreate(cstring name, cstring description, Operation *op);
int      commandIndexOf(Command **cmds, usize commandsSize, cstring name);
Command *commandFind(Command **cmds, usize commandsCount, cstring name);
void     commandPrint(Command *cmd, uint width);
void     commandPrintAll(Command **commands, usize commandsCount, uint width);
uint     commandGetPrintWidth(Command **commands, usize commandsCount);

// ==== Implementations =======================================================/
void commandInit(Command *cmd, cstring name, cstring description, Operation *op) {
    assert(cmd != NULL);
    assert(name != NULL);
    assert(op != NULL);

    cmd->name           = name;
    cmd->description    = description;
    cmd->additionalInfo = NULL;
    cmd->aliases        = NULL;
    cmd->aliasesCount   = 0;
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

void commandSetArgs(Command *cmd, Argument **args, usize argsCount) {
    if (cmd) {
        cmd->args      = args;
        cmd->argsCount = argsCount;
    }
}

void commandSetOpts(Command *cmd, Option **opts, usize optsCount) {
    if (cmd) {
        cmd->opts      = opts;
        cmd->optsCount = optsCount;
    }
}

int commandIndexOf(Command **cmds, usize commandsCount, cstring name) {
    if (!cmds || !name) {
        return -1;
    }

    for (usize i = 0; i < commandsCount; ++i) {
        const Command *cmd = cmds[i];
        assert(cmd != NULL);

        if (cstringEquals(cmd->name, name)) {
            return i;
        }

        if (cmd->aliases) {
            for (usize j = 0; j < cmd->aliasesCount; ++j) {
                if (cstringEquals(cmd->aliases[j], name)) {
                    return i;
                }
            }
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
    pcolorf(GREEN, "  %-*s", width + 2, cmd->name);

    if (cmd->aliases) {
        printf("[");
        for (usize i = 0; i < cmd->aliasesCount; ++i) {
            printf("%s", cmd->aliases[i]);
            if (i + 1 < cmd->aliasesCount) {
                printf("|");
            }
        }
        printf("] ");
    }

    printf("%s\n", cmd->description);
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

void commandSetAliases(Command *cmd, cstring *aliases, usize aliasesCount) {
    if (cmd) {
        cmd->aliases      = aliases;
        cmd->aliasesCount = aliasesCount;
    }
}

#endif // COMMAND_H
