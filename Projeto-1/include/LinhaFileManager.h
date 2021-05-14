#ifndef _LINHAFILEMANAGER_H_
#define _LINHAFILEMANAGER_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <inttypes.h>
    #include <string.h>
    #include "readline.h"

    typedef struct linhaHeader_t {
        char fileStatus;
        int64_t byteNextReg;
        int32_t regNumber;
        int32_t removedRegNum;
        char *codeDescription;
        char *cardDescription;
        char *nameDescription;
        char *colorDescription;
    } LinhaHeader;

    typedef struct linhaData_t {
        char isRemoved;
        int32_t regSize;
        int32_t linhaCode;
        char cardAcceptance;
        int32_t nameSize;
        char *linhaName;
        int32_t colorSize;
        char *linhaColor; 
    } LinhaData;

    FILE *writeLinhaBinary(char *csvFilename);
    LinhaHeader *loadBinaryHeader(FILE *binFile);
    bool freeLinhaHeader(LinhaHeader **header);
    void printHeader(LinhaHeader *header);

#endif 
