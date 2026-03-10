#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "buffer.h"
#include "window.h"
#include "keys.h"

frameNode* leaves[32];
int numLeaves = 0;

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
        if (ch >= CHAR_START && ch <= CHAR_END) {
            insertChar(&focused->item.buf->rows[focused->cy], focused->cx, ch);
            focused->cx++;
        } else if (ch == BS || ch == DEL || ch == MAC_BS) {
            if (focused->cx == 0) {
                if (focused->cy > 0) {
                    int prevLen = focused->item.buf->rows[focused->cy - 1].length;
                    deleteChar(focused->item.buf, focused->cy, -1);
                    focused->cy--;
                    focused->cx = prevLen;
                }
            } else {
                deleteChar(focused->item.buf, focused->cy, focused->cx - 1);
                focused->cx--;
            }
        } else if (ch == '\n' || ch == '\r') {
            insertCR(focused->item.buf, focused->cy, focused->cx);
            focused->cy++;
            focused->cx = 0;
        }
        switch (ch) {
        case CTRL_B:
            if (focused->cx > 0) focused->cx--;
            break;
        case CTRL_N:
            if (focused->cy < focused->item.buf->numrows - 1) focused->cy++;
            if (focused->cy >= focused->scroll + focused->height - 2) focused->scroll = focused->cy - focused->height + 2;
            break;
        case CTRL_P:
            if (focused->cy > 0) focused->cy--;
            if (focused->cy < focused->scroll) focused->scroll = focused->cy;
            break;
        case CTRL_F:
            if (focused->cx < focused->item.buf->rows[focused->cy].length) focused->cx++;
            break;
        case CTRL_A:
            focused->cx = 0;
            break;
        case CTRL_E:
            focused->cx = focused->item.buf->rows[focused->cy].length;
            break;
        case CTRL_K: {
            row* r = &focused->item.buf->rows[focused->cy];
            r->length = focused->cx;
            r->line[focused->cx] = '\0';
            break;
        }
        case CTRL_X:
            // TODO: implement some sort of commandLine();
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
                frameNode* n = neighborInDir(leaves, numLeaves, focused, 0);
                if (n) focused = n;
                break;
            }
            case 'j': {
                frameNode* n = neighborInDir(leaves, numLeaves, focused, 1);
                if (n) focused = n;
                break;
            }
            case 'k': {
                frameNode* n = neighborInDir(leaves, numLeaves, focused, 2);
                if (n) focused = n;
                break;
            }
            case 'l': {
                frameNode* n = neighborInDir(leaves, numLeaves, focused, 3);
                if (n) focused = n;
                break;
            }
            case 'c': {
                frameNode* n  = closeLeaf(focused);
                if (n) focused = n;
                break;
            }
            }
        break;
        }
        }

        if (focused->cy >= focused->item.buf->numrows) focused->cy = focused->item.buf->numrows - 1;
        if (focused->cy < 0) focused->cy = 0;
        if (focused->cx > focused->item.buf->rows[focused->cy].length) focused->cx = focused->item.buf->rows[focused->cy].length;
        if (focused->cx < 0) focused->cx = 0;

        clear();
        drawNode(getRoot(focused), 0, 0, cols, rows);
        numLeaves = 0;
        countLeaves(getRoot(focused), leaves, &numLeaves);

        if (focused->cy >= focused->scroll && focused->cy <= focused->scroll + focused->height - 2)
            move(focused->y + focused->cy - focused->scroll, focused->x + focused->cx);
        refresh();
    } while ((ch = getch()) != CTRL_C);

    endwin();
    free(root);
    freeBuf(buf1);
    return 0;
}

