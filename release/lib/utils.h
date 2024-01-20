#ifndef UTILS_H
#define UTILS_H

#include "types.h"

#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

#ifndef UTILS_DEF
# define UTILS_DEF
#endif // UTILS_DEF

#define ARRAY_LEN(x)      (sizeof(x) / sizeof((x)[0]))

#define NO_COLOUR         "\033[0m"
#define WHITE             "\033[0;37m"
#define WHITE_BOLD        "\033[1;37m"
#define DARK_GRAY         "\033[1;30m"
#define YELLOW            "\033[0;33m"
#define YELLOW_BOLD       "\033[1;33m"
#define ORANGE            "\033[38;5;214m"
#define ORANGE_BOLD_LIGHT "\033[38;5;223m"
#define CYAN              "\033[0;36m"
#define BLUE              "\033[38;5;27m"
#define GREEN             "\033[0;32m"
#define GREEN_BRIGHT      "\033[38;5;10m"
#define GREEN_LIGHT       "\033[38;5;82m"
#define GREEN_OLIVE       "\033[38;5;58m"
#define GREEN_LIME        "\033[38;5;154m"
#define GREEN_DARK        "\033[38;5;22m"
#define EMERALD           "\033[38;5;48m"
#define RED               "\033[0;31m"
#define RED_BOLD          "\033[1;31m"
#define RED_BOLD_LIGHT    "\033[38;5;198m"
#define MAGENTA           "\033[0;35m"
#define MAGENTA_LIGHT     "\033[1;95m"

void    printError(cstring msg);
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

void printError(cstring msg) {
    fprintf(stderr, "%s[ERROR]%s %s\n", RED_BOLD, NO_COLOUR, msg);
}

void panic(cstring msg) {
    printError(msg);
    exit(EXIT_FAILURE);
    /* NOT REACHED */
}

uint max(uint a, uint b) {
    return (a > b) ? a : b;
}

uint min(uint a, uint b) {
    return (a < b) ? a : b;
}

boolean fileExists(cstring fileName) {
    struct stat buffer;
    return (stat(fileName, &buffer) == 0);
}

int fileNotFoundError(cstring fileName) {
    char errorMsg[FILENAME_MAX];
    sprintf(errorMsg, "File '%s' does not exist.", fileName);
    printError(errorMsg);
    return EXIT_FAILURE;
}

int failedToOpenFileError(cstring fileName) {
    char errorMsg[FILENAME_MAX];
    sprintf(errorMsg, "Failed to open file '%s'.", fileName);
    printError(errorMsg);
    return EXIT_FAILURE;
}

// TODO: implement
boolean directoryExists(cstring dirName) {
    printf("[TODO] directoryExists(): %s\n", dirName);
    return false;
}

#endif // UTILS_INCLUDED
#endif // UTILS_IMPLEMENTATION
