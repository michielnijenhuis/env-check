#ifndef OUTPUT_H
#define OUTPUT_H

#if defined(__GNUC__) || defined(__clang__)
// https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
# define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) __attribute__((format(printf, STRING_INDEX, FIRST_TO_CHECK)))
#else
# define PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK)
#endif

#include <colors.h>

#define OUTPUT_MAX_LINE_LEN      120

#define fcolor(color, str)          color str NO_COLOR
#define pcolor(color, str)          writef("%s%s%s", color, str, NO_COLOR)
#define pcolorln(color, str)        writelnf("%s%s%s", color, str, NO_COLOR)
#define pcolorf(color, fmt, ...)    writef("%s" fmt NO_COLOR, color, __VA_ARGS__)
#define pcolorlnf(color, fmt, ...)  writelnf("%s" fmt NO_COLOR, color, __VA_ARGS__)

typedef enum LogLevel {
    LOG_LEVEL_QUIET   = 0,
    LOG_LEVEL_ERROR   = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_CAUTION = 3,
    LOG_LEVEL_SUCCESS = 4,
    LOG_LEVEL_INFO    = 5,
    LOG_LEVEL_DEBUG   = 6,
} log_level_t;

typedef struct OutputStyle {
    const char *error_prefix;
    const char *error_color;
    const char *warning_prefix;
    const char *warning_color;
    const char *success_prefix;
    const char *success_color;
    const char *info_prefix;
    const char *info_color;
    const char *debug_prefix;
    const char *debug_color;
} output_style_t;

void            set_output_style(output_style_t *style);
void            set_error_prefix(const char *prefix);
void            set_error_color(const char *color);
void            set_warning_prefix(const char *prefix);
void            set_warning_color(const char *color);
void            set_success_prefix(const char *prefix);
void            set_success_color(const char *color);
void            set_info_prefix(const char *prefix);
void            set_info_color(const char *color);
void            set_debug_prefix(const char *prefix);
void            set_debug_color(const char *color);

output_style_t *get_output_style(void);

void            erro(const char *str);
void            panic(const char *str);
void            warn(const char *str);
void            success(const char *str);
void            info(const char *str);
void            debug(const char *str);

void            errof(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            panicf(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            warnf(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            successf(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            infof(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            debugf(const char *fmt, ...) PRINTF_FORMAT(1, 2);

void            set_log_level(log_level_t level);
log_level_t     get_log_level(void);
bool            is_quiet(void);

void            writef(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            writeln(const char *str);
void            writelnf(const char *fmt, ...) PRINTF_FORMAT(1, 2);
void            new_line(void);

#endif // OUTPUT_H
