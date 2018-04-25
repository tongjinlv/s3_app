#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ipc.h>

#include<errno.h>
#include<fcntl.h>
#include<semaphore.h>
#include<unistd.h>
#include<termios.h>
#include<pthread.h>

#include"uart_test.h"

#define DEV_NAME "/dev/ttyS1"

int ble_fd;
time_t end_time;
char buff[100];

int open_port(int fd)
{
    fd = open(DEV_NAME, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if (-1 == fd)
    {
       printf("Can't Open device\n");
       return(-1);
    }
    else
    {
       printf("Open device ttyS1\n");
    }

    if (fcntl(fd, F_SETFL, 0)<0)
    {
        printf("fcntl failed!\n");
    }
    else
    {
        printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
    }

    if (isatty(STDIN_FILENO) == 0)
    {
        printf("standard input is not a terminal device\n");
    }
    else
    {
        printf("Is a tty success!\n");
    }

    printf("fd open=%d\n", fd);

    return fd;
}

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;

    if (tcgetattr(fd, &oldtio) != 0)
    {
        printf("Setup Serial fail");
        return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch (nBits)
    {
      case 7:
        newtio.c_cflag |= CS7;
        break;
      case 8:
        newtio.c_cflag |= CS8;
       break;
    }

    switch (nEvent)
    {
      case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
      case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
      case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }
   switch (nSpeed)
    {
      case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
      case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
      case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
      case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
      default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if (nStop == 1)
    {
        newtio.c_cflag &= ~CSTOPB;
    }
    else if (nStop == 2)
    {
        newtio.c_cflag |= CSTOPB;
    }

    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;

    tcflush(fd, TCIFLUSH);

    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        printf("com attr set error\n");
        return -1;
    }
   printf("set done!\n");

    return 0;
}

void ble_send(void *arg)
{
    int nwrite;

    while(time(NULL) < end_time)
    {
       if((nwrite=write(ble_fd,buff,strlen(buff))) == -1)
       {
          printf("The data send fail\n");
       }
       else
       {
          printf("send: %s\n",buff);
       }

       sleep(1);
    }
}

void ble_recv(void *arg)
{
    int nread;

    while(time(NULL) < end_time)
    {
       memset(buff,0,sizeof(buff));

       if((nread = read(ble_fd,buff,100)) > 0)
       {
          printf("receive: %s\n",buff);
       }
       else
       {
          printf("No data was received\n");
       }

       sleep(1);
    }
}

int main(int argc, char* argv[])
{
    int i;
    int ret;
    int iSetOpt;
    end_time = time(NULL)+60;
    pthread_t send_id, recv_id;
    char *func[] = {"send","recv"};

    printf("argc=%d\n\r",argc);
    for(i=0;i<argc;i++)
        printf("%s\n\r",argv[i]);

    if ((ble_fd = open_port(ble_fd))<0)
    {
        printf("open_port error\n");
        exit(1);
    }

    if ((iSetOpt = set_opt(ble_fd,115200,8,'N',1))<0)
    {
        printf("set_opt error\n");
        exit(1);
    }

    printf("Serial fd=%d\n", ble_fd);
    memset(buff,0,100);

    if(!strcmp(argv[1], func[0]))
    {
      strcpy(buff,argv[2]);
      strcat(buff,"\n");
      ret = pthread_create(&send_id,NULL,(void *)ble_send,NULL);
      if(ret != 0)
        printf("Create ble_send pthread error!\n");
      pthread_join(send_id,NULL);
    }
    else if(!strcmp(argv[1], func[1]))
    {
      ret = pthread_create(&recv_id, NULL,(void *)ble_recv,NULL);
      if(ret != 0)
        printf("Create ble_recv pthread error!\n");
      pthread_join(recv_id,NULL);
    }
    else
      printf("Command error!\n");

    exit(0);
}

