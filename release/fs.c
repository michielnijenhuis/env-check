#include <fs.h>
#include <output.h>
#include <stdio.h>
#include <sys/stat.h>

bool file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

bool dir_exists(const char *path) {
    struct stat buffer;
    return stat(path, &buffer) == 0 && S_ISDIR(buffer.st_mode);
}
