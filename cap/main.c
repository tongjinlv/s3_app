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
#include <sys/ioctl.h>

static int xioctl(int fd, int request, void *arg)
{
    int r;
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
        return r;
}
int main(void)
{
	int fd;
    struct v4l2_capability caps = {0};
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 320;
    fmt.fmt.pix.height = 240;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
	fd = open("/dev/video0", O_RDWR);
	if (fd == -1)
	{
        perror("Opening Video device");
    }
    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps))
    {
        perror("Querying Capabilites");
        return 1;
    }
    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
    {
        perror("Setting Pixel Format");
        return 1;
    }
  
    return 1;
}
