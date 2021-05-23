#include "LinhaFileManager.h"
#include "Utility.h"

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

            newHeader->codeDescription = strdup(strsep(&lineRead, COMMA_DELIM));
            newHeader->cardDescription = strdup(strsep(&lineRead, COMMA_DELIM));
            newHeader->nameDescription = strdup(strsep(&lineRead, COMMA_DELIM));
            newHeader->colorDescription = strdup(strsep(&lineRead, LINE_BREAK));
            free(trackReference);
            return newHeader;
        }
        return false;
    }
    return false;
}

// (Global) Parses current CSV file or stdin line to the registry struct passed as parameter
// In CSV file, strsep delimiter is a comma (COMMA_DELIM)
// Return value: If everything succeeded (boolean)
bool readLinhaRegistry(FILE *fpLinha, LinhaData *newData) {
    if (fpLinha) {
        char *lineRead = readline(fpLinha);
        if (fpLinha == stdin)
            tranformToCsvFormat(lineRead);

        if (lineRead && strlen(lineRead) > 0 && lineRead[0] != -1) {
            char *trackReference = lineRead;
            char *auxString = NULL;

            if (lineRead[0] == '*') {
                newData->isRemoved = REMOVED_REGISTRY;
                lineRead++;
            }
            else
                newData->isRemoved = VALID_REGISTRY;
        
            newData->linhaCode = atoi(strsep(&(lineRead), COMMA_DELIM));

            newData->cardAcceptance = CARD_NONE;
            auxString = strsep(&(lineRead), COMMA_DELIM);
            if (strcmp(auxString, "NULO")) {
                newData->cardAcceptance = auxString[0];
            }

            newData->nameSize = 0;
            newData->linhaName = "";
            auxString = strsep(&lineRead, COMMA_DELIM);
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

// (Global) Writes each header field in sequence to the binary file
// Return value: If all data were written as expected (boolean)
bool writeHeaderOnBinary(FILE *binFile, LinhaHeader *headerStruct) {
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

// (Global) Writes each registry field in sequence to the binary file
// Return value: If all data were written (boolean)
bool writeRegistryOnBinary(FILE *binFile, LinhaData *registryStruct) {
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

// (Global) Frees everything inside registry data struct (its pointer not included)
// Return value: If everything succeeded as expected (boolean)
bool freeLinhaData(LinhaData *data) {
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
bool writeLinhaBinary(char *csvFilename, char *binFilename) {
    if (csvFilename && strlen(csvFilename) > 0) {
        FILE *csvFile = fopen(csvFilename, "r");
        if (!csvFile)
            return false;

        LinhaHeader *fileHeader = readLinhaHeader(csvFile);
        if (!fileHeader)
            return false;
        
        FILE *binFile = fopen(binFilename, "wb+");
        if (!binFile)
            return false;
        
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
        fclose(binFile);
        return true;
    }
    return false;
}

// (Global) Reads and builds the header struct from the binary file
// Return value: A pointer for the built struct (LinhaHeader *)
LinhaHeader *loadLinhaBinaryHeader(FILE *binFile) {
    if (binFile) {
        size_t bytesRead = 0;

        // Check file consistency
        char fileStatus;
        bytesRead += fread(&fileStatus, 1, sizeof(char), binFile);
        if (fileStatus == INCONSISTENT_FILE[0] || bytesRead <= 0)
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

// (Global) Reads and builds the registry struct (to an already existent malloc'd struct) from the binary file
// Return value: If the read succeeded as expected (boolean)
bool loadLinhaBinaryRegistry(FILE *binFile, LinhaData *registryStruct) {
    if (binFile && registryStruct) {
        size_t bytesRead = 0;
        
        // Check registry removed status
        int32_t regSize;
        char registryStatus;
        if (!fread(&registryStatus, 1, sizeof(char), binFile)) {
            return false;
        }
        fread(&regSize, 1, sizeof(int32_t), binFile);

        registryStruct->isRemoved = registryStatus;
        registryStruct->regSize = regSize;
        if (registryStatus == REMOVED_REGISTRY) {
            fseek(binFile, regSize, SEEK_CUR);
            return true;
        }

        // Load fixed size fields
        bytesRead += fread(&registryStruct->linhaCode, 1, sizeof(int32_t), binFile);
        bytesRead += fread(&registryStruct->cardAcceptance, 1, sizeof(char), binFile);

        // Load dynamic size fields
        bytesRead += fread(&registryStruct->nameSize, 1, sizeof(int32_t), binFile);
        if (registryStruct->nameSize > 0) {
            registryStruct->linhaName = (char *) malloc(registryStruct->nameSize*sizeof(char));
            bytesRead += fread(registryStruct->linhaName, sizeof(char), registryStruct->nameSize, binFile);
        }
        bytesRead += fread(&registryStruct->colorSize, 1, sizeof(int32_t), binFile);
        if (registryStruct->colorSize > 0) {
            registryStruct->linhaColor = (char *) malloc(registryStruct->colorSize*sizeof(char));
            bytesRead += fread(registryStruct->linhaColor, sizeof(char), registryStruct->colorSize, binFile);
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

// (Global) Shows formated recovered registry information in screen.
// Exception field is a way to select a field to not be printed (used in functionalities 5 and 6).
// Return value: Nothing (void)
void printLinhaRegistry(LinhaHeader *header, LinhaData *registry, const char *exceptionField) {
    if (header && registry) {
        if (strcmp(exceptionField, "codLinha"))
            printf("%.*s: %d\n", CODE_DESC_SIZE, header->codeDescription, registry->linhaCode);

        if (strcmp(exceptionField, "nomeLinha")) {
            printf("%.*s: ", NAME_DESC_SIZE, header->nameDescription);
            if (registry->nameSize == 0)
                printf("%s\n", NULL_FIELD);
            else
                printf("%.*s\n", registry->nameSize, registry->linhaName);
        }

        if (strcmp(exceptionField, "corLinha")) {
            printf("%.*s: ", COLOR_DESC_SIZE, header->colorDescription);
            if (registry->colorSize == 0)
                printf("%s\n", NULL_FIELD);
            else
                printf("%.*s\n", registry->colorSize, registry->linhaColor);
        }
        
        if (strcmp(exceptionField, "aceitaCartao")) {
            printf("%.*s: ", CARD_DESC_SIZE, header->cardDescription);
            switch (registry->cardAcceptance) {
                case 'S': {
                    printf("%s\n", CARD_S_MESSAGE);
                    break;
                }
                case 'N': {
                    printf("%s\n", CARD_N_MESSAGE);
                    break;
                }
                case 'F': {
                    printf("%s\n", CARD_F_MESSAGE);
                    break;
                }
            }
        }
        printf("\n");
    }
}
