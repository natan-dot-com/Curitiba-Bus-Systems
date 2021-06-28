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
            fwrite("@", DISK_PAGE_SIZE-1, sizeof(char), newTree->fp);

            newTree->fileStatus = CONSISTENT_FILE;
            newTree->nextNodeRRN = 1;
            newTree->rootNode = EMPTY;
        }
        return newTree;
    }
    return NULL;
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
    IndexStruct *tempIndex = (IndexStruct * )malloc(sizeof(IndexStruct) * (MAX_KEYS+1));
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

    IndexStruct *promotionIndex = (IndexStruct * )malloc(sizeof(IndexStruct));
    promotionIndex->key = tempIndex[MAX_KEYS/2].key;
    promotionIndex->regOffset = tempIndex[MAX_KEYS/2].regOffset;

    currNode->keyValues[MAX_KEYS/2].key = EMPTY;
        currNode->keyValues[MAX_KEYS/2].regOffset = EMPTY;
    BTreeNode *newNode = createNode(currNode->isLeaf);
    if(newNode) {
        
        newNode->nodeRRN = fileHeader->nextNodeRRN++;
        
        for(int j = MAX_KEYS/2 + 1, k = 0; j < MAX_KEYS + 1; j++, k++) {
            insertKeyOnNode(newNode, tempIndex[j].key, tempIndex[j].regOffset, currNode->childPointers[j], k);
            if(j < MAX_KEYS) {
                currNode->keyValues[j].key = EMPTY;
                currNode->keyValues[j].regOffset = EMPTY;
            }
        }
        currNode->keyCounter = MAX_KEYS/2;

        free(tempIndex);
        writeNode(fileHeader->fp, currNode);
        writeNode(fileHeader->fp, newNode);
    }
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
                        return split(fileHeader, currNode, newKey, newOffset, i);
                    } else {
                        insertKeyOnNode(currNode, promotedIndex->key, promotedIndex->regOffset, currNode->childPointers[i], i);
                        currNode->childPointers[i+1] = fileHeader->nextNodeRRN-1;
                        writeNode(fileHeader->fp, currNode);

                        return NULL;
                    }
                }
                return NULL;
            } else if(currNode->keyCounter == MAX_KEYS) {
                //split
                return split(fileHeader, currNode, newKey, newOffset, i);
            } else {
                insertKeyOnNode(currNode, newKey, newOffset, EMPTY, i);
                writeNode(fileHeader->fp, currNode);
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
            return;
        } else {
            IndexStruct *promotedIndex = insert(fileHeader, fileHeader->rootNode, newKey, newOffset);
            if(promotedIndex) {
                BTreeNode *newRoot = createNode(IS_NOT_LEAF);
                insertKeyOnNode(newRoot, promotedIndex->key, promotedIndex->regOffset, fileHeader->rootNode, 0);
                newRoot->childPointers[1] = fileHeader->nextNodeRRN - 1;
                fileHeader->rootNode = fileHeader->nextNodeRRN;
                newRoot->nodeRRN = fileHeader->nextNodeRRN++;
                writeNode(fileHeader->fp, newRoot);
            }
        }
    }
}

