#include "LinhaFileManager.h"

#define DELIM ","
#define LINE_BREAK "\n"

LinhaHeader *readLinhaHeader(FILE *fpLinha) {
    if (fpLinha) {
        char *lineRead = readline(fpLinha);
        if (lineRead) {
            LinhaHeader *newHeader = (LinhaHeader *) malloc(sizeof *newHeader);

            newHeader->fileStatus = '0';
            newHeader->regNumber = 0;
            newHeader->byteNextReg = 0;
            newHeader->removedRegNum = 0;

            char *trackReference = lineRead;
            newHeader->codeDescription = strdup(strsep(&lineRead, DELIM));
            newHeader->cardDescription = strdup(strsep(&lineRead, DELIM));
            newHeader->nameDescription = strdup(strsep(&lineRead, DELIM));
            newHeader->colorDescription = strdup(strsep(&lineRead, LINE_BREAK));
            free(trackReference);
            return newHeader;
        }
        return NULL;
    }
    return NULL;
}

bool readLinhaRegistry(FILE *fpLinha, LinhaData *newData) {
    if (fpLinha && !feof(fpLinha)) {
        char *lineRead = readline(fpLinha);
        if (lineRead && strlen(lineRead) > 0 && lineRead[0] != -1) {
            char *trackReference = lineRead;
            char *auxString = NULL;

            if (lineRead[0] == '*')
                newData->isRemoved = '0';
            else
                newData->isRemoved = '1';
        
            newData->linhaCode = atoi(strsep(&lineRead, DELIM));
            newData->cardAcceptance = lineRead[0];
            lineRead += 2;

            newData->nameSize = 0;
            newData->linhaName = NULL;
            auxString = strsep(&lineRead, DELIM);
            if (strcmp(auxString, "NULO")) {
                newData->linhaName = strdup(auxString);
                newData->nameSize = strlen(newData->linhaName);
            }

            newData->colorSize = 0;
            newData->linhaColor = NULL;
            auxString = strsep(&lineRead, LINE_BREAK);
            if (strcmp(auxString, "NULO")) {
                newData->linhaColor = strdup(auxString);
                newData->colorSize = strlen(newData->linhaColor);
            }
            free(trackReference);

            newData->regSize = newData->colorSize + newData->nameSize + LINHA_FIXED_SIZE;
            return true;
        }
        free(lineRead);
        return false;
    }
    return false;
}

void freeLinhaHeader(LinhaHeader **header) {
    free((*header)->codeDescription);
    free((*header)->cardDescription);
    free((*header)->nameDescription);
    free((*header)->colorDescription);
    free(*header);
}

void freeLinhaData(LinhaData **data) {
    free((*data)->linhaName);
    free((*data)->linhaColor);
    free(*data);
}

void linhaPrint(LinhaData *data) {

    printf("isRemoved: %c\n", data->isRemoved);
    printf("regSize: %d\n", data->regSize);
    printf("linhaCode: %d\n", data->linhaCode);
    printf("card: %c\n", data->cardAcceptance);
    printf("nameSize: %d\n", data->nameSize);
    printf("linhaName: %s\n", data->linhaName);
    printf("colorSize: %d\n", data->colorSize);
    printf("linhaColor: %s\n", data->linhaColor);
    printf("\n");
}
