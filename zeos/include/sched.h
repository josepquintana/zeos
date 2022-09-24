/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>
#include <sem.h>


#define NR_TASKS      10
#define NR_THREADS    20
#define KERNEL_STACK_SIZE	1024
#define SEM_SIZE      20

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

typedef unsigned char pthread_t;

struct thread_struct {
	pthread_t TID; //0-10 max
	//struct task_struct *PCB; //to unsigned char
	int PID;
    struct list_head list;
	int total_quantum;
	int register_esp;
	enum state_t state;
	int retval;
};

union thread_union {
	struct thread_struct thread;
	unsigned long sys_stack[KERNEL_STACK_SIZE]; // thread+1 -> KERNEL_STACK_SIZE
	unsigned long TLS[2*KERNEL_STACK_SIZE]; // KERNEL_STACK_SIZE+1 -> 2*KERNEL_STACK_SIZE
//	unsigned long user_stack[4*KERNEL_STACK_SIZE];// (2*KERNEL_STACK_SIZE)+1 -> 4*KERNEL_STACK_SIZE
};

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */  //to unsigned char?
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;	/* Task struct enqueuing */
  enum state_t state;		/* State of the process */
  int total_quantum;		/* Total quantum of the process */
  struct stats p_stats;		/* Process stats */

  struct list_head readythreadqueue;
  struct list_head zombiethreadqueue;
  unsigned char threads[10];
};

union task_alineate {  //only needed for alineation to get current(), when all works try to do it smaller
  struct task_struct task;
  unsigned long alineate[KERNEL_STACK_SIZE];
};


extern union thread_union protected_threads[NR_THREADS+2];
extern union thread_union *thread; /* Vector de tasques */

extern union task_alineate protected_tasks[NR_TASKS+2];
extern union task_alineate *task; /* Vector de tasques */
extern struct task_struct *idle_task;

extern struct sem_t sem_array[SEM_SIZE];


#define KERNEL_ESP(t)       	(DWord) &(t)->sys_stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&thread[1])

extern struct list_head freequeue;
extern struct list_head readyqueue;

extern struct list_head freethreadqueue;

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

void schedule(void);

struct task_struct * current();
struct thread_struct * current_thread();

void task_switch(union task_alineate*t);
void switch_stack(int * save_sp, int new_sp);

void thread_switch(union thread_union *th);
void update_thread_state_rr(struct thread_struct *t, struct list_head *dest);
void sched_next_thread_rr(void);
void sched_next_task_rr(void);

void force_task_switch(void);

struct task_struct *list_head_to_task_struct(struct list_head *l);
struct thread_struct *list_head_to_thread_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

void init_stats(struct stats *s);

#endif  /* __SCHED_H__ */
