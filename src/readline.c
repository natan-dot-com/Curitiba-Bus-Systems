#include "readline.h"

#define READLINE_BUFFER 1024

// Reads a string dynamically until '\n' or end of file
// Return value: Read string from stream (char *)
char *readline(FILE *stream) {
    char ch;
    do {
        ch = fgetc(stream);
    } while (ch == '\n');

    char *string = NULL;
    int pos = 0;
    do {
        if (pos != 0) 
            ch = fgetc(stream);
        if (ch != '\r' && ch != '\n') {
            if (pos % READLINE_BUFFER == 0) {
                string = (char *) realloc(string, 1+(pos/READLINE_BUFFER + 1)*READLINE_BUFFER);
            }
            string[pos++] = ch;
        }
    } while (ch != '\n' && ch != EOF);
    if (pos == 0)
        return NULL;
    string[pos] = '\0';
    string = (char *) realloc(string, pos+1);
    return string;
}

