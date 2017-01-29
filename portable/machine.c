/*
	 _____       ___  ___   _   _    _____   _____    __   _   _____   _      
	| ____|     /   |/   | | | / /  | ____| |  _  \  |  \ | | | ____| | |     
	| |__      / /|   /| | | |/ /   | |__   | |_| |  |   \| | | |__   | |     
	|  __|    / / |__/ | | | |\ \   |  __|  |  _  /  | |\   | |  __|  | |     
	| |___   / /       | | | | \ \  | |___  | | \ \  | | \  | | |___  | |___  
	|_____| /_/        |_| |_|  \_\ |_____| |_|  \_\ |_|  \_| |_____| |_____| 

	emKernel machine.c
	Portable common functions and other implementation.

*/

// Ported for stm32

#include "../task.h"
#include "../os.h"
#include "machine.h"
#include "stm32f10x.h"

//EM_INLINE EM_VOID EM_ASM em_changeTask(EM_VOID *taskNext)
__asm void PendSV_Handler(void)
{
	import em_storageHeap
	import em_getHeap
	import em_task_current
	import em_task_next
	
	REQUIRE8
	PRESERVE8

	CPSID I
	
	mov r0, sp
	sub r0, #(8*4) // current process stack point
	push {lr}
	bl.w em_storageHeap // invoke em_storageHeap
	pop {lr}
	
	push {lr}
	bl.w em_getHeap
	pop {lr}
	
	push {r4-r11} // save current r4-r11 to process stack
	mov sp, r0 // change stack
	pop {r4-r11} // restore r4-r11 for another process
	
	ldr r0, =em_task_current
	ldr r1, =em_task_next
	ldr r2, [r1]
	str r2, [r0]
	
	CPSIE I
	
	bx lr
	nop
}

EM_VOID EM_ASM *em_initTaskHeap(EM_VOID (*taskFunc)(EM_VOID), EM_VOID *taskHeapPtr, EM_VOID *taskArgs)
{
	IMPORT em_taskExit
	REQUIRE8
	PRESERVE8
	
	SUB r1, #4
	MOV r3, #0x01000000
	STR r3, [r1] // xPSR
	
	SUB r1, #4
	STR r0, [r1] // PC
	
	SUB r1, #4
	LDR r3, =em_taskExit
	STR r3, [r1] // LR
	
	SUB r1, #(5*4) // R12, R3-R1
	
	STR r2, [r1] // R0, task args
	
	SUB r1, #(8*4) // R11-R4
	
	MOV r0, r1
	BX LR
	NOP
}

EM_VOID em_initUSART(EM_VOID)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	//USART1 Tx(PA.09) 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//USART1 Rx(PA.10) 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//USART1
	USART_InitStructure.USART_BaudRate = 115200; 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; 
	USART_InitStructure.USART_StopBits = USART_StopBits_1; 
	USART_InitStructure.USART_Parity = USART_Parity_No; 
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; 
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE); 
}

EM_VOID em_USART_putc(EM_UINT8 ch)
{
	USART1->SR;
	USART_SendData(USART1, ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
}

EM_VOID em_USART_puts(EM_CHARCONST *str)
{
	while(*str)
		em_USART_putc(*(str ++));
}

EM_VOID EM_ASM em_initPendSV(EM_VOID)
{
	REQUIRE8
	PRESERVE8
	
NVIC_SYSPRI14 EQU 0xE000ED22
NVIC_PENDSV_PRI EQU 0xFF
	
	LDR R1, =NVIC_PENDSV_PRI
	LDR R0, =NVIC_SYSPRI14
	STRB R1, [R0]
	
	BX LR
	NOP
}

EM_VOID EM_ASM em_sendPendSV(EM_VOID)
{
	REQUIRE8
	PRESERVE8
	
NVIC_INT_CTRL EQU 0xE000ED04
NVIC_PENDSVSET EQU 0x10000000
	
	LDR R0, =NVIC_INT_CTRL
	LDR R1, =NVIC_PENDSVSET
	STR R1, [R0]
	
	BX LR
	NOP
}

EM_VOID em_initInterrupt(EM_VOID)
{
	em_initPendSV();
	em_initTaskTick();
}

void HardFault_Handler(void)
{
	em_osAssertCrash("Segmentation fault", 0);
	
	if (CoreDebug->DHCSR & 1) {
		em_println("*** Debugger found, breakpointing...");
		__breakpoint(0);
	}
	
	while(1);
}

void SysTick_Handler(void)
{
	em_task_next = em_taskTick();
	em_sendPendSV();
}
