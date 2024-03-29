#include<unistd.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>


#define KEY1 1234


//in the below example, we are implementing an user-space pipe
//mechanism - for those who are familiar with Unix/other operating
//systems, system space pipe ipc is very popular !!!

//in user-space pipe and system space pipe, the ipc buffer used 
//to store/forward data is a circular buffer - you will understand
//the implementation, if you look into the code below !!! 

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
  int ret,ret1,ret2,status, shm_id1,sem_id2;
  struct sembuf sb1,sb2,sb3,sb_array[3];

  //in this assignment/scenario, 3 semaphores are used in a semaphore object 
  //the usage of the semaphores are as below :

  // first-semaphore(index-0) will be used for counting free-slots
  // in the shared-memory, IPC-area  - this is a counting-semaphore type -
  // as discussed in 11_ipcs_2.txt,for more details -
  // speciality of  a counting-semaphore is, it can keep 
  // count of a set of identical-resources and also, block the 
  // current-process(producer), if the available,free-slots is 0(none) -
  // this effectively, is part of synchronization - 
  // in this context, the initial-value of this counting 
  // semaphore will be the max no. of free slots(no. of resources), in the 
  // IPC-object - in this context, it is 50 (will change, as per application set-up) 
  //
  //in addition, a counting semaphore also can do one more 
  //action - that is, it can wake up a blocked-process(producer), if 
  //the corresponding-resource is available (non zero) - 
  //unblocking-operation , is part of synchronization
  //
  //for the above functionality of a counting-semaphore to 
  //work properly, it is the developer's responsibility to 
  //initialize and operate upon the counting-semaphore,
  //appropriately,as per standard-design/programming-model !!!  
  //in our case, we are using the first- 
  //semaphore(index=0), to keep count of free-slots and to do this, 
  //we initialize
  //the first semaphore to 50 - 50 being the max no of free slots
  //in our implementation - when this changes, we can appropriately
  //change the code !!!
   
  //
  // second semaphore is used for mutual exclusion 
  // in the IPC area 
  // refer to 11_ipcs_2.txt - this is a binary semaphore - as 
  // decided by the developer !!! this semaphore is responsible 
  //for locking, in this example context !!!

  //
  //
  // third-semaphore(index=2) is used, for counting filled-slots 
  // in the IPC-area - this is again a counting-semaphore -
  // refer to 11_ipc_2.txt - this has all the characteristics 
  //of a counting semaphore and its initial value will be 
  //0 - initially, there are no filled slots  ....interpret 
  //its working, based on the semaphore, index-0's descriptions 
  //
 //initial value of counting semaphore managing 
 //free slots, in IPC is set to 50
 //initial value of binary/lock semaphore is set
 //to 1
 //initial value of counting semaphore managing
 //filled slots, in IPC is set to 0 
  unsigned short ary1[3],ary[] = { 50,1,0};  

  union semun u1;

  struct shm_area *shma ;


  printf("the address that faults is %x\n", shma); 

  //shma->buf_size_max = 100; 


  shm_id1 = shmget(KEY1,sizeof(struct shm_area),IPC_CREAT|0600);
                                         //the last field may be 0 ,
              				 //when the shared memory object
                                         //is already created 
  if(shm_id1<0 && errno != EEXIST) { 

              perror("error in creating the shared memory"); exit(1);
  }

  //we are creating 3 semaphores in a given semaphore object !!!

  //param2 is set to 3 such that 3 semaphores are created in 
  //a semaphore object !!!


  sem_id2 = semget(KEY1,3,IPC_CREAT | 0600);//with read/write permissions 

  if(sem_id2<0) {

                perror("error in semget");
                exit(2);
  }

  //last field may or may not be 0
  //add error checking
  //shma will now hold the starting virtual address of the 
  //virtual pages allocated for the shared-memory region
  //
  //internally, this will generate VADs, new page table entries
  //and connect VADs to shared memory object - all these objects
  //work together to ensure that shared memory mechanism works
  //appropriately !!!
  shma = shmat(shm_id1,0,0);  //address space is assigned by 
                        //kernel , flags are 0
                        // customize access to shared-area

  

  printf("the attached address is 0x%x\n", shma);
  printf("the amount of shm memory used is %d\n", \
	  sizeof(struct shm_area));
 


  printf("the actual values of the semaphores are %d, %d, %d\n", \
                    ary[0], ary[1], ary[2]); 
 //semctl() can be used with several commands - SETVAL is a command
 //used to set a single semaphore's value inside a semaphore object !!!
 //semctl() can also be used with SETALL - SETALL is a command 
 //used to set values of all semaphores, in a semaphore object !!!!
 //when you use SETALL with semctl(), second parameter is ignored -
 //meaning, you cannot use SETALL to initialize a semaphore !!! 
 // 
 //sem_id2 is the first parameter
 //second parameter is ignored 
 //third parameter is the command to set the values of all semaphores
 //in the semaphore object !!!
 //fourth param is pointer to an array of unsigned short elements-
 //each unsigned short element contains corresponding semaphore's
 //initial value - no of elements in this array must equal to
 //total no of semaphore elements in the semaphore object
 //array field of union is used for SETALL command - array field 
 //is used to point to an array of unsigned short elements - 
 //each element in the array initializes one semaphore in the semaphore
 //object !!! 

  u1.array = ary; //setting the array ptr in the union

  ret1 = semctl(sem_id2,0,SETALL,u1);//setting the semaphore values

  if(ret1<0) { perror("error in semctl setting"); exit(4); }

  u1.array = ary1;

  ret1 = semctl(sem_id2,0,GETALL,u1);//getting the semaphore values


  if(ret1<0) { perror("error in semctl getting"); exit(5); }


  printf("the actual values of the semaphores are %d, %d, %d\n", \
                    ary1[0], ary1[1], ary1[2]); 

  
  shma->rd_index = 0; 
  shma->wr_index = 0;
  shma->buf_size_max = 50;  
  shma->used_slot_count = 0; 
  
 
   //producer program's code, in a processi
   //
   while(1)
   {
    //
    //
    // 
    //wait for user input and read the user-input
    //it is a blocking system call
    //
    //since this is a blocking system call APi, you cannot invoke 
    //this system call APi, after decrementing a semaphore or 
    //locking a lock - since, after decrementing a semaphore or 
    //taking a lock, blocking is a very bad programming practice
    //
    //this process/thread will trouble other processes/threads, 
    //like there can a starvation or dead-lock ??
    // 
    //
    ret = read(STDIN_FILENO,&value,1);  //this is good programming
    printf("the value is %c\n", value);
    //initial value of the first counting semaphore, index=0, 
    //is set to 50, as per the max. no. of free slots
    //decrement freeslot counting semaphore, index=0, using semop
    //and allocate a free buffer slot
    //if semaphore, index=0's value is +ve, it means, that there
    //are free slots - before using a free slot, we 
    //need to decrement the semaphore value by 1 - 
    //effectively, resource allocation/management - 
    //if there are no free slots, the current semaphore, index=0's 
    //value will be 0 - if the free slots drop to 0, 
    //this semop() will block the producer processi - 
    //effectively, resource allocation/management and
    //synchronization - visualize the above operations ??
    //also, visualize, what happens, if 
    //there is no support, for blocking operations, like above ?? 
    sb1.sem_num = 0;  //semaphore instance no. is 0
    sb1.sem_op = -1;  //decrement operation 
    sb1.sem_flg = 0;  //not using flags
    semop(sem_id2,&sb1,1); //effectively allocating/
                           //requesting a resource instance
                           //the number slots will eventually 
                           //be 0, after all the free-slots 
                           //are consumed - this will be
                           //reflected, in the semaphore 
    //once a free slot is available and allocated, 
    //we can decrement the binary semaphore/lock semaphore
    //
    // 
    //decrement mutual exclusion semaphore using semop
    //entering critical section / atomic section
    //
    //once we decrement binary/lock semaphore, we can 
    //access shared-data of IPC object 
    //
    sb2.sem_num = 1; //semaphore instance no. is 1
    sb2.sem_op = -1;
    sb2.sem_flg = 0;
    semop(sem_id2,&sb2,1); 
   
    //two semaphore operations done atomically
    //
    //sb_array[0].sem_num = 1;//preparing the decrement operation on 
    //sb_array[0].sem_op = -1;//semaphore 0 in the sem.object - 
                            //meaning, use a free slot in producer
    //sb_array[0].sem_flg = 0;

    //sb_array[1].sem_num = 0;   //decrement critical section semaphore 
    //sb_array[1].sem_op = -1;   //acquire semaphore lock !!!
    //sb_array[1].sem_flg = 0;
 
    //semop(sem_id2,sb_array,2);   

    // read(STDIN_FILENO,&value,1); //this is bad programming
    //add an item to the next free slot, in 
    // the circular buffer, if there are free-slots
    if(shma->used_slot_count < shma->buf_size_max)
    {
       shma->buf_area[shma->wr_index] = value;
       shma->wr_index = (shma->wr_index+1)%shma->buf_size_max;
       shma->used_slot_count++;
    }

    //in the following semaphore operations, a binary 
    //semaphore is incremented, as we are leaving 
    //the critical section of shared-data of IPC 
    //in addition, we are incrementing the filled slots 
    //semaphore, as we have transformed a free slot into 
    //a filled slot containing data - a new filled slot 
    //resource is added, so the filled semaphore,index=2's value  is 
    //incremented  
    
    //two semaphore operations done atomically
    //
    sb_array[2].sem_num = 1;  //critical-section semaphore, index-1
    sb_array[2].sem_op = +1;  //releasing critical-section semaphore  
    sb_array[2].sem_flg = 0;
    
    sb_array[1].sem_num = 2; //incrementing filled-slots semaphore, index-2's value 
    sb_array[1].sem_op = +1;  //meaning, this semaphore maintains the count
                              //of filled slots - this is useful, in the
                              //context of consumer 
    sb_array[1].sem_flg = 0;
 
    

    //semop(sem_id2,sb_array,2);  //wrong - incorrect 
    
    //we are doing two operations on different semaphore, 
    //simultaneously - there are practical benefits 
    //
    //
    semop(sem_id2,&sb_array[1],2);//correct    

    //increment  mutual exclusion semaphore using semop
    //we are leaving the critical section
    //sb2.sem_num = 1;  //we are operating on the second semaphore
    //sb2.sem_op = +1;  //increment
    //sb2.sem_flg = 0;
    //semop(sem_id2,&sb2,1); 

    //increment filled slot counting semaphore using semop
    //
    //sb1.sem_num = 2;   //increment semaphore, index=2
    //sb1.sem_op = +1;
    //sb1.sem_flg = 0;
    //semop(sem_id2,&sb1,1);

    //sched_yield();  //optional 
   }// while loop	  


   exit(0);

}




