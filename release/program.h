#ifndef PROGRAM_H
#define PROGRAM_H

#include <command.h>

typedef struct Program {
    const char *name;
    const char *version;
    Command   **cmdv;
    size_t      cmdc;
    Option    **optv;
    size_t      optc;
    Argument  **argv;
    size_t      argc;
    const char *art;
} Program;

void    program_init(Program *program, const char *name, const char *version);
Program program_create(const char *name, const char *version);
void    program_set_subcommands(Program *program, Command **cmdv, size_t cmdc);
void    program_set_opts(Program *program, Option **optv, size_t optc);
void    program_set_args(Program *program, Argument **argv, size_t argc);
void    program_set_ascii_art(Program *program, const char *art);

#endif // PROGRAM_H
