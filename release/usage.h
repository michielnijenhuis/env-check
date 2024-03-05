#include <argument.h>
#include <command.h>
#include <option.h>
#include <program.h>
#include <stdbool.h>
#include <stddef.h>

int usage(program_t *program, bool version_only);
int usage_cmd(command_t *cmd);
int usage_string_cmd(const char *name, option_t **optv, size_t optc, argument_t **argv, size_t argc);