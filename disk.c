#include <stdio.h>
#include "i-node.h"
#include "directory.h"

#define TF 1000;

void initializeDisk(Block disk[]){
  for(int i = 0; i < 1000; i++){
    disk[i].type = 'F';
  }
}

void initializeFreeQueue(Block disk[]){
  int j = 100;
  for(int i=0; i<100; i++){
    for(int k = 0; k<10; k++){
      disk[i].[k] = j;
      j++;
    }
  }
}

void initializeRootDirectory(Block disk[]){
	Directory root;
	root.TL = 0;
	Block newBlock;
	insertNewEntry(root, ".", 100);
	insertNewEntry(root, "..", 100);
	newBlock.directory = root;
	disk[100] = newBlock;
}

void teste(Block disk[]){
	printf("Diretorio ---->\n");
	printf("\n%s", disk[100].directory.name[0]);
	printf("\n%s", disk[100].directory.name[1]);
}

int main(){
  Block disk[1000];
  initializeDisk(disk);
  initializeFreeQueue(disk);
  printf("Disk initialized\n");
  initializeRootDirectory(disk);
  teste(disk);
}