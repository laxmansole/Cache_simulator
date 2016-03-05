#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"basicFun.h"


	FILE *traceFile;
	char *tracebuff;
	char line[256];//
	char *ptr_rw,*ptr_add;
	char ch;
	int memAddr, memRequest,n=0, returnedAdd;
	int BlockSize,L1Size,L1Assoc,VCNumBlock,L2Size,L2Assoc;//input params
	int L1Sets, L1IndexBits, L1Index,OffsetBits, L1TagBits, L1maxLRU;//L1 Parametes
	int L2Sets, L2IndexBits, L2Index,L2TagBits, L2maxLRU;//L2 paramters
	cacheBlock **L1Cache = NULL;
	cacheBlock **L2Cache = NULL;
	cacheBlock *L1Victim = NULL;

int main(int argc, char *argv[]) 
{
	int i,j,k;//for lop iterators
	if(argc <8)
	{
		printf("Error: less than 7 arguments\n");
		exit(0);		
	}
	else if(argc>8)
	{
		printf("Error: More than 8 arguments\n");	
		exit(0);
	}
	/************************************************************************************************/
	/*Parameter extraction and other calculations*/
	extractParam(argv,&BlockSize,&L1Size,&L1Assoc,&VCNumBlock,&L2Size, &L2Assoc);
	L1Sets = L1Size/(L1Assoc*BlockSize);
	if(L2Size!=0)
	{
		L2Sets = L2Size/(L2Assoc*BlockSize);
	}
	L1IndexBits = log2(L1Sets);
	L2IndexBits = log2(L2Sets);
	OffsetBits  = log2(BlockSize);
	L1maxLRU =(L1Assoc-1); 
	L2maxLRU =(L2Assoc-1); 
	//printf("\nL1 Params:\nL1Sets:%d\nL1IndexBits:%d\nOffsetBits= %d\n",L1Sets,L1IndexBits,OffsetBits);
	//printf("\nL2 Params:\nL2Sets:%d\nL2IndexBits:%d\nOffsetBits= %d\n",L2Sets,L2IndexBits,OffsetBits);
	traceFile = fopen(argv[7],"r");
	if(traceFile == NULL)
	{
		printf("Unable to open file %s\n",argv[7]);
		return 1;
	}
	//printf("%s file opened successfully\n",argv[7]);
	/**************************************************************************************************/
	//initCache(L1Cache,L1Sets,L1Assoc,L1Victim,VCNumBlock); //NOT WORKING:(:(
	/********************************************************************************************/
	L1Cache = (cacheBlock **) malloc(L1Sets*sizeof(cacheBlock *));
	for (i = 0; i < L1Sets; i++)
	{	
		L1Cache[i] = (cacheBlock *)malloc(L1Assoc*sizeof(cacheBlock));
	}
	for(i = 0;i<L1Sets;i++)
	{
		for(j = 0; j<L1Assoc;j++)
			{
				L1Cache[i][j].validBit = false;
				L1Cache[i][j].tag = 0;
				//cache[i][j].indx;//index is nothing but row number so used directly to jump to particular location
				L1Cache[i][j].DirtyBit = NOT_DIRTY;
				L1Cache[i][j].LRUCount = j;
			}
	}
	/*********************************************************************************************/
	if(L2Size!=0)
	{
		L2Cache = (cacheBlock **) malloc(L2Sets*sizeof(cacheBlock *));
		for (i = 0; i < L2Sets; i++)
		{	
			L2Cache[i] = (cacheBlock *)malloc(L2Assoc*sizeof(cacheBlock));
		}
		for(i = 0;i<L2Sets;i++)
		{
			for(j = 0; j<L2Assoc;j++)
				{
					L2Cache[i][j].validBit = false;
					L2Cache[i][j].tag = 0;
					//cache[i][j].indx;//index is nothing but row number so used directly to jump to particular location
					L2Cache[i][j].DirtyBit = NOT_DIRTY;
					L2Cache[i][j].LRUCount = j;
				}
		}
	}
	/********************************************************************************************/
	if(VCNumBlock)
	{
		L1Victim = (cacheBlock *)malloc(VCNumBlock*sizeof(cacheBlock));
		for(j = 0; j<VCNumBlock;j++)
			{
				L1Victim[j].validBit = false;
				L1Victim[j].tag = 0;
				//cache[i][j].indx;//index is nothing but row number so used directly to jump to particular location
				L1Victim[j].DirtyBit = NOT_DIRTY;
				L1Victim[j].LRUCount = j;
			}
	 }
	/***********************************************************************************************/
	/*for(i = 0;i<VCNumBlock;i++)
	{
		//printf("\nSet %d:",i);
		for(i = 0; i<VCNumBlock;i++)
			{
				printf("\t%x\t%d\t%d \n  ",L1Victim [i].tag, L1Victim [i].DirtyBit,L1Victim[i].LRUCount);
			}
	}*/
	while(fgets(line,sizeof(line),traceFile))
	{
		ptr_rw = strtok(line," ");
		if(ptr_rw!=NULL)
		{
			//printf("%s\n",ptr_rw);
		}
		ptr_add = strtok(NULL,",");
		if(ptr_add!=NULL)
		{
			sscanf(ptr_add,"%x",&memAddr);
			memAddr = memAddr >> OffsetBits;
			//add = strtol(ptr_add,NULL,16);
			//dec2bin(add);
			//printf("\n%s\t%d\n",ptr_add,add);
		}
		
		if(!strcmp(ptr_rw,"r"))
		{
			memRequest = READ;
			L1Reads++;
		}
		else
		{
			memRequest = WRITE;
			L1Writes++;
		}
		n++;
		//printf("\n\n#%d  ",n);
		//if(memRequest) printf(" Write ");
		//else printf(" Read ");
		//printf(" memAddr : %x\n",memAddr);
		gotoCacheL1(memAddr,L1Cache ,memRequest,L1Assoc,L1IndexBits,L1Victim,VCNumBlock);
		//gotoCache(memAddr,L1Cache,memRequest,L1Assoc,L1IndexBits);
		//printf("\n%d",returnedAdd);
	}
	printf("\n===== Simulator configuration =====\n");
	printf(" BLOCKSIZE:%d\n L1_SIZE:%d\n L1_ASSOC:%d",BlockSize,L1Size,L1Assoc);
	printf("\n VC_NUM_BLOCKS:%d\n L2_SIZE:%d\n L2_ASSOC:%d\n trace_file:%s",VCNumBlock,L2Size,L2Assoc,argv[7]);
	printf("\n===== L1 contents =====\n");
	for(i = 0;i<L1Sets;i++)
	{
		printf("\n set %d:",i);
		
		for(k = 0; k<L1Assoc;k++)
		{	
			for(j = 0; j<L1Assoc;j++)
			{	
				if((k == L1Cache[i][j].LRUCount) && (L1Cache[i][j].tag))
				{
					printf("\t%x\t   ",L1Cache[i][j].tag);
					if(L1Cache[i][j].DirtyBit)
					printf("D");
					else
					printf(" ");
					//printf("%d  ",L1Cache[i][j].LRUCount);
				}
			}
		}
	}
	if(VCNumBlock)	
	{
		printf("\n===== VC contents =====\n");
		printf("set 0:");
		for(k = 0;k<VCNumBlock;k++)
		{
			for(i =0; i<VCNumBlock;i++)
			{
				if( (L1Victim[i].LRUCount == k) && (L1Victim[i].tag))
				{
					printf(" %x",L1Victim[i].tag);
					if(L1Victim[i].DirtyBit==1)
					printf(" D");
					else
					printf("  ");
				}
			}
		}
	}
	if(L2Size!= 0)
	{
		printf("\n===== L2 contents =====\n");
		for(i = 0;i<L2Sets;i++)
		{
			printf("\n set %d:",i);
		
			for(k = 0; k<L2Assoc;k++)
			{	
				for(j = 0; j<L2Assoc;j++)
				{	
					if((k == L2Cache[i][j].LRUCount) && (L2Cache[i][j].tag))
					{
						printf("\t%x\t   ",L2Cache[i][j].tag);
						if(L2Cache[i][j].DirtyBit)
						printf("D");
						else
						printf(" ");
						//printf("%d  ",L2Cache[i][j].LRUCount);
					}
				}
			}
		}
	}
	printf("\n\n===== Simulation results =====");
	printf("\n a. number of L1 reads:\t\t\t%d",L1Reads);
	printf("\n b. number of L1 read misses:\t\t%d",L1ReadMisses);
	printf("\n c. number of L1 writes:\t\t\t%d",L1Writes);
	printf("\n d. number of L1 Write  misses:\t\t%d",L1WriteMisses);
	printf("\n e. number of swap requests:\t\t%d",noOfswapRequest);
		swapRate =(double)noOfswapRequest/n;
	printf("\n f. swap request rate:\t\t\t%.4f",swapRate);
	printf("\n g. number of swaps:\t\t\t%d",noOfSwapsL1);
	printf("\n h. combined L1+VC miss rate:\t\t%.4f",(L1VCMissRate/n));	
	printf("\n i. number writebacks from L1/VC:\t%d",noOfWriteBacksfromL1);
	printf("\n j. number of L2 reads:\t\t\t%d",L2Reads);
	printf("\n k. number of L2 read misses:\t\t%d",L2ReadMisses);
	printf("\n l. number of L2 writes:\t\t\t%d",L2Writes);
	printf("\n m. number of L2 write  misses:\t\t%d",L2WriteMisses);
	if(L2Size)
	{
		L2MissRate = ((double)L2ReadMisses )/((double)L2Reads);// + (double)L2Writes);
	}
	printf("\n n. L2 miss rate:\t\t\t%.4f",L2MissRate);
	printf("\n o. number of writebacks from L2:\t%d",noOfWriteBacksfromL2);
	
	totalMemtraffic = L2Size ?(L2ReadMisses+ L2WriteMisses+ noOfWriteBacksfromL2 ):(L1ReadMisses+ L1WriteMisses+ noOfWriteBacksfromL1-noOfSwapsL1);
	printf("\n p. total memory traffic:\t\t%d\n\n",totalMemtraffic);
	
	/*
	free(**L1Cache);
	free(**L2Cache);
	free(*L1Victim);
	*/
	fclose(traceFile);
	return 0;
}
