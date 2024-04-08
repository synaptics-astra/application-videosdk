#include "gstrxpipeline.h"

extern int srtp_enabled;

void signalRxPathTerminate(gst_rx_t *pGstRx)
{
	pthread_mutex_lock(&pGstRx->rx_mutex);
	pGstRx->gstRxStreaming = 0;
	pthread_cond_signal(&pGstRx->rx_cond);
	pthread_mutex_unlock(&pGstRx->rx_mutex);
}

void signalRestartPipeline(gst_rx_t *pGstRx)
{
	LOG_SEQ("Restart GST pipeline\n");
	gst_element_set_state (pGstRx->pipeline, GST_STATE_NULL);
	gst_element_set_state (pGstRx->pipeline, GST_STATE_PLAYING);
}

static gboolean rx_bus_message (GstBus * bus, GstMessage * message, gst_rx_t *pGstRx)
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
			pGstRx->gstRxStreaming = 0;
			signalRxPathTerminate(pGstRx);
			break;
		}
	case GST_MESSAGE_EOS:
		LOG_SEQ("GST_MESSAGE_EOS\n");
		break;
	default:
		break;
	}

	return TRUE;
}

void * gstRxPathFeeder(void *arg)
{
	gst_rx_t *pGstRx = (gst_rx_t *) arg;

	if(pGstRx->gstRxStreaming){
		LOG_DEF("GSTRX:WORN: Thread(%s) already running(%lu)\n", __func__, pGstRx->thread);
		return NULL;
	}

	/* run */
	pGstRx->gstRxStreaming = 1;
	gst_element_set_state (pGstRx->pipeline, GST_STATE_PLAYING);

	/*lock the mutec*/
	pthread_mutex_lock(&pGstRx->rx_mutex);

	LOG_DEF("GSTRX:Rx: Thread (%s) enters while 1 loop\n ", __func__);
	while(pGstRx->gstRxStreaming){

		/*Wait till streaming condition is false*/
		pthread_cond_wait( &pGstRx->rx_cond, &pGstRx->rx_mutex);
		/*Application will feed rx data by calling gstRxFeed() */

		if(pGstRx->gstRxStreaming)
		{
			// Restart Pipeline
			signalRestartPipeline(pGstRx);
		}

	}
	LOG_DEF("GSTRX:Thread (%s) exit while 1 loop\n ", __func__);

	/*Close pipeline */
	gst_element_set_state (pGstRx->pipeline, GST_STATE_NULL);
	gst_object_unref (pGstRx->pipeline);
	return NULL;
}

static int ProcessSample(gst_rx_t *pGstRx, uint8_t* data, int size)
{
	int i;
	int nalu_size;
	int nalu_type;
	uint8_t	*pOMX_Config = NULL;
	int OMX_ConfigSize = 0;
	int skip = 0;
	//TODO: Need check if we need to set the below to TRUE
	bool short_header = false;

	SendBufferToV4L2H264Decoder(pGstRx, data, size, 0, short_header);
	return 0;
}

/* The appsink has received a buffer */
static GstFlowReturn new_sample (GstElement *sink, gst_rx_t *pGstRx)
{
	GstSample *sample;
	GstBuffer *buffer;
	GstMapInfo map;

	/* Retrieve the buffer */
	sample = gst_app_sink_pull_sample(GST_APP_SINK (sink));

	if (sample) {
		/* received buffer */
		buffer = gst_sample_get_buffer(sample);
		gst_buffer_map(buffer, &map, GST_MAP_READ);

		//LOG_DEF ("GSTRX:Received Sample: map.data(%p) map.size(%d)\n", map.data, map.size);
		ProcessSample(pGstRx, map.data, map.size);
		gst_buffer_unmap(buffer, &map);
		gst_sample_unref(sample);
		return GST_FLOW_OK;
	}
	return GST_FLOW_ERROR;
}

static GstFlowReturn eos_callback  (GstElement *sink, gst_rx_t *pGstRx)
{
	int eError= 0;
	queue_element_t* tmpBuf;
	int index;
	static unsigned long int ts = 330;

	/* check eos */
	gboolean eos = gst_app_sink_is_eos((GstAppSink *)sink);
	LOG_SEQ("**EOS **** Received eos = %d\n",eos);

	/* Signal to restart pipeline */
	pthread_cond_signal(&pGstRx->rx_cond);

	return GST_FLOW_OK;
#if 0
	if(eos)
	{
		tmpBuf = ( queue_element_t*) malloc(sizeof( queue_element_t));
		tmpBuf->nFlags = GST_BUFFER_FLAG_LAST;
		tmpBuf->nTimeStamp = ts;
		tmpBuf->nFilledLen =  0;
		tmpBuf->pBuffer = NULL;
		eError = queue(pGstRx->h264DecoderStreamQueue,tmpBuf);
		if(!eError) {
			tsem_up(pGstRx->h264DecoderQueueSem);
		}
		return GST_FLOW_OK;
	}
	return GST_FLOW_ERROR;
#endif
}

int StartGstRxPipeline(gst_rx_t *pGstRx)
{
	int ret = 0;
	char pipeline[1024];

	pthread_mutex_init(&pGstRx->rx_mutex, NULL);
	pthread_cond_init(&pGstRx->rx_cond, NULL);

	/*Initialize Gstreamer library*/
	gst_init (NULL, NULL);

	/*Get pipeline handle*/
	if(pGstRx->src == NW_H264)
	{
		if(srtp_enabled){
			snprintf(pipeline, 1024, "udpsrc name=myudpsrc ! capsfilter caps=\"application/x-srtp, payload=(int)96, ssrc=(uint)1356955624, srtp-key=(buffer)012345678901234567890123456789012345678901234567890123456789, srtp-cipher=(string)aes-128-icm, srtp-auth=(string)hmac-sha1-80, srtcp-cipher=(string)aes-128-icm, srtcp-auth=(string)hmac-sha1-80, roc=(uint)0\" ! srtpdec !  rtph264depay ! queue ! h264parse ! video/x-h264, stream-format=byte-stream, alignment=au ! appsink name=mysink emit-signals=true");
		}else
			snprintf(pipeline, 1024, "udpsrc name=myudpsrc caps = \"application/x-rtp, media=(string)video, clock-rate=(int)90000,encoding-name=(string)H264, payload=(int)96\" !"
				" rtph264depay ! queue ! h264parse ! video/x-h264, stream-format=byte-stream, alignment=au ! appsink name=mysink max-buffers=2 emit-signals=true");
	}
	else if(pGstRx->src == FILE_H264)
	{
		snprintf(pipeline, 1024, "filesrc name=myfilesrc ! video/x-h264,stream-format=byte-stream,alignment=nalu, framerate=%d/1 ! queue max-size-buffers=2 !"
				" h264parse ! video/x-h264, stream-format=byte-stream, alignment=au ! queue max-size-buffers=2 ! appsink name=mysink max-buffers=2 emit-signals=true", pGstRx->fps);
	}
	else if(pGstRx->src == FILE_H265)
	{
		snprintf(pipeline, 1024, "filesrc name=myfilesrc ! video/x-h265,stream-format=byte-stream,alignment=nalu, framerate=%d/1 ! queue max-size-buffers=2 !"
				" h265parse ! video/x-h265, stream-format=byte-stream, alignment=au ! queue max-size-buffers=2 ! appsink name=mysink max-buffers=2 emit-signals=true", pGstRx->fps);
	}
	else if(pGstRx->src == FILE_VP8)
	{
		snprintf(pipeline, 1024, "filesrc name=myfilesrc ! queue max-size-buffers=2 !"
				" matroskademux ! video/x-vp8, stream-format=byte-stream, alignment=frame ! queue max-size-buffers=2 ! appsink name=mysink max-buffers=2 emit-signals=true", pGstRx->fps);
	}
	else if(pGstRx->src == FILE_VP9)
	{
		snprintf(pipeline, 1024, "filesrc name=myfilesrc ! queue max-size-buffers=2 !"
				" matroskademux ! vp9parse ! video/x-vp9, stream-format=byte-stream, alignment=frame ! queue max-size-buffers=2 ! appsink name=mysink max-buffers=2 emit-signals=true", pGstRx->fps);
	}
	else if(pGstRx->src == FILE_AV1)
	{
		snprintf(pipeline, 1024, "filesrc name=myfilesrc ! queue max-size-buffers=2 !"
				" matroskademux ! av1parse ! queue max-size-buffers=2 ! appsink name=mysink max-buffers=2 emit-signals=true", pGstRx->fps);
	}

	pGstRx->pipeline = gst_parse_launch(pipeline, NULL);
	if(pGstRx->pipeline == NULL){
		LOG_ERR("GSTRX: ERROR: (%s) : Failed to create pipeline\n", __func__);
		return -1;
	}
	LOG_DEF("GSTRX: INFO: (%s) : Success: pipeline created(%p)\n", __func__, pGstRx->pipeline);

	/*Get Bus handle*/
	pGstRx->bus = gst_pipeline_get_bus (GST_PIPELINE (pGstRx->pipeline));
	if(pGstRx->bus == NULL){
		LOG_ERR("GSTRX: ERROR: (%s) : Failed to get pipeline BUS\n", __func__);
		return -1;
	}
	LOG_DEF("GSTRX: INFO: (%s) : Success: pipeline bus handle (%p)\n", __func__, pGstRx->bus);
	/* add watch for messages */
	gst_bus_add_watch (pGstRx->bus, (GstBusFunc) rx_bus_message, pGstRx);

	if(pGstRx->src == NW_H264)
	{
		/*Get appsink and udpsrc and set port*/
		pGstRx->udpsrc = gst_bin_get_by_name (GST_BIN(pGstRx->pipeline), "myudpsrc");
		if(pGstRx->udpsrc == NULL){
			LOG_ERR("GSTRX: ERROR: (%s) : Failed : to get myudpsrc\n", __func__);
			return -1;
		}
		LOG_DEF("GSTRX: INFO: Setting Rx path port (%d)\n", pGstRx->port);
		g_object_set (G_OBJECT (pGstRx->udpsrc), "port", pGstRx->port, NULL);
	}
	else if((pGstRx->src == FILE_H264) || (pGstRx->src == FILE_H265) || (pGstRx->src == FILE_VP8) || (pGstRx->src == FILE_VP9) || (pGstRx->src == FILE_AV1))
	{
		/*Get appsink and udpsrc and set port*/
		pGstRx->filesrc = gst_bin_get_by_name (GST_BIN(pGstRx->pipeline), "myfilesrc");
		if(pGstRx->filesrc == NULL){
			LOG_ERR("GSTRX: ERROR: (%s) : Failed : to get myfilesrc\n", __func__);
			return -1;
		}
		LOG_DEF("GSTRX: INFO: Setting Rx path file (%s)\n", pGstRx->path);
		g_object_set (G_OBJECT (pGstRx->filesrc), "location", pGstRx->path, NULL);
	}

	pGstRx->appsink = gst_bin_get_by_name (GST_BIN(pGstRx->pipeline), "mysink");
	if(pGstRx->appsink == NULL){
		LOG_ERR("GSTRX: ERROR: (%s) : Failed : to get appsink\n", __func__);
		return -1;
	}

	g_signal_connect (pGstRx->appsink, "eos", G_CALLBACK (eos_callback), pGstRx);
	g_signal_connect (pGstRx->appsink, "new-sample", G_CALLBACK (new_sample), pGstRx);

	pGstRx->gstRxStreaming = 0;
	/*Create a separate thread for pumping h264 encoder data to gstreamer pipeline*/
	ret = pthread_create( &pGstRx->thread, NULL, gstRxPathFeeder, (void*)pGstRx);

	return ret;
}

int TerminateGstRxPipeline(gst_rx_t *pGstRx)
{
	LOG_DEF("GSTRX:> Fn(%s)\n", __func__);

	if(pGstRx->gstRxStreaming){
		signalRxPathTerminate(pGstRx);
		pthread_join(pGstRx->thread, NULL);
		LOG_DEF("Terminated GstRxPath Success...\n");
	}
}

#define START_CODE_SIZE 4

#define NALU_TYPE_SLICE                                 1               // Slice non-idr picture
#define NALU_TYPE_DPA                                   2               // Slice data partition A
#define NALU_TYPE_DPB                                   3               // Slice data partition B
#define NALU_TYPE_DPC                                   4               // Slice data partition C
#define NALU_TYPE_IDR                                   5               // Slice (instantaneous decoding refresh),
#define NALU_TYPE_SEI                                   6               // Supplemental Enhancement Information
#define NALU_TYPE_SPS                                   7               // Sequence Parameter Set
#define NALU_TYPE_PPS                                   8               // Picture Parameter Set
#define NALU_TYPE_PD                                    9               // Access unit delimiter
#define NALU_TYPE_EOSEQ                                 10              // End Of SEQuence
#define NALU_TYPE_EOSTREAM                              11              // End Of STREAM
#define NALU_TYPE_FILL                                  12              // Filler data
#define NALU_TYPE_SPS_EXT                               13              // Sequence Parameter Set Extension
#define NALU_TYPE_AUX_SLICE                             19              // Slice Coded Auxillary pictue

int print_data(uint8_t* data, int nalu_size)
{

	int i;
	LOG_DEF("Received: ");
	for(i = 0; i < (START_CODE_SIZE + 1); i++){
		LOG_DEF(" %x ", data[i]);
	}
	LOG_DEF("\n");

	return 0;
}

int SendBufferToV4L2H264Decoder(gst_rx_t *pGstRx, uint8_t* data, int size, int flag, bool short_header)
{
	int eError= 0;
	queue_element_t* tmpBuf;
	int index;
	static unsigned long int ts = 330;

	tmpBuf = ( queue_element_t*) malloc(sizeof( queue_element_t));
	ts += 330;
	tmpBuf->nTimeStamp = ts;
	tmpBuf->nFlags = flag;
	tmpBuf->nFilledLen = size;
	tmpBuf->pBuffer = (uint8_t*) malloc(tmpBuf->nFilledLen + 1);
	if(short_header)
	{
		tmpBuf->pBuffer[0] = 0x0;
		memcpy((tmpBuf->pBuffer + 1), data, tmpBuf->nFilledLen);
		tmpBuf->nFilledLen++;
	}
	else
	{
		memcpy(tmpBuf->pBuffer, data, tmpBuf->nFilledLen);
	}

	eError = queue(pGstRx->h264DecoderStreamQueue, tmpBuf);
	if(!eError){
		tsem_up(pGstRx->h264DecoderQueueSem);
	}

	return 0;
}
