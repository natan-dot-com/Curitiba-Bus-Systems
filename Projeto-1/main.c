#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinhaFileManager.h"
#include "VeiculoFileManager.h"
#include "Utility.h"

// Every general #define is inside Utility.h
// Every #define related to Linha/Veiculo commands is inside its respective header file

int main(int argc, char *argv[]) {
    char *inputLine = readline(stdin);
    char *trackReference = inputLine;

    char *inputCommand = strsep(&inputLine, SPACE_DELIM);
    if (inputCommand) {
        switch(inputCommand[0]) {

            // Functionality 1: Writes a binary file from existent CSV file
            // Related to: 'Veiculo' type CSV files
            case WRITE_VEICULO_BINARY: {
                char *csvFilename, *binaryFilename;
                csvFilename = strsep(&inputLine, SPACE_DELIM);
                binaryFilename = strsep(&inputLine, LINE_BREAK);
                if (!writeVeiculoBinary(csvFilename, binaryFilename)) {
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }
                binarioNaTela(binaryFilename);
                break;
            }

            // Functionality 2: Writes a binary file from existent CSV file
            // Related to: 'Linha' type CSV files
            case WRITE_LINHA_BINARY: {
                char *csvFilename, *binaryFilename;
                csvFilename = strsep(&inputLine, SPACE_DELIM);
                binaryFilename = strsep(&inputLine, LINE_BREAK);
                if (!writeLinhaBinary(csvFilename, binaryFilename)) {
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }
                binarioNaTela(binaryFilename);
                break;
            }
            
            // Functionality 3: Shows on stdout every registry from binary file on screen
            // Related to: 'Veiculo' type binary files
            case SHOW_VEICULO_CONTENT: {
                char *binaryFilename = strsep(&inputLine, LINE_BREAK);
                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }
                VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
                if (fileHeader->regNumber == 0) {
                    printf("%s\n", REG_NOT_FOUND);
                    return 1;
                }
                VeiculoData *newRegistry = (VeiculoData *) malloc(sizeof *newRegistry);
                while (loadVeiculoBinaryRegistry(binFile, newRegistry)) {
                    if (newRegistry->isRemoved == VALID_REGISTRY) {
                        printVeiculoRegistry(fileHeader, newRegistry);
                        freeVeiculoData(newRegistry);
                    }
                }
                free(newRegistry);
                freeVeiculoHeader(&fileHeader);
                fclose(binFile);
                break; 
            }
            
            // Functionality 4: Shows on stdout every registry from binary file on screen
            // Related to: 'Linha' type binary files
            case SHOW_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, LINE_BREAK);
                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }

                LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
                if (!fileHeader) {
                    fclose(binFile);
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }
                else if (fileHeader->regNumber == 0) {
                    fclose(binFile);
                    printf("%s\n", REG_NOT_FOUND);
                    return 1;
                }

                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);
                while (loadLinhaBinaryRegistry(binFile, newRegistry)) {
                    if (newRegistry->isRemoved == VALID_REGISTRY) {
                        printLinhaRegistry(fileHeader, newRegistry, NONE);
                        freeLinhaData(newRegistry);
                    }
                }
                free(newRegistry);
                freeLinhaHeader(&fileHeader);
                fclose(binFile);
                break; 
            }
            
            // Functionality 5: Searches for a specified field inside binary file
            // Related to: 'Veiculo' type binary files
            case SEARCH_VEICULO_CONTENT: {
                break;
            }

            // Functionality 6: Searches for a specified field inside binary file
            // Related to: 'Linha' type binary files
            case SEARCH_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *fieldName = strsep(&inputLine, SPACE_DELIM);
                char *fieldValue = NULL;

                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }

                LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
                if (!fileHeader) {
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }
                else if (fileHeader->regNumber == 0) {
                    printf("%s\n", REG_NOT_FOUND);
                    return 1;
                }
                
                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);

                void *searchedField = NULL;
                int32_t *fieldSize = NULL;
                char fieldType;
                if (!strcmp(fieldName, "codLinha")) {
                    fieldValue = strsep(&inputLine, LINE_BREAK);
                    searchedField = (int32_t *) &newRegistry->linhaCode;
                    fieldType = INTEGER_TYPE;
                }
                else if (!strcmp(fieldName, "aceitaCartao")) {
                    ++inputLine;
                    fieldValue = strsep(&inputLine, QUOTES_DELIM);
                    searchedField = (char *) &newRegistry->cardAcceptance;
                    fieldType = CHAR_TYPE;
                }
                else if (!strcmp(fieldName, "nomeLinha")) {
                    ++inputLine;
                    fieldValue = strsep(&inputLine, QUOTES_DELIM);
                    searchedField = (char **) &newRegistry->linhaName;
                    fieldSize = &newRegistry->nameSize;
                    fieldType = STRING_TYPE;
                }
                else if (!strcmp(fieldName, "corLinha")) {
                    ++inputLine;
                    fieldValue = strsep(&inputLine, QUOTES_DELIM);
                    searchedField = (char **) &newRegistry->linhaColor;
                    fieldSize = &newRegistry->colorSize;
                    fieldType = STRING_TYPE;
                }

                int8_t foundRegistries = 0;
                while (loadLinhaBinaryRegistry(binFile, newRegistry)) {
                    if (newRegistry->isRemoved == VALID_REGISTRY) {
                        switch (fieldType) {
                            case INTEGER_TYPE : {
                                if (*((int32_t *) searchedField) == atoi(fieldValue)) {
                                    printLinhaRegistry(fileHeader, newRegistry, fieldName);
                                    foundRegistries++;
                                }
                                break;
                            }
                            case CHAR_TYPE : {
                                if (*((char *) searchedField) == fieldValue[0]) {
                                    printLinhaRegistry(fileHeader, newRegistry, fieldName);
                                    foundRegistries++;
                                }
                                break;
                            }
                            case STRING_TYPE : {
                                if (!strncmp(*(char **) searchedField, fieldValue, *fieldSize)) {
                                    printLinhaRegistry(fileHeader, newRegistry, fieldName);
                                    foundRegistries++;
                                }
                                break;
                            }
                        }
                        freeLinhaData(newRegistry);
                    }
                }
                fclose(binFile);
                if (foundRegistries == 0)
                    printf("%s\n", REG_NOT_FOUND);

                free(newRegistry);
                freeLinhaHeader(&fileHeader);
                break;
            }
            
            // Functionality 8: Insert 'n' new registries from stdin on binary file
            // Related to: 'Veiculo' type binary files
            case INSERT_VEICULO_CONTENT: {
                break;
            }

            // Functionality 8: Insert 'n' new registries from stdin on binary file
            // Related to: 'Linha' type binary files
            case INSERT_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                uint8_t insertNumber = atoi(strsep(&inputLine, LINE_BREAK));

                FILE *binFile = fopen(binaryFilename, "rb+");
                if (!binFile) {
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }

                LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
                if (!fileHeader) {
                    printf("%s\n", FILE_ERROR);
                    return 1;
                }

                rewind(binFile);
                fwrite(INCONSISTENT_FILE, 1, sizeof(char), binFile);
                fseek(binFile, fileHeader->byteNextReg, SEEK_SET);

                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);
                for (uint8_t i = 0; i < insertNumber; i++) {
                    readLinhaRegistry(stdin, newRegistry);
                    if (newRegistry->isRemoved == REMOVED_REGISTRY)
                        fileHeader->removedRegNum++;
                    else
                        fileHeader->regNumber++;

                    writeRegistryOnBinary(binFile, newRegistry);
                    freeLinhaData(newRegistry);
                }
                free(newRegistry);

                fileHeader->byteNextReg = ftell(binFile);
                rewind(binFile);
                writeHeaderOnBinary(binFile, fileHeader);
                fclose(binFile);

                freeLinhaHeader(&fileHeader);
                binarioNaTela(binaryFilename);
                break;
            }
        }
    }

    free(trackReference);
    return 0;
}
