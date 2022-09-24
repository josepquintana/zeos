/*
 * pthread.h - definició de les crides a sistema de POSIX Threads
 */

typedef unsigned char pthread_t;

int pthread_create(pthread_t* tid, void *(* start_routine) (void *), void* arg);

void pthread_exit(void* status);

int pthread_join(pthread_t tid, void **status);

void aux_routine(void *(*start_routine)(void *), void *arg);
