/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;

void perror(void) 
{
	char *errmsg = "";
	switch(errno) {
		case 5:
			errmsg = "I/O error";
			break;
		case 9:
			errmsg = "Bad file descriptor";
			break;
		case 13:
			errmsg = "Permission denied";
			break;
		case 14:
			errmsg = "Bad address";
			break;
		case 22:
			errmsg = "Invalid argument";
			break;
		case 38:
			errmsg = "Function not implemented";
			break;
		default:
			errmsg = "Unkown error code";
	}
	
	write(1, "\nPERROR: ", strlen("\nPERROR: "));
	write(1, errmsg, strlen(errmsg));
	write(1, "\n", strlen("\n"));
}

void print_current_pid()
{
	char *msg = "\nCurrent PID:\n> ";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	int pid = getpid();
	itoa(pid, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	msg = "\n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
}

void print_clock_ticks()
{
	char *msg = "\nClock ticks:\n> ";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	int time = gettime();
	itoa(time, msg);
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
	msg = "\n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
}

void print_new_line()
{
	char *msg = "\n";
	if(write(1, msg, strlen(msg)) == -1) { perror(); }
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

