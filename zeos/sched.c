/*
 * sched.c - initializes struct for task 0 and task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1 // why initially there's a zero ?
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry(l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

/* Free Queue: list of task_structs not used in the task array */
struct list_head freequeue;

/* Ready Queue: list of task_structs ready to start or continue the execution */
struct list_head readyqueue;

/* Idle Process PCB */
struct task_struct *idle_task;

/* Define default quantum value */
#define DEFAULT_QUANTUM 10

// Variable to store the remaining allowed quantum for the running process
int remaining_allowed_quantum;

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

	// Assign total quantum to this process
	set_quantum(pcb, DEFAULT_QUANTUM);

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

	// Assign total quantum to this process
	set_quantum(pcb, DEFAULT_QUANTUM);

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

	// Set RUN state
	pcb->state = ST_RUN;

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

/*
 * SCHEDULING
 * Make scheduling decisions when necessary
 */
void schedule()
{
	update_sched_data_rr();
	if(needs_sched_rr())
	{
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}
}


/*
 * Update the number of ticks (quantum) that the process has executed since it got assigned the cpu.
 */
void update_sched_data_rr(void) 
{
	if(remaining_allowed_quantum > 0) 
	{
		remaining_allowed_quantum -= 1;
	}
}

/*
 * Decide if it is necessary to change the current process
 * Returns -> 1 if it is necessary to change the current process and 0 otherwise
 */
int needs_sched_rr(void)
{
	// Check if the allowed quantum for the current process has expired
	if(remaining_allowed_quantum == 0) 
	{
		if(! list_empty(&readyqueue)) { return 1; }						// It is necessary to change the current process
		else { remaining_allowed_quantum = get_quantum(current()); }		// Since the are no READY processes there's no need to change the current process
	}
	return 0; 	// No need to change the current process
}

/* 
 * Update the current state of a process by deleting it from its current queue (state) and inserting it into a new queue
 *
 * task_struct *t: PCB of the process to update 
 * list_head *dest_queue: queue according to the new state of the process. 
 *
 * TODO: Update the readyqueue, if current process is not the idle process, by inserting the current process at the end of the readyqueue.
 */
void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue) 
{
	switch(t->state) 
	{
		case ST_RUN:
			/* No need to delete the task from any list */
			list_add_tail(&(t->list), dst_queue); // Add task to the specified destination queue
			if(dst_queue == &readyqueue) { t->state = ST_READY; }  // Update state to READY since the destination queue is the "readyqueue"
			else { t->state = ST_BLOCKED; }
			break;
			
		case ST_BLOCKED:
			list_del(&(t->list)); // Delete task from its current list
			if(dst_queue == NULL) { t->state = ST_RUN; } // Update state to RUN when the destination queue is empty
			else {
				list_add_tail(&(t->list), dst_queue); // Add task to the specified destination queue
				t->state = ST_READY; // Update state to READY since the destination queue is the "readyqueue"
			}
			break;
			
		case ST_READY:
			list_del(&(t->list)); // Delete task from its current list, i.e. the "readyqueue"
			if(dst_queue == NULL) { t->state = ST_RUN; } // Update state to RUN when the destination queue is empty
			else {
				list_add_tail(&(t->list), dst_queue); // Add task to the specified destination queue
				t->state = ST_BLOCKED; // Update state to BLOCKED
			}
			break;

		default:
			break;
	}
}

/*
 * Select the next process to execute, extract it from the Ready queue and to invoke the context switch process
 */
void sched_next_rr(void)
{
	struct task_struct *pcb_next;
	
	// Check if there are any processes READY to get the CPU assigned
	if(! list_empty(&readyqueue))
	{
		// Get the first READY task from the readyqueue list. Then remove it from the list
		struct list_head *ft = list_first(&readyqueue);
		list_del(ft);

		// Get a pointer to the memory of the retrieved READY task which will be the next process to get the CPU assigned
		pcb_next = list_head_to_task_struct(ft);
	}
	else
	{
		// Select the Idle Task as the next process to execute when there are no other processes READY
		pcb_next = idle_task;
	}

	// Change selected next process state to RUN
	pcb_next->state = ST_RUN;
	
	// Set the remaining allowed quantum variable to the value of the selected next process total quantum
	remaining_allowed_quantum = get_quantum(pcb_next);

	// Invoke the context switch method to start executing the selected next process
	task_switch((union task_union*) pcb_next);
}

int get_quantum(struct task_struct *t)
{
  	return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum)
{
  	t->quantum = new_quantum;
}
