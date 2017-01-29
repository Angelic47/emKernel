/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel task.c
	Multi-task support and task manager API implementation.

*/

#include "portable/types.h"
#include "portable/machine.h"
#include "memory/memory.h"
#include "task.h"

// Current system tick & system tick overflow check
EM_UINT32 em_task_systemtick = 0;
EM_UINT8 em_task_tickoverflow = 0;

// Avaliable tasks table
EM_TASKINFO *em_task_avaliable[EM_TASK_PRIORITIES_LIMIT] = {0};

// Delayed tasks table
EM_TASKINFO *em_task_delayed[2] = {0};

// Suspend tasks table
EM_TASKINFO *em_task_suspend = 0;

// Current system tick
EM_UINT32 em_current_tick = 0;

// Current tick mode
EM_UINT8 em_current_tick_mode = EM_TICKMODE_NORMAL;

// Current running task
EM_TASKINFO *em_task_current = 0;

// Task to change next
EM_TASKINFO *em_task_next = 0;

// Called when task exit
EM_VOID em_taskExit(EM_VOID)
{
	em_exit();
}

// Storage current task top
EM_VOID em_storageHeap(EM_VOID *heapTop)
{
	if(em_task_current)
		em_task_current->taskHeapTop = heapTop;
}

// Load current task top
EM_VOID *em_getHeap(EM_VOID)
{
	return em_task_next->taskHeapTop;
}

// While emKernel is running, em_taskTick() will run with the fixed fenquery.
// This function used to manage tasks such as tasks switching.
EM_VOID *em_taskTick(EM_VOID)
{
	EM_TASKINFO *em_task_switch = 0;
	EM_TASKINFO *em_task_delay;
	EM_TASKINFO *em_task_delay_next;
	EM_UINT16 i;
	em_enterCritical();
	em_current_tick ++;
	if(em_current_tick == 0)
		em_current_tick_mode = !em_current_tick_mode;
	em_task_delay = em_task_delayed[em_current_tick_mode];
	for(i = 0; i < EM_TASK_PRIORITIES_LIMIT; i ++)
	{
		if(em_task_avaliable[i])
		{
			em_task_switch = em_task_avaliable[i];
			em_task_avaliable[i] = em_task_avaliable[i]->blockNext;
			break;
		}
	}
	while(em_task_delay)
	{
		em_task_delay_next = em_task_delay->blockNext;
		if(em_task_delay->timeDelay <= em_current_tick)
		{
			if(em_task_delay->blockPrev)
			{
				em_task_delay->blockPrev->blockNext = em_task_delay->blockNext;
				if(em_task_delay->blockNext)
					em_task_delay->blockNext->blockPrev = em_task_delay->blockPrev;
			}
			else
			{
				em_task_delayed[em_current_tick_mode] = em_task_delay->blockNext;
				if(em_task_delay->blockNext)
					em_task_delay->blockNext->blockPrev = 0;
			}
			
			if(em_task_avaliable[em_task_delay->taskPriority])
			{
				em_task_delay->blockNext = em_task_avaliable[em_task_delay->taskPriority];
				em_task_delay->blockPrev = em_task_avaliable[em_task_delay->taskPriority]->blockPrev;
				em_task_avaliable[em_task_delay->taskPriority]->blockPrev->blockNext = em_task_delay;
				em_task_avaliable[em_task_delay->taskPriority]->blockPrev = em_task_delay;
			}
			else
			{
				em_task_delay->blockNext = em_task_delay;
				em_task_delay->blockPrev = em_task_delay;
				em_task_avaliable[em_task_delay->taskPriority] = em_task_delay;
			}
			em_task_delay->taskStatus = EM_TASK_RUNNING;
		}
		em_task_delay = em_task_delay_next;
	}
	em_leaveCritical();
	return em_task_switch;
}

EM_VOID em_tfree(EM_TASKINFO *task, void *memFree)
{
	EM_MEMORY_ALLOCTABLE *em_block_free;
	EM_MEMORY_ALLOCTABLE *em_block_free_prev = 0;
	em_enterCritical();
	em_block_free = task->allocatedMem;
	while(em_block_free)
	{
		if(em_block_free == (EM_MEMORY_ALLOCTABLE *)((EM_UINT8 *)memFree - sizeof(EM_MEMORY_ALLOCTABLE)))
		{
			if(!em_block_free_prev)
				task->allocatedMem = em_block_free->allocBlockNext;
			else
				em_block_free_prev->allocBlockNext = em_block_free->allocBlockNext;
			em_free_nonSafe(memFree);
			break;
		}
		em_block_free_prev = em_block_free;
		em_block_free = em_block_free->allocBlockNext;
	}
	em_leaveCritical();
}

EM_VOID *em_talloc(EM_TASKINFO *task, EM_UINT32 allocSize)
{
	void *memblock;
	EM_MEMORY_ALLOCTABLE *allocTable;
	memblock = em_malloc(allocSize);
	if(!memblock)
		return 0;
	em_enterCritical();
	allocTable = (EM_MEMORY_ALLOCTABLE *) ((EM_UINT8 *)memblock - sizeof(EM_MEMORY_ALLOCTABLE));
	allocTable->allocBlockNext = task->allocatedMem;
	task->allocatedMem = allocTable;
	em_leaveCritical();
	return memblock;
}

EM_VOID em_taskDelay(EM_TASKINFO *task, EM_UINT32 delayTime)
{
	int i;
	if(task->taskStatus != EM_TASK_RUNNING)
		return;
	em_enterCritical();
	if(task->blockPrev == task && task->blockNext == task)
		em_task_avaliable[task->taskPriority] = 0;
	else
	{
		task->blockNext->blockPrev = task->blockPrev;
		task->blockPrev->blockNext = task->blockNext;
		if(task == em_task_avaliable[task->taskPriority])
			em_task_avaliable[task->taskPriority] = task->blockNext;
	}
	task->timeDelay = em_current_tick + delayTime;
	if(task->timeDelay < em_current_tick)
		task->delayTable = !em_current_tick_mode;
	else
		task->delayTable = em_current_tick_mode;
	
	// em_task_delayed is a non-cycle table
	if(em_task_delayed[task->delayTable])
	{
		task->blockNext = em_task_delayed[task->delayTable];
		task->blockPrev = 0; // non-cycle
		em_task_delayed[task->delayTable]->blockPrev = task;
		em_task_delayed[task->delayTable] = task;
	}
	else
	{
		task->blockNext = 0; // non-cycle
		task->blockPrev = 0; // non-cycle
		em_task_delayed[task->delayTable] = task;
	}
	task->taskStatus = EM_TASK_DELAYED;
	
	if(em_task_current == task)
	{
		for(i = 0; i < EM_TASK_PRIORITIES_LIMIT; i ++)
		{
			if(em_task_avaliable[i])
			{
				em_task_next = em_task_avaliable[i];
				em_task_avaliable[i] = em_task_avaliable[i]->blockNext;
				break;
			}
		}
		em_leaveCritical();
		em_requestChangeTask();
	}
	else
		em_leaveCritical();
}

EM_VOID em_taskSuspend(EM_TASKINFO *task)
{
	int i;
	
	if(task->taskStatus == EM_TASK_SUSPEND)
		return;
	em_enterCritical();
	
	switch(task->taskStatus)
	{
		case EM_TASK_RUNNING:
			if(task->blockPrev == task && task->blockNext == task)
			{
				em_task_avaliable[task->taskPriority] = 0;
				break;
			}
			task->blockNext->blockPrev = task->blockPrev;
			task->blockPrev->blockNext = task->blockNext;
			if(task == em_task_avaliable[task->taskPriority])
				em_task_avaliable[task->taskPriority] = task->blockNext;
			break;
		case EM_TASK_DELAYED:
			if(task == em_task_delayed[task->delayTable])
			{
				em_task_delayed[task->delayTable] = em_task_delayed[task->delayTable]->blockNext;
				if(em_task_delayed[task->delayTable])
					em_task_delayed[task->delayTable]->blockPrev = 0;
			}
			else
			{
				task->blockPrev->blockNext = task->blockNext;
				if(task->blockNext)
					task->blockNext->blockPrev = task->blockPrev;
			}
			break;
	}
	
	if(em_task_suspend)
	{
		task->blockNext = em_task_suspend;
		task->blockPrev = em_task_suspend->blockPrev;
		em_task_suspend->blockPrev->blockNext = task;
		em_task_suspend->blockPrev = task;
	}
	else
	{
		task->blockNext = task;
		task->blockPrev = task;
		em_task_suspend = task;
	}
	task->taskStatus = EM_TASK_SUSPEND;
	
	if(em_task_current == task)
	{
		for(i = 0; i < EM_TASK_PRIORITIES_LIMIT; i ++)
		{
			if(em_task_avaliable[i])
			{
				em_task_next = em_task_avaliable[i];
				em_task_avaliable[i] = em_task_avaliable[i]->blockNext;
				break;
			}
		}
		em_leaveCritical();
		em_requestChangeTask();
	}
	else
		em_leaveCritical();
}

EM_VOID em_taskDelete(EM_TASKINFO *task)
{
	EM_MEMORY_ALLOCTABLE *em_block_free;
	EM_MEMORY_ALLOCTABLE *em_block_next;
	int i;
	
	em_enterCritical();
	switch(task->taskStatus)
	{
		case EM_TASK_RUNNING:
			if(task->blockPrev == task && task->blockNext == task)
			{
				em_task_avaliable[task->taskPriority] = 0;
				break;
			}
			task->blockNext->blockPrev = task->blockPrev;
			task->blockPrev->blockNext = task->blockNext;
			if(task == em_task_avaliable[task->taskPriority])
				em_task_avaliable[task->taskPriority] = task->blockNext;
			break;
		case EM_TASK_SUSPEND:
			if(task->blockPrev == task && task->blockNext == task)
			{
				em_task_suspend = 0;
				break;
			}
			task->blockNext->blockPrev = task->blockPrev;
			task->blockPrev->blockNext = task->blockNext;
			if(task == em_task_suspend)
				em_task_suspend = task->blockNext;
			break;
		case EM_TASK_DELAYED:
			if(task == em_task_delayed[task->delayTable])
			{
				em_task_delayed[task->delayTable] = em_task_delayed[task->delayTable]->blockNext;
				if(em_task_delayed[task->delayTable])
					em_task_delayed[task->delayTable]->blockPrev = 0;
			}
			else
			{
				task->blockPrev->blockNext = task->blockNext;
				if(task->blockNext)
					task->blockNext->blockPrev = task->blockPrev;
			}
			break;
	}
	
	em_free_nonSafe(task->taskHeap);
	em_block_free = task->allocatedMem;
	while(em_block_free)
	{
		em_block_next = em_block_free->allocBlockNext;
		em_free_nonSafe(em_block_free + sizeof(EM_MEMORY_ALLOCTABLE));
		em_block_free = em_block_next;
	}
	em_free_nonSafe(task);
	
	if(em_task_current == task)
	{
		for(i = 0; i < EM_TASK_PRIORITIES_LIMIT; i ++)
		{
			if(em_task_avaliable[i])
			{
				em_task_next = em_task_avaliable[i];
				em_task_avaliable[i] = em_task_avaliable[i]->blockNext;
				break;
			}
		}
		em_task_current = 0;
		em_leaveCritical();
		em_requestChangeTask();
	}
	else
		em_leaveCritical();
}

EM_VOID em_taskStart(EM_TASKINFO *task)
{
	if(task->taskStatus == EM_TASK_RUNNING)
		return;
	em_enterCritical();

	switch(task->taskStatus)
	{
		case EM_TASK_SUSPEND:
			if(task->blockPrev == task && task->blockNext == task)
			{
				em_task_suspend = 0;
				break;
			}
			task->blockNext->blockPrev = task->blockPrev;
			task->blockPrev->blockNext = task->blockNext;
			if(task == em_task_suspend)
				em_task_suspend = task->blockNext;
			break;
		case EM_TASK_DELAYED:
			if(task == em_task_delayed[task->delayTable])
			{
				em_task_delayed[task->delayTable] = em_task_delayed[task->delayTable]->blockNext;
				if(em_task_delayed[task->delayTable])
					em_task_delayed[task->delayTable]->blockPrev = 0;
			}
			else
			{
				task->blockPrev->blockNext = task->blockNext;
				if(task->blockNext)
					task->blockNext->blockPrev = task->blockPrev;
			}
			break;
	}
	
	if(em_task_avaliable[task->taskPriority])
	{
		task->blockNext = em_task_avaliable[task->taskPriority];
		task->blockPrev = em_task_avaliable[task->taskPriority]->blockPrev;
		em_task_avaliable[task->taskPriority]->blockPrev->blockNext = task;
		em_task_avaliable[task->taskPriority]->blockPrev = task;
	}
	else
	{
		task->blockNext = task;
		task->blockPrev = task;
		em_task_avaliable[task->taskPriority] = task;
	}
	task->taskStatus = EM_TASK_RUNNING;

	em_leaveCritical();
}

EM_TASKINFO *em_taskCreate(EM_CHARCONST *taskName, EM_VOID (*taskFunc)(EM_VOID), EM_VOID *taskArgs, EM_UINT32 heapSize, EM_UINT16 taskPriority)
{
	EM_TASKINFO *em_tasknew;
	EM_UINT8 *em_taskmem;
	
	em_tasknew = (EM_TASKINFO *) em_malloc(sizeof(EM_TASKINFO));
	if(!em_tasknew)
		return (EM_TASKINFO *) EM_TASK_CREATEFAIL_MEMNOTENOUGH;
	em_tasknew->taskName = taskName;
	em_tasknew->taskStatus = EM_TASK_SUSPEND;
	em_tasknew->allocatedMem = 0;
	em_tasknew->taskHeap = em_malloc(heapSize);
	if(!em_tasknew->taskHeap)
	{
		em_free(em_tasknew);
		return (EM_TASKINFO *) EM_TASK_CREATEFAIL_MEMNOTENOUGH;
	}
	em_tasknew->taskPriority = taskPriority;
	em_taskmem = em_tasknew->taskHeap;
	
	#if(EM_TASK_HEAP_MODE == 0)
		em_taskmem += heapSize;
		em_taskmem = em_initTaskHeap(taskFunc, (EM_VOID *) (EM_TASK_HEAP_ALIGNMASK & (EM_PTR) em_taskmem), taskArgs);
	#else
		if(em_taskmem != ((EM_VOID *) (EM_TASK_HEAP_ALIGNMASK & (EM_PTR) em_taskmem)))
			em_taskmem = ((EM_VOID *) ((EM_TASK_HEAP_ALIGNMASK & (EM_PTR) em_taskmem) + (~ EM_TASK_HEAP_ALIGNMASK)));
		em_taskmem = em_initTaskHeap(taskFunc, em_taskmem);
	#endif
	em_tasknew->taskHeapTop = em_taskmem;
	
	em_enterCritical();
	if(em_task_suspend)
	{
		em_tasknew->blockNext = em_task_suspend;
		em_tasknew->blockPrev = em_task_suspend->blockPrev;
		em_task_suspend->blockPrev->blockNext = em_tasknew;
		em_task_suspend->blockPrev = em_tasknew;
	}
	else
	{
		em_tasknew->blockNext = em_tasknew;
		em_tasknew->blockPrev = em_tasknew;
		em_task_suspend = em_tasknew;
	}
	em_leaveCritical();
	
	return em_tasknew;
}
