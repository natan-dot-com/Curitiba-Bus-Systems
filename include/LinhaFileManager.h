#ifndef _LINHAFILEMANAGER_H_
#define _LINHAFILEMANAGER_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <inttypes.h>
    #include <string.h>
    #include "readline.h"
    #include "Utility.h"

    // Card acceptance messages
    #define CARD_S_MESSAGE "PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR"
    #define CARD_F_MESSAGE "PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA"
    #define CARD_N_MESSAGE "PAGAMENTO EM CARTAO E DINHEIRO"

    // Null card acceptance field
    #define CARD_NONE '\0'

    // Header related constants (size in bytes)
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

    // Registry related constants (size in bytes)
    #define LINHA_FIXED_SIZE 13
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

    LinhaHeader *loadLinhaBinaryHeader(FILE *binFile);
    bool loadLinhaBinaryRegistry(FILE *binFile, LinhaData *registryStruct); 

    bool readLinhaRegistry(FILE *fpLinha, LinhaData *newData);

    bool freeLinhaHeader(LinhaHeader **header);
    bool freeLinhaData(LinhaData *data);

    bool writeLinhaBinary(char *csvFilename, char *binFilename);
    bool writeLinhaRegistryOnBinary(FILE *binFile, LinhaData *registryStruct);
    bool writeLinhaHeaderOnBinary(FILE *binFile, LinhaHeader *headerStruct);
    void printLinhaHeader(LinhaHeader *header);
    void printLinhaRegistry(LinhaHeader *header, LinhaData *registry);

#endif 
