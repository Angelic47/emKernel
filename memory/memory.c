/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel memory.c
	Contains memory manage functions such as allocate and free.

*/

#include "memory.h"
#include "../portable/types.h"
#include "../portable/machine.h"
#include "../emconfig.h"

// Memory Block
// To change the memory size, go to memory.h and change EM_MEMORY_BLOCK_SIZE value.
EM_CHAR em_memory_block[EM_MEMORY_BLOCK_SIZE];

// Memory Allocation Table
// Memory allocation table linked all free memory blocks together and used for memory allocation when needs.
EM_MEMORY_ALLOCTABLE *em_alloc_table;

EM_UINT32 em_getAllMemorySize(EM_VOID)
{
	return EM_MEMORY_BLOCK_SIZE;
}

EM_VOID em_meminit(EM_VOID)
{
	em_alloc_table = (EM_MEMORY_ALLOCTABLE *) em_memory_block;
	
	// Align memory header
	if((EM_PTR) em_alloc_table & EM_MEMORY_ALIGN_MASK)
		em_alloc_table = (EM_MEMORY_ALLOCTABLE *) ((EM_UINT8 *)em_alloc_table + ((EM_MEMORY_ALIGN_MASK + 1 - ((EM_PTR) em_alloc_table & EM_MEMORY_ALIGN_MASK)) & EM_MEMORY_ALIGN_MASK));
	
	em_alloc_table->allocBlockNext = 0;
	em_alloc_table->blockSize = EM_MEMORY_BLOCK_SIZE - ((EM_MEMORY_ALIGN_MASK + 1 - ((EM_PTR) em_alloc_table & EM_MEMORY_ALIGN_MASK)) & EM_MEMORY_ALIGN_MASK);
}

// There is no em_enterCritical() protected and please NEVER use this function on your application.
// This function will called automatically when memory block be freed.
#if (EM_MEMORY_OPTIMIZE)
EM_VOID em_memOptimize(EM_VOID)
{
	EM_MEMORY_ALLOCTABLE *em_block_ptr = em_alloc_table;
	
	if(!em_block_ptr)
		return;
	
	while(1)
	{
		if(!em_block_ptr->allocBlockNext)
			return;
		if((EM_UINT8 *) em_block_ptr + em_block_ptr->blockSize == (EM_VOID *) (em_block_ptr->allocBlockNext))
		{
			em_block_ptr->blockSize += em_block_ptr->allocBlockNext->blockSize;
			em_block_ptr->allocBlockNext = em_block_ptr->allocBlockNext->allocBlockNext;
			continue;
		}
		else
			em_block_ptr = em_block_ptr->allocBlockNext;
	}
}
#endif

EM_UINT32 em_getFreeMemSize(EM_VOID)
{
	EM_MEMORY_ALLOCTABLE *em_block_ptr = em_alloc_table;
	EM_UINT32 result = 0;
	
	em_enterCritical();
	while(em_block_ptr)
	{
		result += em_block_ptr->blockSize - sizeof(EM_MEMORY_ALLOCTABLE);
		em_block_ptr = em_block_ptr->allocBlockNext;
	}
	em_leaveCritical();
	
	return result;
}

EM_VOID *em_malloc(EM_UINT32 allocSize)
{
	EM_MEMORY_ALLOCTABLE *em_block_ptr = em_alloc_table;
	EM_MEMORY_ALLOCTABLE *em_block_use = 0, *em_block_prev = 0;
	
	em_enterCritical();
	// Align allocSize
	if(allocSize & EM_MEMORY_ALIGN_MASK)
		allocSize += (EM_MEMORY_ALIGN_MASK + 1 - (allocSize & EM_MEMORY_ALIGN_MASK)) & EM_MEMORY_ALIGN_MASK;
	
	// Find the suitable block first
	while(em_block_ptr)
	{
		if(em_block_ptr->blockSize < allocSize + sizeof(EM_MEMORY_ALLOCTABLE))
			goto nextBlock;
		if(!em_block_use)
		{
			em_block_prev = em_block_use;
			em_block_use = em_block_ptr;
			goto nextBlock;
		}
		if(em_block_ptr->blockSize < em_block_use->blockSize)
		{
			em_block_prev = em_block_use;
			em_block_use = em_block_ptr;
		}
		nextBlock:
		em_block_ptr = em_block_ptr->allocBlockNext;
	}
	
	if(!em_block_use)
		return (EM_VOID *) EM_MEMORY_ALLOCATE_FAIL;
	
	// Allocate the memory and rebuild the new allocation block
	if(em_block_use->blockSize - (allocSize + sizeof(EM_MEMORY_ALLOCTABLE)) > sizeof(EM_MEMORY_ALLOCTABLE))
	{
		em_block_ptr = (EM_MEMORY_ALLOCTABLE *) ((EM_UINT8 *) em_block_use + allocSize + sizeof(EM_MEMORY_ALLOCTABLE));
		em_block_ptr->allocBlockNext = em_block_use->allocBlockNext;
		em_block_ptr->blockSize = em_block_use->blockSize - (allocSize + sizeof(EM_MEMORY_ALLOCTABLE));
		em_block_use->blockSize = allocSize + sizeof(EM_MEMORY_ALLOCTABLE);
		if(em_block_prev)
			em_block_prev->allocBlockNext = em_block_ptr;
		else
			em_alloc_table = em_block_ptr;
	}
	else
	{
		if(em_block_prev)
			em_block_prev->allocBlockNext = em_block_use->allocBlockNext;
		else
			em_alloc_table = em_block_use->allocBlockNext;
	}
	em_leaveCritical();
	
	return (EM_UINT8 *) em_block_use + sizeof(EM_MEMORY_ALLOCTABLE);
}

EM_VOID em_free_nonSafe(EM_VOID *memFree)
{
	EM_MEMORY_ALLOCTABLE *em_block_free;
	EM_MEMORY_ALLOCTABLE *em_block_ptr = em_alloc_table;
	EM_MEMORY_ALLOCTABLE *em_block_prev = 0;
	
	em_block_free = (EM_MEMORY_ALLOCTABLE *) ((EM_UINT8 *)memFree - sizeof(EM_MEMORY_ALLOCTABLE));
	
	// Find the place to insert block
	while(em_block_ptr && em_block_ptr < em_block_free)
	{
		em_block_prev = em_block_ptr;
		em_block_ptr = em_block_ptr->allocBlockNext;
	}
	
	// Insert the block to allocation table
	if(!em_block_prev)
	{
		em_block_free->allocBlockNext = em_alloc_table;
		em_alloc_table = em_block_free;
	}
	else if(!em_block_ptr)
	{
		em_block_prev->allocBlockNext = em_block_free;
		em_block_free->allocBlockNext = 0;
	}
	else
	{
		em_block_prev->allocBlockNext = em_block_free;
		em_block_free->allocBlockNext = em_block_ptr;
	}
	
	#if (EM_MEMORY_OPTIMIZE)
	// Optimize memory allocation block
	em_memOptimize();
	#endif
	
}

EM_VOID em_free(EM_VOID *memFree)
{
	EM_MEMORY_ALLOCTABLE *em_block_free;
	EM_MEMORY_ALLOCTABLE *em_block_ptr = em_alloc_table;
	EM_MEMORY_ALLOCTABLE *em_block_prev = 0;
	
	em_enterCritical();
	
	em_block_free = (EM_MEMORY_ALLOCTABLE *) ((EM_UINT8 *)memFree - sizeof(EM_MEMORY_ALLOCTABLE));
	
	// Find the place to insert block
	while(em_block_ptr && em_block_ptr < em_block_free)
	{
		em_block_prev = em_block_ptr;
		em_block_ptr = em_block_ptr->allocBlockNext;
	}
	
	// Insert the block to allocation table
	if(!em_block_prev)
	{
		em_block_free->allocBlockNext = em_alloc_table;
		em_alloc_table = em_block_free;
	}
	else if(!em_block_ptr)
	{
		em_block_prev->allocBlockNext = em_block_free;
		em_block_free->allocBlockNext = 0;
	}
	else
	{
		em_block_prev->allocBlockNext = em_block_free;
		em_block_free->allocBlockNext = em_block_ptr;
	}
	
	#if (EM_MEMORY_OPTIMIZE)
	// Optimize memory allocation block
	em_memOptimize();
	#endif
	
	em_leaveCritical();
}
