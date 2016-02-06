#ifndef BASICFUN_H
#define BASICFUN_H
#include<stdbool.h>


enum request
{
	READ,
	WRITE
};
typedef struct
{
	bool validBit;
	int tag;
	int indx;
	//int offset,
	bool DirtyBit;
	int LRUCount;
}cacheBlock;

void extractParam(char *argv[], int *BlockSize, int *L1Size, int *L1Assoc, int *VCNumBlock,int *L2Size, int *L2Assoc);

void initCache(cacheBlock **cache,int noOfSets,int Assoc);

int gotoCache(int memAddr,cacheBlock **cache,bool memRequest);
#endif
