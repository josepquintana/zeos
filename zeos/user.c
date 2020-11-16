#include <libc.h>

// Writing Buffer Pointer
char *msg;

/*
 *   Main entry point to USER AREA BLOCK
 */
int __attribute__ ((__section__(".text.main"))) main(void)
{
	/* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
	/* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	int resAddASM = addASM(0x4, 0x6); resAddASM++;

	/* =============================================================== */
	
	msg = "Writing from inside the user block\n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }

	/* =============================================================== */
	
	msg = "\nClock ticks: ";
	write(1, msg, strlen(msg));
	int time = gettime();
	itoa(time, msg);
	write(1, msg, strlen(msg));
	write(1, "\n", 1);

	/* =============================================================== */

	msg = "\nMy PID: ";
	write(1, msg, strlen(msg));
	int pid = getpid();
	itoa(pid, msg);
	write(1, msg, strlen(msg));
	write(1, "\n", 1);

	/* =============================================================== */

	msg = "\nForking... \n";
	write(1, msg, strlen(msg));
	int child_pid = fork();

	if(child_pid == 0) {
		msg = "\nHello from Child! PID: ";
		write(1, msg, strlen(msg));
		itoa(getpid(), msg);
		write(1, msg, strlen(msg));
		write(1, "\n", 1);
	}
	else if(child_pid > 0) {
		msg = "\nHello from Parent! PID: ";
		write(1, msg, strlen(msg));
		itoa(getpid(), msg);
		write(1, msg, strlen(msg));
		msg = " | My child PID is ";
		write(1, msg, strlen(msg));		
		itoa(child_pid, msg);
		write(1, msg, strlen(msg));
		write(1, "\n", 1);
	}
	else { perror(); }

	/* =============================================================== */
	
	int st_pid = 100; // Process to analyze
	struct stats p_stats;
	if(getpid() == st_pid) {
		// Get statistical information
		if(get_stats(st_pid, &p_stats) == -1) { perror(); }

		msg = "\nStatistical Information\n==============================";
		write(1, msg, strlen(msg));	

		msg = "\n> PID: ";
		write(1, msg, strlen(msg));
		itoa(getpid(), msg);
		write(1, msg, strlen(msg));

		msg = "\n> User Ticks: ";
		write(1, msg, strlen(msg));
		itoa(p_stats.user_ticks, msg);
		write(1, msg, strlen(msg));

		msg = "\n> System Ticks: ";
		write(1, msg, strlen(msg));
		itoa(p_stats.system_ticks, msg);
		write(1, msg, strlen(msg));
		
		msg = "\n> Blocked Ticks: ";
		write(1, msg, strlen(msg));
		itoa(p_stats.blocked_ticks, msg);
		write(1, msg, strlen(msg));

		msg = "\n> Ready Ticks: ";
		write(1, msg, strlen(msg));
		itoa(p_stats.ready_ticks, msg);
		write(1, msg, strlen(msg));

		msg = "\n> Elapsed Total Ticks: ";
		write(1, msg, strlen(msg));
		itoa(p_stats.elapsed_total_ticks, msg);
		write(1, msg, strlen(msg));

		msg = "\n> Total Trans: ";
		write(1, msg, strlen(msg));
		itoa(p_stats.total_trans, msg);
		write(1, msg, strlen(msg));

		msg = "\n> Remaining Ticks: ";
		write(1, msg, strlen(msg));
		itoa(p_stats.remaining_ticks, msg);
		write(1, msg, strlen(msg));

		msg = "\n==============================\n";
		write(1, msg, strlen(msg));
	}

	/* =============================================================== */

	if(getpid() == 100) { 
		exit();
		msg = "\nProcess PID = 100 has exited\n";
		write(1, msg, strlen(msg));
		itoa(getpid(), msg);
		write(1, msg, strlen(msg));

		// Check that the process has exited succesfully
		msg = "\nThis line will never be printed because this process does not exist anymore\n";
		write(1, msg, strlen(msg));
	}

	/* =============================================================== */

	while(1) { }
	
}

