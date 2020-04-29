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
#include <semaphore.h>
#include <math.h>
#include <signal.h>
#include <syslog.h> 
#include "../module/eink_ioctl.h"

#define QUEUE_NAME   "/mqueue"
#define SEM_MUTEX_NAME "/sem-mutex"

#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

#define DISP_WHITE 0
#define DISP_BLACK 1

#define DtoR(X) ((X) * M_PI/(double)180.00)

sem_t *mutex_sem;
int fd;
void handle_sig(int sig)
{
    
    if(sig == SIGINT)
        syslog(LOG_DEBUG,"Caught SIGINT Signal exiting\n");
    if(sig == SIGTERM)
        syslog(LOG_DEBUG,"Caught SIGTERM Signal exiting\n");
    mq_unlink(QUEUE_NAME);
    sem_unlink (SEM_MUTEX_NAME);
   _exit(0);
    
}
uint8_t cursorBitmap[10][10] = 
{
    {0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 0, 0, 0, 0}
};

//pos-pos-X-Y-section
void writeSection(int posX, int posY, int width, int height, bool color, uint8_t *section)
{
    int index = posX + width * posY;
    section[index] = color;
}

// void delay(int number_of_seconds)
// {
    // Converting time into milli_seconds
    // int milli_seconds = 1000 * number_of_seconds;
  // 
    // Storing start time
    // clock_t start_time = clock();
  // 
    // looping till required time is not achieved
    // while (clock() < start_time + milli_seconds)
        // ;
// }

void writePartial(void)
{
    struct pixelDataIn data;
    data.partLUT = true;
    ioctl(fd, EINKCHAR_IOCWRLUT, &data);
}

void drawPR(int pitch, int roll)
{
    struct pixelDataIn data;
    data.disableUpdate = true;

    data.x = 0;
    data.y = 0;
    data.x1 = 200;
    data.y1 = 200;
    data.partLUT = true;

    uint8_t *sec = malloc(200 * 200);

    for(int i = 0; i < 200; i ++)
    {
        for(int j = 0; j < 200; j++)
        {
            writeSection(i, j, 200, 200, DISP_WHITE, sec);
        }
    }

    data.sectionData = sec;
    // ioctl(fd, EINKCHAR_IOCWRSECTION, &data);

    pitch *= -1;
    pitch += 180;
    roll += 180;

    double _pitch = (double)pitch;
    double _roll = (double)roll;

    _pitch /= 3.6;
    _roll /= 3.6;

    // printf("Non-scaled adjusted Pitch: %d Roll %d ", pitch, roll);
    pitch = round(_pitch) / 2;
    roll = round(_roll) / 2;
    printf("scaled adjusted Pitch: %d Roll %d \n", pitch, roll);

    // ioctl(fd, EINKCHAR_IOCWRLUT, &data);

    data.x = (roll) - 5 + 25;
    data.y = (pitch) - 5 + 25;

    // data.x /= 2;
    // data.y /= 2;

    data.x1 = 10;
    data.y1 = 10;

    uint8_t *sec1 = malloc(10 * 10);

    for(int i = 0; i < 10; i ++)
    {
        for(int j = 0; j < 10; j++)
        {
            writeSection(i, j, 10, 10, DISP_WHITE, sec);
        }
    }

    for(int i = 0; i < 10; i ++)
    {
        for(int j = 0; j < 10; j++)
        {
            writeSection(i, j, 10, 10, cursorBitmap[j][i], sec);
        }
    }

    data.sectionData = sec;
    ioctl(fd, EINKCHAR_IOCWRSECTION, &data);
    ioctl(fd, EINKCHAR_IOCWRDISUPD, NULL);

    free(sec);
    free(sec1);
}

void drawYaw(int yaw)
{
    struct pixelDataIn data;
    data.disableUpdate = true;

    data.x = 0;
    data.y = 0;
    data.x1 = 200;
    data.y1 = 200;
    data.partLUT = true;

    uint8_t *sec = malloc(200 * 200);

    for(int i = 0; i < 200; i ++)
    {
        for(int j = 0; j < 200; j++)
        {
            writeSection(i, j, 200, 200, DISP_WHITE, sec);
        }
    }

    data.sectionData = sec;
    ioctl(fd, EINKCHAR_IOCWRSECTION, &data);

    bool flipLine = true;
    if(yaw > 180)
    {
        flipLine = false;
        yaw -= 180;
        printf("Line flipped: Yaw: Old: %d New: %d\n", yaw + 180, yaw);
    }

    data.lineLength = 50;
    int x = round(50 * cos(DtoR(yaw)));
    int y = round(50 * sin(DtoR(yaw)));
    data.x = 50;
    data.y = 150;

    if(flipLine)
        data.x1 = x + 50;
    else
        data.x1 = x + 50 - x - x;

    if(flipLine)
        data.y1 = y + 150 - y - y;
    else
        data.y1 = y + 150;

    printf("Line: (%d, %d) Relative: (%d, %d)\n", x, y, data.x1, data.y1);

    // ioctl(fd, EINKCHAR_IOCWRLUT, &data);
    ioctl(fd, EINKCHAR_IOCWRXYLINE, &data);
    // ioctl(fd, EINKCHAR_IOCWRDISUPD, NULL);

    free(sec);

}

void create_daemon()
 {

    pid_t process_id;
    process_id=fork();
    if (process_id < 0){
        syslog(LOG_DEBUG,"Fork failed\n");
        exit(1);
    }
    if(process_id > 0)
        exit(0);
    umask(0);
    if (setsid() < 0)
    {
        syslog(LOG_DEBUG,"Child process does not lead\n");
        exit(1);
    }
    if(chdir("/") < 0)
    {
        syslog(LOG_DEBUG,"Directory change filed \n"); // chdir() failed.
        exit(1);
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void drawOverlay(int yaw, int pitch, int roll)
{
    struct pixelDataIn data;
    data.disableUpdate = true;

    data.x = 120;
    char overlay[100];

    data.y = 25;
    sprintf(overlay, "Pitch: %4d", pitch);
    data.stringLength = strlen(overlay);
    printf("Overlay Pitch: %s\n", overlay);
    data.stringIn = overlay;
    ioctl(fd, EINKCHAR_IOCWRCHAR, &data);

    data.y = 38;
    sprintf(overlay, "Roll: %5d", roll);
    data.stringLength = strlen(overlay);
    printf("Overlay Roll: %s\n", overlay);
    data.stringIn = overlay;
    ioctl(fd, EINKCHAR_IOCWRCHAR, &data);

    data.y = 150;
    sprintf(overlay, "Yaw: %6d", yaw);
    data.stringLength = strlen(overlay);
    printf("Overlay Yaw: %s\n", overlay);
    data.stringIn = overlay;
    ioctl(fd, EINKCHAR_IOCWRCHAR, &data);

    data.y = 80;
    sprintf(overlay, "Prayag", yaw);
    data.stringLength = strlen(overlay);
    data.stringIn = overlay;
    ioctl(fd, EINKCHAR_IOCWRCHAR, &data);

    data.y = 93;
    sprintf(overlay, "Rahul", yaw);
    data.stringLength = strlen(overlay);
    data.stringIn = overlay;
    ioctl(fd, EINKCHAR_IOCWRCHAR, &data);

    data.y = 106;
    sprintf(overlay, "Sharan", yaw);
    data.stringLength = strlen(overlay);
    data.stringIn = overlay;
    ioctl(fd, EINKCHAR_IOCWRCHAR, &data);

    data.x = 0;
    data.y = 100;
    data.x1 = 110;
    data.y1 = 100;
    ioctl(fd, EINKCHAR_IOCWRXYLINE, &data);

    data.x = 110;
    data.y = 0;
    data.x1 = 110;
    data.y1 = 200;
    ioctl(fd, EINKCHAR_IOCWRXYLINE, &data);
}

int main(int argc, char *argv[])
{
	printf("SIG REG\n");
    signal(SIGTERM,handle_sig);
    signal(SIGINT,handle_sig);
    if(argc == 2){
	printf("Start daemon");
     create_daemon();
    }
    mqd_t qd_rx;
    struct mq_attr attr;

    printf("Init mutex\n");
	
    if ((mutex_sem = sem_open (SEM_MUTEX_NAME, O_CREAT, 0660, 0)) == SEM_FAILED) {
        perror ("sem_open"); exit (1);
    }

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

	printf("Init message Q\n");
    if ((qd_rx = mq_open (QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        exit(-1);
    }
    char in_buffer[MSG_BUFFER_SIZE];
    int i = 0;

	printf("Open device driver\n");
    fd = open("/dev/einkChar", O_WRONLY);

    printf("Write partial\n");
    writePartial();

    while (1) {
        ++i;

		printf("Give semaphore\n");
        if (sem_post (mutex_sem) == -1) {
	        perror ("sem_post: mutex_sem"); exit (1);
        }

        printf("Rx message Q\n");
        // get the oldest message with highest priority
        if (mq_receive (qd_rx, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Server: mq_receive");
            exit (1);
        }
        printf ("Server: message %d received: %s\n", i, in_buffer);

        int yaw = 0, pitch = 0, roll = 0;
        if(sscanf(in_buffer, "Yaw: %4d Pitch: %4d Roll: %4d\0", &yaw, &pitch, &roll) != 3)
        {
            printf("Error in sscanf\n");
        }

        printf("Yaw: %d Pitch %d Roll %d\n", yaw, pitch, roll);
        drawYaw(yaw);
        drawOverlay(yaw, pitch, roll);
        drawPR(pitch, roll);

        // delay(1000);

        
    }
    mq_unlink(QUEUE_NAME);
    sem_unlink (SEM_MUTEX_NAME);
    close(fd);



    return 0;
}
