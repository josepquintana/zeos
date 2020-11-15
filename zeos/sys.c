/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1

#define WRITE_BUFFER 256

/* ZeOS Ticks variable */
extern int zeos_ticks;

/* Last PID allocated ("/proc/sys/kernel/ns_last_pid" in Linux). This variables serves as a global PID counter */
int last_pid = 99;

/* Return from fork for the child process */
int ret_from_fork()
{
	return 0;
}

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}


/************************************************************/

/*
* SYS_WRITE
*
* fd: file descriptor. In this delivery it must always be 1.
* buffer: pointer to the bytes.
* size: number of bytes.
* return -> Negative number in case of error (specifying the kind of error) and the number of bytes written if OK.
*
*/
int sys_write(int fd, char * buffer, int size) 
{
	int error_fd = check_fd(fd, ESCRIPTURA);
	if(error_fd < 0) return error_fd; 
	
	if(buffer == NULL) return -14; /*EFAULT*/
	
	if(size < 0) return -22; /*EINVAL*/
	
	char local_buffer[WRITE_BUFFER];
	int i_size = size;
	int ret_swc = -5; /*EIO (initialization)*/
	
	while (i_size > WRITE_BUFFER) {
		copy_from_user(buffer, local_buffer, WRITE_BUFFER);
		ret_swc = sys_write_console(local_buffer, WRITE_BUFFER);
		i_size -= ret_swc;
		buffer += ret_swc;
	}
	if (i_size > 0) {
		copy_from_user(buffer, local_buffer, i_size);
		ret_swc = sys_write_console(local_buffer, i_size);
		i_size -= ret_swc;
	}
	
	return (size - i_size);
}


/*
* SYS_GETTIME
*/
int sys_gettime() 
{
	return zeos_ticks;
}


/*
* SYS_GETPID
*/
int sys_getpid()
{
	return current()->PID;
}


/*
* SYS_FORK
*
* return -> Negative number in case of error (specifying the kind of error), 0 if child or PID of the created process if parent.
*
* TODO: Never returning 0 to child....
*/
int sys_fork()
{
  	// Check if there's space for a new process, i.e. at least one task in the Free queue
  	if(list_empty(&freequeue) == 1) { return -12; /*ENOMEM*/ }

  	// Get a free task struct for the new process from the freequeue list. Then remove it from the list
  	struct list_head *ft = list_first(&freequeue);
	list_del(ft); 

	// Get a pointer to the memory of the retrieved free task which will be the child task
	struct task_struct *pcb_child = list_head_to_task_struct(ft);
	union task_union *task_child = (union task_union*) pcb_child;

	// Copy the parent's task_union (PCB part only) to the child
	copy_data(current(), task_child, sizeof(union task_union));

	// Initialize new directory base address for the child process 
	allocate_DIR(pcb_child); /* writes "dir_pages_baseAddr" field */

	// Get the Page Table address for the child process. (*In ZeOS only one PT per process*)
 	page_table_entry *page_table_child = get_PT(pcb_child); 

	// Allocate free physical pages (frames) to map the DATA+STACK logical pages of the child process and make the association
 	int new_frame;
  	for(int page = 0; page < NUM_PAG_DATA; page++) 
  	{
		new_frame = alloc_frame(); //  Returns the frame number or -1 if there isn't any frame available

		if(new_frame == -1) 
		{
			// Error => Rollback page allocation already done  
			for(int k = 0; k < page; k++) 
			{
				// Return current frame to "FREE_FRAME" status
				unsigned int frame_to_deallocate = get_frame(page_table_child, PAG_LOG_INIT_DATA + k);
				free_frame(frame_to_deallocate);

				// Removes mapping from current logical page 
				del_ss_pag(page_table_child, PAG_LOG_INIT_DATA + k);
			}

			// Add selected PCB to the Free queue again since it is not usable
			list_add_tail(&(pcb_child->list), &freequeue);	

			return -12; /*ENOMEM*/
		}
		else 
		{
			// Associate logical page with the new physical page (frame)
			set_ss_pag(page_table_child, PAG_LOG_INIT_DATA + page, new_frame);
		}
  	}

	// Map KERNEL and CODE page table entries to the child process by copyng the entries from the parent. (They are shared)
	unsigned int parent_frame;
	for(int page = 0; page < NUM_PAG_KERNEL; page++) 
	{
		parent_frame = get_frame(get_PT(current()), page);
		set_ss_pag(page_table_child, page, parent_frame);
	}

	for(int page = 0; page < NUM_PAG_CODE; page++) 
	{
		parent_frame = get_frame(get_PT(current()), PAG_LOG_INIT_CODE + page);
		set_ss_pag(page_table_child, PAG_LOG_INIT_CODE + page, parent_frame);
	} 

	/* ===========*/
	/* ¿? HELP ¿? */
	/* ===========*/
	// Copy the user DATA+STACK pages from the parent process to the child. To simultaneously access both pages and make the copy, it is required to 
	// temporally map the child physical pages to unused entries of the page table of the parent.
 	page_table_entry *page_table_parent = get_PT(current()); // Get the Page Table address for the parent process.
	for(int page = (NUM_PAG_KERNEL + NUM_PAG_CODE); page < (NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA); page++)
	{
		unsigned int tmp_page = page + NUM_PAG_DATA; // Last page ¿? of the Page Table for temporally mapping
		set_ss_pag(page_table_parent, tmp_page, get_frame(page_table_child, page));
		copy_data((void*) (page << 12), (void*) (tmp_page << 12), PAGE_SIZE);
		del_ss_pag(page_table_parent, tmp_page);
	}

	// Force a TLB flush to disable the parent's access to the child pages
	set_cr3(get_DIR(current() ));

	// Assign a new PID to the child process and increment the global PID counter
	pcb_child->PID = last_pid + 1;
	last_pid += 1;

	// Map the parent's EBP register position in the stack to the child process' PCB
	int parent_ebp_reg;
	__asm__ __volatile__ (
		"mov %%ebp, %0\n\t"
		: "=g" (parent_ebp_reg)
		: );

	parent_ebp_reg = parent_ebp_reg - (int) current() + (int) task_child; // ¿?
	pcb_child->ebp_reg_pos = parent_ebp_reg + sizeof(DWord); // ¿?
	
	/* ===========*/
	/* ¿? HELP ¿? */
	/* ===========*/
	// Prepare the stack with the content expected by "task_switch" to be able to restore it in the future when switching contexts. 
	// This is done by emulating the result of a call to "task_switch" 
	DWord aux_parent_ebp_reg = *(DWord*) parent_ebp_reg; // ¿?
	pcb_child->ebp_reg_pos -= sizeof(DWord);  // ¿?
	*(DWord*) pcb_child->ebp_reg_pos = (DWord) &ret_from_fork;
	pcb_child->ebp_reg_pos -= sizeof(DWord);
	*(DWord*) pcb_child->ebp_reg_pos = aux_parent_ebp_reg;

	// Insert new child process into the Ready queue and update its state since now it is ready to be assigned to the CPU when available
	list_add_tail(&(pcb_child->list), &readyqueue);	
	pcb_child->state = ST_READY;

	/* It is not necessary to set total quantum allowed for the child process since it inherits it from the parent process */

	// Initialize statistical information
	init_stats(&(pcb_child->p_stats));

  	// Return the PID of the newly created child process
  	return pcb_child->PID;
}


/*
 * SYS_EXIT
 */
void sys_exit()
{  
	// Get the Page Table address for the process to destroy. 
 	page_table_entry *page_table_destroy = get_PT(current()); 

 	// Deallocate all the used physical pages and remove mappings of logical pages of the process to destroy
 	for(int page = 0; page < NUM_PAG_DATA; page++)
 	{
 		free_frame(get_frame(page_table_destroy, PAG_LOG_INIT_DATA + page));
 		del_ss_pag(page_table_destroy, PAG_LOG_INIT_DATA + page);
 	}

 	// Add the process to destroy to the Free queue since its PCB will become unused
 	list_add_tail(&(current()->list), &freequeue);

 	// Invalidate PID and the directory base address of the destroyed process
 	current()->PID = -1;
 	current()->dir_pages_baseAddr = NULL;

 	// Schedule the execution of the next READY process and make a context switch
 	sched_next_rr();
}