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
    int cx, cy;
    int scroll;

    struct frameNode* parent;
    struct frameNode* left;
    struct frameNode* right;
} frameNode;

frameNode* newLeaf(buffer*, frameNode*);
frameNode* newSplit(frameNode*, splitType, buffer*);
void drawNode(frameNode*, int, int, int, int);
frameNode* neighborInDir(frameNode**, int, frameNode*, int);
frameNode* closeLeaf(frameNode*);
void countLeaves(frameNode*, frameNode**, int*);
frameNode* getRoot(frameNode*);

#endif
