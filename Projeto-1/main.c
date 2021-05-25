/*  SCC0215 - Organização de Arquivos (Turma A)
 *  Grupo 2: Natan Henrique Sanches (11795680) e Lucas Keiti Anbo Mihara (11796472) 
 *  Projeto Prático I: Leitura e escrita de arquivos binários
*/

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
                    return 0;
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
                    return 0;
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
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }
                
                // Load file header
                VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
                if (!fileHeader) {
                    fclose(binFile);
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }
                if (fileHeader->regNumber == 0) {
                    fclose(binFile);
                    free(trackReference);
                    printf("%s\n", REG_NOT_FOUND);
                    return 0;
                }

                // Read and displays each registry in screen
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
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Load file header
                LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
                if (!fileHeader) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }
                else if (fileHeader->regNumber == 0) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", REG_NOT_FOUND);
                    return 0;
                }

                // Read and displays each registry in screen
                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);
                while (loadLinhaBinaryRegistry(binFile, newRegistry)) {
                    if (newRegistry->isRemoved == VALID_REGISTRY) {
                        printLinhaRegistry(fileHeader, newRegistry);
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
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *fieldName = strsep(&inputLine, SPACE_DELIM);
                char *fieldValue = NULL;

                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Load file header
                VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
                if (!fileHeader) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }
                else if (fileHeader->regNumber == 0) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", REG_NOT_FOUND);
                    return 0;
                }
                
                VeiculoData *newRegistry = (VeiculoData *) malloc(sizeof *newRegistry);

                // Select which field is going to be compared in registry struct with a void pointer
                void *searchedField = NULL;
                int32_t *fieldSize = NULL;
                int32_t fixedFieldSize = 0;
                char fieldType;
                if (!strcmp(fieldName, "prefixo")) {
                    ++inputLine;
                    fieldValue = strsep(&inputLine, QUOTES_DELIM);
                    searchedField = (char **) &newRegistry->prefix;
                    fixedFieldSize = PREFIX_SIZE;
                    fieldType = FIXED_SIZE_STRING_TYPE;
                }
                else if (!strcmp(fieldName, "data")) {
                    ++inputLine;
                    fieldValue = strsep(&inputLine, QUOTES_DELIM);
                    searchedField = (char **) &newRegistry->date;
                    fixedFieldSize = DATE_SIZE;
                    fieldType = FIXED_SIZE_STRING_TYPE;
                }
                else if (!strcmp(fieldName, "quantidadeLugares")) {
                    fieldValue = strsep(&inputLine, LINE_BREAK);
                    searchedField = (int32_t *) &newRegistry->seatsNumber;
                    fieldType = INTEGER_TYPE;
                }
                else if (!strcmp(fieldName, "modelo")) {
                    ++inputLine;
                    fieldValue = strsep(&inputLine, QUOTES_DELIM);
                    searchedField = (char **) &newRegistry->model;
                    fieldSize = &newRegistry->modelSize;
                    fieldType = STRING_TYPE;
                }
                else if (!strcmp(fieldName, "categoria")) {
                    ++inputLine;
                    fieldValue = strsep(&inputLine, QUOTES_DELIM);
                    searchedField = (char **) &newRegistry->category;
                    fieldSize = &newRegistry->categorySize;
                    fieldType = STRING_TYPE;
                }

                // Searches for matches inside file
                int8_t foundRegistries = 0;
                while (loadVeiculoBinaryRegistry(binFile, newRegistry)) {
                    if (newRegistry->isRemoved == VALID_REGISTRY) {
                        switch (fieldType) {
                            case INTEGER_TYPE : {
                                if (*((int32_t *) searchedField) == atoi(fieldValue)) {
                                    printVeiculoRegistry(fileHeader, newRegistry);
                                    foundRegistries++;
                                }
                                break;
                            }
                            case CHAR_TYPE : {
                                if (*((char *) searchedField) == fieldValue[0]) {
                                    printVeiculoRegistry(fileHeader, newRegistry);
                                    foundRegistries++;
                                }
                                break;
                            }
                            case STRING_TYPE : {
                                if (*fieldSize > 0 && !strncmp(*(char **) searchedField, fieldValue, *fieldSize)) {
                                    printVeiculoRegistry(fileHeader, newRegistry);
                                    foundRegistries++;
                                }
                                break;
                            }
                            case FIXED_SIZE_STRING_TYPE : {
                                if (!strncmp(*(char **) searchedField, fieldValue, fixedFieldSize)) {
                                    printVeiculoRegistry(fileHeader, newRegistry);
                                    foundRegistries++;
                                }
                                break;
                            }
                        }
                        freeVeiculoData(newRegistry);
                    }
                }
                fclose(binFile);
                if (foundRegistries == 0)
                    printf("%s\n", REG_NOT_FOUND);

                free(newRegistry);
                freeVeiculoHeader(&fileHeader);
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
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Load file header
                LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
                if (!fileHeader) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }
                else if (fileHeader->regNumber == 0) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", REG_NOT_FOUND);
                    return 0;
                }
                
                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);

                // Select which field is going to be compared in registry struct with a void pointer
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

                // Searches for matches inside file
                int8_t foundRegistries = 0;
                while (loadLinhaBinaryRegistry(binFile, newRegistry)) {
                    if (newRegistry->isRemoved == VALID_REGISTRY) {
                        switch (fieldType) {
                            case INTEGER_TYPE : {
                                if (*((int32_t *) searchedField) == atoi(fieldValue)) {
                                    printLinhaRegistry(fileHeader, newRegistry);
                                    foundRegistries++;
                                }
                                break;
                            }
                            case CHAR_TYPE : {
                                if (*((char *) searchedField) == fieldValue[0]) {
                                    printLinhaRegistry(fileHeader, newRegistry);
                                    foundRegistries++;
                                }
                                break;
                            }
                            case STRING_TYPE : {
                                if (*fieldSize > 0 && !strncmp(*(char **) searchedField, fieldValue, *fieldSize)) {
                                    printLinhaRegistry(fileHeader, newRegistry);
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
            
            // Functionality 7: Insert 'n' new registries from stdin on binary file
            // Related to: 'Veiculo' type binary files
            case INSERT_VEICULO_CONTENT: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                uint8_t insertNumber = atoi(strsep(&inputLine, LINE_BREAK));

                FILE *binFile = fopen(binaryFilename, "rb+");
                if (!binFile) {
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Loads file header
                VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
                if (!fileHeader) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Signs file as "Inconsistent" ('0')
                rewind(binFile);
                fwrite(INCONSISTENT_FILE, 1, sizeof(char), binFile);
                fflush(binFile);

                // Writes each new registry at byteNextReg offset 
                fseek(binFile, fileHeader->byteNextReg, SEEK_SET);
                VeiculoData *newRegistry = (VeiculoData *) malloc(sizeof *newRegistry);
                for (int8_t i = 0; i < insertNumber; i++) {
                    readVeiculoRegistry(stdin, newRegistry);
                    if (newRegistry->isRemoved == REMOVED_REGISTRY)
                        fileHeader->removedRegNum++;
                    else
                        fileHeader->regNumber++;

                    writeVeiculoRegistryOnBinary(binFile, newRegistry);
                    freeVeiculoData(newRegistry);
                }
                free(newRegistry);

                // Refreshes byteNextReg and rewrites the refreshed header
                // Signs file as "Consistent" ('1')
                fileHeader->byteNextReg = ftell(binFile);
                rewind(binFile);
                writeVeiculoHeaderOnBinary(binFile, fileHeader);
                fclose(binFile);

                freeVeiculoHeader(&fileHeader);
                binarioNaTela(binaryFilename);
                break;
            }

            // Functionality 8: Insert 'n' new registries from stdin on binary file
            // Related to: 'Linha' type binary files
            case INSERT_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                uint8_t insertNumber = atoi(strsep(&inputLine, LINE_BREAK));

                FILE *binFile = fopen(binaryFilename, "rb+");
                if (!binFile) {
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Loads file header
                LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
                if (!fileHeader) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Signs file as "Inconsistent" ('0')
                rewind(binFile);
                fwrite(INCONSISTENT_FILE, 1, sizeof(char), binFile);
                fflush(binFile);

                // Writes each new registry at byteNextReg offset
                fseek(binFile, fileHeader->byteNextReg, SEEK_SET);
                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);
                for (uint8_t i = 0; i < insertNumber; i++) {
                    readLinhaRegistry(stdin, newRegistry);
                    if (newRegistry->isRemoved == REMOVED_REGISTRY)
                        fileHeader->removedRegNum++;
                    else
                        fileHeader->regNumber++;

                    writeLinhaRegistryOnBinary(binFile, newRegistry);
                    freeLinhaData(newRegistry);
                }
                free(newRegistry);

                // Refreshes byteNextReg and rewrites the refreshed header
                // Signs file as "Consistent" ('1')
                fileHeader->byteNextReg = ftell(binFile);
                rewind(binFile);
                writeLinhaHeaderOnBinary(binFile, fileHeader);
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
