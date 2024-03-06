#!/bin/bash

CMD="$*"

set -x

$CMD -DENVC_VERSION=\"1.2.0\" -I. dist/main.c dist/argument.c dist/cli.c dist/colors.c dist/fs.c dist/command.c dist/cstring.c dist/input.c dist/option.c dist/output.c dist/program.c dist/usage.c
