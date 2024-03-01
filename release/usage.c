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

int usage(Program *program, bool version_only) {
    if (program->art) {
        printf("%s\n", program->art);
    }

    pcolorf(GREEN, "%s", program->name);
    if (program->version != NULL) {
        printf(" version %s%s%s", COLOR(YELLOW), program->version, NO_COLOUR);
    }
    printf("\n");

    if (version_only) {
        return 1;
    }

    printf("\n");
    pcolor(YELLOW, "Usage:");
    printf("  command [options] [arguments]\n");

    int width = max(option_get_print_width(program->optv, program->optc),
                    command_get_print_width(program->cmdv, program->cmdc));

    printf("\n");
    pcolor(YELLOW, "Options:");
    option_print_all(program->optv, program->optc, width);

    printf("\n");
    pcolor(YELLOW, "Available commands:");
    command_print_all(program->cmdv, program->cmdc, width);

    return 1;
}

int usage_cmd(Command *cmd) {
    pcolor(YELLOW, "Description:");
    printf("  %s\n", cmd->desc);

    printf("\n");
    pcolor(YELLOW, "Usage:");
    printf("  ");
    printf("%s [options]", cmd->name);

    if (cmd->argc > 0) {
        printf(" [--] ");

        bool has_required_args = argument_is_required(cmd->argv[0]);

        if (!has_required_args) {
            printf("[");
        }

        for (size_t i = 0; i < cmd->argc; ++i) {
            Argument *arg = cmd->argv[i];
            assert(arg != NULL);
            assert(arg->name != NULL);

            bool required = argument_is_required(arg);
            bool multi    = argument_is_array(arg);

            if (required && multi && arg->min_count > 1) {
                for (size_t j = 0; j < arg->min_count - 1; ++j) {
                    printf("<%s> ", arg->name);
                }
            }

            printf("%s<%s>%s", !required && i > 0 ? "[" : "", cmd->argv[i]->name, !required && i > 0 ? "]" : "");

            if (multi) {
                printf("...");
            }
        }

        if (!has_required_args) {
            printf("]");
        }
    }
    printf("\n");

    if (cmd->aliasv) {
        for (size_t i = 0; i < cmd->aliasc; ++i) {
            printf("  %s\n", cmd->aliasv[i]);
        }
    }

    int optwidth = option_get_print_width(cmd->optv, cmd->optc);
    int argwidth = argument_get_print_width(cmd->argv, cmd->argc);
    int width    = max(optwidth, argwidth);

    printf("\n");
    pcolor(YELLOW, "Options:");
    if (cmd->optc > 0) {
        option_print_all(cmd->optv, cmd->optc, width);
    }

    if (cmd->argv != NULL) {
        printf("\n");
        pcolor(YELLOW, "Arguments:");
        argument_print_all(cmd->argv, cmd->argc, width);
    }

    if (cmd->info) {
        printf("\n");
        pcolor(YELLOW, "Help:");
        printf("  %s\n", cmd->info);
    }

    return 1;
}

int usage_string_cmd(const char *name, Option **optv, size_t optc, Argument **argv, size_t argc) {
    if (should_be_quiet()) {
        return 1;
    }

    printf(COLOR(GREEN));
    printf("%s", name);

    for (size_t i = 0; i < optc; ++i) {
        option_print_tag(optv[i]);
    }

    if (argc > 0) {
        printf(" [--]");
        for (size_t i = 0; i < argc; ++i) {
            argument_print_tag(argv[i]);
        }
    }

    printf("%s\n", NO_COLOUR);

    return 1;
}