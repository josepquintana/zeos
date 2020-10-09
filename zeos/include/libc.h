/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

int writeIDT(int fd, char *buffer, int size);

int write(int fd, char *buffer, int size);

void perror(void);

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

void exit();

#endif  /* __LIBC_H__ */
