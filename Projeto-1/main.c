#include <stdio.h>
#include <stdlib.h>
#include "LinhaFileManager.h"

int main(int argc, char *argv[])
{
    int flag;
    char filename[] = "test_files/linha.csv";
    FILE *fp = fopen(filename, "r");
    LinhaHeader *linhaHeader = readLinhaHeader(fp);
    LinhaData *linhaData;
    
    do {
        linhaData = (LinhaData*)malloc(sizeof(LinhaData));
        if(flag = readLinhaRegistry(fp, linhaData) == 1) {
            linhaPrint(linhaData);
            freeLinhaData(&linhaData);
        } else {
            free(linhaData);
        }
    } while(flag);

    freeLinhaHeader(&linhaHeader);
    fclose(fp);
    return 0;
}
