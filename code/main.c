#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"basicFun.h"
/*void dec2bin(int c)
{
   int i = 0;
   for(i = 31; i >= 0; i--){
     if((c & (1 << i)) != 0){
       printf("1");
     }else{
       printf("0");
     } 
   }
}*/
	FILE *traceFile;
	char *tracebuff;
	char line[256];//
	char *ptr_rw,*ptr_add;
	char ch;
	int memAddr, memRequest;
	int BlockSize,L1Size,L1Assoc,VCNumBlock,L2Size,L2Assoc;//input params
	int L1Sets, L1IndexBits, L1Index,OffsetBits, L1TagBits, L1maxLRU;//L1 Parametes
	int L2Sets, L2IndexBits, L2Index,L2TagBits, L2maxLRU;//L2 paramters
int main(int argc, char *argv[]) 
{
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
	extractParam(argv,&BlockSize,&L1Size,&L1Assoc,&VCNumBlock,&L2Size, &L2Assoc);
	L1Sets = L1Size/(L1Assoc*BlockSize);
	L2Sets = L2Size/(L2Assoc*BlockSize);
	L1IndexBits = log2(L1Sets);
	L2IndexBits = log2(L2Sets);
	OffsetBits  = log2(BlockSize);
	L1maxLRU =(L1Assoc-1); 
	L2maxLRU =(L2Assoc-1); 
	printf("\nL1 Params:\nL1Sets:%d\nL1IndexBits:%d\nOffsetBits= %d\n",L1Sets,L1IndexBits,OffsetBits);
	printf("\nL2 Params:\nL2Sets:%d\nL2IndexBits:%d\nOffsetBits= %d\n",L2Sets,L2IndexBits,OffsetBits);
	traceFile = fopen(argv[7],"r");
	if(traceFile == NULL)
	{
		printf("Unable to open file %s\n",argv[7]);
		return 1;
	}
	printf("%s file opened successfully\n",argv[7]);
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
			memAddr = memAddr>>OffsetBits;
			//add = strtol(ptr_add,NULL,16);
			//dec2bin(add);
			//printf("\n%s\t%d\n",ptr_add,add);
		}
		
		if(strcmp(ptr_rw,"r"))
		{
			memRequest = READ;
		}
		else
		{
			memRequest = WRITE;
		}
		
		//gotoCache(memAddr,L1Cache,memRequest);
		printf("%s  %d\n",ptr_rw,memRequest);
	}
	fclose(traceFile);
	return 0;
}
