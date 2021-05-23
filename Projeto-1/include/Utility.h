#ifndef _UTILITY_H_
#define _UTILITY_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h>
    #include <stdbool.h>

    // Delimiters for strsep
    #define NONE ""
    #define QUOTES_DELIM "\""
    #define SPACE_DELIM " "
    #define COMMA_DELIM ","
    #define LINE_BREAK "\n"

    // Registry status constants
    #define REMOVED_REGISTRY '0'
    #define VALID_REGISTRY '1'

    // General/error messages during execution
    #define NULL_FIELD "campo com valor nulo"
    #define FILE_ERROR "Falha no processamento do arquivo."
    #define REG_NOT_FOUND "Registro inexistente."

    // Consistent/inconsistent file signature (for fileStatus in header)
    #define INCONSISTENT_FILE "0"
    #define CONSISTENT_FILE '1'

    // Type identifier for read field (used in functionalities 5 and 6)
    #define INTEGER_TYPE 'I'
    #define CHAR_TYPE 'C'
    #define STRING_TYPE 'S'
    #define FIXED_SIZE_STRING_TYPE 'F'

    // Commands code identifier
    #define WRITE_VEICULO_BINARY '1'
    #define WRITE_LINHA_BINARY '2'
    #define SHOW_VEICULO_CONTENT '3'
    #define SHOW_LINHA_CONTENT '4'
    #define SEARCH_VEICULO_CONTENT '5'
    #define SEARCH_LINHA_CONTENT '6'
    #define INSERT_VEICULO_CONTENT '7'
    #define INSERT_LINHA_CONTENT '8'

    void binarioNaTela(char *nomeArquivoBinario);
    
    void tranformToCsvFormat(char *inputString);

#endif 
