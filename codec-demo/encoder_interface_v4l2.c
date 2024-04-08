#include "encoder_interface_v4l2.h"
#include "v4l2camera.h"

int SetupNV12cam(enc_config_t *pCfg)
{
	return 0;
}

void ReadFrameFromNV12cam(enc_config_t *pCfg, int index, void* ptr, int *size)
{
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	//LOG_DEF("ReadFrameFromNV12cam: pCfg: 0x%x, index: %d, ptr: 0x%x, size: %d\n", pCfg, index, ptr, *size);

	memcpy_neon(pEnc->v4l2_enc->mmap_virt_inp[index], ptr, *size);
}

void setFramerateRequest (enc_config_t *pCfg, unsigned int fps_n, unsigned int fps_d)
{
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
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
	setV4L2EncFramerate (pCfg, fps_n, fps_d);
}

void *tx_feedLoop(void *data)
{
	enc_config_t *pCfg = (enc_config_t *)data;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	void *frame_ptr;
	int frame_size;
	int index;
	int ret = -1;

	// Wait for start
	tsem_down(pEnc->feed_loop_start_sem);

	// Start HDMI RX monitor thread
		if(pCfg->src == HDMI_UYVY || pCfg->src == HDMI_EXT)
		tsem_up(pEnc->hdmi_rx_start_sem);

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
			case CAM_MJPEG:
				{
#if 0
					ReadCameraInput(pCfg, &frame_ptr, &frame_size, &index);
					FeedInputToMjpegDec(pCfg, frame_ptr, frame_size);
					// Feed the buffer back
					EnqueCameraBuffer(pCfg, index);
#endif
				}
				break;

			case CAM_NV12:
			case CAM_YUYV:
				{
					int fb_index;
					int v4l2_index, v4l2_size;
					v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);
					if (pEnc->fpsChangeRequest == TRUE)
					{
						setFramerateRequest (pV4l2Cam, pEnc->fps_num, pEnc->fps_den);
						break;
					}

					ret = v4l2enc_get_input_handle(pCfg, &v4l2_index);
					if (ret == 0)
					{
						//LOG_DEF("ret from v4l2enc_get_input_handle %d\n", ret);
						ReadCameraInput(pV4l2Cam, &frame_ptr, &v4l2_size, &fb_index);
						ReadFrameFromNV12cam(pCfg, v4l2_index, frame_ptr, &v4l2_size);
						ret = v4l2h264enc_encode (v4l2_size, v4l2_index, pEnc->v4l2_enc, pCfg);
						if (ret != 0 && ret != ERROR_EAGAIN) {
							LOG_ERR("Encoding failed..\n");
							break;
						}
						else if (ret == ERROR_EAGAIN) {
							LOG_ERR("Encode failed with ERROR_EAGAIN\n");
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

			//case HDMI_NV12:
			case HDMI_UYVY:
				{
					if (pEnc->hdmi_rx_streaming)
					{
						ret = ReadHdmiRxFrame(pCfg, &index, &frame_size);
						if (ret == 0)
						{
							if(index < pEnc->hdmi_rx_out_buf_count)
							{
								//FeedInputToH264EncFromHdmiRx(pCfg, index, frame_size);
							}
						}
						else if (ret == ERROR_EAGAIN)
						{
							LOG_SEQ("HDMI Read EAGAIN\n");
						}
						else
						{
							LOG_SEQ("HDMI Read returns error: %d\n", ret);
						}
					}
					else
					{
						usleep(100000);
					}
					break;
				}
			case HDMI_EXT:
					{
						if (pEnc->hdmi_rx_streaming){
							ret = ReadHdmiRxExtFrame(pCfg, &index, &frame_size);
							if(ret = ERROR_EAGAIN){
								LOG_FUNC("HDMI Read EAGAIN\n");
							}else{
								LOG_FUNC("HDMI Read returns error: %d\n", ret);
							}
						}else{
							usleep(100000);
							LOG_FUNC("Fn(%s): pEnc->hdmi_rx_streaming(%d)\n", __func__, pEnc->hdmi_rx_streaming);
						}
					}
					break;

			case FILE_YUYV:
			case FILE_NV12:
				{
					int v4l2_index;
					if (pEnc->fpsChangeRequest == TRUE)
					{
						setFramerateRequest (pCfg, pEnc->fps_num, pEnc->fps_den);
						break;
					}
					ret = v4l2enc_get_input_handle(pCfg, &v4l2_index);
					//LOG_DEF("ret from v4l2enc_get_input_handle %d\n", ret);

					if (ret == 0)
					{
						if (pEnc->v4l2_enc->eos)
							break;
						if(pCfg->src == FILE_YUYV)
							frame_size = pCfg->width* pCfg->height*2;
						else
							frame_size = (pCfg->width* pCfg->height) * 3/2;

						filereader_t *pFileReader = &(pEnc->fileReader);
						if (!(pEnc->v4l2_enc->eos))
						{
							ReadFrameFromFile(pFileReader, pEnc->v4l2_enc->mmap_virt_inp[v4l2_index], &frame_size);
						}
						ret = v4l2h264enc_encode (frame_size, v4l2_index, pEnc->v4l2_enc, pCfg);
						if (ret != 0 && ret != ERROR_EAGAIN) {
							LOG_DEF("Encoding failed..\n");
							break;
						}
						else if (ret == ERROR_EAGAIN) {
							LOG_ERR("Encode failed with ERROR_EAGAIN\n");
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

static int startCamDecEncGstPipeline_v4l2(enc_config_t *pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
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
		LOG_DEF("SetupCamera returned : %d\n", ret);

		ret = SetupNV12cam(pCfg);
		if(ret < 0)
		{
			LOG_ERR("NV12 Reader cannot be initialized\n");
			return -1;
		}
		LOG_DEF("SetupNV12cam returned : %d\n", ret);
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
		LOG_DEF("File Reader initialized\n");
	}
	else if (pCfg->src == HDMI_UYVY)
	{
		pEnc->hdmi_rx_v4l2_fmt = V4L2_PIX_FMT_UYVY;

		ret = SetupHdmiRx(pCfg);
		if(ret < 0)
		{
			LOG_ERR("HDMI RX cannot be initialized\n");
			return -1;
		}
		LOG_DEF("HDMI RX initialized\n");
	}
	else if(pCfg->src == HDMI_EXT)
	{
		pEnc->hdmi_rx_v4l2_fmt = V4L2_PIX_FMT_NV12;
		ret = SetupHdmiRxExt(pCfg);
		if(ret < 0){
			LOG_ERR("Error: HDMI-RX-EXT Init failed\n");
			return -1;
		}
	}

	else if (pCfg->src == ENC_EXT)
	{
		setupEncEXt(pCfg);
	}

	ret = SetupV4L2H264Enc(pCfg);
	if (ret != 0)
	{
		LOG_ERR("\n SetupV4L2H264Enc Failed\n");
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
			LOG_DEF("GST TX pipeline initialized\n");
		}
#else
		ret = StartGstTxPipeline(pGstTx);
		if(ret < 0)
		{
			LOG_ERR("GST TX pipeline cannot be initialized\n");
			return -1;
		}
		LOG_DEF("GST TX pipeline initialized\n");
	}
#endif


	return 0;
}

void startV4L2Encoder(sess_config_t *sess_cfg)
{
	int ret;
	LOG_DEF("%s: \n", __func__);
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;

	pCfg->priv = malloc(sizeof(priv_encoder_t));
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	memset(pEnc, 0, sizeof(priv_encoder_t));

	pEnc->sess_cfg = sess_cfg;
	pEnc->feed_loop_start_sem = (tsem_t *)malloc(sizeof(tsem_t));
	pEnc->dqbuf_enc_start_sem = (tsem_t *)malloc(sizeof(tsem_t));
	pEnc->h264EncoderSeQueueSem = (tsem_t *)malloc(sizeof(tsem_t));
	pEnc->v4l2_enc = (struct v4l2_enc *)malloc(sizeof(struct v4l2_enc));
	memset(pEnc->v4l2_enc, 0, sizeof(struct v4l2_enc));

	tsem_init(pEnc->feed_loop_start_sem, 0);
	tsem_init(pEnc->dqbuf_enc_start_sem, 0);
	tsem_init(pEnc->h264EncoderSeQueueSem, 0);

	pthread_mutex_init(&pEnc->qt_rendering_buffer_mutex, NULL);
	pEnc->qt_rendering_buffer_index = -1;

	// Calculate timestamp based on fps
	pEnc->inc_pts = (int)(1000000/pCfg->fps);
	pEnc->pts += pEnc->inc_pts;

	ret = startCamDecEncGstPipeline_v4l2(pCfg);
	if (ret < 0)
		return;

	pthread_create(&pEnc->feed_loop_id, NULL, tx_feedLoop, (void *)pCfg);

	pEnc->total_frames = 0;
	pEnc->total_bitrate = 0;
	pEnc->is_running = 1;
	tsem_up(pEnc->feed_loop_start_sem);
	tsem_up(pEnc->dqbuf_enc_start_sem);
}

void stopV4L2Encoder(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);
	int ret = -1;
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	pEnc->is_running = 0;

	// Wait for feedloop to exit
	pthread_join(pEnc->feed_loop_id, NULL);

	if(pCfg->src == ENC_EXT)
	{
		LOG_ERR("No need to %s for ENC_EXT \n",__func__);
		return;
	}
	if(pCfg->src == HDMI_UYVY)
		StopHdmiRX(pCfg);

	ret = v4l2h264enc_stop(pCfg);
	if (ret != 0)
	{
		LOG_ERR("\n v4l2h264enc_stop Failed\n");
		return;
	}
	LOG_DEF("h264enc stop SUCCESS!! \n");
}

void waitForV4L2EncoderShutdownAny(enc_config_t *pCfg)
{
	LOG_DEF("%s: \n", __func__);
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	if (pCfg->src == HDMI_UYVY)
	{
		ShutdownHdmiRx(pCfg);
	}
	else if (pCfg->src == CAM_NV12 || pCfg->src == CAM_YUYV)
	{
		v4l2cam_t *pV4l2Cam = &(pEnc->v4l2Cam);
		ShutdownCamera(pV4l2Cam);
	}
	else if (pCfg->src == FILE_NV12 || pCfg->src == FILE_YUYV)
	{
		filereader_t *pFileReader = &(pEnc->fileReader);
		ShutdownFileReader(pFileReader);
	}

#ifdef LOCAL_LOOPBACK_MODE
	if(pCfg->instance_id != LOCAL_LOOPBACK_ENC_INST)
		TerminateGstTxPipeline(&pEnc->gstTx);
#else
	TerminateGstTxPipeline(&pEnc->gstTx);
#endif //LOCAL_LOOPBACK_MODE

	if (pEnc->v4l2_enc != NULL)
	{
		if(pEnc->v4l2_enc->simulcast != NULL)
		{
			free(pEnc->v4l2_enc->simulcast);
		}
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


void waitForV4L2EncoderShutdown(sess_config_t *sess_cfg)
{
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	if(pCfg->src == ENC_EXT)
	{
		LOG_DEF("CAM_EXT, shutdown will be done by main session\n");
		return;
	}

	waitForV4L2EncoderShutdownAny(pCfg);

	if(pCfg->simcast_en)
	{
		for(int i=0; i< (((codec_config_t *)(pCfg->codec_config))->num_ext_enc_sessions); i++)
		{
			enc_config_t *ext_enc_config = (((codec_config_t *)(pCfg->codec_config))->ext_enc_sess_cfg[i]);
			waitForV4L2EncoderShutdownAny(ext_enc_config);
		}
	}
}

void setBitrate (sess_config_t *sess_cfg, unsigned int rate)
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
	setV4L2EncBitrate (pCfg, rate);
}

void setFramerate (sess_config_t *sess_cfg, unsigned int fps_n, unsigned int fps_d)
{
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	pEnc->fpsChangeRequest = TRUE;
	pEnc->fps_num = fps_n;
	pEnc->fps_den = fps_d;
}

void setIDRFrameRequest (sess_config_t *sess_cfg)
{
	enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
	setV4L2EncIDRFrameRequest (pCfg);
}

