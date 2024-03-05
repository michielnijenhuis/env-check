#!/bin/bash

CMD="$*"

if [ -z "$CMD" ]; then
	CMD="clang -Wall -O2"
fi

set -x

$CMD -DENVC_VERSION=\"1.1.1\" -I. -o ./envc main.c argument.c cli.c colors.c fs.c command.c cstring.c input.c option.c output.c program.c usage.c

sudo cp ./envc /usr/local/bin

