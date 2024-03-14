#include <stdio.h>
#include "i-node.h"

void initializeDisk(Block disk[1000]){
  for(int i = 0; i < 1000; i++){
    disk[i].type = 'F';
  }
}

void initializeFreeQueue(Block disk[1000]){
  int j = 100;
  for(int i=0; i<100; i++){
    for(int k = 0; k<10; k++){
      disk[i].data[k] = j;
      j++;
    }
  }
}

void main(){
  Block disk[1000];
  initializeDisk(&disk);
  initializeFreeQueue(&disk);
  printf("Disk initialized\n");
}