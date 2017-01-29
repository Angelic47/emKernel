/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel task.h
	Data definition for task.c and setting.

*/

#ifndef __EM_DEF_TASK__
#define __EM_DEF_TASK__

#include "portable/types.h"
#include "memory/memory.h"
#include "emconfig.h"

// Defines the limit of task priorities (Lowest priority is 0)
#define EM_TASK_PRIORITIES_LIMIT EM_CONFIG_PRIORITIES_LIMIT

// Heap align
#define EM_TASK_HEAP_ALIGNMASK 0xfffffff8

// Heap mode, 0: down, 1: up
#define EM_TASK_HEAP_MODE 0

// Data structure for task info
struct _EM_TASKINFO
{
	struct _EM_TASKINFO *blockPrev;
	struct _EM_TASKINFO *blockNext;
	EM_CHARCONST *taskName;
	EM_UINT8 taskStatus;
	EM_UINT8 delayTable;
	EM_UINT32 timeDelay;
	EM_UINT16 taskPriority;
	EM_MEMORY_ALLOCTABLE *allocatedMem;
	EM_VOID *taskHeap;
	EM_VOID *taskHeapTop;
};
#define EM_TASKINFO struct _EM_TASKINFO

// Constant values
#define EM_TASK_RUNNING 0
#define EM_TASK_DELAYED 1
#define EM_TASK_SUSPEND 2

#define EM_TASK_CREATESUCCESS 0
#define EM_TASK_CREATEFAIL_MEMNOTENOUGH 1

#define EM_TICKMODE_NORMAL 0
#define EM_TICKMODE_OVERFLOW 1

// Functions

// Called automatically when task exiting.
// Please NEVER use this function on your code.
EM_VOID em_taskExit(EM_VOID);

// While emKernel is running, em_taskTick() will run with the fixed fenquery.
// This function used to manage tasks such as tasks switching.
EM_VOID *em_taskTick(EM_VOID);

// Task allocate memory.
EM_VOID em_tfree(EM_TASKINFO *task, void *memFree);

// Task free memory.
EM_VOID *em_talloc(EM_TASKINFO *task, EM_UINT32 allocSize);

// Task allocate memory (Used for task self).
#define em_sfree(x) em_tfree(em_task_current, x)

// Task free memory (Used for task self).
#define em_salloc(x) em_talloc(em_task_current, x)

// Delay a task (ticks).
// Only running task can be delayed.
EM_VOID em_taskDelay(EM_TASKINFO *task, EM_UINT32 delayTime);

// Delay self (ticks).
#define em_delay(x) em_taskDelay(em_task_current,x)

// Suspend a task. Task will not run until em_taskStart() has been called.
EM_VOID em_taskSuspend(EM_TASKINFO *task);

// Suspend self (ticks).
#define em_suspend() em_taskSuspend(em_task_current)

// Start a task.
EM_VOID em_taskStart(EM_TASKINFO *task);

// Create a new task with giving function and args.
// Using em_taskStart() to start your task when task created successfully.
EM_TASKINFO *em_taskCreate(EM_CHARCONST *taskName, EM_VOID (*taskFunc)(EM_VOID), EM_VOID *taskArgs, EM_UINT32 heapSize, EM_UINT16 taskPriority);

// Delete a task.
EM_VOID em_taskDelete(EM_TASKINFO *task);

// Task exit.
#define em_exit() em_taskDelete(em_task_current)

// Storage current task top
EM_VOID em_storageHeap(EM_VOID *heapTop);

// Load current task top
EM_VOID *em_getHeap(EM_VOID);

// Current running task
extern EM_TASKINFO *em_task_current;

// Task to change next
extern EM_TASKINFO *em_task_next;

#endif
