/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

#define IDT_ENTRIES 256

extern Gate idt[IDT_ENTRIES];
extern Register idtR;

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

void setIdt();

/* Handlers for interrupts */
void keyboard_handler();
void clock_handler();

/* Fast system calls (sysenter) handler */
void syscall_handler_sysenter();

/* 
 * Function to write to the MSR registers 
 * TO DO: Use two variables and store valueHIGH and valueLOW
*/
void writeMSR(int numMSR, int value);

#endif  /* __INTERRUPT_H__ */
