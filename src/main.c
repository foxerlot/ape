#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "buffer.h"
#include "window.h"

int main(int argc, char** argv)
{
    if (argc != 2) return 1;
    FILE* f = fopen(argv[1], "r");
    buffer* buf1 = fileToBuf(f);
    fclose(f);

    frameNode* root = newLeaf(buf1, NULL);
    free(root);

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    clear();
    refresh();

    int ch;
    while ((ch = getch()) != 'q');

    endwin();
    freeBuf(buf1);
    return 0;
}

