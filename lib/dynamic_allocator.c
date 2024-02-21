/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
uint32 get_block_size(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->size ;
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
int8 is_free_block(void* va)
{
	struct BlockMetaData *curBlkMetaData = ((struct BlockMetaData *)va - 1) ;
	return curBlkMetaData->is_free ;
}

//===========================================
// 3) ALLOCATE BLOCK BASED ON GIVEN STRATEGY:
//===========================================
void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockMetaData* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", blk->size, blk->is_free) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
struct  MemBlock_LIST tyr;
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
//	cprintf("Before fault\n");
	if (initSizeOfAllocatedSpace == 0)
	{
//		cprintf("I'm here\n");
		return ;
	}

	is_initialized = 1;
	//=========================================
	//=========================================

	//TODO: [PROJECT'23.MS1 - #5] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator()
//	cprintf("Before fault 2\n");
	struct BlockMetaData *initial_block = (struct BlockMetaData *)daStart;
//	cprintf("Before fault 3\n");
	initial_block->size=initSizeOfAllocatedSpace;
	initial_block->is_free=1;
//	initial_block->prev_next_info.le_next=(struct BlockMetaData *)NULL;
//	initial_block->prev_next_info.le_prev=(struct BlockMetaData *)NULL;
//	cprintf("After fault\n");
	LIST_INIT(&tyr);
	LIST_INSERT_HEAD(&tyr,initial_block);

//	panic("initialize_dynamic_allocator is not implemented yet");
}

//=========================================
// [4] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - #6] [3] DYNAMIC ALLOCATOR - alloc_block_FF()
	//panic("alloc_block_FF is not implemented yet");

	//cprintf("in alloc block ff\n");

	uint32 totalSize = (uint32)size + (uint32)sizeOfMetaData();

//	cprintf("total size = %d\n",totalSize);

	if(((uint32)size) == 0)
		return NULL;

	if (!is_initialized)
	{
		uint32 required_size = size + sizeOfMetaData();
//		cprintf("size to allocate: %d\n", required_size);
		uint32 da_start1 = (uint32)sbrk(required_size);
		//get new break since it's page aligned! thus, the size can be more than the required one
		uint32 da_break1 = (uint32)sbrk(0);
		//is_initialized = 1;
//		cprintf("Before init.\n");
//		cprintf("Start: %x\n", da_start1);
//		cprintf("break: %x\n", da_break1);
		initialize_dynamic_allocator(da_start1, da_break1 - da_start1);
	}

	struct BlockMetaData* itr = tyr.lh_first;
	if(itr == tyr.lh_first)
		itr->prev_next_info.le_prev = itr;

	LIST_FOREACH(itr,&tyr) //&itr
	{
		if(itr->is_free == 1 && itr->size > 0)
		{
			if((uint32)itr->size == totalSize)
			{
				itr->is_free = 0;
				itr->size = totalSize;

				void * final;
				final =(void *) ((uint32)itr+((uint32)sizeOfMetaData()));
			    return final;
			}
			else if((uint32)itr->size > totalSize)
			{
				int splittedSize = (itr->size) - totalSize;
				if(splittedSize > sizeOfMetaData())
				{
					struct BlockMetaData *SplitedBlock= (void *)((uint32)itr +(uint32)totalSize);
					SplitedBlock->is_free = 1;
					SplitedBlock->size = splittedSize;
					itr->size = totalSize;

					LIST_INSERT_AFTER(&tyr, itr, SplitedBlock);
				}
				else
				{
					itr->size = totalSize + splittedSize;
				}

				itr->is_free = 0;

				void * final;
				final =(void *) ((uint32)itr+((uint32)sizeOfMetaData()));
				return final;
			}
		}

	}

	void* sbrk_ret = sbrk(totalSize);

	if(sbrk_ret == (void*)-1)
		return NULL;
	else
	{
//		cprintf("in block ff sbrk \n");
		struct BlockMetaData* lastBlock = LIST_LAST(&tyr);
//		cprintf("1\n");
		struct BlockMetaData* newBlock = (struct BlockMetaData*)sbrk_ret;
//		cprintf("2\n");
//		cprintf("in block ff sbrk newBlock = %x\n",newBlock);
//		cprintf("in block ff sbrk lastBlock = %x\n",lastBlock);
		newBlock->is_free = 0;
//		cprintf("3\n");
		newBlock->size = totalSize;
//		cprintf("4\n");
		LIST_INSERT_TAIL(&tyr,newBlock);
//		cprintf("5\n");

		uint32 new_sbrk = (uint32)sbrk(0);
		uint32 used_size_by_sbrk = new_sbrk - (uint32)sbrk_ret;
		if(used_size_by_sbrk != totalSize){
			uint32 splited_size = used_size_by_sbrk - totalSize ;

			if(splited_size >= sizeOfMetaData()){
				struct BlockMetaData *SplitedBlock= (void *)((uint32)new_sbrk -(uint32)splited_size);
				SplitedBlock->is_free = 1;
				SplitedBlock->size = splited_size;

				LIST_INSERT_TAIL(&tyr,SplitedBlock);
			}
			else
			{
				newBlock->size = totalSize + (int)splited_size;
			}
		}

		void * final;
		final =(void *) ((uint32)sbrk_ret+((uint32)sizeOfMetaData()));
		return final;
	}



	return NULL;

}
//=========================================
// [5] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'23.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF()
	//panic("alloc_block_BF is not implemented yet");

	struct BlockMetaData *itr = tyr.lh_first;    struct BlockMetaData *ptr = NULL;
	    uint32 totalSize = (uint32) size + (uint32) sizeOfMetaData(), finalBlockSize = 0;

	    struct BlockMetaData *final = NULL;

	    if (((uint32) size) == 0)
	        return (void*)NULL;

	    if (itr == tyr.lh_first)
	     	 itr->prev_next_info.le_prev = itr;


	    LIST_FOREACH(itr, &tyr)
	    {

	        if (itr->is_free == 1 && totalSize <= itr->size)
	        {
	            if ((uint32) itr->size == totalSize)
	            {
	                ptr = itr;
	                ptr->is_free = 0;
	                uint32 st = (uint32)ptr + sizeOfMetaData();
	                final = (struct BlockMetaData *) (st);
	                return (void *)final;
	            }

	            else if ((struct BlockMetaData *)ptr == NULL)
	            {
	                ptr = itr;
	                final = (struct BlockMetaData *) ((uint32) sizeOfMetaData() + (uint32)ptr);
	                finalBlockSize = ptr->size - sizeOfMetaData();
	            }

	            else if ((struct BlockMetaData *) ptr != NULL)
	            {
	                if ((uint32) (itr->size-sizeOfMetaData())<(uint32) finalBlockSize)
	                {
	                    ptr = itr;
	                    finalBlockSize = ptr->size-sizeOfMetaData();
	                    final = (struct BlockMetaData *) ((uint32) sizeOfMetaData() + (uint32)ptr);
	                }

	            }
	        }

	    }


	    if (ptr == NULL)
	    {
	        if ((int)sbrk(totalSize) == -1)
	            return NULL;
	        else return NULL;
	    }

	    else
	    {
	        ptr->is_free = 0;
	        if (totalSize < ptr->size -sizeOfMetaData())
	        {
	            struct BlockMetaData *SplitedBlock = (struct BlockMetaData *)(totalSize + (uint32)ptr);
	            SplitedBlock->is_free = 1;
	            SplitedBlock->size = ptr->size - (totalSize);
	            LIST_INSERT_AFTER(&tyr, ptr, SplitedBlock);
	            ptr->size = size + sizeOfMetaData();
	        }
	        return final;
	    }
}

//=========================================
// [6] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [7] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}

//===================================================
// [8] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'23.MS1 - #7] [3] DYNAMIC ALLOCATOR - free_block()
	//panic("free_block is not implemented yet");

	struct BlockMetaData *ptr = (void *)((int)va - (int)sizeOfMetaData());

	if(ptr == tyr.lh_last)
		ptr->prev_next_info.le_next = ptr;

	//	DO Nothing
	  if(ptr == NULL)
	  {
		  return ;
	  }

	//  previous & next are not free
	  if(LIST_NEXT(ptr)->is_free != 1 && LIST_PREV(ptr)->is_free != 1)
	  {
		  ptr->is_free = 1;
	  }
	  //  previous & next are free
	  else if(LIST_NEXT(ptr)->is_free == 1 && LIST_PREV(ptr)->is_free == 1)
	  {
		 struct BlockMetaData* next = ptr->prev_next_info.le_next;
		 LIST_PREV(ptr)->size += LIST_NEXT(ptr)->size + ptr->size;
		 ptr->prev_next_info.le_next->size = 0;
		 ptr->prev_next_info.le_next->is_free = 0;
		 ptr->prev_next_info.le_prev->prev_next_info.le_next =  ptr->prev_next_info.le_next->prev_next_info.le_next;
		 ptr->size = 0;
		 ptr->is_free = 0;
		 LIST_REMOVE(&tyr,next);
		 LIST_REMOVE(&tyr,ptr);
	  }
	  //  previous || next are free
	  else if(LIST_NEXT(ptr)->is_free == 1 || LIST_PREV(ptr)->is_free == 1)
	  {
		  if(LIST_NEXT(ptr)->is_free == 1 )
	      {
			  struct BlockMetaData* next = ptr->prev_next_info.le_next;
			  ptr->size += ptr->prev_next_info.le_next->size ;
			  ptr->prev_next_info.le_next = ptr->prev_next_info.le_next->prev_next_info.le_next;
			  next->size = 0;
			  next->is_free = 0;
			  ptr->is_free = 1;
			  LIST_REMOVE(&tyr,next);
	      }
		  else if(LIST_PREV(ptr)->is_free == 1)
		  {
			  ptr->prev_next_info.le_prev->size += ptr->size ;
			  ptr->prev_next_info.le_prev->prev_next_info.le_next = ptr->prev_next_info.le_next;
			  ptr->size = 0;
//			  ptr->is_free = 1;
			  LIST_REMOVE(&tyr,ptr);
		  }
	  }

}

//=========================================
// [4] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'23.MS1 - #8] [3] DYNAMIC ALLOCATOR - realloc_block_FF()
	//panic("realloc_block_FF is not implemented yet");

	struct BlockMetaData* itr = (void *)((int)va - (int)sizeOfMetaData());
	int totalSize = (int)new_size + (int)sizeOfMetaData();

	if(va == NULL){
//		cprintf("1) va == null %x\n",itr);
		return alloc_block_FF(new_size);
	}

	if(new_size == 0)
	{
		free_block(va);
		return NULL;
	}
	else if(totalSize == itr->size)
	{
		return va;
	}
	else if(totalSize > itr->size)
	{
		if(itr->prev_next_info.le_next->size == (new_size - itr->size))
		{
			struct BlockMetaData* next = itr->prev_next_info.le_next;
			itr->prev_next_info.le_next->size = 0;
			itr->prev_next_info.le_next->is_free = 0;
			itr->prev_next_info.le_next = itr->prev_next_info.le_next->prev_next_info.le_next;
			LIST_REMOVE(&tyr,next);
			itr->size = totalSize;
			itr->is_free = 0;
			return va;
		}
		else if(itr->prev_next_info.le_next->size > (new_size - itr->size))
		{
			int allsize = itr->prev_next_info.le_next->size + itr->size;
			struct BlockMetaData* next = itr->prev_next_info.le_next;
			itr->prev_next_info.le_next->size = 0;
			itr->prev_next_info.le_next->is_free = 0;
			itr->prev_next_info.le_next = itr->prev_next_info.le_next->prev_next_info.le_next;
			LIST_REMOVE(&tyr,next);
			itr->is_free = 0;

			int splittedSize = allsize - totalSize;
			if(splittedSize > sizeOfMetaData())
			{
				struct BlockMetaData *SplitedBlock= (void *)((int)itr +(int)totalSize);
				SplitedBlock->is_free = 1;
				SplitedBlock->size = splittedSize;
				LIST_INSERT_AFTER(&tyr, itr, SplitedBlock);
				itr->size = totalSize;
			}
			else
			{
				itr->size = totalSize + splittedSize;
			}

			void * final;
			final =(void *) ((uint32)itr+((uint32)sizeOfMetaData()));
			return final;
		}
		else{
			free_block(va);
			return alloc_block_FF(new_size);
		}
	}
	else if(totalSize < itr->size)
	{
		int splittedSize = itr->size - totalSize;
		itr->size = totalSize;
		itr->is_free = 0;
		struct BlockMetaData *SplitedBlock= (void *)((int)itr +(int)totalSize);

		if(LIST_NEXT(itr)->is_free == 1 )
		{
		  struct BlockMetaData* next = itr->prev_next_info.le_next;
		  SplitedBlock->size += itr->prev_next_info.le_next->size ;
		  SplitedBlock->prev_next_info.le_next = itr->prev_next_info.le_next->prev_next_info.le_next;
		  next->size = 0;
		  next->is_free = 0;
//		  itr->is_free = 1;
		  LIST_REMOVE(&tyr,next);
		}
		else if(splittedSize > sizeOfMetaData())
		{
			SplitedBlock->is_free = 1;
			SplitedBlock->size = splittedSize;
			LIST_INSERT_AFTER(&tyr, itr, SplitedBlock);
//			itr->size = totalSize;
		}
		else if(splittedSize <= sizeOfMetaData())
		{
			itr->size += splittedSize;
		}



		void * final;
		final =(void *) ((uint32)itr+((uint32)sizeOfMetaData()));
		return final;
	}


	return NULL;
}
