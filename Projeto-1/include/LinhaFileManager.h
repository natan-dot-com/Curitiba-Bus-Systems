#ifndef _LINHAFILEMANAGER_H_
#define _LINHAFILEMANAGER_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <inttypes.h>
    #include <string.h>
    #include "readline.h"

    #define LINHA_HEADER_SIZE 82
    #define CODE_DESC_SIZE 15
    #define CARD_DESC_SIZE 13
    #define NAME_DESC_SIZE 13
    #define COLOR_DESC_SIZE 24
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

    #define LINHA_FIXED_SIZE 13
    #define REMOVED_REGISTRY '0'
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

#endif 
