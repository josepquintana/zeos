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
		case 1:
			errmsg = "Operation not permitted";
			break;
		case 3:
			errmsg = "No such process";
			break;
		case 5:
			errmsg = "I/O error";
			break;
		case 9:
			errmsg = "Bad file descriptor";
			break;
		case 12:
			errmsg = "Out of memory";
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

