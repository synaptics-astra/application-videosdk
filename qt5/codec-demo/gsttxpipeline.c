// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "gsttxpipeline.h"

extern int srtp_enabled;

void signalTxPathTerminate(gst_tx_t *pGstTx)
{
	pthread_mutex_lock(&pGstTx->tx_mutex);
	pGstTx->gstTxStreaming = 0;
	pthread_cond_signal(&pGstTx->tx_cond);
	pthread_mutex_unlock(&pGstTx->tx_mutex);
}

static gboolean tx_bus_message (GstBus * bus, GstMessage * message, gst_tx_t *pGstTx)
{
	GST_DEBUG ("got message %s",
	gst_message_type_get_name (GST_MESSAGE_TYPE (message)));

	switch (GST_MESSAGE_TYPE (message))
	{
		case GST_MESSAGE_ERROR: {
			GError *err = NULL;
			gchar *dbg_info = NULL;

			gst_message_parse_error (message, &err, &dbg_info);
			g_printerr ("ERROR from element %s: %s\n",
			GST_OBJECT_NAME (message->src), err->message);
			g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
			g_error_free (err);
			g_free (dbg_info);
			pGstTx->gstTxStreaming = 0;
			signalTxPathTerminate(pGstTx);
			break;
		}
	case GST_MESSAGE_EOS:
		pGstTx->gstTxStreaming = 0;
		signalTxPathTerminate(pGstTx);
		break;
	default:
		break;
	}

	return TRUE;
}

void * gstTxPathFeeder(void *arg)
{
	gst_tx_t *pGstTx = (gst_tx_t *)arg;

	if(pGstTx->gstTxStreaming){
		LOG_DEF("GSTTX: WORN: Thread(%s) already running(%lu)\n", __func__, pGstTx->thread);
		return NULL;
	}

	/* run */
	pGstTx->gstTxStreaming = 1;
	gst_element_set_state (pGstTx->pipeline, GST_STATE_PLAYING);

	/*lock the mutec*/
	pthread_mutex_lock(&pGstTx->tx_mutex);

	LOG_DEF("GSTTX: Tx: Thread (%s) enters while 1 loop\n ", __func__);
	while(pGstTx->gstTxStreaming){

		/*Wait till streaming condition is false*/
		pthread_cond_wait( &pGstTx->tx_cond, &pGstTx->tx_mutex);
		/*Application will feed tx data by calling gstTxFeed() */

	}
	LOG_DEF("GSTTX: Thread (%s) exit while 1 loop\n ", __func__);

	pthread_mutex_unlock(&pGstTx->tx_mutex);
	/*Close pipeline */
	gst_element_set_state (pGstTx->pipeline, GST_STATE_NULL);
	gst_object_unref (pGstTx->pipeline);
	return NULL;
}

int StartGstTxPipeline(gst_tx_t *pGstTx)
{
	int ret = 0;

	pthread_mutex_init(&pGstTx->tx_mutex, NULL);
	pthread_cond_init(&pGstTx->tx_cond, NULL);

	/*Initialize Gstreamer library*/
	gst_init (NULL, NULL);

	/*Get pipeline handle*/
	if(srtp_enabled){
		pGstTx->pipeline = gst_parse_launch("appsrc name = myappsrc ! h264parse ! rtph264pay ! capsfilter caps= \"application/x-rtp, payload=(int)96, ssrc=(uint)1356955624\" ! srtpenc key=\"012345678901234567890123456789012345678901234567890123456789\" ! udpsink name = myudpsink", NULL);
	}else
		pGstTx->pipeline = gst_parse_launch("appsrc name = myappsrc ! h264parse ! rtph264pay ! udpsink name = myudpsink", NULL);

	if(pGstTx->pipeline == NULL){
		LOG_ERR("GSTTX: ERROR: (%s) : Failed to create pipeline\n", __func__);
		return -1;
	}
	LOG_DEF("GSTTX: INFO: (%s) : Success: pipeline created(%p)\n", __func__, pGstTx->pipeline);

	/*Get Bus handle*/
	pGstTx->bus = gst_pipeline_get_bus (GST_PIPELINE (pGstTx->pipeline));
	if(pGstTx->bus == NULL){
		LOG_ERR("GSTTX: ERROR: (%s) : Failed to get pipeline BUS\n", __func__);
		return -1;
	}
	LOG_DEF("GSTTX: INFO: (%s) : Success: pipeline bus handle (%p)\n", __func__, pGstTx->bus);
	/* add watch for messages */
	gst_bus_add_watch (pGstTx->bus, (GstBusFunc) tx_bus_message, pGstTx);

	/*Get appsrc and udpsink and set remote ip and port*/
	pGstTx->udpsink = gst_bin_get_by_name (GST_BIN(pGstTx->pipeline), "myudpsink");
	if(pGstTx->udpsink == NULL){
		LOG_ERR("GSTTX: ERROR: (%s) : Failed : to get udpsink\n", __func__);
		return -1;
	}
	LOG_DEF("GSTTX: INFO: Setting host ip(%s) and port (%d)\n", pGstTx->ip, pGstTx->port);
	g_object_set (G_OBJECT (pGstTx->udpsink), "host", pGstTx->ip, NULL);
	g_object_set (G_OBJECT (pGstTx->udpsink), "port", pGstTx->port, NULL);

	pGstTx->appsrc = gst_bin_get_by_name (GST_BIN(pGstTx->pipeline), "myappsrc");
	if(pGstTx->appsrc == NULL){
		LOG_ERR("GSTTX: ERROR: (%s) : Failed : to get appsrc\n", __func__);
		return -1;
	}

	/*Create a separate thread for pumping h264 encoder data to gstreamer pipeline*/
	ret = pthread_create( &pGstTx->thread, NULL, gstTxPathFeeder, (void*)pGstTx);

	return ret;
}

/*Function which actually feeds the data to gstreamer pipeline from application */
int FeedGstTx(gst_tx_t *pGstTx, void *ptrData, int size, long long int nTimeStamp){

	int ret;
	GstBuffer *buffer = NULL;
	GstMapInfo info;

	if(!pGstTx->gstTxStreaming){
		LOG_DEF("GSTTX: ERROR: (%s) Gstreamer pipeline not running. don't feed data\n", __func__);
		return -1;
	}

	buffer = gst_buffer_new_allocate(NULL, size, NULL);
	if(buffer ==NULL){
		LOG_DEF("GSTTX: Failed to allocate gst_buffer in Fn(%s)\n", __func__);
		return -1;
	}

	gst_buffer_map(buffer, &info, GST_MAP_WRITE);
	memcpy((info.data), ptrData, size);
	GST_BUFFER_DTS(buffer) = GST_BUFFER_PTS(buffer) = nTimeStamp;
	gst_buffer_unmap(buffer, &info);

	ret = gst_app_src_push_buffer(GST_APP_SRC(pGstTx->appsrc), buffer);
	//LOG_DEF("GSTTX: Fn(%s) returned(%d) pusing data of size(%d)\n", __func__, ret, size);
	return ret;
}


int TerminateGstTxPipeline(gst_tx_t *pGstTx)
{
	LOG_DEF("GSTTX: > Fn(%s)\n", __func__);

	if(pGstTx->gstTxStreaming){
		signalTxPathTerminate(pGstTx);
		pthread_join(pGstTx->thread, NULL);
		LOG_DEF("Terminated GstTxPath Success...\n");
	}
}
