#include <stdio.h>
#include <stdlib.h>
#include "LinhaFileManager.h"
#include "VeiculoFileManager.h"

int main(int argc, char *argv[]) {
    char *filename = readline(stdin);
    //FILE *binFile = fopen(filename, "rb");
    //VeiculoHeader *newHeader = loadVeiculoBinaryHeader(binFile);
    //printVeiculoHeader(newHeader);

    FILE *binFile = writeVeiculoBinary(filename);
    fclose(binFile);
    free(filename);
    return 0;
}
