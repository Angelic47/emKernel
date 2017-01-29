/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel os.c
	OS utils implementation.

*/

#include "memory/memory.h"
#include "portable/machine.h"
#include "portable/types.h"
#include "task.h"

EM_VOID em_os_stdioInit(EM_VOID)
{
	em_initUSART();
}

EM_VOID em_printch(EM_UINT8 ch)
{
	em_USART_putc(ch);
}

EM_VOID em_print(EM_CHARCONST *str)
{
	em_USART_puts(str);
}

EM_VOID em_println(EM_CHARCONST *str)
{
	em_USART_puts(str);
	em_USART_putc('\r');
	em_USART_putc('\n');
}

EM_VOID em_printBanner(EM_VOID)
{
	em_println(" _____       ___  ___   _   _    _____   _____    __   _   _____   _      ");
	em_println("| ____|     /   |/   | | | / /  | ____| |  _  \\  |  \\ | | | ____| | |     ");
	em_println("| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \\| | | |__   | |     ");
	em_println("|  __|    / / |__/ | | | |\\ \\   |  __|  |  _  /  | |\\   | |  __|  | |     ");
	em_println("| |___   / /       | | | | \\ \\  | |___  | | \\ \\  | | \\  | | |___  | |___  ");
	em_println("|_____| /_/        |_| |_|  \\_\\ |_____| |_|  \\_\\ |_|  \\_| |_____| |_____| ");
	em_println("");
	em_println(OS_PORT_UNAME);
}

EM_VOID em_osAssertCrash(EM_CHARCONST *str, EM_UINT8 isBlock)
{
	em_enterCritical();
	em_println("\r\n**** emKernel Crashed! ****");
	em_print("Caused by: ");
	em_println(str);
	em_println("System will going HALT. Please restart the system.");
	if(isBlock)
		while(1);
}

EM_VOID em_idleTask()
{
	while(1);
}

EM_VOID em_osPreInit(EM_VOID)
{
	em_os_stdioInit();
	em_meminit();
}

EM_VOID em_osStartBoot(EM_VOID)
{
	EM_TASKINFO *em_idleTaskBlock;
	
	em_printBanner();
	em_println("OS booting started!");
	em_idleTaskBlock = em_taskCreate("SysIdle", em_idleTask, 0, 128, EM_TASK_PRIORITIES_LIMIT - 1);
	if(em_idleTaskBlock == (EM_TASKINFO *)1)
	{
		em_println("Failed to load task: SysIdle, perhaps memory not enough?");
		em_osAssertCrash("Resource not avaliable", 1);
	}
	em_taskStart(em_idleTaskBlock);
	em_initInterrupt();
	while(1);
}
