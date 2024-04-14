#define MAX_STACKS 10
#define MAX_BLOCKS_PER_STACK 10
#define MAX_FREE_BLOCKS 100

// Cada índice em 'stackHeads' representa o topo de uma pilha individual dentro da pilha de pilhas.
// Inicializamos todos com -1 para indicar pilhas vazias.
int stackHeads[MAX_STACKS];

// array de pilhas, onde cada linha representa uma pilha e armazena os índices dos blocos livres.
int freeBlockStacks[MAX_STACKS][MAX_BLOCKS_PER_STACK];

// Um índice para acompanhar o topo da pilha de pilhas.
int stackOfStacksHead = -1;

// Inicializa todas as pilhas como vazias
void initializeFreeBlockStacks()
{
    for (int i = 0; i < MAX_STACKS; i++)
    {
        stackHeads[i] = -1;
    }
}

// Função para adicionar um bloco livre a uma pilha específica.
void pushToStack(int freeBlockIndex)
{
    int found = 0;
    for (int i = 0; i <= stackOfStacksHead; i++)
    {
        if (stackHeads[i] < MAX_BLOCKS_PER_STACK - 1)
        {
            freeBlockStacks[i][++stackHeads[i]] = freeBlockIndex;
            found = 1;
            break;
        }
    }

    if (!found)
    {
        if (stackOfStacksHead < MAX_STACKS - 1)
        {
            stackOfStacksHead++;
            stackHeads[stackOfStacksHead] = 0;
            freeBlockStacks[stackOfStacksHead][0] = freeBlockIndex;
        }
        else
        {
            printf("Não há mais espaço para novas pilhas de blocos livres.\n");
        }
    }
}

// Função para remover e retornar o topo de uma pilha específica.
int popFromStack(int stackNumber)
{
    if (stackHeads[stackNumber] > -1)
    {
        int blockIndex = freeBlockStacks[stackNumber][stackHeads[stackNumber]];
        stackHeads[stackNumber]--;
        return blockIndex;
    }
    else
    {
        printf("A pilha %d está vazia.\n", stackNumber);
        return -1;
    }
}

// Função para adicionar uma nova pilha ao topo da pilha de pilhas.
void pushNewStack()
{
    if (stackOfStacksHead < MAX_STACKS - 1)
    {
        stackOfStacksHead++;
        stackHeads[stackOfStacksHead] = -1;
    }
}

// Função para remover e retornar o topo da pilha de pilhas.
int popStack()
{
    if (stackOfStacksHead > -1)
    {
        int topStackIndex = stackOfStacksHead;
        stackOfStacksHead--;
        return topStackIndex;
    }
    else
    {
        // Handle empty stack of stacks situation.
        return -1;
    }
}

void fillFreeBlockStacks()
{
    int blockIndex = 1; // Começa do bloco 1, assumindo que o bloco 0 é reservado para o diretório raiz
    for (int i = 0; i < MAX_STACKS && blockIndex < 1000; i++)
    {
        for (int j = 0; j < MAX_BLOCKS_PER_STACK && blockIndex < 1000; j++)
        {
            freeBlockStacks[i][j] = blockIndex++;
            stackHeads[i] = j; // Atualiza o topo da pilha
        }
    }
}