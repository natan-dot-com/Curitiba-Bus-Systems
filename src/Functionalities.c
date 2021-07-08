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
