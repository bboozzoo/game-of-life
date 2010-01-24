SRC=main.c
TARGET=rgraph
CFLAGS=-std=c99 -D_GNU_SOURCE $(shell pkg-config --cflags sdl) -Wall -ggdb
LDFLAGS=$(shell pkg-config --libs sdl) -lSDL_gfx

all:  $(TARGET)

$(TARGET): $(SRC:.c=.o)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o $(TARGET)

