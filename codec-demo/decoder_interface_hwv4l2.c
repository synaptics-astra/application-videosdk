#include "decoder_interface_hwv4l2.h"

static void *rx_feedLoop(void *data)
{
	dec_config_t *pCfg = (dec_config_t *)data;
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	queue_element_t *tmpBuf = NULL;

	// Wait for start
	tsem_down(pDec->feed_loop_start_sem);
	while(pDec->is_running)
	{
#ifdef LOCAL_LOOPBACK_MODE
		if(pCfg->instance_id == LOCAL_LOOPBACK_DEC_INST){
			codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
			tsem_timed_down(codec_config->sharedH264QueueSem, 2*1000);
		}else{
			tsem_down(pDec->gstRx.h264DecoderQueueSem);
		}
#else
		tsem_down(pDec->gstRx.h264DecoderQueueSem);
#endif //LOCAL_LOOPBACK_MODE
		if(pDec->is_running == 0)
			break;
		FeedBufferToHWV4L2H264Decoder(pCfg);
	}

	/* Free up unused buffersfrom queue */
	while(getquenelem(pDec->gstRx.h264DecoderStreamQueue) > 0)
	{
		tmpBuf = dequeue(pDec->gstRx.h264DecoderStreamQueue);
		free(tmpBuf->pBuffer);
		free(tmpBuf);
	}
}

static int startV4L2DecGstPipeline(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int ret = -1;

	if(pCfg->src == NW_H264 || pCfg->src == FILE_H264 || pCfg->src == FILE_H265 || pCfg->src == FILE_VP8 || pCfg->src == FILE_VP9 || pCfg->src == FILE_AV1)
	{
		ret = SetupHWV4L2H264Dec(pCfg);
		if(ret < 0)
		{
			LOG_ERR("Decoder cannot be initialized\n");
			return -1;
		}
		LOG_SEQ("Decoder initialized\n");
		gst_rx_t *pGstRx = &pDec->gstRx;
		pGstRx->src = pCfg->src;
		pGstRx->fps = pCfg->fps;
		pGstRx->port = pCfg->port;
		strncpy(pGstRx->path, pCfg->path, MAX_FILE_PATH_STR);

#ifdef LOCAL_LOOPBACK_MODE
		if(pCfg->instance_id == LOCAL_LOOPBACK_DEC_INST){
			LOG_DEF("Local loopback for decoder pCfg->instance_id(%d)\n", pCfg->instance_id);
		}else{
			ret = StartGstRxPipeline(pGstRx);
			if(ret < 0)
			{
				LOG_ERR("GST RX pipeline cannot be initialized\n");
				return -1;
			}
			LOG_SEQ("GST RX pipeline initialized\n");
		}
#else
		ret = StartGstRxPipeline(pGstRx);
		if(ret < 0)
		{
			LOG_ERR("GST RX pipeline cannot be initialized\n");
			return -1;
		}
		LOG_SEQ("GST RX pipeline initialized\n");

#endif //LOCAL_LOOPBACK_MODE

	}

	pthread_create(&pDec->feed_loop_id, NULL, rx_feedLoop, (void *)pCfg);

	return 0;
}

void startHWV4L2Decoder(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);

	dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;

	pCfg->priv = malloc(sizeof(priv_hw_v4l2_decoder_t));
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;

	memset(pDec, 0, sizeof(priv_hw_v4l2_decoder_t));

	pDec->sess_cfg = sess_cfg;
	pDec->feed_loop_start_sem = (tsem_t *)malloc(sizeof(tsem_t));
	pDec->h264DecoderSeQueueSem = (tsem_t *)malloc(sizeof(tsem_t));
	tsem_init(pDec->feed_loop_start_sem, 0);
	tsem_init(pDec->h264DecoderSeQueueSem, 0);

	pthread_mutex_init(&pDec->qt_rendering_buffer_mutex, NULL);
	pDec->qt_rendering_buffer_index = -1;

	int ret = startV4L2DecGstPipeline(pCfg);
	if (ret < 0)
		return;

	pDec->total_frames = 0;
	pDec->is_running = 1;

	tsem_up(pDec->feed_loop_start_sem);
}

void stopHWV4L2Decoder(sess_config_t *sess_cfg)
{
	dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;

	if (pDec->is_running == 0)
	{
		// If session is not started at all, ublock the thread
		tsem_up(pDec->feed_loop_start_sem);
		return;
	}

	pDec->is_running = 0;
	// If no data is ever received, just unblock the feed_loop thread to exit
	tsem_up(pDec->gstRx.h264DecoderQueueSem);

	// Wait for feedloop to exit
	if (pDec->feed_loop_id)
		pthread_join(pDec->feed_loop_id, NULL);

	// Stop GST pipeline before stopping decoder
	TerminateGstRxPipeline(&pDec->gstRx);
	StopHWV4L2H264Decoder(pCfg);
}

void waitForHWV4L2DecoderShutdown(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);

	dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;

	if(pDec){
		ShutdownHWV4L2H264Decoder(pCfg);

		free(pDec->feed_loop_start_sem);
		free(pDec->h264DecoderSeQueueSem);
		free(pCfg->priv);
		pCfg->priv = NULL;
	}
}
