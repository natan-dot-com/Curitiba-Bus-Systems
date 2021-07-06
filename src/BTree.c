#include "BTree.h"

// (Extern) Frees current loaded B-Tree struct.
// Return value: none (void)
void freeBTree(BTreeHeader *fileHeader) {
    if (fileHeader) {
        fclose(fileHeader->fp);
        free(fileHeader);
    }
}

// (Extern) Creates a B-Tree header from scratch based on given file name
// Return value: A pointer to the header struct of the given file (BTreeHeader *)
BTreeHeader *createBTree(const char *filename) {
    size_t bytesWritten = 0;
    if (filename != NULL) {
        BTreeHeader *newTree = (BTreeHeader *) malloc(sizeof(BTreeHeader));
        if (newTree) {
            newTree->fp = fopen(filename, "wb+");
            if (!newTree->fp) {
                return NULL;
            }

            // Skip header
            bytesWritten += fwrite(INCONSISTENT_FILE, sizeof(char), 1, newTree->fp);
            for (int i = 0; i < 76; i++)
                bytesWritten += fwrite("@", sizeof(char), 1, newTree->fp);

            if (bytesWritten != DISK_PAGE_SIZE) {
                freeBTree(newTree);
                return NULL; 
            }

            newTree->fileStatus = CONSISTENT_FILE;
            newTree->nextNodeRRN = 0;
            newTree->rootNode = EMPTY;
        }
        return newTree;
    }
    return NULL;
}

// (Extern) Writes current header from RAM into the file
// Return value: If the write succeeded (boolean)
bool writeBTreeHeader(BTreeHeader *fileHeader) {
    size_t bytesWritten = 0;
    if (fileHeader) {
        bytesWritten += fwrite(&fileHeader->fileStatus, sizeof(char), 1, fileHeader->fp);
        bytesWritten += fwrite(&fileHeader->rootNode, sizeof(int32_t), 1, fileHeader->fp);
        bytesWritten += fwrite(&fileHeader->nextNodeRRN, sizeof(int32_t), 1, fileHeader->fp);

        if (bytesWritten != BTREE_HEADER_SIZE)
            return false;

        return true;
    }
    return false;
}

// (Extern) Opens an already existent B-Tree file header.
// Return value: A pointer to the header struct of the given file (BTreeHeader *)
BTreeHeader *openBTree(const char *filename) {
    if (filename != NULL) {
        size_t bytesRead = 0;
        BTreeHeader *fileHeader = (BTreeHeader *) malloc(sizeof(BTreeHeader));

        if (fileHeader) {
            fileHeader->fp = fopen(filename, "rb+");

            if(fileHeader->fp != NULL) {
                char fileStatus;
                fread(&fileStatus, 1, sizeof(char), fileHeader->fp);
                if(fileStatus != CONSISTENT_FILE) {
                    freeBTree(fileHeader);
                    return NULL;
                }

                rewind(fileHeader->fp);
                bytesRead += fwrite(INCONSISTENT_FILE, sizeof(char), 1, fileHeader->fp);
                fflush(fileHeader->fp);

                bytesRead += fread(&fileHeader->rootNode, 1, sizeof(int32_t), fileHeader->fp);
                bytesRead += fread(&fileHeader->nextNodeRRN, 1, sizeof(int32_t), fileHeader->fp);
                fileHeader->fileStatus = CONSISTENT_FILE;

                if (bytesRead != BTREE_HEADER_SIZE) {
                    freeBTree(fileHeader); 
                    return NULL;
                }
                
                return fileHeader;
            }
            free(fileHeader);
            return NULL;
        }
        return NULL;
    }
    return NULL;
}

// (Static) Writes an spectific node in the next RRN of current B-Tree.
// Return value: If the write succeeded (boolean)
static bool writeNode(FILE *fp, BTreeNode *currNode) {
    if (fp && currNode) {
        size_t bytesWritten = 0;
        fseek(fp, (currNode->nodeRRN + 1) * DISK_PAGE_SIZE, SEEK_SET);
        
        bytesWritten += fwrite(&currNode->isLeaf, sizeof(char), 1, fp);
        bytesWritten += fwrite(&currNode->keyCounter, sizeof(int32_t), 1, fp);
        bytesWritten += fwrite(&currNode->nodeRRN, sizeof(int32_t), 1, fp);
        for (int i = 0; i < 4; i++) {
            bytesWritten += fwrite(&currNode->childPointers[i], sizeof(int32_t), 1, fp);
            bytesWritten += fwrite(&currNode->keyValues[i].key, sizeof(int32_t), 1, fp);
            bytesWritten += fwrite(&currNode->keyValues[i].regOffset, sizeof(int64_t), 1, fp);
        }
        bytesWritten += fwrite(&currNode->childPointers[4], sizeof(int32_t), 1, fp);

        if (bytesWritten != DISK_PAGE_SIZE)
            return false;

        return true;
    }
    return false;
}

// (Static) Frees an existent B-Tree node from memory.
// Return value: none (void)
static void freeNode(BTreeNode *node) {
    if (node)
        free(node);
}

// (Static) Loads an spectific B-Tree node at given RNN.
// Return value: A pointer to the loaded node struct (BTreeNode *)
static BTreeNode *loadNode(FILE *fp, int32_t regRRN) {
    if (fp) {
        size_t bytesRead = 0;
        fseek(fp, (regRRN + 1) * DISK_PAGE_SIZE , SEEK_SET);

        BTreeNode *node = (BTreeNode *) malloc(sizeof(BTreeNode));
        if (node) {
            bytesRead += fread(&node->isLeaf, 1, sizeof(char), fp);
            bytesRead += fread(&node->keyCounter, 1, sizeof(int32_t), fp);
            bytesRead += fread(&node->nodeRRN, 1, sizeof(int32_t), fp);
            for (int i = 0; i < 4; i++) {
                bytesRead += fread(&node->childPointers[i], 1, sizeof(int32_t), fp);
                bytesRead += fread(&node->keyValues[i].key, 1, sizeof(int32_t), fp);
                bytesRead += fread(&node->keyValues[i].regOffset, 1, sizeof(int64_t), fp);
            }
            bytesRead += fread(&node->childPointers[4], 1, sizeof(int32_t), fp);

            if (bytesRead != DISK_PAGE_SIZE) {
                freeNode(node);
                return NULL;
            }

            return node;
        }
        return NULL;
    }
    return NULL;
}

// (Static) Creates a new B-Tree node.
// Return value: A pointer to the loaded node struct (BTreeNode *)
static BTreeNode *createNode(char isLeaf) {
    BTreeNode *newNode = (BTreeNode *) malloc(sizeof(BTreeNode));
    if (newNode) {
        newNode->isLeaf = isLeaf;
        newNode->keyCounter = 0;
        newNode->nodeRRN = EMPTY;
        for(int i = 0; i < 5; i++) {
            newNode->childPointers[i] = EMPTY;
            if (i < 4) {
                newNode->keyValues[i].key = EMPTY;
                newNode->keyValues[i].regOffset = EMPTY;
            }
        }
    }
    return newNode;
}

// (Static) Inserts a given pair (KEY,OFFSET) into the current node.
// Return value: If the insertion succeeded (boolean)
static bool insertKeyOnNode(BTreeNode *node, int32_t newKey, int64_t newOffset, int32_t childRRN, int i) {
    if (node) {
        for(int j = MAX_KEYS - 1; j > i; j--) {
            node->keyValues[j].key = node->keyValues[j - 1].key;
            node->keyValues[j].regOffset = node->keyValues[j - 1].regOffset;
            node->childPointers[j+1] = node->childPointers[j];
        }
        node->keyValues[i].key = newKey;
        node->keyValues[i].regOffset = newOffset;
        node->childPointers[i] = childRRN;

        node->keyCounter++;
        return true;
    }
    return false;
}

// (Static) Splits a full B-Tree node into two with BTREE_ORDER/2 elements.
// Return value: A pointer to the promoted index (IndexStruct *)
static IndexStruct *split(BTreeHeader *fileHeader, BTreeNode *currNode, int32_t newKey, int64_t newOffset, int32_t i) {
    IndexStruct *tempIndex = (IndexStruct *) malloc(sizeof(IndexStruct) * (MAX_KEYS+1));
    for(int j = 0, k = 0; j < MAX_KEYS + 1; j++, k++) {
        if(j == i) {
            tempIndex[j].key = newKey; 
            tempIndex[j].regOffset = newOffset;

            j++;
        }
        if(k < MAX_KEYS) {
            tempIndex[j] = currNode->keyValues[k];
        }
    }

    int32_t *tempPointers = (int *) malloc(sizeof(int) * (BTREE_ORDER + 1));
    memset(tempPointers, -1, sizeof(int) * (BTREE_ORDER + 1));
    if(currNode->isLeaf == IS_NOT_LEAF) {
        for(int j = 0, k = 0; k < BTREE_ORDER; j++, k++) {
            tempPointers[j] = currNode->childPointers[k];
            if(j == i) {
                tempPointers[++j] = fileHeader->nextNodeRRN - 1;
            }
        }

    }

    IndexStruct *promotionIndex = (IndexStruct *) malloc(sizeof(IndexStruct));
    promotionIndex->key = tempIndex[MAX_KEYS/2].key;
    promotionIndex->regOffset = tempIndex[MAX_KEYS/2].regOffset;

    currNode->keyValues[MAX_KEYS/2].key = EMPTY;
    currNode->keyValues[MAX_KEYS/2].regOffset = EMPTY;
    BTreeNode *newNode = createNode(currNode->isLeaf);
    if(newNode) {
        
        newNode->nodeRRN = fileHeader->nextNodeRRN++;
        
        for(int j = MAX_KEYS/2 + 1, k = 0; j < MAX_KEYS + 1; j++, k++) {
            insertKeyOnNode(newNode, tempIndex[j].key, tempIndex[j].regOffset, tempPointers[j], k);
            currNode->childPointers[j] = EMPTY;
            if(j < MAX_KEYS) {
                currNode->keyValues[j].key = EMPTY;
                currNode->keyValues[j].regOffset = EMPTY;
            }

            currNode->keyValues[k].key = tempIndex[k].key;
            currNode->keyValues[k].regOffset = tempIndex[k].regOffset;
            currNode->childPointers[k] = tempPointers[k];
        }
        currNode->childPointers[BTREE_ORDER/2] = tempPointers[BTREE_ORDER/2];
        newNode->childPointers[BTREE_ORDER/2] = tempPointers[BTREE_ORDER];
        
        currNode->keyCounter = MAX_KEYS/2;

        free(tempIndex);
        free(tempPointers);
        writeNode(fileHeader->fp, currNode);
        writeNode(fileHeader->fp, newNode);
        freeNode(newNode);
        freeNode(currNode);
    }
    return promotionIndex;
}

// (Static) Recursive auxiliar insertion of a new pair (KEY,OFFSET) into the given B-Tree.
// Return value: A pointer to the promoted index (IndexStruct *)
static IndexStruct *insert(BTreeHeader *fileHeader, int32_t regRRN, int32_t newKey, int64_t newOffset) {
    BTreeNode *currNode = loadNode(fileHeader->fp, regRRN);
    
    if(!currNode) {
        return NULL;
    }
    
    for(int i = 0; i <= currNode->keyCounter; i++) {
        if(i == currNode->keyCounter || newKey < currNode->keyValues[i].key) {
            if(currNode->childPointers[i] != EMPTY) {
                IndexStruct *promotedIndex = insert(fileHeader, currNode->childPointers[i], newKey, newOffset);
                if(promotedIndex) {
                    if(currNode->keyCounter == MAX_KEYS) {
                        int32_t promotedKey = promotedIndex->key;
                        int64_t promotedOffset = promotedIndex->regOffset;
                        free(promotedIndex);
                        return split(fileHeader, currNode, promotedKey, promotedOffset, i);
                    } else {
                        insertKeyOnNode(currNode, promotedIndex->key, promotedIndex->regOffset, currNode->childPointers[i], i);
                        currNode->childPointers[i+1] = fileHeader->nextNodeRRN-1;
                        writeNode(fileHeader->fp, currNode);

                        free(promotedIndex);
                        freeNode(currNode);
                        return NULL;
                    }
                }
                freeNode(currNode);
                return NULL;
            } else if(currNode->keyCounter == MAX_KEYS) {
                //split
                return split(fileHeader, currNode, newKey, newOffset, i);
            } else {
                insertKeyOnNode(currNode, newKey, newOffset, EMPTY, i);
                writeNode(fileHeader->fp, currNode);
                
                freeNode(currNode);
                return NULL;
            }
        }
    }
}

// (Extern) Inserts a new pair (KEY,OFFSET) into the given B-Tree.
// Return value: A pointer to the promoted index (IndexStruct *)
void insertOnBTree(BTreeHeader *fileHeader, int32_t newKey, int64_t newOffset) {
    if (fileHeader) {
        if (fileHeader->rootNode == EMPTY) {
            BTreeNode *root = createNode(IS_LEAF);
            root->keyCounter++;
            root->nodeRRN = 0;
            root->keyValues[0].key = newKey;
            root->keyValues[0].regOffset = newOffset; 

            fileHeader->rootNode = root->nodeRRN;
            fileHeader->nextNodeRRN++;

            writeNode(fileHeader->fp, root);
            freeNode(root);
            return;
        } else {
            IndexStruct *promotedIndex = insert(fileHeader, fileHeader->rootNode, newKey, newOffset);
            if(promotedIndex) {
                BTreeNode *newRoot = createNode(IS_NOT_LEAF);
                insertKeyOnNode(newRoot, promotedIndex->key, promotedIndex->regOffset, fileHeader->rootNode, 0);
                newRoot->childPointers[1] = fileHeader->nextNodeRRN - 1;
                fileHeader->rootNode = fileHeader->nextNodeRRN;
                newRoot->nodeRRN = fileHeader->nextNodeRRN++;

                free(promotedIndex);
                writeNode(fileHeader->fp, newRoot);
                freeNode(newRoot);
            }
        }
    }
}

// (Static) Recursive auxiliar search of a given key in B-Tree.
// Return value: The respective offset of the key in data file (int64_t)
static int64_t search(FILE *fp, int32_t RRN, int32_t key) {
    BTreeNode *node = loadNode(fp, RRN);
    if(node) {
        for(int i = 0; i <= node->keyCounter; i++) {
            if(i == node->keyCounter || key < node->keyValues[i].key) {
                int32_t childRRN = node->childPointers[i];
                freeNode(node);

                if(childRRN == EMPTY)
                    return EMPTY;
                    
                return search(fp, childRRN, key);
            }
            if(node->keyValues[i].key == key) {
                int64_t offset = node->keyValues[i].regOffset;
                freeNode(node);
                return offset;
            }
        }
        return EMPTY;
    } 
    return EMPTY;
}

// (Extern) Searches a given key in B-Tree.
// Return value: The respective offset of the key in data file (int64_t)
int64_t searchBTree(BTreeHeader *fileHeader, int32_t key) {
    if(fileHeader) {
        return search(fileHeader->fp, fileHeader->rootNode, key);
    }
    return EMPTY;
}
