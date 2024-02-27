#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>

#define KEY1 3333
#define BBUF_SZ 1024

//we can change the payload structure, as per our application's requirements,
//subject to OS rules..
struct payload{
  char file_data[1024]; //absolute pathnames
  char cli_np[1024];    //absolute pathnames
};


//first field will pass a message type,>=1,  a +ve value
//for our scenario, let us select 1
//second field of msgbuf must contain a data-buffer 
//we normally pass pointer to an instance of this data-type to the
//second field of msgsnd() 
struct msgbuf {

 long mtype; //a type field, which must be filled, with value >=1

 struct payload data;  //application-defined - 
                       //we need to copy our message into  this field 
};

int main(int argc,char *argv[])
{

   int ret1,ret2,id1,id2,npfdr;
   struct msgbuf msgbuf1;
   char *rdbuf;

   if(argc != 3) { printf("enter <absolue path of filename\
                           >and\
                        absolute path of client fifo name\n");
                   exit(1);
   }
   //for each unique KEY, there will be one message-queue
   //created and managed...
   //for now, we will be using a set of default flags - 
   //these default flags will do the job, for now...
   //an unique id is returned, for this message-queue - 
   //it will be used, as the handle to   
   //access the same message queue object 
   //- msgget() will return the id of a 
   // message-queue object,  after
   //creating a new message-queue object, in the system-space 
   //KEY1 and id /handle will uniquely 
   //describe the message queue object
   //use standard set of flags and ignore the details.....
   //
   id1 = msgget(KEY1,IPC_CREAT|0600);

   if(id1<0) { 
      perror("error in msgq creation"); 
      exit(2); //cannot create/set-up a message-queue, so
               //there will be a normal, but unsuccessful
               //termination ..... 
      }   

   //real client code starts
   rdbuf = malloc(BBUF_SZ);
   if(rdbuf==NULL) { printf("error in malloc"); exit(3);}


   msgbuf1.mtype = 1; //for this scenario, we use 1 
   //we must set the message type of a message in the message buffer
   //to a value >0 - can be 1,2,3,  as long as system supports
   //if we set it to 0, system will return error - we must use
   //a non-zero message type - this is as per rules !!

   strncpy(msgbuf1.data.file_data,argv[1],\
           sizeof(msgbuf1.data.file_data));
   ret1 = mkfifo(argv[2],0600); //we are creating a named-pipe file
                                //on the disk - this file is a special file
                                //it does not contain any data-blocks
                                //it has an inode associated with it
                                //it has a directory entry associated with it
                                //it has a different file type - known as
                                //named pipe file type
   if(ret1<0 && errno !=EEXIST)
   {
     perror("error in mkfifo"); 
     exit(4);
   }
   strncpy(msgbuf1.data.cli_np,argv[2],\
           sizeof(msgbuf1.data.cli_np));

   printf("cl1..the operation value is %d\n",msgbuf1.data.op);
   printf("cl1..the filename is %s\n",msgbuf1.data.file_data);
   printf("cl1..the client fifo name is %s\n",\
           msgbuf1.data.cli_np);

    //msgsnd() is the system call API used to send a message to a message queue
    //and add it to the list of messages   

   //in msgsnd() do the following - 
   //p1 - id must  be that of the target message-queue objecti - 
   //p2 - ptr to message-buffer related object, 
   //containing the user-space buffer  - this buffer will be
   //used to pass user-space data to the message-queue -  
   //first-field of the message-buffer object 
   //must contain non-zero message-type - 
   //other fields of the message-buffer object are developer-defined - 
   //third field of the msgsnd uses the size of data stored in the
   //payload of user-space buffer - size of the pay-load is passed - 
   //last field of the msgsnd is flags - for now, set it to 0 and ignore....-
   //ret value here is different from msgrcv() system call API -
   //here, ret value just returns success or failure !!!
   //if ret==0, success - if ret<0, failure - 
   //if msgsnd is successful, the data, in the pay-load of 
   //the message-buffer object will copied into a system- 
   //buffer and added to the message queue object - later, 
   //the data from system-buffer will be copied into 
   //user-space, when msgrcv() is initiated, in another 
   //process...
   ret1 = msgsnd(id1,&msgbuf1,sizeof(msgbuf1.data),0);
   if(ret1<0) { 
        perror("error in receiving message"); 
        exit(5);
   }
   //
   //
   npfdr = open(msgbuf1.data.cli_np,O_RDONLY); //this will block until
                                               //another process opens
                                               //this named pipe file
                                               //for write only
   //npfdr = open(msgbuf1.data.cli_np,O_WRONLY); //this will block until
                                               //another process opens
                                               //for read  only
   //npfdr = open(msgbuf1.data.cli_np,O_WRONLY); //this will block until
   //npfdr = open(msgbuf1.data.cli_np,O_RDWR); //wrong
   //npfdr = open(msgbuf1.data.cli_np,O_RDONLY|0600); //wrong
   //npfdr = open(msgbuf1.data.cli_np,O_RDONLY,0600); //wrong

   if(npfdr<0) { 
        perror("error in opening named pipe"); 
        exit(6);
   }

   do{
     //if you read from an unnamed pipe or named pipe,
     //read will return 0(EOF) only when there are no
     //write descriptors open for the pipe
     ret2 = read(npfdr,rdbuf,BBUF_SZ);
     if(ret2<0) { perror("error in read"); exit(7); }
     if(ret2==0) break;
     write(STDOUT_FILENO,rdbuf,ret2);
   } while(1);


exit(0);

}
        
           


      


    
    

