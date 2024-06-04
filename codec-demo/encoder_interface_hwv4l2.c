// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "encoder_interface_hwv4l2.h"
#include "v4l2camera.h"
#include "v4l2hrx.h"

static int SetupNV12cam(enc_config_t *pCfg)
{
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	pEnc->temp_data = (uint8_t*) malloc(pCfg->width * pCfg->height *2);
	return 0;
}

static YUYVtoNV12Converter(uint8_t* dst,uint8_t* src,int width, int height, int isYUYV)
{
	int i,j;

	uint8_t* pyFrame = dst;
	uint8_t* puvFrame = pyFrame + width*height* sizeof(uint8_t); // Cb

	int uvOffset = width * 2 * sizeof(uint8_t);

	for(i=0; i<height-1; i++)
	{
		for(j=0; j<width; j+=2)
		{
			auto evenRow = ((i&1) == 0);
			if(isYUYV)
				*pyFrame++ = *src++;
			if (evenRow)
			{
				uint16_t calc = *src;
				calc += *(src + uvOffset);
				calc /= 2;
				*puvFrame++ = (uint8_t) calc;
			}
			++src;
			*pyFrame++ = *src++;
			if (evenRow)
			{
				uint16_t calc = *src;
				calc += *(src + uvOffset);
				calc /= 2;
				*puvFrame++ = (uint8_t) calc;
			}
			++src;
			if(!(isYUYV))
				*pyFrame++ = *src++;
		}
	}
}
static void ReadFrameFromNV12cam(enc_config_t *pCfg, int index, void* ptr, int *size)
{
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;

	memset(pEnc->temp_data,0,pCfg->width * pCfg->height *2);
	//LOG_DEF("ReadFrameFromNV12cam: pCfg: 0x%x, index: %d, ptr: 0x%x, size: %d\n", pCfg, index, ptr, *size);
	if((pCfg->src == CAM_YUYV) || (pCfg->src == HDMI_UYVY)) {
		int isYUYV = (pCfg->src == CAM_YUYV) ? 1 : 0;
		YUYVtoNV12Converter((uint8_t*)pEnc->temp_data,(uint8_t* )ptr,pCfg->width,pCfg->height, isYUYV);
		memcpy_neon(pEnc->v4l2_enc->mmap_virt_inp[index][0], pEnc->temp_data, pCfg->width * pCfg->height);
		memcpy_neon(pEnc->v4l2_enc->mmap_virt_inp[index][1], pEnc->temp_data + (pCfg->width * pCfg->height), (pCfg->width * pCfg->height)/2);
	}
	else {
		memcpy_neon(pEnc->v4l2_enc->mmap_virt_inp[index][0], ptr, pCfg->width * pCfg->height);
		memcpy_neon(pEnc->v4l2_enc->mmap_virt_inp[index][1], ptr + (pCfg->width * pCfg->height), (pCfg->width * pCfg->height)/2);
	}

}

void setHWFramerateRequest (enc_config_t *pCfg, unsigned int fps_n, unsigned int fps_d)
{
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	if (fps_d < 1)
	{
		LOG_ERR("Setting Denominator to 1, as it can't be < 1\n");
		fps_d = 1;
	}

	pEnc->fpsChangeRequest = FALSE;
	// Set the FPS on the Source
	if ((pCfg->src == CAM_NV12) || (pCfg->src == CAM_YUYV))
	{
		v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);
		if (SetupCameraFps(pV4l2Cam, fps_n, fps_d) == 0)
		{
			pCfg->fps = fps_n;
			pV4l2Cam->fps = fps_n;
			pEnc->inc_pts = (int)(1000000/pCfg->fps);
			pEnc->v4l2_enc->pkt_duration = (1000000000 / pCfg->fps);
		}
		else
		{
			LOG_ERR("Setting FPS on %s Failed:\n", ((pCfg->src == CAM_NV12) ? "CAM_NV12" : "CAM_YUYV"));
			return;
		}
	}
	else if ((pCfg->src == FILE_NV12) || (pCfg->src == FILE_YUYV))
	{
		pCfg->fps = fps_n;
		pEnc->fileReader.frame_duration_us = (1000*1000)/pCfg->fps;
		pEnc->inc_pts = (int)(1000000/pCfg->fps);
		pEnc->v4l2_enc->pkt_duration = (1000000000 / pCfg->fps);
	}
	else
	{
		LOG_ERR("Unsupported Source type for setting FPS\n");
		return;
	}

	// Set the FPS value on the Encoder
	setV4L2HWEncFramerate (pCfg, fps_n, fps_d);
}

static void *tx_feedLoop(void *data)
{
	enc_config_t *pCfg = (enc_config_t *)data;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	void *frame_ptr;
	int frame_size;
	int index,plane;
	int ret = -1;

	// Wait for start
	tsem_down(pEnc->feed_loop_start_sem);

	// Initialize timestamp for exact feed
	if ((pCfg->src == FILE_NV12) || (pCfg->src == FILE_YUYV))
		gettimeofday(&pEnc->fileReader.time_stamp, NULL);

	if(pCfg->src == ENC_EXT)
	{
		LOG_DEF("ENC_EXT data will be fed by MJPEG session\n");
		return NULL;
	}
	while(pEnc->is_running)
	{
		switch(pCfg->src)
		{
			case CAM_NV12:
			case CAM_YUYV:
				{
					int fb_index;
					int v4l2_index, v4l2_size;
					v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);
/*					if (pEnc->fpsChangeRequest == TRUE)
					{
						setHWFramerateRequest (pV4l2Cam, pEnc->fps_num, pEnc->fps_den);
						break;
					}*/

					ret = hw_v4l2enc_get_input_handle(pCfg, &v4l2_index);
					if (ret == 0)
					{
						//LOG_DEF("ret from hw_v4l2enc_get_input_handle %d\n", ret);
						ReadCameraInput(pV4l2Cam, &frame_ptr, &v4l2_size, &fb_index);
						ReadFrameFromNV12cam(pCfg, v4l2_index, frame_ptr, &v4l2_size);
						ret = hw_h264enc_encode (v4l2_size, v4l2_index, pEnc->v4l2_enc, pCfg);
						if (ret != 0 && ret != EAGAIN) {
							LOG_ERR("Encoding failed..\n");
							break;
						}
						else if (ret == EAGAIN) {
							LOG_ERR("Encode failed with EAGAIN\n");
							break;
						}

						EnqueCameraBuffer(pV4l2Cam, fb_index);
					}
					else {
						// Not expected to enter here
						LOG_ERR("[CAM_NV12/CAM_YUYV] Input not available\n");
						ReadCameraInput(pV4l2Cam, &frame_ptr, &v4l2_size, &fb_index);
						EnqueCameraBuffer(pV4l2Cam, fb_index);
						usleep (10000);
					}
				}
				break;
			case HDMI_UYVY:
			case HDMI_NV12:
				{
					int fb_index;
					int v4l2_index, v4l2_size;
					v4l2hrx_t *pV4l2HRx = &(pEnc->v4l2HRx);

					ret = hw_v4l2enc_get_input_handle(pCfg, &v4l2_index);
					if (ret == 0)
					{
						ReadHRxInput(pV4l2HRx, &frame_ptr, &v4l2_size, &fb_index);
						v4l2_size = pCfg->width * pCfg->height;
						ReadFrameFromNV12cam(pCfg, v4l2_index, frame_ptr, &v4l2_size);
						ret = hw_h264enc_encode (v4l2_size, v4l2_index, pEnc->v4l2_enc, pCfg);
						if (ret != 0 && ret != EAGAIN) {
							LOG_ERR("Encoding failed..\n");
							break;
						}
						else if (ret == EAGAIN) {
							LOG_ERR("Encode failed with EAGAIN\n");
							break;
						}

						EnqueHRxBuffer(pV4l2HRx, fb_index);
					}
					else {
						// Not expected to enter here
						LOG_ERR("[HDMI_NV12/HDMI_YUYV] Input not available\n");
						ReadHRxInput(pV4l2HRx, &frame_ptr, &v4l2_size, &fb_index);
						EnqueHRxBuffer(pV4l2HRx, fb_index);
						usleep (10000);
					}
				}
				break;
			case FILE_YUYV:
			case FILE_NV12:
				{
					int v4l2_index;
					if (pEnc->fpsChangeRequest == TRUE)
					{
						setHWFramerateRequest (pCfg, pEnc->fps_num, pEnc->fps_den);
						break;
					}
					ret = hw_v4l2enc_get_input_handle(pCfg, &v4l2_index);
					if (ret == 0)
					{
						if (pEnc->v4l2_enc->eos)
							break;

						filereader_t *pFileReader = &(pEnc->fileReader);
						if (!(pEnc->v4l2_enc->eos))
						{
							frame_size = pCfg->width * pCfg->height;
							ReadFrameFromFile(pFileReader, pEnc->v4l2_enc->mmap_virt_inp[v4l2_index][0], &frame_size);
							frame_size = pCfg->width * pCfg->height/2;
							ReadFrameFromFile(pFileReader, pEnc->v4l2_enc->mmap_virt_inp[v4l2_index][1], &frame_size);
						}
						ret = hw_h264enc_encode (frame_size, v4l2_index, pEnc->v4l2_enc, pCfg);
						if (ret != 0 && ret != EAGAIN) {
							LOG_DEF("Encoding failed..\n");
							break;
						}
						else if (ret == EAGAIN) {
							LOG_ERR("Encode failed with EAGAIN\n");
							break;
						}
						SleepForMaintainingFps(pFileReader);
					}
					else {
						LOG_ERR("[FILE_NV12/FILE_YUYV] Input not available\n");
						usleep (10000);
					}
				}
				break;
		}
	}
}

static int startCamDecEncGstPipeline_v4l2(enc_config_t *pCfg)
{
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	int ret = -1;

	if((pCfg->src == CAM_NV12) || (pCfg->src == CAM_YUYV))
	{
		v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);
		pV4l2Cam->width = pCfg->width;
		pV4l2Cam->height = pCfg->height;
		pV4l2Cam->fps = pCfg->fps;
		strncpy(pV4l2Cam->path, pCfg->path, MAX_FILE_PATH_STR);

		if (pCfg->src == CAM_YUYV)
			pV4l2Cam->cam_v4l2_fmt = V4L2_PIX_FMT_YUYV;
		else
			pV4l2Cam->cam_v4l2_fmt = V4L2_PIX_FMT_NV12;

		ret = SetupCamera(pV4l2Cam);
		if (ret < 0)
		{
			LOG_ERR("Camera connection failed \n");
			return -1;
		}
		LOG_SEQ("SetupCamera returned : %d\n", ret);

		ret = SetupNV12cam(pCfg);
		if(ret < 0)
		{
			LOG_ERR("NV12 Reader cannot be initialized\n");
			return -1;
		}
		LOG_SEQ("SetupNV12cam returned : %d\n", ret);
	}
	else if ((pCfg->src == FILE_NV12) || (pCfg->src == FILE_YUYV))
	{
		filereader_t *pFileReader = &(pEnc->fileReader);
		pFileReader->src = pCfg->src;
		pFileReader->width = pCfg->width;
		pFileReader->height = pCfg->height;
		pFileReader->fps = pCfg->fps;
		strncpy(pFileReader->path, pCfg->path, MAX_FILE_PATH_STR);

		ret = SetupFileReader(pFileReader);
		if(ret < 0)
		{
			LOG_ERR("File Reader cannot be initialized\n");
			return -1;
		}
		LOG_SEQ("File Reader initialized\n");
	}
	else
	{
		LOG_ERR("Not supported : %d \n",pCfg->src);
		return -1;
	}

	ret = SetupHWV4L2H264Enc(pCfg);
	if (ret != 0)
	{
		LOG_ERR("\n SetupHWV4L2H264Enc Failed\n");
		return;
	}
	if(strcmp(pCfg->ip, "0.0.0.0") != 0)
	{
		gst_tx_t *pGstTx = &pEnc->gstTx;

		pGstTx->port = pCfg->port;
		strncpy(pGstTx->ip, pCfg->ip, MAX_IP_ADDR_STR);


#ifdef LOCAL_LOOPBACK_MODE
		if(pCfg->instance_id == LOCAL_LOOPBACK_ENC_INST){
			LOG_DEF("Local loopback enabled for encoder instance_id(%d)\n", pCfg->instance_id);

		}else{
			ret = StartGstTxPipeline(pGstTx);
			if(ret < 0)
			{
				LOG_ERR("GST TX pipeline cannot be initialized\n");
				return -1;
			}
			LOG_SEQ("GST TX pipeline initialized\n");
		}
#else
		ret = StartGstTxPipeline(pGstTx);
		if(ret < 0)
		{
			LOG_ERR("GST TX pipeline cannot be initialized\n");
			return -1;
		}
		LOG_SEQ("GST TX pipeline initialized\n");
	}
#endif


	return 0;
}

static int startHRxDecEncGstPipeline_v4l2(enc_config_t *pCfg)
{
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	int ret = -1;

	if((pCfg->src == HDMI_UYVY) || (pCfg->src == HDMI_NV12))
	{
		v4l2hrx_t *pV4l2HRx = &(pEnc->v4l2HRx);
		pV4l2HRx->width = pCfg->width;
		pV4l2HRx->height = pCfg->height;
		pV4l2HRx->fps = pCfg->fps;
		strncpy(pV4l2HRx->path, pCfg->path, MAX_FILE_PATH_STR);

		if(pCfg->src == HDMI_UYVY)
			pV4l2HRx->hdmi_rx_fmt = V4L2_PIX_FMT_UYVY;
		else if(pCfg->src == HDMI_NV12)
			pV4l2HRx->hdmi_rx_fmt = V4L2_PIX_FMT_NV12;
		else
			return -1;
		ret = SetupHRx(pV4l2HRx);
		if (ret < 0)
		{
			LOG_ERR("HDMI RX connection failed \n");
			return -1;
		}
		LOG_SEQ("SetupHRx returned : %d\n", ret);

		ret = SetupNV12cam(pCfg);
		if(ret < 0)
		{
			LOG_ERR("NV12 Reader cannot be initialized\n");
			return -1;
		}
		LOG_SEQ("SetupNV12cam returned : %d\n", ret);
	}
	else
	{
		LOG_ERR("Not supported : %d \n",pCfg->src);
		return -1;
	}

	ret = SetupHWV4L2H264Enc(pCfg);
	if (ret != 0)
	{
		LOG_ERR("\n SetupHWV4L2H264Enc Failed\n");
		return;
	}

	ret = StreamOnHRx(&(pEnc->v4l2HRx));
	if (ret < 0)
	{
		LOG_ERR("HDMI RX Stream On failed \n");
		return -1;
	}
	LOG_SEQ("Stream On returned : %d\n", ret);


	gst_tx_t *pGstTx = &pEnc->gstTx;

	pGstTx->port = pCfg->port;
	strncpy(pGstTx->ip, pCfg->ip, MAX_IP_ADDR_STR);

	ret = StartGstTxPipeline(pGstTx);
	if(ret < 0)
	{
		LOG_ERR("GST TX pipeline cannot be initialized\n");
		return -1;
	}
	LOG_SEQ("GST TX pipeline initialized\n");

	return 0;
}

void startHWV4L2Encoder(sess_config_t *sess_cfg)
{
	int ret;
	LOG_FUNC("%s: \n", __func__);
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;

	pCfg->priv = malloc(sizeof(priv_hw_v4l2_encoder_t));
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;

	memset(pEnc, 0, sizeof(priv_hw_v4l2_encoder_t));

	pEnc->sess_cfg = sess_cfg;
	pEnc->feed_loop_start_sem = (tsem_t *)malloc(sizeof(tsem_t));
	pEnc->dqbuf_enc_start_sem = (tsem_t *)malloc(sizeof(tsem_t));
	pEnc->h264EncoderSeQueueSem = (tsem_t *)malloc(sizeof(tsem_t));
	
	#if  0
	pEnc->v4l2_enc = (struct v4l2_enc *)malloc(sizeof(struct v4l2_enc));
	memset(pEnc->v4l2_enc, 0, sizeof(struct v4l2_enc));
	#endif
	pEnc->v4l2_enc = (struct v4l2_hwenc *)malloc(sizeof(struct v4l2_hwenc));
	memset(pEnc->v4l2_enc, 0, sizeof(struct v4l2_hwenc));

	tsem_init(pEnc->feed_loop_start_sem, 0);
	tsem_init(pEnc->dqbuf_enc_start_sem, 0);
	tsem_init(pEnc->h264EncoderSeQueueSem, 0);

	pthread_mutex_init(&pEnc->qt_rendering_buffer_mutex, NULL);
	pEnc->qt_rendering_buffer_index = -1;

	// Calculate timestamp based on fps
	pEnc->inc_pts = (int)(1000000/pCfg->fps);
	pEnc->pts += pEnc->inc_pts;

	if((pCfg->src == HDMI_UYVY) || (pCfg->src == HDMI_NV12))
	{
		ret = startHRxDecEncGstPipeline_v4l2(pCfg);
		if (ret < 0)
			return;
	}
	else
	{
		ret = startCamDecEncGstPipeline_v4l2(pCfg);
		if (ret < 0)
			return;
	}

	pthread_create(&pEnc->feed_loop_id, NULL, tx_feedLoop, (void *)pCfg);

	pEnc->total_frames = 0;
	pEnc->total_bitrate = 0;
	pEnc->is_running = 1;
	tsem_up(pEnc->feed_loop_start_sem);
	tsem_up(pEnc->dqbuf_enc_start_sem);
}

void stopHWV4L2Encoder(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);
	int ret = -1;
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;

	pEnc->is_running = 0;

	// Wait for feedloop to exit
	pthread_join(pEnc->feed_loop_id, NULL);

	if(pCfg->src == ENC_EXT)
	{
		LOG_ERR("No need to %s for ENC_EXT \n",__func__);
		return;
	}
	ret = hw_v4l2h264enc_stop(pCfg);
	if (ret != 0)
	{
		LOG_ERR("\n hw_v4l2h264enc_stop Failed\n");
		return;
	}
	LOG_DEF("h264enc stop SUCCESS!! \n");
}

void waitForHWV4L2EncoderShutdownAny(enc_config_t *pCfg)
{
	LOG_DEF("%s: \n", __func__);
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;

	if (pCfg->src == CAM_NV12 || pCfg->src == CAM_YUYV)
	{
		v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);
		ShutdownCamera(pV4l2Cam);
		free(pEnc->temp_data);
	}
	else if (pCfg->src == HDMI_NV12 || pCfg->src == HDMI_UYVY)
	{
		v4l2hrx_t *pV4l2HRx = &(pEnc->v4l2HRx);
		ShutdownHRx(pV4l2HRx);
		free(pEnc->temp_data);
	}
	else if (pCfg->src == FILE_NV12 || pCfg->src == FILE_YUYV)
	{
		filereader_t *pFileReader = &(pEnc->fileReader);
		ShutdownFileReader(pFileReader);
	}
	else {
		LOG_ERR("\n Not supported\n");
	}

#ifdef LOCAL_LOOPBACK_MODE
	if(pCfg->instance_id != LOCAL_LOOPBACK_ENC_INST)
		TerminateGstTxPipeline(&pEnc->gstTx);
#else
	TerminateGstTxPipeline(&pEnc->gstTx);
#endif //LOCAL_LOOPBACK_MODE

	if (pEnc->v4l2_enc != NULL)
	{
		free (pEnc->v4l2_enc);
		pEnc->v4l2_enc = NULL;
	}
	if (pEnc->feed_loop_start_sem != NULL)
	{
		//LOG_ERR("free pEnc->feed_loop_start_sem %p\n", pEnc->feed_loop_start_sem);
		free(pEnc->feed_loop_start_sem);
		pEnc->feed_loop_start_sem = NULL;
	}

	if (pEnc->dqbuf_enc_start_sem != NULL)
		free(pEnc->dqbuf_enc_start_sem);
	if(pEnc->h264EncoderSeQueueSem != NULL)
		free(pEnc->h264EncoderSeQueueSem);

	if (pCfg->priv != NULL)
	{
		//LOG_ERR("free pCfg->priv %p\n", pCfg->priv);
		free(pCfg->priv);
		pCfg->priv = NULL;
	}
}


void waitForHWV4L2EncoderShutdown(sess_config_t *sess_cfg)
{
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;

	if(pCfg->src == ENC_EXT)
	{
		LOG_DEF("CAM_EXT, shutdown will be done by main session\n");
		return;
	}

	waitForHWV4L2EncoderShutdownAny(pCfg);
}

void setHWBitrate (sess_config_t *sess_cfg, unsigned int rate)
{
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	int max_bitrate;
	if ((pCfg->src == CAM_YUYV) || (pCfg->src == FILE_YUYV))
		max_bitrate = (pCfg->width * pCfg->height * pCfg->fps * 2);
	else
		max_bitrate = ((pCfg->width * pCfg->height * pCfg->fps * 3) >> 1);
	max_bitrate >>= 1; // consider upper limit as half of max bitrate
	if (rate > max_bitrate)
	{
		LOG_ERR("Bitrate exceeding (0.5 * MaxBitrate)\n\tSetting bitrate to (0.5 * MaxBitrate)\n");
		rate = max_bitrate;
	}
	setV4L2HWEncBitrate (pCfg, rate);
}

void setHWFramerate (sess_config_t *sess_cfg, unsigned int fps_n, unsigned int fps_d)
{
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	pEnc->fpsChangeRequest = TRUE;
	pEnc->fps_num = fps_n;
	pEnc->fps_den = fps_d;
}

void setHWIDRFrameRequest (sess_config_t *sess_cfg)
{
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	setV4L2HWEncIDRFrameRequest (pCfg);
}

