#ifndef _VEICULOFILEMANAGER_H_
#define _VEICULOFILEMANAGER_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <inttypes.h>
    #include <string.h>
    #include "readline.h"
    #include "Utility.h"

    // Header related constants (size in bytes)
    #define VEICULO_HEADER_SIZE 175
    #define PREFIX_DESC_SIZE 18
    #define DATE_DESC_SIZE 35
    #define SEATS_DESC_SIZE 42
    #define LINE_DESC_SIZE 26
    #define MODEL_DESC_SIZE 17
    #define CATEGORY_DESC_SIZE 20
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

    // Registry related constants (size in bytes)
    #define VEICULO_FIXED_SIZE 31
    #define PREFIX_SIZE 5
    #define DATE_SIZE 10
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

    VeiculoHeader *loadVeiculoBinaryHeader(FILE *binFile);
    bool loadVeiculoBinaryRegistry(FILE *binFile, VeiculoData *registryStruct);

    bool readVeiculoRegistry(FILE *fpVeiculo, VeiculoData *newData);

    bool freeVeiculoHeader(VeiculoHeader **header);
    bool freeVeiculoData(VeiculoData *data);

    bool writeVeiculoBinary(char *csvFilename, char *binFilename);
    bool writeVeiculoHeaderOnBinary(FILE *binFile, VeiculoHeader *headerStruct);
    bool writeVeiculoRegistryOnBinary(FILE *binFile, VeiculoData *registryStruct);
    
    void printVeiculoHeader(VeiculoHeader *header);
    void printVeiculoRegistry(VeiculoHeader *header, VeiculoData *registry);

#endif 
