#ifndef _FUNC_H_
#define _FUNC_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include "VeiculoFileManager.h"
    #include "LinhaFileManager.h"

    bool createLinhaBinaryFile(char *csvFilename, char *binFilename);
    bool createVeiculoBinaryFile(char *csvFilename, char *binFilename);

#endif