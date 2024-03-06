#ifndef PROGRAM_H
#define PROGRAM_H

#include <command.h>

typedef struct program_t {
    const char  *name;
    const char  *version;
    command_t  **cmdv;
    size_t       cmdc;
    option_t   **optv;
    size_t       optc;
    argument_t **argv;
    size_t       argc;
    const char  *art;
} program_t;

void      program_init(program_t *program, const char *name, const char *version);
program_t program_create(const char *name, const char *version);
void      program_set_subcommands(program_t *program, command_t **cmdv, size_t cmdc);
void      program_set_opts(program_t *program, option_t **optv, size_t optc);
void      program_set_args(program_t *program, argument_t **argv, size_t argc);
void      program_set_ascii_art(program_t *program, const char *art);

#endif // PROGRAM_H
