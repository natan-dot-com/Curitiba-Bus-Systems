#include "BTree.h"

BTreeNode *createTreeNode(const char isLeaf, int32_t nodeRRN) {
    BTreeNode *newNode = (BTreeNode *) malloc(sizeof(BTreeNode));
    if (newNode) {
        newNode->isLeaf = isLeaf;
        newNode->keyCounter = 0;
        newNode->nodeRRN = nodeRRN;
        newNode->childPointers = (int32_t *) malloc(BTREE_ORDER * sizeof(int32_t));
        newNode->keyValues = (IndexStruct **) malloc((BTREE_ORDER-1) * sizeof(IndexStruct *));
    }
    return newNode;
}

int64_t searchNode(int32_t searchKey) {

}
