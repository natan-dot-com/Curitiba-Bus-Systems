#include "LinhaFileManager.h"

#define DELIM ","
#define LINE_BREAK "\n"
#define BIN_FILE_EXT ".bin"

// Header related constants (size in bytes)
#define LINHA_HEADER_SIZE 82
#define CODE_DESC_SIZE 15
#define CARD_DESC_SIZE 13
#define NAME_DESC_SIZE 13
#define COLOR_DESC_SIZE 24
#define INCONSISTENT_FILE "0"
#define CONSISTENT_FILE '1'

// Registry related constants (size in bytes)
#define LINHA_FIXED_SIZE 13
#define REMOVED_REGISTRY '0'
#define VALID_REGISTRY '1'

// (Static) Parses first CSV file line to the header struct
// Return value: A pointer to the built struct (LinhaHeader *)
static LinhaHeader *readLinhaHeader(FILE *fpLinha) {
    if (fpLinha) {
        char *lineRead = readline(fpLinha);
        if (lineRead) {
            LinhaHeader *newHeader = (LinhaHeader *) malloc(sizeof *newHeader);

            newHeader->fileStatus = CONSISTENT_FILE;
            newHeader->regNumber = 0;
            newHeader->byteNextReg = 0;
            newHeader->removedRegNum = 0;

            // Tracks the original pointing address so it can be free'd later on
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

// (Static) Parses current CSV file line to the registry struct passed as parameter
// Return value: If everything succeeded (boolean)
static bool readLinhaRegistry(FILE *fpLinha, LinhaData *newData) {
    if (fpLinha) {
        char *lineRead = readline(fpLinha);
        if (lineRead && strlen(lineRead) > 0 && lineRead[0] != -1) {
            char *trackReference = lineRead;
            char *auxString = NULL;

            if (lineRead[0] == '*') {
                newData->isRemoved = REMOVED_REGISTRY;
                lineRead++;
            }
            else
                newData->isRemoved = VALID_REGISTRY;
        
            newData->linhaCode = atoi(strsep(&(lineRead), DELIM));
            newData->cardAcceptance = lineRead[0];
            lineRead += 2;

            newData->nameSize = 0;
            newData->linhaName = "";
            auxString = strsep(&lineRead, DELIM);
            if (strcmp(auxString, "NULO")) {
                newData->linhaName = strdup(auxString);
                newData->nameSize = strlen(newData->linhaName);
            }

            newData->colorSize = 0;
            newData->linhaColor = "";
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

// (Static) Writes each header field in sequence to the binary file
// Return value: If all data were written as expected (boolean)
static bool writeHeaderOnBinary(FILE *binFile, LinhaHeader *headerStruct) {
    if (binFile && headerStruct) {
        size_t bytesWritten = 0;

        // Fixed size fields (82 bytes)
        bytesWritten += fwrite(&headerStruct->fileStatus, sizeof(char), 1, binFile);
        bytesWritten += fwrite(&headerStruct->byteNextReg, sizeof(int64_t), 1, binFile);
        bytesWritten += fwrite(&headerStruct->regNumber, sizeof(int32_t), 1, binFile);
        bytesWritten += fwrite(&headerStruct->removedRegNum, sizeof(int32_t), 1, binFile);
        bytesWritten += fwrite(headerStruct->codeDescription, sizeof(char), CODE_DESC_SIZE, binFile);
        bytesWritten += fwrite(headerStruct->cardDescription, sizeof(char), CARD_DESC_SIZE, binFile);
        bytesWritten += fwrite(headerStruct->nameDescription, sizeof(char), NAME_DESC_SIZE, binFile);
        bytesWritten += fwrite(headerStruct->colorDescription, sizeof(char), COLOR_DESC_SIZE, binFile);

        if (bytesWritten != LINHA_HEADER_SIZE)
            return false;

        return true;
    }
    return false;
}

// (Static) Writes each registry field in sequence to the binary file
// Return value: If all data were written (boolean)
static bool writeRegistryOnBinary(FILE *binFile, LinhaData *registryStruct) {
    if (binFile && registryStruct) {
        size_t bytesWritten = 0;

        // Fixed size fields (13 bytes)
        bytesWritten += fwrite(&registryStruct->isRemoved, sizeof(char), 1, binFile);
        bytesWritten += fwrite(&registryStruct->regSize, sizeof(int32_t), 1, binFile);
        bytesWritten += fwrite(&registryStruct->linhaCode, sizeof(int32_t), 1, binFile);
        bytesWritten += fwrite(&registryStruct->cardAcceptance, sizeof(char), 1, binFile);

        // Dynamic size fields
        bytesWritten += fwrite(&registryStruct->nameSize, sizeof(int32_t), 1, binFile);
        if (registryStruct->nameSize > 0)
            bytesWritten += fwrite(registryStruct->linhaName, sizeof(char), registryStruct->nameSize, binFile);
        bytesWritten += fwrite(&registryStruct->colorSize, sizeof(int32_t), 1, binFile);
        if (registryStruct->colorSize > 0)
            bytesWritten += fwrite(registryStruct->linhaColor, sizeof(char), registryStruct->colorSize, binFile);

        // Checks if everything were written correctly (Register liquid size + isRemoved size + regSize size)
        if (bytesWritten != registryStruct->regSize + sizeof(char) + sizeof(int32_t))
            return false;

        return true;
    }
    return false;
}

// (Global) Frees everything related to the header struct (its pointer included)
// Return value: If everything succeeded as expected (boolean)
bool freeLinhaHeader(LinhaHeader **header) {
    if (header && *header) {
        if ((*header)->codeDescription)
            free((*header)->codeDescription);
        if ((*header)->cardDescription)
            free((*header)->cardDescription);
        if ((*header)->nameDescription)
            free((*header)->nameDescription);
        if ((*header)->colorDescription)
            free((*header)->colorDescription);
        free(*header);
        return true;
    }
    return false;
}

// (Static) Frees everything inside registry data struct (its pointer not included)
// Return value: If everything succeeded as expected (boolean)
static bool freeLinhaData(LinhaData *data) {
    if (data) {
        if (data->nameSize > 0)
            free(data->linhaName);
        if (data->colorSize > 0)
            free(data->linhaColor);
        return true;
    }
    return false;
}

// (Global) Reads a CSV file of category "Linhas" and write its respective binary file
// Return value: File pointer to binary file (FILE *)
FILE *writeLinhaBinary(char *csvFilename) {
    if (csvFilename && strlen(csvFilename) > 0) {
        FILE *csvFile = fopen(csvFilename, "r");
        if (!csvFile)
            return NULL;

        LinhaHeader *fileHeader = readLinhaHeader(csvFile);
        if (!fileHeader)
            return NULL;
        
        // Generates binary filename
        csvFilename[strlen(csvFilename)-4] = '\0';
        char binFilename[strlen(csvFilename)+1]; 
        strcpy(binFilename, csvFilename);
        strcat(binFilename, BIN_FILE_EXT);

        FILE *binFile = fopen(binFilename, "wb+");
        if (!binFile)
            return NULL;
        
        // Writes 82 bytes as a placeholder for the header
        // Signing file as "inconsistent" (0)
        fwrite(INCONSISTENT_FILE, sizeof(char), 1, binFile);
        fwrite("@", sizeof(char), LINHA_HEADER_SIZE-1, binFile);
        
        // Read each registry from CSV file and write it to binary
        LinhaData *newRegistry = (LinhaData *) malloc(sizeof *newRegistry);
        char EOFFlag;
        do {
            EOFFlag = fgetc(csvFile);
            if (EOFFlag != EOF) {
                fseek(csvFile, -1, SEEK_CUR);
                readLinhaRegistry(csvFile, newRegistry);
                writeRegistryOnBinary(binFile, newRegistry);
                if (newRegistry->isRemoved == REMOVED_REGISTRY)
                    fileHeader->removedRegNum++; 
                else
                    fileHeader->regNumber++;
                freeLinhaData(newRegistry);
            }
        } while (EOFFlag != EOF);
        fclose(csvFile);
        free(newRegistry);

        // Rewind into binary file and write the header struct
        // Signing file as "consistent" (1)
        fileHeader->byteNextReg = (int64_t) ftell(binFile);
        rewind(binFile);
        writeHeaderOnBinary(binFile, fileHeader);
        freeLinhaHeader(&fileHeader);
        return binFile;
    }
    return NULL;
}

// (Global) Reads and builds the header struct from the binary file
// Return value: A pointer for the built struct (LinhaHeader *)
LinhaHeader *loadBinaryLinhaHeader(FILE *binFile) {
    if (binFile) {
        size_t bytesRead = 0;

        // Check file consistency
        char fileStatus;
        bytesRead += fread(&fileStatus, 1, sizeof(char), binFile);
        if (fileStatus == INCONSISTENT_FILE[0] || fileStatus == EOF || bytesRead <= 0)
            return NULL;

        LinhaHeader *newHeader = (LinhaHeader *) malloc(sizeof *newHeader);
        newHeader->fileStatus = fileStatus;

        // Load numeric file statistics
        bytesRead += fread(&newHeader->byteNextReg, 1, sizeof(int64_t), binFile);
        bytesRead += fread(&newHeader->regNumber, 1, sizeof(int32_t), binFile);
        bytesRead += fread(&newHeader->removedRegNum, 1, sizeof(int32_t), binFile);

        // Load header strings
        newHeader->codeDescription = (char *) malloc(CODE_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->codeDescription, sizeof(char), CODE_DESC_SIZE, binFile);
        newHeader->cardDescription = (char *) malloc(CARD_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->cardDescription, sizeof(char), CARD_DESC_SIZE, binFile);
        newHeader->nameDescription = (char *) malloc(NAME_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->nameDescription, sizeof(char), NAME_DESC_SIZE, binFile);
        newHeader->colorDescription = (char *) malloc(COLOR_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->colorDescription, sizeof(char), COLOR_DESC_SIZE, binFile);
        
        if (bytesRead != LINHA_HEADER_SIZE) {
            freeLinhaHeader(&newHeader);
            return NULL;
        }
        return newHeader;

    }
    return NULL;
}

// (Static) Reads and builds the registry struct (to an already existent struct) from the binary file
// Return value: If the read succeeded as expected (boolean)
static bool loadBinaryRegistry(FILE *binFile, LinhaData *registryStruct) {
    if (binFile && registryStruct) {
        size_t bytesRead = 0;
        
        // Check registry removed status
        int32_t regSize;
        char registryStatus;
        fread(&registryStatus, 1, sizeof(char), binFile);
        fread(&regSize, 1, sizeof(int32_t), binFile);

        if (registryStatus == REMOVED_REGISTRY) {
            fseek(binFile, regSize, SEEK_CUR);
            return false;
        }

        // Load fixed size fields
        bytesRead += fread(&registryStruct->linhaCode, 1, sizeof(int32_t), binFile);
        bytesRead += fread(&registryStruct->cardAcceptance, 1, sizeof(char), binFile);

        // Load dynamic size fields
        bytesRead += fread(&registryStruct->nameSize, 1, sizeof(int32_t), binFile);
        if (registryStruct->nameSize > 0) {
            registryStruct->linhaName = (char *) malloc(registryStruct->nameSize*sizeof(char));
            bytesRead += fread(&registryStruct->linhaName, sizeof(char), registryStruct->nameSize, binFile);
        }
        bytesRead += fread(&registryStruct->colorSize, 1, sizeof(int32_t), binFile);
        if (registryStruct->nameSize > 0) {
            registryStruct->linhaColor = (char *) malloc(registryStruct->colorSize*sizeof(char));
            bytesRead += fread(&registryStruct->linhaColor, sizeof(char), registryStruct->colorSize, binFile);
        }

        // Check read status
        if (bytesRead != LINHA_FIXED_SIZE + registryStruct->colorSize + registryStruct->nameSize) {
            freeLinhaData(registryStruct);
            return false;
        }
        return true;
    }
    return false;
}

void printHeader(LinhaHeader *header) {
    if (header) {
        printf("fileStatus: %c\n", header->fileStatus);
        printf("byteNextReg: %" PRId64 "\n", header->byteNextReg);
        printf("regNumber: %d\n", header->regNumber);
        printf("removedRegNum: %d\n", header->removedRegNum);
        printf("codeDescription: %.*s\n", CODE_DESC_SIZE, header->codeDescription);
        printf("cardDescription: %.*s\n", CARD_DESC_SIZE, header->cardDescription);
        printf("nameDescription: %.*s\n", NAME_DESC_SIZE, header->nameDescription);
        printf("colorDescription: %.*s\n", COLOR_DESC_SIZE, header->colorDescription);
    }
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
