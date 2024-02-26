#!/bin/bash

set -x
"$@" -DENVC_VERSION=\"1.1.1\" -o ./envc main.c -L. -largument -lcli -lcolors -lfs -lcommand -lcstring -linput -loption -loutput -lprogram

