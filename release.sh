#!/bin/bash

# shellcheck disable=SC1091
source "$HOME/dotfiles/.aliases"

if [[ ! -d "release" ]]; then
    mkdir -p "release"
fi

rm -rf ./release/**

# Use a for loop to iterate through each file in the directory
LIBS=(
    "argument"
    "array-list"
    "cli"
    "colors"
    "command"
    "cstring"
    "hash-table"
    "input"
    "option"
    "output"
    "program"
    "types"
    "utils"
)

VERSION=$(git describe --tags --abbrev=0 | sed 's/^v//')

for LIB in "${LIBS[@]}"; do
    FILE="/usr/local/include/lib-$LIB.h"
    
    if [ -f "$FILE" ]; then
        sudo cp "$FILE" "release/lib-$LIB.h"
    fi
done

for LIB in "${LIBS[@]}"; do
    FILE="/usr/local/lib/liblib-$LIB.dylib"

    if [ -f "$FILE" ]; then
        sudo cp "$FILE" "release/liblib-$LIB.dylib"
    fi
done

BUILD_SCRIPT="release/build.sh"
touch "$BUILD_SCRIPT"
echo "#!/bin/bash" > $BUILD_SCRIPT
printf "\nset -x\n" >> "$BUILD_SCRIPT"
printf "\"\$@\" -DENVC_VERSION=%s%s%s -o ./envc main.c -L." '\"' "$VERSION" '\"' >> "$BUILD_SCRIPT"
for LIB in "${LIBS[@]}"; do
    if [ -f "release/liblib-$LIB.dylib" ]; then
        printf " -llib-%s" "$LIB" >> "$BUILD_SCRIPT"
    fi
done
printf "\n\n" >> "$BUILD_SCRIPT"

cp "src/main.c" "release/main.c"
# TODO: replace include paths in main.c

compile -r -o bin/main src/main