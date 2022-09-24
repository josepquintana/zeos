
#include <list.h>

struct sem_t {
	int id;
	int count;
	int pid_owner;
	struct list_head sem_queue;
};