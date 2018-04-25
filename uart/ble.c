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
using namespace std;

int Ble::openPort()
{
    this->fd = open( this->name, O_RDWR|O_NOCTTY|O_NDELAY);  
    if (-1 == this->fd)perror("Can't Open Serial /dev/ttyS1 Port");  
    if(fcntl(this->fd, F_SETFL, 0)<0)perror("fcntl failed!");  
    if(isatty(STDIN_FILENO)==0)perror("standard input is not a terminal device");  
    return this->fd;  
}
int Ble::setPort()
{
	struct termios newtio,oldtio;  
    if(tcgetattr( fd,&oldtio)!=0)perror("SetupSerial 1");  
    bzero( &newtio, sizeof( newtio ) );  
    newtio.c_cflag  |=  CLOCAL | CREAD;   
    newtio.c_cflag &= ~CSIZE;   
    newtio.c_cflag |= CS8;   
    newtio.c_cflag &= ~PARENB;  
    cfsetispeed(&newtio, B115200);  
    cfsetospeed(&newtio, B115200);  
    newtio.c_cflag &=  ~CSTOPB;   
    newtio.c_cc[VTIME]  = 0;  
    newtio.c_cc[VMIN] = 0;  
    tcflush(fd,TCIFLUSH);  
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)perror("com set error");  
    return 1;  
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
	this->length=0;
	memset(str,0,sizeof(str));
	strcat(str,buf);
	strcat(str,"\r\n");
	memset(this->revicebuf,0,sizeof(this->revicebuf));
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
		if(this->length)printf("R:%s\r\n",this->revicebuf);
		else printf("FailCount=%d",i++);
		if(strstr(this->revicebuf,ack)>0){printf("OK:%s\r\n",this->revicebuf);return 1;}
	}
	return 0;
}
int Ble::bleReadack(const char *ack,int c)
{
	int i=0;
	while(c--)
	{
		memset(this->revicebuf,0,sizeof(this->revicebuf));
		this->length=0;
		sleep(1);
		if(this->length)printf("R:%s\r\n",this->revicebuf);
		else printf("FailCount=%d",i++);
		if(strstr(this->revicebuf,ack)>0){printf("OK:%s\r\n",this->revicebuf);return 1;}
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

    if ((iSetOpt = setPort())<0)
    {
        printf("set_opt error\n");
        return 0;
    }
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
			result = strtok( this->revicebuf, delims );  
			while( result != NULL )  
			{  
				printf( "[%s]",result); 
				char *b=strstr(result,"Mata");
				char *i=strstr(this->revicebuf,"BOARD");
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
	bleAck("AT+START_SCAN=1","Matalab",2);
	sleep(1);
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
Ble::Ble(const char *name)
{
    this->name=name;
	bleInit();
	disSerach();
	lanch();
	disConnect();
}
int main(int argc, char* argv[])
{
	char mac[20];
	char buf[2]={0x00,0x01};
	Ble ble("/dev/ttyS1");
	ble.serach(mac);
	ble.Connect((char *)"D68A6DE72A66");
	ble.begin();
	while(1)
	{
		ble.writeCmd(buf,2);
		sleep(1);
		printf("ddd\r\n");
	}
    return 0;
}
