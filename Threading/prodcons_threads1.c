#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<sched.h>
#include<sys/time.h>
#include<sys/resource.h>

static buffer_t buffer[5][BUFSIZE];
static buffer_t user_input_buffer[1024];
static pthread_mutex_t  input_buffer_lock   = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  data_buffer_lock    = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  display_buffer_lock = PTHREAD_MUTEX_INITIALIZER;
static int bufin = 0;
static int bufout = 0;
static int freeslots = 0;
static int filledslots = 0;
static volatile sig_atomic_t initdone = 0;
static int initerror = 0;
static pthread_once_t initonce = PTHREAD_ONCE_INIT;
static sem_t semitems;
static sem_t semslots;
static sem_t seminput;
static int bufferinit(void) { /* called exactly once by getitem and putitem  */
   int error;
   if (sem_init(&semitems, 0, 0))
      return errno;
   if (sem_init(&semslots, 0, 5)) {
      error = errno;
      sem_destroy(&semitems);
   }                                      /* free the other semaphore */
   if (sem_init(&seminput, 0, 0)) {
      error = errno;
      sem_destroy(&semslots);      return error;
      sem_destroy(&semitems);      return error;
   }
   return 0;
}
static void initialization(void) {
   initerror = bufferinit();
   if (!initerror)
      initdone = 1;
}
static int bufferinitonce(void) {          /* initialize buffer at most once */
   int error;
   if ((error = pthread_once(&initonce, initialization)) )
      return error;
   return initerror;
}

int getitem(buffer_t *item) {  /* remove item from buffer and put in *itemp */
   int error;
   if (!initdone)
      bufferinitonce();
   while (((error = sem_wait(&semitems)) == -1) && (errno == EINTR)) ;
   if (error)
      return errno;
   if ((error = pthread_mutex_lock(&data_buffer_lock))) //enter critical-section
      return error; 
   strncpy(item,buffer[bufout],1024);  //copy string
   //*itemp = buffer[bufout];
   bufout = (bufout + 1) % 5;

   filledslots--;
   freeslots++;  

   printf("2..get item \n");
   if ((error = pthread_mutex_unlock(&data_buffer_lock))) //exit critical-section
      return error;
   if (sem_post(&semslots) == -1)  //once we have used a filled-slot, we are freeing 
                                   //a slot - so, we increment free-slots, semaphore
      return errno; 
   return 0; 
}

int putitem(buffer_t *item) {                    /* insert item in the buffer */
   int error;
   if (!initdone)
      bufferinitonce();

   while (((error = sem_wait(&semslots)) == -1) && (errno == EINTR)); //decrement free-slots semaphore
   if (error)
      return errno;
   if ((error = pthread_mutex_lock(&data_buffer_lock))) //enter critical-section, 
                                                        //after locking a mutex-lock
      return error;    
   //buffer[bufin] = item;

   strncpy(buffer[bufin],item,1024);  //copy string
   bufin = (bufin + 1) % 5;
   
   filledslots++;
   freeslots--; 
   printf("1..put item \n");
   if ((error = pthread_mutex_unlock(&data_buffer_lock))) //exit critical-section, 
                                                          //after unlocking a mutex-lock
      return error; 
   if (sem_post(&semitems) == -1)  //increment filled-slots semaphore, by 1
      return errno; 
   return 0; 
}


//
//
void *user_input_thread(void *arg)
{  
   int ret;
   char pdatabuf[1024];
   while(1){
     printf("please input a maximum of 1023 character string - :)");
     fflush(stdout);
     ret = fgets(pdatabuf,sizeof(pdatabuf),stdin);    //produce
     if(ret == 0) exit(3); //fatal error
     pthread_mutex_lock(&input_buffer_lock);
     strncpy(user_input_buffer,pdatabuf,sizeof(user_input_buffer));
     pthread_mutex_unlock(&input_buffer_lock);
     sem_post(&seminput);
 //increment operation 
 //sched_yield();


   }   

pthread_exit(NULL);

}


void *producer_thread(void *arg)
  //producer thread 
                                    //invokes putitem() method 
{
  
   int ret;
   char pdatabuf[1024];


   while(1){

     //fgets(pdatabuf, sizeof(pdatabuf), stdin);                 //produce

     sem_wait(&seminput);     
//decrement operation

     pthread_mutex_lock(&input_buffer_lock);

     ret = putitem(user_input_buffer);

     pthread_mutex_unlock(&input_buffer_lock);

     if(ret != 0) exit(3); //fatal error

   }   

pthread_exit(NULL);

} 

//
//
void *consumer_thread(void *arg) //this is the consumer thread, 
                                 //which invokes getitem() 
{
    
   int ret;
   char cdatabuf[1024];

   while(1){

        ret = getitem(cdatabuf);                             //consume
        if(ret != 0) exit(4); //fatal error
        //printf("the received data is %s\n", cdatabuf);
   }

pthread_exit(NULL);

} 

void *display_status_thread(void *arg)
{
    
   int ret;
   char cdatabuf[1024];

   sigset_t set1;

   sigemptyset(&set1);
   sigaddset(&set1,SIGINT);
   sigaddset(&set1,SIGTERM);

   setpriority(PRIO_PROCESS, syscall(SYS_gettid) , +19); 

   pthread_sigmask(SIG_UNBLOCK,&set1,NULL);

   while(1){

        pthread_mutex_lock(&data_buffer_lock);

        pthread_mutex_unlock(&data_buffer_lock);
   }

pthread_exit(NULL);

}

pthread_t thid1,thid2,thid3,thid4;
int main(){
   int ret;
   void *ptr = NULL;
   sigset_t set1,set2;
   struct sched_param param1;
   pthread_attr_t pthread_attr1,pthread_attr4;
   sigfillset(&set1);
   sigprocmask(SIG_BLOCK,&set1,&set2);
   ptr = malloc(32*1024);
   if(ptr!=NULL){
      printf("setting the stack attributes of a thread\n"); 
      ret = pthread_attr_init(&pthread_attr4); 
      if(ret>0) { 
              printf("error in the thread attr initialization\n");  
      }
      //initialization of the stack attributes in the thread attribute obj.
      ret = pthread_attr_setstack(&pthread_attr4,ptr,32*1024);
      if(ret>0) {
              printf("error in the stack attribute settings\n");
      }
   }
   ret = pthread_create(&thid4,&pthread_attr4,display_status_thread,NULL);
   if(ret>0) { printf("error in thread creation for producer\n"); exit(4); }   
   
   pthread_attr_init(&pthread_attr1); 
   pthread_attr_setschedpolicy(&pthread_attr1,SCHED_FIFO);
   param1.sched_priority = 1;
   pthread_attr_setschedparam(&pthread_attr1, &param1);  
   pthread_attr_setinheritsched(&pthread_attr1,PTHREAD_EXPLICIT_SCHED); 
   
   ret = pthread_create(&thid1,&pthread_attr1,user_input_thread,NULL);
   if(ret>0) { printf("error in thread creation for input thread\n"); exit(1); }

   ret = pthread_create(&thid2,NULL,consumer_thread,NULL);
   if(ret>0) { printf("error in thread creation for consumer\n"); exit(2); } 

   ret = pthread_create(&thid3,NULL,producer_thread,NULL);
   if(ret>0) { printf("error in thread creation for producer\n"); exit(3); } 

   pthread_join(thid1,NULL);   
   pthread_join(thid2,NULL);
   pthread_join(thid3,NULL);
   pthread_join(thid4,NULL);
   exit(0);
}






