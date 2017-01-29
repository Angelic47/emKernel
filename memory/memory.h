/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel memory.h
	Data definition for memory.c and setting.

*/

#ifndef __EM_DEF_MEMORY__
#define __EM_DEF_MEMORY__

#include "../portable/types.h"
#include "../emconfig.h"

// Memory align
#define EM_MEMORY_ALIGN_MASK 0x03

// Memory block size, used as all avaliable memory for tasks.
#define EM_MEMORY_BLOCK_SIZE EM_CONFIG_MEMORY

// Data structure for allocation table
struct _EM_MEMORY_ALLOCTABLE
{
	struct _EM_MEMORY_ALLOCTABLE *allocBlockNext;
	EM_UINT32 blockSize;
};
#define EM_MEMORY_ALLOCTABLE struct _EM_MEMORY_ALLOCTABLE

// Constant values
#define EM_MEMORY_ALLOCATE_FAIL 0

// Functions and variables
extern EM_CHAR em_memory_block[EM_MEMORY_BLOCK_SIZE];

EM_UINT32 em_getAllMemorySize(EM_VOID);
EM_VOID em_meminit(EM_VOID);
#if (EM_MEMORY_OPTIMIZE)
EM_VOID em_memOptimize(EM_VOID);
#endif
EM_UINT32 em_getFreeMemSize(EM_VOID);
EM_VOID *em_malloc(EM_UINT32 allocSize);
EM_VOID em_free(EM_VOID *memFree);
EM_VOID em_free_nonSafe(EM_VOID *memFree);


#endif
