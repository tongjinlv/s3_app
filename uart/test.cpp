/*
*******************************************************************************
*    \'-.__.-'/     | Company  :o--Shen Zhen Zowee Technology Co,.Ltd--o
*    / (o)(o) \     | Website  :o--http://www.zowee.com.cn--o
*    \   \/   /     | Copyright: All Rights Reserved
*    /'------'\     | Product  : SVV
*   /,   ..  , \    | File     : uart_ble_master.c
*  /// .::::. \\\   | Descript : program main entry, creat startup task
* ///\ :::::: /\\\  | Version  : V0.10
*''   ).''''.(   `` | Author   : nicholasldf
*====(((====)))==== | EditTime : 2018-01-06-10:00
*******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>

#define FALSE 1
#define TRUE  0

int  uart_ble_fd, number;
char mac_addr_buff[16];
char uart_ble_buff[1024];

int speed_arr[] = { 
	B921600, B460800, B230400, B115200, B57600, B38400, B19200, 
	B9600, B4800, B2400, B1200, B300, 
};

int name_arr[] = {
	921600, 460800, 230400, 115200, 57600, 38400,  19200,  
	9600,  4800,  2400,  1200,  300,  
};

void set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);

	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
		if  (speed == name_arr[i])	{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);

			//luodefu - enable the receiver and set local mode
			Opt.c_cflag |= (CLOCAL | CREAD);

			//Set the new options for the port...
			status = tcsetattr(fd, TCSANOW, &Opt);
			if  (status != 0)
				perror("tcsetattr fd1");
				return;
		}
		tcflush(fd,TCIOFLUSH);
  	 }

	if (i == 12){
		printf("\tSorry, please set the correct baud rate!\n\n");
	}
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if  ( tcgetattr( fd,&options)  !=  0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE ;
	switch (databits){
	case 7:
		options.c_cflag |= CS7;
	break;
	case 8:
		options.c_cflag |= CS8;
	break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return (FALSE);
	}
	
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;//Clear parity enable
		options.c_iflag &= ~INPCK;//Enable parity checking
	break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;//Disnable parity checking
	break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB; //Enable parity */
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;  //Disnable parity checking
	break;
	case 'S':	
	case 's'://as no parity
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
	break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (FALSE);
	}
 	
  	switch (stopbits) {
    case 1:
    	options.c_cflag &= ~CSTOPB;
  	break;
 	case 2:
  		options.c_cflag |= CSTOPB;
  	break;
 	default:
  		fprintf(stderr,"Unsupported stop bits\n");
  		return (FALSE);
 	}
  	//Set input parity option
  	if (parity != 'n')
    	options.c_iflag |= (INPCK | ISTRIP);//(INPCK | ISTRIP)

  	options.c_cc[VTIME] = 0; //10 seconds
    options.c_cc[VMIN]  = 0;

	//options.c_lflag &= ~(ECHO | ICANON);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  	tcflush(fd,TCIFLUSH);//Update the options and do it NOW
  	if (tcsetattr(fd,TCSANOW,&options) != 0) {
    	perror("SetupSerial 3");
  		return (FALSE);
 	}
	return (TRUE);
}



int connect_ble_master(void)
{
	int cnt = 5;
	
	printf("----------------connect_ble_master----------------\r\n");
	while(cnt){
		//clear receive buffer
		tcflush(uart_ble_fd, TCIFLUSH);
		//send cmd
		write(uart_ble_fd, "AT+VERION=?\n", 13);
		usleep(20000);
		
		//AT+OK/r/nA102
		number = read(uart_ble_fd, uart_ble_buff, 13);
		uart_ble_buff[14] = 0;
		printf("BleMasterVersion: %s\r\n", uart_ble_buff);
	}
	
	return 0;
}


int main(int argc, char *argv[])
{
	unsigned int  value;
	
	//usage
	printf("\n\n");
	printf("----------------usage----------------\n");
	printf("uart_ble_master : 115200-8N1;\n");
	printf("-------------------------------------\n");

	uart_ble_fd = open("/dev/ttyS1", O_RDWR); //| O_NOCTTY | O_NDELAY
	if (uart_ble_fd > 0) {
		printf("open /dev/ttyS1 OK\n");
	} else {
			fprintf(stderr, "Error open ttyS1\n");
			exit(1);
	}
	
	
    connect_ble_master();
	close(uart_ble_fd);
	exit(0);
}

