#include <libc.h>
#include <pthread.h>

char buff[24];

int pid;

void *start_routine(void *arg) {
  write(1, "I'm a new thread\n", 17*sizeof(char));
  // while(1) { }
}
void  *bye_routine(void *arg) {
  write(1, "I'm a new threadi, BYE\n", 23*sizeof(char));
  pthread_exit(0);
  while(1);
}


int __attribute__ ((__section__(".text.main"))) main(void)
{
  /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */


write(1, "\n", sizeof(char));

/* 
  // TEST 1:
 
  pthread_t tid;
  if(pthread_create(&tid, &start_routine, 0) == 0) {
    write(1, "OK -> new_tid: ", 15*sizeof(char));
    itoa(tid, buff);
    write(1, buff, strlen(buff));

  }
  else {
    write(1, "ERR: ", 5*sizeof(char));
    perror();
  }
  write(1,"\n", sizeof(char));
*/




/*
  // TEST 2:

  pthread_t tid;
  if(pthread_create(&tid, &bye_routine, 0) == 0) {
    write(1, "OK -> new_tid: ", 15*sizeof(char));
    itoa(tid, buff);
    write(1, buff, strlen(buff));
  }
  else {
    write(1, "ERR: ", 5*sizeof(char));
    perror();
  }
  write(1,"\n", sizeof(char));


*/


  // TEST 3:

  pthread_t tids[5];
  for(int i = 0; i< 5; ++i){
    if(i %2 == 1){
      if(pthread_create(&tids[i], &start_routine, 0) == 0) {
        write(1, "OK -> new_tid: ", 15*sizeof(char));
        itoa(tids[i], buff);
        write(1, buff, strlen(buff));
      }
      else {
        write(1, "ERR: ", 5*sizeof(char));
        perror();
      }
    }
    else {
      if(pthread_create(&tids[i], &bye_routine, 0) == 0) {
        write(1, "OK -> new_tid: ", 15*sizeof(char));
        itoa(tids[i], buff);
        write(1, buff, strlen(buff));
      }
      else {
        write(1, "ERR: ", 5*sizeof(char));
        perror();
      }
    }
    write(1,"\n", sizeof(char));
  }
  


/*
  // TEST 4:
  
  pthread_t tids[10];
  for(int i = 0; i< 10; ++i){
    if(pthread_create(&tids[i], &start_routine, 0) == 0) {
      write(1, "OK -> new_tid: ", 15*sizeof(char));
      itoa(tids[i], buff);
      write(1, buff, strlen(buff));
    }
    else {
      write(1, "ERR: ", 5*sizeof(char));
      perror();
    }
    write(1,"\n", sizeof(char));
  }

*/


/* 
  // TEST 5:

  if(fork() == 0){
    pthread_t tids[6];
    for(int i = 0; i< 6; ++i){
      if(pthread_create(&tids[i], &start_routine, 0) != 0) {
        write(1, "ERR: ", 5*sizeof(char));
        perror();
        write(1,"\n", sizeof(char));
      }
    }
  }
  else{
    if(fork() == 0){
      pthread_t tids[6];
      for(int i = 0; i<6; ++i){
        if(pthread_create(&tids[i], &start_routine, 0) != 0) {
          write(1, "ERR: ", 5*sizeof(char));
          perror();
          write(1,"\n", sizeof(char));
        }
      } 
    }
    else {
      pthread_t tids[6];
      for(int i = 0; i< 6; ++i){
        if(pthread_create(&tids[i], &start_routine, 0) != 0) {
          write(1, "ERR: ", 5*sizeof(char));
          perror();
          write(1,"\n", sizeof(char));
        }
      }
    }
  }
*/

  while(1) { }
}
