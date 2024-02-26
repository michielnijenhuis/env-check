#ifndef COLORS_H
#define COLORS_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define COLOR(color)        (can_use_ansi() ? color : NO_COLOUR)

#define __WHITE             "\033[0;37m"
#define __WHITE_BOLD        "\033[1;37m"
#define __DARK_GRAY         "\033[1;30m"
#define __YELLOW            "\033[0;33m"
#define __YELLOW_BOLD       "\033[1;33m"
#define __ORANGE            "\033[38;5;214m"
#define __ORANGE_BOLD_LIGHT "\033[38;5;223m"
#define __CYAN              "\033[0;36m"
#define __BLUE              "\033[38;5;27m"
#define __GREEN             "\033[0;32m"
#define __GREEN_BRIGHT      "\033[38;5;10m"
#define __GREEN_LIGHT       "\033[38;5;82m"
#define __GREEN_OLIVE       "\033[38;5;58m"
#define __GREEN_LIME        "\033[38;5;154m"
#define __GREEN_DARK        "\033[38;5;22m"
#define __EMERALD           "\033[38;5;48m"
#define __RED               "\033[0;31m"
#define __RED_BOLD          "\033[1;31m"
#define __RED_BOLD_LIGHT    "\033[38;5;198m"
#define __RED_BG_WHITE_TEXT "\033[41;37m"
#define __MAGENTA           "\033[0;35m"
#define __MAGENTA_LIGHT     "\033[1;95m"

#define NO_COLOUR           "\033[0m"
#define WHITE               COLOR(__WHITE)
#define WHITE_BOLD          COLOR(__WHITE_BOLD)
#define DARK_GRAY           COLOR(__DARK_GRAY)
#define YELLOW              COLOR(__YELLOW)
#define YELLOW_BOLD         COLOR(__YELLOW_BOLD)
#define ORANGE              COLOR(__ORANGE)
#define ORANGE_BOLD_LIGHT   COLOR(__ORANGE_BOLD_LIGHT)
#define CYAN                COLOR(__CYAN)
#define BLUE                COLOR(__BLUE)
#define GREEN               COLOR(__GREEN)
#define GREEN_BRIGHT        COLOR(__GREEN_BRIGHT)
#define GREEN_LIGHT         COLOR(__GREEN_LIGHT)
#define GREEN_OLIVE         COLOR(__GREEN_OLIVE)
#define GREEN_LIME          COLOR(__GREEN_LIME)
#define GREEN_DARK          COLOR(__GREEN_DARK)
#define EMERALD             COLOR(__EMERALD)
#define RED                 COLOR(__RED)
#define RED_BOLD            COLOR(__RED_BOLD)
#define RED_BOLD_LIGHT      COLOR(__RED_BOLD_LIGHT)
#define RED_BG_WHITE_TEXT   COLOR(__RED_BG_WHITE_TEXT)
#define MAGENTA             COLOR(__MAGENTA)
#define MAGENTA_LIGHT       COLOR(__MAGENTA_LIGHT)

static bool ansi_enabled             = true;
static bool ansi_mode_set            = false;
static bool terminal_check_cache     = false;
static bool terminal_check_cache_set = false;

void        enable_ansi(void);
void        disable_ansi(void);
bool        can_use_ansi(void);

#endif // COLORS_H
