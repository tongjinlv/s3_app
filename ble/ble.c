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
		
		if(0==strncmp(uart_ble_buff, "AT+OK", 5)){
			return 1;
		}else{
			cnt--;
		}
	}
	
	return 0;
}

int find_ble_device(void)
{
	unsigned int cnt = 1000, CheckLen, scanRspLen;
	
	printf("----------------find_ble_device----------------\r\n");
	usleep(20000);
	//clear receive buffer
	tcflush(uart_ble_fd, TCIFLUSH);
	
	//send cmd
	write(uart_ble_fd, "AT+START_SCAN=1\n", 17);
	usleep(10000);
	
	//AT+OK\r\n
	number = read(uart_ble_fd, uart_ble_buff, 7);
	printf("CmdRet: %s\r\n", uart_ble_buff);
	
	//Advertising Data (AD), Scan Response Data (SRD)
	//AT+BOARD=0xD5A556E5A194-30,XXXXXX
	//AT+SCAN =0xD5A556E5A194-30,XXXXXX
	//clear
	uart_ble_buff[0]=uart_ble_buff[1]=uart_ble_buff[2]=0;
	while(cnt){
		//sync to "AT+"
		if((uart_ble_buff[0]=='A')&&(uart_ble_buff[1]=='T')&&(uart_ble_buff[2]=='+')){
			//read
			number = read(uart_ble_fd, &uart_ble_buff[3], 24);
			printf("advert: %s\r\n", uart_ble_buff);
		}else{
			number = read(uart_ble_fd, &uart_ble_buff[0], 1);
			if('A'!=uart_ble_buff[0]){
				continue;
			}
			number = read(uart_ble_fd, &uart_ble_buff[1], 1);
			if('T'!=uart_ble_buff[1]){
				continue;
			}
			number = read(uart_ble_fd, &uart_ble_buff[2], 1);
			if('+'!=uart_ble_buff[2]){
				continue;
			}
			//read
			number = read(uart_ble_fd, &uart_ble_buff[3], 24);
			printf("advert: %s\r\n", uart_ble_buff);
		}
		
		//AD structures
		while(1){
			//read first 3 bytes
			read(uart_ble_fd, &uart_ble_buff[27], 1);
			if('A'==uart_ble_buff[27]){
				read(uart_ble_fd, &uart_ble_buff[28], 1);
				if('T'==uart_ble_buff[28]){
					read(uart_ble_fd, &uart_ble_buff[29], 1);
					if('+'==uart_ble_buff[29]){
						//End of advertising
						scanRspLen=0;
						uart_ble_buff[0] = 'A';	uart_ble_buff[1] = 'T';	uart_ble_buff[2] = '+';
						break;
					}else{
						//Length > 0
						scanRspLen = uart_ble_buff[27];
						//AD type and data
						read(uart_ble_fd, &uart_ble_buff[30], scanRspLen-2);
					}
				}else{
					//Length > 0
					scanRspLen = uart_ble_buff[27];
					//AD type and data
					read(uart_ble_fd, &uart_ble_buff[29], scanRspLen-1);
				}
			}else if(0==uart_ble_buff[27]){
				//Length = 0
				scanRspLen=0; break;
			}else{
				//Length > 0
				scanRspLen = uart_ble_buff[27];
				//AD type and data
				read(uart_ble_fd, &uart_ble_buff[28], scanRspLen);
			}
			//Length and AD-type
			printf("len: %x, type: %x\r\n", uart_ble_buff[27], uart_ble_buff[28]);
			//0x09 - <<Complete Local Name>>
 			if(uart_ble_buff[28] == 0x09){
				break;
  			}else{
  				continue;
			}
		}
		
		//End of advertising
		if(0==scanRspLen)	continue;
		
		//find name
		//AT+BOARD=0xD5A556E5A194-30,XXXXXX
		printf("ble-dev-name: %s\r\n", &uart_ble_buff[29]);
		if(0==strncmp(&uart_ble_buff[29], "Matalab", 7)){
			mac_addr_buff[0]=uart_ble_buff[11];
			mac_addr_buff[1]=uart_ble_buff[12];
			mac_addr_buff[2]=uart_ble_buff[13];
			mac_addr_buff[3]=uart_ble_buff[14];
			mac_addr_buff[4]=uart_ble_buff[15];
			mac_addr_buff[5]=uart_ble_buff[16];
			mac_addr_buff[6]=uart_ble_buff[17];
			mac_addr_buff[7]=uart_ble_buff[18];
			mac_addr_buff[8]=uart_ble_buff[19];
			mac_addr_buff[9]=uart_ble_buff[20];
			mac_addr_buff[10]=uart_ble_buff[21];
			mac_addr_buff[11]=uart_ble_buff[22];
			mac_addr_buff[12]=0;
			return 1;
		}else{
			cnt--;
		}
	}
	
	return 0;
}


int connect_ble_device(void)
{
	int cnt = 1000;
	
	//AT+CON_128=D5A556E5A194,6E400001B5A3F393E0A9E50E24DCCA9E\r\n
	printf("----------------connect_ble_device----------------\r\n");
	printf("aim-mac-addr: 0x%s\r\n", mac_addr_buff);
	usleep(20000);
	//clear receive buffer
	tcflush(uart_ble_fd, TCIFLUSH);
	//send cmd
	strcpy(uart_ble_buff, "AT+CON_128=D5A556E5A194,6E400001B5A3F393E0A9E50E24DCCA9E\n");
	uart_ble_buff[11]=mac_addr_buff[0];
	uart_ble_buff[12]=mac_addr_buff[1];
	uart_ble_buff[13]=mac_addr_buff[2];
	uart_ble_buff[14]=mac_addr_buff[3];
	uart_ble_buff[15]=mac_addr_buff[4];
	uart_ble_buff[16]=mac_addr_buff[5];
	uart_ble_buff[17]=mac_addr_buff[6];
	uart_ble_buff[18]=mac_addr_buff[7];
	uart_ble_buff[19]=mac_addr_buff[8];
	uart_ble_buff[20]=mac_addr_buff[9];
	uart_ble_buff[21]=mac_addr_buff[10];
	uart_ble_buff[22]=mac_addr_buff[11];
	uart_ble_buff[60]=0;
	printf("Connect cmd: %s\r\n", uart_ble_buff);
	write(uart_ble_fd, uart_ble_buff, 58);//strlen(uart_ble_buff)
	usleep(2000000);
	//AT+OK\r\n0xD5A556E5A194 CON_SUCCESS
	//AT+OK\r\n0xD5A556E5A194 CON_FAIL
	//AT+OK\r\n
	number = read(uart_ble_fd, &uart_ble_buff[0], 5);
	uart_ble_buff[5]=0;
	printf("cmd ret: %s\r\n", uart_ble_buff);
	while(cnt){
		//read data
		//0xD5A556E5A194 CON_SUCCESS
		//0xD5A556E5A194 CON_SUCCESS
		//0xD5A556E5A194 CON_FAIL
		//sync to 0xD5A
		number = read(uart_ble_fd, &uart_ble_buff[0], 1);
		if('0'!=uart_ble_buff[0]){
			continue;
		}
		number = read(uart_ble_fd, &uart_ble_buff[1], 1);
		if('x'!=uart_ble_buff[1]){
			continue;
		}
		number = read(uart_ble_fd, &uart_ble_buff[2], 1);
		if(mac_addr_buff[0]!=uart_ble_buff[2]){
			continue;
		}
		number = read(uart_ble_fd, &uart_ble_buff[3], 1);
		if(mac_addr_buff[1]!=uart_ble_buff[3]){
			continue;
		}
		number = read(uart_ble_fd, &uart_ble_buff[4], 1);
		if(mac_addr_buff[2]!=uart_ble_buff[4]){
			continue;
		}
		
		//read
		number = read(uart_ble_fd, &uart_ble_buff[5], 18);
		uart_ble_buff[24] = 0;
		printf("conn-status: %s\r\n", &uart_ble_buff[0]);
		if(0==strncmp(&uart_ble_buff[15], "CON_SUCC", 8)){
			return 1;
		}else{
			cnt--;
		}
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
	
	//init com
	set_speed(uart_ble_fd, 115200);
	if (set_Parity(uart_ble_fd,8,1,'N')== FALSE) {
		fprintf(stderr, "fd0 Set Parity Error\n");
		close(uart_ble_fd);
		exit(1);
	}
	
	//test com
	if(0 == connect_ble_master()){
		fprintf(stderr, "Com Error\n");
		close(uart_ble_fd);
		exit(1);
	}
	
	//find device
	if(0 == find_ble_device()){
		fprintf(stderr, "Find device Error\n");
		close(uart_ble_fd);
		exit(1);
	}
	
	//connect device
	if(0 == connect_ble_device()){
		fprintf(stderr, "connect device Error\n");
		close(uart_ble_fd);
		exit(1);
	}
	
	//send write cmd
	printf("----------------send write cmd----------------\r\n");
	usleep(20000);
	//clear receive buffer
	tcflush(uart_ble_fd, TCIFLUSH);
	write(uart_ble_fd, "AT+W_DCH=1\n", 12);
	usleep(10000);
	//read
	number = read(uart_ble_fd, &uart_ble_buff[0], 6);
	uart_ble_buff[6] = 0;
	printf("Write Cmd ret: %s\r\n", &uart_ble_buff[0]);
	while(1){
		usleep(1000*1000);
		printf("please input a HEX value:\r\n");
		scanf("%x", &value);
		printf("write 0x%x\r\n", value);
		write(uart_ble_fd, &value, 2);
	}

	close(uart_ble_fd);
	exit(0);
}

