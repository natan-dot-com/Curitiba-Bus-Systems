#ifndef _BTREE_H_
#define _BTREE_H_

    #include <stdlib.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include "Utility.h"

    // B-Tree definition constants
    #define DISK_PAGE_SIZE 77
    #define BTREE_ORDER 5
    #define MAX_KEYS 4

    #define IS_LEAF '1'
    #define IS_NOT_LEAF '0'

    #define EMPTY -1

    // Header liquid size = 9 bytes
    #define BTREE_HEADER_SIZE 9
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

    BTreeHeader *createBTree(const char *filename);
    BTreeHeader *openBTree(const char *filename);

    void insertOnBTree(BTreeHeader *fileHeader, int32_t newKey, int64_t newOffset);
    int64_t searchBTree(BTreeHeader *fileHeader, int32_t key);
    
    bool writeBTreeHeader(BTreeHeader *fileHeader);
    void freeBTree(BTreeHeader *fileHeader);

#endif
