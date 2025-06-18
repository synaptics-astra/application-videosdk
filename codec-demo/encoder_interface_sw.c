// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "encoder_interface_sw.h"

static int SetupNV12cam(enc_config_t *pCfg)
{
	return 0;
}
static YUYVtoNV12Convertor(uint8_t* dst,uint8_t* src,int width, int height)
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
			*pyFrame++ = *src++;
			if (evenRow)
			{
				uint16_t calc = *src;
				calc += *(src + uvOffset);
				calc /= 2;
				*puvFrame++ = *src;
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
		}
	}
}
static void ReadFrameFromNV12cam(enc_config_t *pCfg, int index, void* ptr, int *size)
{
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;

	//LOG_DEF("ReadFrameFromNV12cam: pCfg: 0x%x, index: %d, ptr: 0x%x, size: %d\n", pCfg, index, ptr, *size);
	if(pCfg->src == CAM_YUYV)
		YUYVtoNV12Convertor((uint8_t*)pEnc->pH264EncInSWBuffer[index]->vAddr,(uint8_t* )ptr,pCfg->width,pCfg->height);
	else
		memcpy_neon(pEnc->pH264EncInSWBuffer[index]->vAddr, ptr, *size);
}

static void *tx_feedLoop(void *data)
{
	enc_config_t *pCfg = (enc_config_t *)data;
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;
	ffmpeg_enc *ffmpegEnc = (ffmpeg_enc *)pEnc->ffmpeg_enc;

	void *frame_ptr;
	int frame_size;
	int index;
	int ret = -1;

	// Wait for start
	tsem_down(pEnc->feed_loop_start_sem);

	// Initialize timestamp for exact feed
	if ((pCfg->src == FILE_NV12) || (pCfg->src == FILE_YUYV))
		gettimeofday(&pEnc->fileReader.time_stamp, NULL);

	if(pCfg->src == ENC_EXT)
	{
		LOG_DEF("ENC_EXT data is not supported \n");
		return NULL;
	}
	while(pEnc->is_running)
	{
		switch(pCfg->src)
		{
			case CAM_H264:
				{
					int fb_index, header = 0;
					int v4l2_index, v4l2_size;
					v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);

					ret = swenc_get_input_handle(pCfg, &v4l2_index);
					if (ret == 0)
					{
						//LOG_DEF("ret from v4l2enc_get_input_handle %d\n", ret);
						ReadCameraInput(pV4l2Cam, &frame_ptr, &v4l2_size, &fb_index);

						/*enque the encoded data here for local loopback only*/
						codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
						queue_element_t* tmpBuf = ( queue_element_t*) malloc(sizeof(queue_element_t));
						if(tmpBuf){
							tmpBuf->nFilledLen = v4l2_size;
							tmpBuf->pBuffer = (uint8_t *)malloc(v4l2_size);
							memcpy(tmpBuf->pBuffer, frame_ptr, v4l2_size);
							int eError = queue(codec_config->sharedH264EncoderQueue,tmpBuf);
							tsem_up(codec_config->sharedH264EncoderQueueSem);
						}


						pEnc->total_bitrate += v4l2_size;
						pEnc->total_frames++;
try_again:
						if(strcmp(pCfg->ip, "0.0.0.0") != 0)
						{
							gst_tx_t *pGstTx = &(pEnc->gstTx);
							ret = FeedGstTx(pGstTx, frame_ptr, v4l2_size , ffmpegEnc->frame->pts);
							if (ret != 0 && (!header))
							{
								usleep(5*1000);
								goto try_again;
							}
						}
						EnqueCameraBuffer(pV4l2Cam, fb_index);
					}
					else {
						// Not expected to enter here
						LOG_ERR("Input not available\n");
						ReadCameraInput(pV4l2Cam, &frame_ptr, &v4l2_size, &fb_index);
						EnqueCameraBuffer(pV4l2Cam, fb_index);
						usleep (10000);
					}
				}
				break;
			case CAM_NV12:
			case CAM_YUYV:
				{
					int fb_index;
					int v4l2_index, v4l2_size;
					v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);

					ret = swenc_get_input_handle(pCfg, &v4l2_index);
					if (ret == 0)
					{
						//LOG_DEF("ret from v4l2enc_get_input_handle %d\n", ret);
						ReadCameraInput(pV4l2Cam, &frame_ptr, &v4l2_size, &fb_index);
						ReadFrameFromNV12cam(pCfg, v4l2_index, frame_ptr, &v4l2_size);
						ret = swh264enc_encode (v4l2_size, v4l2_index,ffmpegEnc, pCfg);
						if (ret != 0 ) {
							LOG_ERR("Encoding failed..\n");
							break;
						}
						EnqueCameraBuffer(pV4l2Cam, fb_index);
					}
					else {
						// Not expected to enter here
						LOG_ERR("Input not available\n");
						ReadCameraInput(pV4l2Cam, &frame_ptr, &v4l2_size, &fb_index);
						EnqueCameraBuffer(pV4l2Cam, fb_index);
						usleep (10000);
					}
				}
				break;
			case FILE_YUYV:
			case FILE_NV12:
				{
					int v4l2_index;
					ret = swenc_get_input_handle(pCfg, &v4l2_index);
					//LOG_DEF("ret from v4l2enc_get_input_handle %d\n", ret);

					if (ret == 0)
					{
						if (ffmpegEnc->eos)
							break;

						filereader_t *pFileReader = &(pEnc->fileReader);
						if (!(ffmpegEnc->eos))
						{
							if(pCfg->src == FILE_NV12)
							{
								frame_size = pCfg->height * pCfg->width * 1.5;
								ReadFrameFromFile(pFileReader,pEnc->pH264EncInSWBuffer[v4l2_index]->vAddr, &frame_size);
							}
							else
							{
								frame_size = pCfg->height * pCfg->width * 2;
								ReadFrameFromFile(pFileReader,pFileReader->readPtr, &frame_size);
								YUYVtoNV12Convertor((uint8_t*)pEnc->pH264EncInSWBuffer[v4l2_index]->vAddr,(uint8_t* )pFileReader->readPtr,pCfg->width,pCfg->height);
							}
						}
						ret = swh264enc_encode (frame_size, v4l2_index, ffmpegEnc, pCfg);
						if (ret != 0 ) {
							LOG_DEF("Encoding failed..\n");
							break;
						}
						SleepForMaintainingFps(pFileReader);
					}
					else {
						LOG_ERR("Input not available\n");
						usleep (10000);
					}
				}
				break;
		}
	}
}

static int startCamDecEncGstPipeline_sw(enc_config_t *pCfg)
{
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;
	int ret = -1;

	if((pCfg->src == CAM_NV12) || (pCfg->src == CAM_YUYV) || (pCfg->src == CAM_H264))
	{
		v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);
		pV4l2Cam->width = pCfg->width;
		pV4l2Cam->height = pCfg->height;
		pV4l2Cam->fps = pCfg->fps;
		strncpy(pV4l2Cam->path, pCfg->path, MAX_FILE_PATH_STR);

		if (pCfg->src == CAM_H264)
			pV4l2Cam->cam_v4l2_fmt = V4L2_PIX_FMT_H264;
		else if (pCfg->src == CAM_YUYV)
			pV4l2Cam->cam_v4l2_fmt = V4L2_PIX_FMT_YUYV;
		else
			pV4l2Cam->cam_v4l2_fmt = V4L2_PIX_FMT_NV12;

		ret = SetupCamera(pV4l2Cam);
		if (ret < 0)
		{
			LOG_ERR("Camera connection failed \n");
			return -1;
		}
		LOG_DEF("SetupCamera returned : %d\n", ret);

		if(pCfg->src != CAM_H264)
		{
			ret = SetupNV12cam(pCfg);
			if(ret < 0)
			{
				LOG_ERR("NV12 Reader cannot be initialized\n");
				return -1;
			}
			LOG_DEF("SetupNV12cam returned : %d\n", ret);
		}
	}
	else if ((pCfg->src == FILE_NV12) || (pCfg->src == FILE_YUYV))
	{
		filereader_t *pFileReader = &(pEnc->fileReader);
		pFileReader->src = pCfg->src;
		pFileReader->width = pCfg->width;
		pFileReader->height = pCfg->height;
		pFileReader->fps = pCfg->fps;
		pFileReader->readPtr = (char *)malloc(pCfg->width*pCfg->height*2);
		strncpy(pFileReader->path, pCfg->path, MAX_FILE_PATH_STR);

		ret = SetupFileReader(pFileReader);
		if(ret < 0)
		{
			LOG_ERR("File Reader cannot be initialized\n");
			return -1;
		}
		LOG_DEF("File Reader initialized\n");
	}
	else if (pCfg->src == HDMI_UYVY)
	{
		LOG_ERR("HDMI RX cannot be initialized\n");
		return -1;
	}
	else if(pCfg->src == HDMI_EXT)
	{
		LOG_ERR("Error: HDMI-RX-EXT Init failed\n");
		return -1;
	}

	else if (pCfg->src == ENC_EXT)
	{
		LOG_ERR("%s :%d \n",__func__,__LINE__);
		return -1;
	}
	else
	{
		LOG_ERR("%s :%d \n",__func__,__LINE__);
	}

	ret = SetupSWH264Enc(pCfg);
	if (ret != 0)
	{
		LOG_ERR("\n SetupSWH264Enc Failed\n");
		return -1;
	}

	if(strcmp(pCfg->ip, "0.0.0.0") != 0)
	{
		gst_tx_t *pGstTx = &pEnc->gstTx;

		pGstTx->port = pCfg->port;
		strncpy(pGstTx->ip, pCfg->ip, MAX_IP_ADDR_STR);

		ret = StartGstTxPipeline(pGstTx);
		if(ret < 0)
		{
			LOG_ERR("GST TX pipeline cannot be initialized\n");
			return -1;
		}
		LOG_DEF("GST TX pipeline initialized\n");
	}
	return 0;
}

void startSWEncoder(sess_config_t *sess_cfg)
{
	int ret;
	LOG_DEF("%s: \n", __func__);
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;

	pCfg->priv = malloc(sizeof(priv_sw_encoder_t));
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;

	memset(pEnc, 0, sizeof(priv_sw_encoder_t));

	pEnc->sess_cfg = sess_cfg;
	pEnc->feed_loop_start_sem = (tsem_t *)malloc(sizeof(tsem_t));

	pEnc->ffmpeg_enc = (struct ffmpeg_enc *)malloc(sizeof(ffmpeg_enc));
	memset(pEnc->ffmpeg_enc, 0, sizeof (ffmpeg_enc));

	tsem_init(pEnc->feed_loop_start_sem, 0);

	pthread_mutex_init(&pEnc->qt_rendering_buffer_mutex, NULL);
	pEnc->qt_rendering_buffer_index = -1;

	// Calculate timestamp based on fps
	pEnc->inc_pts = (int)(1000000/pCfg->fps);
	pEnc->pts += pEnc->inc_pts;

	ret = startCamDecEncGstPipeline_sw(pCfg);
	if (ret < 0)
		return;

	pthread_create(&pEnc->feed_loop_id, NULL, tx_feedLoop, (void *)pCfg);

	pEnc->total_frames = 0;
	pEnc->total_bitrate = 0;
	pEnc->is_running = 1;
	tsem_up(pEnc->feed_loop_start_sem);
}

void stopSWEncoder(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);
	int ret = -1;
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;

	pEnc->is_running = 0;

	// Wait for feedloop to exit
	pthread_join(pEnc->feed_loop_id, NULL);

	if(pCfg->src == ENC_EXT)
	{
		LOG_ERR("Not supported %s for ENC_EXT \n",__func__);
		return;
	}
	if(pCfg->src == HDMI_UYVY)
	{
		LOG_ERR("Not supported %s for ENC_EXT \n",__func__);
		return;
	}

	ret = swh264enc_stop(pCfg);
	if (ret != 0)
	{
		LOG_ERR("\n v4l2h264enc_stop Failed\n");
		return;
	}
	LOG_DEF("h264enc stop SUCCESS!! \n");
}

void waitForSWEncoderShutdownAny(enc_config_t *pCfg)
{
	LOG_DEF("%s: \n", __func__);
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;

	if (pCfg->src == HDMI_UYVY)
	{
		LOG_ERR("Not supported %s for HDMI_UYVY \n",__func__);
	}
	else if (pCfg->src == CAM_NV12 || pCfg->src == CAM_YUYV || pCfg->src == CAM_H264)
	{
		v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);
		ShutdownCamera(pV4l2Cam);
	}
	else if (pCfg->src == FILE_NV12 || pCfg->src == FILE_YUYV)
	{
		filereader_t *pFileReader = &(pEnc->fileReader);
		ShutdownFileReader(pFileReader);
	}

	TerminateGstTxPipeline((gst_tx_t *)&pEnc->gstTx);

	if (pEnc->ffmpeg_enc != NULL)
	{
		free (pEnc->ffmpeg_enc);
		pEnc->ffmpeg_enc = NULL;
	}
	if (pEnc->feed_loop_start_sem != NULL)
	{
		//LOG_ERR("free pEnc->feed_loop_start_sem %p\n", pEnc->feed_loop_start_sem);
		free(pEnc->feed_loop_start_sem);
		pEnc->feed_loop_start_sem = NULL;
	}
	if (pCfg->priv != NULL)
	{
		//LOG_ERR("free pCfg->priv %p\n", pCfg->priv);
		free(pCfg->priv);
		pCfg->priv = NULL;
	}
}


void waitForSWEncoderShutdown(sess_config_t *sess_cfg)
{
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;

	if(pCfg->src == ENC_EXT)
	{
		LOG_DEF("Not Supported \n");
		return;
	}

	waitForSWEncoderShutdownAny(pCfg);
}
