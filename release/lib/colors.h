#include <stdarg.h>

#define NO_COLOUR         "\033[0m"
#define WHITE             "\033[0;37m"
#define WHITE_BOLD        "\033[1;37m"
#define DARK_GRAY         "\033[1;30m"
#define YELLOW            "\033[0;33m"
#define YELLOW_BOLD       "\033[1;33m"
#define ORANGE            "\033[38;5;214m"
#define ORANGE_BOLD_LIGHT "\033[38;5;223m"
#define CYAN              "\033[0;36m"
#define BLUE              "\033[38;5;27m"
#define GREEN             "\033[0;32m"
#define GREEN_BRIGHT      "\033[38;5;10m"
#define GREEN_LIGHT       "\033[38;5;82m"
#define GREEN_OLIVE       "\033[38;5;58m"
#define GREEN_LIME        "\033[38;5;154m"
#define GREEN_DARK        "\033[38;5;22m"
#define EMERALD           "\033[38;5;48m"
#define RED               "\033[0;31m"
#define RED_BOLD          "\033[1;31m"
#define RED_BOLD_LIGHT    "\033[38;5;198m"
#define MAGENTA           "\033[0;35m"
#define MAGENTA_LIGHT     "\033[1;95m"

#define colorf(color, str) (color str NO_COLOUR)
#define pcolorf(color, fmt, ...) printf(color fmt NO_COLOUR, __VA_ARGS__)
