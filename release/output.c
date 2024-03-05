#include "output.h"

#include <assert.h>
#include <colors.h>
#include <math-utils.h>
#include <output.h>
#include <stdlib.h>
#include <string.h>

// TODO: wrap lines -> print up to max line length

static const char     *error_prefix   = "[ERROR] ";
static const char     *error_color    = __RED_BG_WHITE_TEXT;
static const char     *warning_prefix = "[WARNING] ";
static const char     *warning_color  = __BG_YELLOW_BLACK_TEXT;
static const char     *info_prefix    = "[INFO] ";
static const char     *info_color     = __BG_BLUE_WHITE_TEXT;
static const char     *success_prefix = "[OK] ";
static const char     *success_color  = __BG_GREEN_WHITE_TEXT;
static const char     *debug_prefix   = NULL;
static const char     *debug_color    = NO_COLOR;

static output_style_t  _style;
static output_style_t *style     = &_style;
static log_level_t     log_level = LOG_LEVEL_ERROR;

static FILE           *get_stream(log_level_t level);
static void            set_default_style(void) __attribute__((constructor));
static void            write_padded(FILE *stream, const char *color, const char *prefix, const char *str);
static const char     *get_color(log_level_t level);
static const char     *get_prefix(log_level_t level);

// ----------------------------------------------------------------

static FILE *get_stream(log_level_t level) {
    if (log_level < level) {
        return NULL;
    }

    switch (log_level) {
        case LOG_LEVEL_QUIET:
            return NULL;
        case LOG_LEVEL_ERROR:
        case LOG_LEVEL_WARNING:
        case LOG_LEVEL_CAUTION:
        case LOG_LEVEL_DEBUG:
            return stderr;
        case LOG_LEVEL_SUCCESS:
        case LOG_LEVEL_INFO:
            return stdout;
    }
}

static void set_default_style(void) {
    set_error_prefix(error_prefix);
    set_error_color(error_color);
    set_warning_prefix(warning_prefix);
    set_warning_color(warning_color);
    set_info_prefix(info_prefix);
    set_info_color(info_color);
    set_success_prefix(success_prefix);
    set_success_color(success_color);
    set_debug_prefix(debug_prefix);
    set_debug_color(debug_color);
}

void set_error_prefix(const char *prefix) {
    style->error_prefix = prefix;
}

void set_error_color(const char *color) {
    style->error_color = color;
}

void set_warning_prefix(const char *prefix) {
    style->warning_prefix = prefix;
}

void set_warning_color(const char *color) {
    style->warning_color = color;
}

void set_success_prefix(const char *prefix) {
    style->success_prefix = prefix;
}

void set_success_color(const char *color) {
    style->success_color = color;
}

void set_info_prefix(const char *prefix) {
    style->info_prefix = prefix;
}

void set_info_color(const char *color) {
    style->info_color = color;
}

void set_debug_prefix(const char *prefix) {
    style->debug_prefix = prefix;
}

void set_debug_color(const char *color) {
    style->debug_color = color;
}

void set_output_style(output_style_t *s) {
    style = s;
}

output_style_t *get_output_style(void) {
    return style;
}

static const char *get_color(log_level_t level) {
    output_style_t *style = get_output_style();
    assert(style != NULL);
    switch (level) {
        case LOG_LEVEL_ERROR:
            return style->error_color;
        case LOG_LEVEL_WARNING:
            return style->warning_color;
        case LOG_LEVEL_CAUTION:
            return style->warning_color;
        case LOG_LEVEL_SUCCESS:
            return style->success_color;
        case LOG_LEVEL_INFO:
            return style->info_color;
        case LOG_LEVEL_DEBUG:
            return style->debug_color;
        case LOG_LEVEL_QUIET:
        default:
            return NULL;
    }
}

static const char *get_prefix(log_level_t level) {
    output_style_t *style = get_output_style();
    assert(style != NULL);
    switch (level) {
        case LOG_LEVEL_ERROR:
            return style->error_prefix;
        case LOG_LEVEL_WARNING:
            return style->warning_prefix;
        case LOG_LEVEL_CAUTION:
            return style->warning_prefix;
        case LOG_LEVEL_SUCCESS:
            return style->success_prefix;
        case LOG_LEVEL_INFO:
            return style->info_prefix;
        case LOG_LEVEL_DEBUG:
            return style->debug_prefix;
        case LOG_LEVEL_QUIET:
        default:
            return NULL;
    }
}

#define __log(msg, level)                                                                                              \
    if (!msg) {                                                                                                        \
        return;                                                                                                        \
    }                                                                                                                  \
    FILE *stream = get_stream(level);                                                                                  \
    if (!stream) {                                                                                                     \
        return;                                                                                                        \
    }                                                                                                                  \
    const char *prefix = get_prefix(level);                                                                            \
    const char *color  = get_color(level);                                                                             \
    if (prefix) {                                                                                                      \
        write_padded(stream, color, prefix, msg);                                                                      \
    } else {                                                                                                           \
        fprintf(stream, "%s\n", msg);                                                                                  \
    }

void erro(const char *str) {
    __log(str, LOG_LEVEL_ERROR);
}

void panic(const char *str) {
    erro(str);
    exit(1);
}

void caution(const char *str) {
    __log(str, LOG_LEVEL_CAUTION);
}

void warn(const char *str) {
    __log(str, LOG_LEVEL_WARNING);
}

void info(const char *str) {
    __log(str, LOG_LEVEL_INFO);
}

void success(const char *str) {
    __log(str, LOG_LEVEL_SUCCESS);
}

void debug(const char *str) {
    __log(str, LOG_LEVEL_DEBUG);
}

// TODO: implement
static void wrapln(FILE *stream, const char *str) {
    fprintf(stream, "%s\n", str);
    // size_t len = strlen(str);
    // if (len <= OUTPUT_MAX_LINE_LEN) {
    //     fprintf(stream, "%s\n", str);
    //     return;
    // }
    // ssize_t remaining = len;
    // char *p = (char *) str;
    // while (remaining > 0) {
    //     fprintf(stream, "%.*s\n", OUTPUT_MAX_LINE_LEN, p);
    //     if (remaining > OUTPUT_MAX_LINE_LEN) {
    //         p += OUTPUT_MAX_LINE_LEN;
    //     }
    //     remaining -= OUTPUT_MAX_LINE_LEN;
    // }
}

void writeln(const char *str) {
    if (!str) {
        return;
    }
    FILE *stream = get_stream(LOG_LEVEL_ERROR);
    if (!stream) {
        return;
    }
    wrapln(stream, str);
}

void writelnf(const char *fmt, ...) {
    if (!fmt) {
        return;
    }
    FILE *stream = get_stream(LOG_LEVEL_ERROR);
    if (!stream) {
        return;
    }
    char    buf[1024];
    char   *p = buf;
    va_list args;
    va_start(args, fmt);
    vsprintf(p, fmt, args);
    va_end(args);
    wrapln(stream, p);
}

#define __logf(level, fmt, ...)                                                                                        \
    if (!fmt) {                                                                                                        \
        return;                                                                                                        \
    }                                                                                                                  \
    FILE *stream = get_stream(level);                                                                                  \
    if (!stream) {                                                                                                     \
        return;                                                                                                        \
    }                                                                                                                  \
    const char *prefix = get_prefix(level);                                                                            \
    const char *color  = get_color(level);                                                                             \
    char        buffer[1024];                                                                                          \
    va_list     args;                                                                                                  \
    va_start(args, fmt);                                                                                               \
    vsprintf(buffer, fmt, args);                                                                                       \
    va_end(args);                                                                                                      \
    if (prefix) {                                                                                                      \
        write_padded(stream, color, prefix, buffer);                                                                   \
    } else {                                                                                                           \
        fprintf(stream, "%s\n", buffer);                                                                               \
    }

void errof(const char *fmt, ...) {
    __logf(LOG_LEVEL_ERROR, fmt);
}

void panicf(const char *fmt, ...) {
    __logf(LOG_LEVEL_ERROR, fmt);
    exit(1);
}

void warnf(const char *fmt, ...) {
    __logf(LOG_LEVEL_WARNING, fmt);
}

void cautionf(const char *fmt, ...) {
    __logf(LOG_LEVEL_CAUTION, fmt);
}

void successf(const char *fmt, ...) {
    __logf(LOG_LEVEL_SUCCESS, fmt);
}

void infof(const char *fmt, ...) {
    __logf(LOG_LEVEL_INFO, fmt);
}

void debugf(const char *fmt, ...) {
    __logf(LOG_LEVEL_DEBUG, fmt);
}

void set_log_level(log_level_t level) {
    log_level = level;
}

static void write_padded(FILE *stream, const char *color, const char *prefix, const char *str) {
    if (!stream || !str) {
        return;
    }

    char *copy = strdup(str);
    assert(copy != NULL);

    int          padding      = 4;
    int          prefix_len   = strlen(prefix);
    int          color_len    = strlen(color);
    unsigned int max_line_len = 0;
    unsigned int lines        = 0;
    char        *token;
    token = strtok((char *) copy, "\n");

    while (token != NULL) {
        max_line_len = max(strlen(token), max_line_len);
        ++lines;
        token = strtok(NULL, "\n");
    }

    int width = color_len + prefix_len + padding + max_line_len;
    new_line();
    fprintf(stream, "%s", color);
    fprintf(stream, "%-*.*s%s\n", width, width, color, NO_COLOR);

    free(copy);
    copy = strdup(str);
    assert(copy != NULL);
    token = strtok(copy, "\n");
    while (token != NULL) {
        fprintf(stream,
                "%s%-*s%s%-*.*s%s\n",
                color,
                padding / 2,
                "",
                prefix,
                (int) max_line_len + (padding / 2),
                width,
                token,
                NO_COLOR);
        token = strtok(NULL, "\n");
    }
    free(copy);
    copy = NULL;

    fprintf(stream, "%-*.*s%s\n", width, width, color, NO_COLOR);
    fprintf(stream, NO_COLOR);
}

void new_line(void) {
    FILE *stream = get_stream(LOG_LEVEL_ERROR);
    if (stream) {
        fprintf(stream, "\n");
    }
}

log_level_t get_log_level(void) {
    return log_level;
}

bool is_quiet(void) {
    log_level_t level = get_log_level();
    return level == LOG_LEVEL_QUIET;
}

void writef(const char *fmt, ...) {
    FILE *stream = get_stream(LOG_LEVEL_ERROR);
    if (!stream) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(stream, fmt, args);
    va_end(args);
}