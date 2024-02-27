#include<unistd.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>


#define KEY1 1234


struct shm_area {
  unsigned short rd_index;  //read index for the circular buffer
  unsigned short wr_index;  //write index for  ""   """

  unsigned short buf_size_max; //max buffer size 
  char buf_area[50]; //this must be treated as circular buffer
  unsigned int used_slot_count;//no elements used in the circular buffer 

  //char my_straread[50][256];   //practice
};

union semun { //used with semctl() system call for initializing 
              //getting the current semaphore count values
  int val;
  unsigned short *array;
  // other fields are omitted

};

int stat_var;

int main()
{

  char value;
  int ret, ret1,ret2,status, shm_id1,sem_id2;
  struct sembuf sb1,sb2,sb3,sb_array[3];

  unsigned short ary1[3],ary[] = { 50,1,0};  

  union semun u1;

  struct shm_area *shma ;


  printf("the address that faults is %x\n", shma); 

  shm_id1 = shmget(KEY1,sizeof(struct shm_area),IPC_CREAT|0600);
                                         //the last field may be 0 ,
              				 //when the shared memory object
                                         //is already created 
  if(shm_id1<0 && errno != EEXIST) { 
              perror("error in creating the shared memory"); exit(1);
  }

  sem_id2 = semget(KEY1,3,IPC_CREAT | 0600);//with read/write permissions 

  if(sem_id2<0) {

                perror("error in semget");
                exit(2);
  }


  shma = shmat(shm_id1,0,0);  //address space is assigned by 
                        //kernel , flags are 0
                        // customize access to shared-area

  

  printf("the attached address is 0x%x\n", shma);
  printf("the amount of shm memory used is %d\n", \
	  sizeof(struct shm_area));
 


  printf("the actual values of the semaphores are %d, %d, %d\n", \
                    ary[0], ary[1], ary[2]); 


  u1.array = ary; //setting the array ptr in the union

  ret1 = semctl(sem_id2,0,SETALL,u1);//setting the semaphore values

  if(ret1<0) { perror("error in semctl setting"); exit(4); }

  u1.array = ary1;

  ret1 = semctl(sem_id2,0,GETALL,u1);//getting the semaphore values


  if(ret1<0) { perror("error in semctl getting"); exit(5); }


  printf("the actual values of the semaphores are %d, %d, %d\n", \
                    ary1[0], ary1[1], ary1[2]); 

  
  shma->rd_index = 0;   //set the rd_index of the  circular-buffer of a pipe to 0 
  shma->wr_index = 0;   //set the wr_index of the circular-buffer of a pipe to 0
  shma->buf_size_max = 50; //set the total-number of elements, in the circular-buffer 
  shma->used_slot_count = 0; //initially, all the buffers are free 
  
 
   //producer
   //
   while(1)
   {
    
    //wait for user-input and read the user-input - 

    //it is a blocking system-call - it will wait, for 
    //user-input 
    //
    ret = read(STDIN_FILENO,&value,1);  //this is good programming
    printf("the value is %c\n", value);
    //decrement freeslot counting semaphore using semop
    //and allocating a free semaphore
    //sb1.sem_num = 0;  //semaphore instance no.
    //sb1.sem_op = -1;  //decrement operation 
    //sb1.sem_flg = 0;  //not using flags
    //semop(sem_id2,&sb1,1);  
     
    
    //decrement mutual exclusion semaphore(index=1) using semop
    //entering critical section / atomic section
    sb2.sem_num = 1;  //-->we are accessing semaphore-index=1, in 
                      //   a semaphore-objecti
    sb2.sem_op = -1;  //-->we are decrementing 
    sb2.sem_flg = 0;  //??
    semop(sem_id2,&sb2,1); //
   
    //two semaphore operations done atomically
    //
    //sb_array[0].sem_num = 0;//preparing the decrement operation on 
    //sb_array[0].sem_op = -1;//semaphore 0 in the sem.object - 
                            //meaning, use a free slot in producer
    //sb_array[0].sem_flg = 0;

    //sb_array[1].sem_num = 1;   //decrement critical section semaphore 
    //sb_array[1].sem_op = -1;   //acquire semaphore lock !!!
    //sb_array[1].sem_flg = 0;
 
    //semop(sem_id2,sb_array,2);   

    // read(STDIN_FILENO,&value,1); //this is bad programming
    //add an item to the next free slot in 
    //in the circular buffer
    //
    //the below code-block will be CS11, in this program/producer-program
    //
    if(shma->used_slot_count < shma->buf_size_max) //start of CS11
    {
       shma->buf_area[shma->wr_index] = value; //we are filling a value 
       shma->wr_index = (shma->wr_index+1)%shma->buf_size_max;
       shma->used_slot_count++;
    } //end of CS11
    //two semaphore operations done atomically
    //
    //sb_array[2].sem_num = 1;  //critical section semaphore
    //sb_array[2].sem_op = +1;  //releasing critical section semaphore  
    //sb_array[2].sem_flg = 0;

    //sb_array[1].sem_num = 2;  //incrementing filled slots semaphore count 
    //sb_array[1].sem_op = +1;  //meaning, this semaphore maintains the count
                              //of filled slots - this is use ful in the
                              //context of consumer 
    //sb_array[1].sem_flg = 0;
 
    //semop(sem_id2,&sb_array[1],2);   

    //increment  mutual exclusion semaphore using semop
    //we are leaving the critical section
    sb2.sem_num = 1;  //we are operating on the second semaphore
    sb2.sem_op = +1;  //increment
    sb2.sem_flg = 0;
    semop(sem_id2,&sb2,1); 

    //increment filled slot counting semaphore using semop
    //
    //sb1.sem_num = 2;
    //sb1.sem_op = +1;
    //sb1.sem_flg = 0;
    //semop(sem_id2,&sb1,1);

    //sched_yield();  //optional - may be used, if needed
   }//loop	  


   exit(0);

}




