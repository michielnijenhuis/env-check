#ifndef CSTRING_H
#define CSTRING_H

#include <lib-types.h>

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

#endif // CSTRING_H
