ifeq ($(OS),Windows_NT)
	CC = x86_64-w64-mingw32-gcc
	TARGET = ape.exe
	CFLAGS   = -O3 -Wall -Wextra -Werror -Wpedantic -std=c99 -I/mingw64/include/ncurses
	LFLAGS   = -L/mingw64/lib -lncursesw
else
	CC = gcc
	TARGET = ape
	CFLAGS      = -O3 -Wall -Wextra -Werror -Wpedantic -std=c99
	LFLAGS      = -lncurses
endif

BLACK   = \033[0;30m
RED     = \033[0;31m
GREEN   = \033[0;32m
YELLOW  = \033[0;33m
BLUE    = \033[0;34m
MAGENTA = \033[0;35m
CYAN    = \033[0;36m
WHITE   = \033[0;37m
NC      = \033[0m

SOURCES     = $(wildcard src/*.c)

all: clean build

debug: CFLAGS = -g -Wall -Wextra -std=c99
debug: clean build

install: build
	@echo "$(BLUE)Installing $(TARGET)...$(NC)"
	cp $(TARGET) /usr/local/bin/

uninstall:
	@echo "$(RED)Uninstalling $(TARGET)...$(NC)"
	cp $(TARGET) /usr/local/bin/

build: $(SOURCES)
	@echo "$(GREEN)Building $(TARGET)...$(NC)"
	$(CC) $(CFLAGS) $^ $(LFLAGS) -o $(TARGET)

clean:
	@echo "$(RED)Cleaning $(TARGET)...$(NC)"
	rm -f $(TARGET)

help:
	@echo "$(BLACK)Available targets:$(NC)"
	@echo "  make build             Build program"
	@echo "  make debug             Debug build"
	@echo "  make clean             Remove build files"
	@echo "  (sudo) make install    Install to /usr/local/bin"
	@echo "  (sudo) make uninstall  Remove from /usr/local/bin"

.PHONY: all debug build clean install uninstall help
