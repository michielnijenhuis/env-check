#ifndef CSTRING_H
#define CSTRING_H

#include "types.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#ifndef CSTRINGDEF
# define CSTRINGDEF
#endif // CSTRINGDEF

typedef struct String {
    usize   length;
    cstring data;
} String;

#define STRING(cstr) stringFromParts(cstr, sizeof(cstr) - 1)
#define STRING_NULL  stringFromParts(NULL, 0)

CSTRINGDEF String  stringFrom(cstring str);
CSTRINGDEF String *stringMalloc(const String str);
CSTRINGDEF void    stringFree(String *str);
CSTRINGDEF String  stringFromParts(cstring str, usize length);
CSTRINGDEF String  stringFromCharCode(short charCode, string buffer);
CSTRINGDEF String  stringTrimRight(const String str, string buffer);
CSTRINGDEF String  stringTrimLeft(const String str, string buffer);
CSTRINGDEF String  stringTrim(const String str, string buffer);
CSTRINGDEF String  stringSlice(const String str, usize start, usize end, string buffer);
CSTRINGDEF String  stringSliceLeft(const String str, usize n, string buffer);
CSTRINGDEF String  stringSliceLeftWhile(const String str, boolean (*predicate)(char x), string buffer);
CSTRINGDEF String  stringSliceRight(const String str, usize n, string buffer);
CSTRINGDEF String  stringSliceRightWhile(const String str, boolean (*predicate)(char x), string buffer);
CSTRINGDEF String  stringCapitalize(const String str, string buffer);
CSTRINGDEF String  stringToUpper(const String str, string buffer);
CSTRINGDEF String  stringToLower(const String str, string buffer);
CSTRINGDEF String  stringPadLeft(const String str, usize maxLen, char fill, string buffer);
CSTRINGDEF String  stringPadRight(const String str, usize maxLen, char fill, string buffer);
CSTRINGDEF String  stringConcat(const String a, const String b, string buffer);
CSTRINGDEF String  stringReplaceChar(const String str, usize index, char c, string buffer);
CSTRINGDEF String  stringCopy(const String str, string buffer);
CSTRINGDEF boolean stringIndexOf(const String str, char c, usize *index);
CSTRINGDEF boolean stringEquals(const String a, const String b);
CSTRINGDEF boolean stringEqualsCaseInsensitive(const String a, const String b);
CSTRINGDEF boolean stringStartsWith(const String str, const String prefix);
CSTRINGDEF boolean stringEndsWith(const String str, const String suffix);
CSTRINGDEF boolean stringIsEmpty(const String str);
CSTRINGDEF boolean stringIsNull(const String str);
CSTRINGDEF boolean stringContains(const String str, const String substr);
CSTRINGDEF boolean stringContainsCstr(const String str, cstring substr);
CSTRINGDEF boolean stringIsNullTerminated(const String str);
CSTRINGDEF int     stringSplitByDelim(const String str, char delim, String *buffer, usize bufferSize);
CSTRINGDEF int     stringSplitByString(const String str, const String delim, String *buffer, usize bufferSize);
CSTRINGDEF usize   stringCharOccurrences(const String str, char c);
CSTRINGDEF usize   stringSubstringOccurrences(String str, const String substr);
CSTRINGDEF usize   stringLength(const String str);
CSTRINGDEF cstring stringToCstring(const String str);
CSTRINGDEF void    stringPrint(const String str);
CSTRINGDEF void    stringEnsureNullTerminator(string buffer, usize length);

CSTRINGDEF boolean cstringEquals(cstring a, cstring b);
CSTRINGDEF boolean cstringEqualsCaseInsensitive(cstring a, cstring b);
CSTRINGDEF boolean cstringStartsWith(cstring str, cstring prefix);
CSTRINGDEF boolean cstringEndsWith(cstring str, cstring suffix);
CSTRINGDEF boolean cstringIsEmpty(cstring str);
CSTRINGDEF boolean cstringContains(cstring str, cstring substr);

#endif // SV_H_

#ifdef CSTRING_IMPLEMENTATION
#ifndef CSTRING_INCLUDED
# define CSTRING_INCLUDED

CSTRINGDEF String stringFromParts(cstring data, usize length) {
    String str;
    str.length = length;
    str.data   = data;

    if (str.data != NULL && str.data[str.length] != '\0') {
        fprintf(stderr, "String is not null terminated: %s\n", str.data);
    }

    return str;
}

CSTRINGDEF String stringFrom(cstring str) {
    if (str == NULL) {
        return STRING_NULL;
    }

    return stringFromParts(str, strlen(str));
}

CSTRINGDEF String *stringMalloc(const String str) {
    String *result = (String *) malloc(sizeof(*result));
    cstring data   = strdup(str.data);
    result->data   = data;
    result->length = str.length;
    return result;
}

CSTRINGDEF void stringFree(String *str) {
    free((string) str->data);
    free(str);
}

CSTRINGDEF String stringTrimLeft(const String str, string buffer) {
    if (stringIsNull(str)) {
        return STRING_NULL;
    }

    usize i = 0;
    while (i < str.length && isspace(str.data[i])) {
        i += 1;
    }
    strcpy(buffer, str.data + i);
    stringEnsureNullTerminator(buffer, str.length - i);
    return stringFromParts(buffer, str.length - i);
}

CSTRINGDEF String stringTrimRight(const String str, string buffer) {
    if (stringIsNull(str)) {
        return STRING_NULL;
    }

    usize i = 0;
    while (i < str.length && isspace(str.data[str.length - 1 - i])) {
        i += 1;
    }
    usize newLen = str.length - i;
    strncpy(buffer, str.data, newLen);
    stringEnsureNullTerminator(buffer, newLen);
    return stringFromParts(buffer, newLen);
}

CSTRINGDEF String stringTrim(const String str, string buffer) {
    char bufferLeft[str.length + 1];
    return stringTrimRight(stringTrimLeft(str, bufferLeft), buffer);
}

CSTRINGDEF String stringSliceLeft(const String str, usize n, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    if (n > str.length) {
        n = str.length;
    }

    usize newLength = str.length - n;
    strncpy(buffer, str.data + n, newLength);
    stringEnsureNullTerminator(buffer, newLength);
    return stringFromParts(buffer, newLength);
}

CSTRINGDEF String stringSliceRight(const String str, usize n, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    if (n >= str.length) {
        memcpy(buffer, str.data, str.length + 1);
        return stringFromParts(buffer, str.length);
    }

    usize newLength = str.length - n;
    strncpy(buffer, str.data, newLength);
    stringEnsureNullTerminator(buffer, newLength);
    return stringFromParts(buffer, newLength);
}

CSTRINGDEF boolean stringIndexOf(const String str, char c, usize *index) {
    if (stringIsNull(str)) {
        return false;
    }

    usize i = 0;
    while (i < str.length && str.data[i] != c) {
        i += 1;
    }

    if (i < str.length) {
        if (index) {
            *index = i;
        }
        return true;
    } else {
        return false;
    }
}

CSTRINGDEF boolean stringStartsWith(String str, String expectedPrefix) {
    if (stringIsNull(str) || stringIsNull(expectedPrefix)) {
        return false;
    }

    if (expectedPrefix.length <= str.length) {
        return strncmp(expectedPrefix.data, str.data, expectedPrefix.length) == 0;
    }

    return false;
}

CSTRINGDEF boolean stringEndsWith(String str, String expectedSuffix) {
    if (stringIsNull(str) || stringIsNull(expectedSuffix)) {
        return false;
    }

    if (expectedSuffix.length <= str.length) {
        String actualSuffix = stringFromParts(str.data + str.length - expectedSuffix.length, expectedSuffix.length);
        return stringEquals(expectedSuffix, actualSuffix);
    }

    return false;
}

CSTRINGDEF boolean stringEquals(String a, String b) {
    if (stringIsNull(a) || stringIsNull(b)) {
        return false;
    }

    if (a.length != b.length) {
        return false;
    } else {
        return memcmp(a.data, b.data, a.length) == 0;
    }
}

CSTRINGDEF boolean stringEqualsCaseInsensitive(String a, String b) {
    if (stringIsNull(a) || stringIsNull(b)) {
        return false;
    }

    if (a.length != b.length) {
        return false;
    }

    char x, y;
    for (usize i = 0; i < a.length; i++) {
        x = 'A' <= a.data[i] && a.data[i] <= 'Z' ? a.data[i] + 32 : a.data[i];

        y = 'A' <= b.data[i] && b.data[i] <= 'Z' ? b.data[i] + 32 : b.data[i];

        if (x != y) {
            return false;
        }
    }
    return true;
}

CSTRINGDEF String stringSliceLeftWhile(String str, boolean (*predicate)(char x), string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    usize i = 0;
    while (i < str.length && predicate(str.data[i])) {
        ++i;
    }
    return stringSliceLeft(str, i, buffer);
}

CSTRINGDEF String stringSliceRightWhile(String str, boolean (*predicate)(char x), string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    usize i = str.length - 1;
    while (i >= 0 && predicate(str.data[i])) {
        --i;
    }
    return stringSliceRight(str, i + 1, buffer);
}

CSTRINGDEF void stringPrint(String str) {
    printf("%s\n", str.data);
}

CSTRINGDEF boolean stringIsNull(const String str) {
    return str.data == NULL;
}

CSTRINGDEF boolean stringIsEmpty(const String str) {
    if (str.length == 0 || stringIsNull(str)) {
        return true;
    }

    for (usize i = 0; i < str.length; ++i) {
        if (!isspace(str.data[i])) {
            return false;
        }
    }

    return true;
}

CSTRINGDEF String stringCapitalize(const String str, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    memcpy(buffer, str.data, str.length + 1);
    buffer[0] = toupper(buffer[0]);
    return stringFromParts(buffer, str.length);
}

CSTRINGDEF String stringToUpper(const String str, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    usize i = 0;
    for (; i < str.length; ++i) {
        buffer[i] = toupper(str.data[i]);
    }
    stringEnsureNullTerminator(buffer, i);
    return stringFromParts(buffer, i);
}

CSTRINGDEF String stringToLower(const String str, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    usize i = 0;
    for (; i < str.length; ++i) {
        buffer[i] = tolower(str.data[i]);
    }
    stringEnsureNullTerminator(buffer, i);
    return stringFromParts(buffer, i);
}

CSTRINGDEF usize stringLength(const String str) {
    if (stringIsNull(str)) {
        return 0;
    }

    return str.length;
}

CSTRINGDEF cstring stringToCstring(const String str) {
    if (stringIsNull(str)) {
        return NULL;
    }

    return str.data;
}

CSTRINGDEF usize stringCharOccurrences(const String str, char c) {
    if (stringIsNull(str)) {
        return 0;
    }

    usize count = 0;
    for (usize i = 0; i < str.length; ++i) {
        if (c == str.data[i]) {
            ++count;
        }
    }
    return count;
}

CSTRINGDEF String stringSlice(const String str, usize start, usize end, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    if (end > str.length) {
        end = str.length;
    }

    if (start > end) {
        start = end;
    }

    usize newLength = end - start;
    strncpy(buffer, str.data + start, newLength);
    stringEnsureNullTerminator(buffer, newLength);
    return stringFromParts(buffer, newLength);
}

usize stringSubstringOccurrences(const String str, const String substr) {
    if (stringIsNull(str) || stringIsNull(substr)) {
        return 0;
    }

    usize count = 0;
    char  buffer[str.length + 1];
    memcpy(buffer, str.data, str.length + 1);
    string tmp = buffer;

    while ((tmp = strstr(tmp, substr.data)) != NULL) {
        count++;
        tmp++;
    }

    return count;
}

CSTRINGDEF String stringFromCharCode(short charCode, string buffer) {
    if (buffer == NULL) {
        return STRING_NULL;
    }

    buffer[0] = charCode;
    buffer[1] = '\0';
    return stringFromParts(buffer, 1);
}

CSTRINGDEF String stringPadLeft(const String str, usize maxLen, char fill, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    usize curLen    = str.length;
    int   remaining = maxLen - curLen;
    if (remaining <= 0 || curLen >= maxLen) {
        return stringCopy(str, buffer);
    }

    for (int i = 0; i < remaining; ++i) {
        buffer[i] = fill;
    }
    strcpy(buffer + remaining, str.data);
    stringEnsureNullTerminator(buffer, maxLen);
    return stringFromParts(buffer, maxLen);
}

CSTRINGDEF String stringPadRight(const String str, usize maxLen, char fill, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    usize curLen = str.length;

    if (curLen >= maxLen) {
        return stringCopy(str, buffer);
    }

    strncpy(buffer, str.data, str.length);
    memset(buffer + 1, fill, maxLen - curLen);
    stringEnsureNullTerminator(buffer, maxLen);
    return stringFromParts(buffer, maxLen);
}

CSTRINGDEF String stringConcat(const String a, const String b, string buffer) {
    if (stringIsNull(a) || stringIsNull(b) || buffer == NULL) {
        return STRING_NULL;
    }

    usize newLen = a.length + b.length;
    memcpy(buffer, a.data, a.length + 1);
    strcat(buffer, b.data);
    stringEnsureNullTerminator(buffer, newLen);
    return stringFromParts(buffer, newLen);
}

CSTRINGDEF boolean stringContains(const String str, const String substr) {
    if (stringIsNull(str) || stringIsNull(substr)) {
        return false;
    }

    return stringContainsCstr(str, substr.data);
}

CSTRINGDEF boolean stringContainsCstr(const String str, cstring substr) {
    if (stringIsNull(str) || substr == NULL) {
        return false;
    }

    return strstr(str.data, substr) != NULL;
}

CSTRINGDEF String stringReplaceChar(const String str, usize index, char c, string buffer) {
    if (stringIsNull(str) || buffer == NULL) {
        return STRING_NULL;
    }

    memcpy(buffer, str.data, str.length + 1);
    buffer[index] = c;
    return stringFromParts(buffer, str.length);
}

CSTRINGDEF int stringSplitByDelim(const String str, char delim, String *buffer, usize bufferSize) {
    if (stringIsNull(str) || buffer == NULL || bufferSize == 0) {
        return -1;
    }

    usize start         = 0;
    usize fragmentIndex = 0;

    while (start < str.length) {
        usize end = start;
        while (end < str.length && str.data[end] != delim) {
            ++end;
        }

        if (fragmentIndex + 1 < bufferSize) {
            usize  len  = end - start;
            string data = (string) malloc(sizeof(char) * (len + 1));
            if (data == NULL) {
                return -3;
            }
            strncpy(data, str.data + start, len);
            data[len]                    = '\0';
            buffer[fragmentIndex].length = len;
            buffer[fragmentIndex].data   = data;
        } else {
            usize  len  = str.length - start;
            string data = (string) malloc(sizeof(char) * (len + 1));
            if (data == NULL) {
                return -3;
            }
            strncpy(data, str.data + start, len);
            data[len]                    = '\0';
            buffer[fragmentIndex].length = len;
            buffer[fragmentIndex].data   = data;
            buffer[fragmentIndex]        = stringFromParts(data, str.length - start);
            return end + 1 < str.length ? -2 : 0;
        }

        start = end + 1;
        ++fragmentIndex;
    }

    for (usize i = fragmentIndex; i < bufferSize; ++i) {
        buffer[i] = STRING_NULL;
    }

    return 0;
}

CSTRINGDEF int stringSplitByString(const String str, const String delim, String *buffer, usize bufferSize) {
    if (stringIsNull(str) || stringIsNull(delim) || buffer == NULL || bufferSize == 0) {
        return -1;
    }

    usize start            = 0;
    usize fragmentIndex    = 0;

    while (start < str.length) {
        usize end = start;
        while (end < str.length && strncmp(str.data + end, delim.data, delim.length) != 0) {
            ++end;
        }

        if (fragmentIndex + 1 < bufferSize) {
            usize  len  = end - start;
            string data = (string) malloc(sizeof(char) * (len + 1));
            if (data == NULL) {
                return -3;
            }
            strncpy(data, str.data + start, len);
            data[len]                    = '\0';
            buffer[fragmentIndex].length = len;
            buffer[fragmentIndex].data   = data;
        } else {
            usize  len  = str.length - start;
            string data = (string) malloc(sizeof(char) * (len + 1));
            if (data == NULL) {
                return -3;
            }
            strncpy(data, str.data + start, len);
            data[len]                    = '\0';
            buffer[fragmentIndex].length = len;
            buffer[fragmentIndex].data   = data;
            buffer[fragmentIndex]        = stringFromParts(data, str.length - start);
            return end + delim.length < str.length ? -2 : 0;
        }

        start = end + delim.length;
        ++fragmentIndex;
    }

    for (usize i = fragmentIndex; i < bufferSize; ++i) {
        buffer[i]       = STRING_NULL;
    }

    return 0;
}

void stringEnsureNullTerminator(string buffer, usize length) {
    if (buffer == NULL) {
        return;
    }

    char cur = buffer[length];

    if (cur != '\0') {
        buffer[length] = '\0';
    }
}

CSTRINGDEF boolean stringIsNullTerminated(const String str) {
    return str.data[str.length] == '\0';
}

CSTRINGDEF String stringCopy(const String str, string buffer) {
    memcpy(buffer, str.data, str.length);
    stringEnsureNullTerminator(buffer, str.length);
    return stringFromParts(buffer, str.length);
}

CSTRINGDEF boolean cstringEquals(cstring a, cstring b) {
    if (a == NULL || b == NULL) {
        return false;
    }

    return strcmp(a, b) == 0;
}

CSTRINGDEF boolean cstringEqualsCaseInsensitive(cstring a, cstring b) {
    if (a == NULL || b == NULL) {
        return false;
    }

    char x, y;
    for (usize i = 0; i < strlen(a); i++) {
        x = 'A' <= a[i] && a[i] <= 'Z' ? a[i] + 32 : a[i];

        y = 'A' <= b[i] && b[i] <= 'Z' ? b[i] + 32 : b[i];

        if (x != y) {
            return false;
        }
    }
    return true;
}

CSTRINGDEF boolean cstringStartsWith(cstring str, cstring prefix) {
    if (str == NULL || prefix == NULL) {
        return false;
    }

    return strncmp(prefix, str, strlen(prefix)) == 0;
}

CSTRINGDEF boolean cstringEndsWith(cstring str, cstring suffix) {
    if (str == NULL || suffix == NULL) {
        return false;
    }

    usize strLen    = strlen(str);
    usize suffixLen = strlen(suffix);

    if (suffixLen > strLen) {
        return false;
    }

    return strncmp(str + strLen - suffixLen, suffix, suffixLen) == 0;
}

CSTRINGDEF boolean cstringIsEmpty(cstring str) {
    if (str == NULL) {
        return true;
    }

    for (usize i = 0; i < strlen(str); ++i) {
        if (!isspace(str[i])) {
            return false;
        }
    }

    return true;
}

CSTRINGDEF boolean cstringContains(cstring str, cstring substr) {
    if (str == NULL || substr == NULL) {
        return false;
    }

    return strstr(str, substr) != NULL;
}

#endif // CSTRING_INCLUDED
#endif // SV_IMPLEMENTATION
