#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>

#define KEY1 3333

#define BBUF_SZ 1024

struct payload{
 // char op;               //unused field
  char file_data[256];   //char message1[256];
  char cli_npipe[256];   //char message[256];
};



struct msgbuf {

 long mtype;           //must be this, always - used to hold message type 
                       //of received message or sent message !!!

 struct payload data;  //payload space -  can be changed, 
                       //as required by developer !!!
};

//this signal handler will execute asynchronously, when 
//there is a SIGCHLD signal notified to the parent process

void sig_chld(int signo)
{

  int ret1;

  while(1){
      //Note : signal handlers are not supposed to block
      //WNOHANG flag is added such that 
      //waitpid() will not block, if there are children and
      //none are in terminated(zombie) state - it will return 0
      //summary - if waitpid() is called with WNOHANG, ret values are:
      //+ve - if a child is cleaned-up
      //0   - if there are children and none are in zombie state
      //-1  - if there are no children in any state or there is an error
      ret1 = waitpid(-1,&status,WNOHANG);
      if(ret1<0) break;
      if(ret1==0) break;  //condition for non-blocking

  }

}      


int main()
{


   int ret,ret1,ret2,ret3,id1,id2,fd,npfd;
   struct msgbuf msgbuf1;
   struct payload pld1;
   struct sigaction act1,act2;
   sigset_t set1,set2;

   char *rdbuf;

   //install signal handler for SIGCHLD using sigaction

   sigfillset(&set1);                 //library call
   sigdelset(&set1,SIGCHLD);          //library call
   sigprocmask(SIG_SETMASK,&set1,&set2);//system call //mask is used outside signal 
                                                     //handlers

   //act1.sa_handler = SIG_DFL; //install the default action 
   //act1.sa_handler = SIG_IGN; //action will be to ignore 
   act1.sa_handler = sig_chld; //install a signal handler
   act1.sa_flags = 0;
   sigfillset(&act1.sa_mask);//mask used during the signal handler's execution only
   //you must call sigaction() for each signal that be specially 
   //handled by the system/process
   //sigaction() is a system call API to install a special 
   //signal handler, for a specific SIGNAL notification -
   //in this context, we are installing a special signal 
   //handler for SIGCHLD 

   sigaction(SIGCHLD,&act1,&act2);
   //sigaction(SIGINT,&act1,NULL);

   rdbuf = malloc(1024);
   if(!rdbuf) { 
     printf("error in malloc");
     exit(1);
   }

   ret = mkfifo(argv[1],0600);//create server fifo if needed 
   if(ret<0 && errno != EEXIST) { perror(""); exit(2); }
   while(1){ //server continues to serve
   npsfd = open(argv[1], O_RDONLY);   
   if(npsfd<0) { 
       temp_errno = errno; perror(""); if(temp_errno == EINTR) continue;  
       exit(3); }

   ret = read(npsfd,&pld1,sizeof(pld1));

   if(ret<0) { perror(""); exit(4); }
   
   if(ret==0) continue;  //end of file case is handled without redundant children
   //message has come
   close(npsfd);
   ret = fork(); //create a child per request

   if(ret<0) { perror(""); exit(5); }
   if(ret>0) { close(npsfd); continue; }

   if(ret==0) {
        close(npsfd);  //closing the server name pipe 
	ffd = open(pld1.file_data,O_RDONLY); //opening the data-file
        //add error checking
        npcfd = open(pld1.cli_npipe,O_WRONLY); //opening the npipe of client
        while( (ret=read(ffd,buf,sizeof(buf))) > 0){
                write(npcfd,buf,ret);
        }
        exit(0);
    }	 
  } //while loop

   //msgget() enables to create a new mq object 
   //KEY identifies an mq-object uniquely, in the system 
   //-->if we need to access a common, message-queue objecti, 
   //   we must pass the same KEY value to multiple programs/active 
   //   applications/processes - if invoked multiple-time, in 
   //   different-processes, msgget() will create a new, mq-objecti, 
   //   for the first-time, only - after that, it will just return 
   //   the id of the existing, mq-objecti 
   // 
   //flags are used to control the behaviour of msgget()
   //IPC_CREAT is needed to create a new mq object - if 
   //object does not exit, create - otherwise, use what exists !!!
   //0600 is the permissions in this context - we are allowing 
   //read/write permissions to the owner of this message queue -
   //currently, use what is given - we will understand more
   //during file permissions !! for now, let us use the default 
   //set of flags...
   //if msgget() is successful, returns id of the message queue
   //object !!!id/handle will be used for further operations, not KEY  !!!
  
   //the following msgget() will create a new message queue object,
   //if no object exists, with the KEY1 value - if a message-queue 
   //object already exists, with KEY1,  just returns id of the 
   //existing message queue object
   //--->all these rules apply to msgget(), in any process, using 
   //    a common message-queue objecti
   //
   //
     id1 = msgget(KEY1,IPC_CREAT|0600);

   if(id1<0) { 
      perror("error in msgq creation"); 
      exit(2); 
   }

   while(1)
   {
       //waitpid(-1,&status,0);     //wrong place -- 1
  
       //server-process waits for messages from clients - msgrcv() 
       //is a blocking system call API - until a message
       //is received, the current, server-process is blocked, in the
       //wq of this message-queue object 
       //
       //first-parameter is the id - we do not use KEY - that 
       //is the peculiarity of implementation !!
       //second-parameter is "pointer to an user-space message-
       //buffer object" - it must follow strict rules
       //of the system-call API - see the object defined above !!!
      //third parameter is the size of the payload in the 
      //message buffer related object - strict - must be followed 
      //fourth parameter is the type of message to be received - 
      //if it is 0, receive the oldest message of any type, 
      //in the message queue !!! if it is non-zero, we have the
      //flexibility to set the type of message of our interest - 
      //we will see more of this during sending - if we set to 
      //non-zero value, oldest message of that type will be received- 
      //otherwise, receiving process is blocked until the oldest 
      //message of the given type is received !!!
      //last field(fifth parameter) is flags - mostly set to 0, since
      //we are not using - 
      //- if needed, refer to man page - if this field is 0, 
      //the msgrcv() will behave,as a blocking system call API -
      //however, if it is set to another value, the system call 
      //API will behave, as a non-blocking service call !!
      //if return value is -ve or 0 , there is error 
      //if return value is +ve, a message was successfuly retrieved
      //from the message queue, received -  
      //the actual size of the received message, in the pay-load is returned in 
      //return value - a +ve no !!! we pass the max possible 
      //message size/pay-load buffer size ,that we may receive using 3rd param - actual 
       //message size is passed back to us using ret - 
      //we must use ret's value to actually interpret the actual 
      //payload - if we do not interpret correctly, we may 
      //use garbage data !!!

       //if successful, a stored message is copied, from 
       //a system buffer, in the message queue 
       //-->if the 4th parameter is set to 0, any message will 
       //   be received from the message queue objecti - meaning, oldest message
       //   will be copied and received , and message-type of messages will be 
       //   ignored 
       ret1 = msgrcv(id1,&msgbuf1,sizeof(msgbuf1.data),0,0);  //<--- common usage 
       //-->if the 4th parameter is set to 1(or another, +ve value), only 
       //   messages with type 1(or, another, value)  will 
       //   be received from the message-queue objecti - meaning, oldest 
       //   message of type equals to 1 will be copied/received - other messages
       //   with  a message-type, not equal to 1,  will not be received 
       //ret1 = msgrcv(id1,&msgbuf1,sizeof(msgbuf1.data),1,0);//<---- special usage 
       
       if(ret1<0 && errno == EINTR) continue;
       if(ret1<0) { 
          perror("error in receiving message"); 
          exit(3);
       }

       //if msgrcv() is successful, we are creating a new child-process
       //and assigning the job of a client-request to 
       //this child-process - parent process will continue and 
       //do it msgrcv() and repeat the following code-block 
       //
       //fork and let the child handle the client
       ret1 = fork();
       if(ret1<0) { perror("error in fork"); exit(4); }
       if(ret1>0) {

                //waitpid(-1,&status,0);   //wrong place -- 2
                continue;
       }
       if(ret1==0) //job of each child - a different job per child 
       {
             printf("S..the operation value is %d\n",\
                     msgbuf1.data.op);
       	     printf("S..the filename is %s\n",\
                     msgbuf1.data.file_data);
             printf("S..the client fifo name is %s\n",\
                     msgbuf1.data.cli_np);

             fd = open(msgbuf1.data.file_data,O_RDONLY);
             if(fd<0) { perror("error in opening data file");
                         exit(4);
             }
             npfd = open(msgbuf1.data.cli_np,O_WRONLY);
             if(npfd<0) { 
                  perror("error in opening named\
                          pipe");
                  exit(5);
             }

             printf("S...just after file is open and \
                     fifo is open\n");

             if(msgbuf1.data.op){
       
             do{

               ret2 = read(fd,rdbuf,BBUF_SZ);//reading from a reg. file
               if(ret2<0) { 
               perror("error in reading from datafile");       
               exit(6); //can be somethingelse
               }
               if(ret2 == 0) break;
               //if you write to the pipe and no read descriptors to
               //the pipe are open, the system will generate SIGPIPE
               //signal to the process that is writing 
               ret3 = write(npfd,rdbuf,ret2); //writing to a FIFO
             } while(1);

              printf("S...just after writing the file data\n");
              close(npfd);
             }//if data.op==1
             exit(0); //if the child's code/job completes    
                      //successfully, the child process will 
                      //be terminated, using exit(0)
                      //in this context, SIGCHLD will be 
                      //generated to the parent process 
                      //the parent process can install 
                      //a signal handler to process
                      //SIGCHLD notifications to the 
                      //parent process    

          } //child ends here
    }// the server while

exit(0);

}
        
           


      


    
    

