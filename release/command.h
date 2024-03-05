#ifndef COMMAND_H
#define COMMAND_H

#include <argument.h>
#include <option.h>

typedef struct command_t command_t;

typedef int(cmd_op_func)(command_t *cmd);

typedef int(cmd_handler_func)(command_t *cmd, int argc, char **argv);

typedef struct command_t {
    const char       *name;
    const char       *desc;
    const char       *info;
    const char      **aliasv;
    size_t            aliasc;
    cmd_op_func      *operation;
    option_t        **optv;
    size_t            optc;
    argument_t      **argv;
    size_t            argc;
    option_t        **__default_optv;
    size_t            __default_optc;
    cmd_handler_func *handler;
} command_t;

command_t  command_create(const char *name, const char *desc, cmd_op_func *op);
command_t  command_define(const char *name, const char *desc, cmd_handler_func *handler, cmd_op_func *op);
void       command_init(command_t *cmd, const char *name, const char *desc, cmd_op_func *op);
void       command_add_info(command_t *cmd, const char *info);
void       command_set_args(command_t *cmd, argument_t **argv, size_t argc);
void       command_set_opts(command_t *cmd, option_t **optv, size_t optc);
void       command_set_aliases(command_t *cmd, const char **aliasv, size_t aliasc);
int        command_index_of(command_t **cmdv, size_t cmdc, const char *name);
command_t *command_find(command_t **cmdv, size_t cmdc, const char *name);
int        command_find_loose(command_t **cmdv, size_t cmdc, const char *input, const char **buf, size_t bufsize);
void       command_print(command_t *cmd, int width);
void       command_print_all(command_t **cmdv, size_t cmdc, int width);
int        command_get_print_width(command_t **cmdv, size_t cmdc);

#endif // COMMAND_H
