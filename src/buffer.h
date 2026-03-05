#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

typedef struct {
    int length;
    char* line;
} row;

typedef struct {
    row* rows;
    int numrows;
    int capacity;
} buffer;

buffer* newBuf(void);
buffer* fileToBuf(FILE*);
FILE* bufToFile(buffer*);
void insertChar(row*, int, char);
void deleteChar(buffer*, int, int);
void insertCR(buffer*, int, int);
void deleteCR(buffer*, int);
void freeBuf(buffer*);
long int fileGetline(char**, size_t*, FILE*);
void printBuf(buffer*);

#endif
