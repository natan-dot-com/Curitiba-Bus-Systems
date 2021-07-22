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
    int8_t printVeiculoContent(char *binaryFilename);
    int8_t printLinhaContent(char *binaryFilename);
    int8_t searchVeiculoContent(char *binaryFilename, char *fieldName, char *inputLine);
    int8_t searchLinhaContent(char *binaryFilename, char *fieldName, char *fieldValue);
    int8_t insertVeiculoContent(char *binaryFilename, int8_t insertNumber);
    int8_t insertLinhaContent(char *binaryFilename, int8_t insertNumber);
    int8_t createVeciculoBTree(char *binaryFilename, char *indexFilename);
    int8_t createLinhaBTree(char *binaryFilename, char *indexFilename);
    int8_t searchVeiculoPrefix(char *binaryFilename, char *indexFilename, char *fieldName, char *fieldValue);
    int8_t searchLinhaPrefix(char *binaryFilename, char *indexFilename, char *fieldName, int32_t fieldValue);
    int8_t insertVeiculoBTree(char *binaryFilename, char *indexFilename, int8_t insertNumber);
    int8_t insertLinhaBTree(char *binaryFilename, char *indexFilename, int8_t insertNumber);
    int8_t sortVeiculoFile(char *binFilename, char *sortedFilename);
    int8_t sortLinhaFile(char *binFilename, char *sortedFilename);
    int8_t printVeiculoMerged(char *veiculoBinFilename, char *linhaBinFilename);
    int8_t printVeiculoMergedWithBTree(char *veiculoBinFilename, char *linhaBinFilename, char *linhaBTreeFilename);
    int8_t printVeiculoSortMerge(char *veiculoBinFilename, char *linhaBinFilename);

#endif
