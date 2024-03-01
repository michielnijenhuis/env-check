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
    "usage"
)

VERSION=$(git describe --tags --abbrev=0 | sed 's/^v//')

for LIB in "${LIBS[@]}"; do
    FILE="/usr/local/include/$LIB.h"
    
    if [ -f "$FILE" ]; then
        sudo cp "$FILE" "release/$LIB.h"
    fi
done

for LIB in "${LIBS[@]}"; do
    # FILE="/usr/local/lib/lib$LIB.dylib"
    FILE="/Users/michielnijenhuis/Code/c-libs/src/$LIB.c"

    if [ -f "$FILE" ]; then
        sudo cp "$FILE" "release/$LIB.c"
    fi
done

BUILD_SCRIPT="release/build.sh"
touch "$BUILD_SCRIPT"
printf "#!/bin/bash\n\n" > $BUILD_SCRIPT
{
    printf "CMD=\"\$*\"\n\n";
    printf "if [ -z \"\$CMD\" ]; then\n";
    printf "\tCMD=\"clang -Wall -O2\"\n";
    printf "fi\n\n"
    printf "set -x\n\n";
} >> $BUILD_SCRIPT

printf "\$CMD -DENVC_VERSION=%s%s%s -o ./envc main.c -L." '\"' "$VERSION" '\"' >> "$BUILD_SCRIPT"
for LIB in "${LIBS[@]}"; do
    if [ -f "release/$LIB.c" ]; then
        printf " -l%s" "$LIB" >> "$BUILD_SCRIPT"
    fi
done
printf "\n\n" >> "$BUILD_SCRIPT"

cp "src/main.c" "release/main.c"

compile -r -DENVC_VERSION=\\\""$VERSION"\\\" -o bin/main src/main