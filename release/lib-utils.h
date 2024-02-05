#ifndef UTILS_H
#define UTILS_H

#include <lib-types.h>
#include <lib-colors.h>
#include <lib-output.h>

#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

#define ARRAY_LEN(x)      (sizeof(x) / sizeof((x)[0]))

void    printErr(cstring msg);
void    panic(cstring msg);
uint    max(uint a, uint b);
uint    min(uint a, uint b);
boolean fileExists(cstring fileName);
int     fileNotFoundError(cstring fileName);
int     failedToOpenFileError(cstring fileName);
boolean directoryExists(cstring dirName);

#endif // UTILS_H
