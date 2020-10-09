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
	
	char * msg;
	msg = "Writing from inside the user block\n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	
	int time = gettime();
	itoa(time, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	
	while(1) { }
	
}

