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
  int freeBlockIndex = getRandomFreeBlock(); // pegando bloco livre de forma aleatoria
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
  disk[freeBlockIndex].type = 'D';
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
      if (foundIndex == -1 || disk[disk[dirIndex].directory.index[foundIndex]].type != 'D')
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

void deleteFile(Block disk[], char *fileName)
{
  int dirIndex = currentDirectoryIndex; // Usa o diretório atual.
  int fileIndex = findFileInDirectory(disk[dirIndex].directory, fileName);

  if (fileIndex == -1)
  {
    printf("Arquivo não encontrado.\n");
    return;
  }

  int blockIndex = disk[dirIndex].directory.index[fileIndex];
  if (disk[blockIndex].type != 'I')
  {
    printf("O item especificado não é um arquivo.\n");
    return;
  }

  // Marca o bloco como livre
  disk[blockIndex].type = 'F';
  memset(&disk[blockIndex].principalInode, 0, sizeof(PrincipalInode)); // Limpa os dados do inode

  // Remove o arquivo do diretório
  for (int i = fileIndex; i < disk[dirIndex].directory.TL - 1; i++)
  {
    disk[dirIndex].directory.index[i] = disk[dirIndex].directory.index[i + 1];
    strcpy(disk[dirIndex].directory.name[i], disk[dirIndex].directory.name[i + 1]);
  }
  disk[dirIndex].directory.TL--;

  pushToStack(blockIndex); // Devolve o bloco para a pilha de blocos livres
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
  if (disk[blockIndex].type != 'D')
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
  disk[blockIndex].type = 'F';
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

int main()
{
  int numBlocks;
  printf("Digite o número de blocos para o disco: ");
  scanf("%d", &numBlocks);

  while (getchar() != '\n')
    ;

  Block disk[numBlocks];
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
    else if (sscanf(command, "touch %s", arg1) == 1)
    {
      touch(disk, arg1);
    }
    else if (strcmp(command, "help") == 0)
    {
      printf("Comandos disponíveis: exit, ls, touch <filename>, mkdir <directory>, cd <directory>, rm <filename>, rmdir <directory>, clear\n");
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
    else
    {
      printf("Comando não reconhecido.\n");
    }
  }

  return 0;
}