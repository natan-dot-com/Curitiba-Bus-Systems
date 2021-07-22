#include "Functionalities.h"

// Functionality 1: Writes a binary file from existent CSV file
// Related to: 'Veiculo' type CSV files
// Return value: Error code (0 if none)
int8_t createVeiculoBinaryFile(char *csvFilename, char *binFilename) {
    if (csvFilename && strlen(csvFilename) > 0) {
        FILE *csvFile = fopen(csvFilename, "r");
        if (!csvFile) {
            return ERR_FILE_PROC;
        }

        VeiculoHeader *fileHeader = readVeiculoHeader(csvFile);
        if (!fileHeader) {
            fclose(csvFile);
            return ERR_FILE_PROC;
        }

        FILE *binFile = fopen(binFilename, "wb+");
        if (!binFile) {
            fclose(csvFile);
            freeVeiculoHeader(&fileHeader);
            return ERR_FILE_PROC;
        }
        
        // Writes 175 bytes as a placeholder for the header
        // Signing file as "inconsistent" ('0')
        fwrite(INCONSISTENT_FILE, sizeof(char), 1, binFile);
        fwrite("@", sizeof(char), VEICULO_HEADER_SIZE-1, binFile);
        
        // Read each registry from CSV file and write it to binary
        VeiculoData *newRegistry = (VeiculoData *) malloc(sizeof *newRegistry);
        char EOFFlag;
        do {
            EOFFlag = fgetc(csvFile);
            if (EOFFlag != EOF) {
                ungetc(EOFFlag, csvFile);
                readVeiculoRegistry(csvFile, newRegistry);
                writeVeiculoRegistryOnBinary(binFile, newRegistry);

                if (newRegistry->isRemoved == REMOVED_REGISTRY) {
                    fileHeader->removedRegNum++; 
                }
                else {
                    fileHeader->regNumber++;
                }
                freeVeiculoData(newRegistry);
            }
        } while (EOFFlag != EOF);
        fclose(csvFile);
        free(newRegistry);

        // Rewind into binary file and write the header struct
        // Signing file as "consistent" ('1')
        fileHeader->byteNextReg = (int64_t) ftell(binFile);
        rewind(binFile);
        writeVeiculoHeaderOnBinary(binFile, fileHeader);
        freeVeiculoHeader(&fileHeader);
        fclose(binFile);

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 2: Writes a binary file from existent CSV file
// Related to: 'Linha' type CSV files
// Return value: Error code (0 if none)
int8_t createLinhaBinaryFile(char *csvFilename, char *binFilename) {
    if (csvFilename && strlen(csvFilename) > 0) {
        FILE *csvFile = fopen(csvFilename, "r");
        if (!csvFile) {
            return ERR_FILE_PROC;
        }

        LinhaHeader *fileHeader = readLinhaHeader(csvFile);
        if (!fileHeader) {
            fclose(csvFile);
            return ERR_FILE_PROC;
        }
        
        FILE *binFile = fopen(binFilename, "wb+");
        if (!binFile) {
            fclose(csvFile);
            freeLinhaHeader(&fileHeader);
            return ERR_FILE_PROC;
        }
        
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
                ungetc(EOFFlag, csvFile);
                readLinhaRegistry(csvFile, newRegistry);
                writeLinhaRegistryOnBinary(binFile, newRegistry);

                if (newRegistry->isRemoved == REMOVED_REGISTRY) {
                    fileHeader->removedRegNum++; 
                }
                else {
                    fileHeader->regNumber++;
                }
                freeLinhaData(newRegistry);
            }
        } while (EOFFlag != EOF);
        fclose(csvFile);
        free(newRegistry);

        // Rewind into binary file and write the header struct
        // Signing file as "consistent" (1)
        fileHeader->byteNextReg = (int64_t) ftell(binFile);
        rewind(binFile);
        writeLinhaHeaderOnBinary(binFile, fileHeader);
        freeLinhaHeader(&fileHeader);
        fclose(binFile);

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 3: Shows on stdout every registry from binary file on screen
// Related to: 'Veiculo' type binary files
// Return value: Error code (0 if none)
int8_t printVeiculoContent(char *binaryFilename) {
    if (binaryFilename) {
        FILE *binFile = fopen(binaryFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }
        
        // Load file header
        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
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

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 4: Shows on stdout every registry from binary file on screen
// Related to: 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t printLinhaContent(char *binaryFilename) {
    if (binaryFilename) {
        FILE *binFile = fopen(binaryFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Load file header
        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
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

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 5: Searches for a specified field inside binary file
// Related to: 'Veiculo' type binary files
// Return value: Error code (0 if none)
int8_t searchVeiculoContent(char *binaryFilename, char *fieldName, char *inputLine) {
    if (binaryFilename && fieldName && inputLine) {
        char *fieldValue = NULL;
        FILE *binFile = fopen(binaryFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Load file header
        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
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
        if (foundRegistries == 0) {
            return ERR_REG_NOT_FOUND;
        }

        free(newRegistry);
        freeVeiculoHeader(&fileHeader);
        
        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}


// Functionality 6: Searches for a specified field inside binary file
// Related to: 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t searchLinhaContent(char *binaryFilename, char *fieldName, char *inputLine) {
    if (binaryFilename && fieldName && inputLine) {
        char *fieldValue = NULL;

        FILE *binFile = fopen(binaryFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Load file header
        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
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
        if (foundRegistries == 0) {
            return ERR_REG_NOT_FOUND;
        }

        free(newRegistry);
        freeLinhaHeader(&fileHeader);

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 7: Insert 'n' new registries from stdin on binary file
// Related to: 'Veiculo' type binary files
// Return value: Error code (0 if none)
int8_t insertVeiculoContent(char *binaryFilename, int8_t insertNumber) {
    if (binaryFilename) {
        FILE *binFile = fopen(binaryFilename, "rb+");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Loads file header
        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
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
            if (newRegistry->isRemoved == REMOVED_REGISTRY) {
                fileHeader->removedRegNum++;
            }
            else {
                fileHeader->regNumber++;
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

        freeVeiculoHeader(&fileHeader);

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 8: Insert 'n' new registries from stdin on binary file
// Related to: 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t insertLinhaContent(char *binaryFilename, int8_t insertNumber) {
    if (binaryFilename) {
        FILE *binFile = fopen(binaryFilename, "rb+");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Loads file header
        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
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
            if (newRegistry->isRemoved == REMOVED_REGISTRY) {
                fileHeader->removedRegNum++;
            }
            else {
                fileHeader->regNumber++;
            }

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

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 9: Creates a B-Tree index file based on binary registry file
// Related to: 'Veiculo' type binary files
// Return value: Error code (0 if none)
int8_t createVeciculoBTree(char *binaryFilename, char *indexFilename) {
    if(binaryFilename && indexFilename) {
        FILE *binFile = fopen(binaryFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Load Veiculo file header
        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
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
        
        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 10: Creates a B-Tree index file based on binary registry file
// Related to: 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t createLinhaBTree(char *binaryFilename, char *indexFilename) {
    if(binaryFilename && indexFilename) {
        FILE *binFile = fopen(binaryFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Load Linha file header
        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
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

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 11: Searches for a registry based on its prefix (optimized search)
// Related to: 'Veiculo' type binary files
// Return value: Error code (0 if none)
int8_t searchVeiculoPrefix(char *binaryFilename, char *indexFilename, char *fieldName, char *fieldValue) {
    if (binaryFilename && indexFilename && fieldName && fieldValue) {
        FILE *binFile = fopen(binaryFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Load file header
        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
        }

        // Load B-Tree file header
        BTreeHeader *indexHeader = openBTree(indexFilename);
        if (!indexHeader) {
            freeVeiculoHeader(&fileHeader);
            freeBTree(indexHeader);
            fclose(binFile);
            return ERR_FILE_PROC;
        }

        int64_t regOffset = searchBTree(indexHeader, convertePrefixo(fieldValue));
        if (regOffset == EMPTY) {
            freeVeiculoHeader(&fileHeader);
            freeBTree(indexHeader);
            fclose(binFile);
            return ERR_REG_NOT_FOUND;
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

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 12: Searches for a registry based on its prefix (optimized search)
// Related to: 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t searchLinhaPrefix(char *binaryFilename, char *indexFilename, char *fieldName, int32_t fieldValue) {
    if (binaryFilename && indexFilename && fieldName) {
        FILE *binFile = fopen(binaryFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Load Linha file header
        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
        }

        // Load B-Tree file header
        BTreeHeader *indexHeader = openBTree(indexFilename);
        if (!indexHeader) {
            fclose(binFile);
            freeLinhaHeader(&fileHeader);
            return ERR_FILE_PROC;
        }

        int64_t regOffset = searchBTree(indexHeader, fieldValue);
        if (regOffset == EMPTY) {
            fclose(binFile);
            freeLinhaHeader(&fileHeader);
            freeBTree(indexHeader);
            return ERR_REG_NOT_FOUND;
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

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 13: Insert 'n' new registries from stdin on binary and index files
// Related to: 'Veiculo' type binary files
// Return value: Error code (0 if none)
int8_t insertVeiculoBTree(char *binaryFilename, char *indexFilename, int8_t insertNumber) {
    if (binaryFilename && indexFilename) {
        FILE *binFile = fopen(binaryFilename, "rb+");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Loads file header
        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
        }

        // Loads B-Tree header
        BTreeHeader *indexHeader = openBTree(indexFilename);
        if (!indexHeader) {
            fclose(binFile);
            freeVeiculoHeader(&fileHeader);
            return ERR_FILE_PROC;
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
        
        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 14: Insert 'n' new registries from stdin on binary and index files
// Related to: 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t insertLinhaBTree(char *binaryFilename, char *indexFilename, int8_t insertNumber) {
    if (binaryFilename && indexFilename) {
        FILE *binFile = fopen(binaryFilename, "rb+");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        // Loads file header
        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
        }

        // Loads B-Tree file header
        BTreeHeader *indexHeader = openBTree(indexFilename);
        if (!indexHeader) {
            freeLinhaHeader(&fileHeader);
            fclose(binFile);
            return ERR_FILE_PROC;
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

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 15: Merge binary files sequentially
// Related to: Both 'Veiculo' and 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t printVeiculoMerged(char *veiculoBinFilename, char *linhaBinFilename) {
    if (veiculoBinFilename && linhaBinFilename) {
        // Check files
        FILE *veiculoBinFile = fopen(veiculoBinFilename, "rb");
        if (!veiculoBinFile) {
            return ERR_FILE_PROC;
        }
        
        FILE *linhaBinFile = fopen(linhaBinFilename, "rb");
        if (!linhaBinFile) {
            fclose(veiculoBinFile);
            return ERR_FILE_PROC;
        }
        
        VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
        if (!veiculoHeader) {
            fclose(veiculoBinFile);
            fclose(linhaBinFile);
            return ERR_FILE_PROC;
        }

        LinhaHeader *linhaHeader = loadLinhaBinaryHeader(linhaBinFile);
        if (!linhaHeader) {
            fclose(veiculoBinFile);
            fclose(linhaBinFile);
            freeVeiculoHeader(&veiculoHeader);
            return ERR_FILE_PROC;
        }
        if (linhaHeader->regNumber == 0) {
            fclose(veiculoBinFile);
            fclose(linhaBinFile);
            freeVeiculoHeader(&veiculoHeader);
            freeLinhaHeader(&linhaHeader);
            return ERR_REG_NOT_FOUND;
        }

        // Allocate memory for both registries
        VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
        LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);

        int mergedRegistries = 0; 

        // Load each Veiculo registry 
        while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {
            
            if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
                
                if(newVeiculoRegistry->linhaCode != EMPTY) {
                    // Skip Header
                    fseek(linhaBinFile, LINHA_HEADER_SIZE, SEEK_SET);

                    // Search for the right code in the Linha file
                    while(loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry)) {
                        if(newLinhaRegistry->isRemoved == VALID_REGISTRY) {
                            if(newVeiculoRegistry->linhaCode == newLinhaRegistry->linhaCode) {
                                printRegistries(veiculoHeader, newVeiculoRegistry, linhaHeader, newLinhaRegistry);
                                freeLinhaData(newLinhaRegistry);
                                mergedRegistries++;
                                break;
                            } else {
                                freeLinhaData(newLinhaRegistry);
                            }
                        }
                    }

                }

                freeVeiculoData(newVeiculoRegistry);
            }
        }

        free(newLinhaRegistry);
        free(newVeiculoRegistry);
        freeVeiculoHeader(&veiculoHeader);
        freeLinhaHeader(&linhaHeader);
        fclose(linhaBinFile);
        fclose(veiculoBinFile);

        if(mergedRegistries == 0) {
            return ERR_REG_NOT_FOUND;
        }

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 16: Optimized merge from binary files using B-Tree
// Related to: Both 'Veiculo' and 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t printVeiculoMergedWithBTree(char *veiculoBinFilename, char *linhaBinFilename, char *linhaBTreeFilename) {
    if (veiculoBinFilename && linhaBinFilename && linhaBTreeFilename) {
        // Check files
        FILE *veiculoBinFile = fopen(veiculoBinFilename, "rb");
        if (!veiculoBinFile) {
            return ERR_FILE_PROC;
        }

        FILE *linhaBinFile = fopen(linhaBinFilename, "rb");
        if (!linhaBinFile) {
            fclose(veiculoBinFile);
            return ERR_FILE_PROC;
        }
        
        BTreeHeader *bTreeHeader = openBTree(linhaBTreeFilename);
        if(!bTreeHeader) {
            fclose(veiculoBinFile);
            fclose(linhaBinFile);
            return ERR_FILE_PROC;
        }
        
        VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
        if (!veiculoHeader) {
            fclose(veiculoBinFile);
            fclose(linhaBinFile);
            freeBTree(bTreeHeader);
            return ERR_FILE_PROC;
        }

        LinhaHeader *linhaHeader = loadLinhaBinaryHeader(linhaBinFile);
        if (!linhaHeader) {
            fclose(veiculoBinFile);
            fclose(linhaBinFile);
            freeVeiculoHeader(&veiculoHeader);
            freeBTree(bTreeHeader);
            return ERR_FILE_PROC;
        }
        
        // Allocate memory for both registries
        VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
        LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);
        int mergedRegistries = 0;

        // Load each Veiculo registry
        while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {
            if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
                
                if(newVeiculoRegistry->linhaCode != EMPTY) {

                    // Search for the right code using a BTree
                    int64_t offset = searchBTree(bTreeHeader, newVeiculoRegistry->linhaCode);
                    if(offset != EMPTY) {
                        fseek(linhaBinFile, offset, SEEK_SET);
                        loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry);
                        printRegistries(veiculoHeader, newVeiculoRegistry, linhaHeader, newLinhaRegistry);
                        mergedRegistries++;
                        freeLinhaData(newLinhaRegistry);
                    }
                }
                freeVeiculoData(newVeiculoRegistry);
            }
        }

        free(newLinhaRegistry);
        free(newVeiculoRegistry);
        freeVeiculoHeader(&veiculoHeader);
        freeLinhaHeader(&linhaHeader);
        freeBTree(bTreeHeader);
        fclose(linhaBinFile);
        fclose(veiculoBinFile);

        if(mergedRegistries == 0) {
            return ERR_REG_NOT_FOUND;
        }

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 17: Sort 'Veiculo' binary file in O(nlog(n)) time complexity
// Related to: 'Veiculo' type binary files
// Return value: Error code (0 if none)
int8_t sortVeiculoFile(char *binFilename, char *sortedFilename) {
    if (binFilename && sortedFilename) {
        FILE *binFile = fopen(binFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
        }
        fileHeader->fileStatus = CONSISTENT_FILE;
        fileHeader->removedRegNum = 0;

        FILE *sortedFile = fopen(sortedFilename, "wb+");
        if (!sortedFile) {
            fclose(binFile);
            freeVeiculoHeader(&fileHeader);
            return ERR_FILE_PROC;
        }

        fwrite(INCONSISTENT_FILE, sizeof(char), 1, sortedFile);
        fwrite("@", sizeof(char), VEICULO_HEADER_SIZE-1, sortedFile);
        
        VeiculoData **veiculoList = (VeiculoData **) calloc(fileHeader->regNumber, sizeof(VeiculoData *));
        for (int i = 0; i < fileHeader->regNumber; i++) {
            if(!veiculoList[i]) {
                veiculoList[i] = (VeiculoData *) calloc(1, sizeof(VeiculoData));
            }
            if(loadVeiculoBinaryRegistry(binFile, veiculoList[i])) {
                if (veiculoList[i]->isRemoved == REMOVED_REGISTRY) {
                    freeVeiculoData(veiculoList[i]);
                    i--;
                }
            } else {
                i--;
            }
            
        }
        fclose(binFile);

        qsort(veiculoList, fileHeader->regNumber, sizeof(VeiculoData *), &veiculoCompare);
        for (int i = 0; i < fileHeader->regNumber; i++) {
            writeVeiculoRegistryOnBinary(sortedFile, veiculoList[i]);
            freeVeiculoData(veiculoList[i]);
            free(veiculoList[i]);
        }

        fileHeader->byteNextReg = ftell(sortedFile);
        rewind(sortedFile);
        writeVeiculoHeaderOnBinary(sortedFile, fileHeader);

        fclose(sortedFile);
        freeVeiculoHeader(&fileHeader);
        free(veiculoList);

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 18: Sort 'Linha' binary file in O(nlog(n)) time complexity
// Related to: 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t sortLinhaFile(char *binFilename, char *sortedFilename) {
    if (binFilename && sortedFilename) {
        FILE *binFile = fopen(binFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
        }
        fileHeader->fileStatus = CONSISTENT_FILE;
        fileHeader->removedRegNum = 0;

        FILE *sortedFile = fopen(sortedFilename, "wb+");
        if (!sortedFile) {
            fclose(binFile);
            freeLinhaHeader(&fileHeader);
            return ERR_FILE_PROC;
        }

        fwrite(INCONSISTENT_FILE, sizeof(char), 1, sortedFile);
        fwrite("@", sizeof(char), LINHA_HEADER_SIZE-1, sortedFile);
        
        LinhaData **linhaList = (LinhaData **) calloc(fileHeader->regNumber, sizeof(LinhaData *));
        for (int i = 0; i < fileHeader->regNumber; i++) {
            if (!linhaList[i]) {
                linhaList[i] = (LinhaData *) calloc(1, sizeof(LinhaData));
            }
            if(loadLinhaBinaryRegistry(binFile, linhaList[i])) {
                if (linhaList[i]->isRemoved == REMOVED_REGISTRY) {
                    freeLinhaData(linhaList[i]);
                    i--;
                }
            } else {
                i--;
            }
        }
        fclose(binFile);

        qsort(linhaList, fileHeader->regNumber, sizeof(LinhaData *), &linhaCompare);
        for (int i = 0; i < fileHeader->regNumber; i++) {
            writeLinhaRegistryOnBinary(sortedFile, linhaList[i]);
            freeLinhaData(linhaList[i]);
            free(linhaList[i]);
        }

        fileHeader->byteNextReg = ftell(sortedFile);
        rewind(sortedFile);
        writeLinhaHeaderOnBinary(sortedFile, fileHeader);

        fclose(sortedFile);
        freeLinhaHeader(&fileHeader);
        free(linhaList);

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}

// Functionality 19: Merge binary files using previously created sorted files
// Related to: Both 'Veiculo' and 'Linha' type binary files
// Return value: Error code (0 if none)
int8_t printVeiculoSortMerge(char *veiculoBinFilename, char *linhaBinFilename) {
    if (veiculoBinFilename && linhaBinFilename) {

        // Create the sorted files of Veiculo and Linha
        char *sortedVeiculoFilename = "VeiculoSorted";
        char *sortedLinhaFilename = "LinhaSorted";

        int8_t errorHandler;
        if (errorHandler = sortVeiculoFile(veiculoBinFilename, sortedVeiculoFilename)) {
            return errorHandler;
        }

        if (errorHandler = sortLinhaFile(linhaBinFilename, sortedLinhaFilename)) {
            return errorHandler;
        }
        
        // Check the files
        FILE *veiculoBinFile = fopen(sortedVeiculoFilename, "rb");
        if (!veiculoBinFile) {
            return ERR_FILE_PROC;
        }

        FILE *linhaBinFile = fopen(sortedLinhaFilename, "rb");
        if (!linhaBinFile) {
            fclose(veiculoBinFile);
            return ERR_FILE_PROC;
        }
        
        VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
        if (!veiculoHeader) {
            fclose(veiculoBinFile);
            fclose(linhaBinFile);
            return ERR_FILE_PROC;
        }

        LinhaHeader *linhaHeader = loadLinhaBinaryHeader(linhaBinFile);
        if (!linhaHeader) {
            fclose(veiculoBinFile);
            fclose(linhaBinFile);
            freeVeiculoHeader(&veiculoHeader);
            return ERR_FILE_PROC;
        }

        // Allocate memory for both registries
        VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
        LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);

        // Get an initial registry
        do {
            loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry);
        } while(!newLinhaRegistry || newLinhaRegistry->isRemoved == REMOVED_REGISTRY);
        
        int mergedRegistries = 0;
        while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {
            if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
                
                if(newVeiculoRegistry->linhaCode != EMPTY) {
                    if(newLinhaRegistry->linhaCode == newVeiculoRegistry->linhaCode) {
                        printRegistries(veiculoHeader, newVeiculoRegistry, linhaHeader, newLinhaRegistry);
                        mergedRegistries++;
                    } else {
                        // Check if the current linhaCode is greater than the searched one
                        if(newLinhaRegistry->linhaCode > newVeiculoRegistry->linhaCode) {
                            fseek(linhaBinFile, LINHA_HEADER_SIZE, SEEK_SET);
                        }

                        freeLinhaData(newLinhaRegistry);
                        bool isFileEnd = true; // Flag to check if the whole file was searched

                        // Search for linhaCode 
                        while(loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry)) {
                            if(newVeiculoRegistry->linhaCode == newLinhaRegistry->linhaCode) {
                                printRegistries(veiculoHeader, newVeiculoRegistry, linhaHeader, newLinhaRegistry);
                                mergedRegistries++;
                                isFileEnd = false;
                                break;
                            } else if(newLinhaRegistry->linhaCode > newVeiculoRegistry->linhaCode) {
                                isFileEnd = false;
                                break;
                            }
                            freeLinhaData(newLinhaRegistry);
                        }
                        if(isFileEnd) {
                            // If the whole file has been searched return to the first registry
                            fseek(linhaBinFile, LINHA_HEADER_SIZE, SEEK_SET);
                            do {
                                loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry);
                            } while(!newLinhaRegistry || newLinhaRegistry->isRemoved == REMOVED_REGISTRY);
                        }
                    }
                }
                freeVeiculoData(newVeiculoRegistry);
            }
        }
        remove(sortedVeiculoFilename);
        remove(sortedLinhaFilename);
        freeLinhaData(newLinhaRegistry);

        free(newLinhaRegistry);
        free(newVeiculoRegistry);
        freeVeiculoHeader(&veiculoHeader);
        freeLinhaHeader(&linhaHeader);
        fclose(linhaBinFile);
        fclose(veiculoBinFile);

        if(mergedRegistries == 0) {
            return ERR_REG_NOT_FOUND;
        }

        return EXIT_SUCCESS;
    }
    return ERR_INVALID_STRING;
}