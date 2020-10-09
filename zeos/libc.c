/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;

void perror(void) 
{
	char *msg = "";
	switch(errno) {
		case 5:
			msg = "I/O error";
			break;
		case 9:
			msg = "Bad file descriptor";
			break;
		case 13:
			msg = "Permission denied";
			break;
		case 14:
			msg = "Bad address";
			break;
		case 22:
			msg = "Invalid argument";
			break;
		case 38:
			msg = "Function not implemented";
			break;
	}
	// write(1, msg, strlen(msg));
}

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

