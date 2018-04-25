#ifndef _SERIAL_H_  
#define _SERIAL_H_  
  
#include <stdio.h>  
#include <string.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <termios.h>  
#include <stdlib.h>  
#include <errno.h>  
  
class MySerial  
{  
public:  
    MySerial();  
    ~MySerial();  
public:  
    static int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);  
    static int open_port(int fd,int comport);  
    static int nwrite(int serialfd,const char *data,int datalength);  
    static void nread(int fd,char *data,int datalength);  
};  
#endif  
