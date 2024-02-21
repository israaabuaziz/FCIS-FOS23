#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

int total_num_of_pages = 0;
uint32 user_pages[((uint32)USER_HEAP_MAX-(uint32)USER_HEAP_START)/(uint32)PAGE_SIZE]={0};
uint32 gh=0;

uint32 FF_logic(int num_of_pages) {
	uint32 itr_index = (((uint32)sys_get_hard_limit() + (uint32)PAGE_SIZE) - (uint32)USER_HEAP_START) / (uint32)PAGE_SIZE;
	uint32 end = ((uint32)USER_HEAP_MAX - (uint32)USER_HEAP_START) / (uint32)PAGE_SIZE;
	bool found = 0;
	int found_pages = 0;
	uint32 va;
	while(itr_index < end){

		if(user_pages[itr_index] == 0){
			found_pages++;
		}
		else{
			found_pages = 0;
			itr_index += user_pages[itr_index];
			itr_index--;
		}

		if(found_pages == 1)
			va = USER_HEAP_START + (itr_index * PAGE_SIZE);

		if(found_pages == num_of_pages){
			found = 1;
			break;
		}

		itr_index ++;
	}

	if(found == 0)
		return 0;

	uint32 temp = ((uint32)va - (uint32)USER_HEAP_START) / (uint32)PAGE_SIZE;
	user_pages[temp] = num_of_pages;

//	cprintf("va = %x\n",va);
	return va;
}

void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'23.MS2 - #09] [2] USER HEAP - malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//	panic("malloc() is not implemented yet...!!");
	//	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

	total_num_of_pages = (uint32)USER_HEAP_MAX - (((uint32)sys_get_hard_limit() + (uint32)PAGE_SIZE)) / (uint32)PAGE_SIZE;
	uint32 new_size = ROUNDUP(size,PAGE_SIZE);
	uint32 num_of_pages = new_size / (uint32)PAGE_SIZE;
	uint32 va;

	if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		if(sys_isUHeapPlacementStrategyFIRSTFIT()){
			return alloc_block_FF(size);
		}
	}
	else{
		if(sys_isUHeapPlacementStrategyFIRSTFIT()){
			uint32 ret = FF_logic(num_of_pages);

			if(ret == 0)
			{
				return NULL;//ask here or in  allocate user mem.
			}
			else
			{
				va = ret;
			}

			sys_allocate_user_mem(va,(uint32)new_size);


			return (void*)va;
		}
	}


	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================

void free(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #11] [2] USER HEAP - free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

    uint32 hard_limit=sys_get_hard_limit();
	if(((uint32)virtual_address>=USER_HEAP_START) &&((uint32)virtual_address<hard_limit)){
		free_block(virtual_address);
	}
	else if(((uint32)virtual_address>=hard_limit+PAGE_SIZE)&&((uint32)virtual_address<USER_HEAP_MAX)){

		uint32 va =ROUNDDOWN((uint32)virtual_address,PAGE_SIZE);
		int index = ((uint32)va - (uint32)USER_HEAP_START) / (uint32)PAGE_SIZE;

		uint32 size=(user_pages[index])*PAGE_SIZE;

		if(size == 0){
			return;
		}

		sys_free_user_mem((uint32)virtual_address,size);

		//update new
		user_pages[index]=0;

	}
	else{
			panic("Invalid address");
	}
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	panic("smalloc() is not implemented yet...!!");
	return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");
	return NULL;
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
