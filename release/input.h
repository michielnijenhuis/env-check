#ifndef INPUT_H
#define INPUT_H

#include <argument.h>
#include <command.h>
#include <option.h>

typedef struct InputParser {
    Option     **optv;
    size_t       optc;
    Argument   **argv;
    size_t       argc;
    unsigned int offset;
    char        *errbuf;
    unsigned int parsed;
} InputParser;

void input_parse(InputParser *parser, int argc, char **argv);
InputParser
       input_parser_create(Argument **argv, size_t argc, Option **optv, size_t optc, char *errbuf, unsigned int offset);
char  *get_arg(Command *cmd, const char *name);
bool   get_bool_opt(Command *cmd, const char *name);
char  *get_string_opt(Command *cmd, const char *name);
char **get_string_array_opt(Command *cmd, const char *name);

#endif // INPUT_H
