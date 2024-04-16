#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "i-node.h"
#include "directory.h"
#include "stack.h"

#define NUM_BLOCOS 1000
#define MAX_DIRECT_POINTERS 8
#define MAX_INDIRECT_POINTERS 5

int currentDirectoryIndex = 100;
char currentPath[1024] = "/";

void printPrompt()
{
  printf("\033[0;32muser@group:\033[0m\033[0;34m%s\033[0m $ ", currentPath);
}

void initializeDisk(Block disk[])
{
  for (int i = 0; i < 1000; i++)
  {
    strcpy(disk[i].type, "F");
  }
}

void printBlockStatus(Block disk[])
{
  int freeBlocks = 0;
  int badBlocks = 0;
  int inodeBlocks = 0;
  int dataBlocks = 0;
  int dirBlocks = 0;

  for (int i = 0; i < NUM_BLOCOS; i++)
  {
    if (strcmp(disk[i].type, "F") == 0)
    {
      freeBlocks++;
    }
    else if (strcmp(disk[i].type, "B") == 0)
    {
      badBlocks++;
    }
    else if (strcmp(disk[i].type, "I") == 0)
    {
      inodeBlocks++;
    }
    else if (strcmp(disk[i].type, "D") == 0)
    {
      dataBlocks++;
    }
    else if (strcmp(disk[i].type, "DIR") == 0)
    {
      dirBlocks++;
    }
  }

  printf("Blocos livres: %d\n", freeBlocks);
  printf("Blocos ruins: %d\n", badBlocks);
  printf("Blocos de inode: %d\n", inodeBlocks);
  printf("Blocos de dados: %d\n", dataBlocks);
  printf("Blocos de diretório: %d\n", dirBlocks);
}

void df(Block disk[], int blockSize)
{
  int freeBlocks = 0, inodeBlocks = 0, dataBlocks = 0, dirBlocks = 0;

  // Conta o número de cada tipo de bloco
  for (int i = 0; i < NUM_BLOCOS; i++)
  {
    if (strcmp(disk[i].type, "F") == 0)
      freeBlocks++;
    else if (strcmp(disk[i].type, "I") == 0)
      inodeBlocks++;
    else if (strcmp(disk[i].type, "D") == 0)
      dataBlocks++;
    else if (strcmp(disk[i].type, "DIR") == 0)
      dirBlocks++;
  }

  int totalBlocks = inodeBlocks + dataBlocks + dirBlocks;
  int freeBytes = freeBlocks * blockSize;
  int usedBytes = totalBlocks * blockSize;

  // Exibe um relatório detalhado do uso do espaço no disco em bytes
  printf("\nUso do Disco em Bytes:\n");
  printf("Espaço livre: %d bytes\n", freeBytes);
  printf("Espaço ocupado: %d bytes\n", usedBytes);
}

void initializeIndirectInode(Block disk[], int blockIndex)
{
  if (blockIndex != -1)
  {
    strcpy(disk[blockIndex].type, "I");
    for (int i = 0; i < MAX_INDIRECT_POINTERS; i++)
    {
      disk[blockIndex].indirectInode.pointer[i] = -1;
    }
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
  strcpy(newBlock.type, "DIR");
  disk[100] = newBlock;
}

int findDirectory(Block disk[], char *directoryName)
{
  for (int i = 0; i < 1000; i++)
  {
    if (strcmp(disk[i].type, "DIR") == 0)
    {
      if (strcmp(disk[i].directory.name[0], directoryName) == 0)
      {
        return i;
      }
    }
  }
  return -1;
}

int getRandomFreeBlock()
{
  int attempts = 0;
  while (attempts < MAX_STACKS)
  {
    int stackIndex = rand() % MAX_STACKS;
    if (stackHeads[stackIndex] > -1)
    {
      int blockIndex = freeBlockStacks[stackIndex][stackHeads[stackIndex]--];
      return blockIndex;
    }
    attempts++;
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

void ls(Block disk[])
{
  int directoryIndex = currentDirectoryIndex;
  if (directoryIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  // Itera sobre todas as entradas no diretório
  for (int i = 0; i < disk[directoryIndex].directory.TL; i++)
  {
    // Pula as entradas "." e ".."
    if (strcmp(disk[directoryIndex].directory.name[i], ".") != 0 &&
        strcmp(disk[directoryIndex].directory.name[i], "..") != 0)
    {
      printf("%s\n", disk[directoryIndex].directory.name[i]);
    }
  }
}

void ls_l(Block disk[])
{
  int directoryIndex = currentDirectoryIndex;
  if (directoryIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  for (int i = 0; i < disk[directoryIndex].directory.TL; i++)
  {
    int inodeIndex = disk[directoryIndex].directory.index[i];
    PrincipalInode inode = disk[inodeIndex].principalInode;

    // Pula as entradas "." e ".."
    if (strcmp(disk[directoryIndex].directory.name[i], ".") == 0 ||
        strcmp(disk[directoryIndex].directory.name[i], "..") == 0)
    {
      continue; // Ignora a impressão de "." e ".."
    }

    char typePrefix = (inode.permissions[0] == 'd') ? 'd' : '-';

    if (inode.date != NULL)
    {
      printf("%c%s -> %d | links | user: %s | group: %s | size: %db | criado em: %02d/%02d | permissoes: %s\n",
             typePrefix, disk[directoryIndex].directory.name[i], inode.countLinks, inode.userName, inode.groupName,
             inode.size, inode.date->tm_mday, inode.date->tm_mon + 1, inode.permissions);
    }
    else
    {
      printf("%c%s -> %d | links | user: %s | group: %s | size: %db | permissoes: %s\n",
             typePrefix, disk[directoryIndex].directory.name[i], inode.countLinks, inode.userName, inode.groupName, inode.size, inode.permissions);
    }
  }
}

void mkdir(Block disk[], int parentDirIndex, char *dirName, int blockSize)
{
  if (parentDirIndex == -1)
  {
    printf("Diretório pai não encontrado.\n");
    return;
  }

  if (findFileInDirectory(disk[parentDirIndex].directory, dirName) != -1)
  {
    printf("Diretório já existe.\n");
    return;
  }

  int dirInodeIndex = getRandomFreeBlock(); // Inode do novo diretório
  if (dirInodeIndex == -1)
  {
    printf("Não há espaço disponível para o inode do diretório.\n");
    return;
  }

  // Configura o inode do diretório
  strcpy(disk[dirInodeIndex].type, "I");
  PrincipalInode *dirInode = &disk[dirInodeIndex].principalInode;
  strcpy(dirInode->name, dirName);
  dirInode->size = blockSize;
  dirInode->countLinks = 1;
  strcpy(dirInode->permissions, "drwxr-xr-x");
  strcpy(dirInode->userName, "user");
  strcpy(dirInode->groupName, "group");
  dirInode->indirect = -1; // Não há blocos indiretos inicialmente

  time_t now = time(NULL);          // Pega a hora e data atual
  dirInode->date = localtime(&now); // Armazena a data e hora no inode

  int dirBlockIndex = getRandomFreeBlock(); // Bloco para as entradas de diretório
  if (dirBlockIndex == -1)
  {
    printf("Não há espaço disponível para o bloco de diretório.\n");
    return;
  }

  // Configura o bloco de diretório
  strcpy(disk[dirBlockIndex].type, "DIR");
  disk[dirBlockIndex].directory = insertNewEntry(disk[dirBlockIndex].directory, ".", dirInodeIndex);
  disk[dirBlockIndex].directory = insertNewEntry(disk[dirBlockIndex].directory, "..", parentDirIndex);
  disk[parentDirIndex].directory = insertNewEntry(disk[parentDirIndex].directory, dirName, dirInodeIndex);
  dirInode->pointer[0] = dirBlockIndex; // O primeiro bloco de diretório é apontado pelo inode
}

void mkdirFromPath(Block disk[], char *path, char *dirName, int blockSize)
{
  char buffer[1024];
  strcpy(buffer, path); // Copia o caminho para um buffer manipulável
  char *token;
  char *rest = buffer;

  int dirIndex = currentDirectoryIndex; // Começa do diretório atual
  if (path[0] == '/')
  {
    // Caminho absoluto
    dirIndex = 100;             // Supondo que o índice 100 é a raiz
    strtok_r(rest, "/", &rest); // Ignora o primeiro token se o caminho começar com '/'
  }

  // Navega pelos componentes do caminho
  while ((token = strtok_r(NULL, "/", &rest)))
  {
    if (strcmp(token, ".") == 0)
    {
      continue;
    }
    else if (strcmp(token, "..") == 0)
    {
      dirIndex = disk[dirIndex].directory.index[1];
    }
    else
    {
      int foundIndex = findFileInDirectory(disk[dirIndex].directory, token);
      if (foundIndex == -1 || strcmp(disk[disk[dirIndex].directory.index[foundIndex]].type, "DIR") != 0)
      {
        printf("Caminho inválido.\n");
        return;
      }
      dirIndex = disk[dirIndex].directory.index[foundIndex];
    }
  }

  mkdir(disk, dirIndex, dirName, blockSize);
}

void cd(Block disk[], char *dirName)
{
  if (strcmp(dirName, ".") == 0)
  {
    return;
  }
  else if (strcmp(dirName, "..") == 0)
  {
    // Muda para o diretório pai.
    currentDirectoryIndex = disk[currentDirectoryIndex].directory.index[1]; // Índice de '..'
    if (strcmp(currentPath, "/") != 0)
    {
      // Encontra o último '/' e termina a string ali para remover o último diretório do caminho.
      char *lastSlash = strrchr(currentPath, '/');
      if (lastSlash != NULL)
      {
        *lastSlash = '\0'; // Remove o último diretório do caminho.
        if (strlen(currentPath) == 0)
        {
          strcpy(currentPath, "/"); // Se vazio, volta para a raiz.
        }
      }
    }
    return;
  }

  int dirIndex = findFileInDirectory(disk[currentDirectoryIndex].directory, dirName);
  if (dirIndex == -1)
  {
    printf("cd %s: No such file or directory\n", dirName);
    return;
  }

  int inodeIndex = disk[currentDirectoryIndex].directory.index[dirIndex];
  if (strcmp(disk[inodeIndex].type, "I") == 0 && disk[inodeIndex].principalInode.pointer[0] != -1)
  {
    currentDirectoryIndex = disk[inodeIndex].principalInode.pointer[0];
    // Atualiza o caminho atual ao entrar no novo diretório
    if (strcmp(currentPath, "/") != 0)
    {
      strcat(currentPath, "/"); // Adiciona o separador de diretório
    }
    strcat(currentPath, dirName); // Adiciona o nome do novo diretório ao caminho
  }
  else
  {
    printf("%s não é um diretório.\n", dirName);
  }
}

void clearScreen()
{

  printf("\033[2J\033[H");
}

void deleteFile(Block disk[], char *fileName)
{
  int dirIndex = currentDirectoryIndex;
  if (dirIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  int fileIndex = findFileInDirectory(disk[dirIndex].directory, fileName);
  if (fileIndex == -1)
  {
    printf("rm %s: No such file or directory\n", fileName);
    return;
  }

  int inodeIndex = disk[dirIndex].directory.index[fileIndex];
  if (strcmp(disk[inodeIndex].type, "I") != 0)
  {
    printf("O item especificado não é um arquivo.\n");
    return;
  }

  // Decrementa o contador de links
  if (--disk[inodeIndex].principalInode.countLinks == 0)
  {
    // Libera blocos diretos se não houver mais links
    for (int i = 0; i < MAX_DIRECT_POINTERS && disk[inodeIndex].principalInode.pointer[i] != -1; i++)
    {
      disk[disk[inodeIndex].principalInode.pointer[i]].type[0] = 'F'; // Marca o bloco como livre
      disk[disk[inodeIndex].principalInode.pointer[i]].type[1] = '\0';
      pushToStack(disk[inodeIndex].principalInode.pointer[i]);
    }

    // Libera blocos indiretos, se houver
    if (disk[inodeIndex].principalInode.indirect != -1)
    {
      IndirectInode *indirect = &disk[disk[inodeIndex].principalInode.indirect].indirectInode;
      for (int i = 0; i < MAX_INDIRECT_POINTERS && indirect->pointer[i] != -1; i++)
      {
        disk[indirect->pointer[i]].type[0] = 'F';
        disk[indirect->pointer[i]].type[1] = '\0';
        pushToStack(indirect->pointer[i]);
      }
      disk[disk[inodeIndex].principalInode.indirect].type[0] = 'F';
      disk[disk[inodeIndex].principalInode.indirect].type[1] = '\0';
      pushToStack(disk[inodeIndex].principalInode.indirect);
    }

    // Limpa o inode principal completamente se não houver mais links
    clearInode(disk, inodeIndex);
  }

  // Remove a entrada do diretório sempre
  removeEntryFromDirectory(disk, dirIndex, fileIndex);
  printf("Arquivo '%s' removido com sucesso.\n", fileName);
}

void deleteDirectory(Block disk[], char *dirName)
{
  int dirIndex = currentDirectoryIndex;
  int directoryIndex = findFileInDirectory(disk[dirIndex].directory, dirName);

  if (directoryIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  int inodeIndex = disk[dirIndex].directory.index[directoryIndex];
  if (strcmp(disk[inodeIndex].type, "I") != 0 || disk[disk[inodeIndex].principalInode.pointer[0]].directory.TL > 2)
  {
    printf("O diretório não está vazio ou o item especificado não é um diretório.\n");
    return;
  }

  // Remove o bloco de diretório
  int dirBlockIndex = disk[inodeIndex].principalInode.pointer[0];
  strcpy(disk[dirBlockIndex].type, "F");
  memset(&disk[dirBlockIndex].directory, 0, sizeof(Directory));

  // Remove o inode do diretório
  strcpy(disk[inodeIndex].type, "F");
  memset(&disk[inodeIndex].principalInode, 0, sizeof(PrincipalInode));

  // Atualiza o diretório pai
  for (int i = directoryIndex; i < disk[dirIndex].directory.TL - 1; i++)
  {
    disk[dirIndex].directory.index[i] = disk[dirIndex].directory.index[i + 1];
    strcpy(disk[dirIndex].directory.name[i], disk[dirIndex].directory.name[i + 1]);
  }
  disk[dirIndex].directory.TL--;
}

void Bad(Block disk[], int blockIndex)
{
  strcpy(disk[blockIndex].type, "B");
}

void chmod(Block disk[], char *fileName, char *command)
{
  int directoryIndex = currentDirectoryIndex;
  if (directoryIndex == -1)
  {
    printf("chmod %s %s: No such file or directory\n", fileName, command);
    return;
  }

  int fileIndex = findFileInDirectory(disk[directoryIndex].directory, fileName);
  if (fileIndex == -1)
  {
    printf("chmod %s %s: No such file or directory\n", fileName, command);
    return;
  }

  int blockIndex = disk[directoryIndex].directory.index[fileIndex];
  if (strcmp(disk[blockIndex].type, "I") != 0)
  {
    printf("%s não é um arquivo.\n", fileName);
    return;
  }

  char command_type;
  char who;
  char permissions[4];
  if (sscanf(command, "%c%c%s", &who, &command_type, permissions) != 3)
  {
    printf("Comando de permissão inválido.\n");
    return;
  }

  PrincipalInode inode = disk[blockIndex].principalInode;
  int perm_base = (who == 'u' ? 0 : who == 'g' ? 3
                                               : 6);

  for (int i = 0; i < (int)strlen(permissions); i++)
  {
    int perm_index = perm_base + (permissions[i] == 'r' ? 0 : permissions[i] == 'w' ? 1
                                                                                    : 2);
    if (command_type == '+')
    {
      // Só adiciona se ainda não tem a permissão
      if (inode.permissions[perm_index] != permissions[i])
      {
        inode.permissions[perm_index] = permissions[i];
      }
    }
    else if (command_type == '-')
    {
      // Só remove se a permissão existir
      if (inode.permissions[perm_index] != '-')
      {
        inode.permissions[perm_index] = '-';
      }
    }
  }

  disk[blockIndex].principalInode = inode;
  printf("Permissões do arquivo %s alteradas com sucesso.\n", fileName);
}

void touch(Block disk[], char *fileName, int fileSizeInBytes, int blockSize)
{
  if (fileSizeInBytes <= 0)
  {
    printf("Tamanho do arquivo inválido.\n");
    return;
  }

  int numBlocks = (fileSizeInBytes + blockSize - 1) / blockSize;
  int directoryIndex = currentDirectoryIndex;
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

  int inodeIndex = getRandomFreeBlock();
  if (inodeIndex == -1)
  {
    printf("Não há espaço para um novo inode.\n");
    return;
  }

  strcpy(disk[inodeIndex].type, "I");
  PrincipalInode inode = disk[inodeIndex].principalInode;
  strcpy(inode.name, fileName);
  inode.size = fileSizeInBytes;
  inode.countLinks = 1;
  strcpy(inode.permissions, "rw-r--r--"); // Default permissions
  strcpy(inode.userName, "user");
  strcpy(inode.groupName, "group");
  inode.indirect = -1;

  time_t now = time(0);
  inode.date = localtime(&now);

  int allocatedBlocks = 0;
  for (int i = 0; i < numBlocks && i < MAX_DIRECT_POINTERS; i++)
  {
    int blockIndex = getRandomFreeBlock();
    if (blockIndex == -1)
    {
      printf("Espaço insuficiente para alocar blocos diretos.\n");
      return;
    }
    inode.pointer[i] = blockIndex;
    strcpy(disk[blockIndex].type, "D");
    allocatedBlocks++;
  }

  if (numBlocks > MAX_DIRECT_POINTERS)
  {
    int indirectIndex = getRandomFreeBlock();
    if (indirectIndex == -1)
    {
      printf("Espaço insuficiente para inode indireto.\n");
      return;
    }
    initializeIndirectInode(disk, indirectIndex);
    inode.indirect = indirectIndex;

    for (int i = 0; i < numBlocks - MAX_DIRECT_POINTERS && i < MAX_INDIRECT_POINTERS; i++)
    {
      int blockIndex = getRandomFreeBlock();
      if (blockIndex == -1)
      {
        printf("Espaço insuficiente para alocar blocos indiretos.\n");
        return;
      }
      disk[indirectIndex].indirectInode.pointer[i] = blockIndex;
      strcpy(disk[blockIndex].type, "D");
      allocatedBlocks++;
    }
  }

  disk[directoryIndex].directory = insertNewEntry(disk[directoryIndex].directory, fileName, inodeIndex);
  disk[inodeIndex].principalInode = inode; // Update inode back to disk array after modifications
}

void linkFile(Block disk[], char *sourceFileName, char *destFileName, char type)
{
  int directoryIndex = currentDirectoryIndex;
  int sourceFileIndex = findFileInDirectory(disk[directoryIndex].directory, sourceFileName);
  if (sourceFileIndex == -1)
  {
    printf("Arquivo fonte não encontrado.\n");
    return;
  }

  int sourceInodeIndex = disk[directoryIndex].directory.index[sourceFileIndex];
  if (type == 'h')
  {
    // Link físico
    if (disk[sourceInodeIndex].principalInode.countLinks >= 255)
    {
      printf("Número máximo de links alcançado.\n");
      return;
    }
    disk[sourceInodeIndex].principalInode.countLinks++;
    disk[directoryIndex].directory = insertNewEntry(disk[directoryIndex].directory, destFileName, sourceInodeIndex);
  }
  else if (type == 's')
  {
    // Link simbólico
    int symlinkInodeIndex = getRandomFreeBlock();
    if (symlinkInodeIndex == -1)
    {
      printf("Não há espaço para um novo inode.\n");
      return;
    }
    strcpy(disk[symlinkInodeIndex].type, "L"); // 'L' para link simbólico
    strcpy(disk[symlinkInodeIndex].principalInode.name, destFileName);
    strcpy(disk[symlinkInodeIndex].principalInode.symlinkPath, sourceFileName);
    disk[symlinkInodeIndex].principalInode.size = strlen(sourceFileName);
    strcpy(disk[symlinkInodeIndex].principalInode.permissions, "lrwxrwxrwx");
    strcpy(disk[symlinkInodeIndex].principalInode.userName, "user");
    strcpy(disk[symlinkInodeIndex].principalInode.groupName, "group");
    time_t now = time(0);
    disk[symlinkInodeIndex].principalInode.date = localtime(&now);
    disk[symlinkInodeIndex].principalInode.countLinks = 1;

    disk[directoryIndex].directory = insertNewEntry(disk[directoryIndex].directory, destFileName, symlinkInodeIndex);
  }
}

void unlinkFile(Block disk[], char *fileName, char type)
{
  int directoryIndex = currentDirectoryIndex;
  if (directoryIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  int fileIndex = findFileInDirectory(disk[directoryIndex].directory, fileName);
  if (fileIndex == -1)
  {
    printf("Arquivo '%s' não encontrado.\n", fileName);
    return;
  }

  int inodeIndex = disk[directoryIndex].directory.index[fileIndex];
  if (type == 'h')
  { // Remover link físico
    if (--disk[inodeIndex].principalInode.countLinks == 0)
    {
      // Remove o inode completamente se não há mais links
      clearInode(disk, inodeIndex);
    }
  }
  else if (type == 's')
  {                               // Remover link simbólico
    clearInode(disk, inodeIndex); // Limpa o inode do link simbólico
  }

  // Remove a entrada do diretório
  removeEntryFromDirectory(disk, directoryIndex, fileIndex);
  printf("Link '%s' removido com sucesso.\n", fileName);
}

void removeEntryFromDirectory(Block disk[], int directoryIndex, int entryIndex)
{
  if (entryIndex < 0 || entryIndex >= disk[directoryIndex].directory.TL)
  {
    printf("Índice de entrada inválido.\n");
    return;
  }

  // Mover todas as entradas após o índice para uma posição anterior para preencher o espaço
  for (int i = entryIndex; i < disk[directoryIndex].directory.TL - 1; i++)
  {
    strcpy(disk[directoryIndex].directory.name[i], disk[directoryIndex].directory.name[i + 1]);
    disk[directoryIndex].directory.index[i] = disk[directoryIndex].directory.index[i + 1];
  }

  // Decrementar o total de entradas no diretório
  disk[directoryIndex].directory.TL--;
}

void clearInode(Block disk[], int inodeIndex)
{
  disk[inodeIndex].principalInode.countLinks = 0;
  disk[inodeIndex].principalInode.size = 0;
  disk[inodeIndex].principalInode.indirect = -1;
  disk[inodeIndex].principalInode.date = NULL;

  for (int i = 0; i < MAX_DIRECT_POINTERS; i++)
  {
    disk[inodeIndex].principalInode.pointer[i] = -1;
  }

  strcpy(disk[inodeIndex].principalInode.permissions, "---------");
  strcpy(disk[inodeIndex].principalInode.userName, "");
  strcpy(disk[inodeIndex].principalInode.groupName, "");
  strcpy(disk[inodeIndex].principalInode.name, "");
  disk[inodeIndex].principalInode.symlinkPath[0] = '\0';
}

int main()
{
  int blockSize;
  printf("Digite o tamanho em bytes de cada bloco do disco: ");
  scanf("%d", &blockSize);

  while (getchar() != '\n')
    ;

  int fileSizeInBytes;
  Block disk[NUM_BLOCOS];
  char command[256];
  char arg1[256];
  char arg2[256];

  initializeDisk(disk);
  initializeFreeBlockStacks();
  fillFreeBlockStacks();
  initializeRootDirectory(disk);

  printf("Disk initialized\n");
  printf("Bem-vindo ao Simulador de Sistema de Arquivos. Digite 'help' para comandos.\n");

  while (1)
  {
    printPrompt();
    fgets(command, 256, stdin);

    command[strcspn(command, "\n")] = 0;

    if (strcmp(command, "exit") == 0)
    {
      printf("Saindo...\n");
      break;
    }
    else if (strcmp(command, "ls -l") == 0)
    {
      ls_l(disk);
    }
    else if (strcmp(command, "ls") == 0)
    {
      ls(disk);
    }
    else if (sscanf(command, "touch %s %d", arg1, &fileSizeInBytes) == 2)
    {
      touch(disk, arg1, fileSizeInBytes, blockSize);
    }
    else if (sscanf(command, "mkdir %s", arg1) == 1)
    {
      mkdir(disk, currentDirectoryIndex, arg1, blockSize);
    }
    else if (sscanf(command, "mkdir %s %s", arg1, arg2) == 2)
    {
      mkdirFromPath(disk, arg1, arg2, blockSize);
    }
    else if (sscanf(command, "cd %s", arg1) == 1)
    {
      cd(disk, arg1);
    }
    else if (strcmp(command, "clear") == 0)
    {
      clearScreen();
    }
    else if (sscanf(command, "rmdir %s", arg1) == 1)
    {
      deleteDirectory(disk, arg1);
    }
    else if (sscanf(command, "rm %s", arg1) == 1)
    {
      deleteFile(disk, arg1);
    }
    else if (sscanf(command, "chmod %s %s", arg1, arg2) == 2)
    {
      chmod(disk, arg1, arg2);
    }
    else if (sscanf(command, "bad %s", arg1) == 1)
    {
      int blockIndex = atoi(arg1);
      if (blockIndex < 0 || blockIndex >= NUM_BLOCOS)
      {
        printf("Bloco inválido.\n");
      }
      else
      {
        Bad(disk, blockIndex);
      }
    }
    else if (strcmp(command, "df") == 0)
    {
      df(disk, blockSize);
    }
    else if (sscanf(command, "link -h %s %s", arg1, arg2) == 2)
    {
      linkFile(disk, arg1, arg2, 'h');
    }
    else if (sscanf(command, "link -s %s %s", arg1, arg2) == 2)
    {
      linkFile(disk, arg1, arg2, 's');
    }
    else if (sscanf(command, "unlink -h %s", arg1) == 1)
    {
      unlinkFile(disk, arg1, 'h');
    }
    else if (sscanf(command, "unlink -s %s", arg1) == 1)
    {
      unlinkFile(disk, arg1, 's');
    }
    else if (strcmp(command, "help") == 0)
    {
      printf("Comandos disponíveis: exit, ls, ls -l, touch <filename>, mkdir <directory>, cd <directory>, rm <filename>, rmdir <directory>, df, bad <block>, chmod <filename> <permission> clear\n");
    }
    else
    {
      printf("%s: command not found\n", command);
    }
  }

  return 0;
}