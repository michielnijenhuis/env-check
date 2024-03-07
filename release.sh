#!/bin/bash

# shellcheck disable=SC1091
source "$HOME/dotfiles/.aliases"

CLIBS=$(realpath "$HOME/Code/c-libs")

rm -rf ./dist
mkdir -p dist

LIBS=(
    "argument"
    "array"
    "cli"
    "colors"
    "fs"
    "command"
    "cstring"
    "ht"
    "input"
    "option"
    "output"
    "program"
    "math-utils"
    "usage"
)

for LIB in "${LIBS[@]}"; do
    FILE="/usr/local/include/$LIB.h"
    
    if [ -f "$FILE" ]; then
        sudo cp "$FILE" "dist/$LIB.h"
    fi
done

for LIB in "${LIBS[@]}"; do
    FILE="$CLIBS/src/$LIB.c"

    if [ -f "$FILE" ]; then
        sudo cp "$FILE" "dist/$LIB.c"
    fi
done

cp "src/main.c" "dist/main.c"