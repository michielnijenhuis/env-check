#include <assert.h>
#include <command.h>
#include <program.h>
#include <stddef.h>

void program_init(program_t *program, const char *name, const char *version) {
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

program_t program_create(const char *name, const char *version) {
    program_t program;
    program_init(&program, name, version);
    return program;
}

void program_set_subcommands(program_t *program, command_t **cmdv, size_t cmdc) {
    if (program) {
        program->cmdv = cmdv;
        program->cmdc = cmdc;
    }
}

void program_set_opts(program_t *program, option_t **optv, size_t optc) {
    if (program) {
        program->optv = optv;
        program->optc = optc;
    }
}

void program_set_args(program_t *program, argument_t **argv, size_t argc) {
    if (program) {
        program->argv = argv;
        program->argc = argc;
    }
}

void program_set_ascii_art(program_t *program, const char *art) {
    if (program) {
        program->art = art;
    }
}
