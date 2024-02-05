#ifndef PROGRAM_H
#define PROGRAM_H

#include <lib-command.h>
#include <lib-types.h>

#include <assert.h>

// ==== Typedefs ==============================================================/
typedef struct Program
{
    cstring name;
    cstring version;
    Command **commands;
    usize commandsCount;
    Option **opts;
    usize optsCount;
    Argument **args;
    usize argsCount;
    cstring art;
} Program;

// ==== Definitions ===========================================================/
void programInit(Program *program, cstring name, cstring version);
Program programCreate(cstring name, cstring version);
void programSetSubcommands(Program *program, Command **commands, usize commandsCount);
void programSetOpts(Program *program, Option **opts, usize optsCount);
void programSetArgs(Program *program, Argument **args, usize argsCount);
void programSetAsciiArt(Program *program, cstring art);

#endif // PROGRAM_H
