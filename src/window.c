#include <stdlib.h>
#include <ncurses.h>
#include "window.h"

frameNode* newLeaf(buffer* buf, frameNode* parent)
{
    if (!buf) return NULL;

    frameNode* n = calloc(1, sizeof(frameNode));
    if (!n) return NULL;

    n->type = LEAF_NODE;
    n->item.buf = buf;
    if (parent) n->parent = parent;

    return n;
}

frameNode* newSplit(frameNode* node, splitType split, buffer* newBuf)
{
    if (!node || !newBuf || node->type != LEAF_NODE) return NULL;

    frameNode* parent = node->parent;
    frameNode* n = calloc(1, sizeof(frameNode));
    if (!n) return NULL;
    frameNode* tempNode = calloc(1, sizeof(frameNode));
    if (!tempNode) {
        free(n);
        return NULL;
    }

    n->type = SPLIT_NODE;
    n->item.split = split;
    n->parent = parent;
    n->left = node;
    n->right = tempNode;
    node->parent = n;
    tempNode->parent = n;

    if (parent) { // TODO: add better management of the root node
        if (parent->left == node)
            parent->left = n;
        else if (parent->right == node)
            parent->right = n;
    }

    return n;
}

void drawNode(frameNode* node, int x, int y, int w, int h)
{
    if (!node) return;

    (void)x;
    (void)y;
    (void)w;
    (void)h;

    if (node->type == LEAF_NODE) {
        buffer* buf = node->item.buf;
        for (int i = 0; i < buf->numrows; i++) {
            move(node->y + i, node->x);
            for (int j = 0; j < buf->rows[i].length && j < w; j++)
                addch(buf->rows[i].line[j]);
        }
    } else if (node->type == SPLIT_NODE) {
        if (node->item.split == VERTICAL) {
        } else {}
    }
}

