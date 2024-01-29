#ifndef PROGRAM_H
#define PROGRAM_H

#include "types.h"
#include "command.h"

#include <assert.h>

// ==== Typedefs ==============================================================/
typedef struct Program {
    cstring    name;
    cstring    executableName;
    cstring    version;
    Command  **commands;
    usize      commandsCount;
    Option   **opts;
    usize      optsCount;
    Argument **args;
    usize      argsCount;
} Program;

// ==== Definitions ===========================================================/
void    programInit(Program *program, cstring name, cstring executableName, cstring version);
Program programCreate(cstring name, cstring executableName, cstring version);

// ==== Implementations =======================================================/
void programInit(Program *program, cstring name, cstring executableName, cstring version) {
    assert(program != NULL);

    program->name           = name;
    program->executableName = executableName;
    program->version        = version;
    program->commands       = NULL;
    program->commandsCount  = 0;
    program->opts           = NULL;
    program->optsCount      = 0;
    program->args           = NULL;
    program->argsCount      = 0;
}

Program programCreate(cstring name, cstring executableName, cstring version) {
    Program program;
    programInit(&program, name, executableName, version);
    return program;
}

#endif // PROGRAM_H
