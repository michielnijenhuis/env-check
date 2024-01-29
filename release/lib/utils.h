#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include "colors.h"

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

#ifdef UTILS_IMPLEMENTATION
#ifndef UTILS_INCLUDED
# define UTILS_INCLUDED

void printErr(cstring msg) {
    fprintf(stderr, "%s[ERROR]%s %s\n", RED_BOLD, NO_COLOUR, msg);
}

void panic(cstring msg) {
    printErr(msg);
    exit(EXIT_FAILURE);
    /* NOT REACHED */
}

boolean fileExists(cstring fileName) {
    struct stat buffer;
    return (stat(fileName, &buffer) == 0);
}

int fileNotFoundError(cstring fileName) {
    char errorMsg[FILENAME_MAX];
    sprintf(errorMsg, "File '%s' does not exist.", fileName);
    printErr(errorMsg);
    return EXIT_FAILURE;
}

int failedToOpenFileError(cstring fileName) {
    char errorMsg[FILENAME_MAX];
    sprintf(errorMsg, "Failed to open file '%s'.", fileName);
    printErr(errorMsg);
    return EXIT_FAILURE;
}

// TODO: implement
boolean directoryExists(cstring dirName) {
    printf("[TODO] directoryExists(): %s\n", dirName);
    return false;
}

#endif // UTILS_INCLUDED
#endif // UTILS_IMPLEMENTATION
