CC=gcc
VERSION=
VERSION := $(shell git describe --tags --abbrev=0 | sed 's/^v//')
CFLAGS=-g -fsanitize=address,undefined -Wall -Wextra -Werror=implicit -pedantic -DENVC_VERSION=\"$(VERSION)\"
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
BINDIR=bin
BIN=$(BINDIR)/main

all: $(BIN)

release: CFLAGS=-Wall -O3 -flto -funroll-loops -mtune=native -DNDEBUG -DENVC_VERSION=\"$(VERSION)\"
	SRC=release
release: clean
release: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BINDIR)/* $(OBJ)/*