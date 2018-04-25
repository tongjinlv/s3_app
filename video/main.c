#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>           
#include <fcntl.h>         
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>        
#include <linux/videodev2.h>
#include <pthread.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define REQ_COUNT 6
//#define REQ_COUNT 9
#define uchar unsigned char

int g_width = 640;
int g_height = 480;

typedef struct Image 
{
	unsigned int width;
	unsigned int height;
	unsigned int channel;
	unsigned int size;

	unsigned char* data; 
}Image;

Image *Cameraimge;
Image *Yimge;

int YUVimageSave(char *path,unsigned char *data,unsigned int length)
{
    FILE* fp = fopen(path, "wb");    
    if (fp == NULL)
    {
       printf("Can not write to file:%s\n", path);
       return -1;
    }
    fwrite(data, 1, length, fp);
    fflush(fp);
    fclose(fp);
	printf("SaveImge to %s \n" , path );
    return 0;
}


struct buffer {
	void *                  start;Image *Cameraimge;
	Image *Yimge;
	size_t                  length;
};

static char *           dev_name        = "/dev/video0"; 
//static char *           dev_name        = "/dev/video1";
static int              fd              = -1;
struct buffer *         buffers         = NULL;
// static unsigned int     n_buffers       = 0;
// static int iFlagCamera = 0;
volatile int iFlagCamera = 0;


FILE *file_fd;
int i;
int r;

struct v4l2_capability cap;
struct v4l2_format fmt;
struct v4l2_buffer buf[REQ_COUNT];
struct v4l2_requestbuffers req;
struct v4l2_buffer tmp_buf;
enum v4l2_buf_type type;

struct timeval tv;
struct timeval tp1;	
struct timeval tp3;
struct timeval tp4;

// void * mem[REQ_COUNT];

fd_set fds;

#define V4L2_CAP_VIDEO_CAPTURE		0x00000001  /* Is a video capture device */
#define V4L2_CAP_VIDEO_OUTPUT		0x00000002  /* Is a video output device */
#define V4L2_CAP_VIDEO_OVERLAY		0x00000004  /* Can do video overlay */
#define V4L2_CAP_VBI_CAPTURE		0x00000010  /* Is a raw VBI capture device */
#define V4L2_CAP_VBI_OUTPUT		0x00000020  /* Is a raw VBI output device */
#define V4L2_CAP_SLICED_VBI_CAPTURE	0x00000040  /* Is a sliced VBI capture device */
#define V4L2_CAP_SLICED_VBI_OUTPUT	0x00000080  /* Is a sliced VBI output device */
#define V4L2_CAP_RDS_CAPTURE		0x00000100  /* RDS data capture */
#define V4L2_CAP_VIDEO_OUTPUT_OVERLAY	0x00000200  /* Can do video output overlay */
#define V4L2_CAP_HW_FREQ_SEEK		0x00000400  /* Can do hardware frequency seek  */
#define V4L2_CAP_RDS_OUTPUT		0x00000800  /* Is an RDS encoder */

/* Is a video capture device that supports multiplanar formats */
#define V4L2_CAP_VIDEO_CAPTURE_MPLANE	0x00001000
/* Is a video output device that supports multiplanar formats */
#define V4L2_CAP_VIDEO_OUTPUT_MPLANE	0x00002000

#define V4L2_CAP_TUNER			0x00010000  /* has a tuner */
#define V4L2_CAP_AUDIO			0x00020000  /* has audio support */
#define V4L2_CAP_RADIO			0x00040000  /* is a radio device */
#define V4L2_CAP_MODULATOR		0x00080000  /* has a modulator */

#define V4L2_CAP_READWRITE              0x01000000  /* read/write systemcalls */
#define V4L2_CAP_ASYNCIO                0x02000000  /* async I/O */
#define V4L2_CAP_STREAMING              0x04000000  /* streaming I/O ioctls */

#define V4L2_MBUS_FMT_YUYV8_2X8          0x2008


void getCameraCapabilities(int iCameraCapabilities)
{
	if(iCameraCapabilities&V4L2_CAP_VIDEO_CAPTURE)
	{
		printf("!!!!**** camera ****!!!! Is a video capture device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_OUTPUT)
	{
		printf("**** camera **** Is a video output device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_OVERLAY)
	{
		printf("**** camera **** Can do video overlay\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VBI_CAPTURE)
	{
		printf("**** camera **** Is a raw VBI capture device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VBI_OUTPUT)
	{
		printf("**** camera **** Is a raw VBI output device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_SLICED_VBI_CAPTURE)
	{
		printf("**** camera **** Is a sliced VBI capture device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_SLICED_VBI_OUTPUT)
	{
		printf("**** camera **** Is a sliced VBI output device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_RDS_CAPTURE)
	{
		printf("**** camera **** RDS data capture\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_OUTPUT_OVERLAY)
	{
		printf("**** camera **** Can do video output overlay\n");
	}

	if(iCameraCapabilities&V4L2_CAP_HW_FREQ_SEEK)
	{
		printf("**** camera **** Can do hardware frequency seek\n");
	}

	if(iCameraCapabilities&V4L2_CAP_RDS_OUTPUT)
	{
		printf("**** camera **** Is an RDS encoder\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_CAPTURE_MPLANE)
	{
		printf("**** camera **** Is a video capture device that supports multiplanar formats\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_OUTPUT_MPLANE)
	{
		printf("**** camera **** Is a video output device that supports multiplanar formats\n");
	}

	if(iCameraCapabilities&V4L2_CAP_TUNER)
	{
		printf("**** camera **** has a tuner\n");
	}

	if(iCameraCapabilities&V4L2_CAP_AUDIO)
	{
		printf("**** camera **** has audio support\n");
	}

	if(iCameraCapabilities&V4L2_CAP_RADIO)
	{
		printf("**** camera **** is a radio device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_MODULATOR)
	{
		printf("**** camera **** has a modulator\n");
	}

	if(iCameraCapabilities&V4L2_CAP_READWRITE)
	{
		printf("**** camera **** read/write systemcalls\n");
	}

	if(iCameraCapabilities&V4L2_CAP_ASYNCIO)
	{
		printf("**** camera **** async I/O\n");
	}

	if(iCameraCapabilities&V4L2_CAP_STREAMING)
	{
		printf("!!!!**** camera ****!!!! streaming I/O ioctls\n");
	}
}
/*  Four-character-code (FOURCC) */
#define v4l2_fourcc(a, b, c, d)\
((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))
#define V4L2_PIX_FMT_JZ420B  v4l2_fourcc('J', 'Z', '1', '2') /* 12  YUV 4:2:0 B   */

// #include "imgprocess.h"

#include <sys/time.h>

#define ALIGN_16B(x) (((x) + (15)) & ~(15))
void thread2(void)
{

	int iCounterCamera = 0;
	int iCounter100frame = 0;

// unsigned char head[54] = {0x42,0x4d,0x36,0x10,0x0e,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,0x00,0x00,0x80,0x02,0x00,0x00,0xe0,0x01,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
	
//	int g_nWidth  = 640;
//	int g_nHeight = 480;
//       int g_nWidth  = 320;
//       int g_nHeight = 240;

	int g_nWidth  = g_width;
	int g_nHeight = g_height;

	if((fd=open(dev_name,O_RDWR | O_NONBLOCK,0))<0)
	{
		printf("Error   66666: open %s\n",dev_name);

		// return 0;
		return;
	}
	else
	{
		printf("****  Open camera video0 success!!!! fd=%d\n", fd);
	}


	//return ;//for test bill

	struct v4l2_fmtdesc fmtd; 
	int ret = 0;
	memset(&fmtd, 0, sizeof(fmtd)); 
	fmtd.index = 0; 
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 

	while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmtd)) == 0) 
	{ 
		fmtd.index++; 
		printf("\n...... pixelformat = ''%c%c%c%c'', description = ''%s'' \n", fmtd.pixelformat & 0xFF, (fmtd.pixelformat >> 8) & 0xFF, (fmtd.pixelformat >> 16) & 0xFF, (fmtd.pixelformat >> 24) & 0xFF, fmtd.description); 
	}


	if(

		(fd, VIDIOC_QUERYCAP, &cap)<0)
	{
		printf("Error: query cap\n");

		// return 0;
		return;
	}
	printf("\n capabilities= 0x%08x\n",cap.capabilities);
	int iCapCapabilities=cap.capabilities;

	// if(ioctl(fd, VIDIOC_S_INPUT, 0)<0)
	
	int index = 0;
	if(ioctl(fd, VIDIOC_S_INPUT, &index)<0)
	{
		printf("Error:VIDIOC_S_INPUT \n");

		// return 0;
		return;
	}
	struct v4l2_format fmt2;
	fmt2.type=V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	ret = ioctl(fd,VIDIOC_G_FMT, &fmt2);
	printf("VIDIOC_G_FMT ret=%d \n", ret);

	//memset ( &fmt, 0, sizeof(fmt) );
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;

	fmt.fmt.pix.pixelformat = v4l2_fourcc('Y', 'U', '1', '2');//V4L2_PIX_FMT_YUYV;// V4L2_PIX_FMT_JZ420B;// V4L2_PIX_FMT_YUV422P;// V4L2_PIX_FMT_YUYV;// V4L2_PIX_FMT_SBGGR8;// V4L2_PIX_FMT_YUYV;// 0x55595659;// V4L2_PIX_FMT_YUV420;//V4L2_MBUS_FMT_YUYV8_2X8;// V4L2_PIX_FMT_JPEG
//        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0)
	{
		printf("set format failed\n");
		return ;
	}

	printf("**** fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; ****\n");
	printf("pixelformat = ''%c%c%c%c''\n", fmt2.fmt.pix.pixelformat & 0xFF, (fmt2.fmt.pix.pixelformat >> 8) & 0xFF, (fmt2.fmt.pix.pixelformat >> 16) & 0xFF, (fmt2.fmt.pix.pixelformat >> 24) & 0xFF); 

	req.count               = REQ_COUNT ;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if(ioctl(fd, VIDIOC_REQBUFS, &req)<0)
	{
		printf("Error:VIDIOC_REQBUFS\n");

		// return 0;
		return;
	}

	printf("req.count=%d\n",req.count);

	buffers = calloc (req.count, sizeof (*buffers));
	printf(" chenmuyuan767676767sizeof (*buffers) =%08x\n", sizeof (*buffers));

	for(i=0;i<req.count;i++)
	{
		buf[i].type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf[i].memory      = V4L2_MEMORY_MMAP;
		buf[i].index       = i;
		printf("**chenmuyuan 1**  buf.length=%d \n",buf[i].length);

		if(ioctl(fd, VIDIOC_QUERYBUF, buf+i)<0)
		{
			printf("Error:VIDIOC_QUERYBUF\n");

			// return 0;
			return;
		}


		printf("**cmy 2** i=%d buf.length=%d, \n",buf[i].length);
	    //‭1FDFF0-1FA400‬    3137536-3110400=27136  460800  3110400
		
		buffers[i].length =buf[i].length;//14400
		buffers[i].start =
			mmap (NULL /* start anywhere */,   
				  buf[i].length,
				  PROT_READ | PROT_WRITE /* required */,
				  MAP_SHARED /* recommended */,
				  fd, buf[i].m.offset);

		//printf("**** i=%d buf.length=%d, buffers[i].start=%08x buf.m.offset=%08x\n", i, buf[i].length, *( (unsigned int *) (buffers[i].start) ), buf.m.offset);
		printf("**** i=%d buf.length=%d, buffers[i].start=%08x buf.m.offset=%08x\n", i, buf[i].length, *( (unsigned int *) (buffers[i].start) ), buf[i].m.offset);


		buf[i].type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf[i].memory      = V4L2_MEMORY_MMAP;
		buf[i].index       = i;

		if(ioctl(fd, VIDIOC_QBUF, buf+i)<0)
		{
			printf("Error: VIDIOC_QBUF\n");
		}
	}

	type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd, VIDIOC_STREAMON, &type)<0)
	{
		printf("Error: VIDIOC_STREAMON\n");

		// return 0;
		return;
	}
	
	gettimeofday(&tv, NULL);
	int iSecond = tv.tv_sec;
	int iuSecond = tv.tv_usec;
	int iCostTime = 0;
	
	int iSecond3 = tv.tv_sec;
	int iuSecond3 = tv.tv_usec;
	int iCostTime3 = 0;
	
	int iSecond4 = tv.tv_sec;
	int iuSecond4 = tv.tv_usec;
	int iCostTime4 = 0;
	
	int iCameraCounter = 0;
	
	int iFrameCounter = 0;
	int iFrameCounter3 = 0;
	int iFrameCounter4 = 0;
	iFlagCamera=50;
	while(1)
	{
		if(iFlagCamera>1000)
		{
			break;
		}

		iFrameCounter++;
		
		if(iFrameCounter%100==0)
		{
		 	gettimeofday(&tp1, NULL);
		 	
		 	iCostTime = (tp1.tv_sec - iSecond)*1000000 + tp1.tv_usec - iuSecond;
		 	printf("@@@@ 100 frame=%u us, fps=%f\n",iCostTime, 1000000*100.0/iCostTime) ;
		 	
		 	iSecond = tp1.tv_sec;
		 	iuSecond = tp1.tv_usec;
		}

		FD_ZERO (&fds);
		FD_SET (fd, &fds);
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select (fd + 1, &fds, NULL, NULL, &tv);
		//printf("------------r=%d\n",r);

		if(r<0)
		{
			
			printf("**** Error:select error no %d\n", errno);
//			 continue;
			// return 0;
			return;
		}

		tmp_buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		tmp_buf.memory      = V4L2_MEMORY_MMAP;
		tmp_buf.index = 0;

#if 1
		if(ioctl(fd, VIDIOC_DQBUF, &tmp_buf)<0)
		{
			printf("**** Error:VIDIOC_DQBUF\n");

			// return 0;
			continue;
			//    return;
		}
#endif

		iFrameCounter3++;
		if(iFrameCounter3%100==0)
		{
		 	gettimeofday(&tp3, NULL);
		 	
		 	iCostTime3 = (tp3.tv_sec - iSecond3)*1000000 + tp3.tv_usec - iuSecond3;
			printf("####100 frame=%u us, fps=%f\n", iCostTime3, 1000000*100.0/iCostTime3) ;
		 	
		 	iSecond3 = tp3.tv_sec;
		 	iuSecond3 = tp3.tv_usec;
		}
		
		
		printf("test fps=29.999165 ...11...\n");	

		// if(j>200)
		// if(j>20)
		if(iFlagCamera)
		{
			iCameraCounter++;
			iFlagCamera--;
			printf("test fps=29.999084 ...1...\n");
			char GRAYname[100];

			// sprintf(GRAYname, "gray%04d.bmp", j);
			// sprintf(GRAYname, "/sdcard/gray%04d.bmp", j);
			sprintf(GRAYname, "gray%04d.yuv", iCameraCounter);
			Cameraimge->data =  buffers[tmp_buf.index].start;
			int l=ALIGN_16B(Cameraimge->width) * Cameraimge->height * 3 / 2;
			printf("ALIGN_16B=%d\r\n",l);
			printf("buffers[tmp_buf.index].length=%d",buffers[tmp_buf.index].length);
			YUVimageSave(GRAYname,buffers[tmp_buf.index].start,buffers[tmp_buf.index].length);
			printf ("**** write BMP success j=%d!!!!\n", iCameraCounter);
		}


		tmp_buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		tmp_buf.memory      = V4L2_MEMORY_MMAP;

		if(ioctl(fd, VIDIOC_QBUF, &tmp_buf)<0)
		{
			printf("#### Error:VIDIOC_QBUF\n");

			// return 0;
			continue;
		}
		
		iFrameCounter4++;
		if(iFrameCounter4%100==0)
		{
			gettimeofday(&tp4, NULL);
			
			iCostTime4 = (tp4.tv_sec - iSecond4)*1000000 + tp4.tv_usec - iuSecond4;
			printf("@@@@ 100 frame=%u us, fps=%f\n",iCostTime, 1000000*100.0/iCostTime) ;
			printf("####100 frame=%u us, fps=%f\n", iCostTime3, 1000000*100.0/iCostTime3) ;
			printf("^^^^ 100 frame=%u us, fps=%f\n", iCostTime4, 1000000*100.0/iCostTime4) ;
			//	fps = 1/( (iCostTime4/1000000)/100 );
			printf("^^^^ 100 frame=%u us, fps=%f iCounter100frame=%d\n", iCostTime4, 1000000*100.0/iCostTime4, iCounter100frame) ;
			//	printf("&&&&video0 100 frame=%u us, fps=%f iCounter100frame=%d\n", iCostTime4, 1000000*100.0/iCostTime4, iCounter100frame) ;
			
			iSecond4 = tp4.tv_sec;
			iuSecond4 = tp4.tv_usec;
			
			iCounter100frame++;
			printf("\n\n") ;
		}
	}

	if(ioctl(fd, VIDIOC_STREAMOFF, &type)<0)
	{
		printf("Error:VIDIOC_STREAMOFF\n");

		// return 0;
		return;
	}

	printf("\n capabilities= 0x%08x, iCapCapabilities=0x%08x \n",cap.capabilities, iCapCapabilities);
	getCameraCapabilities(iCapCapabilities);

	for(i=0;i<req.count;i++)
	{
		munmap(buffers[i].start, buf[i].length);
	}

	close(fd);

	// return 0;
	return;
} 

int main(int argc, char **argv)
{
	pthread_t id1;
	pthread_t id2;
	int ret2;

	Cameraimge = (Image*)malloc(sizeof(Image));
	Cameraimge->width = g_width;
	Cameraimge->height = g_height;
	Cameraimge->channel = 1;
	Cameraimge->data = NULL;

	Yimge = (Image*)malloc(sizeof(Image));;
	Yimge->width = g_width;
	Yimge->height = g_height;
	Yimge->channel = 1;
	Yimge->data = malloc( Yimge->width *Yimge->height * Yimge->channel );;

	//ret2=pthread_create(&id1,NULL,(void *) thread1,NULL);

	if(ret2!=0)
	{
		printf ("Create pthread1 error!\n");
		// exit (1);

		//return 0;
	}

	ret2=pthread_create(&id2,NULL,(void *) thread2,NULL);

	if(ret2!=0)
	{
		printf ("Create pthread2 error!\n");
		// exit (1);

		return 0;
	}

	//pthread_join(id1,NULL);
    pthread_join(id2,NULL);
	return 0;
}

