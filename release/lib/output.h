#ifndef OUTPUT_H
#define OUTPUT_H

#include "colors.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

// ==== Defines ===============================================================/
#ifndef ERROR_PREFIX
# define ERROR_PREFIX "[ERROR] "
#endif // ERROR_PREFIX

#ifndef INFO_PREFIX
# define INFO_PREFIX "[INFO] "
#endif // INFO_PREFIX

#ifndef WARNING_PREFIX
# define WARNING_PREFIX "[WARNING] "
#endif // WARNING_PREFIX

#ifndef DEBUG_PREFIX
# define DEBUG_PREFIX "[DEBUG] "
#endif // DEBUG_PREFIX

#ifndef ERROR_COLOR
# define ERROR_COLOR RED_BOLD
#endif // ERROR_COLOR

#ifndef INFO_COLOR
# define INFO_COLOR NO_COLOUR
#endif // INFO_COLOR

#ifndef WARNING_COLOR
# define WARNING_COLOR ORANGE
#endif // WARNING_COLOR

#ifndef DEBUG_COLOR
# define DEBUG_COLOR CYAN
#endif // DEBUG_COLOR

#ifndef ERROR_BANNER_COLOR
# define ERROR_BANNER_COLOR RED_BG_WHITE_TEXT
#endif // ERROR_BANNER_COLOR

#ifndef ERROR_BUFFER_MAX
# define ERROR_BUFFER_MAX 1024
#endif // ERROR_BUFFER_MAX

// ==== Typedefs ==============================================================/
typedef enum LogLevel {
    LOG_LEVEL_QUIET   = 0,
    LOG_LEVEL_ERROR   = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO    = 3,
    LOG_LEVEL_DEBUG   = 4,
} LogLevel;

// ==== Definitions ===========================================================/
void            setLogLevel(LogLevel level);
void            writeln(cstring format, ...);
void            print(LogLevel logLevel, cstring format, ...);
void            printError(cstring format, ...);
void            printErrorLarge(cstring format, ...);
void            printWarning(cstring format, ...);
void            printInfo(cstring format, ...);
void            printDebug(cstring format, ...);
void            printNewLine(void);
boolean         canPrint(LogLevel logLevel);

static void     ERR(cstring msg);
static LogLevel LOG_LEVEL = LOG_LEVEL_ERROR;

// ==== Implementations =======================================================/

void setLogLevel(LogLevel level) {
    LOG_LEVEL = level;
}

void print(LogLevel logLevel, cstring format, ...) {
    assert(format != NULL);

    if (canPrint(logLevel)) {
        switch (logLevel) {
            case LOG_LEVEL_QUIET: {
                return;
            }
            case LOG_LEVEL_ERROR: {
                printf("%s" ERROR_PREFIX, COLOR(ERROR_COLOR));
                break;
            }
            case LOG_LEVEL_WARNING: {
                printf("%s" WARNING_PREFIX, COLOR(WARNING_COLOR));
                break;
            }
            case LOG_LEVEL_INFO: {
                printf("%s" INFO_PREFIX, COLOR(INFO_COLOR));
                break;
            }
            case LOG_LEVEL_DEBUG: {
                printf("%s" DEBUG_PREFIX, COLOR(DEBUG_COLOR));
                break;
            }
        }

        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);

        printf(NO_COLOUR "\n");
    }
}

void writeln(cstring format, ...) {
    assert(format != NULL);

    if (!canPrint(LOG_LEVEL_ERROR)) {
        return;
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printNewLine();
}

static void ERR(cstring msg) {
    string red = "\033[41;37m";
    // ANSI escape codes for colored text
    printf("\n"
           "%s",
           red); // Set background to red and text to white

    int padding  = 4;
    int colorlen = strlen(red);
    int len      = strlen(msg);
    int width    = len + padding + colorlen;
    // Vertical padding above the printed statement
    printf("%-*.*s%s\n", width, width, red, NO_COLOUR);

    // Padding added to the printed statement
    printf("%s%-*s%-*.*s%s\n", red, padding / 2, "", len + (padding / 2), width, msg, NO_COLOUR);

    // Vertical padding below the printed statement
    printf("%-*.*s%s\n", width, width, red, NO_COLOUR);

    // Reset colors
    printf(NO_COLOUR);
}

void printError(cstring format, ...) {
    assert(format != NULL);

    if (!canPrint(LOG_LEVEL_ERROR)) {
        return;
    }

    printf("%s" ERROR_PREFIX, COLOR(ERROR_COLOR));
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(NO_COLOUR "\n");
}

void printErrorLarge(cstring format, ...) {
    assert(format != NULL);

    if (!canPrint(LOG_LEVEL_ERROR)) {
        return;
    }

    char buffer[ERROR_BUFFER_MAX];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    if (canUseAnsi()) {
        ERR(buffer);
    } else {
        printf("%s\n", buffer);
    }
}

void printWarning(cstring format, ...) {
    assert(format != NULL);

    if (!canPrint(LOG_LEVEL_WARNING)) {
        return;
    }

    printf("%s" WARNING_PREFIX, COLOR(WARNING_COLOR));
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(NO_COLOUR "\n");
}

void printInfo(cstring format, ...) {
    assert(format != NULL);

    if (!canPrint(LOG_LEVEL_INFO)) {
        return;
    }

    printf("%s" INFO_PREFIX, COLOR(INFO_COLOR));
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(NO_COLOUR "\n");
}

void printDebug(cstring format, ...) {
    assert(format != NULL);

    if (!canPrint(LOG_LEVEL_DEBUG)) {
        return;
    }

    printf("%s" DEBUG_PREFIX, COLOR(DEBUG_COLOR));
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(NO_COLOUR "\n");
}

void printNewLine() {
    printf("\n");
}

boolean canPrint(LogLevel logLevel) {
    return logLevel <= LOG_LEVEL;
}

#endif // OUTPUT_H
