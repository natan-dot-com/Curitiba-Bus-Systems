#include "Functionalities.h"

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

// (Extern) Reads a CSV file of category "Veiculo" and write its respective binary file
// Return value: If everything succeeded as expected (boolean)
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
    return ERR_FILE_PROC;
}

// (Extern) Reads a CSV file of category "Linhas" and write its respective binary file
// Return value: File pointer to binary file (FILE *)
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
    return ERR_FILE_PROC;
}

int8_t sortVeiculoFile(char *binFilename, char* sortedFilename) {
    if (binFilename) {
        FILE *binFile = fopen(binFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
        }
        if (fileHeader->regNumber == 0) {
            freeVeiculoHeader(&fileHeader);
            fclose(binFile);
            return ERR_REG_NOT_FOUND;
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
    return ERR_FILE_PROC;
}

int8_t sortLinhaFile(char *binFilename, char* sortedFilename) {
    if (binFilename) {
        FILE *binFile = fopen(binFilename, "rb");
        if (!binFile) {
            return ERR_FILE_PROC;
        }

        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            return ERR_FILE_PROC;
        }
        if (fileHeader->regNumber == 0) {
            fclose(binFile);
            freeLinhaHeader(&fileHeader);
            return ERR_REG_NOT_FOUND;
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
    return ERR_FILE_PROC;
}

int8_t printVeiculoMerged(char *veiculoBinFilename, char *linhaBinFilename) {
    FILE *veiculoBinFile = fopen(veiculoBinFilename, "rb");
    if (!veiculoBinFile) {
        return ERR_FILE_PROC;
    }

    FILE *linhaBinFile = fopen(linhaBinFilename, "rb");
    if (!linhaBinFile) {
        fclose(veiculoBinFile);
        return ERR_FILE_PROC;
    }
    
    // Load file header
    VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
    if (!veiculoHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        return ERR_FILE_PROC;
    }
    if (veiculoHeader->regNumber == 0) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(&veiculoHeader);
        return ERR_REG_NOT_FOUND;
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

    // Read and displays each registry in screen
    VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
    LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);

    int mergedRegistries = 0;
    while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {

        if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
            
            if(newVeiculoRegistry->linhaCode != EMPTY) {
                // Skip Header
                fseek(linhaBinFile, LINHA_HEADER_SIZE, SEEK_SET);

                while(loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry)) {
                    if(newLinhaRegistry->isRemoved == VALID_REGISTRY) {
                        if(newVeiculoRegistry->linhaCode == newLinhaRegistry->linhaCode) {
                            // printVeiculoRegistry(veiculoHeader, newVeiculoRegistry);
                            // printLinhaRegistry(linhaHeader, newLinhaRegistry);
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

int8_t printVeiculoMergedWithBTree(char *veiculoBinFilename, char *linhaBinFilename, char *linhaBTreeFilename) {
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
    
    // Load file header
    VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
    if (!veiculoHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeBTree(bTreeHeader);
        return ERR_FILE_PROC;
    }
    if (veiculoHeader->regNumber == 0) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeBTree(bTreeHeader);
        freeVeiculoHeader(&veiculoHeader);
        return ERR_REG_NOT_FOUND;
    }

    LinhaHeader *linhaHeader = loadLinhaBinaryHeader(linhaBinFile);
    if (!linhaHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(&veiculoHeader);
        freeBTree(bTreeHeader);
        return ERR_FILE_PROC;
    }
    if (linhaHeader->regNumber == 0) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeBTree(bTreeHeader);
        freeVeiculoHeader(&veiculoHeader);
        freeLinhaHeader(&linhaHeader);
        return ERR_REG_NOT_FOUND;
    }

    // Read and displays each registry in screen
    VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
    LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);
    int mergedRegistries = 0;
    while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {
        if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
            
            if(newVeiculoRegistry->linhaCode != EMPTY) {
                int64_t offset = searchBTree(bTreeHeader, newVeiculoRegistry->linhaCode);
                if(offset != EMPTY) {
                    fseek(linhaBinFile, offset, SEEK_SET);
                    loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry);
                    // printVeiculoRegistry(veiculoHeader, newVeiculoRegistry);
                    // printLinhaRegistry(linhaHeader, newLinhaRegistry);
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

int8_t printVeiculoSortMerge(char *veiculoBinFilename, char *linhaBinFilename) {
    char *sortedVeiculoFilename = "VeiculoSorted";
    char *sortedLinhaFilename = "LinhaSorted";

    int8_t errorHandler;
    if (errorHandler = sortVeiculoFile(veiculoBinFilename, sortedVeiculoFilename)) {
        return errorHandler;
    }

    if (errorHandler = sortLinhaFile(linhaBinFilename, sortedLinhaFilename)) {
        return errorHandler;
    }

    FILE *veiculoBinFile = fopen(sortedVeiculoFilename, "rb");
    if (!veiculoBinFile) {
        return ERR_FILE_PROC;
    }

    FILE *linhaBinFile = fopen(sortedLinhaFilename, "rb");
    if (!linhaBinFile) {
        fclose(veiculoBinFile);
        return ERR_FILE_PROC;
    }
    
    // Load file header
    VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
    if (!veiculoHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        return ERR_FILE_PROC;
    }
    if (veiculoHeader->regNumber == 0) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(&veiculoHeader);
        return ERR_REG_NOT_FOUND;
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

    // Read and displays each registry in screen
    VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
    LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);
    loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry);
    int mergedRegistries = 0;
    while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {
        if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
            
            if(newVeiculoRegistry->linhaCode != EMPTY) {
                if(newLinhaRegistry->linhaCode == newVeiculoRegistry->linhaCode) {
                    // printVeiculoRegistry(veiculoHeader, newVeiculoRegistry);
                    // printLinhaRegistry(linhaHeader, newLinhaRegistry);
                    printRegistries(veiculoHeader, newVeiculoRegistry, linhaHeader, newLinhaRegistry);
                    mergedRegistries++;
                } else {
                    if(newLinhaRegistry->linhaCode > newVeiculoRegistry->linhaCode) {
                        fseek(linhaBinFile, LINHA_HEADER_SIZE, SEEK_SET);
                    }

                    freeLinhaData(newLinhaRegistry);
                    bool hasRegistry = false;
                    while(loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry)) {
                        if(newVeiculoRegistry->linhaCode == newLinhaRegistry->linhaCode) {
                            // printVeiculoRegistry(veiculoHeader, newVeiculoRegistry);
                            // printLinhaRegistry(linhaHeader, newLinhaRegistry);
                            printRegistries(veiculoHeader, newVeiculoRegistry, linhaHeader, newLinhaRegistry);
                            mergedRegistries++;
                            hasRegistry = true;
                            break;
                        } else if(newLinhaRegistry->linhaCode > newVeiculoRegistry->linhaCode) {
                            hasRegistry = true;
                            break;
                        }
                        freeLinhaData(newLinhaRegistry);
                    }
                    if(!hasRegistry) {
                        fseek(linhaBinFile, LINHA_HEADER_SIZE, SEEK_SET);
                        loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry);
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

// 11 13 16 17