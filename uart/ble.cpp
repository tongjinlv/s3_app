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
#include <signal.h>  
#include <stdio.h>  
#include <pthread.h>  
#include <unistd.h>  
#include "ble.h"
#define DEV_NAME "/dev/ttyS1"
using namespace std;
//char uart_buff[500],uart_length=0;
//pthread_mutex_t mut;

int Ble::openPort()
{
    this->fd = open(this->name,O_RDWR|O_NOCTTY);//O_NONBLOCK
    if (-1 == this->fd)
    {
       printf("Can't Open device\n");
       return 0;
    }
    if (fcntl(this->fd, F_SETFL, 0)<0)
    {
        printf("fcntl failed!\n");
		return 0;
    }
    if (isatty(STDIN_FILENO) == 0)
    {
        printf("standard input is not a terminal device\n");
		return 0;
    }
    printf("fd open=%d\n", this->fd);
	return 1;
}
int Ble::setPort(int nSpeed, int nBits, char nEvent, int nStop)
{
     termios newtio, oldtio;
    if (tcgetattr(this->fd, &oldtio) != 0)
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

    tcflush(this->fd, TCIFLUSH);
    if ((tcsetattr(this->fd, TCSANOW, &newtio)) != 0)
    {
        printf("com attr set error\n");
        return -1;
    }
    printf("set done!\n");
    return 0;
}
Ble::Ble(const char *name)
{
    this->name=name;
}
void *Ble::sendThread(void *)
{
    while(1)
    {
	   char str[250];
	   memset(str,0,sizeof(str));
	   scanf("%s",str);
	   strcat(str,"\r\n");
       write(this->fd,str,strlen(str));
    }
}
void *Ble::recviceThread(void* __this)
{
	Ble * _this =(Ble *)__this;
    while(1)
    {
       if(_this->length==0)
	   {
		   _this->length=read(_this->fd,_this->revicebuf,sizeof(_this->revicebuf));
	   }
	   usleep(1000);
    }
}
int Ble::bleWrite(const char *buf)
{
	char str[500];
	uart_length=0;
	memset(str,0,sizeof(str));
	strcat(str,buf);
	strcat(str,"\r\n");
	memset(this->buf,0,sizeof(this->buf));
	write(this->fd,str,strlen(str));
}
int Ble::bleAck(const char *buf,const char *ack,int c)
{
	int i=0;
	while(c--)
	{
		bleWrite(buf);
		printf("S:%s\r\n",buf);
		sleep(1);
		if(uart_length)printf("R:%s\r\n",this->buf);
		else printf("FailCount=%d",i++);
		if(strstr(this->buf,ack)>0){printf("OK:%s\r\n",this->buf);return 1;}
	}
	return 0;
}
int Ble::bleReadack(const char *ack,int c)
{
	int i=0;
	while(c--)
	{
		memset(this->buf,0,sizeof(this->buf));
		uart_length=0;
		sleep(1);
		if(uart_length)printf("R:%s\r\n",this->buf);
		else printf("FailCount=%d",i++);
		if(strstr(this->buf,ack)>0){printf("OK:%s\r\n",this->buf);return 1;}
	}
	return 0;
}
int Ble::bleInit()
{
	int iSetOpt;
	if ((this->fd = openPort())<0)
    {
        printf("open_port error\n");
        return 0;
    }

    if ((iSetOpt = setPort(115200,8,'N',1))<0)
    {
        printf("set_opt error\n");
        return 0;
    }
			return 0;
    printf("Serial fd=%d\n", this->fd);
	return 1;
}
int Ble::serach(char *mac)
{
	int c=200;
	char riss=0;
	tcflush(this->fd, TCIFLUSH);
	bleAck("AT+START_SCAN=1","Matalab",1);
	while(c--)
	{
		if(bleReadack("Matalab",1))
		{
			char delims[] = "\n";  
			char *result = NULL;  
			result = strtok( this->buf, delims );  
			while( result != NULL )  
			{  
				printf( "[%s]",result); 
				char *b=strstr(result,"Mata");
				char *i=strstr(this->buf,"BOARD");
				if(i>0)if(b>0)
				{
					printf("b=%d,i=%d",b,i);
					strncpy(mac,&i[8],12);
					mac[13]=0;
					char *str;
					i[15]=0;
					riss=strtol(&i[13], &str, 14);
					printf("name(Matalab)mac(%s)riss(%d)\r\n",mac,riss);
					return riss;
				}
				result = strtok( NULL, delims );  
			}  
		}
		
	}
	return riss;
}
int Ble::disSerach()
{
	bleAck("AT+START_SCAN=0","OK",2);
}
int Ble::disConnect()
{
	tcflush(this->fd, TCIFLUSH);
	if(bleAck("AT+DISCON=0","OK",2)>0)return 1;
	return 0;
}
int Ble::Connect(char *mac)
{
	char buf[200];
	tcflush(this->fd, TCIFLUSH);
	memset(buf,0,sizeof(buf));
	strcat(buf, "AT+CON_128=");
	strcat(buf, mac);
	strcat(buf, ",6E400001B5A3F393E0A9E50E24DCCA9E\r\n");
    printf(buf);
	if(bleAck(buf,"CON",1)>0);
	return 0;
}
int Ble::begin()
{
	tcflush(this->fd, TCIFLUSH);
	if(bleAck("AT+W_DCH=1","OK",1)>0)return 1;
	bleReadack("OK",1);
	return 0;
}
int Ble::writeCmd(char *buf,int Length)
{
	buf[0]=0x01;buf[1]=0x02;
	write(this->fd, buf, 2);
}

void Ble::lanch()
{
    pthread_t pth;
    pthread_create(&pth,NULL,recviceThread,(void*)this);
}
int main(int argc, char* argv[])
{
	char mac[20];
	char buf[2]={0x00,0x01};
	pthread_t send_id, recv_id;
	Ble ble(DEV_NAME);
	ble.bleInit();
	ble.disSerach();
	ble.lanch();
	ble.disConnect();
	ble.serach(mac);
	ble.Connect(mac);
	ble.begin();
	while(1)
	{
		ble.writeCmd(buf,2);
		sleep(2);
	}
    return 0;
}
/*
static int open_port(int fd)
{
    fd = open(DEV_NAME,O_RDWR|O_NOCTTY);//O_NONBLOCK
    if (-1 == fd)
    {
       printf("Can't Open device\n");
       return(-1);
    }
    if (fcntl(fd, F_SETFL, 0)<0)
    {
        printf("fcntl failed!\n");
    }
    if (isatty(STDIN_FILENO) == 0)
    {
        printf("standard input is not a terminal device\n");
    }
    printf("fd open=%d\n", fd);
    return fd;
}

int port_set(int fd, int nSpeed, int nBits, char nEvent, int nStop)
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
	
    while(1)
    {
	   char str[250];
	   memset(str,0,sizeof(str));
	   scanf("%s",str);
	   strcat(str,"\r\n");
       write(ble_fd,str,strlen(str));
    }
}

void *ble_recv(void *)
{
	char length=0;
    while(1)
    {
       
       if(uart_length==0)
	   {
		   uart_length=read(ble_fd,uart_buff,sizeof(uart_buff));
	   }
	   usleep(1000);
    }
}
void ble_write(const char *buf)
{
	char str[500];
	uart_length=0;
	memset(str,0,sizeof(str));
	strcat(str,buf);
	strcat(str,"\r\n");
	memset(uart_buff,0,sizeof(uart_buff));
	write(ble_fd,str,strlen(str));
}
char ble_ack(const char *buf,const char *ack,int c)
{
	int i=0;
	while(c--)
	{
		ble_write(buf);
		printf("S:%s\r\n",buf);
		sleep(1);
		if(uart_length)printf("R:%s\r\n",uart_buff);
		else printf("FailCount=%d",i++);
		if(strstr(uart_buff,ack)>0){printf("OK:%s\r\n",uart_buff);return 1;}
	}
	return 0;
}
char ble_manyack(const char *ack,int c)
{
	int i=0;
	while(c--)
	{
		memset(uart_buff,0,sizeof(uart_buff));
		uart_length=0;
		sleep(1);
		if(uart_length)printf("R:%s\r\n",uart_buff);
		else printf("FailCount=%d",i++);
		if(strstr(uart_buff,ack)>0){printf("OK:%s\r\n",uart_buff);return 1;}
	}
	return 0;
}
char port_init()
{
	int iSetOpt;
	if ((ble_fd = open_port(ble_fd))<0)
    {
        printf("open_port error\n");
        return 0;
    }

    if ((iSetOpt = port_set(ble_fd,115200,8,'N',1))<0)
    {
        printf("set_opt error\n");
        return 0;
    }
    printf("Serial fd=%d\n", ble_fd);
	return 1;
}
char ble_init()
{
	if(ble_ack("AT+VERION=?","AT",5)==0)return 0;
	return 1;
}


char ble_serach(char *mac)
{
	int c=200;
	char riss=0;
	tcflush(ble_fd, TCIFLUSH);
	ble_ack("AT+START_SCAN=1","Matalab",1);
	while(c--)
	{
		if(ble_manyack("Matalab",1))
		{
			char delims[] = "\n";  
			char *result = NULL;  
			result = strtok( uart_buff, delims );  
			while( result != NULL )  
			{  
				printf( "[%s]",result); 
				char *b=strstr(result,"Mata");
				char *i=strstr(uart_buff,"BOARD");
				if(i>0)if(b>0)
				{
					printf("b=%d,i=%d",b,i);
					strncpy(mac,&i[8],12);
					mac[13]=0;
					char *str;
					i[15]=0;
					riss=strtol(&i[13], &str, 14);
					printf("name(Matalab)mac(%s)riss(%d)\r\n",mac,riss);
					return riss;
				}
				result = strtok( NULL, delims );  
			}  
		}
		
	}
	return riss;
}
char ble_disconnect()
{
	tcflush(ble_fd, TCIFLUSH);
	if(ble_ack("AT+DISCON=0","OK",2)>0)return 1;
	return 0;
}
char ble_connect(char *mac)
{
	char buf[200];
	tcflush(ble_fd, TCIFLUSH);
	memset(buf,0,sizeof(buf));
	strcat(buf, "AT+CON_128=");
	strcat(buf, mac);
	strcat(buf, ",6E400001B5A3F393E0A9E50E24DCCA9E\r\n");
    printf(buf);
	if(ble_ack(buf,"CON",1)>0);
	return 0;
}
char ble_begin()
{
	tcflush(ble_fd, TCIFLUSH);
	if(ble_ack("AT+W_DCH=1","OK",1)>0)return 1;
	ble_manyack("OK",1);
	return 0;
}
char ble_cmd()
{
	char buf[2];
	//tcflush(ble_fd, TCIFLUSH);
	buf[0]=0x01;buf[1]=0x02;
	write(ble_fd, buf, 2);
}
int main1(int argc, char* argv[])
{
    pthread_t send_id, recv_id;
	port_init();
    //pthread_create(&send_id,NULL,(void *)ble_send,NULL);
    //pthread_create(&recv_id, NULL,(void *)ble_recv,NULL);
	pthread_create(&recv_id,NULL,ble_recv,NULL);
	ble_init();
	ble_ack("AT+START_SCAN=0","OK",2);
	char mac[15];
	
	ble_disconnect();//AT+DISCON=0
	ble_serach(mac);//AT+START_SCAN=01
	ble_connect(mac);//AT+CON_128=D68A6DE72A66,6E400001B5A3F393E0A9E50E24DCCA9E
	ble_begin();//AT+W_DCH=1
	ble_cmd();
	int value;
	while(1)
	{
	   sleep(4);
	   ble_cmd();
	}
    pthread_join(send_id,NULL);
	pthread_join(recv_id,NULL);
    exit(0);
}*/

