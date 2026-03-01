#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/types.h>
#include "buffer.h"

#ifdef WIN32
#   include <io.h>
#   define F_OK 0
#   define access _access
#else
#   include <unistd.h>
#endif

int bufcount = 0;

void parseArgs(int, char**);

int main(int argc, char** argv)
{
    parseArgs(argc, argv);
    editorInit();
    sleep(2);
    editorCleanup(EXIT_SUCCESS);
    return 0;
}

void parseArgs(int argc, char** argv)
{
    if (argc != 2) editorCleanup(EXIT_FAILURE);
    FILE* f = fopen(argv[1], "r");
    if (!f) editorCleanup(EXIT_FAILURE);

    buflist[bufcount++] = fileToBuf(f);
    fclose(f);
}
