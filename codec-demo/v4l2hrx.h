// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef __V4L2HRX_H
#define __V4L2HRX_H

#include "syna_std_headers.h"
#include "config.h"

#define HDMI_RX_EXT 4
#define VIDEO_MAX_PLANES 8
#define HDMI_VIDEO_DEVICE "/dev/video0"

typedef struct __v4l2hrx
{
	int handle;			//handle of v4l2hdmiRxExt component
	void **mmap_virtual_input; /* Pointer tab of output video frames */
	int *mmap_size_input;
	int current_nb_buf_output;
	int number_output_buffer;
	int hdmi_rx_fmt;
	int hdmi_rx_fd;
	int width;
	int height;
	int pixelformat;
	int EndOfStream;
	int fps;
	char path[30];

} v4l2hrx_t;

int events_unsubscribe(v4l2hrx_t * hrx);
int events_subscribe(v4l2hrx_t * hrx);
int v4l2_open_device(v4l2hrx_t * hrx);
int v4l2hrx_t_set_capture_format (v4l2hrx_t * hrx);
int v4l2hrx_t_get_capture_format (v4l2hrx_t * hrx);
int SetupHRx(v4l2hrx_t* pV4l2HRx);
void ReadHRxInput(v4l2hrx_t *pV4l2HRx, void** ptr, int* size, int* fb_index);
void EnqueHRxBuffer(v4l2hrx_t *pV4l2HRx, int index);
void ShutdownHRx(v4l2hrx_t *pV4l2HRx);
int StreamOnHRx(v4l2hrx_t* pV4l2HRx);

#endif //__V4L2HRX_H
