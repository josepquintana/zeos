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

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
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

