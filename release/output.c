#include <assert.h>
#include <math-utils.h>
#include <output.h>
#include <stdlib.h>
#include <string.h>

static void vlog(FILE *stream, const char *color, const char *tag, const char *fmt, va_list args);
static void vlog_padded(FILE *stream, const char *color, const char *msg);

static void vlog(FILE *stream, const char *color, const char *tag, const char *fmt, va_list args) {
    fprintf(stream, "%s%s%s", color, tag, NO_COLOUR);
    vfprintf(stream, fmt, args);
    fprintf(stream, "\n");
}

void print(const char *fmt, ...) {
    assert(fmt != NULL);
    if (should_be_quiet()) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void loginfo(const char *fmt, ...) {
    assert(fmt != NULL);
    if (!can_print(LOG_LEVEL_INFO)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vlog(stderr, INFO_LOG_COLOR, INFO_PREFIX, fmt, args);
    va_end(args);
}

void logwarn(const char *restrict fmt, ...) {
    assert(fmt != NULL);
    if (!can_print(LOG_LEVEL_WARNING)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vlog(stderr, WARNING_LOG_COLOR, WARNING_PREFIX, fmt, args);
    va_end(args);
}

void logerro(const char *fmt, ...) {
    assert(fmt != NULL);
    if (!can_print(LOG_LEVEL_ERROR)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vlog(stderr, ERROR_LOG_COLOR, ERROR_PREFIX, fmt, args);
    va_end(args);
}

void logdebug(const char *restrict fmt, ...) {
    assert(fmt != NULL);
    if (!can_print(LOG_LEVEL_DEBUG)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    vlog(stderr, DEBUG_LOG_COLOR, DEBUG_PREFIX, fmt, args);
    va_end(args);
}

void info(const char *msg) {
    if (!msg || !can_print(LOG_LEVEL_INFO)) {
        return;
    }
    vlog_padded(stderr, INFO_COLOR, msg);
}

void erro(const char *msg) {
    if (!msg || !can_print(LOG_LEVEL_ERROR)) {
        return;
    }
    vlog_padded(stderr, ERROR_COLOR, msg);
}

void warn(const char *msg) {
    if (!msg || !can_print(LOG_LEVEL_WARNING)) {
        return;
    }
    vlog_padded(stderr, WARNING_COLOR, msg);
}

void debug(const char *msg) {
    if (!msg || !can_print(LOG_LEVEL_DEBUG)) {
        return;
    }
    vlog_padded(stderr, DEBUG_COLOR, msg);
}

void panic(const char *msg) {
    if (!msg || !can_print(LOG_LEVEL_ERROR)) {
        return;
    }
    vlog_padded(stderr, ERROR_COLOR, msg);
    exit(1);
}

void infof(const char *fmt, ...) {
    if (!fmt || !can_print(LOG_LEVEL_INFO)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    char err[1024];
    vsprintf(err, fmt, args);
    vlog_padded(stderr, INFO_COLOR, err);
    va_end(args);
}

void errof(const char *fmt, ...) {
    assert(fmt != NULL);
    if (!can_print(LOG_LEVEL_ERROR)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    char err[1024];
    vsprintf(err, fmt, args);
    vlog_padded(stderr, ERROR_COLOR, err);
    va_end(args);
}

void warnf(const char *fmt, ...) {
    assert(fmt != NULL);
    if (!can_print(LOG_LEVEL_WARNING)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    char err[1024];
    vsprintf(err, fmt, args);
    vlog_padded(stderr, WARNING_COLOR, err);
    va_end(args);
}

void debugf(const char *fmt, ...) {
    assert(fmt != NULL);
    if (!can_print(LOG_LEVEL_DEBUG)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    char err[1024];
    vsprintf(err, fmt, args);
    vlog_padded(stderr, DEBUG_COLOR, err);
    va_end(args);
}

void panicf(const char *fmt, ...) {
    assert(fmt != NULL);
    if (!can_print(LOG_LEVEL_ERROR)) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    char err[1024];
    vsprintf(err, fmt, args);
    vlog_padded(stderr, ERROR_COLOR, err);
    va_end(args);
    exit(1);
}

void set_log_level(LogLevel level) {
    LOG_LEVEL = level;
}

void writeln(const char *format, ...) {
    assert(format != NULL);
    if (!can_print(LOG_LEVEL_ERROR)) {
        return;
    }
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    new_line();
}

static void vlog_padded(FILE *stream, const char *color, const char *msg) {
    if (should_be_quiet()) {
        return;
    }

    char *copy = strdup(msg);
    assert(copy != NULL);

    int          padding      = PADDING;
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

    new_line();
    int width = max_line_len + padding + color_len;
    fprintf(stream, "%s", color);
    fprintf(stream, "%-*.*s%s\n", width, width, color, NO_COLOUR);

    free(copy);
    copy = strdup(msg);
    assert(copy != NULL);
    token = strtok(copy, "\n");
    while (token != NULL) {
        fprintf(stream,
                "%s%-*s%-*.*s%s\n",
                color,
                padding / 2,
                "",
                (int) max_line_len + (padding / 2),
                width,
                token,
                NO_COLOUR);
        token = strtok(NULL, "\n");
    }
    free(copy);
    copy = NULL;

    fprintf(stream, "%-*.*s%s\n", width, width, color, NO_COLOUR);
    fprintf(stream, NO_COLOUR);
}

void new_line(void) {
    if (!should_be_quiet()) {
        printf("\n");
    }
}

bool can_print(LogLevel logLevel) {
    return logLevel <= LOG_LEVEL;
}

bool should_be_quiet(void) {
    return LOG_LEVEL == LOG_LEVEL_QUIET;
}
