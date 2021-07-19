#include "Sort.h"

inline int veiculoCompare(const void *data1, const void *data2) {
    //printf("1: %d, 2: %d\n", (*(VeiculoData **) data1)->linhaCode, (*(VeiculoData **) data2)->linhaCode);
    return (*(VeiculoData **) data1)->linhaCode - (*(VeiculoData **) data2)->linhaCode;
}

inline int linhaCompare(const void *data1, const void *data2) {
    return (*(LinhaData **) data1)->linhaCode - (*(LinhaData **) data2)->linhaCode;
}
