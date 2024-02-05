#ifndef INPUT_H
#define INPUT_H

#include <lib-argument.h>
#include <lib-command.h>
#include <lib-option.h>
#include <lib-types.h>

#include <assert.h>

#define ARG_PARSER_ERR_SIZE 1024

// ==== Typedefs ==============================================================/
typedef struct InputParser {
    Option   **options;
    usize      optionsc;
    Argument **arguments;
    usize      argumentsc;
    uint       offset;
    string     errBuffer;
    uint       parsed;
} InputParser;

// ==== Definitions ===========================================================/
void        inputParse(InputParser *parser, int argc, string argv[]);
InputParser inputParserCreate(Argument **args, usize argsc, Option **opts, usize optsc, string errBuffer, usize offset);
string      getArg(Command *cmd, cstring name);
boolean     getBoolOpt(Command *cmd, cstring name);
string      getStringOpt(Command *cmd, cstring name);
string     *getStringArrayOpt(Command *cmd, cstring name);

#endif // INPUT_H
