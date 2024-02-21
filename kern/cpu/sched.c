#include "sched.h"

#include <inc/assert.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/cmd/command_prompt.h>

#include <inc/fixed_point.h>

uint32 isSchedMethodRR(){if(scheduler_method == SCH_RR) return 1; return 0;}
uint32 isSchedMethodMLFQ(){if(scheduler_method == SCH_MLFQ) return 1; return 0;}
uint32 isSchedMethodBSD(){if(scheduler_method == SCH_BSD) return 1; return 0;}

//===================================================================================//
//============================ SCHEDULER FUNCTIONS ==================================//
//===================================================================================//

//===================================
// [1] Default Scheduler Initializer:
//===================================
void sched_init()
{
	old_pf_counter = 0;

	sched_init_RR(INIT_QUANTUM_IN_MS);

	init_queue(&env_new_queue);
	init_queue(&env_exit_queue);
	scheduler_status = SCH_STOPPED;
}

//=========================
// [2] Main FOS Scheduler:
//=========================
void
fos_scheduler(void)
{
	//	cprintf("inside scheduler\n");

	chk1();
	scheduler_status = SCH_STARTED;

	//This variable should be set to the next environment to be run (if any)
	struct Env* next_env = NULL;

	if (scheduler_method == SCH_RR)
	{
		// Implement simple round-robin scheduling.
		// Pick next environment from the ready queue,
		// and switch to such environment if found.
		// It's OK to choose the previously running env if no other env
		// is runnable.

		//If the curenv is still exist, then insert it again in the ready queue
		if (curenv != NULL)
		{
			enqueue(&(env_ready_queues[0]), curenv);
		}

		//Pick the next environment from the ready queue
		next_env = dequeue(&(env_ready_queues[0]));

		//Reset the quantum
		//2017: Reset the value of CNT0 for the next clock interval
		kclock_set_quantum(quantums[0]);
		//uint16 cnt0 = kclock_read_cnt0_latch() ;
		//cprintf("CLOCK INTERRUPT AFTER RESET: Counter0 Value = %d\n", cnt0 );

	}
	else if (scheduler_method == SCH_MLFQ)
	{
		next_env = fos_scheduler_MLFQ();
	}
	else if (scheduler_method == SCH_BSD)
	{
		next_env = fos_scheduler_BSD();
	}
	//temporarily set the curenv by the next env JUST for checking the scheduler
	//Then: reset it again
	struct Env* old_curenv = curenv;
	curenv = next_env ;
	chk2(next_env) ;
	curenv = old_curenv;

	//sched_print_all();

	if(next_env != NULL)
	{
		//		cprintf("\nScheduler select program '%s' [%d]... counter = %d\n", next_env->prog_name, next_env->env_id, kclock_read_cnt0());
		//		cprintf("Q0 = %d, Q1 = %d, Q2 = %d, Q3 = %d\n", queue_size(&(env_ready_queues[0])), queue_size(&(env_ready_queues[1])), queue_size(&(env_ready_queues[2])), queue_size(&(env_ready_queues[3])));
		env_run(next_env);
	}
	else
	{
		/*2015*///No more envs... curenv doesn't exist any more! return back to command prompt
		curenv = NULL;
		//lcr3(K_PHYSICAL_ADDRESS(ptr_page_directory));
		lcr3(phys_page_directory);

		//cprintf("SP = %x\n", read_esp());

		scheduler_status = SCH_STOPPED;
		//cprintf("[sched] no envs - nothing more to do!\n");
		while (1)
			run_command_prompt(NULL);

	}
}

//=============================
// [3] Initialize RR Scheduler:
//=============================
void sched_init_RR(uint8 quantum)
{

	// Create 1 ready queue for the RR
	num_of_ready_queues = 1;
#if USE_KHEAP
	sched_delete_ready_queues();
	env_ready_queues = kmalloc(sizeof(struct Env_Queue));
	quantums = kmalloc(num_of_ready_queues * sizeof(uint8)) ;
#endif
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);
	init_queue(&(env_ready_queues[0]));

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_RR;
	//=========================================
	//=========================================
}

//===============================
// [4] Initialize MLFQ Scheduler:
//===============================
void sched_init_MLFQ(uint8 numOfLevels, uint8 *quantumOfEachLevel)
{
#if USE_KHEAP
	//=========================================
	//DON'T CHANGE THESE LINES=================
	sched_delete_ready_queues();
	//=========================================
	//=========================================

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_MLFQ;
	//=========================================
	//=========================================
#endif
}

//===============================
// [5] Initialize BSD Scheduler:
//===============================
void sched_init_BSD(uint8 numOfLevels, uint8 quantum)
{
#if USE_KHEAP
	//TODO: [PROJECT'23.MS3 - #4] [2] BSD SCHEDULER - sched_init_BSD
	//Your code is here
	//Comment the following line

	num_of_ready_queues =numOfLevels ;
	sched_delete_ready_queues();
	env_ready_queues = kmalloc(sizeof(struct Env_Queue)*(numOfLevels));
	quantums = kmalloc(sizeof(uint8)) ;
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);
	for(int i = 0; i<num_of_ready_queues;i++)
	{
		init_queue(&(env_ready_queues[i]));
	}
	required_ticks = fix_round(fix_div(fix_int(1000), fix_int(quantum)));

	load_average = 0;

//   PRI_MAX = numOfLevels-1;


	//panic("Not implemented yet");

	//=========================================
	//DON'T CHANGE THESE LINES=================
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_BSD;
	//=========================================
	//=========================================
#endif
}



//=========================
// [6] MLFQ Scheduler:
//=========================
struct Env* fos_scheduler_MLFQ()
{
	panic("not implemented");
	return NULL;
}

//=========================
// [7] BSD Scheduler:
//=========================
struct Env* fos_scheduler_BSD()
{
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - fos_scheduler_BSD
	//Your code is here
	//Comment the following line
	//panic("Not implemented yet");
	//struct Env_Queue *iterator;

	//check on running process
	if(curenv != NULL)
	{
		int index = (num_of_ready_queues - 1) - curenv->priority ;
		curenv->env_status = ENV_READY;

		if(index > (num_of_ready_queues - 1))
			index = (num_of_ready_queues - 1);
		else if(index < PRI_MIN)
			index = PRI_MIN;

		enqueue(&env_ready_queues[index],curenv);
	}

	for(uint8 i = 0 ; i < num_of_ready_queues ; i++)
	{
		if(queue_size(&env_ready_queues[i])!=0)
		{
			struct Env *top_env = dequeue(&env_ready_queues[i]);

			if(top_env != NULL){
				kclock_set_quantum(quantums[0]);
				return top_env;
			}

		}
	}

	load_average = 0;
	return NULL;
}

//========================================
// [8] Clock Interrupt Handler
//	  (Automatically Called Every Quantum)
//========================================
void clock_interrupt_handler()
{
	//TODO: [PROJECT'23.MS3 - #5] [2] BSD SCHEDULER - Your code is here
	    if(isSchedMethodBSD())
		{
//	    	cprintf("clock_interrupt_handler \n");
			int64 num_of_ticks = timer_ticks() + 1;
			struct Env* env = curenv;
	//		int milliseconds = num_of_ticks * quantums[env->priority];
			int milliseconds = num_of_ticks * quantums[0];

			//update recent_cpu of running process
			env->env_recent_cpu ++;
			//the second
			if(num_of_ticks%required_ticks == 0)
			{
				// update load_average (𝑙𝑜𝑎𝑑_𝑎𝑣𝑔 (𝑡) =  59/60×𝑙𝑜𝑎𝑑_𝑎𝑣𝑔 (𝑡−1) + 1/60×𝑟𝑒𝑎𝑑𝑦_𝑝𝑟𝑜𝑐𝑒𝑠𝑠𝑒𝑠(t) )
				//load avg updated once per second
				fixed_point_t old_load_avg = fix_int(load_average);
				//how to add run process ? (add 1 ?) & is this correct for all priority queues ?
				int ready_processes;
				if(curenv != NULL)
				{ ready_processes =  1;}
				else
			    {ready_processes =  0;	}

				for(int i = PRI_MIN; i < num_of_ready_queues; i++)
				{
					ready_processes += queue_size(&env_ready_queues[i]);
				}
				fixed_point_t temp_load_avg = fix_add(fix_mul(fix_unscale(fix_int(59),60),old_load_avg) , fix_mul(fix_unscale(fix_int(1),60),fix_int(ready_processes)));
				load_average = fix_round(temp_load_avg);


				// update recent_cpu for all processes -> recent_cpu (t) = ( (2*load_avg)/ ((2*load_avg)+1) ) * recent_cpu (t-1) + nice
				//updated on each timer tick for running process & once per second for every process
				//how to get all processes to update recent_cpu ?
				for(uint8 i = 0; i < num_of_ready_queues; i++)
				{

					struct Env_Queue iterator_queue = env_ready_queues[i];
					struct Env* iterator = NULL;
					LIST_FOREACH(iterator,&iterator_queue)
					{
						if(iterator!=NULL)
						{
						fixed_point_t load_avg = fix_int(load_average);
						fixed_point_t recent_fraction = fix_div(fix_scale(load_avg,2) , fix_add(fix_scale(load_avg,2),fix_int(1)));
						fixed_point_t old_recent = fix_int(iterator->env_recent_cpu); // recent (t-1)

						fixed_point_t recent_temp = fix_add(fix_mul(recent_fraction , old_recent) , fix_int(iterator->env_nice));
						iterator->env_recent_cpu = fix_round(recent_temp) ;
						}
					}
				}

				if(curenv != NULL)
				{
				fixed_point_t load_avg = fix_int(load_average);
				fixed_point_t recent_fraction = fix_div(fix_scale(load_avg,2) , fix_add(fix_scale(load_avg,2),fix_int(1)));
				fixed_point_t old_recent = fix_int(curenv->env_recent_cpu); // recent (t-1)

				fixed_point_t recent_temp = fix_add(fix_mul(recent_fraction , old_recent) , fix_int(curenv->env_nice));
				curenv->env_recent_cpu = fix_round(recent_temp) ;
				}
			}

			//update priority each 4 ticks
			// priority = PRI_MAX - (𝑟𝑒𝑐𝑒𝑛𝑡_𝑐𝑝𝑢  / 4) - (nice × 2)
			if(num_of_ticks % 4 == 0)
			{
				for(uint8 i = 0; i < num_of_ready_queues; i++)
				{
					struct Env_Queue iterator_queue = env_ready_queues[i];
					struct Env* iterator = NULL;
					LIST_FOREACH(iterator,&iterator_queue)
					{
						if(iterator != NULL)
						{	fixed_point_t pri_temp = fix_sub( fix_sub( fix_int((int)(num_of_ready_queues - 1)) , fix_unscale(fix_int(iterator->env_recent_cpu),4) ), fix_scale(fix_int(iterator->env_nice),2) );
							int temp = fix_round(pri_temp);
							if(temp > (num_of_ready_queues-1))
								temp = num_of_ready_queues-1;
							else if(temp < PRI_MIN)
								temp = PRI_MIN;

							iterator->priority = temp;

						}
					}
				}
				if(curenv != NULL)
				{
				    fixed_point_t pri_temp = fix_sub( fix_sub( fix_int((int)(num_of_ready_queues - 1)) , fix_unscale(fix_int(curenv->env_recent_cpu),4) ), fix_scale(fix_int(curenv->env_nice),2) );
					int temp = fix_round(pri_temp);
					if(temp > (num_of_ready_queues-1))
						temp = num_of_ready_queues-1;
					else if(temp < PRI_MIN)
						temp = PRI_MIN;

					curenv->priority = temp;

					//update its place in queues
//					int index = (num_of_ready_queues - 1) - curenv->priority ;
//					cprintf("enque curenv whwn updating priority \n");
//					enqueue(&env_ready_queues[index],curenv);
				}

				//update places by priority in queues
				for(uint8 i = 0; i < num_of_ready_queues; i++)
				{
					struct Env_Queue iterator_queue = env_ready_queues[i];
					struct Env* iterator = NULL;
					int size_of_each_queu = queue_size(&env_ready_queues[i]);

					LIST_FOREACH(iterator,&env_ready_queues[i])
					{
						int count =0;

						int index = (num_of_ready_queues - 1) - iterator->priority ;

						if(count == size_of_each_queu){
							break;
						}

						LIST_REMOVE(&env_ready_queues[i],iterator);
						enqueue(&env_ready_queues[index],iterator);

						count++;
					}

			   }
			}
		}


	/********DON'T CHANGE THIS LINE***********/
	ticks++ ;
	if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX))
	{
		update_WS_time_stamps();
	}
	//cprintf("Clock Handler\n") ;
	fos_scheduler();
	/*****************************************/
}

//===================================================================
// [9] Update LRU Timestamp of WS Elements
//	  (Automatically Called Every Quantum in case of LRU Time Approx)
//===================================================================
void update_WS_time_stamps()
{
	struct Env *curr_env_ptr = curenv;

	if(curr_env_ptr != NULL)
	{
		struct WorkingSetElement* wse ;
		{
			int i ;
#if USE_KHEAP
			LIST_FOREACH(wse, &(curr_env_ptr->page_WS_list))
			{
#else
			for (i = 0 ; i < (curr_env_ptr->page_WS_max_size); i++)
			{
				wse = &(curr_env_ptr->ptr_pageWorkingSet[i]);
				if( wse->empty == 1)
					continue;
#endif
				//update the time if the page was referenced
				uint32 page_va = wse->virtual_address ;
				uint32 perm = pt_get_page_permissions(curr_env_ptr->env_page_directory, page_va) ;
				uint32 oldTimeStamp = wse->time_stamp;

				if (perm & PERM_USED)
				{
					wse->time_stamp = (oldTimeStamp>>2) | 0x80000000;
					pt_set_page_permissions(curr_env_ptr->env_page_directory, page_va, 0 , PERM_USED) ;
				}
				else
				{
					wse->time_stamp = (oldTimeStamp>>2);
				}
			}
		}

		{
			int t ;
			for (t = 0 ; t < __TWS_MAX_SIZE; t++)
			{
				if( curr_env_ptr->__ptr_tws[t].empty != 1)
				{
					//update the time if the page was referenced
					uint32 table_va = curr_env_ptr->__ptr_tws[t].virtual_address;
					uint32 oldTimeStamp = curr_env_ptr->__ptr_tws[t].time_stamp;

					if (pd_is_table_used(curr_env_ptr->env_page_directory, table_va))
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2) | 0x80000000;
						pd_set_table_unused(curr_env_ptr->env_page_directory, table_va);
					}
					else
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2);
					}
				}
			}
		}
	}
}

