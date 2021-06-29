#include "BTree.h"

#define EMPTY -1

BTreeHeader *createBTree(const char *filename) {
    if (filename != NULL) {
        BTreeHeader *newTree = (BTreeHeader *) malloc(sizeof(BTreeHeader));
        if (newTree) {
            newTree->fp = fopen(filename, "wb+");
            if (!newTree->fp) {
                return NULL;
            }

            // Skip header
            fwrite(INCONSISTENT_FILE, sizeof(char), 1, newTree->fp);

            // placeholder
            for (int i = 0; i < 76; i++)
                fwrite("@", sizeof(char), 1, newTree->fp);

            newTree->fileStatus = CONSISTENT_FILE;
            newTree->nextNodeRRN = 1;
            newTree->rootNode = EMPTY;
        }
        return newTree;
    }
    return NULL;
}

void freeBTree(BTreeHeader *fileHeader) {
    fclose(fileHeader->fp);
    free(fileHeader);
}

static bool writeNode(FILE *fp, BTreeNode *currNode) {
    if (fp && currNode) {
        fseek(fp, currNode->nodeRRN * DISK_PAGE_SIZE, SEEK_SET);
        
        fwrite(&currNode->isLeaf, sizeof(char), 1, fp);
        fwrite(&currNode->keyCounter, sizeof(int32_t), 1, fp);
        fwrite(&currNode->nodeRRN, sizeof(int32_t), 1, fp);
        for (int i = 0; i < 4; i++) {
            fwrite(&currNode->childPointers[i], sizeof(int32_t), 1, fp);
            fwrite(&currNode->keyValues[i].key, sizeof(int32_t), 1, fp);
            fwrite(&currNode->keyValues[i].regOffset, sizeof(int64_t), 1, fp);
        }
        fwrite(&currNode->childPointers[4], sizeof(int32_t), 1, fp);
        return true;
    }
    return false;
}

static BTreeNode *loadNode(FILE *fp, int32_t regRRN) {
    if (fp) {
        fseek(fp, regRRN * DISK_PAGE_SIZE , SEEK_SET);

        BTreeNode *node = (BTreeNode *) malloc(sizeof(BTreeNode));
        if (node) {
            fread(&node->isLeaf, sizeof(char), 1, fp);
            fread(&node->keyCounter, sizeof(int32_t), 1, fp);
            fread(&node->nodeRRN, sizeof(int32_t), 1, fp);
            for (int i = 0; i < 4; i++) {
                fread(&node->childPointers[i], sizeof(int32_t), 1, fp);
                fread(&node->keyValues[i].key, sizeof(int32_t), 1, fp);
                fread(&node->keyValues[i].regOffset, sizeof(int64_t), 1, fp);
            }
            fread(&node->childPointers[4], sizeof(int32_t), 1, fp);
            return node;
        }
        return node;
    }
    return NULL;
}

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

void freeNode(BTreeNode *node) {
    free(node);
}

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
    //printf("no promovido: %d\n", promotionIndex->key);
    return promotionIndex;
}

static IndexStruct *insert(BTreeHeader *fileHeader, int32_t regRRN, int32_t newKey, int64_t newOffset) {
    BTreeNode *currNode = loadNode(fileHeader->fp, regRRN);
    
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

void insertOnBTree(BTreeHeader *fileHeader, int32_t newKey, int64_t newOffset) {
    if (fileHeader) {
        if (fileHeader->rootNode == EMPTY) {
            BTreeNode *root = createNode(IS_LEAF);
            root->keyCounter++;
            root->nodeRRN = 1;
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

static int64_t search(FILE *fp, int32_t RRN, int32_t key) {
    BTreeNode *node = loadNode(fp, RRN);
    if(node) {
        for(int i = 0; i <= node->keyCounter; i++) {
            if(i == node->keyCounter || key < node->keyValues[i].key) {
                int32_t childRRN = node->childPointers[i];
                freeNode(node);
                return search(fp, childRRN, key);
            }
            if(node->keyValues[i].key == key) {
                int64_t offset = node->keyValues[i].regOffset;
                freeNode(node);
                return offset;
            }
        }
    } else {
        return -1;
    }
}

int64_t searchBTree(BTreeHeader *fileHeader, int32_t key) {
    return search(fileHeader->fp, fileHeader->rootNode, key);
}

static void printBTreeRec(FILE* fp, BTreeHeader *fileHeader, int RRN, int depth) {
    BTreeNode *node = loadNode(fileHeader->fp, RRN);
    for(int i = 0; i < depth; i++) {
        fprintf(fp, "\t");
        
    }
    fprintf(fp, "RRN: %d -> ", node->nodeRRN);
    for(int i = 0; i < node->keyCounter; i++) {
        //fprintf(fp, "%d: %ld | ", node->keyValues[i].key, node->keyValues[i].regOffset);
        fprintf(fp, "%d | ", node->keyValues[i].key);
    }
    fprintf(fp,"\n");
    if(node->isLeaf == IS_NOT_LEAF) {
        for(int i = 0; i <= node->keyCounter; i++) {
            printBTreeRec(fp, fileHeader, node->childPointers[i], depth+1);
        }
    }
    freeNode(node);
}

void printBTree(BTreeHeader *fileHeader) {
    FILE *fp = fopen("btreeOut.txt", "w");
    printBTreeRec(fp, fileHeader, fileHeader->rootNode, 0);
    fclose(fp);
}