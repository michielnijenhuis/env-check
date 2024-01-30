#ifndef PROGRAM_H
#define PROGRAM_H

#include "command.h"
#include "types.h"

#include <assert.h>

// ==== Typedefs ==============================================================/
typedef struct Program {
    cstring    name;
    cstring    version;
    Command  **commands;
    usize      commandsCount;
    Option   **opts;
    usize      optsCount;
    Argument **args;
    usize      argsCount;
    cstring    art;
} Program;

// ==== Definitions ===========================================================/
void    programInit(Program *program, cstring name, cstring version);
Program programCreate(cstring name, cstring version);
void    programSetSubcommands(Program *program, Command **commands, usize commandsCount);
void    programSetOpts(Program *program, Option **opts, usize optsCount);
void    programSetArgs(Program *program, Argument **args, usize argsCount);

// ==== Implementations =======================================================/
void programInit(Program *program, cstring name, cstring version) {
    assert(program != NULL);
    assert(name != NULL);

    program->name          = name;
    program->version       = version;
    program->commands      = NULL;
    program->commandsCount = 0;
    program->opts          = NULL;
    program->optsCount     = 0;
    program->args          = NULL;
    program->argsCount     = 0;
    program->art           = NULL;
}

Program programCreate(cstring name, cstring version) {
    Program program;
    programInit(&program, name, version);
    return program;
}

void programSetSubcommands(Program *program, Command **commands, usize commandsCount) {
    if (program) {
        program->commands      = commands;
        program->commandsCount = commandsCount;
    }
}

void programSetOpts(Program *program, Option **opts, usize optsCount) {
    if (program) {
        program->opts      = opts;
        program->optsCount = optsCount;
    }
}

void programSetArgs(Program *program, Argument **args, usize argsCount) {
    if (program) {
        program->args      = args;
        program->argsCount = argsCount;
    }
}

void programSetAsciiArt(Program *program, cstring art) {
    if (program) {
        program->art = art;
    }
}

#endif // PROGRAM_H
