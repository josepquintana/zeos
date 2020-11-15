#include <libc.h>

char buff[24];

int pid;

/*
 *   Main entry point to USER AREA BLOCK
 */
int __attribute__ ((__section__(".text.main")))
  main(void)
{
	/* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
	/* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	int resAddASM = addASM(0x4, 0x6); resAddASM++;

	/* =============================================================== */
	
	char *msg;
	msg = "Writing from inside the user block\n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }

	/* =============================================================== */
	
	msg = "Clock ticks: ";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	int time = gettime();
	itoa(time, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	msg = "\n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }

	/* =============================================================== */

	msg = "Current PID: ";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	pid = getpid();
	itoa(pid, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	msg = "\n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }

	/* =============================================================== */

	msg = "Attempting to fork... \n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	int child_pid = fork();
	if(child_pid == 0) {
		msg = "Child PID: ";
		if(write(1, msg, strlen(msg)) == -1) { perror(); }
		itoa(getpid(), msg);
		if(write(1, msg, strlen(msg)) == -1) { perror(); }
		msg = "\n";
		if(write(1, msg, strlen(msg)) == -1) { perror(); }
	}

	/* =============================================================== */

	child_pid = fork();
	itoa(child_pid, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); } 
	print_new_line();

	child_pid = fork();
	itoa(child_pid, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	print_new_line();

	child_pid = fork();
	itoa(child_pid, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	print_new_line();

	child_pid = fork();
	itoa(child_pid, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	print_new_line();

	msg = "                                                                                                                                                                ================================================================================================================================================================                                                                                                                                                                ";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }

	while(1)
	{
		
	}
	
}

