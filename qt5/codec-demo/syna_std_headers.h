// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _SYNA_STD_HEADERS_H_
#define _SYNA_STD_HEADERS_H_

#include <assert.h>

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <getopt.h>
#include <poll.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ext/videodev2.h"
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>


#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include <drm/drm.h>
#include <gbm.h>

#include "logs.h"
#include "tsemaphore.h"
#include "queue.h"

#define true 1
#define false 0

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 240
#define DEFAULT_FPS 30
#define REQBUFS_COUNT 4
#define V4L2_BUF_FLAG_LAST  0x00100000

#define ROUNDUP(x, base)	((x + base - 1) & ~(base - 1))
#define ROUNDUP16(x)		ROUNDUP(x, 16)
#define ROUNDUP128(x)		ROUNDUP(x, 128)

typedef struct
{
	uint8_t *pBuffer;
	uint32_t nFilledLen;
	uint32_t nTimeStamp;
	uint32_t nFlags;
} queue_element_t;

#endif // _SYNA_STD_HEADERS_H_
