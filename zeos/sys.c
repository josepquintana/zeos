/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

typedef unsigned char pthread_t;

void *get_ebp();
int check_fd(int fd, int permissions)
{
  if (fd != 1)
    return -EBADF;
  if (permissions != ESCRIPTURA)
    return -EACCES;
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
  return -ENOSYS;
}

int sys_getpid()
{
  return current()->PID;
}

int global_PID = 1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent_task = NULL;
  struct list_head *lhcurrent_thread = NULL;

  // union task_union *uchild;
  union task_alineate *uchild_task;
  union thread_union *uchild_thread;

  /* Any free task_struct? */
  if (list_empty(&freequeue))
    return -ENOMEM;

  /* Any free thread_struct? */
  if (list_empty(&freethreadqueue))
    return -ENOMEM;

  /*Look if current task have 10 thread*/

  lhcurrent_task = list_first(&freequeue);
  lhcurrent_thread = list_first(&freethreadqueue);

  list_del(lhcurrent_task);
  list_del(lhcurrent_thread);

  // uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  uchild_task = (union task_alineate *)list_head_to_task_struct(lhcurrent_task);

  uchild_thread = (union thread_union *)list_head_to_thread_struct(lhcurrent_thread);

  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild_task, sizeof(union task_alineate));

  /* Copy the parent's thread struct to child's */
  copy_data(current_thread(), uchild_thread, sizeof(union thread_union));

  /* new pages dir */
  allocate_DIR((struct task_struct *)uchild_task);
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild_task->task);
  for (pag = 0; pag < NUM_PAG_DATA; pag++)
  {
    new_ph_pag = alloc_frame();
    if (new_ph_pag != -1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA + pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i = 0; i < pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA + i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent_task, &freequeue);

      /* Return error */
      return -EAGAIN;
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag = 0; pag < NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag = 0; pag < NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE + pag, get_frame(parent_PT, PAG_LOG_INIT_CODE + pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag = NUM_PAG_KERNEL + NUM_PAG_CODE; pag < NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag + NUM_PAG_DATA, get_frame(process_PT, pag));
    copy_data((void *)(pag << 12), (void *)((pag + NUM_PAG_DATA) << 12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag + NUM_PAG_DATA);
  }
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild_task->task.PID = ++global_PID;

  int register_ebp; /* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int)get_ebp();
  register_ebp = (register_ebp - (int)current_thread()) + (int)(uchild_thread);

  // uchild_task->task.register_esp=register_ebp + sizeof(DWord);
  uchild_thread->thread.register_esp = register_ebp + sizeof(DWord);

  DWord temp_ebp = *(DWord *)register_ebp;
  /* Prepare child stack for context switch */
  // uchild_task->task.register_esp-=sizeof(DWord);
  uchild_thread->thread.register_esp -= sizeof(DWord);
  // *(DWord*)(uchild_task->task.register_esp)=(DWord)&ret_from_fork;
  *(DWord *)(uchild_thread->thread.register_esp) = (DWord)&ret_from_fork;
  // uchild_task->task.register_esp-=sizeof(DWord);
  uchild_thread->thread.register_esp -= sizeof(DWord);
  // *(DWord*)(uchild_task->task.register_esp)=temp_ebp;
  *(DWord *)(uchild_thread->thread.register_esp) = temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild_task->task.p_stats));

  /* Set Thread ID and its PID */
  uchild_thread->thread.TID = 0;
  uchild_task->task.threads[0] = 1;
  uchild_thread->thread.PID = uchild_task->task.PID;

  /* Queue child process into readyqueue */
  uchild_task->task.state = ST_READY;
  list_add_tail(&(uchild_task->task.list), &readyqueue);

  /* Queue child process.thread into readythreadqueue */
  uchild_thread->thread.state = ST_READY;
  INIT_LIST_HEAD(&(uchild_task->task.readythreadqueue));
  INIT_LIST_HEAD(&(uchild_task->task.zombiethreadqueue));
  // uchild_task->task.readythreadqueue=uchild_thread->thread.list;
  list_add_tail(&(uchild_thread->thread.list), &(uchild_task->task.readythreadqueue));

  return uchild_task->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes)
{
  char localbuffer[TAM_BUFFER];
  int bytes_left;
  int ret;

  if ((ret = check_fd(fd, ESCRIPTURA)))
    return ret;
  if (nbytes < 0)
    return -EINVAL;
  if (!access_ok(VERIFY_READ, buffer, nbytes))
    return -EFAULT;

  bytes_left = nbytes;
  while (bytes_left > TAM_BUFFER)
  {
    copy_from_user(buffer, localbuffer, TAM_BUFFER);
    ret = sys_write_console(localbuffer, TAM_BUFFER);
    bytes_left -= ret;
    buffer += ret;
  }
  if (bytes_left > 0)
  {
    copy_from_user(buffer, localbuffer, bytes_left);
    ret = sys_write_console(localbuffer, bytes_left);
    bytes_left -= ret;
  }
  return (nbytes - bytes_left);
}

extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  for (i = 0; i < NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA + i);
  }

  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);

  current()->PID = -1;

  /* Restarts execution of the next process */
  sched_next_task_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;

  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats)))
    return -EFAULT;

  if (pid < 0)
    return -EINVAL;
  for (i = 0; i < NR_TASKS; i++)
  {
    if (task[i].task.PID == pid)
    {
      task[i].task.p_stats.remaining_ticks = remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

#define DEFAULT_QUANTUM 10 // ¿?

int sys_pthread_create(pthread_t *tid, void aux_routine(void *(*start_routine)(void *), void *), void *(*start_routine)(void *), void *arg)
{

  // Any free global thread_struct?
  if (list_empty(&freethreadqueue))
    return -EAGAIN;

  // Any free thread_struct in this task?
  int i;
  for (i = 0; i < 10 && current()->threads[i]; i++)
    ;
  if (i == 10)
    return -EAGAIN;

  // Get first available thread
  struct list_head *lh = list_first(&freethreadqueue);
  list_del(lh);
  struct thread_struct *ts = list_head_to_thread_struct(lh);
  union thread_union *tu = (union thread_union *)ts;

  copy_data(current_thread(), tu, sizeof(union thread_union));
  // Set TCB fields and increment thread counter for the current task

  // i from the begining
  ts->TID = i;
  current()->threads[i] = 1;
  // alocate mem
  int new_ph_pag;
  page_table_entry *process_PT = get_PT(current());

  new_ph_pag = alloc_frame();
  if (new_ph_pag != -1) /* One page allocated */
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_DATA + NUM_PAG_DATA + (ts->TID), new_ph_pag);
  }
  else /* No more free pages left. Deallocate everything */
  {
    /* Deallocate allocated pages. Up to pag. */
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + NUM_PAG_DATA + (ts->TID)));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA + NUM_PAG_DATA + (ts->TID));
    /* Deallocate task_struct */
    list_add_tail(&(ts->list), &freethreadqueue);
    /* Return error */
    return -EAGAIN;
  }

  unsigned long *u_stack;
  u_stack = (unsigned long *)((PAG_LOG_INIT_DATA + NUM_PAG_DATA + (ts->TID)) * 0x1000);
  unsigned long pag_s = PAGE_SIZE / sizeof(unsigned long);
  u_stack[pag_s - 1] = (unsigned long)arg;                                   // arg
  u_stack[pag_s - 2] = (unsigned long)start_routine;                         // start_routine
  u_stack[pag_s - 3] = (unsigned long)aux_routine;                           // aux_routine
                                                                             // ADD ARGS & FUNC
  tu->sys_stack[KERNEL_STACK_SIZE - 2] = (unsigned long)&u_stack[pag_s - 3]; // ESP (top stack)
  tu->sys_stack[KERNEL_STACK_SIZE - 5] = (unsigned long)aux_routine;         // EIP

  // mod sys_stack to return to the start_routine
  int register_ebp; /* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int)get_ebp();
  register_ebp = (register_ebp - (int)current_thread()) + (int)(tu);
  tu->thread.register_esp = register_ebp + sizeof(DWord);
  DWord temp_ebp = *(DWord *)register_ebp;
  /* Prepare child stack for context switch */
  tu->thread.register_esp -= sizeof(DWord);
  *(DWord *)(tu->thread.register_esp) = (DWord)&ret_from_fork;
  tu->thread.register_esp -= sizeof(DWord);
  *(DWord *)(tu->thread.register_esp) = temp_ebp;

  // Add new thread to the readythreadqueue of the current task
  ts->state = ST_READY;
  list_add_tail(&(ts->list), &(current()->readythreadqueue));

  // Store the TID of the new thread in the buffer pointer passed as a parameter
  *tid = (ts->TID);

  return 0;
}

void sys_pthread_exit(void *status)
{

  struct thread_struct *ts = current_thread();
  current()->threads[ts->TID] = 0;

  int is_last_thread = 1;
  for (int i = 0; i < sizeof(current()->threads); ++i)
  {
    if (current()->threads[i] == 1)
    {
      is_last_thread = 0;
      break;
    }
  }

  if (is_last_thread == 1)
  {
    /* Exit process if no more threads */
    sys_exit();
  }
  else
  {
    /* Exit only current_thread */
    page_table_entry *process_PT = get_PT(current());
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + NUM_PAG_DATA + (ts->TID)));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA + NUM_PAG_DATA + (ts->TID));

    /* REBENTA si els posem a 0, el current() del sched_next_thread_rr els necessita -> si es posa despres del sched_next_thread_rr no "hi arriba" mai */
    // ts->TID = -1;
    // ts->PID = -1;
    ts->retval = (int)&status;

    update_thread_state_rr(ts, &(current()->zombiethreadqueue));
    sched_next_thread_rr(); /* Restarts execution of the next thread */
  }
}

int sys_pthread_join(pthread_t tid, void **status)
{

  return 0;
}

int sys_sem_init(int *id, int count)
{
  int i;
  for (i = 0; i < SEM_SIZE; i++)
  {
    if (sem_array[i].pid_owner == -1)
    {
      *id = i;
      sem_array[i].pid_owner = current()->PID;
      sem_array[i].count = count;
      INIT_LIST_HEAD(&sem_array[i].sem_queue);
      return 0;
    }
  }
  return -EINVAL;
}

int sys_sem_wait(int id)
{
  int ret = 0;
  if (id < 0 || id >= SEM_SIZE)
    return -EINVAL;
  if (sem_array[id].pid_owner == -1)
    return -EINVAL;
  if (sem_array[id].count <= 0)
  {
    update_thread_state_rr(current_thread(), &(sem_array[id].sem_queue));
    sched_next_thread_rr();
  }
  else
    sem_array[id].count--;

  if (sem_array[id].pid_owner == -1)
    ret = -EAGAIN;

  return ret;
}

int sys_sem_post(int id)
{
  int ret = 0;

  if (id < 0 || id >= SEM_SIZE)
    return -EINVAL;
  if (sem_array[id].pid_owner == -1)
    return -EINVAL;
  if (list_empty(&sem_array[id].sem_queue))
    sem_array[id].count++;
  else
  {
    struct list_head *thread_list = list_first(&sem_array[id].sem_queue);
    list_del(thread_list);
    struct thread_struct *sem_thread = list_head_to_thread_struct(thread_list);
    update_thread_state_rr(sem_thread, &(current()->readythreadqueue));
  }

  return ret;
}

int sys_sem_destroy(int id)
{
  int ret = 0;

  if (id < 0 || id >= SEM_SIZE)
    return -EINVAL;
  if (sem_array[id].pid_owner == -1)
    return -EINVAL;
  if (current()->PID == sem_array[id].pid_owner)
  {
    sem_array[id].pid_owner = -1;
    while (!list_empty(&sem_array[id].sem_queue))
    {
      struct list_head *thread_list = list_first(&sem_array[id].sem_queue);
      list_del(thread_list);
      struct thread_struct *sem_thread = list_head_to_thread_struct(thread_list);
      update_thread_state_rr(sem_thread, &(current()->readythreadqueue));
    }
  }
  else
    ret = -EACCES;

  return ret;
}
