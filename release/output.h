#ifndef OUTPUT_H
#define OUTPUT_H

#if defined(__GNUC__) || defined(__clang__)
// https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
# define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__((format(printf, STRING_INDEX, FIRST_TO_CHECK)))
#else
# define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

#include <colors.h>
#include <stdarg.h>
#include <string.h>

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

#ifndef ERROR_LOG_COLOR
# define ERROR_LOG_COLOR RED_BOLD
#endif // ERROR_LOG_COLOR

#ifndef INFO_LOG_COLOR
# define INFO_LOG_COLOR NO_COLOUR
#endif // INFO_LOG_COLOR

#ifndef WARNING_LOG_COLOR
# define WARNING_LOG_COLOR ORANGE
#endif // WARNING_LOG_COLOR

#ifndef DEBUG_LOG_COLOR
# define DEBUG_LOG_COLOR CYAN
#endif // DEBUG_LOG_COLOR

#ifndef ERROR_COLOR
# define ERROR_COLOR RED_BG_WHITE_TEXT
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

#define PADDING                  4

#define fcolor(color, str)       color str NO_COLOUR
#define pcolor(color, str)       print("%s" str "\n" NO_COLOUR, color)
#define pcolorinit(color, str)   print("%s" str, color)
#define pcolorf(color, fmt, ...) print("%s" fmt NO_COLOUR, color, __VA_ARGS__)

typedef enum LogLevel {
    LOG_LEVEL_QUIET   = 0,
    LOG_LEVEL_ERROR   = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO    = 3,
    LOG_LEVEL_DEBUG   = 4,
} LogLevel;

void            print(const char *fmt, ...) PRINTF_FORMAT(1, 2);

void            loginfo(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            logwarn(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            logerro(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            logdebug(const char *fmt, ...) PRINTF_FORMAT(1, 2);

void            info(const char *msg);
void            warn(const char *msg);
void            erro(const char *msg);
void            debug(const char *msg);
void            panic(const char *msg);

void            infof(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            warnf(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            errof(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            debugf(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            panicf(const char *fmt, ...) PRINTF_FORMAT(1, 2);

void            set_log_level(LogLevel level);
bool            can_print(LogLevel level);
bool            should_be_quiet(void);
void            writeln(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            new_line(void);

static LogLevel LOG_LEVEL = LOG_LEVEL_ERROR;

#endif // OUTPUT_H
