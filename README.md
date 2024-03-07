# Env Check
A program that compares two .env files and shows their values and differences.

## Usage:
```console
$ command [options] [arguments]
```

## Options:
```console
-h, --help              Display help for the given command.
-q, --quiet             Do not output any message.
    --ansi|no-ansi      Force (or disable) ANSI output.
-V, --version           Show the current version of the program.
```

## Available commands:
```console
cmp     [compare] Compares two env files files.
list    Lists all variables in the target env file, sorted alphabetically.
```

# Installation

## Homebrew
```console
$ brew install michielnijenhuis/cli/envc
```

## Makefile
```console
$ make
```