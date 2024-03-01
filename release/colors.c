#include <colors.h>

void enable_ansi(void) {
    ansi_enabled = true;
    ansi_mode_set = true;
}

void disable_ansi(void) {
    ansi_enabled = false;
    ansi_mode_set = true;
}

bool can_use_ansi(void) {
    if (ansi_mode_set) {
        return ansi_enabled;
    }

    if (!terminal_check_cache_set) {
        terminal_check_cache = isatty(fileno(stdout));
    }

    return terminal_check_cache;
}
