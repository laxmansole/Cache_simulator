#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"basicFun.h"

cacheBlock **L1Cache;
cacheBlock **L2cache;

void extractParam(char *argv[], int *BlockSize, int *L1Size, int *L1Assoc, int *VCNumBlock,int *L2Size, int *L2Assoc)
{
	*BlockSize = atoi(argv[1]);
	*L1Size = atoi(argv[2]);
	*L1Assoc = atoi(argv[3]);
	*VCNumBlock = atoi(argv[4]);
	*L2Size = atoi(argv[5]);
	*L2Assoc = atoi(argv[6]);
}

void initCache(cacheBlock **cache,int noOfSets,int Assoc)
{
	int i,j;
	cache = (cacheBlock **) malloc(noOfSets*sizeof(cacheBlock *));
	for (i = 0; i < noOfSets; i++)
	{	
		cache[i] = (cacheBlock *)malloc(Assoc*sizeof(cacheBlock));
	}
	for(i = 0;i<noOfSets;i++)
	{
		for(j = 0; j<Assoc;j++)
			{
				cache[i][j].validBit = false;
				cache[i][j].tag = 0;
				//cache[i][j].indx;//index is nothing but row number so used directly to jump to particular location
				cache[i][j].DirtyBit = false;
				cache[i][j].LRUCount = j;
			}
	}
}

int gotoCache(int memAddr, cacheBlock **cache ,bool memRequest)
{
 	return 0;
}
