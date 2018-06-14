#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>

struct input_event buff; 
int fd; 
int read_nu; 
pthread_t time_id;
pthread_t time_id1;
pthread_t time_id2;
volatile int iTimeCount = 0;
volatile int iTimeCount1 = 0;
volatile char iTimeCountEn= 0;
volatile int iTimeCountPD= 0;
volatile int iTimeCountPCount= 0;
volatile int KeyPlay= 0;
void write_led(int fd,const char *cmd)
{
    int length=strlen(cmd);
    write(fd,cmd,length);
}
void time_base(void *arg)
{ 
    char start= 0;
    system("/root/shell/sysinit.sh");
    system("/root/shell/run.sh");
	int fd = open("/dev/mygpio", O_RDWR);
    while(1)
    {
            if(iTimeCountEn)
	    {
              sleep(2);
              if(iTimeCount>2){if(KeyPlay)system("/root/shell/pair.sh"); else {if(start)system("/root/shell/run.sh");else system("/root/shell/update.sh");start=!start;}}
              iTimeCountEn=0;
              iTimeCount=0;
        }else
	    {
              usleep(200000);
		}       
          

    }
}
void power_down(void *arg)
{
	while(1)
	{
		sleep(1);
		if(iTimeCountPCount)iTimeCountPD++;else iTimeCountPD=0;
		if(iTimeCountPD>1)
		{
			system("/root/shell/off.sh");
		}
	}
}
void keyworker(void *arg)
{
	struct input_event buff;
	int fdkey = open("/dev/input/event0", O_RDONLY); 
	if (fdkey < 0) {
		perror("can not open device usbkeyboard!");
		exit(1);
	}
	while(1)
	{
        //sleep(1);
		while(read(fdkey,&buff,sizeof(struct input_event))==0);
        if(buff.type==1)if(buff.value==1){if(KeyPlay==0)printf("KeyPlay Touch Down....\n");KeyPlay=1;}
        if(buff.type==1)if(buff.value==0){if(KeyPlay==1)printf("KeyPlay Touch Up......\n");KeyPlay=0;}
        
	}
	close(fdkey);
}
int main(int argc, char *argv[])
{
	int i = 0;
	fd = open("/dev/input/event1", O_RDONLY); //may be the powerkey is /dev/input/event1
	if (fd < 0) { 
		perror("can not open device usbkeyboard!"); 
		exit(1); 
	} 
	pthread_create(&time_id,NULL,(void *)time_base,NULL);
	pthread_create(&time_id1,NULL,(void *)power_down,NULL);
    pthread_create(&time_id2,NULL,(void *)keyworker,NULL);
	printf("--fd:%d--\n",fd);
	while(1)
	{
		read(fd,&buff,sizeof(struct input_event));
               // printf("type:%d code:%d value:%d\n",buff.type,buff.code,buff.value);
				if(buff.value)if(buff.type){if(iTimeCountPCount==0)printf("KeyPower Touch Down....\n"); iTimeCountPCount=1;}
				if(buff.type)if(buff.value==0){if(iTimeCountPCount==1)printf("KeyPower Touch Up....\n"); iTimeCountPCount=0;}
                if(buff.value)
                   {
                    iTimeCountEn=1;
                    iTimeCount++;
                   }
	} 
	close(fd); 
	return 1;
}
