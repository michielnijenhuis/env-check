#!/bin/bash

CMD="$*"

if [ -z "$CMD" ]; then
	CMD="clang -Wall -O2"
fi

set -x

$CMD -DENVC_VERSION=\"1.1.1\" -o ./envc main.c -L. -largument -lcli -lcolors -lfs -lcommand -lcstring -linput -loption -loutput -lprogram -lusage

