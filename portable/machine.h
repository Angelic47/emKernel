/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel machine.h
	Portable common functions and other implementation.

*/

// Ported for stm32

#ifndef __EM_DEF_MACHINE__
#define __EM_DEF_MACHINE__

#include "types.h"
#include "stm32f10x.h"

// Task tick time (default: 72000000/1000 => 1ms)
#define TICK_TIME 72000000/1000

#define OS_PORT_UNAME "emKernel 2.0 Beta on NetNode32 stm32f103c8t6"

//EM_VOID EM_ASM em_changeTask(EM_VOID *newHeap);
EM_VOID EM_ASM em_initInterrupt(EM_VOID);
EM_VOID EM_ASM *em_initTaskHeap(EM_VOID (*taskFunc)(EM_VOID), EM_VOID *taskHeapPtr, EM_VOID *taskArgs);
EM_VOID EM_ASM em_sendPendSV(EM_VOID);
#define em_enterCritical() EM_ASM("CPSID I")
#define em_leaveCritical() EM_ASM("CPSIE I")
#define em_initTaskTick() SysTick_Config(TICK_TIME)
#define em_requestChangeTask() em_sendPendSV()
EM_VOID em_initUSART(EM_VOID);
EM_VOID em_USART_putc(EM_UINT8 ch);
EM_VOID em_USART_puts(EM_CHARCONST *str);

#endif
