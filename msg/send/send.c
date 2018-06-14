#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <time.h>
#define MAX_TEXT 512
struct msg_st
{
	long int msg_type;
	char text[MAX_TEXT];
};
void get_oneline()
{
    FILE * fp;
    char val[102];
    const char *pathname="/sys/class/power_supply/battery/voltage_now";
    if(access(pathname, F_OK) != 0) {
    }

    if(access(pathname, R_OK) != 0) {
    }
    fp = fopen(pathname, "r");
    
    if (fp == NULL)return;
    if(fgets(val, 1000, fp) != NULL) {
        fclose(fp);
        int v=atoi(val);
        printf("bat=%.02fv\n",(float)v/1000000);
        return ;
    }
    fclose(fp);
}
void get_onelin(const char *pathname, char *val)
{
        FILE * fp;

        if(access(pathname, F_OK) != 0) {
        }

        if(access(pathname, R_OK) != 0) {
        }
        fp = fopen(pathname, "r");
        if (fp == NULL)return;

        if(fgets(val, 1000, fp) != NULL) {
                fclose(fp);
                return ;
        }
        fclose(fp);
}
void msg_send(char *msg)
{
    int running = 1;
	struct msg_st data;
	int msgid = -1;
	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
	if(msgid == -1)
	{
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
    data.msg_type = 1;    //注意2
    strcpy(data.text, msg);
    if(msgsnd(msgid, (void*)&data, MAX_TEXT, 0) == -1)
    {
        fprintf(stderr, "msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
	exit(EXIT_SUCCESS);
}
typedef struct CPU_PACKED         //定义一个cpu occupy的结构体  
{  
char name[20];             //定义一个char类型的数组名name有20个元素  
unsigned int user;        //定义一个无符号的int类型的user  
unsigned int nice;        //定义一个无符号的int类型的nice  
unsigned int system;    //定义一个无符号的int类型的system  
unsigned int idle;         //定义一个无符号的int类型的idle  
unsigned int iowait;  
unsigned int irq;  
unsigned int softirq;  
}CPU_OCCUPY;
double cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n)  
{  
    double od, nd;  
    double id, sd;  
    double cpu_use ;  
  
    od = (double) (o->user + o->nice + o->system +o->idle+o->softirq+o->iowait+o->irq);//第一次(用户+优先级+系统+空闲)的时间再赋给od  
    nd = (double) (n->user + n->nice + n->system +n->idle+n->softirq+n->iowait+n->irq);//第二次(用户+优先级+系统+空闲)的时间再赋给od  
  
    id = (double) (n->idle);    //用户第一次和第二次的时间之差再赋给id  
    sd = (double) (o->idle) ;    //系统第一次和第二次的时间之差再赋给sd  
    if((nd-od) != 0)  
    cpu_use =100.0- ((id-sd))/(nd-od)*100.00; //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used  
    else cpu_use = 0;  
    return cpu_use;  
}    
void get_cpuoccupy (CPU_OCCUPY *cpust)  
{  
    FILE *fd;  
    int n;  
    char buff[256];  
    CPU_OCCUPY *cpu_occupy;  
    cpu_occupy=cpust;  
  
    fd = fopen ("/proc/stat", "r");  
    fgets (buff, sizeof(buff), fd);  
  
    sscanf (buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle ,&cpu_occupy->iowait,&cpu_occupy->irq,&cpu_occupy->softirq);  
  
    fclose(fd);  
}  
double getCpuRate()  
{  
    CPU_OCCUPY cpu_stat1;  
    CPU_OCCUPY cpu_stat2;  
    double cpu;  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);  
    sleep(1);  
  
    //第二次获取cpu使用情况  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);  
  
    //计算cpu使用率  
    cpu = cal_cpuoccupy ((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);  
    printf("cpurate=%.02f%%\n",(float)cpu);
    return cpu;  
}  
int main(int argc, char *argv[])
{
    char value[102];
    if(strstr(argv[1],"test")!=0)msg_send("test");
    if(strstr(argv[1],"getbat")!=0)get_oneline();
    if(strstr(argv[1],"getcpurate")!=0)getCpuRate();
	
}
