#ifndef __V4L2HDMIEXT_H
#define __V4L2HDMIEXT_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <pthread.h>            /* POSIX Threads */
#include <semaphore.h>
#include "v4l2_types.h"

#define HDMI_RX_EXT 4

typedef struct __v4l2hdmiExt
{
    V4L2_HANDLE handle;         //handle of v4l2hdmiRxExt component
    void **mmap_virtual_output; /* Pointer tab of output video frames */
    int *mmap_size_output;
    int current_nb_buf_output;
    int width;
    int height;
    int pixelformat;
    int EndOfStream;

} v4l2hdmiExt;

#endif //__V4L2HDMIEXT_H
