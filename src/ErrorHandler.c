#include "ErrorHandler.h"

inline const char *matchError(int8_t errorHandler) {
    switch (errorHandler) {
        case ERR_FILE_PROC: return FILE_PROC_ERROR;
        case ERR_REG_NOT_FOUND: return REG_NOT_FOUND;
        case ERR_INVALID_STRING: return INVALID_STRING;
        default: exit(ERR_UNEXPECTED);
    }
}