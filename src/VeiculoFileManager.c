#include "VeiculoFileManager.h"

// (Extern) Parses first CSV file line to the header struct
// Return value: A pointer to the built struct (VeiculoHeader *)
VeiculoHeader *readVeiculoHeader(FILE *fpVeiculo) {
    if (fpVeiculo) {
        char *lineRead = readline(fpVeiculo);
        if (lineRead) {
            VeiculoHeader *newHeader = (VeiculoHeader *) malloc(sizeof *newHeader);

            newHeader->fileStatus = CONSISTENT_FILE;
            newHeader->regNumber = 0;
            newHeader->byteNextReg = 0;
            newHeader->removedRegNum = 0;

            // Tracks the original pointing address so it can be free'd later on
            char *trackReference = lineRead;

            newHeader->prefixDescription = strdup(strsep(&lineRead, COMMA_DELIM));
            newHeader->dateDescription = strdup(strsep(&lineRead, COMMA_DELIM));
            newHeader->seatsDescription = strdup(strsep(&lineRead, COMMA_DELIM));
            newHeader->lineDescription = strdup(strsep(&lineRead, COMMA_DELIM));
            newHeader->modelDescription = strdup(strsep(&lineRead, COMMA_DELIM));
            newHeader->categoryDescription = strdup(strsep(&lineRead, LINE_BREAK));
            free(trackReference);
            return newHeader;
        }
        return NULL;
    }
    return NULL;
}

// (Extern) Parses current CSV file line to the registry struct passed as parameter
// Return value: If everything succeeded (boolean)
bool readVeiculoRegistry(FILE *fpVeiculo, VeiculoData *newData) {
    if (fpVeiculo) {
        char *stringVeiculo = readline(fpVeiculo);
        if (fpVeiculo == stdin)
            tranformToCsvFormat(stringVeiculo);

        if (stringVeiculo && strlen(stringVeiculo) > 0 && stringVeiculo[0] != -1) {
            char *trackReference = stringVeiculo;
            char *auxString = NULL;

            if (stringVeiculo[0] == '*') {
                newData->isRemoved = REMOVED_REGISTRY;
                stringVeiculo++;
            }
            else
                newData->isRemoved = VALID_REGISTRY;

            auxString = strsep(&stringVeiculo, COMMA_DELIM);
            newData->prefix = strdup(auxString);

            auxString = strsep(&stringVeiculo, COMMA_DELIM);
            if (strcmp(auxString, "NULO")) {
                newData->date = strdup(auxString);
            } else {
                newData->date = (char *) calloc(sizeof(char), DATE_SIZE);
                strncpy(newData->date, "@@@@@@@@@@", DATE_SIZE);
                newData->date[0] = '\0';
            }

            auxString = strsep(&stringVeiculo, COMMA_DELIM);
            if (strcmp(auxString, "NULO")) {
                newData->seatsNumber = atoi(auxString);
            } else {
                newData->seatsNumber = -1;
            }

            auxString = strsep(&stringVeiculo, COMMA_DELIM);
            if (strcmp(auxString, "NULO")) {
                newData->linhaCode = atoi(auxString);
            } else {
                newData->linhaCode = -1;
            }
            
            auxString = strsep(&stringVeiculo, COMMA_DELIM);
            if (strcmp(auxString, "NULO")) {
                newData->model = strdup(auxString);
                newData->modelSize = strlen(newData->model);
            } else {
                newData->modelSize = 0;
            }
            auxString = strsep(&stringVeiculo, LINE_BREAK);
            if (strcmp(auxString, "NULO")) {
                newData->category = strdup(auxString);
                newData->categorySize = strlen(newData->category);
            } else {
                newData->categorySize = 0;
            }

            free(trackReference);

            newData->regSize = newData->modelSize + newData->categorySize + VEICULO_FIXED_SIZE;
            return true;
        }
        free(stringVeiculo);
        return false;
    }
    return false;
}

// (Extern) Writes each header field in sequence to the binary file
// Return value: If all data were written as expected (boolean)
bool writeVeiculoHeaderOnBinary(FILE *binFile, VeiculoHeader *headerStruct) {
    if (binFile && headerStruct) {
        size_t bytesWritten = 0;

        // Fixed size fields (175 bytes)
        bytesWritten += fwrite(&headerStruct->fileStatus, sizeof(char), 1, binFile);
        bytesWritten += fwrite(&headerStruct->byteNextReg, sizeof(int64_t), 1, binFile);
        bytesWritten += fwrite(&headerStruct->regNumber, sizeof(int32_t), 1, binFile);
        bytesWritten += fwrite(&headerStruct->removedRegNum, sizeof(int32_t), 1, binFile);
        bytesWritten += fwrite(headerStruct->prefixDescription, sizeof(char), PREFIX_DESC_SIZE, binFile);
        bytesWritten += fwrite(headerStruct->dateDescription, sizeof(char), DATE_DESC_SIZE, binFile);
        bytesWritten += fwrite(headerStruct->seatsDescription, sizeof(char), SEATS_DESC_SIZE, binFile);
        bytesWritten += fwrite(headerStruct->lineDescription, sizeof(char), LINE_DESC_SIZE, binFile);
        bytesWritten += fwrite(headerStruct->modelDescription, sizeof(char), MODEL_DESC_SIZE, binFile);
        bytesWritten += fwrite(headerStruct->categoryDescription, sizeof(char), CATEGORY_DESC_SIZE, binFile);

        if (bytesWritten != VEICULO_HEADER_SIZE)
            return false;

        return true;
    }
    return false;
}

// (Extern) Writes each registry field in sequence to the binary file
// Return value: If all data were written (boolean)
bool writeVeiculoRegistryOnBinary(FILE *binFile, VeiculoData *registryStruct) {
    if (binFile && registryStruct) {
        size_t bytesWritten = 0;

        bytesWritten += fwrite(&registryStruct->isRemoved, sizeof(char), 1, binFile);
        bytesWritten += fwrite(&registryStruct->regSize, sizeof(int32_t), 1, binFile);

        // Fixed size fields (31 bytes)
        bytesWritten += fwrite(registryStruct->prefix, sizeof(char), PREFIX_SIZE, binFile);
        bytesWritten += fwrite(registryStruct->date, sizeof(char), DATE_SIZE, binFile);
        bytesWritten += fwrite(&registryStruct->seatsNumber, sizeof(int32_t), 1, binFile);
        bytesWritten += fwrite(&registryStruct->linhaCode, sizeof(int32_t), 1, binFile);
        
        // Dynamic size fields
        bytesWritten += fwrite(&registryStruct->modelSize, sizeof(int32_t), 1, binFile);
        if (registryStruct->modelSize > 0)
            bytesWritten += fwrite(registryStruct->model, sizeof(char), registryStruct->modelSize, binFile);
        bytesWritten += fwrite(&registryStruct->categorySize, sizeof(int32_t), 1, binFile);
        if (registryStruct->categorySize > 0)
            bytesWritten += fwrite(registryStruct->category, sizeof(char), registryStruct->categorySize, binFile);

        // Checks if everything were written correctly (Register liquid size + isRemoved size + regSize size)
        if (bytesWritten != registryStruct->regSize + sizeof(char) + sizeof(int32_t))
            return false;

        return true;
    }
    return false;
}

// (Extern) Frees everything related to the header struct (its pointer included)
// Return value: If everything succeeded as expected (boolean)
bool freeVeiculoHeader(VeiculoHeader **header) {
    if (header && *header) {
        if ((*header)->prefixDescription)
            free((*header)->prefixDescription);
        if ((*header)->dateDescription)
            free((*header)->dateDescription);
        if ((*header)->seatsDescription)
            free((*header)->seatsDescription);
        if ((*header)->lineDescription)
            free((*header)->lineDescription);
        if ((*header)->modelDescription)
            free((*header)->modelDescription);
        if ((*header)->categoryDescription)
            free((*header)->categoryDescription);
            
        free(*header);
        return true;
    }
    return false;
}

// (Extern) Frees everything inside registry data struct (its pointer not included)
// Return value: If everything succeeded as expected (boolean)
bool freeVeiculoData(VeiculoData *data) {
    if (data) {
        free(data->prefix);
        if (data->date)
            free(data->date);
        if (data->modelSize > 0)
            free(data->model);
        if (data->categorySize > 0)
            free(data->category);
        return true;
    }
    return false;
}

// (Extern) Reads and builds the header struct from the binary file
// Return value: A pointer for the built struct (VeiculoHeader *)
VeiculoHeader *loadVeiculoBinaryHeader(FILE *binFile) {
    if (binFile) {
        size_t bytesRead = 0;

        // Check file consistency
        char fileStatus;
        bytesRead += fread(&fileStatus, 1, sizeof(char), binFile);
        if (fileStatus == INCONSISTENT_FILE[0] || bytesRead <= 0)
            return NULL;

        VeiculoHeader *newHeader = (VeiculoHeader *) malloc(sizeof *newHeader);
        newHeader->fileStatus = fileStatus;

        // Load numeric file statistics
        bytesRead += fread(&newHeader->byteNextReg, 1, sizeof(int64_t), binFile);
        bytesRead += fread(&newHeader->regNumber, 1, sizeof(int32_t), binFile);
        bytesRead += fread(&newHeader->removedRegNum, 1, sizeof(int32_t), binFile);

        // Load header strings
        newHeader->prefixDescription = (char *) malloc(PREFIX_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->prefixDescription, sizeof(char), PREFIX_DESC_SIZE, binFile);
        newHeader->dateDescription = (char *) malloc(DATE_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->dateDescription, sizeof(char), DATE_DESC_SIZE, binFile);
        newHeader->seatsDescription = (char *) malloc(SEATS_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->seatsDescription, sizeof(char), SEATS_DESC_SIZE, binFile);
        newHeader->lineDescription = (char *) malloc(LINE_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->lineDescription, sizeof(char), LINE_DESC_SIZE, binFile);
        newHeader->modelDescription = (char *) malloc(MODEL_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->modelDescription, sizeof(char), MODEL_DESC_SIZE, binFile);
        newHeader->categoryDescription = (char *) malloc(CATEGORY_DESC_SIZE*sizeof(char));
        bytesRead += fread(newHeader->categoryDescription, sizeof(char), CATEGORY_DESC_SIZE, binFile);
        
        if (bytesRead != VEICULO_HEADER_SIZE) {
            freeVeiculoHeader(&newHeader);
            return NULL;
        }
        return newHeader;

    }
    return NULL;
}

// (Extern) Reads and builds the registry struct (to an already existent struct) from the binary file
// Return value: If the read succeeded as expected (boolean)
bool loadVeiculoBinaryRegistry(FILE *binFile, VeiculoData *registryStruct) {
    if (binFile && registryStruct) {
        size_t bytesRead = 0;
        
        // Check registry removed status
        int32_t regSize;
        char registryStatus;
        if(!fread(&registryStatus, 1, sizeof(char), binFile)) {
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
        registryStruct->prefix = (char *) malloc(PREFIX_SIZE*sizeof(char));
        bytesRead += fread(registryStruct->prefix, sizeof(char), PREFIX_SIZE, binFile);
        
        registryStruct->date = (char *) malloc(DATE_SIZE*sizeof(char));
        bytesRead += fread(registryStruct->date, sizeof(char), DATE_SIZE, binFile);

        bytesRead += fread(&registryStruct->seatsNumber, 1, sizeof(int32_t), binFile);
        bytesRead += fread(&registryStruct->linhaCode, 1, sizeof(int32_t), binFile);

        // Load dynamic size fields
        bytesRead += fread(&registryStruct->modelSize, 1, sizeof(int32_t), binFile);
        if (registryStruct->modelSize > 0) {
            registryStruct->model = (char *) malloc(registryStruct->modelSize*sizeof(char));
            bytesRead += fread(registryStruct->model, sizeof(char), registryStruct->modelSize, binFile);
        }
        bytesRead += fread(&registryStruct->categorySize, 1, sizeof(int32_t), binFile);
        if (registryStruct->categorySize > 0) {
            registryStruct->category = (char *) malloc(registryStruct->categorySize*sizeof(char));
            bytesRead += fread(registryStruct->category, sizeof(char), registryStruct->categorySize, binFile);
        }

        // Check read status
        if (bytesRead != VEICULO_FIXED_SIZE + registryStruct->modelSize + registryStruct->categorySize) {
            freeVeiculoData(registryStruct);
            return false;
        }
        return true;
    }
    return false;
}

// (Extern) Shows formated recovered registry information in screen.
// Return value: None (void)
void printVeiculoRegistry(VeiculoHeader *header, VeiculoData *registry) {
    if (header && registry) {
        printf("%.*s: %.*s\n", PREFIX_DESC_SIZE, header->prefixDescription, PREFIX_SIZE, registry->prefix);

        printf("%.*s: ", MODEL_DESC_SIZE, header->modelDescription);
        if (registry->modelSize == 0)
            printf("%s\n", NULL_FIELD);
        else
            printf("%.*s\n", registry->modelSize, registry->model);

        printf("%.*s: ", CATEGORY_DESC_SIZE, header->categoryDescription);
        if (registry->categorySize == 0)
            printf("%s\n", NULL_FIELD);
        else
            printf("%.*s\n", registry->categorySize, registry->category);

        printf("%.*s: ", DATE_DESC_SIZE, header->dateDescription);
        if(registry->date[0] == '\0') {
            printf("%s\n", NULL_FIELD);
        }
        else {
            // Proccess date formatting
            int year, month;

            char *trackReference = registry->date;
            year = atoi(strsep(&registry->date, "-"));
            month = atoi(strsep(&registry->date, "-"));

            char day[3];
            strncpy(day, registry->date, 2);
            day[2] = '\0';
            registry->date = trackReference;
            
            printf("%s de ", day);
            switch(month) {
                case 1:
                    printf("janeiro");
                    break;
                case 2:
                    printf("fevereiro");
                    break;
                case 3:
                    printf("março");
                    break;
                case 4:
                    printf("abril");
                    break;
                case 5:
                    printf("maio");
                    break;
                case 6:
                    printf("junho");
                    break;
                case 7:
                    printf("julho");
                    break;
                case 8:
                    printf("agosto");
                    break;
                case 9:
                    printf("setembro");
                    break;
                case 10:
                    printf("outubro");
                    break;
                case 11:
                    printf("novembro");
                    break;
                case 12:
                    printf("dezembro");
                    break;
            }
            printf(" de %d\n", year);
        }

        printf("%.*s: ", SEATS_DESC_SIZE, header->seatsDescription);
        if(registry->seatsNumber == -1)
            printf("%s\n", NULL_FIELD);
        else
            printf("%d\n", registry->seatsNumber);

        printf("\n");
    }
}

void printRegistries(VeiculoHeader *veiculoHeader, VeiculoData *veiculoRegistry, LinhaHeader *linhaHeader, LinhaData *linhaRegistry) {
    if (veiculoHeader && veiculoRegistry) {
        printf("%.*s: %.*s\n", PREFIX_DESC_SIZE, veiculoHeader->prefixDescription, PREFIX_SIZE, veiculoRegistry->prefix);

        printf("%.*s: ", MODEL_DESC_SIZE, veiculoHeader->modelDescription);
        if (veiculoRegistry->modelSize == 0)
            printf("%s\n", NULL_FIELD);
        else
            printf("%.*s\n", veiculoRegistry->modelSize, veiculoRegistry->model);

        printf("%.*s: ", CATEGORY_DESC_SIZE, veiculoHeader->categoryDescription);
        if (veiculoRegistry->categorySize == 0)
            printf("%s\n", NULL_FIELD);
        else
            printf("%.*s\n", veiculoRegistry->categorySize, veiculoRegistry->category);

        printf("%.*s: ", DATE_DESC_SIZE, veiculoHeader->dateDescription);
        if(veiculoRegistry->date[0] == '\0') {
            printf("%s\n", NULL_FIELD);
        }
        else {
            // Proccess date formatting
            int year, month;

            char *trackReference = veiculoRegistry->date;
            year = atoi(strsep(&veiculoRegistry->date, "-"));
            month = atoi(strsep(&veiculoRegistry->date, "-"));

            char day[3];
            strncpy(day, veiculoRegistry->date, 2);
            day[2] = '\0';
            veiculoRegistry->date = trackReference;
            
            printf("%s de ", day);
            switch(month) {
                case 1:
                    printf("janeiro");
                    break;
                case 2:
                    printf("fevereiro");
                    break;
                case 3:
                    printf("março");
                    break;
                case 4:
                    printf("abril");
                    break;
                case 5:
                    printf("maio");
                    break;
                case 6:
                    printf("junho");
                    break;
                case 7:
                    printf("julho");
                    break;
                case 8:
                    printf("agosto");
                    break;
                case 9:
                    printf("setembro");
                    break;
                case 10:
                    printf("outubro");
                    break;
                case 11:
                    printf("novembro");
                    break;
                case 12:
                    printf("dezembro");
                    break;
            }
            printf(" de %d\n", year);
        }

        printf("%.*s: ", SEATS_DESC_SIZE, veiculoHeader->seatsDescription);
        if(veiculoRegistry->seatsNumber == -1)
            printf("%s\n", NULL_FIELD);
        else
            printf("%d\n", veiculoRegistry->seatsNumber);

    }

    if (linhaHeader && linhaRegistry) {
        printf("%.*s: %d\n", CODE_DESC_SIZE, linhaHeader->codeDescription, linhaRegistry->linhaCode);

        printf("%.*s: ", NAME_DESC_SIZE, linhaHeader->nameDescription);
        if (linhaRegistry->nameSize == 0)
            printf("%s\n", NULL_FIELD);
        else
            printf("%.*s\n", linhaRegistry->nameSize, linhaRegistry->linhaName);

        printf("%.*s: ", COLOR_DESC_SIZE, linhaHeader->colorDescription);
        if (linhaRegistry->colorSize == 0)
            printf("%s\n", NULL_FIELD);
        else
            printf("%.*s\n", linhaRegistry->colorSize, linhaRegistry->linhaColor);
        
        printf("%.*s: ", CARD_DESC_SIZE, linhaHeader->cardDescription);
        switch (linhaRegistry->cardAcceptance) {
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
        printf("\n");
    }
}
