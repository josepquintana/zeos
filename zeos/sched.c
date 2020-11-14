/*
 * sched.c - initializes struct for task 0 and task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1 // why initially there's a zero
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

/* Free Queue: list of task_structs not used in the task array */
struct list_head freequeue;

/* Ready Queue: list of task_structs ready to start or continue the execution */
struct list_head readyqueue;

/* Idle Process PCB */
struct task_struct *idle_task;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}

int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	// Retrieve the first element of the freequeue list, i.e. task[0]
	struct list_head *t = list_first(&freequeue);
	list_del(t); 

	// Get pointer to the memory of the task_struct
	struct task_struct *pcb = list_head_to_task_struct(t);

	// Assign PID 0 to this process
	pcb->PID = 0;

	// Initialize new directory base address 
	allocate_DIR(pcb);


	// Initialize an execution context for the process to restore it when it gets assigned the CPU
	union task_union *tu0 = (union task_union*) pcb;
	tu0->stack[KERNEL_STACK_SIZE-1] = (unsigned long) &cpu_idle; 				/* Return address */
	tu0->stack[KERNEL_STACK_SIZE-2] = 0;										/* Register EBP */
	pcb->ebp_reg_pos = (unsigned int) &(tu0->stack[KERNEL_STACK_SIZE-2]);		/* Current top of the stack (EBP) */

	// Initialize global idle_task variable
	idle_task = pcb;

}

void init_task1(void)
{
	/* This process is called from system.c and the code is implemented in user.c */

	// Retrieve the first element of the freequeue list, now being task[1]
	struct list_head *t = list_first(&freequeue);
	list_del(t); 

	// Get pointer to the memory of the task_struct
	struct task_struct *pcb = list_head_to_task_struct(t);

	// Assign PID 1 to this process
	pcb->PID = 1;

	// Initialize new directory base address for this process
	allocate_DIR(pcb);

	// Allocate physical pages to hold the user address space
	set_user_pages(pcb);

	// Update TSS to point to this task's system stack
	union task_union *tu1 = (union task_union*) pcb;
	tss.esp0 = (DWord) &(tu1->stack[KERNEL_STACK_SIZE]);
	writeMSR(0x175, (int) tss.esp0); 	/* Modify the SYSENTER_ESP_MSR registry */

	// Set process page directory as the current directory in the system (will flush TLB)
	set_cr3(pcb->dir_pages_baseAddr);

}

void init_freequeue()
{
	// Initialize the list
	INIT_LIST_HEAD( &freequeue );
	
	// Add all task_structs to this queue
	for (int i = 0; i < NR_TASKS; i++) {
		task[i].task.PID = -1;
		list_add_tail(&(task[i].task.list), &freequeue);	
	}
}

void init_readyqueue()
{
	// Initialize the list
	INIT_LIST_HEAD( &readyqueue );
	
	// This queue is empty at the beginning
}

void init_sched()
{
	init_freequeue();
	init_readyqueue();
}
 
/* ======= */ 
/* HELP ?? */
/* ======= */
void inner_task_switch(union task_union *new)
{
	/* Restore the execution of the new process in the same state it had before invoking this routine */

	// Update TSS to point to the stack of the new task
	tss.esp0 = (DWord) &(new->stack[KERNEL_STACK_SIZE]);
	writeMSR(0x175, (int) tss.esp0); 	/* Modify the SYSENTER_ESP_MSR registry */

	// Set new task page directory as the current directory in the system (will flush TLB)
	set_cr3(get_DIR(&(new->task)));

	// Store current value of EBP register
	__asm__ __volatile__ (
		"mov %%ebp, %0\n\t"
		: "=g" (current()->ebp_reg_pos)
		: );

	// Change current system stack by setting the ESP registry to point to the stored EBP value in the new_task PCB
	// Process has been switched
	__asm__ __volatile__ (
		"mov %0, %%esp\n\t"
		:
		: "g" (new->task.ebp_reg_pos) );
	
	// Restore EBP register from the stack
	__asm__ __volatile__ (
		"pop %%ebp\n\t"
		:
		: );

	// Return to caller routine with own RET instruction
	__asm__ __volatile__ (
		"ret\n\t"
		:
		: );

}

/* 
 * Returns the  address of the current PCB (task_struct) from the 
 * stack pointer (esp register) due to their overlapping of the 
 * physical memory space. (The task_struct is located at the begining)
 */ 
struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

