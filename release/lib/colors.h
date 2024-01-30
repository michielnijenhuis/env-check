#ifndef COLORS_H
#define COLORS_H

#include "types.h"

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#define NO_COLOUR         "\033[0m"
#define WHITE             (canUseAnsi() ? "\033[0;37m" : NO_COLOUR)
#define WHITE_BOLD        (canUseAnsi() ? "\033[1;37m" : NO_COLOUR)
#define DARK_GRAY         (canUseAnsi() ? "\033[1;30m" : NO_COLOUR)
#define YELLOW            (canUseAnsi() ? "\033[0;33m" : NO_COLOUR)
#define YELLOW_BOLD       (canUseAnsi() ? "\033[1;33m" : NO_COLOUR)
#define ORANGE            (canUseAnsi() ? "\033[38;5;214m" : NO_COLOUR)
#define ORANGE_BOLD_LIGHT (canUseAnsi() ? "\033[38;5;223m" : NO_COLOUR)
#define CYAN              (canUseAnsi() ? "\033[0;36m" : NO_COLOUR)
#define BLUE              (canUseAnsi() ? "\033[38;5;27m" : NO_COLOUR)
#define GREEN             (canUseAnsi() ? "\033[0;32m" : NO_COLOUR)
#define GREEN_BRIGHT      (canUseAnsi() ? "\033[38;5;10m" : NO_COLOUR)
#define GREEN_LIGHT       (canUseAnsi() ? "\033[38;5;82m" : NO_COLOUR)
#define GREEN_OLIVE       (canUseAnsi() ? "\033[38;5;58m" : NO_COLOUR)
#define GREEN_LIME        (canUseAnsi() ? "\033[38;5;154m" : NO_COLOUR)
#define GREEN_DARK        (canUseAnsi() ? "\033[38;5;22m" : NO_COLOUR)
#define EMERALD           (canUseAnsi() ? "\033[38;5;48m" : NO_COLOUR)
#define RED               (canUseAnsi() ? "\033[0;31m" : NO_COLOUR)
#define RED_BOLD          (canUseAnsi() ? "\033[1;31m" : NO_COLOUR)
#define RED_BOLD_LIGHT    (canUseAnsi() ? "\033[38;5;198m" : NO_COLOUR)
#define RED_BG_WHITE_TEXT (canUseAnsi() ? "\033[41;37m" : NO_COLOUR)
#define MAGENTA           (canUseAnsi() ? "\033[0;35m" : NO_COLOUR)
#define MAGENTA_LIGHT     (canUseAnsi() ? "\033[1;95m" : NO_COLOUR)

static boolean ansiModeSet = false;
static boolean ansiEnabled = true;

#define COLOR(color)             (canUseAnsi() ? color : NO_COLOUR)
#define fcolor(color, str)       (canUseAnsi() ? (color str NO_COLOUR) : str)
#define pcolor(color, str)       printf("%s" str NO_COLOUR, (canUseAnsi()) ? color : NO_COLOUR)
#define pcolorinit(color, str)   printf("%s" str, (canUseAnsi()) ? color : NO_COLOUR)
#define pcolorf(color, fmt, ...) printf("%s" fmt NO_COLOUR, (canUseAnsi()) ? color : NO_COLOUR, __VA_ARGS__)

void    enableAnsi(void);
void    disableAnsi(void);
boolean canUseAnsi(void);

void    enableAnsi(void) {
    ansiEnabled = true;
    ansiModeSet = true;
}

void disableAnsi(void) {
    ansiEnabled = false;
    ansiModeSet = true;
}

boolean canUseAnsi(void) {
    if (ansiModeSet) {
        return ansiEnabled;
    }

    return isatty(fileno(stdout));
}

#endif // COLORS_H
