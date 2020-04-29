#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <mqueue.h>
#include <time.h> 
#include <semaphore.h>
#include "../module/eink_ioctl.h"

#define QUEUE_NAME   "/mqueue"
#define SEM_MUTEX_NAME "/sem-mutex"

#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

sem_t *mutex_sem;

  
void delay(int number_of_seconds) 
{ 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds) 
        ; 
} 

int printRandoms(int lower, int upper) 
{ 
    int num = (rand() % (upper - lower + 1)) + lower; 
    return num;
} 

int main(void)
{
    mqd_t qd_tx;
    struct mq_attr attr;

    if ((mutex_sem = sem_open (SEM_MUTEX_NAME, O_CREAT, 0660, 0)) == SEM_FAILED) {
        perror ("sem_open"); exit (1);
    }

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

   if ((qd_tx = mq_open (QUEUE_NAME, O_WRONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        exit(-1);
    }


    char out_buffer[MSG_BUFFER_SIZE] = "HELLO\0";
    int i = 0;
    int temp = -180;
    int temp1 = 0;
    while (1) {
        ++i;

        // snprintf(out_buffer, 33, "Yaw: %4d Pitch: %4d Roll: %4d\0", printRandoms(0, 360), printRandoms(-180, 180), printRandoms(-180, 180));
        snprintf(out_buffer, 33, "Yaw: %4d Pitch: %4d Roll: %4d\0", temp1, temp, temp);

        temp += 20;
        temp1 += 20;

        if (sem_wait (mutex_sem) == -1) {
	        perror ("sem_take: muex_sem"); exit (1);
        }

        if (mq_send (qd_tx, out_buffer, strlen (out_buffer) + 1, 0) == -1) {
            perror ("Server: Not able to send message to client");
            continue;
        }

        printf ("Server: response %d sent %s to client\n", i, out_buffer);

        
        if(i > 18)
            break;
    }
    mq_unlink(QUEUE_NAME);
    sem_unlink (SEM_MUTEX_NAME);



    return 0;
}