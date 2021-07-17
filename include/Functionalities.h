#ifndef _FUNC_H_
#define _FUNC_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include "VeiculoFileManager.h"
    #include "LinhaFileManager.h"
    #include "BTree.h"

    bool createLinhaBinaryFile(char *csvFilename, char *binFilename);
    bool createVeiculoBinaryFile(char *csvFilename, char *binFilename);
    bool sortVeiculoFile(char *binFilename, char* sortedFilename);
    bool sortLinhaFile(char *binFilename, char* sortedFilename);
    void printVeiculoMerged(char *veiculoBinFilename, char *linhaBinFilename);
    void printVeiculoMergedWithBTree(char *veiculoBinFilename, char *linhaBinFilename, char *linhaBTreeFilename);
    void printVeiculoSortMerge(char *veiculoBinFilename, char *linhaBinFilename);

#endif