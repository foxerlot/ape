#ifndef WINDOW_H
#define WINDOW_H

#include "buffer.h"

typedef enum {
    LEAF_NODE,
    SPLIT_NODE
} nodeType;

typedef enum {
    VERTICAL,
    HORIZONTAL
} splitType;

typedef struct frameNode {
    nodeType type;

    union {
        buffer* buf;
        splitType split;
    } item;

    int x, y;
    int width, height;

    struct frameNode* parent;
    struct frameNode* left;
    struct frameNode* right;
} frameNode;

frameNode* newLeaf(buffer*, frameNode*);
frameNode* newSplit(frameNode*, splitType, buffer*);

#endif
