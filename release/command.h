#ifndef COMMAND_H
#define COMMAND_H

#include <argument.h>
#include <option.h>

typedef struct Command Command;

typedef int(Operation)(Command *cmd);

typedef int(CommandHandler)(Command *cmd, int argc, char **argv);

typedef struct Command {
    const char     *name;
    const char     *desc;
    const char     *info;
    const char    **aliasv;
    size_t          aliasc;
    Operation      *operation;
    Option        **optv;
    size_t          optc;
    Argument      **argv;
    size_t          argc;
    Option        **__default_optv;
    size_t          __default_optc;
    CommandHandler *handler;
} Command;

Command  command_create(const char *name, const char *desc, Operation *op);
Command  command_define(const char *name, const char *desc, CommandHandler *handler, Operation *op);
void     command_init(Command *cmd, const char *name, const char *desc, Operation *op);
void     command_add_info(Command *cmd, const char *info);
void     command_set_args(Command *cmd, Argument **argv, size_t argc);
void     command_set_opts(Command *cmd, Option **optv, size_t optc);
void     command_set_aliases(Command *cmd, const char **aliasv, size_t aliasc);
int      command_index_of(Command **cmdv, size_t cmdc, const char *name);
Command *command_find(Command **cmdv, size_t cmdc, const char *name);
int      command_find_loose(Command **cmdv, size_t cmdc, const char *input, const char **buf, size_t bufsize);
void     command_print(Command *cmd, int width);
void     command_print_all(Command **cmdv, size_t cmdc, int width);
int      command_get_print_width(Command **cmdv, size_t cmdc);

#endif // COMMAND_H
