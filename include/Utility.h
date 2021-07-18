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

    #define NULL_FIELD "campo com valor nulo"

    // Registry status constants
    #define REMOVED_REGISTRY '0'
    #define VALID_REGISTRY '1'

    // Consistent/inconsistent file signature (for fileStatus in header)
    #define INCONSISTENT_FILE "0"
    #define CONSISTENT_FILE '1'

    // Type identifier for read field (used in functionalities 5 and 6)
    #define INTEGER_TYPE 'I'
    #define CHAR_TYPE 'C'
    #define STRING_TYPE 'S'
    #define FIXED_SIZE_STRING_TYPE 'F'

    // Registry extra size considering 'isRemoved' and 'regSize' fields
    #define REG_EXTRA_SIZE 5

    // Commands code identifier   
    enum InputCommands {
        WRITE_VEICULO_BINARY = 1,  
        WRITE_LINHA_BINARY,  
        SHOW_VEICULO_CONTENT,  
        SHOW_LINHA_CONTENT,  
        SEARCH_VEICULO_CONTENT,  
        SEARCH_LINHA_CONTENT,  
        INSERT_VEICULO_CONTENT,  
        INSERT_LINHA_CONTENT,  
        CREATE_VEICULO_BTREE,
        CREATE_LINHA_BTREE,
        SEARCH_VEICULO_PREFIX,
        SEARCH_LINHA_PREFIX,
        INSERT_VEICULO_BTREE,
        INSERT_LINHA_BTREE,
        MERGE_VEICULO_LINHA,
        MERGE_BTREE_VEICULO_LINHA,
        SORT_LINHA_FILE,
        SORT_VEICULO_FILE,
        MERGE_LINHA_VEICULO,
    };

    void binarioNaTela(char *nomeArquivoBinario);
    void tranformToCsvFormat(char *inputString);
    int convertePrefixo(char* str);

#endif 
