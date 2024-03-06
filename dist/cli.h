#ifndef CLI_H
#define CLI_H

#include <command.h>
#include <program.h>

#ifndef CLIDEF
# define CLIDEF
#endif // CLIDEF

int run_application(program_t *program, int argc, char **argv);
int run_command(command_t *cmd, int argc, char **argv);

#endif // CLI_H
