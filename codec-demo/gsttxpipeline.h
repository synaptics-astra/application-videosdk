// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _GSTTXPIPELINE_H_
#define _GSTTXPIPELINE_H_

#include "config.h"

typedef struct gst_tx_t
{
	char				ip[MAX_IP_ADDR_STR];
	int					port;

	// GST TX pipeline data
	GstElement			*pipeline;
	GstElement			*appsrc;
	GstElement			*udpsink;
	GstBus				*bus;
	int					gstTxStreaming;
	pthread_t			thread;
	pthread_mutex_t		tx_mutex;
	pthread_cond_t		tx_cond;
}gst_tx_t;

int StartGstTxPipeline(gst_tx_t *pGstTx);
int FeedGstTx(gst_tx_t *pGstTx, void *ptrData, int size, long long int nTimeStamp);
int TerminateGstTxPipeline(gst_tx_t *pGstTx);

#endif //_GSTTXPIPELINE_H_
