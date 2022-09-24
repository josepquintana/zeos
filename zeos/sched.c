/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <types.h>
#include <hardware.h>
#include <segment.h>
#include <sched.h>
#include <mm.h>
#include <io.h>
#include <utils.h>
#include <p_stats.h>
/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */

union task_alineate protected_tasks[NR_TASKS + 2]
    __attribute__((__section__(".data.task")));

union task_alineate *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

union thread_union protected_threads[NR_THREADS + 2]
    __attribute__((__section__(".data.thread")));

union thread_union *thread = &protected_threads[1];

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
struct thread_struct *list_head_to_thread_struct(struct list_head *l)
{
  return list_entry( l, struct thread_struct, list);
}
#endif

extern struct list_head blocked;

// Free task structs
struct list_head freequeue;
// Ready queue
struct list_head readyqueue;

// Free task structs
struct list_head freethreadqueue;

struct sem_t sem_array[SEM_SIZE];

void init_stats(struct stats *s)
{
  s->user_ticks = 0;
  s->system_ticks = 0;
  s->blocked_ticks = 0;
  s->ready_ticks = 0;
  s->elapsed_total_ticks = get_ticks();
  s->total_trans = 0;
  s->remaining_ticks = get_ticks();
}

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry *get_DIR(struct task_struct *t)
{
  return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry *get_PT(struct task_struct *t)
{
  return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr)) << 12);
}

int allocate_DIR(struct task_struct *t)
{
  int pos;

  pos = ((int)t - (int)task) / sizeof(union task_alineate);

  t->dir_pages_baseAddr = (page_table_entry *)&dir_pages[pos];

  return 1;
}

void cpu_idle(void)
{
  __asm__ __volatile__("sti"
                       :
                       :
                       : "memory");

  while (1)
  {
    ;
  }
}

int remaining_quantum = 0;
int remaining_thread_quantum = 0;

int get_quantum(struct task_struct *t)
{
  return t->total_quantum;
}

void set_quantum(struct task_struct *t, int new_quantum)
{
  t->total_quantum = new_quantum;
}

int get_thread_quantum(struct thread_struct *t)
{
  return t->total_quantum;
}

void set_thread_quantum(struct thread_struct *t, int new_quantum)
{
  t->total_quantum = new_quantum;
}

struct task_struct *idle_task = NULL;

void update_sched_data_rr(void)
{
  remaining_thread_quantum--;
}

int needs_task_sched_rr(void)
{
  if ((remaining_quantum == 0) && (!list_empty(&(readyqueue))))
    return 1;
  if (remaining_quantum == 0)
    remaining_quantum = get_quantum(current());
  return 0;
}

int needs_thread_sched_rr(void)
{
  if ((remaining_thread_quantum == 0) && (!list_empty(&(current()->readythreadqueue))))
  {
    --remaining_quantum;
    return 1;
  }
  if (remaining_thread_quantum == 0)
  {
    --remaining_quantum;
    remaining_thread_quantum = get_thread_quantum(current_thread());
  }
  return 0;
}

void update_task_state_rr(struct task_struct *t, struct list_head *dst_queue)
{
  if (t->state != ST_RUN)
    list_del(&(t->list));
  if (dst_queue != NULL)
  {
    list_add_tail(&(t->list), dst_queue);
    if (dst_queue != &readyqueue)
      t->state = ST_BLOCKED;
    else
    {
      update_stats(&(t->p_stats.system_ticks), &(t->p_stats.elapsed_total_ticks));
      t->state = ST_READY;
    }
  }
  else
    t->state = ST_RUN;
}

void update_thread_state_rr(struct thread_struct *th, struct list_head *dst_queue)
{
  if (th->state != ST_RUN)
    list_del(&(th->list));
  if (dst_queue != NULL)
  {
    list_add_tail(&(th->list), dst_queue);
    if (dst_queue != &(current()->readythreadqueue))
      th->state = ST_BLOCKED;
    else
    {
      // TODO: update stats
      th->state = ST_READY;
    }
  }
  else
    th->state = ST_RUN;
}
void sched_next_task_rr(void)
{
  struct list_head *e;
  struct task_struct *t;

  if (!list_empty(&readyqueue))
  {
    e = list_first(&readyqueue);
    list_del(e);

    t = list_head_to_task_struct(e);
  }
  else
    t = idle_task;
  t->state = ST_RUN;
  remaining_quantum = get_quantum(t);

  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
  update_stats(&(t->p_stats.ready_ticks), &(t->p_stats.elapsed_total_ticks));
  t->p_stats.total_trans++;

  task_switch((union task_alineate *)t);
}

void sched_next_thread_rr(void)
{
  struct list_head *e;
  struct thread_struct *th;

  if (!list_empty(&(current()->readythreadqueue)))
  {
    e = list_first(&(current()->readythreadqueue));
    list_del(e);

    th = list_head_to_thread_struct(e);
  }
  else
  {
    return;
    // TODO:TASK_SWITCH?
  }
  th->state = ST_RUN;
  remaining_thread_quantum = get_thread_quantum(th);

  // TODO:STATS
  thread_switch((union thread_union *)th);
}
void schedule()
{
  update_sched_data_rr();
  if ((remaining_thread_quantum == 0) || (remaining_quantum == 0))
  { // needs change
    if (needs_thread_sched_rr())
    { // thread_switch
      update_thread_state_rr(current_thread(), &(current()->readythreadqueue));
      sched_next_thread_rr();
    }
    else if (needs_task_sched_rr())
    { // task_switch
      update_task_state_rr(current(), &readyqueue);
      update_thread_state_rr(current_thread(), &(current()->readythreadqueue));
      sched_next_task_rr();
    }
    else if ((remaining_thread_quantum == 0) || (remaining_quantum == 0))
    { // needs change
      printk("WTF\n");
    }
  }
}

#define DEFAULT_QUANTUM 10

void init_idle(void)
{
  struct list_head *l = list_first(&freequeue);
  list_del(l);
  struct task_struct *c = list_head_to_task_struct(l);
  // union task_union *uc = (union task_union*)c; No existeix

  struct list_head *lt = list_first(&freethreadqueue);
  list_del(lt);
  struct thread_struct *ts = list_head_to_thread_struct(lt);
  union thread_union *tu = (union thread_union *)ts;

  c->PID = 0;

  c->total_quantum = DEFAULT_QUANTUM;

  init_stats(&c->p_stats);

  allocate_DIR(c);

  INIT_LIST_HEAD(&(c->readythreadqueue));
  INIT_LIST_HEAD(&(c->zombiethreadqueue));

  ts->TID = 0;
  c->threads[0] = 1;

  // ts->PCB = c;
  ts->PID = c->PID;

  ts->total_quantum = DEFAULT_QUANTUM;

  tu->sys_stack[KERNEL_STACK_SIZE - 1] = (unsigned long)&cpu_idle; /* Return address */
  tu->sys_stack[KERNEL_STACK_SIZE - 2] = 0;                        /* register ebp */

  ts->register_esp = (int)&(tu->sys_stack[KERNEL_STACK_SIZE - 2]); /* top of the stack */

  list_add_tail(&(ts->list), &(c->readythreadqueue));

  idle_task = c;
}

void setMSR(unsigned long msr_number, unsigned long high, unsigned long low);

void init_task1(void)
{
  struct list_head *l = list_first(&freequeue);
  list_del(l);
  struct task_struct *c = list_head_to_task_struct(l);

  struct list_head *lt = list_first(&freethreadqueue);
  list_del(lt);
  struct thread_struct *ts = list_head_to_thread_struct(lt);
  union thread_union *tu = (union thread_union *)ts;

  c->PID = 1;

  c->total_quantum = DEFAULT_QUANTUM;

  c->state = ST_RUN;

  remaining_quantum = c->total_quantum;

  init_stats(&c->p_stats);

  allocate_DIR(c);

  set_user_pages(c);

  INIT_LIST_HEAD(&(c->readythreadqueue));
  INIT_LIST_HEAD(&(c->readythreadqueue));

  ts->TID = 0;
  c->threads[0] = 1;

  // ts->PCB = c;
  ts->PID = c->PID;

  ts->total_quantum = DEFAULT_QUANTUM;

  remaining_thread_quantum = ts->total_quantum;

  tss.esp0 = (DWord) & (tu->sys_stack[KERNEL_STACK_SIZE]);
  setMSR(0x175, 0, (unsigned long)&(tu->sys_stack[KERNEL_STACK_SIZE]));

  // list_add_tail(&(ts->list), &(c->readythreadqueue));

  set_cr3(c->dir_pages_baseAddr);
}

void init_freequeue()
{
  int i;

  INIT_LIST_HEAD(&freequeue);

  /* Insert all task structs in the freequeue */
  for (i = 0; i < NR_TASKS; i++)
  {
    task[i].task.PID = -1;

    INIT_LIST_HEAD(&(task[i].task.readythreadqueue));
    INIT_LIST_HEAD(&(task[i].task.zombiethreadqueue));
    for (int j = 0; j < 10; j++)
    {
      task[i].task.threads[j] = 0;
    }
    list_add_tail(&(task[i].task.list), &freequeue);
  }
}

void init_freethreadqueue()
{
  int i;
  INIT_LIST_HEAD(&freethreadqueue);

  for (i = 0; i < NR_THREADS; i++)
  {
    thread[i].thread.TID = -1;
    // thread[i].thread.PCB = NULL;
    thread[i].thread.PID = -1;
    list_add_tail(&(thread[i].thread.list), &freethreadqueue);
  }
}

void init_semarray(void)
{
  int i;
  for (i = 0; i < SEM_SIZE; ++i)
  {
    sem_array[i].id = i;
    sem_array[i].pid_owner = -1;
    sem_array[i].count = 0;
  }
}

void init_sched()
{
  init_freequeue();
  init_freethreadqueue();
  INIT_LIST_HEAD(&readyqueue);
}

struct task_struct *current()
{
  //  int ret_value;-
  int x = current_thread()->PID;
  int i;
  for (i = 0; i < NR_TASKS; ++i)
  {
    if (task[i].task.PID == x)
      return &(task[i].task);
  }
  int ret_value;

  return (struct task_struct *)(((unsigned int)&ret_value) & 0xfffff000);

  //  return current_thread()->PCB;
}

struct thread_struct *current_thread()
{
  int ret_value;

  return (struct thread_struct *)(((unsigned int)&ret_value) & 0xffffe000); // alineate 4xKERNEL_STACK_SIZE
}

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return (struct task_struct *)((int)l & 0xfffff000);
}

struct thread_struct *list_head_to_thread_struct(struct list_head *l)
{
  return (struct thread_struct *)((int)l & 0xffffe000);
}

/* Do the magic of a task switch */
void inner_task_switch(union task_alineate *new)
{

  struct thread_struct *ths = list_head_to_thread_struct(list_first(&(new->task.readythreadqueue)));
  list_del(&(ths->list));
  union thread_union *thu = (union thread_union *)ths;
  thread_switch(thu);
  /* TLB flush. New address space */
}

void inner_thread_switch(union thread_union *new)
{
  /* Update TSS and MSR to make it point to the new stack */

  tss.esp0 = (int)&(new->sys_stack[KERNEL_STACK_SIZE]);
  setMSR(0x175, 0, (unsigned long)&(new->sys_stack[KERNEL_STACK_SIZE]));
  int i;
  for (i = 0; i < NR_TASKS; ++i)
  {
    if (task[i].task.PID == new->thread.PID)
      set_cr3(get_DIR(&(task[i].task)));
  }
  new->thread.state = ST_RUN;

  remaining_thread_quantum = get_thread_quantum(&(new->thread));

  switch_stack(&current_thread()->register_esp, new->thread.register_esp);
}

/* Force a task switch assuming that the scheduler does not work with priorities */
void force_task_switch()
{
  update_task_state_rr(current(), &readyqueue);
  update_thread_state_rr(current_thread(), &(current()->readythreadqueue));

  sched_next_task_rr();
}
