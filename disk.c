#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "i-node.h"
#include "directory.h"
#include "stack.h"

#define NUM_BLOCOS 1000
#define MAX_DIRECT_POINTERS 8
#define MAX_INDIRECT_POINTERS 5

int currentDirectoryIndex = 100;
char currentPath[1024] = "/";

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

void touch(Block disk[], char *fileName, int fileSizeInBytes, int blockSize)
{
  if (fileSizeInBytes <= 0)
  {
    printf("Tamanho do arquivo inválido.\n");
    return;
  }

  int numBlocks = (fileSizeInBytes + blockSize - 1) / blockSize; // Arrendondamento para cima
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

  // Encontrar um bloco livre para o inode
  int inodeIndex = getRandomFreeBlock();
  if (inodeIndex == -1)
  {
    printf("Não há espaço para um novo inode.\n");
    return;
  }

  // Inicializando o inode principal
  strcpy(disk[inodeIndex].type, "I"); // Definindo como inode
  PrincipalInode *inode = &disk[inodeIndex].principalInode;
  strcpy(inode->name, fileName);
  inode->size = fileSizeInBytes;
  inode->countLinks = 1;
  strcpy(inode->permissions, "rw-r--r--");
  strcpy(inode->userName, "user");
  strcpy(inode->groupName, "group");
  inode->indirect = -1;

  // Alocando blocos diretos se necessário
  int allocatedBlocks = 0;
  for (int i = 0; i < numBlocks && i < MAX_DIRECT_POINTERS; i++)
  {
    int blockIndex = getRandomFreeBlock();
    if (blockIndex == -1)
    {
      printf("Espaço insuficiente para alocar blocos diretos.\n");
      return;
    }
    inode->pointer[i] = blockIndex;
    strcpy(disk[blockIndex].type, "D"); // Definindo como bloco de dados
    allocatedBlocks++;
  }

  // Alocando blocos indiretos se necessário
  if (numBlocks > MAX_DIRECT_POINTERS)
  {
    int indirectIndex = getRandomFreeBlock();
    if (indirectIndex == -1)
    {
      printf("Espaço insuficiente para inode indireto.\n");
      return;
    }
    initializeIndirectInode(disk, indirectIndex);
    inode->indirect = indirectIndex;

    for (int i = 0; i < numBlocks - MAX_DIRECT_POINTERS && i < MAX_INDIRECT_POINTERS; i++)
    {
      int blockIndex = getRandomFreeBlock();
      if (blockIndex == -1)
      {
        printf("Espaço insuficiente para alocar blocos indiretos.\n");

        return;
      }
      disk[indirectIndex].indirectInode.pointer[i] = blockIndex;
      strcpy(disk[blockIndex].type, "D"); // Definição como bloco de dados
      allocatedBlocks++;
    }
  }

  disk[directoryIndex].directory = insertNewEntry(disk[directoryIndex].directory, fileName, inodeIndex);
  printf("Arquivo '%s' criado com sucesso com %d blocos.\n", fileName, allocatedBlocks);
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

void mkdir(Block disk[], int parentDirIndex, char *dirName)
{
  if (parentDirIndex == -1)
  {
    printf("Diretório pai não encontrado.\n");
    return;
  }

  // Correção: Verificar se o diretório já existe no diretório pai
  if (findFileInDirectory(disk[parentDirIndex].directory, dirName) != -1)
  {
    printf("Diretório já existe.\n");
    return;
  }

  int freeBlockIndex = getRandomFreeBlock();
  if (freeBlockIndex == -1)
  {
    printf("Não há espaço disponível.\n");
    return;
  }

  // Configura o bloco como um diretório
  strcpy(disk[freeBlockIndex].type, "DIR");
  disk[freeBlockIndex].directory = insertNewEntry(disk[freeBlockIndex].directory, ".", freeBlockIndex);  // link para si mesmo
  disk[freeBlockIndex].directory = insertNewEntry(disk[freeBlockIndex].directory, "..", parentDirIndex); // link para o diretório pai

  // Adiciona o novo diretório ao diretório pai
  disk[parentDirIndex].directory = insertNewEntry(disk[parentDirIndex].directory, dirName, freeBlockIndex);
}

void mkdirFromPath(Block disk[], char *path, char *dirName)
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

  mkdir(disk, dirIndex, dirName);
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
    if (strcmp(disk[newDirIndex].type, "DIR") == 0)
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

void deleteFile(Block disk[], char *fileName)
{
  int dirIndex = currentDirectoryIndex; // Usa o diretório atual.
  if (dirIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  int fileIndex = findFileInDirectory(disk[dirIndex].directory, fileName);
  if (fileIndex == -1)
  {
    printf("Arquivo não encontrado.\n");
    return;
  }

  int inodeIndex = disk[dirIndex].directory.index[fileIndex];
  if (strcmp(disk[inodeIndex].type, "I") != 0)
  {
    printf("O item especificado não é um arquivo.\n");
    return;
  }

  // Obtém o inode do arquivo
  PrincipalInode *inode = &disk[inodeIndex].principalInode;

  // Libera blocos diretos
  for (int i = 0; i < MAX_DIRECT_POINTERS && inode->pointer[i] != -1; i++)
  {
    disk[inode->pointer[i]].type[0] = 'F';  // Marca o bloco como livre
    disk[inode->pointer[i]].type[1] = '\0'; // Garante que o tipo é uma string vazia após 'F'
    pushToStack(inode->pointer[i]);         // Devolve o bloco para a pilha de blocos livres
  }

  // Libera blocos indiretos, se houver
  if (inode->indirect != -1)
  {
    IndirectInode *indirect = &disk[inode->indirect].indirectInode;
    for (int i = 0; i < MAX_INDIRECT_POINTERS && indirect->pointer[i] != -1; i++)
    {
      disk[indirect->pointer[i]].type[0] = 'F';
      disk[indirect->pointer[i]].type[1] = '\0';
      pushToStack(indirect->pointer[i]);
    }
    disk[inode->indirect].type[0] = 'F';
    disk[inode->indirect].type[1] = '\0';
    pushToStack(inode->indirect);
  }

  // Limpa o inode principal
  disk[inodeIndex].type[0] = 'F';
  disk[inodeIndex].type[1] = '\0';
  memset(inode, 0, sizeof(PrincipalInode));
  pushToStack(inodeIndex);

  // Remove o arquivo do diretório
  for (int i = fileIndex; i < disk[dirIndex].directory.TL - 1; i++)
  {
    disk[dirIndex].directory.index[i] = disk[dirIndex].directory.index[i + 1];
    strcpy(disk[dirIndex].directory.name[i], disk[dirIndex].directory.name[i + 1]);
  }
  disk[dirIndex].directory.TL--;
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

  int blockIndex = disk[dirIndex].directory.index[directoryIndex];
  if (strcmp(disk[blockIndex].type, "DIR") != 0)
  {
    printf("O item especificado não é um diretório.\n");
    return;
  }

  if (disk[blockIndex].directory.TL > 2)
  { // Conta apenas . e ..
    printf("O diretório não está vazio.\n");
    return;
  }

  // Marca o bloco como livre
  strcpy(disk[blockIndex].type, "F");
  memset(&disk[blockIndex].directory, 0, sizeof(Directory)); // Limpa os dados do diretório

  // Remove o diretório do diretório pai
  for (int i = directoryIndex; i < disk[dirIndex].directory.TL - 1; i++)
  {
    disk[dirIndex].directory.index[i] = disk[dirIndex].directory.index[i + 1];
    strcpy(disk[dirIndex].directory.name[i], disk[dirIndex].directory.name[i + 1]);
  }
  disk[dirIndex].directory.TL--;

  pushToStack(blockIndex); // Devolve o bloco para a pilha de blocos livres
}

void Bad(Block disk[], int blockIndex)
{
  strcpy(disk[blockIndex].type, "B");
}

void chmod(Block disk[], char *fileName, char *command)
{
  int directoryIndex = currentDirectoryIndex; // Usa o diretório atual.
  if (directoryIndex == -1)
  {
    printf("Diretório não encontrado.\n");
    return;
  }

  // Encontra o arquivo no diretório atual
  int fileIndex = findFileInDirectory(disk[directoryIndex].directory, fileName);
  if (fileIndex == -1)
  {
    printf("Arquivo não encontrado.\n");
    return;
  }

  int blockIndex = disk[directoryIndex].directory.index[fileIndex];
  if (strcmp(disk[blockIndex].type, "I") != 0)
  {
    printf("%s não é um arquivo.\n", fileName);
    return;
  }

  // Interpretando o comando de permissão
  char command_type;
  char who[4];
  char permissions[4];
  if (sscanf(command, "%c%s%s", &command_type, who, permissions) != 3)
  {
    printf("Comando de permissão inválido.\n");
    return;
  }

  // Determina qual permissão será alterada (R, W, ou X)
  char perm;
  switch (permissions[0])
  {
  case 'R':
    perm = 'r';
    break;
  case 'W':
    perm = 'w';
    break;
  case 'X':
    perm = 'x';
    break;
  default:
    printf("Permissão inválida.\n");
    return;
  }

  // Determina o índice inicial no array de permissões
  int start_index;
  switch (who[0])
  {
  case 'u':
    start_index = 0;
    break;
  case 'g':
    start_index = 3;
    break;
  case 'o':
    start_index = 6;
    break;
  default:
    printf("Usuário inválido.\n");
    return;
  }

  // Atualiza as permissões do arquivo de acordo com o comando
  for (int i = 0; i < (int)strlen(permissions); i++)
  {
    if (command_type == '+')
      disk[blockIndex].principalInode.permissions[start_index + i] = perm;
    else if (command_type == '-')
      disk[blockIndex].principalInode.permissions[start_index + i] = '-';
  }
  printf("Permissões do arquivo %s alteradas com sucesso.\n", fileName);
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
    printf("\n%s\n > ", currentPath);
    fgets(command, 256, stdin);

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
    else if (sscanf(command, "touch %s %d", arg1, &fileSizeInBytes) == 2)
    {
      touch(disk, arg1, fileSizeInBytes, blockSize);
    }
    else if (strcmp(command, "help") == 0)
    {
      printf("Comandos disponíveis: exit, ls, touch <filename>, mkdir <directory>, cd <directory>, rm <filename>, rmdir <directory>, df, clear\n");
    }
    else if (sscanf(command, "mkdir %s", arg1) == 1)
    {
      mkdir(disk, currentDirectoryIndex, arg1);
    }
    else if (sscanf(command, "mkdir %s %s", arg1, arg2) == 2)
    {
      mkdirFromPath(disk, arg1, arg2);
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

    else
    {
      printf("Comando não reconhecido.\n");
    }
  }

  return 0;
}