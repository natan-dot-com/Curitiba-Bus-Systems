#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinhaFileManager.h"
#include "VeiculoFileManager.h"
#include "Utility.h"

#define INTEGER_TYPE 'I'
#define CHAR_TYPE 'C'
#define STRING_TYPE 'S'

#define DELIM " "
#define LINE_BREAK "\n"
#define ERROR "Falha no processamento do arquivo."
#define REG_NOT_FOUND "Registro inexistente."

#define WRITE_VEICULO_BINARY '1'
#define WRITE_LINHA_BINARY '2'
#define SHOW_VEICULO_CONTENT '3'
#define SHOW_LINHA_CONTENT '4'
#define SEARCH_VEICULO_CONTENT '5'
#define SEARCH_LINHA_CONTENT '6'

int main(int argc, char *argv[]) {
    char *inputLine = readline(stdin);
    char *trackReference = inputLine;

    char *inputCommand = strsep(&inputLine, DELIM);
    if (inputCommand) {
        switch(inputCommand[0]) {
            case WRITE_VEICULO_BINARY: {
                char *csvFilename, *binaryFilename;
                csvFilename = strsep(&inputLine, DELIM);
                binaryFilename = strsep(&inputLine, LINE_BREAK);
                if (!writeVeiculoBinary(csvFilename, binaryFilename)) {
                    free(trackReference);
                    printf("%s\n", ERROR);
                    return 1;
                }
                binarioNaTela(binaryFilename);
                break;
            }
            case WRITE_LINHA_BINARY: {
                char *csvFilename, *binaryFilename;
                csvFilename = strsep(&inputLine, DELIM);
                binaryFilename = strsep(&inputLine, LINE_BREAK);
                if (!writeLinhaBinary(csvFilename, binaryFilename)) {
                    free(trackReference);
                    printf("%s\n", ERROR);
                    return 1;
                }
                binarioNaTela(binaryFilename);
                break;
            }
            case SHOW_VEICULO_CONTENT: {
                char *binaryFilename = strsep(&inputLine, LINE_BREAK);
                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    printf("%s\n", ERROR);
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
            case SHOW_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, LINE_BREAK);
                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    printf("%s\n", ERROR);
                    return 1;
                }
                LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
                if (fileHeader->regNumber == 0) {
                    printf("%s\n", REG_NOT_FOUND);
                    return 1;
                }
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
            case SEARCH_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, DELIM);
                char *fieldName = strsep(&inputLine, DELIM);
                char *fieldValue = NULL;
                
                LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);

                void *searchedField = NULL;
                int32_t *fieldSize = NULL;
                char fieldType;
                if (!strcmp(fieldName, "codLinha")) {
                    fieldValue = strsep(&(inputLine), LINE_BREAK);
                    searchedField = (int32_t *) &newRegistry->linhaCode;
                    fieldType = INTEGER_TYPE;
                }
                else if (!strcmp(fieldName, "aceitaCartao")) {
                    ++inputLine;
                    fieldValue = strsep(&(inputLine), "\"");
                    searchedField = (char *) &newRegistry->cardAcceptance;
                    fieldType = CHAR_TYPE;
                }
                else if (!strcmp(fieldName, "nomeLinha")) {
                    ++inputLine;
                    fieldValue = strsep(&(inputLine), "\"");
                    searchedField = (char **) &newRegistry->linhaName;
                    fieldSize = &newRegistry->nameSize;
                    fieldType = STRING_TYPE;
                }
                else if (!strcmp(fieldName, "corLinha")) {
                    ++inputLine;
                    fieldValue = strsep(&(inputLine), "\"");
                    searchedField = (char **) &newRegistry->linhaColor;
                    fieldSize = &newRegistry->colorSize;
                    fieldType = STRING_TYPE;
                }

                FILE *binFile = fopen(binaryFilename, "rb");
                if (!binFile) {
                    printf("%s\n", ERROR);
                    return 1;
                }

                LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
                while (loadLinhaBinaryRegistry(binFile, newRegistry)) {
                    if (newRegistry->isRemoved == VALID_REGISTRY) {
                        switch (fieldType) {
                            case INTEGER_TYPE : {
                                if (*((int32_t *) searchedField) == atoi(fieldValue))
                                    printLinhaRegistry(fileHeader, newRegistry);
                                break;
                            }
                            case CHAR_TYPE : {
                                if (*((char *) searchedField) == fieldValue[0]) 
                                    printLinhaRegistry(fileHeader, newRegistry);
                                break;
                            }
                            case STRING_TYPE : {
                                if (!strncmp(*(char **) searchedField, fieldValue, *fieldSize))
                                    printLinhaRegistry(fileHeader, newRegistry);
                                break;
                            }
                        }
                        freeLinhaData(newRegistry);
                    }
                }
                free(newRegistry);
                freeLinhaHeader(&fileHeader);
                fclose(binFile);
                break;
            }
        }
    }

    free(trackReference);
    return 0;
}
