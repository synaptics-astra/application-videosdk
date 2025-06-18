// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _GSTRXPIPELINE_H_
#define _GSTRXPIPELINE_H_

#include "config.h"

typedef struct gst_rx_t
{
	/* GST RX pipeline data */
	dec_src_type_e		src;
	int					fps;
	char				path[MAX_FILE_PATH_STR];
	int					port;
	GstElement			*pipeline;
	GstElement			*appsink;
	GstElement			*udpsrc;
	GstElement			*filesrc;
	GstBus				*bus;
	int					gstRxStreaming;
	pthread_t			thread;
	pthread_mutex_t		rx_mutex;
	pthread_cond_t		rx_cond;
	tsem_t				*h264DecoderQueueSem;
	queue_t				*h264DecoderStreamQueue;
}gst_rx_t;

int StartGstRxPipeline(gst_rx_t *pGstRx);
int TerminateGstRxPipeline(gst_rx_t *pGstRx);
int SendBufferToH264Decoder(gst_rx_t *pGstRx, uint8_t* data, int size, int flag, bool short_header);
int SendBufferToV4L2H264Decoder(gst_rx_t *pGstRx, uint8_t* data, int size, int flag, bool short_header);
#endif //_GSTRXPIPELINE_H_