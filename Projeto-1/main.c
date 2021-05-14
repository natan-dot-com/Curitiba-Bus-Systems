#include <stdio.h>
#include <stdlib.h>
#include "LinhaFileManager.h"

int main(int argc, char *argv[]) {
    char *filename = readline(stdin);
    FILE *binFile = fopen(filename, "rb");
    LinhaHeader *newHeader = loadBinaryHeader(binFile);
    printHeader(newHeader);
    fclose(binFile);
    free(filename);
    return 0;
}
