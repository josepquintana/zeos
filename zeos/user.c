#include <libc.h>

char buff[24];

int pid;


int add(int par1, int par2);

void josep();

/*
 *   Main entry point to USER AREA BLOCK
 */
int __attribute__ ((__section__(".text.main")))
  main(void)
{
	/* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
	/* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	//int res = add(0x42, 0x666);
	
	//int resASM = addASM(0x4, 0x6);
	
	josep();
    
	while(1) { }
}

int add(int par1, int par2) {
	return par1 + par2;
}

void josep() {
	int a = 4;
	a++;
}
