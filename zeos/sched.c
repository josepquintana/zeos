/*
 * sched.c - initializes struct for task 0 and task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 0
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

}

void init_task1(void)
{

}

void init_freequeue() {
	// Initialize the list
	INIT_LIST_HEAD( &freequeue );
	
	// Add all task_structs to this queue
	for (int i = 0; i < NR_TASKS; i++) {
		task[i].task.PID = -1;
		list_add_tail(&(task[i].task.list), &freequeue);	
	}
}

void init_readyqueue() {
	// Initialize the list
	INIT_LIST_HEAD( &readyqueue );
	
	// This queue is empty at the beginning
}

void init_sched()
{
	init_freequeue();
	init_readyqueue();
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

