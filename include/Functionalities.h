#ifndef _FUNC_H_
#define _FUNC_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include "VeiculoFileManager.h"
    #include "LinhaFileManager.h"
    #include "BTree.h"
    #include "Sort.h"

    bool createLinhaBinaryFile(char *csvFilename, char *binFilename);
    bool createVeiculoBinaryFile(char *csvFilename, char *binFilename);
    bool sortVeiculoFile(char *binFilename, char* sortedFilename);
    bool sortLinhaFile(char *binFilename, char* sortedFilename);
    bool printVeiculoMerged(char *veiculoBinFilename, char *linhaBinFilename);
    bool printVeiculoMergedWithBTree(char *veiculoBinFilename, char *linhaBinFilename, char *linhaBTreeFilename);
    bool printVeiculoSortMerge(char *veiculoBinFilename, char *linhaBinFilename);

#endif
