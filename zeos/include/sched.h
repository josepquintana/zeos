/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h> 

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
  	int PID;								/* Process ID. This MUST be the first field of the struct. */
  	page_table_entry * dir_pages_baseAddr; 	/* Directory base address */
  	struct list_head list;					/* Task struct enqueuing */
	unsigned int kernel_esp;				/* Position of the stack with the initial value for the EBP register */
	enum state_t state;						/* State of the process */
	int quantum;							/* Quantum: Time allowed to run in the CPU */
	struct stats p_stats;					/* Statistical information of the process */
};

union task_union {
  	struct task_struct task;
  	unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */

extern struct list_head freequeue;

extern struct list_head readyqueue;

#define KERNEL_ESP(t)       (DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

extern int remaining_allowed_quantum;

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

void init_freequeue(void);

void init_readyqueue(void);

struct task_struct *current();

void task_switch(union task_union *new);

void inner_task_switch(union task_union *new);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry *get_PT (struct task_struct *t) ;

page_table_entry *get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void update_sched_data_rr();
int needs_sched_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
void sched_next_rr();
void schedule(void);

int get_quantum(struct task_struct *t);
void set_quantum(struct task_struct *t, int new_quantum);

/* Statistical information */
void init_stats(struct stats *s);

/* Function to write to the MSR registers */
void writeMSR(int numMSR, int value);

#endif  /* __SCHED_H__ */
