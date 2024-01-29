#ifndef OUTPUT_H
#define OUTPUT_H

#include "colors.h"

#include <stdarg.h>
#include <assert.h>

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
void            printWarning(cstring format, ...);
void            printInfo(cstring format, ...);
void            printDebug(cstring format, ...);
void            printNewLine(void);
boolean         canPrint(LogLevel logLevel);

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
                printf(ERROR_COLOR ERROR_PREFIX);
                break;
            }
            case LOG_LEVEL_WARNING: {
                printf(WARNING_COLOR WARNING_PREFIX);
                break;
            }
            case LOG_LEVEL_INFO: {
                printf(INFO_COLOR INFO_PREFIX);
                break;
            }
            case LOG_LEVEL_DEBUG: {
                printf(DEBUG_COLOR DEBUG_PREFIX);
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

void printError(cstring format, ...) {
    assert(format != NULL);

    if (!canPrint(LOG_LEVEL_ERROR)) {
        return;
    }

    printf(ERROR_COLOR ERROR_PREFIX);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(NO_COLOUR "\n");
}

void printWarning(cstring format, ...) {
    assert(format != NULL);

    if (!canPrint(LOG_LEVEL_WARNING)) {
        return;
    }

    printf(WARNING_COLOR WARNING_PREFIX);
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

    printf(INFO_COLOR INFO_PREFIX);
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

    printf(DEBUG_COLOR DEBUG_PREFIX);
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
