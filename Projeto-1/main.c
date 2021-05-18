#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinhaFileManager.h"
#include "VeiculoFileManager.h"
#include "Utility.h"

#define DELIM " "
#define LINE_BREAK "\n"
#define ERROR "Falha no processamento do arquivo."
#define REG_NOT_FOUND "Registro inexistente."

#define WRITE_VEICULO_BINARY '1'
#define WRITE_LINHA_BINARY '2'
#define SHOW_VEICULO_CONTENT '3'
#define SHOW_LINHA_CONTENT '4'

int main(int argc, char *argv[]) {
    char *inputLine = readline(stdin);
    char *trackReference = inputLine;

    char *inputCommand = strsep(&inputLine, DELIM);
    if (inputCommand) {
        switch(inputCommand[0]) {
            case WRITE_VEICULO_BINARY: {
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
        }
    }

    free(trackReference);
    return 0;
}
