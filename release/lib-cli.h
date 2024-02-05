#ifndef CLI_H
#define CLI_H

#include <lib-command.h>
#include <lib-program.h>

#ifndef CLIDEF
# define CLIDEF
#endif // CLIDEF

// ==== Definitions ===========================================================/
int        runApplication(Program *program, int argc, string argv[]);
int        runCommand(Command *cmd, int argc, string argv[]);

#endif // CLI_H
