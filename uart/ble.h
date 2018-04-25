#ifndef _BLE_H_
#define _BLE_H_
#include <stdio.h>

using namespace std;
class Ble
{
    public:
        Ble(const char *name);
		int openPort(void);
		int setPort(void);
		void *sendThread(void *);
		static void *recviceThread(void* __this);
		void recvice(void *tags);
		int bleWrite(const char *buf);
		int bleAck(const char *buf,const char *ack,int c);
		int bleReadack(const char *ack,int c);
		int bleInit(void);
	    void lanch(void);
		static  void * insert_pth(void*);
		int serach(char *mac);
		int disSerach(void);
		int disConnect(void);
		int Connect(char *mac);
		int begin(void);
		int writeCmd(char *buf,int Length);
    private:
	    int fd;
		int length;
        char revicebuf[200];
		const char *name;
};

#endif




