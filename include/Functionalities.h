#ifndef _FUNC_H_
#define _FUNC_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include "VeiculoFileManager.h"
    #include "LinhaFileManager.h"
    #include "BTree.h"
    #include "ErrorHandler.h"
    #include "Sort.h"

    int8_t createLinhaBinaryFile(char *csvFilename, char *binFilename);
    int8_t createVeiculoBinaryFile(char *csvFilename, char *binFilename);
    int8_t sortVeiculoFile(char *binFilename, char* sortedFilename);
    int8_t sortLinhaFile(char *binFilename, char* sortedFilename);
    int8_t printVeiculoMerged(char *veiculoBinFilename, char *linhaBinFilename);
    int8_t printVeiculoMergedWithBTree(char *veiculoBinFilename, char *linhaBinFilename, char *linhaBTreeFilename);
    int8_t printVeiculoSortMerge(char *veiculoBinFilename, char *linhaBinFilename);

#endif
