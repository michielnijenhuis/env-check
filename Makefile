VERSION = $(shell git describe --tags --abbrev=0 | sed 's/^v//')

CC = clang
CFLAGS = -Wall
DEFINES = -DNDEBUG -DENVC_VERSION=\"$(VERSION)\"
OPT = -O2

BIN_DIR = bin
DIST_DIR = dist
OUT_DIR = $(BIN_DIR)

OUT_NAME = envc
OUT = $(OUT_DIR)/$(OUT_NAME)

DEPS = -I$(DIST_DIR) $(DIST_DIR)/main.c $(DIST_DIR)/argument.c $(DIST_DIR)/cli.c $(DIST_DIR)/colors.c $(DIST_DIR)/fs.c $(DIST_DIR)/command.c $(DIST_DIR)/cstring.c $(DIST_DIR)/input.c $(DIST_DIR)/option.c $(DIST_DIR)/output.c $(DIST_DIR)/program.c $(DIST_DIR)/usage.c


.PHONY: all
all: build

build:
	$(CC) $(CFLAGS) $(OPT) $(DEFINES) -o $(OUT) $(DEPS)

clean:
	$(RM) $(OUT)

mv:
	sudo cp $(OUT) /usr/local/bin/$(NAME)