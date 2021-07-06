/*  SCC0215 - Organização de Arquivos (Turma A)
 *  Grupo 2: Natan Henrique Sanches (11795680) e Lucas Keiti Anbo Mihara (11796472) 
 *  Projeto Prático II: Implementação e otimização de busca em disco (Árvore B)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinhaFileManager.h"
#include "VeiculoFileManager.h"
#include "Utility.h"
#include "BTree.h"

// Every general #define is inside Utility.h
// Every #define related to Linha/Veiculo commands is inside its respective header file

int main(int argc, char *argv[]) {
    char *inputLine = readline(stdin);
    char *trackReference = inputLine;

    char *inputCommand = strsep(&inputLine, SPACE_DELIM);
    if (inputCommand) {
        switch(atoi(inputCommand)) {

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

            // Functionality 9: Creates a B-Tree index file based on binary registry file
            // Related to: 'Veiculo' type binary files
            case CREATE_VEICULO_BTREE: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, LINE_BREAK);
                
                
                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Load Veiculo file header
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

                // Inserts each registry into the B-Tree
                BTreeHeader *newTree = createBTree(indexFilename);
                VeiculoData *newData = (VeiculoData *) malloc(sizeof *newData);
                while (loadVeiculoBinaryRegistry(binFile, newData)) {
                    if (newData->isRemoved != REMOVED_REGISTRY) {
                        int convertedPrefix = convertePrefixo(newData->prefix);
                        int64_t byteOffset = ftell(binFile) - newData->regSize - REG_EXTRA_SIZE;
                        insertOnBTree(newTree, convertedPrefix, byteOffset);
                        freeVeiculoData(newData);
                    }
                }
                free(newData);
                freeVeiculoHeader(&fileHeader);
                fclose(binFile);

                // Rewrites the refreshed header
                // Signs file as "Consistent" ('1')
                rewind(newTree->fp);
                writeBTreeHeader(newTree);

                freeBTree(newTree);
                binarioNaTela(indexFilename);
                break;
            }

            // Functionality 10: Creates a B-Tree index file based on binary registry file
            // Related to: 'Linha' type binary files
            case CREATE_LINHA_BTREE: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, LINE_BREAK);
                
                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    free(trackReference);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Load Linha file header
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

                // Inserts each registry into the B-Tree
                BTreeHeader *newTree = createBTree(indexFilename);
                LinhaData *newData = (LinhaData *) malloc(sizeof *newData);
                while (loadLinhaBinaryRegistry(binFile, newData)) {
                    if (newData->isRemoved != REMOVED_REGISTRY) {
                        int64_t byteOffset = ftell(binFile) - newData->regSize - REG_EXTRA_SIZE;
                        insertOnBTree(newTree, newData->linhaCode, byteOffset);
                        freeLinhaData(newData);
                    }
                }
                free(newData);
                freeLinhaHeader(&fileHeader);
                fclose(binFile);

                // Rewrites the refreshed header
                // Signs file as "Consistent" ('1')
                rewind(newTree->fp);
                writeBTreeHeader(newTree);

                freeBTree(newTree);
                binarioNaTela(indexFilename);
                break;
            }

            // Functionality 11: Searches for a registry based on its prefix (optimized search)
            // Related to: 'Veiculo' type binary files
            case SEARCH_VEICULO_PREFIX: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, SPACE_DELIM);
                char *fieldName = strsep(&inputLine, SPACE_DELIM);
                
                ++inputLine;
                char *fieldValue = strsep(&inputLine, QUOTES_DELIM);

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

                // Load B-Tree file header
                BTreeHeader *indexHeader = openBTree(indexFilename);
                if (!indexHeader) {
                    printf("%s\n", FILE_ERROR);
                    freeVeiculoHeader(&fileHeader);
                    freeBTree(indexHeader);
                    free(trackReference);
                    fclose(binFile);
                    return 0;
                }

                int64_t regOffset = searchBTree(indexHeader, convertePrefixo(fieldValue));
                if (regOffset == EMPTY) {
                    printf("%s\n", REG_NOT_FOUND);
                    freeVeiculoHeader(&fileHeader);
                    free(trackReference);
                    freeBTree(indexHeader);
                    fclose(binFile);
                    return 0;
                }

                // Loads found registry and print it
                VeiculoData *newRegistry = (VeiculoData *) malloc(sizeof *newRegistry);
                fseek(binFile, regOffset, SEEK_SET);
                loadVeiculoBinaryRegistry(binFile, newRegistry);
                printVeiculoRegistry(fileHeader, newRegistry);

                fclose(binFile);
                freeVeiculoHeader(&fileHeader);
                freeVeiculoData(newRegistry);
                free(newRegistry);

                freeBTree(indexHeader);
                break;
            }

            // Functionality 12: Searches for a registry based on its prefix (optimized search)
            // Related to: 'Linha' type binary files
            case SEARCH_LINHA_PREFIX: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, SPACE_DELIM);
                char *fieldName = strsep(&inputLine, SPACE_DELIM);
                
                int32_t fieldValue = atoi(strsep(&inputLine, QUOTES_DELIM));

                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    free(trackReference);
                    fclose(binFile);
                    printf("%s\n", FILE_ERROR);
                    return 0;
                }

                // Load Linha file header
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

                // Load B-Tree file header
                BTreeHeader *indexHeader = openBTree(indexFilename);
                if (!indexHeader) {
                    printf("%s\n", FILE_ERROR);
                    freeBTree(indexHeader);
                    freeLinhaHeader(&fileHeader);
                    free(trackReference);
                    fclose(binFile);
                    return 0;
                }

                int64_t regOffset = searchBTree(indexHeader, fieldValue);
                if (regOffset == EMPTY) {
                    printf("%s\n", REG_NOT_FOUND);
                    freeLinhaHeader(&fileHeader);
                    free(trackReference);
                    freeBTree(indexHeader);
                    fclose(binFile);
                    return 0;
                }

                // Loads found registry and print it
                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);
                fseek(binFile, regOffset, SEEK_SET);
                loadLinhaBinaryRegistry(binFile, newRegistry);
                printLinhaRegistry(fileHeader, newRegistry);

                fclose(binFile);
                freeLinhaHeader(&fileHeader);
                freeLinhaData(newRegistry);
                free(newRegistry);

                freeBTree(indexHeader);
                break;
            }

            // Functionality 13: Insert 'n' new registries from stdin on binary and index files
            // Related to: 'Veiculo' type binary files
            case INSERT_VEICULO_BTREE: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, SPACE_DELIM);
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

                // Loads B-Tree header
                BTreeHeader *indexHeader = openBTree(indexFilename);
                if (!indexHeader) {
                    printf("%s\n", FILE_ERROR);
                    freeBTree(indexHeader);
                    freeVeiculoHeader(&fileHeader);
                    free(trackReference);
                    fclose(binFile);
                    return 0;
                }

                // Signs Linha file as "Inconsistent" ('0')
                rewind(binFile);
                fwrite(INCONSISTENT_FILE, 1, sizeof(char), binFile);
                fflush(binFile);

                // Signs B-Tree file as "Inconsistent" ('0')
                rewind(indexHeader->fp);
                fwrite(INCONSISTENT_FILE, 1, sizeof(char), indexHeader->fp);
                fflush(indexHeader->fp);

                // Writes each new registry at byteNextReg offset 
                fseek(binFile, fileHeader->byteNextReg, SEEK_SET);
                VeiculoData *newRegistry = (VeiculoData *) malloc(sizeof *newRegistry);
                for (int8_t i = 0; i < insertNumber; i++) {
                    readVeiculoRegistry(stdin, newRegistry);
                    if (newRegistry->isRemoved == REMOVED_REGISTRY)
                        fileHeader->removedRegNum++;
                    else {
                        fileHeader->regNumber++;
                        insertOnBTree(indexHeader, convertePrefixo(newRegistry->prefix), ftell(binFile));
                    }

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

                // Rewrites the refreshed header
                // Signs file as "Consistent" ('1')
                rewind(indexHeader->fp);
                writeBTreeHeader(indexHeader);
                freeBTree(indexHeader);

                freeVeiculoHeader(&fileHeader);
                binarioNaTela(indexFilename);
                break;
            }

            // Functionality 14: Insert 'n' new registries from stdin on binary and index files
            // Related to: 'Linha' type binary files
            case INSERT_LINHA_BTREE: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, SPACE_DELIM);
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

                // Loads B-Tree file header
                BTreeHeader *indexHeader = openBTree(indexFilename);
                if (!indexHeader) {
                    printf("%s\n", FILE_ERROR);
                    freeBTree(indexHeader);
                    freeLinhaHeader(&fileHeader);
                    free(trackReference);
                    fclose(binFile);
                    return 0;
                }

                // Signs Linha file as "Inconsistent" ('0')
                rewind(binFile);
                fwrite(INCONSISTENT_FILE, 1, sizeof(char), binFile);
                fflush(binFile);

                // Signs B-Tree file as "Inconsistent" ('0')
                rewind(indexHeader->fp);
                fwrite(INCONSISTENT_FILE, 1, sizeof(char), indexHeader->fp);
                fflush(indexHeader->fp);

                // Writes each new registry at byteNextReg offset 
                fseek(binFile, fileHeader->byteNextReg, SEEK_SET);
                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);
                for (int8_t i = 0; i < insertNumber; i++) {
                    readLinhaRegistry(stdin, newRegistry);
                    if (newRegistry->isRemoved == REMOVED_REGISTRY)
                        fileHeader->removedRegNum++;
                    else {
                        fileHeader->regNumber++;
                        insertOnBTree(indexHeader, newRegistry->linhaCode, ftell(binFile));
                    }

                    writeLinhaRegistryOnBinary(binFile, newRegistry);
                    freeLinhaData(newRegistry);
                }
                free(newRegistry);

                // Refreshes byteNextReg and rewrites the refreshed header of binary file
                // Signs file as "Consistent" ('1')
                fileHeader->byteNextReg = ftell(binFile);
                rewind(binFile);
                writeLinhaHeaderOnBinary(binFile, fileHeader);
                fclose(binFile);

                // Rewrites the refreshed header of index file
                // Signs file as "Consistent" ('1')
                rewind(indexHeader->fp);
                writeBTreeHeader(indexHeader);
                freeBTree(indexHeader);

                freeLinhaHeader(&fileHeader);
                binarioNaTela(indexFilename);
                break;
            }
        }
    }
    free(trackReference);
    return 0;
}
