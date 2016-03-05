#ifndef BASICFUN_H
#define BASICFUN_H
#include<stdbool.h>
int L1Reads,L1ReadMisses,L1Writes,L1WriteMisses,noOfWriteBacksfromL1,noOfSwapsL1,totalMemtraffic,noOfswapRequest;
double swapRate,L2MissRate,L1VCMissRate;
int L2Reads,L2ReadMisses,L2Writes,L2WriteMisses,noOfWriteBacksfromL2;
//void gotoL1Victim(bool missType, int memAddr,int VCNumBlock);

enum request
{
	READ,
	WRITE
};
enum dbit_status
{
	NOT_DIRTY,
	DIRTY
};
typedef struct
{
	bool validBit;
	unsigned int tag;
	unsigned int addr;
	//int indx;
	//int offset,
	bool DirtyBit;
	int LRUCount;
}cacheBlock;

void extractParam(char *argv[], int *BlockSize, int *L1Size, int *L1Assoc, int *VCNumBlock,int *L2Size, int *L2Assoc);

void initCache(cacheBlock **cache,int noOfSets,int Assoc,cacheBlock *L1Victim,int VCNumBlock);

int gotoCacheL1(int memAddr, cacheBlock **cache ,bool memRequest,int L1Assoc,int L1IndexBits, cacheBlock *victim,  int VCNumBlock);
void gotoCacheL2(int memAddr, cacheBlock **cache ,bool memRequest,int L1Assoc,int L1IndexBits);
#endif
