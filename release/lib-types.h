#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef int8_t       i8;
typedef int16_t      i16;
typedef int32_t      i32;
typedef int64_t      i64;

typedef unsigned int uint;
typedef uint8_t      u8;
typedef uint16_t     u16;
typedef uint32_t     u32;
typedef uint64_t     u64;

typedef float        f32;
typedef double       f64;

typedef size_t       usize;
typedef ssize_t      ssize;

typedef bool         boolean;

typedef char         byte;
typedef char        *string;
typedef const char  *cstr;
typedef const char  *cstring;
typedef char         mstring[];

typedef FILE         File;

#endif // TYPES_H
