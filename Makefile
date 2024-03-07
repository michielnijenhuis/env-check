CC = clang
CFLAGS = -Wall
DEFINES = -DNDEBUG -DVERSION=\"1.2.6\"
OPT = -O2

BIN_DIR = bin
DIST_DIR = dist
OUT_DIR = bin

OUT_NAME = envc
OUT = $(OUT_DIR)/$(OUT_NAME)

DEPS = -Idist $(DIST_DIR)/cli.c $(DIST_DIR)/command.c $(DIST_DIR)/argument.c $(DIST_DIR)/colors.c $(DIST_DIR)/cstring.c $(DIST_DIR)/output.c $(DIST_DIR)/option.c $(DIST_DIR)/program.c $(DIST_DIR)/input.c $(DIST_DIR)/usage.c $(DIST_DIR)/fs.c

.PHONY: all
all: build

build:
	$(CC) $(CFLAGS) $(OPT) $(DEFINES) -o $(OUT) $(DIST_DIR)/envc.c $(DEPS)

clean:
	$(RM) $(OUT)

mv:
	sudo cp $(OUT) /usr/local/bin/$(OUT_NAME)