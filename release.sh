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

VERSION=$(git describe --tags --abbrev=0 | sed 's/^v//')

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

BUILD_SCRIPT="./build.sh"
touch "$BUILD_SCRIPT"
printf "#!/bin/bash\n\n" > $BUILD_SCRIPT
{
    printf "CMD=\"\$*\"\n\n";
    printf "if [ -z \"\$CMD\" ]; then\n";
    printf "\tCMD=\"clang -Wall -O2\"\n";
    printf "fi\n\n"
    printf "set -x\n\n";
} >> $BUILD_SCRIPT

printf "\$CMD -DENVC_VERSION=%s%s%s -Idist -o bin/envc dist/main.c" '\"' "$VERSION" '\"' >> "$BUILD_SCRIPT"
for LIB in "${LIBS[@]}"; do
    if [ -f "dist/$LIB.c" ]; then
        printf " dist/%s.c" "$LIB" >> "$BUILD_SCRIPT"
    fi
done
printf "\n"

cp "src/main.c" "dist/main.c"

compile -r -DENVC_VERSION=\\\""$VERSION"\\\" -o bin/main src/main