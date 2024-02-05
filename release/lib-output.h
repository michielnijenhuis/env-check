#ifndef OUTPUT_H
#define OUTPUT_H

#include <lib-colors.h>

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

#define PADDING                  4

#define fcolor(color, str)       color str NO_COLOUR
#define pcolor(color, str)       printText("%s" str NO_COLOUR, color)
#define pcolorinit(color, str)   printText("%s" str, color)
#define pcolorf(color, fmt, ...) printText("%s" fmt NO_COLOUR, color, __VA_ARGS__)

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
void            printText(cstring format, ...);
void            printError(cstring format, ...);
void            printErrorLarge(cstring format, ...);
void            printWarning(cstring format, ...);
void            printInfo(cstring format, ...);
void            printDebug(cstring format, ...);
void            printNewLine(void);
void            printfPadded(string buffer, cstring color, cstring format, ...);
void            sprintfPadded(string buffer, string strBuffer, cstring color, cstring format, ...);
boolean         canPrint(LogLevel logLevel);
boolean         shouldBeQuiet();

static LogLevel LOG_LEVEL = LOG_LEVEL_ERROR;

#endif // OUTPUT_H
