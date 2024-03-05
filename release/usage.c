#include <argument.h>
#include <assert.h>
#include <colors.h>
#include <math-utils.h>
#include <option.h>
#include <output.h>
#include <program.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <usage.h>

int usage(program_t *program, bool version_only) {
    if (program->art) {
        writeln(program->art);
    }

    pcolorf(GREEN, "%s", program->name);
    if (program->version != NULL) {
        writef(" version %s%s%s", COLOR(YELLOW), program->version, NO_COLOR);
    }
    new_line();

    if (version_only) {
        return 1;
    }

    new_line();
    pcolorln(YELLOW, "Usage:");
    writeln("  command [options] [arguments]");

    int width = max(option_get_print_width(program->optv, program->optc),
                    command_get_print_width(program->cmdv, program->cmdc));

    new_line();
    pcolorln(YELLOW, "Options:");
    option_print_all(program->optv, program->optc, width);

    new_line();
    pcolorln(YELLOW, "Available commands:");
    command_print_all(program->cmdv, program->cmdc, width);

    return 1;
}

int usage_cmd(command_t *cmd) {
    pcolorln(YELLOW, "Description:");
    writelnf("  %s", cmd->desc);

    new_line();
    pcolorln(YELLOW, "Usage:");
    writef("  ");
    writef("%s [options]", cmd->name);

    if (cmd->argc > 0) {
        writef(" [--] ");

        bool has_required_args = argument_is_required(cmd->argv[0]);

        if (!has_required_args) {
            writef("[");
        }

        for (size_t i = 0; i < cmd->argc; ++i) {
            argument_t *arg = cmd->argv[i];
            assert(arg != NULL);
            assert(arg->name != NULL);

            bool required = argument_is_required(arg);
            bool multi    = argument_is_array(arg);

            if (required && multi && arg->min_count > 1) {
                for (size_t j = 0; j < arg->min_count - 1; ++j) {
                    writef("<%s> ", arg->name);
                }
            }

            writef("%s<%s>%s", !required && i > 0 ? "[" : "", cmd->argv[i]->name, !required && i > 0 ? "]" : "");

            if (multi) {
                writef("...");
            }
        }

        if (!has_required_args) {
            writef("]");
        }
    }
    new_line();

    if (cmd->aliasv) {
        for (size_t i = 0; i < cmd->aliasc; ++i) {
            writelnf("  %s", cmd->aliasv[i]);
        }
    }

    int optwidth = option_get_print_width(cmd->optv, cmd->optc);
    int argwidth = argument_get_print_width(cmd->argv, cmd->argc);
    int width    = max(optwidth, argwidth);

    new_line();
    pcolorln(YELLOW, "Options:");
    if (cmd->optc > 0) {
        option_print_all(cmd->optv, cmd->optc, width);
    }

    if (cmd->argv != NULL) {
        new_line();
        pcolorln(YELLOW, "Arguments:");
        argument_print_all(cmd->argv, cmd->argc, width);
    }

    if (cmd->info) {
        new_line();
        pcolorln(YELLOW, "Help:");
        writelnf("  %s", cmd->info);
    }

    return 1;
}

int usage_string_cmd(const char *name, option_t **optv, size_t optc, argument_t **argv, size_t argc) {
    if (is_quiet()) {
        return 1;
    }

    writef(GREEN);
    writef("%s", name);

    for (size_t i = 0; i < optc; ++i) {
        option_print_tag(optv[i]);
    }

    if (argc > 0) {
        writef(" [--]");
        for (size_t i = 0; i < argc; ++i) {
            argument_print_tag(argv[i]);
        }
    }

    writef("%s\n", NO_COLOR);

    return 1;
}