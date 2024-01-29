#!/bin/bash

if [ ! -d "src/lib" ]; then
    mkdir -p "src/lib"
fi

cd "src/lib" || exit 1

function symlink_lib() {
    for lib in "$@"; do
        ln -sf "$HOME/dotfiles/lib/$lib.h" "$(pwd)";
    done
}

symlink_lib "argument" \
    "array-list" \
    "cli" \
    "colors" \
    "command" \
    "cstring" \
    "hash-table" \
    "input" \
    "math" \
    "option" \
    "output" \
    "program" \
    "types" \
    "utils"