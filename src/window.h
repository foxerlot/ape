#ifndef WINDOW_H
#define WINDOW_H

#include "buffer.h"

typedef enum {
    FR_LEAF,
    FR_ROW,
    FR_COL,
} frame_type;

typedef struct window {
    buffer *buf;

    int cursor_row;
    int cursor_col;

    int row_offset;
    int col_offset;

    int x, y;
    int width, height;
} window;

typedef struct frame {
    frame_type type;

    struct frame *parent;
    struct frame *child;
    struct frame *next;
    struct frame *prev;

    window *win;

    int x, y;
    int width, height;
} frame;

#endif
