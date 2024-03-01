#include <argument.h>
#include <command.h>
#include <option.h>
#include <program.h>
#include <stdbool.h>
#include <stddef.h>

int usage(Program *program, bool version_only);
int usage_cmd(Command *cmd);
int usage_string_cmd(const char *name, Option **optv, size_t optc, Argument **argv, size_t argc);