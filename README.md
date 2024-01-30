# Env Check
A program that compares two .env files and shows their values and differences.

## Usage:
```sh
command [options] [arguments]
```

## Options:
```sh
-h, --help              Display help for the given command.
-q, --quiet             Do not output any message.
    --ansi|no-ansi      Force (or disable) ANSI output.
-V, --version           Show the current version of the program.
-v|vv|vvv, --verbose    Set the log level for the program.
```

## Available commands:
```sh
cmp     Compares two env files files.
list    Lists all variables in the target env file, sorted alphabetically.
```