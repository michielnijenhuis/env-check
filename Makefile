.PHONY: all
all: build

CC = clang
CFLAGS = -Wall
OPT = -O2
BIN_DIR = bin
NAME = envc
OUT = $(BIN_DIR)/$(NAME)

build:
	sh ./build.sh $(CC) $(CFLAGS) $(OPT) -o $(OUT)

clean:
	$(RM) $(OUT)

mv:
	sudo cp $(OUT) /usr/local/bin/$(NAME)