/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel emconfig.h
	emKernel configuration file.

*/

#ifndef __EM_DEF__CONFIG__
#define __EM_DEF__CONFIG__

// Defines how many memory avaliable for memory allocation
#define EM_CONFIG_MEMORY 15360

// Defines the limit of task priorities (Lowest priority is 0)
#define EM_CONFIG_PRIORITIES_LIMIT 2

// Defines if use memory optimize.
#define EM_MEMORY_OPTIMIZE 1

#endif
