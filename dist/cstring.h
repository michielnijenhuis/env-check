#ifndef CSTRING_H
#define CSTRING_H

#include <stdbool.h>
#include <stddef.h>

void   str_from_charcode(short charcode, char *dest);
void   str_trim_right(const char *str, char *dest);
void   str_trim_left(const char *str, char *dest);
void   str_trim(const char *str, char *dest);
void   str_slice(const char *str, size_t start, size_t end, char *dest);
void   str_slice_left(const char *str, size_t n, char *dest);
void   str_slice_left_while(const char *str, bool (*predicate)(char x), char *dest);
void   str_slice_right(const char *str, size_t n, char *dest);
void   str_slice_right_while(char *str, bool (*predicate)(char x), char *dest);
void   str_capitalize(char *str);
void   str_to_upper(char *str);
void   str_to_lower(char *str);
void   str_pad_left(const char *str, size_t maxlen, char fill, char *dest);
void   str_pad_right(const char *str, size_t maxlen, char fill, char *dest);

int    str_split_by_delim(const char *str, char delim, char **strv, size_t strc);
int    str_split_by_substr(const char *str, const char *substr, char **strv, size_t strc);

size_t str_count_char(const char *str, char c);
size_t str_count_substr(const char *str, const char *substr);

bool   str_index_of(const char *str, char c, size_t *index);
bool   str_equals(const char *a, const char *b);
bool   str_equals_case_insensitive(const char *a, const char *b);
bool   str_starts_with(const char *str, const char *prefix);
bool   str_ends_with(const char *str, const char *suffix);
bool   str_contains(const char *str, const char *substr);
bool   str_is_empty(const char *str);

#endif // CSTRING_H
