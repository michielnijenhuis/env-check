#!/bin/bash

# shellcheck disable=SC1091
source "$HOME/dotfiles/.aliases"

compile -o bin/main src/main

sudo cp bin/main /usr/local/bin/envc