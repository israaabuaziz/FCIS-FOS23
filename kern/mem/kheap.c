#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"

uint32 total_num_of_pages = 0;

int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'23.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator()
	//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
	//All pages in the given range should be allocated
	//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
	//Return:
	//	On success: 0
	//	Otherwise (if no memory OR initial size exceed the given limit): E_NO_MEM

	//Comment the following line(s) before start coding...
//	panic("not implemented yet");

	KernelHeapStart = ROUNDDOWN(daStart, PAGE_SIZE);
	    Sbrk = ROUNDUP(KernelHeapStart + initSizeToAllocate, PAGE_SIZE);
		KernelHardLimit = ROUNDUP(daLimit, PAGE_SIZE);
		total_num_of_pages = (KERNEL_HEAP_MAX - (KernelHardLimit + PAGE_SIZE))/ PAGE_SIZE;

		struct FrameInfo *ptr_frame_info = 0;

		 for (uint32 i = KernelHeapStart; i < Sbrk ; i += PAGE_SIZE)
		 {
			 uint32 *PtrPageTable = NULL;
			//ptr_frame_info = get_frame_info( ptr_page_directory,i,&PtrPageTable);

			//fram allocating
//			allocate_frame(&ptr_frame_info);
			if(allocate_frame(&ptr_frame_info)!= 0)
			{
				// Failure
				return E_NO_MEM;
			}

			//fram mapping
//			map_frame( ptr_page_directory, ptr_frame_info,  i,  PERM_WRITEABLE);
			if(  map_frame( ptr_page_directory, ptr_frame_info,  i,  PERM_WRITEABLE)!= 0)
			{
			   // Failure
				return E_NO_MEM;
			}

			ptr_frame_info->va = i;

		 }

	  initialize_dynamic_allocator( KernelHeapStart, initSizeToAllocate);

	  //success
	  return 0;
}

void* sbrk(int increment)
{
	//TODO: [PROJECT'23.MS2 - #02] [1] KERNEL HEAP - sbrk()
	/* increment > 0: move the segment break of the kernel to increase the size of its heap,
	 * 				you should allocate pages and map them into the kernel virtual address space as necessary,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * increment = 0: just return the current position of the segment break
	 * increment < 0: move the segment break of the kernel to decrease the size of its heap,
	 * 				you should deallocate pages that no longer contain part of the heap as necessary.
	 * 				and returns the address of the new break (i.e. the end of the current heap space).
	 *
	 * NOTES:
	 * 	1) You should only have to allocate or deallocate pages if the segment break crosses a page boundary
	 * 	2) New segment break should be aligned on page-boundary to avoid "No Man's Land" problem
	 * 	3) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, kernel should panic(...)
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING====
	//panic("not implemented yet");
//	cprintf("Entered kernel\n");
	uint32 PreviousSbrk = Sbrk;

		if(increment == 0)
		{
//			cprintf("in sbrk = 0\n");
			return (void*) Sbrk;
		}

		if(increment > 0)
		{
//			cprintf("in sbrk > 0\n");
			uint32 NewSbrk= ROUNDUP(Sbrk + (uint32)increment,PAGE_SIZE);

			if(NewSbrk > KernelHardLimit)
			{
				return (void*)-1;
			}

	//		int NumberOfPages = (NewSbrk - Sbrk)/PAGE_SIZE;

			 struct FrameInfo *ptr_frame_info ;

			 for (uint32 i = Sbrk; i <= NewSbrk ; i += PAGE_SIZE)
				 {
					uint32 *PtrPageTable = NULL;
					ptr_frame_info = get_frame_info( ptr_page_directory,i,&PtrPageTable);

					//fram allocating
//					allocate_frame(&ptr_frame_info);
					if(allocate_frame(&ptr_frame_info)!= 0)
					{
						// Failure
						return (void*)-1;
					}

					 //frame mapping
//					map_frame( ptr_page_directory, ptr_frame_info,  i,  PERM_WRITEABLE && PERM_PRESENT);
					if( map_frame( ptr_page_directory, ptr_frame_info,  i,  PERM_WRITEABLE | PERM_PRESENT) != 0)
					{
						return (void*)-1;
					}

					ptr_frame_info->va = i;
				 }

			 Sbrk=NewSbrk;
			 return (void*) PreviousSbrk;

		  }

		if(increment < 0)
		{
//			cprintf("in sbrk < 0\n");
			//uint32 NewNegativeSbrk=ROUNDDOWN(Sbrk + increment,PAGE_SIZE );

			uint32 NewNegativeSbrk= Sbrk + (uint32)increment;

			if (NewNegativeSbrk < KernelHeapStart)
			{
				panic("sbrk() Below Kernel Heap Start ");
			}
			else if((increment*-1) < 4)
			{
				Sbrk = NewNegativeSbrk;
			    return (void*) Sbrk;
			}
			else if((increment*-1) >= 4)
			{
				int NumOfPages = (increment*-1)/4;

				for (int i = 1; i <= NumOfPages ; i++)
				{
				 unmap_frame(ptr_page_directory,(uint32) sbrk);
				 Sbrk-=PAGE_SIZE;
			    }

				Sbrk = NewNegativeSbrk;
				return (void*) Sbrk;
		    }

	   }

	return (void*)-1 ;
}

struct strust_process{
	uint32 start_address;
	uint32 num_of_used_pages;
};
struct strust_process process_array[300];
int num_of_used_process = -1;

int num_of_used_pages(uint32 start, uint32 end, bool is_free){
	uint32 used_pages = 0;
	uint32 itr_address = start;

	while(itr_address < end){
		uint32* ptr_page_table;
		struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, itr_address, &ptr_page_table);

		if(ptr_frame_info == NULL){
			if(is_free == 1)
				used_pages++;
		}
		else if(ptr_frame_info != NULL){
			if(is_free == 0)
				used_pages++;
		}

		itr_address += PAGE_SIZE;
	}

	return used_pages;
}

uint32 FF_logic(uint32 num_of_pages){
//	cprintf("in FF_logic\n");
	uint32 itr_address = KernelHardLimit + PAGE_SIZE;
	bool found = 0;
	uint32 found_pages = 0;
	uint32 va;
	while(itr_address < KERNEL_HEAP_MAX){
		uint32* ptr_page_table;
		struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory, itr_address, &ptr_page_table);

		if(ptr_frame_info == NULL){
			found_pages++;
		}
		else
			found_pages = 0;

		if(found_pages == 1)
			va = itr_address;

		if(found_pages == num_of_pages){
			found = 1;
			break;
		}

		itr_address += PAGE_SIZE;
	}

	if(found == 0)
		return 0;

//	cprintf("va = %x\n",va);
	return va;
}

int allocate_map(uint32 va, uint32 num_of_pages){
//	cprintf("in allocate_map");
	uint32 temp = va;
	for(uint32 i = 0; i < num_of_pages; i++){
		struct FrameInfo* frame_ptr = NULL;
		int res = allocate_frame(&frame_ptr);
		if(res == E_NO_MEM)
			return 0;

		int ret = map_frame(ptr_page_directory,frame_ptr,temp,PERM_WRITEABLE);
		if(ret != 0)
			return 0;

		//to kheap_va
		frame_ptr->va = temp;

		temp += PAGE_SIZE;
	}
	return 1;
}

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'23.MS2 - #03] [1] KERNEL HEAP - kmalloc()
	//refer to the project presentation and documentation for details
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//change this "return" according to your answer
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	total_num_of_pages = KERNEL_HEAP_MAX - ((KernelHardLimit + PAGE_SIZE));
	uint32 new_size = ROUNDUP(size,PAGE_SIZE);
	uint32 num_of_pages = new_size / PAGE_SIZE;
//	cprintf("new_size = %d\n",new_size);
//	cprintf("num_of_pages = %d\n",num_of_pages);


	if(size == 0)
		return NULL;
	else if(size <= DYN_ALLOC_MAX_BLOCK_SIZE){
//		cprintf("in size <\n");
		if(isKHeapPlacementStrategyFIRSTFIT() == 1){
			return alloc_block_FF(size);
		}
	}
	else
	{
//		cprintf("in size > (else) \n");
		if(isKHeapPlacementStrategyFIRSTFIT() == 1){
			uint32 used_pages = num_of_used_pages(KernelHardLimit+PAGE_SIZE, KERNEL_HEAP_MAX, 0);
			used_pages += num_of_pages;
			if(used_pages > total_num_of_pages)
				return NULL;

			//FF call
			uint32 va;
			uint32 ret = FF_logic(num_of_pages);
			if(ret == 0)
				return NULL;
			else
				va = ret;

			//allocate & map call
			int res = allocate_map(va,num_of_pages);
			if(res == 0)
				return NULL;

			//data needed for free
			num_of_used_process++;
			process_array[num_of_used_process].start_address = va;
			process_array[num_of_used_process].num_of_used_pages = num_of_pages;

			return (void*)va;
		}

	}

	return NULL;
}


int get_process_index(uint32 va){
	va = ROUNDDOWN(va, PAGE_SIZE);
	uint32 i;
	for(i = 0; i <= num_of_used_process; i++){
		if(process_array[i].start_address == va){
			break;
		}
	}

	return i;
}

void delete_index_of_free(int index){
	for(int i = index; i < num_of_used_process; i++){
		process_array[i] = process_array[i+1];
	}

	num_of_used_process--;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #04] [1] KERNEL HEAP - kfree()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	if((uint32)virtual_address >= KernelHeapStart && (uint32)virtual_address < KernelHardLimit){
		free_block(virtual_address);
	}
	else if((uint32)virtual_address >= KernelHardLimit+PAGE_SIZE && (uint32)virtual_address < KERNEL_HEAP_MAX){
		int index = get_process_index((uint32)virtual_address);
		if(index>num_of_used_process)
			return;
		void* va = ROUNDDOWN(virtual_address, PAGE_SIZE);

		for(uint32 i = 0; i < process_array[index].num_of_used_pages; i++){
//			uint32* ptr_page_dir = NULL;
//			struct FrameInfo* ptr_frame_info = get_frame_info(ptr_page_directory,(uint32)va,&ptr_page_dir);
//			ptr_frame_info->va = (uint32)0;

			unmap_frame(ptr_page_directory,(uint32)va);
//			free_frame(ptr_frame);

			va = (void*) ((uint32)va + (uint32)PAGE_SIZE);
		}

		delete_index_of_free(index);

		//Am I delete page table or not?
	}
	else{
		panic("Invalid address");
	}


}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'23.MS2 - #05] [1] KERNEL HEAP - kheap_virtual_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
//	panic("kheap_virtual_address() is not implemented yet...!!");

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================

	uint32 FrameNumber = physical_address / PAGE_SIZE;
	uint32 Offset = physical_address % PAGE_SIZE;
//	uint32 Offset = physical_address & 0xFFF;

	uint32 virtualAdrr;
	struct FrameInfo* ptr_frame_info = to_frame_info(physical_address);
	virtualAdrr =ptr_frame_info->va;

	uint32* ptrTable=NULL;
	struct FrameInfo* result = get_frame_info(ptr_page_directory , virtualAdrr , &ptrTable);
	if(result == NULL)
		return 0;

	if(ptr_frame_info != NULL)
	{
		virtualAdrr = (ptr_frame_info->va) + Offset;
		return virtualAdrr;
	}
	else
		return 0;

	//change this "return" according to your answer
	//return va;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'23.MS2 - #06] [1] KERNEL HEAP - kheap_physical_address()
	//refer to the project presentation and documentation for details
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	uint32 PhysicalAddress;

	uint32 PageDirectoryEntry = PDX(virtual_address);
	uint32 pageTableEntry = PTX(virtual_address);
	uint32 Offset = virtual_address  & 0xFFF  ;   // & 0xFFF   <<20

	if (!(ptr_page_directory[PageDirectoryEntry] & PERM_PRESENT))
	{
		return 0;
	}

	uint32 *ptr_page_table = NULL;
	int ret = get_page_table(ptr_page_directory, virtual_address, &ptr_page_table);

	if (ret != TABLE_IN_MEMORY)
	{
		return 0;
	}

	if (!(ptr_page_table[pageTableEntry] & PERM_PRESENT))
	{
		return 0;
	}

	 PhysicalAddress = (ptr_page_table[pageTableEntry] & 0xFFFFF000 ) + Offset;// >> 12

	return PhysicalAddress;

	//change this "return" according to your answer
	//return 0;
}


void kfreeall()
{
	panic("Not implemented!");

}

void kshrink(uint32 newSize)
{
	panic("Not implemented!");
}

void kexpand(uint32 newSize)
{
	panic("Not implemented!");
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'23.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc()
	// Write your code here, remove the panic and write your code

	if(virtual_address == NULL)
		return kmalloc(new_size);
	else{
		int index = get_process_index((uint32)virtual_address);
		uint32 old_size = process_array[index].num_of_used_pages * (int)PAGE_SIZE ;

		if(new_size == 0){
			kfree(virtual_address);
		}
		else if(new_size == old_size){
			return virtual_address;
		}
		else if(new_size > old_size){
			kfree(virtual_address);
			return kmalloc(new_size);
		}
		else if(new_size < old_size){
			if((uint32)virtual_address >= KernelHeapStart && (uint32)virtual_address < KernelHardLimit){
				return realloc_block_FF(virtual_address,new_size);
			}
			else if((uint32)virtual_address >= KernelHardLimit+PAGE_SIZE && (uint32)virtual_address < KERNEL_HEAP_MAX){
				int num_of_pages = ROUNDUP(new_size,PAGE_SIZE) / (uint32)PAGE_SIZE;
				int num_of_old_pages = process_array[index].num_of_used_pages;
				void* va = virtual_address ;

				if(num_of_pages != num_of_old_pages){
					for(int i = num_of_pages; i < num_of_old_pages; i++){
						va = (void*) ((uint32)va + ((uint32)PAGE_SIZE*i) );
						unmap_frame(ptr_page_directory,(uint32)va);
					}

					process_array[index].num_of_used_pages = num_of_pages;
				}

				return virtual_address;
			}
		}
	}

	return NULL;
//	panic("krealloc() is not implemented yet...!!");
}
