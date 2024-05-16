#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

unsigned char* fill_data(char c, int len) {
    unsigned char* str = malloc(len + 1);
    memset(str, c, len);
    str[len] = '\0';
    return str;
}

bool prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

/*
char** string_split(char* string_to_split, char* delimiter) {
    // from https://stackoverflow.com/a/9210560
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = string_to_split;
    char* last_comma = 0;
    char delim[2];
    delim[0] = *delimiter;
    delim[1] = 0;

    // Count how many elements will be extracted. 
    while (*tmp) {
        if (*delimiter == *tmp){
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    // Add space for trailing token. 
    count += last_comma < (string_to_split + strlen(string_to_split) - 1);

    // Add space for terminating null string so caller
    //   knows where the list of returned strings ends. 
    count++;

    result = malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(string_to_split, delim);

        while (token){
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        *(result + idx) = 0;
        free(token);
        token = NULL;
    }

    return result;
}
*/
