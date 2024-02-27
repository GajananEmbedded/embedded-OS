#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include <sys/sem.h>
#include<sys/shm.h>


//we need to use unique KEY values, for creating
//a shared-memory IPC mechanism instance and
//a semaphore IPC mechanism instance
//
//
#define KEY1 1555   //this will be unique, in the range of keys of 
                    //shared-memory IPC 
#define KEY2 1555   //this will be unique, in the range of keys of 
                    //semaphore IPC 
 
struct shmarea {

  unsigned long count;
  unsigned long count1;
  unsigned long count2;
  unsigned long count3;
  unsigned long count4;

}; 


struct shmarea *shma; //we will store a shared-data segment's
                      //starting virtual-address, in this 
                      //variable, after a shared-memory 
                      //set-up  ??


union semun { //used with semctl() system call for initializing
              //getting the current semaphore count values
  int val;    //we will be using val field of an union, 
              //for SETVAL-command 
              
  unsigned short *array; //we will be using array-field of 
                         //the union, for SETALL-command
                 
                         // other fields are omitted, since
                         // we are not using 

};  




int main()
{

   int ret,ret1,ret2,status,id,sem_id;
  

   struct sembuf sembuf1; //sembuf1 is an user-space variable, 
                          //that is used to pass parmeters to 
                          //semop() - do not mix it with 
                          //          system-space semaphore-object 
   union semun u1; //an instance this union is passed to 
                   //semctl

   unsigned long int i=0, j=0;//these iteration variables
                              //used


     id =  shmget(KEY1, 2*2*getpagesize(), IPC_CREAT|0600);

   shma = shmat(id,0,0); 

   if(shma == (-1)) { perror("error in shmat"); exit(2); }


   sem_id = semget(KEY2,1,IPC_CREAT|0600); 
   if(sem_id <0) { perror("error in creating a semaphore"); exit(2); }

   u1.val = 1 ; //we are using val field of u1, to pass 
               //the initial value of a semaphore, as 
               //mentioned, in p2

   ret1 = semctl(sem_id,0,SETVAL,u1);//setting the semaphore values

   if(ret1<0) { perror("error in semctl setting"); exit(3); }

   ret1 = semctl(sem_id,0,GETVAL);//setting the semaphore values

   if(ret1<0) { perror("error in semctl getting"); exit(4); }

   printf("current semaphore value is %d\n", ret1); 


  printf("pointer address of shmA is %lx\n", shma);
  printf("pointer address of shma->count is %lx\n", &shma->count);
  printf("pointer address of shma->count1 is %lx\n", &shma->count1);
  printf("pointer address of shma->count2 is %lx\n", &shma->count2);

  shma->count =  3353; //this is the initial value  
  shma->count1 = 4353; //this is the initial value  
  shma->count2 = 5353; //this is the initial value  
  shma->count3 = 6353; //this is the initial value  
  shma->count4 = 7353; //this is the initial value 

 
//pause();
//while(1);
//sleep(10);

  while(i++<1){ //change this with a reasonable condition !!!

   ret = fork();

   if(ret<0){ 
           perror("error in fork"); 
           printf("the final value of i is %lu\n", i);
          
           //an example for normal termination, but not successful 
           exit(1); 
   }

   //parent context 
   if(ret>0){ 
	   printf("I am in parent process context\n"); 
           printf("in parent .. ppid is %lu ...and pid is %lu\n", 
		   getppid(),getpid());	   
         //struct sembuf object's elements 
         sembuf1.sem_num = 0; //semaphore-index value -
                              //we are operating on first-semaphore/index 0
         sembuf1.sem_op = -1; //we are doing a decrement-operation on 
                              //semaphore-instance-0 
         sembuf1.sem_flg = 0; //as always, flags are set to 0 - 
                              //initially, let us ignore flags

         if( ret1<0 ) { perror( "error in semop" ); exit(1); }



           while(j++ < 5000000) {   //--->CS11's code-block starts

                        shma->count++;     //ok
                        shma->count1++;
                        shma->count2++;
              //end of a critical section code block 
           }  //--->CS11's code-blocks ends
           sembuf1.sem_num = 0; //similar to above - semaphore index 0
           sembuf1.sem_op = +1; //increment operation on first semahore
           sembuf1.sem_flg = 0; //flags is 0

	   ret1= semop(sem_id,&sembuf1, 1);//incrementing a semaphore
           if(ret1<0) { perror("error in semop"); exit(5); }


printf("finished in parent\n"); 

           continue;
   }
   //it is  a fork() return value/ret
   if(ret==0) { 
	   printf("I am in child process context\n"); 
           printf("in child .. ppid is %lu ...and pid is %lu\n", 
		   getppid(),getpid());	   
//pause();	   
          //this is a simple shared-data access
         sembuf1.sem_num = 0;
         sembuf1.sem_op = -1;
         sembuf1.sem_flg = 0;
	 ret1= semop(sem_id,&sembuf1, 1);//decrementing a semaphore 
         if(ret1<0) { perror("error in semop"); exit(4); }

         while(j++<5000000) { //-->CS12's code-block starts
         //while(j++<5000) {
                shma->count--;
                shma->count1--;
                shma->count2--;

         }  //--->CS12's code-block ends
	 sembuf1.sem_num = 0;
         sembuf1.sem_op = +1;
         sembuf1.sem_flg = 0;
	 ret1= semop(sem_id,&sembuf1, 1);//incrementing a semaphore   
         if(ret1<0) { perror("error in semop"); exit(5); }
     
           exit(0); 
   }

 }//while
 

 if(ret>0)
 {
   while(1){ 


    ret = waitpid(-1,&status,0);
    if(ret>0){

    if(WIFEXITED(status))     //normal termination of the process
    {
       if(WEXITSTATUS(status) == 0){ //normal and successfull
       }
       else{//normal, but not successfull
           //you may have to set a flag to be used later 
       }
    }
    else{ //abnormal (did not succeed)
        //you may have to set a flag here to be used later
    }
   }//ret>0

    if(ret<0) { 
           //pause();

     printf("final value of shared counter is %d\n", shma->count); 
     printf("final value of shared counter1 is %d\n", shma->count1); 
     printf("final value of shared counter2 is %d\n", shma->count2); 
           shmctl(id,IPC_RMID,0); //destroy the shared memory object !!!
    //printf("final value of shared counter is %d\n", shma->count); 
           exit(0); 

    }
   } 
  }
return 0;
   
}


