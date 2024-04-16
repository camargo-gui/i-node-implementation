#pragma once
#include <stdio.h>
#include <string.h>

#define nameLength 32

struct directory
{
    char name[10][9];
    int index[10];
    int TL;
};

typedef struct directory Directory;

Directory insertNewEntry(Directory directory, char name[], int index)
{
    if (strlen(name) > nameLength)
    {
        printf("\nTamanho maior que o suportado!!!\n");
    }
    else if (directory.TL == 10)
    {
        printf("\nDiretorio cheio!!!\n");
    }
    else
    {
        directory.index[directory.TL] = index;
        strcpy(directory.name[directory.TL], name);
        directory.TL++;
    }
    return directory;
}
