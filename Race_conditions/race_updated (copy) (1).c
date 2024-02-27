#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include <sys/sem.h>
#include<sys/shm.h>



#define KEY1 1555
#define KEY2 1555


struct shmarea {   //this is a shared-data object, 
                   //in a shared-segment, in user-space

  unsigned long count;     //a shared-data variable
  unsigned long count1;    //another, shared-data variable
  unsigned long count2;    //""""
  unsigned long count3;    //""""
  unsigned long count4;    //""""""""

}; 


struct shmarea *shma; //we will store a shared-data segment's
                      //starting virtual address, in this 
                      //variable ??


union semun { //used with semctl() system call for initializing
              //getting the current semaphore count values
  int val;
  unsigned short *array;
  // other fields are omitted

};  




int main()
{

   int ret,ret1,ret2,status,id,sem_id;
  

   struct sembuf sembuf1; 
   union semun u1; //an instance this union is passed to 
                   //semctl

   unsigned long int i=0, j=0;//these iteration variables
                              
     id =  shmget(KEY1,2*2*getpagesize(),IPC_CREAT|0600);
     if(id <0) { perror("error in shmget"); exit(5); } 

   shma = shmat(id,0,0); 

 
  shma->count = 5533; //this is the initial value  
  shma->count1 = 6533; //this is the initial value  
  shma->count2 = 7533; //this is the initial value  
  shma->count3 = 8533; //this is the initial value  
  shma->count4 = 9533; //this is the initial value  

  while(i++<1){ //change this with a reasonable condition !!!

   ret = fork();

   if(ret<0){ //fork() failed 
              // 
           perror("error in fork"); 
           printf("the final value of i is %lu\n", i);
          
           //an example for normal termination, but not successful 
           exit(1); 
   }

   //parent context - a block of code  
   if(ret>0){ 
	   printf("I am in parent process context\n"); 
           printf("in parent .. ppid is %lu ...and pid is %lu\n", 
		   getppid(),getpid());	   

           while(j++ < 6000000) {
              //start of a critical section code block
              //shared-data access  
                        shma->count++;     //ok
                        shma->count1++;
                        shma->count2++;
              //end of a critical section code block 
           }
 
printf("finished in parent\n"); 

           continue;
   }
   //it is  a fork() return value/ret
   if(ret==0) { //child instance 
	   printf("I am in child process context\n"); 
           printf("in child .. ppid is %lu ...and pid is %lu\n", 
		   getppid(),getpid());	   
 
         while(j++<6000000) {
                shma->count--;
                shma->count1--;
                shma->count2--;

         }
      
           exit(0); 
   }

 }//while


 if(ret>0)
 {
   while(1){ //this while(1) is ok - it has been used with a clear purpose
             //it will break when a certain condition is true - see below !!! 

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
    //we may use break instead of exit(0) 
    if(ret<0) { 
           //pause();

     printf("final value of shared counter is %d\n", shma->count); 
     printf("final value of shared counter1 is %d\n", shma->count1); 
     printf("final value of shared counter2 is %d\n", shma->count2); 
           exit(0); 

    } //no child is in any state for this process
                           //all the children have terminated and 
                           //cleaned-up by the parent process
   } //second while 
  }//if after while loop 

return 0;
   
}


