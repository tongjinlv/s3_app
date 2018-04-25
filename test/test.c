#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <time.h>
struct input_event buff;
static struct input_event data;
int fd;
int read_nu;
void get_oneline(const char *pathname, char *val)
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

int main(int argc, char *argv[])
{
      
      int delay=atoi(argv[1]);
      char value[102],time1[100];
      get_oneline("/sys/class/power_supply/battery/voltage_now",value);
      int v=atoi(value);
      printf("read:%s  %.02f",value,(float)v/1000000);
      time_t timep;
      struct tm *p;
      timep=time(0);
      p=localtime(&timep); /*取得当地时间*/
      printf ("%d %d %d  \n", (p->tm_hour),( p->tm_min), p->tm_sec);
      FILE *fp;
      fp=fopen("/root/nande/test/date_bat_reord.csv","a+");
      fprintf(fp,"start\n");
      fclose(fp);
      while(1)
      {
          timep=time(0);
          p=localtime(&timep);
          get_oneline("/sys/class/power_supply/battery/voltage_now",value);
          int v=atoi(value);
          char buf[200];
          printf("%d:%d:%d,%.02f\r\n", (p->tm_hour),( p->tm_min), p->tm_sec,(float)v/1000000);
          fp=fopen("/root/nande/test/date_bat_reord.csv","a+");
          fprintf(fp,"%d:%d:%d,%.02f\n", (p->tm_hour),( p->tm_min), p->tm_sec,(float)v/1000000);
          fclose(fp);
          sleep(delay);
      }
      return 1;
}

