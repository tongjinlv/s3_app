#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#define MATATA_VIDIOC_G_CTRL	(VIDIOC_G_CTRL | 0x40000)
#define MATATA_VIDIOC_S_CTRL    (VIDIOC_S_CTRL | 0x40000)
struct matata_v4l2_control {
		__u32 	id;
		__s32	value;
//		__u32	user_pt;	//add to match allwinner platform kernel struct definition
};

int fd;
const char *input_dev = "/dev/video0";
const char *qctrl_name = NULL;
int qctrl_value = 0;

struct v4l2_capability cap;
struct v4l2_queryctrl qctrl;
int main(int argc, char **argv)
{
  int ret, i;
struct matata_v4l2_control ctrl;
  

  fd = open(input_dev, O_RDWR);
  if (fd < 0) {
    perror("open video failed");
    return -1;
  }
  printf("open video '%s' success, G_CTRL = %u\n", input_dev, VIDIOC_G_CTRL);
//struct v4l2_control ctrl;

  ctrl.id = V4L2_CID_EXPOSURE;
//  ctrl.id = V4L2_CID_EXPOSURE_AUTO;
  if (ioctl(fd, MATATA_VIDIOC_G_CTRL, &ctrl) < 0) {
    perror("get ctrl failed");
    ctrl.value = -999;
  }
printf("****kevin test, V4L2_CID_EXPOSURE, ctrl.value is %u\n", ctrl.value);
  ctrl.id = V4L2_CID_EXPOSURE_AUTO;
  ctrl.value = V4L2_EXPOSURE_MANUAL;
  if (ioctl(fd, MATATA_VIDIOC_S_CTRL, &ctrl) < 0) {
    perror("set ctrl failed, ctrl.id = V4L2_CID_EXPOSURE_AUTO");
    //ctrl.value = -999;
  }
  ctrl.id = V4L2_CID_EXPOSURE;
  ctrl.value = 0x1FFF;
  if (ioctl(fd, MATATA_VIDIOC_S_CTRL, &ctrl) < 0) {
    perror("set ctrl failed, ctrl.id = V4L2_CID_EXPOSURE");
    //ctrl.value = -999;

  }
for(i=0;i<10;i++)
{
  sleep(10);
  ctrl.id = V4L2_CID_EXPOSURE;
//  ctrl.user_pt = 0;
  if (ioctl(fd, MATATA_VIDIOC_G_CTRL, &ctrl) < 0) {
    perror("get ctrl failed");
    ctrl.value = -999;
  } 
printf("****kevin test, get exposure value 2 is %u\n", ctrl.value);
 ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
// ctrl.user_pt = 0;
  if (ioctl(fd, MATATA_VIDIOC_G_CTRL, &ctrl) < 0) {
    perror("get ctrl failed");
    ctrl.value = -999;
  } 
printf("****kevin test, get exposure absolute value 2 is %u\n", ctrl.value);
}

printf("****kevin test, ctrl.value is %u\n", ctrl.value);

return 0;
}

