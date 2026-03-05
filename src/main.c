#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "buffer.h"
#include "window.h"
#include "keys.h"

frameNode* leaves[32];
int numleaves = 0;

int main(int argc, char** argv)
{
    if (argc != 2) return 1;
    FILE* f = fopen(argv[1], "r");
    buffer* buf1 = fileToBuf(f);
    fclose(f);

    frameNode* root = newLeaf(buf1, NULL);
    frameNode* focused = root;

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int ch = 0;
    do {
        switch (ch) {
            case 'h':
                if (focused->cx > 0) focused->cx--;
                break;
            case 'j':
                if (focused->cy < focused->item.buf->numrows - 1) focused->cy++;
                break;
            case 'k':
                if (focused->cy > 0) focused->cy--;
                break;
            case 'l':
                if (focused->cx < focused->item.buf->rows[focused->cy].length) focused->cx++;
                break;
            case CTRL_W: {
                             ch = getch();
                             switch (ch) {
                                 case 'n': {
                                               frameNode* n = newSplit(focused, HORIZONTAL, focused->item.buf);
                                               if (n) focused = n->right;
                                               break;
                                           }
                                 case 'v': {
                                               frameNode* n = newSplit(focused, VERTICAL, focused->item.buf);
                                               if (n) focused = n->right;
                                               break;
                                           }
                                 case 'h': {
                                               frameNode* n = neighborInDir(leaves, numleaves, focused, 0);
                                               if (n) focused = n;
                                               break;
                                           }
                                 case 'j': {
                                               frameNode* n = neighborInDir(leaves, numleaves, focused, 1);
                                               if (n) focused = n;
                                               break;
                                           }
                                 case 'k': {
                                               frameNode* n = neighborInDir(leaves, numleaves, focused, 2);
                                               if (n) focused = n;
                                               break;
                                           }
                                 case 'l': {
                                               frameNode* n = neighborInDir(leaves, numleaves, focused, 3);
                                               if (n) focused = n;
                                               break;
                                           }
                             }
                             break;
                         }
        }
        clear();
        drawNode(getRoot(focused), 0, 0, cols, rows);
        numleaves = 0;
        collectLeaves(getRoot(focused), leaves, &numleaves);
        move(focused->y + focused->cy, focused->x + focused->cx);
        refresh();
    } while ((ch = getch()) != 'q');

    endwin();
    free(root);
    freeBuf(buf1);
    return 0;
}

