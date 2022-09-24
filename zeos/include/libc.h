/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

extern int errno;

int write(int fd, char *buffer, int size);

int gettime();

void perror(void);

int addASM(int par1, int par2);

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

void exit();

int get_stats(int pid, struct stats *st);

#endif  /* __LIBC_H__ */
