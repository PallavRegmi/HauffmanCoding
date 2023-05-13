#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

unsigned long frequency[255]; 
int queueLength = 0;
struct huffTree *tree[256]; 

int minVal(struct huffTree* root)
{
  if (root == NULL)
    return -1;  

  if (root->left == NULL && root->right == NULL)
    return root->symbol;  

  if (root->left != NULL)
    return minVal(root->left);  

  return minVal(root->right);  
}


void WriteHeader(FILE* outputFile, unsigned long frequency[])
{
    int count, i = 0;
    unsigned long total = 0;

    for (i = 0; i < 256; i++)
    {
        if (frequency[i])
        {
            fwrite(&i, sizeof(char), 1, outputFile);
            fwrite(&(frequency[i]), sizeof(unsigned long), 1, outputFile);
            count++;
            total += frequency[i];
        }
    }

    fwrite(&count, sizeof(int), 1, outputFile);
}


unsigned int flipBinary(int num)
{
  unsigned int finalValue = 0;
  int exponent = 128; 
  int i;
  for (i = 0; i < 8; i++)
  {
    finalValue |= (num & 1) * exponent;
    num >>= 1;
    exponent >>= 1;
  }

  return finalValue;
}

void writeChar(char* code, unsigned int* rem, int* remLength, FILE* out)
{
    unsigned int finalValue = *rem;
    unsigned int bitLength = *remLength;
    int length = strlen(code);
    int count;
    for (count = 0; count < length; count++)
    {
        finalValue <<= 1;
        finalValue |= code[count] - '0';
        bitLength++;

        if (bitLength == 8)
        {
            finalValue = flipBinary(finalValue);
            fwrite(&finalValue, sizeof(char), 1, out);
            bitLength = 0;
            finalValue = 0;
        }
    }

    *rem = finalValue;
    *remLength = bitLength;
}

void BuildHuffmanTable(char** table)
{
    int index = 0;
    char* prefix = (char*)calloc(1, sizeof(char));
    int i;
    for (i = 0; i < 256; i++)
    {
        if (frequency[i])
        {
            struct huffTree* n = createNodes(frequency, i);
            tree[index] = n;
            queueLength++;
            index++;
        }
    }
    BuildTree();
    traverseTree(*tree, prefix, table);
}

void startEncode(FILE* in, FILE* out)
{
    unsigned long totalChars = 0;
    char* table[256];
    unsigned int rem = 0;
    int length = 0;

    totalChars = countFrequency(in);
    BuildHuffmanTable(table);

    WriteHeader(out, frequency);
    fwrite(&totalChars, sizeof(unsigned long), 1, out);

    rewind(in);

    while (1)
    {
        int c = fgetc(in);
        if (c == EOF)
            break;
        writeChar(table[c], &rem, &length, out);
    }

    printTable(frequency, table);
    freeList(*tree);
}

void readHeader(FILE* in)
{
    unsigned char length;
    char symbol;
    unsigned long freq;
    int i;
    fread(&length, sizeof(unsigned char), 1, in);
    
    for (i = 0; i < length; i++)
    {
        fread(&symbol, sizeof(char), 1, in);
        fread(&freq, sizeof(unsigned long), 1, in);
        frequency[(unsigned char)symbol] = freq;
    }

    fread(&freq, sizeof(unsigned long), 1, in);
}


int getBit(FILE* in)
{
    static int bits = 0;
    static int bitcount = 0;
    int lastBit;

    if (bitcount == 0)
    {
        bits = fgetc(in);
        if (bits == EOF)
            return 10000;
        bitcount = 8;
    }

    lastBit = bits & 1;
    bits >>= 1;
    bitcount--;

    return lastBit;
}

int DecodeSymbol(FILE* in, struct huffTree* tree)
{
    int next;
  
    while (tree->left || tree->right)
    {
        next = getBit(in);

        if (next == 10000)
            return 10000;

        if (next)
            tree = tree->right;
        else
            tree = tree->left;
    }

    return tree->symbol;
}

void startDecode(FILE* in, FILE* out)
{
    int c;
    char* table[256];

    readHeader(in);
    BuildHuffmanTable(table);

    while ((c = DecodeSymbol(in, *tree)) != 10000)
        fwrite(&c, sizeof(char), 1, out);

    printTable(frequency, table);
}



void traverseTree(struct huffTree *tree, char *prefix, char **table)
{
  if(!tree->left && !tree->right)
  {
    table[tree->symbol]=prefix;
  }
  else
  {
    if(tree->left)
    {
      traverseTree(tree->left, Concat(prefix, '0'),table);
    }
    if(tree->right)
    {
      traverseTree(tree->right, Concat(prefix, '1'),table);
    }
    free(prefix);
  }
}


char* Concat(char* prefix, char letter)
{
    size_t prefixLen = strlen(prefix);
    char* result = (char*)malloc(prefixLen + 2);

    sprintf(result, "%s%c", prefix, letter);

    return result;
}


void BuildTree()
{
    int length = queueLength;
    int i;
    while (length > 1)
    {
        struct huffTree* node = malloc(sizeof(struct huffTree));
        struct huffTree** n = getTree(tree, 0);
        struct huffTree** m = getTree(tree, 1);

        qsort(tree, queueLength, sizeof(struct huffTree*), compareFreq);

        node->left = (*n);
        node->right = (*m);
        node->symbol = node->left->symbol;
        node->frequency = node->right->frequency + node->left->frequency;

        tree[0] = node;
        queueLength--;
        length--;

        for (i = 1; i <= queueLength; i++)
        {
            tree[i] = tree[i + 1];
        }
    }
}


int compareFreq(const void* a, const void* b)
{
   struct huffTree **first = ( struct huffTree**)a;
   struct huffTree **second = (struct huffTree**)b;

  if((*first)->frequency < (*second)->frequency)
  {
    return -1;
  }
  else if((*first)->frequency > (*second)->frequency)
  {
    return +1;
  }
  if((*first)->frequency == (*second)->frequency)
    {
      return (minVal(*first) < minVal(*second)) ? -1:1;
    }
  else return 0;
}


struct huffTree** getTree(struct huffTree** headRef, int i)
{
    struct huffTree** toGet = &headRef[i];
    return toGet;
}


struct huffTree* createNodes(unsigned long* frequency, int i)
{
    struct huffTree* node = malloc(sizeof(struct huffTree));
    node->symbol = i;
    node->frequency = frequency[i];
    node->right = NULL;
    node->left = NULL;

    return node;
}

unsigned long countFrequency(FILE* in)
{
    unsigned long count = 0;
    int c;
    int i;
    for (i = 0; i < 256; i++)
    {
        frequency[i] = 0;
    }

    while ((c = getc(in)) != EOF)
    {
        if (c >= 0 && c <= 255)
        {
            frequency[c]++;
            count++;
        }
    }

    return count;
}


void printCode(char* code)
{
    int length = strlen(code);
    int i;
    for (i = 0; i < length; i++)
    {
        printf("%c", code[i]);
    }

    printf("\n");
}



void printTable(unsigned long* frequency, char** table)
{
    int count = 0;
    int j;
    printf("Symbol\tFreq\tCode\n");
    for (j = 0; j < 256; j++)
    {
        if (frequency[j])
        {
            count += frequency[j];
            if (j < 33 || j > 126)
            {
                printf("=%d \t%lu\t", j, frequency[j]);
            }
            else
            {
                printf("%c \t%lu\t", j, frequency[j]);
            }
            printCode(table[j]);
        }
    }

    printf("Total chars = %d\n", count);
}


void freeList(struct huffTree* head)
{
    struct huffTree* current = head;
    struct huffTree* temp;

    while (current != NULL)
    {
        temp = current;
        current = current->right;
        free(temp);
    }
}
