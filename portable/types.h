/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel types.h
	Portable basic data types for emKernel.

*/

// Ported for stm32

#ifndef __EM_DEF_TYPES__
#define __EM_DEF_TYPES__

#define EM_VOLATILE volatile
#define EM_ASM __asm
#define EM_VOID void
#define EM_CHAR char
#define EM_CHARCONST const char
#define EM_UINT8 unsigned char
#define EM_INT8 signed char
#define EM_UINT16 unsigned int
#define EM_INT16 signed int
#define EM_UINT32 unsigned long int
#define EM_INT32 signed long int
#define EM_PTR int 
#define EM_INLINE __inline

#endif
