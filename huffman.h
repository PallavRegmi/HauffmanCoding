/**********************************************/
/*Himanshu Chaudhary                          */
/*May 11, 2016                                */
/*CS 241L    Section #003                     */
/**********************************************/

#ifndef HUFFMAN_H
#define HUFFMAN_H

/*the strcuture of each node in huffman Tree*/
struct huffTree
{
	int symbol;
	unsigned long frequency;
	struct huffTree* right;
	struct huffTree* left;
};


/*The main two functions which stats the enocding and decoding process  */

void startEncode(FILE* in, FILE* out);
void startDecode(FILE* in, FILE* out);

/* functions used to sort the elemnts for the huffman Tree*/
int compareFreq(const void* a, const void* b);

/*functions used to create the frequency table, huffmantrr
 * and the symbolcodes*/
void BuildTree();
void traverseTree(struct huffTree *pool, char *prefix, char **table);
char *Concat(char *prefix, char letter);
unsigned long countFrequency(FILE* in);
int minVal(struct huffTree* root);
void BuildHuffmanTable(char **table);
void BuildTree();


/*functions used to perform operations on huffTree*/
struct huffTree* createNodes(unsigned long *frequency, int i);
struct huffTree** getTree(struct huffTree** headRef, int i);


/* Print list data on single line, separated with spaces. */
void printList(struct huffTree* queue);
void printTable(unsigned long *frequency, char** table);
void freeList(struct huffTree* head);

/*function used to encode,decode data and read and write data from file*/
void WriteHeader(FILE *out, unsigned long frequencies[]);
unsigned int flipBinary(int num);
void writeChar(char *code, unsigned int *rem, int *remLength, FILE *out);
void readHeader(FILE* in);
int getBit(FILE *in);
int DecodeSymbol(FILE*in, struct huffTree *tree);
void decodeFile(FILE* in, FILE* out);





















#endif
