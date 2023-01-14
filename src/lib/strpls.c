#include "strpls.h"

void strswp(char* first, char* second) {
    char temp[strlen(first)];
    strcpy(temp, first);
    strcpy(first, second);
    strcpy(second, temp);
}

void removeAllAfter(char* destination, const char searched) {
    char* parent = strrchr(destination, searched);
    if (parent != NULL) {
        *parent = '\0';
    }
}

void removeAllOccurences(char* destination, const char searched) {
    int writer = 0;
    int reader = 0;

    while (destination[reader]) {
        if (destination[reader] != searched) {
            destination[writer++] = destination[reader];
        }
        reader++;
    }
    destination[writer] = '\0';
}
