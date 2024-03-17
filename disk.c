#include <stdio.h>
#include "i-node.h"
#include "directory.h"
#include "stack.h"

#define TF 1000;

int currentDirectoryIndex = 100;
char currentPath[1024] = "/";

void initializeDisk(Block disk[])
{
  for (int i = 0; i < 1000; i++)
  {
    disk[i].type = 'F';
  }
}

void initializeRootDirectory(Block disk[])
{
  Directory root;
  root.TL = 0;
  Block newBlock;
  root = insertNewEntry(root, ".", 100);
  root = insertNewEntry(root, "..", 100);

  newBlock.directory = root;
  newBlock.type = 'D'; // diretório
  disk[100] = newBlock;
}

int findDirectory(Block disk[], char *directoryName)
{
  for (int i = 0; i < 1000; i++)
  {
    if (disk[i].type == 'D')
    {
      if (strcmp(disk[i].directory.name[0], directoryName) == 0)
      {
        return i;
      }
    }
  }
  return -1;
}

int findFileInDirectory(Directory directory, char *fileName)
{
  for (int i = 0; i < directory.TL; i++)
  {
    if (strcmp(directory.name[i], fileName) == 0)
    {
      return i;
    }
  }
  return -1;
}

void touch(Block disk[], char *fileName)
{
  int directoryIndex = currentDirectoryIndex; // Usa o diretório atual.
  if (directoryIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  if (findFileInDirectory(disk[directoryIndex].directory, fileName) != -1)
  {
    printf("Arquivo já existe.\n");
    return;
  }

  // Localiza um bloco livre para o arquivo.
  int freeBlockIndex = popFromStack(0); // sempre usa a primeira pilha.
  if (freeBlockIndex == -1)
  {
    printf("Não há espaço disponível.\n");
    return;
  }

  // Configura o bloco como um inode de arquivo.
  disk[freeBlockIndex].type = 'I';
  strcpy(disk[freeBlockIndex].principalInode.name, fileName);

  // Adiciona a entrada no diretório.
  disk[directoryIndex].directory = insertNewEntry(disk[directoryIndex].directory, fileName, freeBlockIndex);
}

void ls(Block disk[])
{
  int directoryIndex = currentDirectoryIndex;
  if (directoryIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  for (int i = 0; i < disk[directoryIndex].directory.TL; i++)
  {
    printf("%s\n", disk[directoryIndex].directory.name[i]);
  }
}

void mkdir(Block disk[], char *parentDirName, char *dirName)
{
  int parentDirIndex = findDirectory(disk, parentDirName);
  if (parentDirIndex == -1)
  {
    printf("Diretório pai não encontrado.\n");
    return;
  }

  if (findDirectory(disk, dirName) != -1)
  {
    printf("Diretório já existe.\n");
    return;
  }

  int freeBlockIndex = popFromStack(0);
  if (freeBlockIndex == -1)
  {
    printf("Não há espaço disponível.\n");
    return;
  }

  disk[freeBlockIndex].type = 'D';
  disk[freeBlockIndex].directory = insertNewEntry(disk[freeBlockIndex].directory, ".", freeBlockIndex);
  disk[freeBlockIndex].directory = insertNewEntry(disk[freeBlockIndex].directory, "..", parentDirIndex);

  disk[parentDirIndex].directory = insertNewEntry(disk[parentDirIndex].directory, dirName, freeBlockIndex);
}

void cd(Block disk[], char *dirName)
{
  if (strcmp(dirName, ".") == 0)
  {
    // Não faz nada, permanece no diretório atual.
  }
  else if (strcmp(dirName, "..") == 0)
  {
    // Volta ao diretório pai.
    if (strcmp(currentPath, "/") != 0)
    { // Verifica se não está na raiz.
      char *lastSlash = strrchr(currentPath, '/');
      if (lastSlash != NULL)
      {
        *lastSlash = '\0'; // Remove o último diretório do caminho.
        if (strlen(currentPath) == 0)
        {
          strcpy(currentPath, "/"); // Se vazio, volta para a raiz.
        }
      }
      // Atualiza o currentDirectoryIndex para o diretório pai.
      currentDirectoryIndex = disk[currentDirectoryIndex].directory.index[1];
    }
  }
  else
  {
    // Muda para um novo diretório.
    int dirIndex = findFileInDirectory(disk[currentDirectoryIndex].directory, dirName);
    if (dirIndex == -1)
    {
      printf("Diretório não encontrado.\n");
      return;
    }
    int newDirIndex = disk[currentDirectoryIndex].directory.index[dirIndex];
    if (disk[newDirIndex].type == 'D')
    {
      // Atualiza o caminho.
      if (strcmp(currentPath, "/") != 0)
        strcat(currentPath, "/");
      strcat(currentPath, dirName);
      // Atualiza o currentDirectoryIndex.
      currentDirectoryIndex = newDirIndex;
    }
    else
    {
      printf("%s não é um diretório.\n", dirName);
    }
  }
}

void clearScreen()
{
  printf("\033[2J\033[H");
}

int main()
{
  Block disk[1000];
  char command[256];
  char arg1[256];
  char arg2[256];

  initializeDisk(disk);
  initializeFreeBlockStacks();
  fillFreeBlockStacks();
  printf("Disk initialized\n");
  initializeRootDirectory(disk);

  printf("Bem-vindo ao Simulador de Sistema de Arquivos. Digite 'help' para comandos.\n");

  while (1)
  {
    printf("\n%s\n > ", currentPath);
    fgets(command, 256, stdin);

    // Remove a newline do final da string de comando
    command[strcspn(command, "\n")] = 0;

    if (strcmp(command, "exit") == 0)
    {
      printf("Saindo...\n");
      break;
    }
    else if (strcmp(command, "ls") == 0)
    {
      ls(disk);
    }
    else if (sscanf(command, "touch %s", arg1) == 1)
    {
      touch(disk, arg1); // Modificado para não passar o diretório.
    }
    else if (strcmp(command, "help") == 0)
    {
      printf("Comandos disponíveis: exit, ls, touch <filename>, mkdir <directory>, cd <directory>, clear\n");
    }
    else if (sscanf(command, "mkdir %s", arg1) == 1)
    {
      mkdir(disk, ".", arg1); // apenas do caminho atual por enquanto
    }
    else if (sscanf(command, "cd %s", arg1) == 1)
    {
      cd(disk, arg1);
    }
    else if (strcmp(command, "clear") == 0)
    {
      clearScreen();
    }
    else
    {
      printf("Comando não reconhecido.\n");
    }
  }

  return 0;
}