#ifndef _VEICULOFILEMANAGER_H_
#define _VEICULOFILEMANAGER_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <inttypes.h>
    #include <string.h>
    #include "readline.h"

    typedef struct veiculoHeader_t {
        char fileStatus;
        int64_t byteNextReg;
        int32_t regNumber;
        int32_t removedRegNum;
        char *prefixDescription;
        char *dateDescription;
        char *seatsDescription;
        char *lineDescription;
        char *modelDescription;
        char *categoryDescription;
    } VeiculoHeader;

    typedef struct veiculoData_t {
        char isRemoved;
        int32_t regSize;
        char *prefix;
        char *date;
        int32_t seatsNumber;
        int32_t linhaCode;
        int32_t modelSize;
        char *model;
        int32_t categorySize;
        char *category;
    } VeiculoData;

    bool writeVeiculoBinary(char *csvFilename, char *binFilename);
    bool freeVeiculoHeader(VeiculoHeader **header);
    VeiculoHeader *loadVeiculoBinaryHeader(FILE *binFile);
    bool loadVeiculoBinaryRegistry(FILE *binFile, VeiculoData *registryStruct);
    bool freeVeiculoData(VeiculoData *data);
    
    void printVeiculoHeader(VeiculoHeader *header);
    void printVeiculoRegistry(VeiculoHeader *header, VeiculoData *registry);

#endif 
