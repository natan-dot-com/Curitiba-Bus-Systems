/*  SCC0215 - Organização de Arquivos (Turma A)
 *  Grupo 2: Natan Henrique Sanches (11795680) e Lucas Keiti Anbo Mihara (11796472) 
 *  Projeto Prático III: Junção e ordenação de multiplos arquivos de dados
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Functionalities.h"
#include "ErrorHandler.h"

// Every general #define is inside Utility.h
// Every #define related to Linha/Veiculo commands is inside its respective header file
// Each functionality is implemented in a proper file: Functionality.c

int main(int argc, char *argv[]) {
    char *inputLine = readline(stdin);
    char *trackReference = inputLine;

    char *inputCommand = strsep(&inputLine, SPACE_DELIM);
    if (inputCommand) {

        int8_t errorHandler;
        switch(atoi(inputCommand)) {
            case WRITE_VEICULO_BINARY: {
                char *csvFilename, *binaryFilename;
                csvFilename = strsep(&inputLine, SPACE_DELIM);
                binaryFilename = strsep(&inputLine, LINE_BREAK);

                if (errorHandler = createVeiculoBinaryFile(csvFilename, binaryFilename)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(binaryFilename);
                break;
            }
            case WRITE_LINHA_BINARY: {
                char *csvFilename, *binaryFilename;
                csvFilename = strsep(&inputLine, SPACE_DELIM);
                binaryFilename = strsep(&inputLine, LINE_BREAK);

                if (errorHandler = createLinhaBinaryFile(csvFilename, binaryFilename)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(binaryFilename);
                break;
            }
            case SHOW_VEICULO_CONTENT: {
                char *binaryFilename = strsep(&inputLine, LINE_BREAK);

                if (errorHandler = printVeiculoContent(binaryFilename)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                break;
            }
            case SHOW_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, LINE_BREAK);

                if(errorHandler = printLinhaContent(binaryFilename)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }

                break; 
            }
            case SEARCH_VEICULO_CONTENT: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *fieldName = strsep(&inputLine, SPACE_DELIM);
                
                if(errorHandler =  searchVeiculoContent(binaryFilename, fieldName, inputLine)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }

                break;
            }
            case SEARCH_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *fieldName = strsep(&inputLine, SPACE_DELIM);
                
                if(errorHandler =  searchLinhaContent(binaryFilename, fieldName, inputLine)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                break;
            }
            case INSERT_VEICULO_CONTENT: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                uint8_t insertNumber = atoi(strsep(&inputLine, LINE_BREAK));

                if(errorHandler =  insertVeiculoContent(binaryFilename, insertNumber)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(binaryFilename);
                break;
            }
            case INSERT_LINHA_CONTENT: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                uint8_t insertNumber = atoi(strsep(&inputLine, LINE_BREAK));

                if(errorHandler =  insertLinhaContent(binaryFilename, insertNumber)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(binaryFilename);
                break;
            }
            case CREATE_VEICULO_BTREE: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, LINE_BREAK);
                
                if(errorHandler =  createVeciculoBTree(binaryFilename, indexFilename)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(indexFilename);
                break;
            }
            case CREATE_LINHA_BTREE: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, LINE_BREAK);
                
                if(errorHandler =  createLinhaBTree(binaryFilename, indexFilename)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(indexFilename);
                break;
            }
            case SEARCH_VEICULO_PREFIX: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, SPACE_DELIM);
                char *fieldName = strsep(&inputLine, SPACE_DELIM);
                ++inputLine;
                char *fieldValue = strsep(&inputLine, QUOTES_DELIM);

                if(errorHandler =  searchVeiculoPrefix(binaryFilename, indexFilename, fieldName, fieldValue)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                break;
            }
            case SEARCH_LINHA_PREFIX: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, SPACE_DELIM);
                char *fieldName = strsep(&inputLine, SPACE_DELIM);
                int32_t fieldValue = atoi(strsep(&inputLine, QUOTES_DELIM));

                if(errorHandler =  searchLinhaPrefix(binaryFilename, indexFilename, fieldName, fieldValue)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                break;
            }
            case INSERT_VEICULO_BTREE: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, SPACE_DELIM);
                uint8_t insertNumber = atoi(strsep(&inputLine, LINE_BREAK));

                if(errorHandler =  insertVeiculoBTree(binaryFilename, indexFilename, insertNumber)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(indexFilename);
                break;
            }
            case INSERT_LINHA_BTREE: {
                char *binaryFilename = strsep(&inputLine, SPACE_DELIM);
                char *indexFilename = strsep(&inputLine, SPACE_DELIM);
                uint8_t insertNumber = atoi(strsep(&inputLine, LINE_BREAK));

                if(errorHandler =  insertLinhaBTree(binaryFilename, indexFilename, insertNumber)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(indexFilename);
                break;
            }
            case MERGE_VEICULO_LINHA: {
                char *veiculoBinary = strsep(&inputLine, SPACE_DELIM);
                char *linhaBinary = strsep(&inputLine, SPACE_DELIM);
                char *veiculoField = strsep(&inputLine, SPACE_DELIM);
                char *linhaField = strsep(&inputLine, LINE_BREAK);

                if (errorHandler = printVeiculoMerged(veiculoBinary, linhaBinary)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                break;
                
            }
            case MERGE_BTREE_VEICULO_LINHA: {
                char *veiculoBinary = strsep(&inputLine, SPACE_DELIM);
                char *linhaBinary = strsep(&inputLine, SPACE_DELIM);
                char *veiculoField = strsep(&inputLine, SPACE_DELIM);
                char *linhaField = strsep(&inputLine, SPACE_DELIM);
                char *linhaBTree = strsep(&inputLine, LINE_BREAK);

                if (errorHandler = printVeiculoMergedWithBTree(veiculoBinary, linhaBinary, linhaBTree)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                break;
            }
            case SORT_VEICULO_FILE: {
                char *veiculoBinary = strsep(&inputLine, SPACE_DELIM);
                char *sortedBinary = strsep(&inputLine, SPACE_DELIM);
                char *sortField = strsep(&inputLine, LINE_BREAK);

                if (errorHandler = sortVeiculoFile(veiculoBinary, sortedBinary)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(sortedBinary);
                break;
            }

            case SORT_LINHA_FILE: {
                char *linhaBinary = strsep(&inputLine, SPACE_DELIM);
                char *sortedBinary = strsep(&inputLine, SPACE_DELIM);
                char *sortField = strsep(&inputLine, LINE_BREAK);

                if (errorHandler = sortLinhaFile(linhaBinary, sortedBinary)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                binarioNaTela(sortedBinary);
                break;
            }

            case MERGE_LINHA_VEICULO: {
                char *veiculoBinary = strsep(&inputLine, SPACE_DELIM);
                char *linhaBinary = strsep(&inputLine, SPACE_DELIM);
                char *veiculoField = strsep(&inputLine, SPACE_DELIM);
                char *linhaField = strsep(&inputLine, LINE_BREAK);

                if (errorHandler = printVeiculoSortMerge(veiculoBinary, linhaBinary)) {
                    free(trackReference);
                    printf("%s\n", matchError(errorHandler));
                    return 0;
                }
                break;
            }
        }
    }
    free(trackReference);
    return 0;
}