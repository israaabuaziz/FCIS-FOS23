/*
 * fault_handler.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */

#include "trap.h"
#include <kern/proc/user_environment.h>
#include "../cpu/sched.h"
#include "../disk/pagefile_manager.h"
#include "../mem/memory_manager.h"
#include "../mem/kheap.h"

//2014 Test Free(): Set it to bypass the PAGE FAULT on an instruction with this length and continue executing the next one
// 0 means don't bypass the PAGE FAULT
uint8 bypassInstrLength = 0;

//===============================
// REPLACEMENT STRATEGIES
//===============================
//2020
void setPageReplacmentAlgorithmLRU(int LRU_TYPE)
{
	assert(LRU_TYPE == PG_REP_LRU_TIME_APPROX || LRU_TYPE == PG_REP_LRU_LISTS_APPROX);
	_PageRepAlgoType = LRU_TYPE ;
}
void setPageReplacmentAlgorithmCLOCK(){_PageRepAlgoType = PG_REP_CLOCK;}
void setPageReplacmentAlgorithmFIFO(){_PageRepAlgoType = PG_REP_FIFO;}
void setPageReplacmentAlgorithmModifiedCLOCK(){_PageRepAlgoType = PG_REP_MODIFIEDCLOCK;}
/*2018*/ void setPageReplacmentAlgorithmDynamicLocal(){_PageRepAlgoType = PG_REP_DYNAMIC_LOCAL;}
/*2021*/ void setPageReplacmentAlgorithmNchanceCLOCK(int PageWSMaxSweeps){_PageRepAlgoType = PG_REP_NchanceCLOCK;  page_WS_max_sweeps = PageWSMaxSweeps;}

//2020
uint32 isPageReplacmentAlgorithmLRU(int LRU_TYPE){return _PageRepAlgoType == LRU_TYPE ? 1 : 0;}
uint32 isPageReplacmentAlgorithmCLOCK(){if(_PageRepAlgoType == PG_REP_CLOCK) return 1; return 0;}
uint32 isPageReplacmentAlgorithmFIFO(){if(_PageRepAlgoType == PG_REP_FIFO) return 1; return 0;}
uint32 isPageReplacmentAlgorithmModifiedCLOCK(){if(_PageRepAlgoType == PG_REP_MODIFIEDCLOCK) return 1; return 0;}
/*2018*/ uint32 isPageReplacmentAlgorithmDynamicLocal(){if(_PageRepAlgoType == PG_REP_DYNAMIC_LOCAL) return 1; return 0;}
/*2021*/ uint32 isPageReplacmentAlgorithmNchanceCLOCK(){if(_PageRepAlgoType == PG_REP_NchanceCLOCK) return 1; return 0;}

//===============================
// PAGE BUFFERING
//===============================
void enableModifiedBuffer(uint32 enableIt){_EnableModifiedBuffer = enableIt;}
uint8 isModifiedBufferEnabled(){  return _EnableModifiedBuffer ; }

void enableBuffering(uint32 enableIt){_EnableBuffering = enableIt;}
uint8 isBufferingEnabled(){  return _EnableBuffering ; }

void setModifiedBufferLength(uint32 length) { _ModifiedBufferLength = length;}
uint32 getModifiedBufferLength() { return _ModifiedBufferLength;}

//===============================
// FAULT HANDLERS
//===============================

//Handle the table fault
void table_fault_handler(struct Env * curenv, uint32 fault_va)
{
	//panic("table_fault_handler() is not implemented yet...!!");
	//Check if it's a stack page
	uint32* ptr_table;
#if USE_KHEAP
	{
		ptr_table = create_page_table(curenv->env_page_directory, (uint32)fault_va);
	}
#else
	{
		__static_cpt(curenv->env_page_directory, (uint32)fault_va, &ptr_table);
	}
#endif
}

//Handle the page fault

void page_fault_handler(struct Env * curenv, uint32 fault_va)
{
#if USE_KHEAP
		struct WorkingSetElement *victimWSElement = NULL;
		uint32 wsSize = LIST_SIZE(&(curenv->page_WS_list));
#else
		int iWS =curenv->page_last_WS_index;
		uint32 wsSize = env_page_ws_get_size(curenv);
#endif

	//cprintf("REPLACEMENT=========================WS Size = %d\n", wsSize );
	//refer to the project presentation and documentation for details
	if(isPageReplacmentAlgorithmFIFO())
	{
		//TODO: [PROJECT'23.MS3 - #1] [1] PAGE FAULT HANDLER - FIFO Replacement
		// Write your code here, remove the panic and write your code
	//			panic("page_fault_handler() FIFO Replacement is not implemented yet...!!");

		if(wsSize < (curenv->page_WS_max_size))
		{
			//cprintf("PLACEMENT=========================WS Size = %d\n", wsSize );
			//TODO: [PROJECT'23.MS2 - #15] [3] PAGE FAULT HANDLER - Placement
			// Write your code here, remove the panic and write your code
	//		panic("page_fault_handler().PLACEMENT is not implemented yet...!!");

			struct FrameInfo *place_ptr=NULL;
			int result=allocate_frame(&place_ptr);
			if(result !=E_NO_MEM){
				map_frame(curenv->env_page_directory,place_ptr,fault_va,PERM_USER|PERM_WRITEABLE);
			}
			else{
				cprintf("NO_MEM");
			}
			  //step_2
			//        struct Env* ptr_env;
			int ret = pf_read_env_page(curenv,(void *)fault_va);
			if(ret ==E_PAGE_NOT_EXIST_IN_PF){
				if((fault_va>=USTACKBOTTOM && fault_va<USTACKTOP) || (fault_va>=USER_HEAP_START && fault_va<USER_HEAP_MAX))
				{

				}
				else{//big else

					cprintf("here_4");
				//		    	cprintf("Faulted at: %x\n", fault_va);
					sched_kill_env(curenv->env_id);

				}//big else

			 }// if ret
			  struct WorkingSetElement *add_element=env_page_ws_list_create_element(curenv,fault_va);
			  if (add_element==NULL)
				  sched_kill_env(curenv->env_id);
			  LIST_INSERT_TAIL(&curenv->page_WS_list,add_element); //add

			  if(LIST_SIZE(&curenv->page_WS_list)<curenv->page_WS_max_size)//update
				  curenv->page_last_WS_element=NULL;
			  else{
				curenv->page_last_WS_element=LIST_FIRST(&curenv->page_WS_list);
			  }

			//refer to the project presentation and documentation for details
		}
		else
		{  //replace
//			cprintf ("replace\n");
			struct FrameInfo *place_ptr=NULL;
			int result=allocate_frame(&place_ptr);
			if(result !=E_NO_MEM){
				map_frame(curenv->env_page_directory,place_ptr,fault_va,PERM_USER|PERM_WRITEABLE);
			}
			else{
				cprintf("NO_MEM");
			}
			//step_2
			int ret = pf_read_env_page(curenv,(void *)fault_va);
			if(ret ==E_PAGE_NOT_EXIST_IN_PF){
				if((fault_va>=USTACKBOTTOM && fault_va<USTACKTOP) || (fault_va>=USER_HEAP_START && fault_va<USER_HEAP_MAX))
				{


				}
				else{//big else

					cprintf("here_4");
//					cprintf("Faulted at: %x\n", fault_va);
					sched_kill_env(curenv->env_id);

				}//big else

			}
			uint32 perms = pt_get_page_permissions(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address);
			if (perms & PERM_MODIFIED)

			{
//				cprintf ("prems\n");
			uint32* ptr_page_table ;
			struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address,&ptr_page_table);
			pf_update_env_page(curenv, curenv->page_last_WS_element->virtual_address, ptr_frame_info);
			}

			unmap_frame(curenv->env_page_directory,curenv->page_last_WS_element->virtual_address);

			env_page_ws_invalidate(curenv ,curenv->page_last_WS_element->virtual_address);// remove

			struct WorkingSetElement *add_element=env_page_ws_list_create_element(curenv,fault_va);

			LIST_INSERT_TAIL(&curenv->page_WS_list,add_element);

//			if (curenv->page_last_WS_element!=NULL)
//			{
//				LIST_INSERT_BEFORE(&curenv->page_WS_list,curenv->page_last_WS_element,add_element);
//			}
//			else
//			{
//				LIST_INSERT_TAIL(&curenv->page_WS_list,add_element);
//				curenv->page_last_WS_element=LIST_FIRST(&curenv->page_WS_list);
//			}
		}
	}

	if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_LISTS_APPROX))
	{
		//TODO: [PROJECT'23.MS3 - #2] [1] PAGE FAULT HANDLER - LRU Replacement
		// Write your code here, remove the panic and write your code
		//panic("page_fault_handler() LRU Replacement is not implemented yet...!!");

	/*     uint32 current_sizeOfActiveList=LIST_SIZE(&curenv->ActiveList);
			uint32 current_sizeOfSecondList=LIST_SIZE(&curenv->SecondList);
				//placement
			if((current_sizeOfActiveList+current_sizeOfSecondList)<(curenv->ActiveListSize+curenv->SecondListSize))
			{
			  if(current_sizeOfActiveList < curenv->ActiveListSize)
			  {
				 struct FrameInfo *placeLru_ptr=NULL;
				 int result=allocate_frame(&placeLru_ptr);
				 if(result !=E_NO_MEM)
				 {
					 map_frame(curenv->env_page_directory,placeLru_ptr,fault_va,(PERM_PRESENT | PERM_USER | PERM_WRITEABLE));
					 int ret = pf_read_env_page(curenv,(void *)fault_va);
					 if(ret ==E_PAGE_NOT_EXIST_IN_PF)
					 {
					 if((fault_va>=USTACKBOTTOM && fault_va<USTACKTOP) || (fault_va>=USER_HEAP_START && fault_va<USER_HEAP_MAX))
					   {
					   }
					 else{//big else
						   cprintf("here_5\n");
						   cprintf("Faulted at: %x\n", fault_va);
						   sched_kill_env(curenv->env_id);
							}//big else
					  }

				  }// if ret
				struct WorkingSetElement *add_element_inActiveList=env_page_ws_list_create_element(curenv,fault_va);

				LIST_INSERT_HEAD(&curenv->ActiveList,add_element_inActiveList);
				pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);
	//==================o(1)
			  placeLru_ptr->element = add_element_inActiveList;


			  }//condition_1

			  else if((current_sizeOfActiveList == curenv->ActiveListSize) &&
					   (current_sizeOfSecondList < curenv->SecondListSize))
			  {
				  //define pointer to compare
					struct WorkingSetElement *ptr_search=NULL;
					struct WorkingSetElement *temp=NULL;
					struct WorkingSetElement *tail_ActiveList=NULL;
					LIST_FOREACH(ptr_search,&(curenv->SecondList))
					{
					  if((ROUNDDOWN(ptr_search->virtual_address,PAGE_SIZE)==ROUNDDOWN(fault_va,PAGE_SIZE)))
					  {
						  //if page found
						tail_ActiveList=LIST_LAST(&curenv->ActiveList);
						LIST_REMOVE(&(curenv->ActiveList),tail_ActiveList);
						//insert tail in second list
						LIST_INSERT_HEAD(&curenv->SecondList,tail_ActiveList);
						pt_set_page_permissions(curenv->env_page_directory,tail_ActiveList->virtual_address,0,PERM_PRESENT);
						//insert page in active list
						LIST_REMOVE(&(curenv->SecondList),ptr_search);
						LIST_INSERT_HEAD(&curenv->ActiveList,ptr_search);
						pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);//present=1
						return;
					  }//Condition_2
					}
						  //if page not found
						  tail_ActiveList=LIST_LAST(&curenv->ActiveList);
						  LIST_REMOVE(&(curenv->ActiveList),tail_ActiveList);
						  //insert tail in second list
						  LIST_INSERT_HEAD(&curenv->SecondList,tail_ActiveList);
						  pt_set_page_permissions(curenv->env_page_directory,tail_ActiveList->virtual_address,0,PERM_PRESENT);//present=0
						 //insert new page in active list
						 struct FrameInfo *placeLru_ptr=NULL;
						int result=allocate_frame(&placeLru_ptr);
						if(result !=E_NO_MEM)
						{
						   map_frame(curenv->env_page_directory,placeLru_ptr,fault_va,(PERM_PRESENT | PERM_USER | PERM_WRITEABLE));
						   int ret = pf_read_env_page(curenv,(void *)fault_va);
						   if(ret ==E_PAGE_NOT_EXIST_IN_PF)
						   {
							  if((fault_va>=USTACKBOTTOM && fault_va<USTACKTOP) || (fault_va>=USER_HEAP_START && fault_va<USER_HEAP_MAX))
							   {
							   }
							  else
							   {//big else
									 cprintf("here_7\n");
									 cprintf("Faulted at: %x\n", fault_va);
									  sched_kill_env(curenv->env_id);
								}//big else

						   }// if ret
						}

				   struct WorkingSetElement *add_element_inActiveList=env_page_ws_list_create_element(curenv,fault_va);
				   LIST_INSERT_HEAD(&curenv->ActiveList,add_element_inActiveList);
					pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);//present=1
	//==================o(1)
			  placeLru_ptr->element = add_element_inActiveList;

					return;
			  }//condition_2&3

			}//end placement lru      */

		int count = 0;
		if(count == 0){
			struct WorkingSetElement* ptr_search = NULL;
			LIST_FOREACH(ptr_search,&(curenv->ActiveList)){
				uint32 * ptr_page_table =NULL;
				struct FrameInfo* ptr_frame_info = get_frame_info(curenv->env_page_directory, ptr_search->virtual_address, &ptr_page_table);
				ptr_frame_info->element = ptr_search;
//				cprintf("Aelement : %x\n",ptr_frame_info->element);
			}

			ptr_search = NULL;
			LIST_FOREACH(ptr_search,&(curenv->SecondList)){
				uint32 * ptr_page_table =NULL;
				struct FrameInfo* ptr_frame_info = get_frame_info(curenv->env_page_directory, ptr_search->virtual_address, &ptr_page_table);
				ptr_frame_info->element = ptr_search;
//				cprintf("Selement : %x\n",ptr_frame_info->element);
			}

			count++;
		}

		uint32 current_sizeOfActiveList=LIST_SIZE(&curenv->ActiveList);
		uint32 current_sizeOfSecondList=LIST_SIZE(&curenv->SecondList);
		//placement
		if((current_sizeOfActiveList+current_sizeOfSecondList)<(curenv->ActiveListSize+curenv->SecondListSize))
		{
			if(current_sizeOfActiveList < curenv->ActiveListSize){
				 struct FrameInfo *placeLru_ptr=NULL;
				 int result=allocate_frame(&placeLru_ptr);
				 if(result !=E_NO_MEM){
					 map_frame(curenv->env_page_directory,placeLru_ptr,fault_va,(PERM_PRESENT | PERM_USER | PERM_WRITEABLE));
					 int ret = pf_read_env_page(curenv,(void *)fault_va);
					 if(ret ==E_PAGE_NOT_EXIST_IN_PF){
						if((fault_va>=USTACKBOTTOM && fault_va<USTACKTOP) || (fault_va>=USER_HEAP_START && fault_va<USER_HEAP_MAX))
						{}
						else{//big else
						   cprintf("here_5");
//						   cprintf("Faulted at: %x\n", fault_va);
						   sched_kill_env(curenv->env_id);
						}//big else
					 }

				  }// if ret
				  struct WorkingSetElement *add_element_inActiveList=env_page_ws_list_create_element(curenv,fault_va);

				  LIST_INSERT_HEAD(&curenv->ActiveList,add_element_inActiveList);
				  pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);
//---------------------------- o(1)
				  placeLru_ptr->element = add_element_inActiveList;
//				  cprintf("placeLru_ptr->element : %x\n",placeLru_ptr->element);

			}//condition_1
			else if((current_sizeOfActiveList == curenv->ActiveListSize) &&
					   (current_sizeOfSecondList < curenv->SecondListSize)){
//------------------------------------ o(1) ----------------------------
//					env_page_ws_print(curenv);
					struct WorkingSetElement *tail_ActiveList=NULL;
					uint32 *ptr_page_table = NULL;
					struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory, fault_va, &ptr_page_table);

					if(ptr_frame_info != NULL){

//						if(ptr_frame_info->element == NULL){cprintf("null\n");}
						if(ptr_frame_info->element != NULL){
							if((ROUNDDOWN(ptr_frame_info->element->virtual_address,PAGE_SIZE)==ROUNDDOWN(fault_va,PAGE_SIZE))){
								uint32 page_permission = pt_get_page_permissions(curenv->env_page_directory, ptr_frame_info->element->virtual_address);

//								cprintf("found_element : %x\n",ptr_frame_info->element);
								if(page_permission & PERM_PRESENT){
//									  cprintf("1\n");
									LIST_REMOVE(&(curenv->ActiveList),ptr_frame_info->element);
									LIST_INSERT_HEAD(&curenv->ActiveList,ptr_frame_info->element);
									//present=1
									pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);
									return;
								}
								else{
//									cprintf("2\n");
									tail_ActiveList=LIST_LAST(&curenv->ActiveList);
									LIST_REMOVE(&(curenv->ActiveList),tail_ActiveList);
									//insert tail in second list
									LIST_INSERT_HEAD(&curenv->SecondList,tail_ActiveList);
									pt_set_page_permissions(curenv->env_page_directory,tail_ActiveList->virtual_address,0,PERM_PRESENT);
									//insert page in active list
									LIST_REMOVE(&(curenv->SecondList),ptr_frame_info->element);
									LIST_INSERT_HEAD(&curenv->ActiveList,ptr_frame_info->element);
									pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);//present=1
									return;
								}
							}
						}
					}
//					env_page_ws_print(curenv);


					//if page not found
					tail_ActiveList=LIST_LAST(&curenv->ActiveList);
					LIST_REMOVE(&(curenv->ActiveList),tail_ActiveList);
					//insert tail in second list
					LIST_INSERT_HEAD(&curenv->SecondList,tail_ActiveList);
					pt_set_page_permissions(curenv->env_page_directory,tail_ActiveList->virtual_address,0,PERM_PRESENT);//present=0
// -------------------------- o(1) -------------------------
					ptr_page_table =NULL;
					ptr_frame_info = get_frame_info(curenv->env_page_directory, tail_ActiveList->virtual_address, &ptr_page_table);
					ptr_frame_info->element = tail_ActiveList;
					pt_set_page_permissions(curenv->env_page_directory,ptr_frame_info->element->virtual_address,0,PERM_PRESENT);//present=0
//					cprintf("replace_element : %x\n",ptr_frame_info->element);

					//insert new page in active list
					struct FrameInfo *placeLru_ptr=NULL;
					int result=allocate_frame(&placeLru_ptr);
					if(result !=E_NO_MEM){
						map_frame(curenv->env_page_directory,placeLru_ptr,fault_va,(PERM_PRESENT | PERM_USER | PERM_WRITEABLE));
						int ret = pf_read_env_page(curenv,(void *)fault_va);
						if(ret ==E_PAGE_NOT_EXIST_IN_PF){
							if((fault_va>=USTACKBOTTOM && fault_va<USTACKTOP) || (fault_va>=USER_HEAP_START && fault_va<USER_HEAP_MAX))
							{}
							else{//big else
								cprintf("here_7");
//								cprintf("Faulted at: %x\n", fault_va);
								sched_kill_env(curenv->env_id);
							}//big else

						}// if ret
					}


				   struct WorkingSetElement *add_element_inActiveList=env_page_ws_list_create_element(curenv,fault_va);
//				   tail_ActiveList=LIST_LAST(&curenv->ActiveList);
//				   LIST_REMOVE(&curenv->ActiveList,tail_ActiveList);
//				   LIST_INSERT_HEAD(&curenv->SecondList,tail_ActiveList);
//				   pt_set_page_permissions(curenv->env_page_directory,tail_ActiveList->virtual_address,0,PERM_PRESENT);
				   LIST_INSERT_HEAD(&curenv->ActiveList,add_element_inActiveList);
				   pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);//present=1
//---------------------------- o(1)
				   placeLru_ptr->element = add_element_inActiveList;
//					cprintf("placeLru_ptr->element : %x\n",placeLru_ptr->element);

				   return;


					//foreach


			}//condition_2&3

		}//end placement lru
		else{
//			cprintf("in lru replacement \n");
//			struct WorkingSetElement *ptr_search = NULL;
//			struct WorkingSetElement *temp = NULL;
//			struct WorkingSetElement *tail_ActiveList = NULL;
//			struct WorkingSetElement *tail_SecondList = NULL;
//
//			//checks if the element is already exists in the second list
//			LIST_FOREACH(ptr_search,&(curenv->SecondList)){
//				if((ROUNDDOWN(ptr_search->virtual_address,PAGE_SIZE) == ROUNDDOWN(fault_va,PAGE_SIZE))){
//					//if page found
//					tail_ActiveList=LIST_LAST(&curenv->ActiveList);
//					LIST_REMOVE(&(curenv->ActiveList),tail_ActiveList);
//
//					//insert tail in second list
//					LIST_INSERT_HEAD(&curenv->SecondList,tail_ActiveList);
//					pt_set_page_permissions(curenv->env_page_directory,tail_ActiveList->virtual_address,0,PERM_PRESENT);
//
//					//insert page in active list
//					LIST_REMOVE(&(curenv->SecondList),ptr_search);
//					LIST_INSERT_HEAD(&curenv->ActiveList,ptr_search);
//					pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);//present=1
//					return;
//				}
//			}
//
//			//checks if the element is already exists in the active list
//			ptr_search = NULL;
//			LIST_FOREACH(ptr_search,&(curenv->ActiveList)){
//				if((ROUNDDOWN(ptr_search->virtual_address,PAGE_SIZE) == ROUNDDOWN(fault_va,PAGE_SIZE))){
//					//if page found
//					LIST_REMOVE(&(curenv->ActiveList),ptr_search);
//					LIST_INSERT_HEAD(&curenv->ActiveList,ptr_search);
//					pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);//present=1
//					return;
//				}
//			}
//
//			//if the added page is new
//
//			//delete the tail of second list (victim) & if modified store it in disk
//			tail_SecondList = LIST_LAST(&curenv->SecondList);
//			uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory, tail_SecondList->virtual_address);
//			if(page_permissions & PERM_MODIFIED)
//			{
//				uint32* ptr_page_table = NULL;
//				struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory,tail_SecondList->virtual_address,&ptr_page_table);
//				pf_update_env_page(curenv, tail_SecondList->virtual_address, ptr_frame_info);
//			}
//			env_page_ws_invalidate(curenv,tail_SecondList->virtual_address);
//
//
//			//swap the tail of active list to the head of second list (p=0)
//			tail_ActiveList = LIST_LAST(&curenv->ActiveList);
//			LIST_REMOVE(&(curenv->ActiveList),tail_ActiveList);
//			LIST_INSERT_HEAD(&(curenv->SecondList),tail_ActiveList);
//			pt_set_page_permissions(curenv->env_page_directory,tail_ActiveList->virtual_address,0,PERM_PRESENT);
//
//
//			struct FrameInfo *placeLru_ptr=NULL;
//			 int result=allocate_frame(&placeLru_ptr);
//			 if(result !=E_NO_MEM){
//				 map_frame(curenv->env_page_directory,placeLru_ptr,fault_va,(PERM_PRESENT | PERM_USER | PERM_WRITEABLE));
//				 int ret = pf_read_env_page(curenv,(void *)fault_va);
//				 if(ret ==E_PAGE_NOT_EXIST_IN_PF){
//					if((fault_va>=USTACKBOTTOM && fault_va<USTACKTOP) || (fault_va>=USER_HEAP_START && fault_va<USER_HEAP_MAX))
//					{}
//					else{//big else
//					   cprintf("here_5");
////					   cprintf("Faulted at: %x\n", fault_va);
//					   sched_kill_env(curenv->env_id);
//					}//big else
//				 }
//
//			  }// if ret
//
//			//add the new element (p=1)
//			struct WorkingSetElement *add_element_inActiveList=env_page_ws_list_create_element(curenv,fault_va);
//			LIST_INSERT_HEAD(&(curenv->ActiveList),add_element_inActiveList);
//			pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);

//--------------------start -------------

		//TODO: [PROJECT'23.MS3 - BONUS] [1] PAGE FAULT HANDLER - O(1) implementation of LRU replacement

//			cprintf("curenv_element : %x\n",fault_va);
//			env_page_ws_print(curenv);


//			if(count == 0){
//				struct WorkingSetElement* ptr_search = NULL;
//				LIST_FOREACH(ptr_search,&(curenv->ActiveList)){
//					uint32 * ptr_page_table =NULL;
//					struct FrameInfo* ptr_frame_info = get_frame_info(curenv->env_page_directory, ptr_search->virtual_address, &ptr_page_table);
//					ptr_frame_info->element = ptr_search;
//	//				cprintf("Aelement : %x\n",ptr_frame_info->element);
//				}
//
//				ptr_search = NULL;
//				LIST_FOREACH(ptr_search,&(curenv->SecondList)){
//					uint32 * ptr_page_table =NULL;
//					struct FrameInfo* ptr_frame_info = get_frame_info(curenv->env_page_directory, ptr_search->virtual_address, &ptr_page_table);
//					ptr_frame_info->element = ptr_search;
//	//				cprintf("Selement : %x\n",ptr_frame_info->element);
//				}
//
//				count++;
//			}

//			env_page_ws_print(curenv);
			struct WorkingSetElement *tail_ActiveList=NULL;
			uint32 *ptr_page_table = NULL;
			struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory, fault_va, &ptr_page_table);

			if(ptr_frame_info != NULL){

				if(ptr_frame_info->element == NULL){cprintf("null\n");}
				if(ptr_frame_info->element != NULL){
					if((ROUNDDOWN(ptr_frame_info->element->virtual_address,PAGE_SIZE)==ROUNDDOWN(fault_va,PAGE_SIZE))){
						uint32 page_permission = pt_get_page_permissions(curenv->env_page_directory, ptr_frame_info->element->virtual_address);

//						cprintf("found_element : %x\n",ptr_frame_info->element);
						if(page_permission & PERM_PRESENT){
//									  cprintf("1\n");
							LIST_REMOVE(&(curenv->ActiveList),ptr_frame_info->element);
							LIST_INSERT_HEAD(&curenv->ActiveList,ptr_frame_info->element);
							//present=1
							pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);
							return;
						}
						else{
//									cprintf("2\n");
							tail_ActiveList=LIST_LAST(&curenv->ActiveList);
							LIST_REMOVE(&(curenv->ActiveList),tail_ActiveList);
							//insert tail in second list
							LIST_INSERT_HEAD(&curenv->SecondList,tail_ActiveList);
							pt_set_page_permissions(curenv->env_page_directory,tail_ActiveList->virtual_address,0,PERM_PRESENT);
							//insert page in active list
							LIST_REMOVE(&(curenv->SecondList),ptr_frame_info->element);
							LIST_INSERT_HEAD(&curenv->ActiveList,ptr_frame_info->element);
							pt_set_page_permissions(curenv->env_page_directory,fault_va,PERM_PRESENT,0);//present=1
							return;
						}
					}
				}
			}

			// Case 2: Page not found in Second List and new
//			env_page_ws_print(curenv);

			// remove victim from tail of second
			struct WorkingSetElement* Last_element_in_sec = LIST_LAST(&curenv->SecondList);
			// if modified, write it to disk
			uint32 page_permissions = pt_get_page_permissions(curenv->env_page_directory, Last_element_in_sec->virtual_address);
			if (page_permissions & PERM_MODIFIED)
			{
//				cprintf("h0.0\n");
				uint32* ptr_page_table = NULL;
				struct FrameInfo *ptr_frame_info = get_frame_info(curenv->env_page_directory, Last_element_in_sec->virtual_address, &ptr_page_table);
				pf_update_env_page(curenv, Last_element_in_sec->virtual_address, ptr_frame_info);
//				cprintf("h0.1\n");
			}
			//remove victim
//			cprintf("h1\n");
			unmap_frame(curenv->env_page_directory,Last_element_in_sec->virtual_address);
			LIST_REMOVE(&(curenv->SecondList), Last_element_in_sec);
			kfree(Last_element_in_sec);
//			cprintf("h2\n");

			//(swap) remove last element in active and insert it in head of second
			struct WorkingSetElement* Last_element_in_active = NULL;
			Last_element_in_active = LIST_LAST(&curenv->ActiveList);
			LIST_REMOVE(&(curenv->ActiveList), Last_element_in_active);
			LIST_INSERT_HEAD(&curenv->SecondList, Last_element_in_active);

			// present = 0 in second
			pt_set_page_permissions(curenv->env_page_directory, Last_element_in_active->virtual_address, 0, PERM_PRESENT);

			//update element
			ptr_page_table =NULL;
			ptr_frame_info = get_frame_info(curenv->env_page_directory, Last_element_in_active->virtual_address, &ptr_page_table);
			ptr_frame_info->element = Last_element_in_active;
			pt_set_page_permissions(curenv->env_page_directory,ptr_frame_info->element->virtual_address,0,PERM_PRESENT);//present=0


			// insert fault_va in head of active
//			cprintf("h3\n");
			struct FrameInfo *replaceLru_ptr=NULL;
			int result=allocate_frame(&replaceLru_ptr);
			if(result !=E_NO_MEM)
			{
			   map_frame(curenv->env_page_directory,replaceLru_ptr,fault_va,(PERM_PRESENT | PERM_USER | PERM_WRITEABLE));
			   int ret = pf_read_env_page(curenv,(void *)fault_va);
			   if(ret ==E_PAGE_NOT_EXIST_IN_PF)
			   {
				  if((fault_va>=USTACKBOTTOM && fault_va<USTACKTOP) || (fault_va>=USER_HEAP_START && fault_va<USER_HEAP_MAX))
				   {
				   }
				  else
				   {
						 cprintf("Faulted at: %x\n", fault_va);
						 sched_kill_env(curenv->env_id);
				   }
			   }
			}
//			cprintf("h4\n");
			//add the new element
			struct WorkingSetElement* element_added = env_page_ws_list_create_element(curenv, fault_va);
			LIST_INSERT_HEAD(&curenv->ActiveList, element_added);
//			cprintf("h5\n");
			// present = 1 in active
			pt_set_page_permissions(curenv->env_page_directory, element_added->virtual_address, PERM_PRESENT, 0);

//==================o(1)
			replaceLru_ptr->element = element_added;
//			env_page_ws_print(curenv);

//---------------------------end------------------

	}
  }

}

void __page_fault_handler_with_buffering(struct Env * curenv, uint32 fault_va)
{
	panic("this function is not required...!!");
}



