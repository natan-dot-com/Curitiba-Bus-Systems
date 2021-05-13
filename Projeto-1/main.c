#include <stdio.h>
#include <stdlib.h>
#include "LinhaFileManager.h"

int main(int argc, char *argv[]) {
    char *filename = readline(stdin);
    FILE *binFile = writeLinhaBinary(filename);
    fclose(binFile);
    free(filename);
    return 0;
}
