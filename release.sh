#!/bin/bash

# shellcheck disable=SC1091
source "$HOME/dotfiles/.aliases"

rm -rf ./release
mkdir -p release

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
)

VERSION=$(git describe --tags --abbrev=0 | sed 's/^v//')

for LIB in "${LIBS[@]}"; do
    FILE="/usr/local/include/$LIB.h"
    
    if [ -f "$FILE" ]; then
        sudo cp "$FILE" "release/$LIB.h"
    fi
done

for LIB in "${LIBS[@]}"; do
    FILE="/usr/local/lib/lib$LIB.dylib"

    if [ -f "$FILE" ]; then
        sudo cp "$FILE" "release/lib$LIB.dylib"
    fi
done

BUILD_SCRIPT="release/build.sh"
touch "$BUILD_SCRIPT"
echo "#!/bin/bash" > $BUILD_SCRIPT
printf "\nset -x\n" >> "$BUILD_SCRIPT"
printf "\"\$@\" -DENVC_VERSION=%s%s%s -o ./envc main.c -L." '\"' "$VERSION" '\"' >> "$BUILD_SCRIPT"
for LIB in "${LIBS[@]}"; do
    if [ -f "release/lib$LIB.dylib" ]; then
        printf " -l%s" "$LIB" >> "$BUILD_SCRIPT"
    fi
done
printf "\n\n" >> "$BUILD_SCRIPT"

cp "src/main.c" "release/main.c"

compile -r -DENVC_VERSION=\\\""$VERSION"\\\" -o bin/main src/main