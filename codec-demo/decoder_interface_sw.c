#include "decoder_interface_sw.h"

static void *rx_feedLoop(void *data)
{
	dec_config_t *pCfg = (dec_config_t *)data;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;
	queue_element_t *tmpBuf = NULL;

	// Wait for start
	tsem_down(pDec->feed_loop_start_sem);
	while(pDec->is_running)
	{
		if(pCfg->src == CAM_H264)
		{
			codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
			tsem_down(codec_config->sharedH264EncoderQueueSem);
		}
		else
		{
			tsem_down(pDec->gstRx.h264DecoderQueueSem);
		}
		if(pDec->is_running == 0)
			break;
		FeedBufferToSWH264Decoder(pCfg);
	}

	/* Free up unused buffersfrom queue */
	if(pCfg->src == CAM_H264)
	{
		codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
		while(getquenelem(codec_config->sharedH264EncoderQueue) > 0)
		{
			tmpBuf = dequeue(codec_config->sharedH264EncoderQueue);
			free(tmpBuf->pBuffer);
			free(tmpBuf);
		}
	}
	else
	{
		while(getquenelem(pDec->gstRx.h264DecoderStreamQueue) > 0)
		{
			tmpBuf = dequeue(pDec->gstRx.h264DecoderStreamQueue);
			free(tmpBuf->pBuffer);
			free(tmpBuf);
		}
	}
}
static int startSWDecGstPipeline(dec_config_t *pCfg)
{
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;
	int ret = -1;

	if(pCfg->src == NW_H264 || pCfg->src == FILE_H264 || pCfg->src == CAM_H264)
	{
		ret = SetupSWH264Dec(pCfg);
		if(ret < 0)
		{
			LOG_ERR("H264 Decoder cannot be initialized\n");
			return -1;
		}
		LOG_DEF("H264 Decoder initialized\n");
		gst_rx_t *pGstRx = &pDec->gstRx;
		pGstRx->src = pCfg->src;
		pGstRx->fps = pCfg->fps;
		pGstRx->port = pCfg->port;
		strncpy(pGstRx->path, pCfg->path, MAX_FILE_PATH_STR);
		if(pCfg->src != CAM_H264)
		{
			ret = StartGstRxPipeline(pGstRx);
			if(ret < 0)
			{
				LOG_ERR("GST RX pipeline cannot be initialized\n");
				return -1;
			}
		}
		LOG_DEF("GST RX pipeline initialized\n");
	}

	pthread_create(&pDec->feed_loop_id, NULL, rx_feedLoop, (void *)pCfg);

	return 0;
}

void startSWDecoder(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);

	dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;

	pCfg->priv = malloc(sizeof(priv_sw_decoder_t));
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;

	memset(pDec, 0, sizeof(priv_sw_decoder_t));

	pDec->ffmpeg_dec = (struct ffmpeg_dec *)malloc(sizeof(ffmpeg_dec));
	memset(pDec->ffmpeg_dec, 0, sizeof (ffmpeg_dec));

	pDec->sess_cfg = sess_cfg;
	pDec->feed_loop_start_sem = (tsem_t *)malloc(sizeof(tsem_t));
	pDec->h264DecoderSeQueueSem = (tsem_t *)malloc(sizeof(tsem_t));
	tsem_init(pDec->feed_loop_start_sem, 0);
	tsem_init(pDec->h264DecoderSeQueueSem, 0);

	pthread_mutex_init(&pDec->qt_rendering_buffer_mutex, NULL);
	pDec->qt_rendering_buffer_index = -1;

	int ret = startSWDecGstPipeline(pCfg);
	if (ret < 0)
		return;

	pDec->total_frames = 0;
	pDec->is_running = 1;

	tsem_up(pDec->feed_loop_start_sem);
}

void stopSWDecoder(sess_config_t *sess_cfg)
{
	dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;

	if (pDec->is_running == 0)
	{
		// If session is not started at all, ublock the thread
		tsem_up(pDec->feed_loop_start_sem);
		return;
	}

	pDec->is_running = 0;
	// If no data is ever received, just unblock the feed_loop thread to exit
	if(pCfg->src != CAM_H264)
	{
		tsem_up(pDec->gstRx.h264DecoderQueueSem);
	}
	else
	{
		codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
		tsem_up(codec_config->sharedH264EncoderQueueSem);
	}

	// Wait for feedloop to exit
	if (pDec->feed_loop_id)
		pthread_join(pDec->feed_loop_id, NULL);

	if(pCfg->src != CAM_H264)
	{
		// Stop GST pipeline before stopping decoder
		TerminateGstRxPipeline(&pDec->gstRx);
	}
	StopSWH264Decoder(pCfg);
}

void waitForSWDecoderShutdown(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);

	dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;

	if(pDec){
		ShutdownSWH264Decoder(pCfg);

		free(pDec->feed_loop_start_sem);
		free(pDec->h264DecoderSeQueueSem);
		free(pDec->ffmpeg_dec);
		free(pCfg->priv);
		pCfg->priv = NULL;
	}
}
