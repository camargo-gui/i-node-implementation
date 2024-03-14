struct indirectInode {
  int pointer[5];
};

typedef struct indirectInode IndirectInode;

struct principalInode {
  char permissions[10]; // 1 - d ou i, 2 - u[RWX], 3 - g[RWX], 4... - o[RWX]
  char date[10];
  char hour[10];
  char userName[10];
  char groupName[10];
  int size;
  int countLinks;
  int directPointer[5];
  int indirectPointer[3];
};

typedef struct principalInode PrincipalInode;


struct block {
  char data[512];
  PrincipalInode principalInode;
  char type; // F - Free, B - Bad, I = Inode, D - Data
};

typedef struct block Block;

// Perguntas:
// Qual o tamanho total do bloco? Está correta a distribuiçao de bytes?
// Como diferenciar i-node direto e indireto?
// Como criar o arquivo de diretórios?
// Como inserir um novo arquivo? Será apenas texto de acordo com os char disponíveis?