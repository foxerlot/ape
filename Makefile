CC          = gcc
WINCC       = x86_64-w64-mingw32-gcc

CFLAGS      = -O3 -Wall -Wextra -Werror -Wpedantic -std=c99
LFLAGS      = -lncurses
WINLFLAGS   = -L/mingw64/lib -lncursesw
WINCFLAGS   = -Wall -Wextra -Werror -Wpedantic -std=c99 -I/mingw64/include/ncurses
TARGET      = ape
WIN_TARGET  = ape.exe

SOURCES     = $(wildcard src/*.c)

all: clean build

build:
	$(CC) $(CFLAGS) $(SOURCES) $(LFLAGS) -o $(TARGET)

windows:
	$(WINCC) $(WINCFLAGS) $(SOURCES) $(WINLFLAGS) -o $(WIN_TARGET)

clean:
	rm -f $(TARGET) $(WIN_TARGET)

.PHONY: all build windows clean
