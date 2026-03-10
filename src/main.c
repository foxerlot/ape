#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "buffer.h"
#include "window.h"
#include "keys.h"

frameNode* leaves[32];
int numLeaves = 0;

/* Draws the command line at the bottom row, prompts with ':', lets the user
 * type a command, and returns a heap-allocated string (caller must free).
 * Returns NULL if the user cancelled with ESC. */
static char* readCommandLine(int rows, int cols)
{
    int cap = 128;
    char* cmd = malloc(cap);
    if (!cmd) return NULL;
    int len = 0;
    cmd[0] = '\0';

    while (1) {
        move(rows - 1, 0);
        clrtoeol();
        mvaddch(rows - 1, 0, ':');
        mvaddnstr(rows - 1, 1, cmd, cols - 2);
        move(rows - 1, 1 + len);
        refresh();

        int c = getch();

        if (c == ESC) {
            free(cmd);
            return NULL;
        } else if (c == '\n' || c == '\r' || c == CR) {
            break;
        } else if (c == BS || c == DEL || c == MAC_BS) {
            if (len > 0) {
                len--;
                cmd[len] = '\0';
            }
        } else if (c >= CHAR_START && c <= CHAR_END) {
            if (len + 1 >= cap) {
                cap *= 2;
                char* tmp = realloc(cmd, cap);
                if (!tmp) { free(cmd); return NULL; }
                cmd = tmp;
            }
            cmd[len++] = (char)c;
            cmd[len]   = '\0';
        }
    }
    return cmd;
}

static char* trim(char* s)
{
    while (*s == ' ') s++;
    char* end = s + strlen(s) - 1;
    while (end > s && *end == ' ') *end-- = '\0';
    return s;
}

typedef enum { CMD_NONE, CMD_QUIT, CMD_WRITE, CMD_WRITE_QUIT, CMD_ERROR } cmdResult;

static cmdResult runCommand(const char* input, buffer* buf,
                            const char* filename, int* shouldQuit,
                            char* statusMsg, int statusLen)
{
    char copy[256];
    strncpy(copy, input, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
    char* cmd = trim(copy);

    if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0 ||
        strcmp(cmd, "q!") == 0 || strcmp(cmd, "quit!") == 0) {
        *shouldQuit = 1;
        return CMD_QUIT;

    } else if (strncmp(cmd, "w", 1) == 0) {
        const char* rest = cmd + 1;
        if (strncmp(cmd, "write", 5) == 0) rest = cmd + 5;
        rest = rest + strspn(rest, " ");

        int doQuit = 0;
        if (strcmp(rest, "q") == 0 || strcmp(rest, "quit") == 0) {
            doQuit = 1;
            rest   = "";
        }

        const char* target = (*rest != '\0') ? rest : filename;
        if (!target || *target == '\0') {
            snprintf(statusMsg, statusLen, "E: no filename");
            return CMD_ERROR;
        }

        FILE* f = fopen(target, "w");
        if (!f) {
            snprintf(statusMsg, statusLen, "E: cannot open \"%s\"", target);
            return CMD_ERROR;
        }
        for (int i = 0; i < buf->numrows; i++) {
            fwrite(buf->rows[i].line, 1, buf->rows[i].length, f);
            fputc('\n', f);
        }
        fclose(f);
        snprintf(statusMsg, statusLen, "\"%s\" written", target);

        if (doQuit) {
            *shouldQuit = 1;
            return CMD_WRITE_QUIT;
        }
        return CMD_WRITE;

    } else {
        snprintf(statusMsg, statusLen, "E: unknown command \"%s\"", cmd);
        return CMD_ERROR;
    }
}

int main(int argc, char** argv)
{
    if (argc != 2) return 1;
    const char* filename = argv[1];
    FILE* f = fopen(filename, "r");
    buffer* buf1 = fileToBuf(f);
    if (f) fclose(f);

    frameNode* root = newLeaf(buf1, NULL);
    frameNode* focused = root;

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    char statusMsg[256] = "";
    int  statusMsgFresh = 0;
    int shouldQuit = 0;
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
            if (focused->cy >= focused->scroll + focused->height - 2)
                focused->scroll = focused->cy - focused->height + 2;
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

        case CTRL_X: {
            getmaxyx(stdscr, rows, cols);
            char* input = readCommandLine(rows, cols);
            if (input) {
                statusMsgFresh = 1;
                statusMsg[0]   = '\0';
                runCommand(input, focused->item.buf, filename,
                           &shouldQuit, statusMsg, sizeof(statusMsg));
                free(input);
                if (shouldQuit) goto done;
            } else {
                move(rows - 1, 0);
                clrtoeol();
            }
            break;
        }

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
                frameNode* n = closeLeaf(focused);
                if (n) focused = n;
                break;
            }
            }
            break;
        }
        }

        if (focused->cy >= focused->item.buf->numrows)
            focused->cy = focused->item.buf->numrows - 1;
        if (focused->cy < 0) focused->cy = 0;
        if (focused->cx > focused->item.buf->rows[focused->cy].length)
            focused->cx = focused->item.buf->rows[focused->cy].length;
        if (focused->cx < 0) focused->cx = 0;

        getmaxyx(stdscr, rows, cols);
        clear();
        drawNode(getRoot(focused), 0, 0, cols, rows);
        numLeaves = 0;
        countLeaves(getRoot(focused), leaves, &numLeaves);

        if (statusMsgFresh) {
            move(rows - 1, 0);
            clrtoeol();
            mvaddnstr(rows - 1, 0, statusMsg, cols - 1);
            statusMsgFresh = 0;
        }

        if (focused->cy >= focused->scroll &&
            focused->cy <= focused->scroll + focused->height - 2)
            move(focused->y + focused->cy - focused->scroll,
                 focused->x + focused->cx);

        refresh();
    } while (!shouldQuit && (ch = getch()) != CTRL_C);

done:
    endwin();
    free(root);
    freeBuf(buf1);
    return 0;
}
