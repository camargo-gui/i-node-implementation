#pragma once
#include "directory.h"

struct indirectInode
{
  int pointer[5];
};

typedef struct indirectInode IndirectInode;

struct principalInode
{
  char permissions[10]; // 1 - d ou l, 2 - u[RWX], 3 - g[RWX], 4... - o[RWX]
  struct tm *date;
  char userName[10];
  char groupName[10];
  int size;
  int countLinks;
  int pointer[8];
  char name[256];
  int indirect;
};

typedef struct principalInode PrincipalInode;

struct block
{
  char link[50];
  Directory directory;
  IndirectInode indirectInode;
  PrincipalInode principalInode;
  char type[4];  // F - Free, B - Bad, I = Inode, D - Data, DIR - Directory
};

typedef struct block Block;