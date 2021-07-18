#ifndef _ERR_HANDLER_H_
#define _ERR_HANDLER_H_

    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>

    // General/error messages during execution
    #define FILE_PROC_ERROR "Falha no processamento do arquivo."
    #define FILE_LOAD_ERROR "Falha no carregamento do arquivo."
    #define REG_NOT_FOUND "Registro inexistente."

    // Error handling enum
    enum ErrHandling {
        ERR_FILE_PROC = 1,
        ERR_FILE_LOAD,
        ERR_REG_NOT_FOUND,
        ERR_UNEXPECTED = -1,
    };
    
    const char *matchError(int8_t errorHandler);

#endif