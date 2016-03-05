#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"basicFun.h"
#include<math.h>

extern int BlockSize,L1Size,L1Assoc,VCNumBlock,L2Size,L2Assoc;//input params
extern int L2Sets, L2IndexBits, L2Index,L2TagBits, L2maxLRU;//L2 paramters
extern cacheBlock **L2Cache;

void extractParam(char *argv[], int *BlockSize, int *L1Size, int *L1Assoc, int *VCNumBlock,int *L2Size, int *L2Assoc)
{
	*BlockSize = atoi(argv[1]);
	*L1Size = atoi(argv[2]);
	*L1Assoc = atoi(argv[3]);
	*VCNumBlock = atoi(argv[4]);
	*L2Size = atoi(argv[5]);
	*L2Assoc = atoi(argv[6]);
}

int gotoCacheL1(int memAddr, cacheBlock **cache ,bool memRequest,int L1Assoc,int L1IndexBits, cacheBlock *L1Victim, int VCNumBlock)
{
	int i,j,k,p,index, memTag,tempLRU/*,tempMemTag=0*/,memAddrnew,memRequestnew;
	cacheBlock tempBlock;
	bool Hit = false;
	index = L1IndexBits?(memAddr & (int)(pow(2,L1IndexBits)-1)):0;
	memTag = memAddr >> L1IndexBits;
	switch(memRequest)
 	{
 		case READ:
 		//printf("L1 Read: %x(tag %x index %d)",memAddr,memTag,index);
 			for(i =0;i<L1Assoc;i++)
		 	{
		 	
		 		if((cache[index][i].tag == memTag)&&(cache[index][i].validBit == true))
		 		{
					tempLRU = cache[index][i].LRUCount; 
					
					for(j =0;j<L1Assoc;j++)
					{
						if(cache[index][j].LRUCount < tempLRU)
						{
							cache[index][j].LRUCount +=1;
						}
					}
					cache[index][i].LRUCount = 0;
					Hit = true; 
					//printf("\nL1 Read Hit");
		 			break;
		 		}
		 	}	
		 	if(!Hit)//L1 Read Miss
		 	{
		 		L1ReadMisses++;
			 	//printf("\nL1 Read Miss");
		 		if(VCNumBlock)
		 		{
		 			//gotoL1Victim(cache,index,READ,memAddr,VCNumBlock);
					
		 			for(i =0;i<L1Assoc;i++)
					{
						if(cache[index][i].LRUCount == (L1Assoc -1))
						{
							//check if it is valid or not
							if(cache[index][i].validBit == true)//LRU Block is VALID
							{
								noOfswapRequest++;
				/*----------------------------//Block found in VICTIM. Now do swapping stuff---------------------------------------------------*/
				 				for(k = 0; k<VCNumBlock;k++)
				 				{
				 				
							 		if((L1Victim[k].tag == memAddr)&&(L1Victim[k].validBit == true))
							 		{
							 			//printf("BLOCK found in Victm/ swapping\n");
										//printf("l1:%x, VC:%x\n",cache[index][i].addr,(L1Victim[k].tag >> L1IndexBits));
							 			noOfSwapsL1++;
							 			tempBlock.tag = L1Victim[k].tag >> L1IndexBits;
							 			tempBlock.DirtyBit = L1Victim[k].DirtyBit;
							 			tempBlock.addr = L1Victim[k].addr;
							 			
							 			L1Victim[k].tag = cache[index][i].addr;
							 			L1Victim[k].DirtyBit = cache[index][i].DirtyBit;
							 			L1Victim[k].addr = cache[index][i].addr;
							 			L1Victim[k].validBit = true;
							 			//Update LRU
							 			for(p =0;p<VCNumBlock;p++)
										{
											if(L1Victim[p].LRUCount < L1Victim[k].LRUCount)
											{
												L1Victim[p].LRUCount +=1;
											}
										}
										L1Victim[k].LRUCount = 0;
						
										cache[index][i].tag = tempBlock.tag;
										cache[index][i].DirtyBit = tempBlock.DirtyBit;//for read keep DIRTYBIT as it is.
										cache[index][i].addr = tempBlock.addr ;
										cache[index][i].validBit = true;
										//update LRU
										for(p =0;p<L1Assoc;p++)
										{
											if(cache[index][p].LRUCount < cache[index][i].LRUCount)
											{
												cache[index][p].LRUCount +=1;
											}
										}
										cache[index][i].LRUCount = 0;
										i = L1Assoc;
										k = VCNumBlock;
										

										return 0;
							 		}
							 	}
				/*--------------------------------------------------------------------------------------------------------*/
							 	//We are here means Block is NOT in Victim
							 
							 	for(k = 0; k<VCNumBlock;k++)
				 				{
				 						
				 					if(L1Victim[k].LRUCount == (VCNumBlock -1))//GO to LRU
				 					{
				 						if(L1Victim[k].validBit == true)
				 						{
				 							//printf("\nBlock is NOT in Victim && Victim has valid block\n");	
				 							if(L1Victim[k].DirtyBit == DIRTY)
				 							{
				 								noOfWriteBacksfromL1++;
				 								if(L2Size!=0) //you have L2 configured. Issue WRITE request to L2 
				 								{
				 									L2Writes++;
				 									gotoCacheL2(L1Victim[k].addr, L2Cache, WRITE ,L2Assoc, L2IndexBits);
				 								}
				 							}
				 							L1Victim[k].tag = cache[index][i].addr;
				 							L1Victim[k].DirtyBit = cache[index][i].DirtyBit;
				 							L1Victim[k].validBit = true;
				 							L1Victim[k].addr = cache[index][i].addr;
				 							
				 							for(p =0;p<VCNumBlock;p++)
											{
												if(L1Victim[p].LRUCount < L1Victim[k].LRUCount)
												{
													L1Victim[p].LRUCount +=1;
												}
											}
											L1Victim[k].LRUCount = 0;
											if(L2Size!=0) //you have L2 configured. Issue READ request to L2 
											{
												L2Reads++;
												gotoCacheL2(memAddr, L2Cache, READ ,L2Assoc, L2IndexBits);
											}
											L1VCMissRate++;
				 							cache[index][i].addr = memAddr;
				 							cache[index][i].DirtyBit = NOT_DIRTY;
				 							cache[index][i].validBit = true;
				 							cache[index][i].tag = memTag;
				 							
				 							for(j =0;j<L1Assoc;j++)
											{
												if(cache[index][j].LRUCount < cache[index][i].LRUCount)
												{
													cache[index][j].LRUCount +=1;
												}
											}
											cache[index][i].LRUCount = 0;
											
											i = L1Assoc;
											k = VCNumBlock;
				 							break;
				 						}
				 						else//it is NOT valid. that means we have empty L1Victm block. PLace L1 block here
				 						{
				 							//printf("\nBlock is NOT in Victim && Victim has empty block\n");	
				 							L1Victim[k].tag = cache[index][i].addr;
								 			L1Victim[k].DirtyBit = cache[index][i].DirtyBit;
								 			L1Victim[k].addr = cache[index][i].addr;
											L1Victim[k].validBit = true;
											for(p =0;p<VCNumBlock;p++)
											{
												if(L1Victim[p].LRUCount < L1Victim[k].LRUCount)
												{
													L1Victim[p].LRUCount +=1;
												}
											}
											L1Victim[k].LRUCount = 0;
								
							
											if(L2Size!=0) //you have L2 configured. Issue READ request to L2 
											{
												L2Reads++; 
												gotoCacheL2(memAddr, L2Cache, READ ,L2Assoc, L2IndexBits);							 			
								 			}
								 			L1VCMissRate++;
								 			cache[index][i].DirtyBit = NOT_DIRTY;//make is NOT_DIRTY as its a read request
						 					cache[index][i].validBit = 1;
											cache[index][i].tag = memTag;
											cache[index][i].addr = memAddr;
							
											for(j =0;j<L1Assoc;j++)
											{
												if(cache[index][j].LRUCount < cache[index][i].LRUCount)
												{
													cache[index][j].LRUCount +=1;
												}
											}
											cache[index][i].LRUCount = 0;
											
											i = L1Assoc;
											k = VCNumBlock;
											break;
				 						}
				 					}
				 				}
							}
							else//L1 has invalid block 
							{
								if(L2Size!=0) //you have L2 configured. Issue READ request to L2 
								{
				 					L2Reads++; 
									gotoCacheL2(memAddr, L2Cache ,READ, L2Assoc, L2IndexBits);
								}
								//printf("L1 has invalid block\n");
								L1VCMissRate++;
								cache[index][i].DirtyBit = NOT_DIRTY;
								cache[index][i].validBit = 1;
								cache[index][i].tag = memTag;
								cache[index][i].addr = memAddr;
								tempLRU = cache[index][i].LRUCount; 
			
								for(j =0;j<L1Assoc;j++)
								{
									if(cache[index][j].LRUCount < tempLRU)
									{
										cache[index][j].LRUCount +=1;
									}
								}
								cache[index][i].LRUCount = 0;
								break;
							}
						}
					}
		 			
		 		}
		 		else
		 		{
		 			for(i =0;i<L1Assoc;i++)
			 		{
			 			if(cache[index][i].LRUCount == (L1Assoc -1))
			 			{
			 				//check if it is valid or not
			 				if(cache[index][i].validBit == true)
			 				{
			 					if(L2Size!=0)
			 					{
					 				if(cache[index][i].DirtyBit == DIRTY)
					 				{
					 					memAddrnew = cache[index][i].addr;//(cache[index][i].tag );//<< L1IndexBits) | (index);
					 					memRequestnew = WRITE; 
					 					L2Writes++;
					 					noOfWriteBacksfromL1++;
					 					//printf("\n%x\n",cache[index][i].addr);
					 					//printf("memAddrNew = %x  memRequestnew = %x  Assoc = %d  L2indexBit = %d ",memAddrnew,memRequestnew,L2Assoc,L2IndexBits);
					 					gotoCacheL2(memAddrnew, L2Cache,memRequestnew,L2Assoc,L2IndexBits);
					 				}
					 				memAddrnew = memAddr;//cache[index][i].addr;//tag;// << L1IndexBits) | (index);
				 					memRequestnew = READ;
				 					L2Reads++; 
									//printf("\n%x\n",cache[index][i].addr);
				 					//printf("memAddrNew = %x  memRequestnew = %x  Assoc = %d  L2indexBit = %d ",memAddrnew,memRequestnew,L2Assoc,L2IndexBits);
				 					gotoCacheL2(memAddrnew, L2Cache ,memRequestnew,L2Assoc,L2IndexBits);
			 					}
			 					L1VCMissRate++;
			 					cache[index][i].DirtyBit = NOT_DIRTY;
			 					cache[index][i].validBit = 1;
								cache[index][i].tag = memTag;
								cache[index][i].addr = memAddr;
								tempLRU = cache[index][i].LRUCount; 
						
								for(j =0;j<L1Assoc;j++)
								{
									if(cache[index][j].LRUCount < tempLRU)
									{
										cache[index][j].LRUCount +=1;
									}
								}
								cache[index][i].LRUCount = 0;
								break;
							}
							else
							{
								if(L2Size!=0)
								{
									memAddrnew = memAddr;//(cache[index][i].addr);// << L1IndexBits) | (index);
				 					memRequestnew = READ;
				 					L2Reads++; 
									//printf("\n%x\n",cache[index][i].tag);
				 					//printf("memAddrNew = %x  memRequestnew = %x  Assoc = %d  L2indexBit = %d ",memAddrnew,memRequestnew,L2Assoc,L2IndexBits);
				 					gotoCacheL2(memAddrnew, L2Cache ,memRequestnew,L2Assoc,L2IndexBits);
			 					}
			 					L1VCMissRate++;
			 					cache[index][i].DirtyBit = NOT_DIRTY;
			 					cache[index][i].validBit = 1;
								cache[index][i].tag = memTag;
								cache[index][i].addr = memAddr;
								tempLRU = cache[index][i].LRUCount; 
						
								for(j =0;j<L1Assoc;j++)
								{
									if(cache[index][j].LRUCount < tempLRU)
									{
										cache[index][j].LRUCount +=1;
									}
								}
								cache[index][i].LRUCount = 0;
								break;
							}
			 			}
			 		}		 			
		 		}

			}
 		break;
 		
 		case WRITE://write request
 		//printf("L1 Write: %x(tag %x index %d)\n",memAddr,memTag,index);
 		for(i =0;i<L1Assoc;i++)
	 	{
	 		if((cache[index][i].tag == memTag)&&(cache[index][i].validBit == true))//HIT or miss
	 		{																	   //HIT		
				cache[index][i].tag = memTag;
				cache[index][i].addr = memAddr;
				cache[index][i].DirtyBit = DIRTY;
				//printf("Set Dirty1");
				tempLRU = cache[index][i].LRUCount; 
				
				cache[index][i].validBit = 1;
				for(j =0;j<L1Assoc;j++)
				{
					if(cache[index][j].LRUCount < tempLRU)
					{
						cache[index][j].LRUCount +=1;
					}
				}
				cache[index][i].LRUCount = 0;
				Hit = true; 
				//printf("L1 write Hit\n");
	 			break;
	 		}
	 	}	
	 	if(!Hit)//L1 Write Miss
	 	{
		 	L1WriteMisses++;
		 	//printf("\nL1 Write Miss");
		 	if(VCNumBlock)
	 		{
	 				for(i =0;i<L1Assoc;i++)
					{
						if(cache[index][i].LRUCount == (L1Assoc -1))
						{
							//check if it is valid or not
							if(cache[index][i].validBit == true)//LRU Block is VALID
							{
								noOfswapRequest++;
				/*----------------------------//Block found in VICTIM. Now do swapping stuff---------------------------------------------------*/
				 				for(k = 0; k<VCNumBlock;k++)
				 				{
				 					
							 		if((L1Victim[k].tag == memAddr)&&(L1Victim[k].validBit == true))
							 		{
								 		//printf("BLOCK found in Victm/ swapping\n");
										//printf("l1:%x, VC:%x\n",cache[index][i].addr,(L1Victim[k].tag >> L1IndexBits));
							 			noOfSwapsL1++;
							 			tempBlock.tag = L1Victim[k].tag >> L1IndexBits;
							 			tempBlock.DirtyBit = L1Victim[k].DirtyBit;
							 			tempBlock.addr = L1Victim[k].addr;
							 			
							 			L1Victim[k].tag = cache[index][i].addr;
							 			L1Victim[k].DirtyBit = cache[index][i].DirtyBit;
							 			L1Victim[k].addr = cache[index][i].addr;
							 			L1Victim[k].validBit = true;
							 			//Update LRU
							 			for(p =0;p<VCNumBlock;p++)
										{
											if(L1Victim[p].LRUCount < L1Victim[k].LRUCount)
											{
												L1Victim[p].LRUCount +=1;
											}
										}
										L1Victim[k].LRUCount = 0;
						
										cache[index][i].tag = tempBlock.tag;
										cache[index][i].DirtyBit = DIRTY;
										cache[index][i].addr = tempBlock.addr ;
										cache[index][i].validBit = true;
										//update LRU
										for(p =0;p<L1Assoc;p++)
										{
											if(cache[index][p].LRUCount < cache[index][i].LRUCount)
											{
												cache[index][p].LRUCount +=1;
											}
										}
										cache[index][i].LRUCount = 0;
										
										i = L1Assoc;
										k = VCNumBlock;
										return 0;//break breaks this for loop but executes next hence return
							 		}
							 	}
				/*--------------------------------------------------------------------------------------------------------*/
							 	//We are here means Block is NOT in Victim
							 	//printf("\nBlock is NOT in Victim.\n");
							 	for(k = 0; k<VCNumBlock;k++)
				 				{
				 					
				 					if(L1Victim[k].LRUCount == (VCNumBlock -1))//GO to LRU
				 					{
				 						if(L1Victim[k].validBit == true)
				 						{
				 							//printf("\nBlock is NOT in Victim && Victim has valid block\n");	
				 							if(L1Victim[k].DirtyBit == DIRTY)
				 							{
				 								noOfWriteBacksfromL1++;
				 								if(L2Size!=0) //you have L2 configured. Issue WRITE request to L2 
				 								{
				 									L2Writes++;
				 									gotoCacheL2(L1Victim[k].addr, L2Cache, WRITE ,L2Assoc, L2IndexBits);
				 								}
				 							}
				 							L1Victim[k].tag = cache[index][i].addr;
				 							L1Victim[k].DirtyBit = cache[index][i].DirtyBit;
				 							L1Victim[k].validBit = true;
				 							L1Victim[k].addr = cache[index][i].addr;
				 							
				 							for(p =0;p<VCNumBlock;p++)
											{
												if(L1Victim[p].LRUCount < L1Victim[k].LRUCount)
												{
													L1Victim[p].LRUCount +=1;
												}
											}
											L1Victim[k].LRUCount = 0;
											if(L2Size!=0) //you have L2 configured. Issue READ request to L2 
											{
												L2Reads++;
												gotoCacheL2(memAddr, L2Cache, READ ,L2Assoc, L2IndexBits);
											}
											L1VCMissRate++;
				 							cache[index][i].addr = memAddr;
				 							cache[index][i].DirtyBit = DIRTY;
				 							cache[index][i].validBit = true;
				 							cache[index][i].tag = memTag;
				 							
				 							for(j =0;j<L1Assoc;j++)
											{
												if(cache[index][j].LRUCount < cache[index][i].LRUCount)
												{
													cache[index][j].LRUCount +=1;
												}
											}
											cache[index][i].LRUCount = 0;
				 							i = L1Assoc;
											k = VCNumBlock;
											break;
				 						}
				 						else//it is NOT valid. that means we have empty L1Victm block. PLace L1 block here
				 						{
				 							//printf("\nBlock is NOT in Victim && Victim has empty block\n");	
				 							L1Victim[k].tag = cache[index][i].addr;
								 			L1Victim[k].DirtyBit = cache[index][i].DirtyBit;
								 			L1Victim[k].addr = cache[index][i].addr;
											L1Victim[k].validBit = true;
											for(p =0;p<VCNumBlock;p++)
											{
												if(L1Victim[p].LRUCount < L1Victim[k].LRUCount)
												{
													L1Victim[p].LRUCount +=1;
												}
											}
											L1Victim[k].LRUCount = 0;
								
							
											if(L2Size!=0) //you have L2 configured. Issue READ request to L2 
											{
												L2Reads++; 
												gotoCacheL2(memAddr, L2Cache, READ ,L2Assoc, L2IndexBits);							 			
								 			}
								 			L1VCMissRate++;
								 			cache[index][i].DirtyBit = DIRTY;
						 					cache[index][i].validBit = 1;
											cache[index][i].tag = memTag;
											cache[index][i].addr = memAddr;
							
											for(j =0;j<L1Assoc;j++)
											{
												if(cache[index][j].LRUCount < cache[index][i].LRUCount)
												{
													cache[index][j].LRUCount +=1;
												}
											}
											cache[index][i].LRUCount = 0;
											i = L1Assoc;
											k = VCNumBlock;
											break;
				 						}
				 					}
				 				}
							}
							else//L1 has invalid block 
							{
								//printf("L1 has invalid block \n");
								if(L2Size!=0) //you have L2 configured. Issue READ request to L2 
								{
				 					L2Reads++; 
									gotoCacheL2(memAddr, L2Cache ,READ, L2Assoc, L2IndexBits);
								}
								L1VCMissRate++;
								cache[index][i].DirtyBit = DIRTY;				
								
								cache[index][i].validBit = 1;
								cache[index][i].tag = memTag;
								cache[index][i].addr = memAddr;
								tempLRU = cache[index][i].LRUCount; 
			
								for(j =0;j<L1Assoc;j++)
								{
									if(cache[index][j].LRUCount < tempLRU)
									{
										cache[index][j].LRUCount +=1;
									}
								}
								cache[index][i].LRUCount = 0;
								break;
							}
						}
					}
	 			
	 		}
	 		else
	 		{
	 			for(i =0;i<L1Assoc;i++)
	 			{
			 			if(cache[index][i].LRUCount == (L1Assoc -1))//found block to be replace
			 			{
			 				//check if it is valid or not
			 				if(cache[index][i].validBit == true)
			 				{
				 				if(L2Size!=0)
				 				{
					 				if(cache[index][i].DirtyBit == DIRTY)
					 				{											//evacuate, writeback to L2
					 					memAddrnew = (cache[index][i].addr );//<< L1IndexBits) | (index);
					 					memRequestnew = WRITE; 
					 					L2Writes++;
					 					noOfWriteBacksfromL1++;
										//printf("\n%x\n",cache[index][i].tag);
					 					//printf("memAddrNew = %x  memRequestnew = %x  Assoc = %d  L2indexBit = %d ",memAddrnew,memRequestnew,L2Assoc,L2IndexBits);
					 					gotoCacheL2(memAddrnew, L2Cache ,memRequestnew,L2Assoc,L2IndexBits);
					 				}
					 				memAddrnew = memAddr;//(cache[index][i].addr );//<< L1IndexBits) | (index);
				 					memRequestnew = READ; 
				 					L2Reads++;
									//printf("\n%x\n",cache[index][i].tag);
				 					//printf("memAddrNew = %x  memRequestnew = %x  Assoc = %d  L2indexBit = %d ",memAddrnew,memRequestnew,L2Assoc,L2IndexBits);
				 					gotoCacheL2(memAddrnew, L2Cache ,memRequestnew,L2Assoc,L2IndexBits);
				 				}	
				 				L1VCMissRate++;
			 					cache[index][i].DirtyBit = DIRTY;
			 					//printf("Set Dirty2");
			 					cache[index][i].validBit = 1;
								cache[index][i].tag = memTag;
								cache[index][i].addr = memAddr;
								tempLRU = cache[index][i].LRUCount; 
								for(j =0;j<L1Assoc;j++)
								{
									if(cache[index][j].LRUCount < tempLRU)
									{
										cache[index][j].LRUCount +=1;
									}
								}
								cache[index][i].LRUCount = 0;
								break;
							}
							else
							{
								if(L2Size!=0)
								{
									memAddrnew = memAddr;//(cache[index][i].addr);// << L1IndexBits) | (index);
				 					memRequestnew = READ; 
				 					L2Reads++;
									//printf("\n%x\n",cache[index][i].tag);
				 					//printf("memAddrNew = %x  memRequestnew = %x  Assoc = %d  L2indexBit = %d ",memAddrnew,memRequestnew,L2Assoc,L2IndexBits);
				 					gotoCacheL2(memAddrnew, L2Cache ,memRequestnew,L2Assoc,L2IndexBits);
			 					}
			 					L1VCMissRate++;
			 					cache[index][i].DirtyBit = DIRTY;
			 					//printf("Set Dirty3");
			 					cache[index][i].validBit = 1;
								cache[index][i].tag = memTag;
								cache[index][i].addr = memAddr;
								tempLRU = cache[index][i].LRUCount; 
								for(j =0;j<L1Assoc;j++)
								{
									if(cache[index][j].LRUCount < tempLRU)
									{
										cache[index][j].LRUCount +=1;
									}
								}
								cache[index][i].LRUCount = 0;
								break;
							}
			 			}
			 		}		 	
			 	}
			 			
	 		}
		 	
 		break;
   	}
   	return 0;
}
void gotoCacheL2(int memAddr, cacheBlock **cache ,bool memRequest,int L1Assoc,int L1IndexBits)
{
	int i,j,index, memTag,tempLRU;//,tempMemTag=0,memAddrnew,memRequestnew;
	bool Hit = false;
	index = L1IndexBits?(memAddr & (int)(pow(2,L1IndexBits)-1)):0;
	memTag = memAddr >> L1IndexBits;
	switch(memRequest)
 	{
 		case READ:
 		//printf("\nL2 Read: %x(tag %x index %d)",memAddr,memTag,index);
 			for(i =0;i<L1Assoc;i++)
		 	{
		 	
		 		if((cache[index][i].tag == memTag)&&(cache[index][i].validBit == true))
		 		{
					tempLRU = cache[index][i].LRUCount; 
					
					for(j =0;j<L1Assoc;j++)
					{
						if(cache[index][j].LRUCount < tempLRU)
						{
							cache[index][j].LRUCount +=1;
						}
					}
					cache[index][i].LRUCount = 0;
					Hit = true; 
					//printf("\nL2 Read Hit\n");
		 			break;
		 		}
		 	}	
		 	if(!Hit)//L2 Read Miss
		 	{
			 	L2ReadMisses++;
			 	//printf("\nL2 Read Miss\n");
			 	for(i =0;i<L1Assoc;i++)
		 		{
		 			if(cache[index][i].LRUCount == (L1Assoc -1))
		 			{
		 				//check if it is valid or not
		 				if(cache[index][i].validBit == true)
		 				{
			 				/*********************************************************/
			 				if(cache[index][i].DirtyBit == DIRTY)
			 				{
			 					//memAddrnew = (cache[index][i].tag << L1IndexBits &index);
			 					//memRequestnew = WRITE; 
			 					noOfWriteBacksfromL2++;
			 				//	gotoCacheL2(memAddrnew, cacheL2 ,memRequestnew,L2Assoc,L2IndexBits);
			 				}
			 				//memAddrnew = (cache[index][i].tag << L1IndexBits &index);
		 					//memRequestnew = READ; 
		 					//gotoCacheL2(memAddrnew, cacheL2 ,memRequestnew,L2Assoc,L2IndexBits);*/
		 					/**********************************************************/
		 					L2MissRate++;
		 					cache[index][i].DirtyBit = NOT_DIRTY;
		 					cache[index][i].validBit = 1;
							cache[index][i].tag = memTag;
							tempLRU = cache[index][i].LRUCount; 
						
							for(j =0;j<L1Assoc;j++)
							{
								if(cache[index][j].LRUCount < tempLRU)
								{
									cache[index][j].LRUCount +=1;
								}
							}
							cache[index][i].LRUCount = 0;
							break;
						}
						else
						{
							/*
							memAddrnew = (cache[index][i].tag << L1IndexBits &index);
		 					memRequestnew = READ; 
		 					//gotoCacheL2(memAddrnew, cacheL2 ,memRequestnew,L2Assoc,L2IndexBits);
		 					*/
		 					L2MissRate++;
		 					cache[index][i].DirtyBit = NOT_DIRTY;
		 					cache[index][i].validBit = 1;
							cache[index][i].tag = memTag;
							tempLRU = cache[index][i].LRUCount; 
						
							for(j =0;j<L1Assoc;j++)
							{
								if(cache[index][j].LRUCount < tempLRU)
								{
									cache[index][j].LRUCount +=1;
								}
							}
							cache[index][i].LRUCount = 0;
							break;
						}
		 			}
		 		}
			 	
		 	}
 		break;
 		case WRITE://write request
 		//printf("\nL2 Write: %x(tag %x index %d)\n",memAddr,memTag,index);
 		for(i =0;i<L1Assoc;i++)
	 	{
	 		if((cache[index][i].tag == memTag)&&(cache[index][i].validBit == true))//HIT or miss
	 		{																	   //HIT		
				cache[index][i].tag = memTag;
				cache[index][i].DirtyBit = DIRTY;
				//printf("Set Dirty1");
				tempLRU = cache[index][i].LRUCount; 
				
				cache[index][i].validBit = 1;
				for(j =0;j<L1Assoc;j++)
				{
					if(cache[index][j].LRUCount < tempLRU)
					{
						cache[index][j].LRUCount +=1;
					}
				}
				cache[index][i].LRUCount = 0;
				Hit = true; 
				//printf("\nL2 write Hit\n");
	 			break;
	 		}
	 	}	
	 	if(!Hit)//L2 Write Miss
	 	{
		 	L2WriteMisses++;
		 	for(i =0;i<L1Assoc;i++)
	 		{
	 			if(cache[index][i].LRUCount == (L1Assoc -1))//found block to be replace
	 			{
	 				//check if it is valid or not
	 				if(cache[index][i].validBit == true)
	 				{
		 				if(cache[index][i].DirtyBit == DIRTY)
		 				{											//evacuate, writeback to L2
		 					//memAddrnew = (cache[index][i].tag << L1IndexBits &index);
		 					//memRequestnew = WRITE;
		 					noOfWriteBacksfromL2++; 
		 				//	gotoCacheL2(memAddrnew, cacheL2 ,memRequestnew,L2Assoc,L2IndexBits);
		 				}
		 				//memAddrnew = (cache[index][i].tag << L1IndexBits &index);
	 					//memRequestnew = READ; 
	 					//gotoCacheL2(memAddrnew, cacheL2 ,memRequestnew,L2Assoc,L2IndexBits);
	 					L2MissRate++;
	 					cache[index][i].DirtyBit = DIRTY;
	 					//printf("Set Dirty2");
	 					cache[index][i].validBit = 1;
						cache[index][i].tag = memTag;
						tempLRU = cache[index][i].LRUCount; 
						for(j =0;j<L1Assoc;j++)
						{
							if(cache[index][j].LRUCount < tempLRU)
							{
								cache[index][j].LRUCount +=1;
							}
						}
						cache[index][i].LRUCount = 0;
						break;
					}
					else
					{
						/*
						memAddrnew = (cache[index][i].tag << L1IndexBits &index);
	 					memRequestnew = READ; 
	 					//gotoCacheL2(memAddrnew, cacheL2 ,memRequestnew,L2Assoc,L2IndexBits);
	 					*/
	 					L2MissRate++;
	 					cache[index][i].DirtyBit = DIRTY;
	 					//printf("Set Dirty3");
	 					cache[index][i].validBit = 1;
						cache[index][i].tag = memTag;
						tempLRU = cache[index][i].LRUCount; 
						for(j =0;j<L1Assoc;j++)
						{
							if(cache[index][j].LRUCount < tempLRU)
							{
								cache[index][j].LRUCount +=1;
							}
						}
						cache[index][i].LRUCount = 0;
						break;
					}
	 			}
	 		}		 	
		 	
	 	}
 		break;
   	}
}
