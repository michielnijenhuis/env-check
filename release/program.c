#include <assert.h>
#include <command.h>
#include <program.h>
#include <stddef.h>

void program_init(Program *program, const char *name, const char *version) {
    assert(program != NULL);
    assert(name != NULL);

    program->name    = name;
    program->version = version;
    program->cmdv    = NULL;
    program->cmdc    = 0;
    program->optv    = NULL;
    program->optc    = 0;
    program->argv    = NULL;
    program->argc    = 0;
    program->art     = NULL;
}

Program program_create(const char *name, const char *version) {
    Program program;
    program_init(&program, name, version);
    return program;
}

void program_set_subcommands(Program *program, Command **cmdv, size_t cmdc) {
    if (program) {
        program->cmdv = cmdv;
        program->cmdc = cmdc;
    }
}

void program_set_opts(Program *program, Option **optv, size_t optc) {
    if (program) {
        program->optv = optv;
        program->optc = optc;
    }
}

void program_set_args(Program *program, Argument **argv, size_t argc) {
    if (program) {
        program->argv = argv;
        program->argc = argc;
    }
}

void program_set_ascii_art(Program *program, const char *art) {
    if (program) {
        program->art = art;
    }
}
