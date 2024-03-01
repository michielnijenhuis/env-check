#include <assert.h>
#include <cstring.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void str_trim_left(const char *str, char *dest) {
    assert(str != NULL);
    assert(dest != NULL);

    size_t len = strlen(str);
    size_t i   = 0;
    while (i < len && isspace(str[i])) {
        i += 1;
    }
    strcpy(dest, str + i);
    dest[len - i] = '\0';
}

void str_trim_right(const char *str, char *dest) {
    assert(str != NULL);
    assert(dest != NULL);

    size_t len = strlen(str);
    size_t i   = 0;
    while (i < len && isspace(str[len - 1 - i])) {
        i += 1;
    }
    size_t newlen = len - i;
    strncpy(dest, str, newlen);
    dest[newlen] = '\0';
}

void str_trim(const char *str, char *dest) {
    str_trim_right(str, dest);
    str_trim_left(str, dest);
}

void str_slice_left(const char *str, size_t n, char *dest) {
    assert(str != NULL);
    assert(dest != NULL);

    size_t len = strlen(str);

    if (n > len) {
        n = len;
    }

    size_t newlen = len - n;
    strncpy(dest, str + n, newlen);
    dest[newlen] = '\0';
}

void str_slice_right(const char *str, size_t n, char *dest) {
    assert(str != NULL);
    assert(dest != NULL);

    size_t len = strlen(str);
    if (n >= len) {
        memcpy(dest, str, len + 1);
        return;
    }

    size_t newlen = len - n;
    strncpy(dest, str, newlen);
    dest[newlen] = '\0';
}

bool str_index_of(const char *str, char c, size_t *index) {
    assert(str != NULL);

    size_t len = strlen(str);
    size_t i   = 0;

    while (i < len && str[i] != c) {
        i += 1;
    }

    if (i < len) {
        if (index) {
            *index = i;
        }
        return true;
    }

    return false;
}

bool str_starts_with(const char *str, const char *prefix) {
    if (!str || !prefix) {
        return false;
    }

    size_t str_len    = strlen(str);
    size_t prefix_len = strlen(prefix);

    if (prefix_len <= str_len) {
        return strncmp(prefix, str, prefix_len) == 0;
    }

    return false;
}

bool str_ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) {
        return false;
    }

    size_t str_len    = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len <= str_len) {
        return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
    }

    return false;
}

bool str_equals(const char *a, const char *b) {
    if (!a || !b) {
        return false;
    }

    return strcmp(a, b) == 0;
}

bool str_equals_case_insensitive(const char *a, const char *b) {
    if (!a || !b) {
        return false;
    }

    size_t alen = strlen(a);

    char   x, y;
    for (size_t i = 0; i < alen; ++i) {
        x = 'A' <= a[i] && a[i] <= 'Z' ? a[i] + 32 : a[i];
        y = 'A' <= b[i] && b[i] <= 'Z' ? b[i] + 32 : b[i];

        if (x != y) {
            return false;
        }
    }
    return true;
}

void str_slice_left_while(const char *str, bool (*predicate)(char x), char *dest) {
    assert(str != NULL);
    assert(predicate != NULL);
    assert(dest != NULL);

    size_t len = strlen(str);
    size_t i   = 0;
    while (i < len && predicate(str[i])) {
        ++i;
    }
    str_slice_left(str, i, dest);
}

void str_slice_right_while(char *str, bool (*predicate)(char x), char *dest) {
    assert(str != NULL);
    assert(predicate != NULL);
    assert(dest != NULL);

    size_t len = strlen(str);
    size_t i   = len - 1;
    while (i >= 0 && predicate(str[i])) {
        --i;
    }
    str_slice_right(str, i + 1, dest);
}

bool str_is_empty(const char *str) {
    if (str == NULL) {
        return true;
    }

    size_t len = strlen(str);

    if (len == 0 || str[0] == '\0') {
        return true;
    }
    
    for (size_t i = 0; i < len; ++i) {
        if (!isspace(str[i])) {
            return false;
        }
    }

    return true;
}

void str_capitalize(char *str) {
    assert(str != NULL);
    str[0] = toupper(str[0]);
}

void str_to_upper(char *str) {
    assert(str != NULL);
    size_t i   = 0;
    size_t len = strlen(str);
    for (; i < len; ++i) {
        str[i] = toupper(str[i]);
    }
}

void str_to_lower(char *str) {
    assert(str != NULL);
    size_t i   = 0;
    size_t len = strlen(str);
    for (; i < len; ++i) {
        str[i] = tolower(str[i]);
    }
}

size_t str_count_char(const char *str, char c) {
    if (!str) return 0;
    size_t count = 0;
    size_t len   = strlen(str);
    for (size_t i = 0; i < len; ++i) {
        if (c == str[i]) {
            ++count;
        }
    }
    return count;
}

void str_slice(const char *str, size_t start, size_t end, char *dest) {
    assert(str != NULL);
    assert(dest != NULL);
    assert(start <= end);
    size_t len = strlen(str);
    if (end > len) {
        end = len;
    }
    size_t newlen = end - start;
    strncpy(dest, str + start, newlen);
    dest[newlen] = '\0';
}

size_t str_count_substr(const char *str, const char *substr) {
    if (!str || !substr) return 0;
    size_t count = 0;
    size_t len   = strlen(str);
    char   buf[len + 1];
    memcpy(buf, str, len + 1);
    char *p = buf;

    while ((p = strstr(p, substr)) != NULL) {
        ++count;
        ++p;
    }

    return count;
}

void str_from_charcode(short charcode, char *dest) {
    assert(dest != NULL);
    dest[0] = charcode;
    dest[1] = '\0';
}

void str_pad_left(const char *str, size_t maxlen, char fill, char *dest) {
    assert(str != NULL);
    assert(maxlen > 0);
    assert(dest != NULL);
    size_t len = strlen(str);
    if (len >= maxlen) {
        strcpy(dest, str);
        return;
    }
    int remaining = maxlen - len;
    for (int i = 0; i < remaining; ++i) {
        dest[i] = fill;
    }
    strcpy(dest + remaining, str);
    dest[maxlen] = '\0';
}

void str_pad_right(const char *str, size_t maxlen, char fill, char *dest) {
    assert(str != NULL);
    assert(maxlen > 0);
    assert(dest != NULL);
    size_t len = strlen(str);
    if (len >= maxlen) {
        strcpy(dest, str);
        return;
    }
    strncpy(dest, str, len);
    memset(dest + 1, fill, maxlen - len);
    dest[maxlen] = '\0';
}

bool str_contains(const char *str, const char *substr) {
    assert(str != NULL);
    assert(substr != NULL);

    return strstr(str, substr) != NULL;
}

int str_split_by_delim(const char *str, char delim, char **strv, size_t strc) {
    if (!str || !strv || strc == 0) {
        return -1;
    }

    size_t str_len = strlen(str);
    size_t start   = 0;
    size_t i       = 0;

    while (start < str_len) {
        size_t end = start;
        while (end < str_len && str[end] != delim) {
            ++end;
        }

        if (i + 1 < strc) {
            size_t len      = end - start;
            char  *fragment = (char *) malloc(sizeof(char) * (len + 1));
            if (fragment == NULL) {
                return -3;
            }
            strncpy(fragment, str + start, len);
            fragment[len] = '\0';
            strv[i]       = fragment;
        } else {
            size_t len      = str_len - start;
            char  *fragment = (char *) malloc(sizeof(char) * (len + 1));
            if (fragment == NULL) {
                return -3;
            }
            strncpy(fragment, str + start, len);
            fragment[len] = '\0';
            strv[i]       = fragment;
            return end + 1 < str_len ? -2 : 0;
        }

        start = end + 1;
        ++i;
    }

    for (; i < strc; ++i) {
        strv[i] = NULL;
    }

    return 0;
}

int str_split_by_substr(const char *str, const char *substr, char **strv, size_t strc) {
    if (!str || !substr || !strv || strc == 0) {
        return -1;
    }

    size_t str_len    = strlen(str);
    size_t substr_len = strlen(substr);
    size_t start      = 0;
    size_t i          = 0;

    while (start < str_len) {
        size_t end = start;
        while (end < str_len && strncmp(str + end, substr, substr_len) != 0) {
            ++end;
        }

        if (i + 1 < strc) {
            size_t len      = end - start;
            char  *fragment = (char *) malloc(sizeof(char) * (len + 1));
            if (fragment == NULL) {
                return -3;
            }
            strncpy(fragment, str + start, len);
            fragment[len] = '\0';
            strv[i]       = fragment;
        } else {
            size_t len      = str_len - start;
            char  *fragment = (char *) malloc(sizeof(char) * (len + 1));
            if (fragment == NULL) {
                return -3;
            }
            strncpy(fragment, str + start, len);
            fragment[len] = '\0';
            strv[i]       = fragment;
            return end + substr_len < str_len ? -2 : 0;
        }

        start = end + substr_len;
        ++i;
    }

    for (; i < strc; ++i) {
        strv[i] = NULL;
    }

    return 0;
}
