#include <stdlib.h>
#include <limits.h>
#include <ncurses.h>
#include "window.h"
#include "buffer.h"

/*
 * Returns a new leaf.
 */
frameNode* newLeaf(buffer* buf, frameNode* parent)
{
    if (!buf) return NULL;

    frameNode* n = calloc(1, sizeof(frameNode));
    if (!n) return NULL;

    n->type = LEAF_NODE;
    n->item.buf = buf;
    n->parent = parent;

    return n;
}

/*
 * Returns a new split node whose left node is `node` and right node is a new node.
 */
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

    tempNode->type = LEAF_NODE;
    tempNode->item.buf = newBuf;
    tempNode->parent = n;
    node->parent = n;

    if (parent) { // TODO: add better management of the root node
        if (parent->left == node)
            parent->left = n;
        else if (parent->right == node)
            parent->right = n;
    }

    return n;
}

/*
 * Draw an individual node. Recurses if `node->type == SPLIT_NODE`.
 */
void drawNode(frameNode* node, int x, int y, int w, int h)
{
    if (!node || w <= 0 || h <= 0) return; // TODO: make limits for how many windows can be on screen

    if (node->type == LEAF_NODE) {
        node->x = x;
        node->y = y;
        node->width = w;
        node->height = h;
        const buffer* buf = node->item.buf;
        for (int i = 0; (i + node->scroll) < buf->numrows && i < h - 1; i++) {
            const row* r = &buf->rows[i + node->scroll];
            const int len = r->length < w ? r->length : w;
            move(y + i, x);
            addnstr(r->line, len);
        }
        mvhline(y + h - 1, x, '-', w); // TODO: add statusline
    } else if (node->type == SPLIT_NODE) {
        if (node->item.split == VERTICAL) {
            const int wLeft = w * 0.5; // TODO: add better ratios
            const int wRight = w - wLeft - 1; // -1 to account for the border
            drawNode(node->left, x, y, wLeft, h);
            mvvline(y, x + wLeft, '|', h);
            drawNode(node->right, x + wLeft + 1, y, wRight, h);
        } else { // node->type == SPLIT_NODE
            const int hTop = h * 0.5;
            const int hBot = h - hTop;
            drawNode(node->left, x, y, w, hTop);
            drawNode(node->right, x, y + hTop, w, hBot);
        }
    }
}

/*
 * Returns the frameNode closest to `focused` in direction `dir`.
 * Does not work very well lol...
 * dir: 0=left, 1=down, 2=up, 3=right
 */
frameNode* neighborInDir(frameNode** leaves, int count, frameNode* focused, int dir)
{
    if (!leaves || !focused || count <= 0) return NULL;
    frameNode* best = NULL;
    int bestDist = INT_MAX;
    const int fx  = focused->x;
    const int fy  = focused->y;
    const int fx2 = fx + focused->width;
    const int fy2 = fy + focused->height;

    for (int i = 0; i < count; i++) {
        frameNode* l = leaves[i];
        if (l == focused) continue;

        int dist;
        switch (dir) {
            case 0: if (l->x + l->width  > fx)  continue; dist = fx  - l->x; break;
            case 1: if (l->y             < fy2)  continue; dist = l->y - fy;  break;
            case 2: if (l->y + l->height > fy)   continue; dist = fy  - l->y; break;
            case 3: if (l->x             < fx2)  continue; dist = l->x - fx;  break;
            default: continue;
        }
        if (dist < bestDist) { bestDist = dist; best = l; }
    }
    return best;
}

/*
 * Closes leaf and returns sibling as the new focus, or the new root if grandparent is NULL.
 */
frameNode* closeLeaf(frameNode* leaf)
{
    if (!leaf || leaf->type != LEAF_NODE) return NULL;
    frameNode* parent = leaf->parent;
    if (!parent) {
        free(leaf);
        return NULL;
    }

    frameNode* sibling = (parent->left == leaf) ? parent->right : parent->left;
    frameNode* grandparent = parent->parent;

    sibling->parent = grandparent;

    if (grandparent) {
        if (grandparent->left == parent)       grandparent->left  = sibling;
        else if (grandparent->right == parent) grandparent->right = sibling;
    }

    free(leaf);
    free(parent);

    return sibling;
}

void countLeaves(frameNode* node, frameNode** leaves, int* count)
{
    if (!node) return;
    if (node->type == LEAF_NODE)
        leaves[(*count)++] = node;
    else {
        countLeaves(node->left, leaves, count);
        countLeaves(node->right, leaves, count);
    }
}

frameNode* getRoot(frameNode* node)
{
    if (!node) return NULL;
    while (node->parent)
        node = node->parent;
    return node;
}
