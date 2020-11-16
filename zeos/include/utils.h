#ifndef UTILS_H
#define UTILS_H

void copy_data(void *start, void *dest, int size);
int copy_from_user(void *start, void *dest, int size);
int copy_to_user(void *start, void *dest, int size);

#define VERIFY_READ	0
#define VERIFY_WRITE	1
int access_ok(int type, const void *addr, unsigned long size);

#define min(a,b)	(a<b?a:b)

unsigned long get_ticks(void);

/* Function to update statistical information about the processes */
void update_p_stats(unsigned long *state_ticks, unsigned long *elapsed_total_ticks);

/* Functions to update statistical information when entering and leaving the system */
void update_statistics_user_to_system(void);
void update_statistics_system_to_user(void);

#endif
