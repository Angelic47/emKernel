/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel os.h
	OS utils functions for emKernel.

*/

#ifndef __EM_DEF_OS__
#define __EM_DEF_OS__

#include "portable/types.h"

// Initalize standard input & output for console
EM_VOID em_os_stdioInit(EM_VOID);

// Print a char to console
EM_VOID em_printch(EM_UINT8 ch);

// Print a string to console
EM_VOID em_print(EM_CHARCONST *str);

// Print a line to console
EM_VOID em_println(EM_CHARCONST *str);

// Print emKernel banner. Used for startup screen.
EM_VOID em_printBanner(EM_VOID);

// Initalize OS
EM_VOID em_osPreInit(EM_VOID);

// Start emKernel
EM_VOID em_osStartBoot(EM_VOID);

// Assert crash. Will kill os self.
EM_VOID em_osAssertCrash(EM_CHARCONST *str, EM_UINT8 isBlock);

#endif
