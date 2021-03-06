#include "serial.h"  
  
MySerial::MySerial( )  
{  
}  
  
MySerial::~MySerial()  
{  
  
}  
  
int MySerial::set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)  //串口设置  
{  
    struct termios newtio,oldtio;  
    if  ( tcgetattr( fd,&oldtio)  !=  0) {   
        perror("SetupSerial 1");  
        return -1;  
    }  
    bzero( &newtio, sizeof( newtio ) );  
    newtio.c_cflag  |=  CLOCAL | CREAD;   
    newtio.c_cflag &= ~CSIZE;   
//  newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  
//  newtio.c_oflag &= ~OPOST;  
    switch( nBits )  
    {  
    case 7:  
        newtio.c_cflag |= CS7;  
        break;  
    case 8:  
        newtio.c_cflag |= CS8;  
        break;  
    }  
  
    switch( nEvent )  
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
  
    switch( nSpeed )  
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
    if( nStop == 1 )  
        newtio.c_cflag &=  ~CSTOPB;  
    else if ( nStop == 2 )  
    newtio.c_cflag |=  CSTOPB;  
    newtio.c_cc[VTIME]  = 0;  
    newtio.c_cc[VMIN] = 0;  
    tcflush(fd,TCIFLUSH);  
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)  
    {  
        perror("com set error");  
        return -1;  
    }  
    return 0;  
}  
  
int MySerial::open_port(int fd,int comport)                 //通过参数，打开相应的串口  
{  
    if (comport==1)  
    {   fd = open( "/dev/ttyS0", O_RDWR|O_NOCTTY|O_NDELAY);  
        if (-1 == fd){  
            perror("Can't Open Serial /dev/ttyS0 Port");  
            return(-1);  
        }  
    }  
    else if(comport==2)  
    {   fd = open( "/dev/ttyS1", O_RDWR|O_NOCTTY|O_NDELAY);  
        if (-1 == fd){  
            perror("Can't Open Serial /dev/ttyS1 Port");  
            return(-1);  
        }  
    }  
    else if (comport==3)  
    {  
        fd = open( "/dev/ttyS2", O_RDWR|O_NOCTTY|O_NDELAY);  
        if (-1 == fd){  
            perror("Can't Open Serial /dev/ttyS2 Port");  
            return(-1);  
        }  
    }  
    if(fcntl(fd, F_SETFL, 0)<0)  
        perror("fcntl failed!");  
    if(isatty(STDIN_FILENO)==0)  
        perror("standard input is not a terminal device");  
    return fd;  
}  
  
int MySerial::nwrite (int serialfd, const char *data, int datalength )  //写串口信息  
{  
    int len = 0, total_len = 0;  
    for (total_len = 0 ; total_len < datalength;)   
    {  
    len = 0;  
    len = write(serialfd, &data[total_len], datalength - total_len);  
        if (len > 0)   
            total_len += len;            
   }  
    return (total_len);  
}  
  
void MySerial::nread(int fd,char *data,int datalength)   //读取串口信息  
{  
    int readlen=0;  
    if((readlen=read(fd,data,datalength))>0)  
    {  
        printf("current condition is %s\n",data);  
    }  
    return ;  
}  
int main()
{
	MySerial ms;
	char buf[100];
	int fd=ms.open_port(fd,2);
	ms.set_opt(fd,115200,8,'N',1);
	while(1)
	{
		ms.nwrite(fd,"AT+VERION=?\r\n",sizeof("AT+VERION=?\r\n"));
		sleep(1);
		ms.nread(fd,buf,100);
		printf("read%s",buf);
		sleep(1);
	}
}