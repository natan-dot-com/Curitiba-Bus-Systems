#ifndef _BTREE_H_
#define _BTREE_H_

    #include <stdlib.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include "Utility.h"

    #define DISK_PAGE_SIZE 77
    #define BTREE_ORDER 5
    #define MAX_KEYS 4

    // Header liquid size = 9 bytes
    typedef struct _BTreeHeader {
        FILE *fp;
        char fileStatus;
        int32_t rootNode;
        int32_t nextNodeRRN;
    } BTreeHeader;

    // BTree data registry liquid size = DISK_PAGE_SIZE
    typedef struct _IndexStruct {
        int32_t key;
        int64_t regOffset;
    } IndexStruct;

    typedef struct _BTreeNode {
        char isLeaf;
        int32_t keyCounter;
        int32_t nodeRRN;
        int32_t childPointers[5];
        IndexStruct keyValues[4];
    } BTreeNode;

    void insertOnBTree(BTreeHeader *fileHeader, int32_t newKey, int64_t newOffset);
    BTreeHeader *createBTree(const char *filename);

#endif
