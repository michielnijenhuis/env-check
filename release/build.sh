#!/bin/bash

set -x
"$@" -DENVC_VERSION=\"1.1.1\" -o ./envc main.c -L. -llib-argument -llib-array-list -llib-cli -llib-colors -llib-command -llib-cstring -llib-hash-table -llib-input -llib-option -llib-output -llib-program -llib-utils

