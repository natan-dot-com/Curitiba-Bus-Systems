#include "Functionalities.h"

// (Extern) Reads a CSV file of category "Veiculo" and write its respective binary file
// Return value: If everything succeeded as expected (boolean)
bool createVeiculoBinaryFile(char *csvFilename, char *binFilename) {
    if (csvFilename && strlen(csvFilename) > 0) {
        FILE *csvFile = fopen(csvFilename, "r");
        if (!csvFile) {
            return false;
        }

        VeiculoHeader *fileHeader = readVeiculoHeader(csvFile);
        if (!fileHeader) {
            fclose(csvFile);
            return false;
        }

        FILE *binFile = fopen(binFilename, "wb+");
        if (!binFile) {
            fclose(csvFile);
            freeVeiculoHeader(&fileHeader);
            return false;
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
        return true;
    }
    return false;
}

// (Extern) Reads a CSV file of category "Linhas" and write its respective binary file
// Return value: File pointer to binary file (FILE *)
bool createLinhaBinaryFile(char *csvFilename, char *binFilename) {
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
        return true;
    }
    return false;
}

int veiculoComparar(VeiculoData *data1, VeiculoData *data2) {
    return data1->linhaCode - data2->linhaCode;
}

int linhaComparar(LinhaData *data1, LinhaData *data2) {
    return data1->linhaCode - data2->linhaCode;
}

bool sortVeiculoFile(char *binFilename, char* sortedFilename) {
    if (binFilename) {
        FILE *binFile = fopen(binFilename, "rb");
        if (!binFile) {
            printf("%s\n", FILE_ERROR);
            return false;
        }

        VeiculoHeader *fileHeader = loadVeiculoBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            printf("%s\n", FILE_ERROR);
            return false;
        }
        fileHeader->fileStatus = CONSISTENT_FILE;
        fileHeader->removedRegNum = 0;

        FILE *sortedFile = fopen(sortedFilename, "wb+");
        if (sortedFile) {
            fclose(binFile);
            freeVeiculoHeader(&fileHeader);
            printf("%s\n", FILE_ERROR);
            return false;
        }

        fwrite(INCONSISTENT_FILE, sizeof(char), 1, sortedFile);
        fwrite("@", sizeof(char), VEICULO_HEADER_SIZE-1, sortedFile);
        
        VeiculoData **veiculoList = (VeiculoData **) malloc(fileHeader->regNumber * sizeof(VeiculoData *));
        for (int i = 0; i < fileHeader->regNumber; i++) {
            loadVeiculoBinaryRegistry(binFile, veiculoList[i]);
            if (veiculoList[i] == NULL) {
                i--;
            }
        }
        fclose(binFile);

        qsort(veiculoList, fileHeader->regNumber, sizeof(VeiculoData *), veiculoComparar);
        for (int i = 0; i < fileHeader->regNumber; i++) {
            writeVeiculoRegistryOnBinary(sortedFile, veiculoList[i]);
            freeVeiculoData(veiculoList[i]);
        }

        fileHeader->byteNextReg = ftell(sortedFile);
        rewind(sortedFile);
        writeVeiculoHeaderOnBinary(sortedFile, fileHeader);

        fclose(sortedFile);
        freeVeiculoHeader(fileHeader);
        free(veiculoList);

        binarioNaTela(sortedFilename);
    }
}

bool sortLinhaFile(char *binFilename, char* sortedFilename) {
    if (binFilename) {
        FILE *binFile = fopen(binFilename, "rb");
        if (!binFile) {
            printf("%s\n", FILE_ERROR);
            return false;
        }

        LinhaHeader *fileHeader = loadLinhaBinaryHeader(binFile);
        if (!fileHeader) {
            fclose(binFile);
            printf("%s\n", FILE_ERROR);
            return false;
        }
        fileHeader->fileStatus = CONSISTENT_FILE;
        fileHeader->removedRegNum = 0;

        FILE *sortedFile = fopen(sortedFilename, "wb+");
        if (sortedFile) {
            fclose(binFile);
            freeLinhaHeader(&fileHeader);
            printf("%s\n", FILE_ERROR);
            return false;
        }

        fwrite(INCONSISTENT_FILE, sizeof(char), 1, sortedFile);
        fwrite("@", sizeof(char), LINHA_HEADER_SIZE-1, sortedFile);
        
        LinhaData **linhaList = (LinhaData **) malloc(fileHeader->regNumber * sizeof(LinhaData *));
        for (int i = 0; i < fileHeader->regNumber; i++) {
            loadLinhaBinaryRegistry(binFile, linhaList[i]);
            if (linhaList[i] == NULL) {
                i--;
            }
        }
        fclose(binFile);

        qsort(linhaList, fileHeader->regNumber, sizeof(LinhaData *), linhaComparar);
        for (int i = 0; i < fileHeader->regNumber; i++) {
            writeLinhaRegistryOnBinary(sortedFile, linhaList[i]);
            freeLinhaData(linhaList[i]);
        }

        fileHeader->byteNextReg = ftell(sortedFile);
        rewind(sortedFile);
        writeLinhaHeaderOnBinary(sortedFile, fileHeader);

        fclose(sortedFile);
        freeLinhaHeader(fileHeader);
        free(linhaList);

        binarioNaTela(sortedFilename);
    }
}

void printVeiculoMerged(char *veiculoBinFilename, char *linhaBinFilename) {
    FILE *veiculoBinFile = fopen(veiculoBinFilename, "rb");
    if (!veiculoBinFile) {
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    FILE *linhaBinFile = fopen(linhaBinFilename, "rb");
    if (!linhaBinFile) {
        fclose(veiculoBinFile);
        printf("%s\n", FILE_ERROR);
        return 0;
    }
    
    // Load file header
    VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
    if (!veiculoHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    LinhaHeader *linhaHeader = loadLinhaBinaryHeader(linhaBinFile);
    if (!linhaHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(veiculoHeader);
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    if (veiculoHeader->regNumber == 0 || linhaHeader->regNumber == 0) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(veiculoHeader);
        freeLinhaHeader(linhaHeader);
        printf("%s\n", REG_NOT_FOUND);
        return 0;
    }

    // Read and displays each registry in screen
    VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
    LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);
    while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {
        if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
            printVeiculoRegistry(veiculoHeader, newVeiculoRegistry);
            
            if(newVeiculoRegistry->linhaCode != EMPTY) {
                // Skip Header
                fseek(linhaBinFile, LINHA_HEADER_SIZE, SEEK_SET);

                while(loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry)) {
                    if(newVeiculoRegistry->linhaCode == newLinhaRegistry->linhaCode) {
                        break;
                    } else {
                        freeLinhaData(newLinhaRegistry);
                    }
                }
                printLinhaRegistry(linhaHeader, newLinhaRegistry);

                freeLinhaData(newLinhaRegistry);
                
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
}

void printVeiculoMergedWithBTree(char *veiculoBinFilename, char *linhaBinFilename, char *linhaBTreeFilename) {
    FILE *veiculoBinFile = fopen(veiculoBinFilename, "rb");
    if (!veiculoBinFile) {
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    FILE *linhaBinFile = fopen(linhaBinFilename, "rb");
    if (!linhaBinFile) {
        fclose(veiculoBinFile);
        printf("%s\n", FILE_ERROR);
        return 0;
    }
    
    BTreeHeader *bTreeHeader = openBTree(linhaBTreeFilename);
    if(!bTreeHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        printf("%s\n", FILE_ERROR);
        return 0;
    }
    
    // Load file header
    VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
    if (!veiculoHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeBTree(bTreeHeader);
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    LinhaHeader *linhaHeader = loadLinhaBinaryHeader(linhaBinFile);
    if (!linhaHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(veiculoHeader);
        freeBTree(bTreeHeader);
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    if (veiculoHeader->regNumber == 0 || linhaHeader->regNumber == 0) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(veiculoHeader);
        freeBTree(bTreeHeader);
        freeLinhaHeader(linhaHeader);
        printf("%s\n", REG_NOT_FOUND);
        return 0;
    }

    // Read and displays each registry in screen
    VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
    LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);
    while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {
        if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
            printVeiculoRegistry(veiculoHeader, newVeiculoRegistry);
            
            if(newVeiculoRegistry->linhaCode != EMPTY) {
                int64_t offset = searchBTree(bTreeHeader, newVeiculoRegistry->linhaCode);
                if(offset != EMPTY) {
                    fseek(linhaBinFile, offset, SEEK_SET);
                    loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry);
                    printLinhaRegistry(linhaHeader, newLinhaRegistry);

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
}

void printVeiculoSortMerge(char *veiculoBinFilename, char *linhaBinFilename) {
    
    char sortedVeiculoFilename[] = "VeiculoSorted";
    char sortedLinhaFilename[] = "LinhaSorted";

    if(!sortVeiculoFile(veiculoBinFilename, sortedVeiculoFilename)) {
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    if(!sortLinhaFile(linhaBinFilename, sortedLinhaFilename)) {
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    FILE *veiculoBinFile = fopen(sortedVeiculoFilename, "rb");
    if (!veiculoBinFile) {
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    FILE *linhaBinFile = fopen(sortedLinhaFilename, "rb");
    if (!linhaBinFile) {
        fclose(veiculoBinFile);
        printf("%s\n", FILE_ERROR);
        return 0;
    }
    
    // Load file header
    VeiculoHeader *veiculoHeader = loadVeiculoBinaryHeader(veiculoBinFile);
    if (!veiculoHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    LinhaHeader *linhaHeader = loadLinhaBinaryHeader(linhaBinFile);
    if (!linhaHeader) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(veiculoHeader);
        printf("%s\n", FILE_ERROR);
        return 0;
    }

    if (veiculoHeader->regNumber == 0 || linhaHeader->regNumber == 0) {
        fclose(veiculoBinFile);
        fclose(linhaBinFile);
        freeVeiculoHeader(veiculoHeader);
        freeLinhaHeader(linhaHeader);
        printf("%s\n", REG_NOT_FOUND);
        return 0;
    }

    
    // Read and displays each registry in screen
    VeiculoData *newVeiculoRegistry = (VeiculoData *) malloc(sizeof *newVeiculoRegistry);
    
    LinhaData *newLinhaRegistry = (LinhaData *) malloc(sizeof *newLinhaRegistry);
    loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry);

    while (loadVeiculoBinaryRegistry(veiculoBinFile, newVeiculoRegistry)) {
        if (newVeiculoRegistry->isRemoved == VALID_REGISTRY) {
            printVeiculoRegistry(veiculoHeader, newVeiculoRegistry);
            
            if(newVeiculoRegistry->linhaCode != EMPTY) {
                if(newVeiculoRegistry->linhaCode != newLinhaRegistry->linhaCode) {
                    freeLinhaData(newLinhaRegistry);
                    while(loadLinhaBinaryRegistry(linhaBinFile, newLinhaRegistry)) {
                        if(newVeiculoRegistry->linhaCode == newLinhaRegistry->linhaCode) {
                            break;
                        } else {
                            freeLinhaData(newLinhaRegistry);
                        }
                    }
                }
                printLinhaRegistry(linhaHeader, newLinhaRegistry);
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
}