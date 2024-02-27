
//
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<sched.h>
#include<sys/time.h>
#include<sys/resource.h>

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT            ( 0xfffffff )

unsigned long x = 0, y = 0, z = 0;
int pfd0, pfd1, pfd2, pfd3;
struct thread_arg{
             unsigned int pfd;
             unsigned int period; 
} thread_arg[4];  //each element is used to fill certain parameters and 
                  //address of element is passed to p4 of pthread_create() 
                  //- this will be passed to the re-rentrant thread-method, 
                  //by the system, using hw-context 



//char g_led1_rd_buf[2]; //good enough to hold the '1' or '0' and '\0'
void ledToggle(int fd)
{
        //char rd_buf[2], wr_buf[2]; //good enough to hold the '1' or '0' and '\0'
        int ret;
	char *rd_buf;
/*      pdfd0  = open("/sys/bus/platform/devices/custom_leds/leds/led1", O_RDWR); 
        //pdfd0  = open("/dev/pseudo0", O_RDWR|O_NONBLOCK); //device file accessed in non-blocking mode
        if(pdfd0<0) { 
                perror("error in opening first device"); 
                exit(1);
        }*/

	rd_buf = malloc(2);

        ret = lseek(fd, 0, SEEK_SET);
        //ret = lseek(fd, 0, SEEK_CUR);
        //printf("1.lseek ret = %d\n", ret);
        ret = read(fd, rd_buf, 1);
        //ret = read(fd, g_led1_rd_buf, 1);
        if (ret != 1) {
                printf("error occurred\n");
                printf("ret = %d\n", ret);
        }
        else {
        //      printf("ret = %d\n", ret);
        //      printf("rd_buf = %s\n", rd_buf);
        //      printf("rd_buf = %s\n", g_led1_rd_buf);
        //      ret = lseek(fd, 0, SEEK_SET);
        //      ret = lseek(fd, 0, SEEK_CUR);
        //      printf("2.lseek ret = %d\n", ret);

        //      if(strcmp(g_led1_rd_buf, "1") == 0) {  //current state is ON
        	if(strcmp(rd_buf, "1") == 0) {  //current state is ON
        //              *wr_buf = '0';
        //              *(wr_buf+1) = '\0';
        //              write(fd, wr_buf, 1);
                        write(fd, "0", 1);
                }
                else {
        //              *wr_buf = '1';
        //              *(wr_buf+1) = '\0';
        //              write(fd, wr_buf, 1);
                        write(fd, "1", 1);
                }
        }
	
	free(rd_buf);
//      close(pdfd0);

}


//this is a re-entrant method, which is used, in several threads
//
//however, for each thread instance, the arg passed will be different, 
//so this thread method will process a  different jobj, using a different 
//arg's contents - the arg passed, using p4 of pthread_create() - refer
//to main() of this applicationi 
//
//
void *blinkLedThreadn_fn(void *arg)
{
   	int ret;
//	int fd = (int)arg;
//	int fd = pdfd0;
//	volatile unsigned long ul;
//	struct timespec ts = {.tv_sec = 3, .tv_nsec = 0};
	struct timespec ts;


/*	int policy, s;
        struct sched_param param;

        s = pthread_getschedparam(pthread_self(), &policy, &param);
	printf("thread1    policy=%s, priority=%d\n",
                   (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (policy == SCHED_RR)    ? "SCHED_RR" :
                   (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                   "???",
                   param.sched_priority);*/

       	ret = clock_gettime(CLOCK_REALTIME, &ts);
        if(ret) {
        	perror("clock_gettime");
                pthread_exit(NULL);
        }

       //based on the period passed, using a different arg, 
       //we are using a different period, for toggling an LED  
        ts.tv_sec += ((struct thread_arg *)arg)->period;


   	while(1){

       		/* Delay for 1 seconds. */
        	//usleep(1000000);
		//printf("led1 toggle ");
        	//ledToggle(fd);

		ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);
                if(ret)
                        perror("clock_nanosleep");

        //	ts.tv_sec += 3;
       //based on the period passed, using a different arg, 
       //we are using a different period, for toggling an LED  
                ts.tv_sec += ((struct thread_arg *)arg)->period;


               //lecture - a dummy loop - just for demo ??
                /* Delay for a period. */
                //for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
                {
                        /* This loop is just a very crude delay implementation.  There is
                        nothing to do in here.  Later exercises will replace this crude
                        loop with a proper delay/sleep function. */
                }

                //ledToggle(param1) is another re-entrant method 
                //based on the param1, it will operate on a different LED
                //
                //based on different contents of arg, we are passing a different 
                //LED identification value
                //
              	ledToggle( ((struct thread_arg *)arg)->pfd );
                //these variables can be used to check the execution of the task 
                x++;
	//	printf("x value is %ld\n", x);


	//	sched_yield();
   	}   

	pthread_exit(NULL);
} 


//struct timespec ts;
void *blinkLedThread1_fn(void *arg)
{
   	int ret;
	int fd = (int)arg;
//	int fd = pdfd0;
//	volatile unsigned long ul;
//	struct timespec ts = {.tv_sec = 3, .tv_nsec = 0};
	struct timespec ts;


/*	int policy, s;
        struct sched_param param;

        s = pthread_getschedparam(pthread_self(), &policy, &param);
	printf("thread1    policy=%s, priority=%d\n",
                   (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (policy == SCHED_RR)    ? "SCHED_RR" :
                   (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                   "???",
                   param.sched_priority);*/

       	ret = clock_gettime(CLOCK_REALTIME, &ts);
        if(ret) {
        	perror("clock_gettime");
                pthread_exit(NULL);
        }

        ts.tv_sec += 3;


   	while(1){

       		/* Delay for 1 seconds. */
        	//usleep(1000000);
		//printf("led1 toggle ");
        	//ledToggle(fd);

		ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);
                if(ret)
                        perror("clock_nanosleep");

        	ts.tv_sec += 3;


               //lecture - a dummy loop - just for demo ??
                /* Delay for a period. */
                //for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
                {
                        /* This loop is just a very crude delay implementation.  There is
                        nothing to do in here.  Later exercises will replace this crude
                        loop with a proper delay/sleep function. */
                }

        	ledToggle(fd);
                //these variables can be used to check the execution of the task 
                x++;
	//	printf("x value is %ld\n", x);


	//	sched_yield();
   	}   

	pthread_exit(NULL);
} 

void *blinkLedThread2_fn(void *arg)
{
   	int ret;
	int fd = (int)arg;
//	int fd = pdfd1;
//	volatile unsigned long ul;
//	struct timespec ts = {.tv_sec = 3, .tv_nsec = 0};
	struct timespec ts;


/*	int policy, s;
        struct sched_param param;

        s = pthread_getschedparam(pthread_self(), &policy, &param);
	printf("thread1    policy=%s, priority=%d\n",
                   (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (policy == SCHED_RR)    ? "SCHED_RR" :
                   (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                   "???",
                   param.sched_priority);*/

       	ret = clock_gettime(CLOCK_REALTIME, &ts);
        if(ret) {
        	perror("clock_gettime");
                pthread_exit(NULL);
        }

        ts.tv_sec += 3;


   	while(1){

       		/* Delay for 1 seconds. */
        	//usleep(1000000);
		//printf("led2 toggle ");
        	//ledToggle(fd);

		ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);
                if(ret)
                        perror("clock_nanosleep");

        	ts.tv_sec += 3;


               //lecture - a dummy loop - just for demo ??
                /* Delay for a period. */
                //for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
                {
                        /* This loop is just a very crude delay implementation.  There is
                        nothing to do in here.  Later exercises will replace this crude
                        loop with a proper delay/sleep function. */
                }

        	ledToggle(fd);
                //these variables can be used to check the execution of the task 
                y++;
	//	printf("y value is %ld\n", y);


	//	sched_yield();
   	}   

	pthread_exit(NULL);
} 

void *blinkLedThread3_fn(void *arg)
{
   	int ret;
	int fd = (int)arg;
//	int fd = pdfd2;
//	volatile unsigned long ul;
	struct timespec ts;

	/*int policy, s;
        struct sched_param param;

        s = pthread_getschedparam(pthread_self(), &policy, &param);
	printf("thread3    policy=%s, priority=%d\n",
                   (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (policy == SCHED_RR)    ? "SCHED_RR" :
                   (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                   "???",
                   param.sched_priority);*/

       	ret = clock_gettime(CLOCK_REALTIME, &ts);
        if(ret) {
        	perror("clock_gettime");
                pthread_exit(NULL);
        }

        ts.tv_sec += 3;

   	while(1){
       		/* Delay for 1 seconds. */
        //	usleep(3000000);
	//	printf("led3 toggle\n");
        	//ledToggle(fd);
		ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);
                if(ret)
                        perror("clock_nanosleep");

        	ts.tv_sec += 3;

               //lecture - a dummy loop - just for demo ??
                /* Delay for a period. */
                //for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
                {
                        /* This loop is just a very crude delay implementation.  There is
                        nothing to do in here.  Later exercises will replace this crude
                        loop with a proper delay/sleep function. */
                }
        	ledToggle(fd);
                //these variables can be used to check the execution of the task 
                z++;
		//printf("z value is %ld\n", z);
		//ret = sched_yield();
		//if(ret != 0)
		//	printf("shced_yield failed\n");
   	}   

	pthread_exit(NULL);
} 

//
//user-space thread library ids/objects - these are not same, as 
//system space thread ids/objects - do not mix up - however, 
//thread library APIs take care of hiding their abstraction !!!
//internally, these IDs/objects will be  mapped to system-space - 
//for all manipulations, thread-library will take care  !!!

//user-space thread IDs are abstract and opaque !!!

pthread_t blinkLedThread1_id, blinkLedThread2_id, blinkLedThread3_id, blinkLedThread4_id;


int main()
{

   	int ret;
   	struct sched_param param1;
       	//int policy, s;

   	//these are abstract thread attr objects - 
   	//these can be local variables and
   	//they must be initialized before using them !!!!
	pthread_attr_t pthread_attr1, pthread_attr2, pthread_attr3, pthread_attr4;


	param1.sched_priority = 2;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param1);


/*        s = pthread_getschedparam(pthread_self(), &policy, &param1);
        printf("main thread    policy=%s, priority=%d\n",
                   (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (policy == SCHED_RR)    ? "SCHED_RR" :
                   (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                   "???",
                   param1.sched_priority);
*/

   	//pdfd0  = open("/sys/bus/platform/devices/custom_leds/leds/led1", O_RDWR); 
        //we are accessing a sysfs regular file and setting-up an active file handle -
        //this will connect us to an attribute/device-specific attribute, which can 
        //enable us to access a gpio pin/its connected-led (it can be a sensor, as well)
        //
        // 
   	pfd0  = open("/sys/bus/platform/devices/ext_leds/led1/led1_value", O_RDWR); 
   	//pdfd0  = open("/dev/pseudo0", O_RDWR|O_NONBLOCK); //device file accessed in non-blocking mode
   	if(pfd0<0) { 
		perror("error in opening led1"); 
		exit(1);
	}

   	//pdfd1  = open("/sys/bus/platform/devices/custom_leds/leds/led2", O_RDWR); 
        //we are accessing a sysfs regular file and setting-up an active file handle -
        //this will connect us to an attribute/device-specific attribute, which can 
        //enable us to access a gpio pin/its connected-led (it can be a sensor, as well)
        //
   	pfd1  = open("/sys/bus/platform/devices/ext_leds/led2/led2_value", O_RDWR); 
   	//pdfd0  = open("/dev/pseudo0", O_RDWR|O_NONBLOCK); //device file accessed in non-blocking mode
   	if(pfd1<0) { 
		perror("error in opening led2"); 
		exit(1);
	}

   	//pdfd2  = open("/sys/bus/platform/devices/custom_leds/leds/led3", O_RDWR); 
        //we are accessing a sysfs regular file and setting-up an active file handle -
        //this will connect us to an attribute/device-specific attribute, which can 
        //enable us to access a gpio pin/its connected-led (it can be a sensor, as well)
        //
   	pfd2  = open("/sys/bus/platform/devices/ext_leds/led3/led3_value", O_RDWR); 
   	//pdfd0  = open("/dev/pseudo0", O_RDWR|O_NONBLOCK); //device file accessed in non-blocking mode
   	if(pfd2<0) { 
		perror("error in opening led3"); 
		exit(1);
	}

   	//pdfd2  = open("/sys/bus/platform/devices/custom_leds/leds/led4", O_RDWR); 
        //we are accessing a sysfs regular file and setting-up an active file handle -
        //this will connect us to an attribute/device-specific attribute, which can 
        //enable us to access a gpio pin/its connected-led (it can be a sensor, as well)
        //
   	pfd3  = open("/sys/bus/platform/devices/ext_leds/led4/led4_value", O_RDWR); 
   	//pdfd0  = open("/dev/pseudo0", O_RDWR|O_NONBLOCK); //device file accessed in non-blocking mode
   	if(pfd3<0) { 
		perror("error in opening led3"); 
		exit(1);
	}

//      char wr_buf[] = "0";
//      printf("\nmain:wr_buf = %s\n", wr_buf);

       //the following write() system call APIs initialize the state of 
       //gpio pins/their connected-leds
       //
       //
        write(pfd0, "0", 1); //turn off led1
        write(pfd1, "0", 1); //turn off led2
        write(pfd2, "0", 1); //turn off led3
        write(pfd2, "0", 1); //turn off led3
      	
        //initialization of the thread attribute obj. is a must
      	//before using the attribute object in the pthread_create()

      	//after initializing the thread attr object, you can set the
      	//attributes as per your requirements 

      	//still, initializing thread attr object ensures that 
      	//all attributes are initially set to default system settings
      	//for more details on default settings, refer to manual pages !!!
      	ret = pthread_attr_init(&pthread_attr1);
      	if(ret>0) {
        	printf("error in the pthread_attr1 initialization\n");
      	}
	//apis below cannot be used to change non realtime priorities !!!

   	//setting the policy to realtime, priority based - in this example, 
        //SCHED_FIFO - you may change this to SCHED_RR, if needed
        //
   	pthread_attr_setschedpolicy(&pthread_attr1, SCHED_FIFO);
   	//pthread_attr_setschedpolicy(&pthread_attr1, SCHED_RR);

   	//realtime priority of 1 ( 1-99 is the range) 
        //
        //we are setting the real-time priority to 1, in this example - 
        //we may change this value, if needed
        //
   	param1.sched_priority = 1;
   	pthread_attr_setschedparam(&pthread_attr1, &param1);

   	//you must set the following attribute, if you wish to use
   	//explicit scheduling parameters - if you do not, system 
   	//will ignore the explicit scheduling parameters passed
   	//via attrib object and inherit the scheduling parameters
   	//of the creating sibling thread !!! 
   	pthread_attr_setinheritsched(&pthread_attr1, PTHREAD_EXPLICIT_SCHED);

        //we need to pass two parameters, using p4 of pthread_create() 
        //
        thread_arg[0].pfd    = pfd0; //this will be a handle to a gpio pin's sysfs active file
        thread_arg[0].period = 3;    //we need to toggle an LED, using certain period 
                                     //, or we can set an event-based toggling  

  	//ret = pthread_create(&blinkLedThread1_id, &pthread_attr1, blinkLedThreadn_fn, (void*)pdfd0);
        //
        //we create a new thread and pass appropriate parameters, as discussed above 
        //
        //for this first thread, we are using blinkLedThreadn_fn(), which is a re-entrant method - 
        //also, we are setting-up and passing p4, for a specific job, in blinkLedThreadn_fn()  
        // 
  	ret = pthread_create(&blinkLedThread1_id, &pthread_attr1, blinkLedThreadn_fn, (void*)&thread_arg[0]);
  	//ret = pthread_create(&blinkLedThread1_id, &pthread_attr1, blinkLedThreadn_fn1, (void*)&thread_arg[0]);
   	if(ret>0) { 
		printf("error in blinkLedThread1 creation\n"); 
		exit(4); 
	}   

	pthread_attr_destroy(&pthread_attr1);

	printf("1.debug point\n");
      	ret = pthread_attr_init(&pthread_attr2);
      	if(ret>0) {
        	printf("error in the pthread_attr2 initialization\n");
      	}

   	//setting the policy to realtime, priority based 
   	pthread_attr_setschedpolicy(&pthread_attr2, SCHED_FIFO);
   	//pthread_attr_setschedpolicy(&pthread_attr2, SCHED_RR);

   	//realtime priority of 1 ( 1-99 is the range) 
   	param1.sched_priority = 1;
   	pthread_attr_setschedparam(&pthread_attr2, &param1);

  	pthread_attr_setinheritsched(&pthread_attr2, PTHREAD_EXPLICIT_SCHED);

	printf("2.debug point\n");

        //for another thread, we are setting-up p4 of pthread_create and passing certain parameters
        //
        thread_arg[1].pfd    = pfd1;
        thread_arg[1].period = 6;
        //for this second thread, we are using blinkLedThreadn_fn(), which is a re-entrant method - 
        //also, we are setting-up and passing p4, for a specific job, in blinkLedThreadn_fn()  
        // 
	ret = pthread_create(&blinkLedThread2_id, &pthread_attr2, blinkLedThreadn_fn, (void*)&thread_arg[1]);
  	//ret = pthread_create(&blinkLedThread2_id, &pthread_attr1, blinkLedThreadn_fn2, (void*)&thread_arg[0]);
   	if(ret>0) { 
		printf("error in blinkLedThread2 creation\n"); 
		exit(4); 
	}   

	pthread_attr_destroy(&pthread_attr2);

      	ret = pthread_attr_init(&pthread_attr3);
      	if(ret>0) {
        	printf("error in the pthread_attr3 initialization\n");
      	}

   	//setting the policy to realtime, priority based 
   	pthread_attr_setschedpolicy(&pthread_attr3, SCHED_FIFO);
   	//pthread_attr_setschedpolicy(&pthread_attr3, SCHED_RR);

   	//realtime priority of 1 ( 1-99 is the range) 
   	param1.sched_priority = 1;
   	pthread_attr_setschedparam(&pthread_attr3, &param1);

  	pthread_attr_setinheritsched(&pthread_attr3, PTHREAD_EXPLICIT_SCHED);


        //for another thread, we are setting-up p4 of pthread_create and passing certain parameters
        //
        thread_arg[2].pfd    = pfd2;
        thread_arg[2].period = 9;
        //for this third thread, we are using blinkLedThreadn_fn(), which is a re-entrant method - 
        //also, we are setting-up and passing p4, for a specific job, in blinkLedThreadn_fn()  
        // 
  	ret = pthread_create(&blinkLedThread3_id, &pthread_attr3, blinkLedThreadn_fn, (void*)&thread_arg[2]);
  	//ret = pthread_create(&blinkLedThread3_id, &pthread_attr1, blinkLedThreadn_fn3, (void*)&thread_arg[0]);
   	if(ret>0) { 
		printf("error in blinkLedThread3 creation\n"); 
		exit(4); 
	}   
 
     	ret = pthread_attr_init(&pthread_attr4);
      	if(ret>0) {
        	printf("error in the pthread_attr4 initialization\n");
      	}

   	//setting the policy to realtime, priority based 
   	pthread_attr_setschedpolicy(&pthread_attr4, SCHED_FIFO);
   	//pthread_attr_setschedpolicy(&pthread_attr4, SCHED_RR);

   	//realtime priority of 1 ( 1-99 is the range) 
   	param1.sched_priority = 1;
   	pthread_attr_setschedparam(&pthread_attr4, &param1);

  	pthread_attr_setinheritsched(&pthread_attr4, PTHREAD_EXPLICIT_SCHED);
        
        //for another thread, we are setting-up p4 of pthread_create and passing certain parameters
        //
        thread_arg[3].pfd    = pfd3;
        thread_arg[3].period = 12;
        //for this fourth thread, we are using blinkLedThreadn_fn(), which is a re-entrant method - 
        //also, we are setting-up and passing p4, for a specific job, in blinkLedThreadn_fn()  
        // 
  	//ret = pthread_create(&blinkLedThread4_id, &pthread_attr4, blinkLedThreadn_fn, (void*)&thread_arg[3]);
  	ret = pthread_create(&blinkLedThread4_id, &pthread_attr1, blinkLedThreadn_fn4, (void*)&thread_arg[0]);
   	if(ret>0) { 
		printf("error in blinkLedThread3 creation\n"); 
		exit(4); 
	} 


  //a thread that is created is generally created as a joinable
   //thread - meaning, a sibling thread(mostly main) must join/wait
   //for a newly created thread - this is a natural step - if you miss
   //this your application's main may terminate and eventually, all
   //your threads may prematurely terminate - there is no point 
   //in doing so !!! - however, there are cases where a detached 
   //thread may be created, if needed - in this case, a detached
   //thread cannot be joined/waited upon - it is purely upto the
   //developer to decide and implement the threads as joinable or
   //detached !!! by default, in most cases, it preferred to 
   //maintain joinable threads - for a joinable thread, we can also
   //collect any return information (ptr) returned by the corresponding
   //thread !!!!
   // 
   pthread_join(blinkLedThread1_id,NULL);
   pthread_join(blinkLedThread2_id,NULL);
   pthread_join(blinkLedThread3_id,NULL);
   pthread_join(blinkLedThread4_id,NULL);


   exit(0);

}






