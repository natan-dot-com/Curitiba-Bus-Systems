#ifndef _ERR_HANDLER_H_
#define _ERR_HANDLER_H_

    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>

    // General/error messages during execution
    #define FILE_PROC_ERROR "Falha no processamento do arquivo."
    #define INVALID_STRING "String de entrada inválida."
    #define REG_NOT_FOUND "Registro inexistente."

    // Error handling enum
    enum ErrHandling {
        ERR_FILE_PROC = 1,
        ERR_INVALID_STRING,
        ERR_REG_NOT_FOUND,
        ERR_UNEXPECTED = -1,
    };
    
    const char *matchError(int8_t errorHandler);

#endif