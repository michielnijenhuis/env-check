#ifndef INPUT_H
#define INPUT_H

#include <argument.h>
#include <command.h>
#include <option.h>

typedef struct InputParser {
    option_t   **optv;
    size_t       optc;
    argument_t **argv;
    size_t       argc;
    unsigned int offset;
    char        *errbuf;
    unsigned int parsed;
} input_parser_t;

void input_parse(input_parser_t *parser, int argc, char **argv);
input_parser_t
input_parser_create(argument_t **argv, size_t argc, option_t **optv, size_t optc, char *errbuf, unsigned int offset);
char  *get_arg(command_t *cmd, const char *name);
bool   get_bool_opt(command_t *cmd, const char *name);
char  *get_string_opt(command_t *cmd, const char *name);
char **get_string_array_opt(command_t *cmd, const char *name);

#endif // INPUT_H
