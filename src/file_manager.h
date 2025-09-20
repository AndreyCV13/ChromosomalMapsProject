#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "logic.h"

typedef struct {
    int amountOfEntries;
    char** names;
    char** probabilities;
} fileContent;

int saveFile(fileContent* content, const char* filename);
fileContent* loadFile(const char* filename);
void freeFileContent(fileContent* content);
int saveConfig(int totalGenes, const char *filename, GtkGrid *grid);

#endif