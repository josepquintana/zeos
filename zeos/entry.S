/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/


#define SAVE_ALL \
	pushl %gs; \
	pushl %fs; \
	pushl %es; \
	pushl %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $__KERNEL_DS, %edx;    \
	movl %edx, %ds;           \
	movl %edx, %es;



#define RESTORE_ALL\
	popl %ebx; \
	popl %ecx; \
	popl %edx; \
	popl %esi; \
	popl %edi; \
	popl %ebp; \
	popl %eax; \
	popl %ds; \
	popl %es; \
	popl %fs; \
	popl %gs;



#define EOI \
	movb $0x20, %al; \
	outb %al, $0x20;
	
	
	
/************************************************/	


	
ENTRY(keyboard_handler)
	SAVE_ALL
	push	%eax
	call	update_statistics_user_to_system					  
	pop 	%eax
	EOI
	call	keyboard_routine
	push	%eax
	call	update_statistics_system_to_user					  
	pop 	%eax	
	RESTORE_ALL
	iret



ENTRY(clock_handler)
	SAVE_ALL
	push	%eax			 
	call	update_statistics_user_to_system					  
	pop 	%eax			
	EOI
	call	clock_routine
	push	%eax
	call	update_statistics_system_to_user					  
	pop 	%eax			
	RESTORE_ALL
	iret



ENTRY(syscall_handler_sysenter)
	push	$__USER_DS
	push	%ebp				// User stack address
	pushfl
	push	$__USER_CS
	push	4(%ebp)       		// User return address
	
	SAVE_ALL
	
	cmpl	$0, %eax
	jl  	sysenter_err
	cmpl	$MAX_SYSCALL, %eax
	jg  	sysenter_err
	 
	call	*sys_call_table(, %eax, 0x04)
	
	jmp 	sysenter_fin
	
sysenter_err:
	movl	$-38, %eax	/*ENOSYS*/
	
sysenter_fin:
	movl	%eax, 0x18(%esp)
	RESTORE_ALL
	movl	(%esp), %edx		// Return address
	movl	12(%esp), %ecx		// User stack address
	sti							// Enable interrupts again
	sysexit


