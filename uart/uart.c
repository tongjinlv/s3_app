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

void *ble_send(void *arg)
{
	
    while(1)
    {
	   char str[250];
	   memset(str,0,sizeof(str));
	   scanf("%s",str);
	   strcat(str,"\r\n");
       write(ble_fd,str,strlen(str));
    }
}

void *ble_recv(void *arg)
{
    int c=10;
    char cmd[100];
    memset(cmd,0,sizeof(cmd));
    while(c--)
    {
	   char str[250];
       memset(str,0,sizeof(str));
       if(read(ble_fd,str,sizeof(str)) > 0)
       {
          tcflush(ble_fd, TCIFLUSH);
          printf("R=%s\r\n",str);
          if(str[0]=='M')
          {
          cmd[0]=0xa1;
          for(int i=1;i<13;i++)cmd[i]=str[7+i];
          cmd[13]=0xff;
          write(ble_fd,cmd,14);
          printf("\r\n");
          usleep(100000);
          }
          
       }
       usleep(100000);
    }
    cmd[0]=0xa0;
    cmd[1]=0x51;
    cmd[2]=0x01;
    cmd[3]=0x51;
    cmd[4]=0x02;
    cmd[5]=0xff;
    while(1)
    {
        
    write(ble_fd,cmd,6);
    sleep(3);
    }
}
int main(int argc, char* argv[])
{
    int i;
    int ret;
    int iSetOpt;
    end_time = time(NULL)+60;
    pthread_t send_id, recv_id;

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
    pthread_create(&send_id,NULL,ble_send,NULL);
    pthread_create(&recv_id, NULL,ble_recv,NULL);
    pthread_join(send_id,NULL);
	pthread_join(recv_id,NULL);
    exit(0);
}

