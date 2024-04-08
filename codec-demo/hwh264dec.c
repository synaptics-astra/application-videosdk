#include "hwh264dec.h"

//#define DUMP_DECODED_DATA_HW 1
#if DUMP_DECODED_DATA_HW
void dumpData_hw(char *data, int data_size)
{
	static FILE *fp = NULL;
	static int iCnt;

	if(!iCnt){
		fp = fopen("hw_decoder_output.raw","wb");
	} else {
		fp = fopen("hw_decoder_output.raw","ab");
	}
	fwrite(data, 1, data_size, fp);
	fsync(fileno(fp));
	fclose(fp);
	iCnt++;
}
#endif

static void SleepForMaintainingFpsAtDecoder(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	struct timeval end;
	int ret = 0;

	gettimeofday(&end, NULL);

	unsigned long seconds = (end.tv_sec - pDec->time_stamp.tv_sec);
	unsigned long micros = ((seconds * 1000000) + end.tv_usec) - (pDec->time_stamp.tv_usec);
	long usec_sleep = pDec->frame_duration_us - micros;

	/* TODO: Fix it properly in gstreamer */
	// Sleep 2ms less than required to catch up any extra data feed by gst
	usec_sleep -= 2000;
	if (usec_sleep > 0)
		usleep(usec_sleep);

	gettimeofday(&pDec->time_stamp, NULL);
}

void *hw_v4l2_event_watcher(void *data)
{
	dec_config_t *pCfg = (dec_config_t *) data;
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;
	struct v4l2_event dqevent;
	struct v4l2_decoder_cmd v4l2DecCmd;

	while(pDec->bH264DecoderRecievedEOS != true)
	{
		memset(&dqevent, 0, sizeof(struct v4l2_event));
		//LOG_DEF("%s: %d VIDIOC_DQEVENT: %d\n", __func__,__LINE__, dqevent.type);
		ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_DQEVENT, &dqevent);
		if (ret < 0)
		{
			usleep(10000);
			continue;
		}
		LOG_SEQ("%s: %d VIDIOC_DQEVENT: %d\n", __func__,__LINE__, dqevent.type);

		switch (dqevent.type) {
			case V4L2_EVENT_EOS:
				LOG_SEQ("%s: %d EOS %d\n", __func__,__LINE__, dqevent.u.src_change.changes);
				break;

			case V4L2_EVENT_SOURCE_CHANGE:
				LOG_SEQ("%s: %d src_change %d\n", __func__,__LINE__, dqevent.u.src_change.changes);
				if (dqevent.u.src_change.changes == V4L2_EVENT_SRC_CH_RESOLUTION) {
					pDec->src_res_change++;
				}
				if (pDec->src_res_change > 1) {
					// get format
					struct v4l2_format v4l2dec_g_fmt;

					memset (&v4l2dec_g_fmt, 0, sizeof v4l2dec_g_fmt);
					v4l2dec_g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
					v4l2dec_g_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M; // NM12 format

					ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_G_FMT, &v4l2dec_g_fmt);
					if (ret < 0)
					{
						LOG_ERR ("%s: Capture VIDIOC_G_FMT failed\n", __func__);
						usleep(10000);
						continue;
					}
					LOG_PARAM("VIDIOC_G_FMT: (%dx%d), OrigRes(%dx%d)\n", v4l2dec_g_fmt.fmt.pix_mp.width, v4l2dec_g_fmt.fmt.pix_mp.height, pCfg->width, pCfg->height);
					if ((pCfg->width != v4l2dec_g_fmt.fmt.pix_mp.width) || (pCfg->height != v4l2dec_g_fmt.fmt.pix_mp.height))
					{
						pDec->bH264DecoderConfigurationChanged = true;
						//StreamOffHWPort(pCfg, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); //disable capture port when resolution changes
					}
				}
				tsem_up(pDec->videoH264DecoderEventSem);
			break;

			default:
				LOG_DEF("%s: %d unknown event type(%d)\n", __func__,__LINE__, dqevent.type);
		}
	}
}

int StreamOnHWPort(dec_config_t *pCfg, unsigned int type)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;
	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_STREAMON, &type);
	if (ret < 0)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_STREAMON Failed with ret %x with errno(%d),strerror(%s)\n", __func__,__LINE__, ret, errno,strerror(errno));
		return -1;
	}
	return 0;
}

int StreamOffHWPort(dec_config_t *pCfg, unsigned int type)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;
	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_STREAMOFF, &type);
	if (ret < 0)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_STREAMOFF Failed\n", __func__,__LINE__);
		return -1;
	}
	return 0;
}

int SetupHWV4L2H264DecoderInput(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer querybuf, qbuf;
	unsigned int type;
	int i, ret = 0;

	/* Memory mapping for input buffers in V4L2 */
	memset (&reqbuf, 0, sizeof reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbuf.count = H264_DEC_IN_BUF_COUNT_DEF;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_REQBUFS Failed\n", __func__,__LINE__);
		goto err;
	}

	LOG_PARAM("VIDIOC_REQBUFS: IN requested(%d) vs allocated(%d)\n", H264_DEC_IN_BUF_COUNT_DEF, reqbuf.count);

	pDec->mmap_virtual_input = malloc (sizeof (void *) * reqbuf.count);
	pDec->mmap_size_input = malloc (sizeof (void *) * reqbuf.count);
	pDec->h264DecInBufCount = reqbuf.count;

	for (i = 0; i < reqbuf.count; i++)
	{
		void *m_input_ptr;
		memset(&querybuf, 0, sizeof querybuf);

		querybuf.index = i;
		querybuf.length = 1; //Single plane
		querybuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		querybuf.memory = V4L2_MEMORY_MMAP;
		querybuf.m.planes = (struct v4l2_plane *) malloc(sizeof(struct v4l2_plane) * VIDEO_MAX_PLANES);

		pDec->current_nb_buf_input++;

		/* Memory mapping for input buffers */
		ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_QUERYBUF, &querybuf);
		if (ret < 0)
		{
			LOG_ERR("%s: %d ERROR: VIDIOC_QUERYBUF Failed with ret %x with errno(%d),strerror(%s)\n", __func__,__LINE__, ret, errno,strerror(errno));
			goto err;
		}

		pDec->pH264DecInV4L2Buffer[i] = querybuf;
		m_input_ptr = mmap(NULL, querybuf.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, pDec->hVideoH264Decoder, querybuf.m.planes[0].m.mem_offset);
		if (m_input_ptr == NULL)
		{
			LOG_ERR("%s: %d ERROR: mmap Failed\n", __func__,__LINE__);
			goto err;
		}

		LOG_PARAM("%s: input memory mapped [%d]: ptr[%p] length[%d]\n", __func__, i, m_input_ptr, querybuf.m.planes[0].length);

		/* Queue V4L2 buffer */
		pDec->mmap_virtual_input[i] = m_input_ptr;
		pDec->mmap_size_input[i] = querybuf.m.planes[0].length;
		pDec->bH264DecInBufferWithUs[i] = true;
	}
	//Set stream ON for input
	ret = StreamOnHWPort(pCfg, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
	if (ret != 0)
	{
		LOG_ERR("%s: StreamOnHWPort output failed\n", __func__ );
		goto err;
	}


	return 0;
err:
	return -1;
}

void *hw_dqbuf_capture_watcher(void *data)
{
	dec_config_t *pCfg = (dec_config_t *) data;
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
	int ret = 0;
	struct v4l2_buffer dqbuf, qbuf;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];

	while(pDec->is_running)
	{
		if (pDec->bH264DecoderConfigurationChanged == true) {// in case resolution changes dynamically
			usleep(10000);
			continue;
		}

		// we already submitted a frame during output setup, check if we have a output ready
		memset (&dqbuf, 0, sizeof dqbuf);
		memset(&planes, 0, sizeof(planes));

		dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		dqbuf.length = (pDec->number_output_planes > 1) ? pDec->number_output_planes: 1;//Multi planar for NV12M format
		dqbuf.memory = V4L2_MEMORY_MMAP;
		dqbuf.m.planes = planes;

		ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_DQBUF, &dqbuf);
		if (ret < 0)
		{
			if((errno != EAGAIN) &&  (pDec->bH264DecoderConfigurationChanged != true))
			{
				LOG_ERR("ERROR: VIDIOC_DQBUF Failed on capture errno(%d) with ret %d\n", errno, ret);
				break;
			}
			usleep(10000);
			continue;
		}
		else
		{
			if ((dqbuf.flags & V4L2_BUF_FLAG_LAST)) {
				LOG_SEQ("VIDIOC_DQBUF, capture: EOS received\n");
				struct v4l2_decoder_cmd v4l2DecCmd;
				// Send V4L2_DEC_CMD_START command to decoder followed by streamoff
				memset(&v4l2DecCmd, 0, sizeof v4l2DecCmd);
				v4l2DecCmd.cmd = V4L2_DEC_CMD_START;
				ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_DECODER_CMD, &v4l2DecCmd);
				if (ret < 0)
				{
					LOG_ERR ("%s: VIDIOC_DECODER_CMD failed\n", __func__ );
					return;
				}
				//Set stream ON for capture
				ret = StreamOnHWPort(pCfg, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
				if (ret != 0)
				{
					LOG_ERR("%s: StreamOnHWPort capture failed\n", __func__ );
					return;
				}
				usleep(20000);
				dqbuf.flags &= 0;
				//continue;
			}

			if (dqbuf.m.planes[0].bytesused > 0) {
#if DUMP_DECODED_DATA_HW
				if(codec_config->num_of_sessions < 2 && pDec->total_frames < 200) //dump data only when one session running
				{
					for (int l = 0; l < pDec->number_output_planes; l++)
					{
						dumpData_hw(pDec->mmap_virtual_output[dqbuf.index], dqbuf.m.planes[0].bytesused);
					}
				}
#endif
				pDec->total_frames++;
			}
		}

		qbuf = dqbuf;
		for (int l = 0; l < pDec->number_output_planes; l++)
		{
			qbuf.m.planes[l].bytesused = 0;
		}

		if(pCfg->display_window){
			//handle for multiplanar output / capture  data
			// Currently set for 2 planes
			// TODO: Support for multiplanar generically
			sendProcessDMABufId(pCfg->display_window, pDec->mmap_virtual_output_sharedFDs[dqbuf.index][0], pDec->mmap_virtual_output_sharedFDs[dqbuf.index][1]);
			//sendProcessDMABufId(pCfg->display_window, pDec->mmap_virtual_output_sharedFDs[dqbuf.index][0]);
		}else{
			ret = ioctl(pDec->hVideoH264Decoder,VIDIOC_QBUF, &qbuf);
			if (ret < 0)
			{
				LOG_ERR("ERROR: VIDIOC_QBUF, capture Failed\n");
				break;
			}
		}
	}
}

void *hw_dqbuf_output_watcher(void *data)
{
	dec_config_t *pCfg = (dec_config_t *) data;
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;
	struct v4l2_buffer dqbuf;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];

	while(pDec->is_running)
	{
		memset (&dqbuf, 0, sizeof dqbuf);
		memset(&planes, 0, sizeof(planes));

		dqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		dqbuf.length = 1;//Single Plane
		dqbuf.memory = V4L2_MEMORY_MMAP;
		dqbuf.m.planes = planes;

		ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_DQBUF, &dqbuf);
		if (ret < 0)
		{
			if(errno != EAGAIN)
			{
				LOG_ERR("ERROR: VIDIOC_DQBUF Failed on output: %d\n", ret);
				break;
			}
			usleep(10000);
		}
		else
		{
			pDec->bH264DecInBufferWithUs[dqbuf.index] = true;
		}
	}
}


int SetupHWV4L2H264DecoderOutput(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	struct v4l2_buffer qbuf, querybuf, dqbuf;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_control ctrl;
	int pixelType = 0;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	struct v4l2_exportbuffer expBuff;

	memset (&planes, 0, sizeof (planes));
	int i, ret = 0;

	memset (&expBuff, 0, sizeof (expBuff));
	expBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	expBuff.plane = 0;

	memset (&ctrl, 0, sizeof ctrl);
	ctrl.value = (H264_DEC_OUT_BUF_COUNT_DEF + 1);

	//Request for n count buffers
	memset (&reqbuf, 0, sizeof reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbuf.count = (ctrl.value > H264_DEC_OUT_BUF_COUNT_DEF ? ctrl.value : H264_DEC_OUT_BUF_COUNT_DEF);
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_REQBUFS Failed\n", __func__, __LINE__);
		goto err;
	}
	//LOG_DEF("VIDIOC_REQBUFS: IN requested(%d) vs allocated(%d)\n", (H264_DEC_OUT_BUF_COUNT_DEF + 1), reqbuf.count);

	pDec->h264DecOutBufCount = reqbuf.count;
	pDec->mmap_virtual_output = malloc(sizeof (void *) * reqbuf.count);
	pDec->mmap_size_output = malloc(sizeof (void *) * reqbuf.count);
	for (int v = 0; v < pDec->h264DecOutBufCount; v++)
	{
		pDec->mmap_virtual_output[v] = malloc(sizeof (void *) * pDec->number_output_planes);
		pDec->mmap_size_output[v] = malloc(sizeof (int) * pDec->number_output_planes);
	}

	//query and memory mapping for input buffers
	memset(&querybuf, 0, sizeof querybuf);
	querybuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	querybuf.memory = V4L2_MEMORY_MMAP;
	querybuf.m.planes = planes;

	for(i = 0; i < reqbuf.count; i++)
	{
		void *m_ptr;
		pDec->current_nb_buf_output++;
		querybuf.index = i;
		querybuf.length = (pDec->number_output_planes > 1) ? pDec->number_output_planes: 1;//Multi planar for NV12M format
		expBuff.index = i;

		/* Memory mapping for input buffers */
		ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_QUERYBUF, &querybuf);
		if (ret < 0)
		{
			LOG_ERR("%s: %d ERROR: VIDIOC_QUERYBUF Failed\n",__func__, __LINE__);
			goto err;
		}

		qbuf = querybuf;						/* index from querybuf */
		for (int k = 0; k < pDec->number_output_planes; k++)
		{
			m_ptr = mmap(NULL, querybuf.m.planes[k].length, PROT_READ | PROT_WRITE, MAP_SHARED, pDec->hVideoH264Decoder, querybuf.m.planes[k].m.mem_offset);
			if (m_ptr == NULL)
			{
				LOG_ERR("%s: %d ERROR: mmap Failed\n",__func__, __LINE__);
				goto err;
			}
			//LOG_DEF("%s: output memory mapped [%d]: ptr[%p] length[%d]\n", __func__, i, m_ptr, querybuf.m.planes[k].length);

			pDec->mmap_virtual_output[i][k] = m_ptr;
			pDec->mmap_size_output[i][k] = querybuf.m.planes[k].length;

			qbuf.m.planes[k].bytesused = 0;			/* enqueue it with no data */
		}
		ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_QBUF, &qbuf);
		if (ret < 0)
		{
			LOG_ERR("%s: %d ERROR: VIDIOC_QBUF Failed\n",__func__, __LINE__);
			goto err;
		}

		/*Get shared_fd*/
		if(pCfg->display_window)
		{
			for (int l = 0; l < pDec->number_output_planes; l++)
			{
				expBuff.plane = l;
				ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_EXPBUF, &expBuff);
				if (ret != 0)
				{
					LOG_ERR("Unexpected Error: VIDIOC_EXPBUF failed = %d @ %d\n", ret, __LINE__);
					pDec->mmap_virtual_output_sharedFDs[i][l] = 0;
				}
				else
				{
					//LOG_DEF("Output side SharedFD(%x)\n", expBuff.fd);
					pDec->mmap_virtual_output_sharedFDs[i][l] = expBuff.fd;
				}
			}
		}
	}

	//Set stream ON for capture
	ret = StreamOnHWPort(pCfg, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
	if (ret != 0)
	{
		LOG_ERR("%s: StreamOnHWPort capture failed\n", __func__ );
		goto err;
	}

	/*Send Init msg to UI about this ARGB window */
	if(pCfg->display_window)
	{
		//pixelType = PIXEL_NV12;
		pixelType = PIXEL_NV12M;
		SendInitMessageToUI(SESS_HWDEC,
				pCfg,
				(pCfg->display_window - 1),
				reqbuf.count,
				pCfg->width,
				pCfg->height,
				pixelType);
	}


	return 0;
err:
	return -1;
}
int SetupHWV4L2H264DecoderPorts(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;

	unsigned int ret = 0;
	struct v4l2_format v4l2dec_s_fmt, v4l2dec_g_fmt;
	struct v4l2_event_subscription sub;

	//Set Format for input
	memset (&v4l2dec_s_fmt, 0, sizeof v4l2dec_s_fmt);
	v4l2dec_s_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	v4l2dec_s_fmt.fmt.pix_mp.field = V4L2_FIELD_INTERLACED;
	v4l2dec_s_fmt.fmt.pix_mp.colorspace = V4L2_COLORSPACE_REC709;
	v4l2dec_s_fmt.fmt.pix_mp.width = pCfg->width;
	v4l2dec_s_fmt.fmt.pix_mp.height = pCfg->height;
	if(pCfg->src == FILE_H265)
		v4l2dec_s_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_HEVC;
	else if(pCfg->src == FILE_VP8)
		v4l2dec_s_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VP8;
	else if(pCfg->src == FILE_VP9)
		v4l2dec_s_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VP9;
	else if(pCfg->src == FILE_AV1)
		v4l2dec_s_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_AV1;
	else
		v4l2dec_s_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	v4l2dec_s_fmt.fmt.pix_mp.num_planes = 1;
	/* Considering at least 50% compression of NV12 frame */
	v4l2dec_s_fmt.fmt.pix_mp.plane_fmt[0].sizeimage = (pCfg->width * pCfg->height * 3) >> 2;
	v4l2dec_s_fmt.fmt.pix_mp.plane_fmt[0].bytesperline = 0;

	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_S_FMT, &v4l2dec_s_fmt);
	if (ret < 0)
	{
		LOG_ERR("%s: Output VIDIOC_S_FMT failed\n", __func__);
		goto err;
	}

	memset (&v4l2dec_g_fmt, 0, sizeof v4l2dec_g_fmt);
	v4l2dec_g_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_G_FMT, &v4l2dec_g_fmt);
	if (ret < 0)
	{
		LOG_ERR ("%s: Output VIDIOC_G_FMT failed\n", __func__ );
		goto err;
	}

	LOG_PARAM("Output VIDIOC_G_FMT: (%dx%d), buffersize(%d)\n", v4l2dec_g_fmt.fmt.pix_mp.width, v4l2dec_g_fmt.fmt.pix_mp.height, v4l2dec_g_fmt.fmt.pix_mp.plane_fmt[0].sizeimage);

	//Set Format for output
	v4l2dec_s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	v4l2dec_s_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M; // NM12 format

	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_S_FMT, &v4l2dec_s_fmt);
	if (ret < 0)
	{
		LOG_ERR ("%s: Capture VIDIOC_S_FMT failed\n", __func__);
		goto err;
	}

	memset (&v4l2dec_g_fmt, 0, sizeof v4l2dec_g_fmt);
	v4l2dec_g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_G_FMT, &v4l2dec_g_fmt);
	if (ret < 0)
	{
		LOG_ERR ("%s: Capture VIDIOC_G_FMT failed\n", __func__ );
		goto err;
	}
	pDec->number_output_planes = v4l2dec_g_fmt.fmt.pix_mp.num_planes;

	for(int i=0; i < pDec->number_output_planes; i++)
	{
		LOG_PARAM("Capture VIDIOC_G_FMT: (%dx%d), buffersize(%d)\n", v4l2dec_g_fmt.fmt.pix_mp.width, v4l2dec_g_fmt.fmt.pix_mp.height, v4l2dec_g_fmt.fmt.pix_mp.plane_fmt[i].sizeimage);
	}

	//subscribe events
	memset(&sub, 0, sizeof sub);
	sub.type = V4L2_EVENT_SOURCE_CHANGE;
	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_SUBSCRIBE_EVENT, &sub);
	if (ret < 0)
	{
		LOG_ERR ("%s: VIDIOC_SUBSCRIBE_EVENT failed\n", __func__ );
		return -1;
	}

	memset(&sub, 0, sizeof sub);
	sub.type = V4L2_EVENT_EOS;
	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_SUBSCRIBE_EVENT, &sub);
	if (ret < 0)
	{
		LOG_ERR ("%s: VIDIOC_SUBSCRIBE_EVENT failed\n", __func__ );
		return -1;
	}

	//create event watcher thread to receive events from HW v4l2 decoder driver
	pthread_create(&pDec->v4l2_event_watcher_id, NULL, hw_v4l2_event_watcher, (void *)pCfg);

	return 0;
err:
	return -1;
}

static int hw_v4l2dec_open_device(dec_config_t *pCfg)
{
    int fd = -1;
    int i = 0;
    short found = FALSE;
    char path[100];

	for (i = 0; i < HW_MAX_DEVICES; i++) {
		snprintf (path, sizeof (path), "/dev/video%d", i);

		fd = open (path, O_RDWR | O_NONBLOCK, 0);
		if (fd < 0) {
			LOG_ERR("Couldn't open path:%s continue\n",path);
			continue;
		}

		struct v4l2_capability cap;
		struct v4l2_fmtdesc fdesc;

		memset (&cap, 0, sizeof cap);
		if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
			LOG_ERR("Failed to verify capabilities: %m\n");
			close (fd);
			continue;
		}

		LOG_PARAM("caps (%s): driver=\"%s\" bus_info=\"%s\" card=\"%s\" "
				"version=%u.%u.%u\n", path, cap.driver, cap.bus_info, cap.card,
				(cap.version >> 16) & 0xff,
				(cap.version >> 8) & 0xff,
				cap.version & 0xff);

		if (!(cap.capabilities & V4L2_CAP_STREAMING) ||
				!(cap.capabilities & V4L2_CAP_VIDEO_M2M_MPLANE)) {
			LOG_ERR("Insufficient capabilities for video device (is %s correct?)\n",
					path);
			close (fd);
			continue;
		}

		memset (&fdesc, 0, sizeof fdesc);
		fdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

		while (!ioctl(fd, VIDIOC_ENUM_FMT, &fdesc)) {
			LOG_PARAM("  %s %d\n", fdesc.description, fdesc.index);

			switch (fdesc.pixelformat) {
				case V4L2_PIX_FMT_NV12M:
					found = TRUE;
					break;
				default:
					{
						LOG_ERR("%s is not a decoder video device @(%d)\n", path, __LINE__);
						close (fd);
						continue;
					}
			}

			if (found)
				break;

			fdesc.index++;
		}

		found = FALSE;
		memset (&fdesc, 0, sizeof fdesc);
		fdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;

		while (!ioctl(fd, VIDIOC_ENUM_FMT, &fdesc)) {
			LOG_PARAM("  %s\n", fdesc.description);

			switch (fdesc.pixelformat) {
				case V4L2_PIX_FMT_H264:
					found = TRUE;
					break;
				case V4L2_PIX_FMT_HEVC:
					found = TRUE;
					break;
				default:
					{
						LOG_ERR("%s is not a decoder video device @(%d)\n", path, __LINE__);
						close (fd);
						continue;
					}
			}

			if (found)
				break;

			fdesc.index++;
		}
		if (found)
			break;
	}

    if (!found) {
        LOG_ERR("No device found matching format V4L2_PIX_FMT_NV12M\n");
        return -1;
    }

    LOG_DEF("Device %s opened for format V4L2_PIX_FMT_NV12M\n", path);

    return fd;
}

int SetupHWV4L2H264Dec(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;

	pDec->bH264DecoderInitialized = false;
	//pDec->window_displaying = 0;
	pDec->videoH264DecoderEventSem = malloc(sizeof(tsem_t));
	pDec->videoH264DecoderEofSem = malloc(sizeof(tsem_t));

	tsem_init(pDec->videoH264DecoderEventSem, 0);
	tsem_init(pDec->videoH264DecoderEofSem, 0);


	pDec->hVideoH264Decoder = hw_v4l2dec_open_device(pCfg);
	if (pDec->hVideoH264Decoder < 0)
	{
		LOG_ERR("HW Decoder Open failed: %x\n", ret);
		goto err;
	}
	else {
		//LOG_DEF("HW Decoder handle = 0x%x\n", (int)pDec->hVideoH264Decoder);
	}

	if(SetupHWV4L2H264DecoderPorts(pCfg) < 0)
	{
		LOG_ERR("SetupHWV4L2H264DecoderPorts failed: %x\n", ret);
		goto err;
	}

	if(SetupHWV4L2H264DecoderInput(pCfg) < 0)
	{
		LOG_ERR("SetupHWV4L2H264DecoderInput failed: %x\n", ret);
		goto err;
	}

	pDec->gstRx.h264DecoderQueueSem = calloc(1,sizeof(tsem_t));
	tsem_init(pDec->gstRx.h264DecoderQueueSem, 0);

	pDec->gstRx.h264DecoderStreamQueue = calloc(1,sizeof(queue_t));
	queue_init(pDec->gstRx.h264DecoderStreamQueue);

	pDec->h264DecInBufCurrCount = 0;

	if ((pCfg->src == FILE_H264) || (pCfg->src == FILE_H265)) {
		pDec->frame_duration_us = (1000*1000)/pCfg->fps;
		gettimeofday(&pDec->time_stamp, NULL);
	}

	return 0;
err:
	return -1;
}

int GetHWV4L2H264DecFreeInputBuffer(dec_config_t *pCfg, int *index)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;

	for(int i=0; i<pDec->h264DecInBufCount; i++)
	{
		if(pDec->bH264DecInBufferWithUs[i] == true)
		{
			*index = i;
			return 0;
		}
	}

	return -1;
}

#ifdef LOCAL_LOOPBACK_MODE
queue_element_t * checkSPS_PPS_IDR(queue_element_t *tmpBuf, dec_config_t *pCfg) {
	int i;
	queue_element_t *newTmpBuf1 = NULL;
	queue_element_t *newTmpBuf2 = NULL;
	bool short_header = false;
	int nalu_type = -1;
	unsigned char *data = tmpBuf->pBuffer;

	if(data[2] == 0x01){
		short_header = true;
		nalu_type = (data[3] & 0x1F);
	}else if(data[3] == 0x01){
		nalu_type = (data[4] & 0x1F);
	}

	if(nalu_type == -1){
		LOG_DEF("ERROR: nalu_type is invalid\n");
		return tmpBuf;
	}

	if(nalu_type == 7){ //SPS
		codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
		newTmpBuf1 = dequeue(codec_config->sharedH264Queue);
		tsem_timed_down(codec_config->sharedH264QueueSem, 2*1000);

		queue_element_t *newTmpBuf2 = (queue_element_t*)malloc(sizeof(queue_element_t));
		newTmpBuf2->pBuffer = (uint32_t)malloc(tmpBuf->nFilledLen+ newTmpBuf1->nFilledLen);
		memcpy(newTmpBuf2->pBuffer, tmpBuf->pBuffer, tmpBuf->nFilledLen);
		memcpy(newTmpBuf2->pBuffer + tmpBuf->nFilledLen, newTmpBuf1->pBuffer, newTmpBuf1->nFilledLen);
		newTmpBuf2->nFilledLen = tmpBuf->nFilledLen + newTmpBuf1->nFilledLen;
		free(tmpBuf->pBuffer);
		free(tmpBuf);
		free(newTmpBuf1->pBuffer);
		free(newTmpBuf1);
		return newTmpBuf2;
	}else{
		return tmpBuf;
	}

}
#endif

int FeedBufferToHWV4L2H264Decoder(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	queue_element_t *tmpBuf = NULL;
	struct v4l2_buffer qbuf, querybuf;
	int i = 0;
	int ret = 0;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);

	if (pDec->bH264DecoderConfigurationChanged)
	{
		LOG_SEQ("%s: bH264DecoderConfigurationChanged: %d\n", __func__, pDec->bH264DecoderConfigurationChanged );
		tsem_down(pDec->videoH264DecoderEventSem);
		StreamOffHWPort(pCfg, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); //disable capture port when resolution changes
		//munmap for output
		for (int index = 0; index < pDec->h264DecOutBufCount; index++)
		{
			for (int k = 0; k < pDec->number_output_planes; k++)
			{
				if (0 != munmap(pDec->mmap_virtual_output[index][k], pDec->mmap_size_output[index][k]))
				{
					LOG_ERR("Output Munmap failed for index: %d\n", index);
					return -1;
				}
			}
			if (pDec->mmap_virtual_output[index])
				free(pDec->mmap_virtual_output[index]);
			if (pDec->mmap_size_output[index])
				free(pDec->mmap_size_output[index]);
		}

		if (pDec->mmap_virtual_output)
			free(pDec->mmap_virtual_output);
		if (pDec->mmap_size_output)
			free(pDec->mmap_size_output);

		if(SetupHWV4L2H264DecoderOutput(pCfg) < 0)
		{
			LOG_ERR("SetupHWV4L2H264DecoderOutput failed: %x @(%d)\n", ret, __LINE__);
			goto err;
		}
		pDec->bH264DecoderConfigurationChanged = false;
		//LOG_DEF("%s: Configured decoder\n", __func__);
	}
#if 0
TODO: To be enabled for NW usecase where we may see corrupted data
	if (pDec->bH264DecoderInitialized == true)
	{

		while (GetHWV4L2H264DecFreeInputBuffer(pCfg, &i) < 0)
		{
			int queueLen;
			queueLen = getquenelem(pDec->gstRx.h264DecoderStreamQueue);
			if(queueLen > 50)
			{
				//LOG_DEF("Dropping frame\n");
				tmpBuf = dequeue(pDec->gstRx.h264DecoderStreamQueue);
				free(tmpBuf->pBuffer);
				free(tmpBuf);
			}
			else
			{
				//LOG_DEF("%s: No free buffer available..sleeping 30ms\n", __func__);
				usleep(5000);
				// No free buffer so increment semaphore as we have not used the buffer from the queue
				tsem_up(pDec->gstRx.h264DecoderQueueSem);
			}
			return 0;
		}

		return 0;
	}
	else
#endif
	{
		if(GetHWV4L2H264DecFreeInputBuffer(pCfg, &i) < 0)
		{
#ifdef LOCAL_LOOPBACK_MODE
			if(pCfg->instance_id == LOCAL_LOOPBACK_DEC_INST){
				codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
				tsem_up(codec_config->sharedH264QueueSem);
			}else{
				tsem_up(pDec->gstRx.h264DecoderQueueSem);
			}
#else
			// No free buffer so increment semaphore as buffer not used from the queue
			tsem_up(pDec->gstRx.h264DecoderQueueSem);
#endif //LOCAL_LOOPBACK_MODE
			return 0;
		}
	}
#ifdef LOCAL_LOOPBACK_MODE
	if(pCfg->instance_id == LOCAL_LOOPBACK_DEC_INST){
		codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
		tmpBuf = dequeue(codec_config->sharedH264Queue);
	}else{
		tmpBuf = dequeue(pDec->gstRx.h264DecoderStreamQueue);
	}
#else
	tmpBuf = dequeue(pDec->gstRx.h264DecoderStreamQueue);
#endif //LOCAL_LOOPBACK_MODE
	if(tmpBuf == NULL){
		LOG_ERR("Fn(%s): dequeue returned NULL\n", __func__);
		goto err;
	}

	if(tmpBuf->nFilledLen > pDec->mmap_size_input[i]) {
		LOG_ERR ("%s:%d Error - AU is larger than buffer\n", __FUNCTION__,__LINE__);
		goto err;
	}

#ifdef LOCAL_LOOPBACK_MODE
	/*Check if SPS/PPS, Accumulate complete AU*/
	if(pCfg->instance_id == LOCAL_LOOPBACK_DEC_INST){
		codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
		tmpBuf = checkSPS_PPS_IDR(tmpBuf, pCfg);
		while(getquenelem(codec_config->sharedH264Queue) > 1){
			queue_element_t *newTmpBuf1 = dequeue(codec_config->sharedH264Queue);
			free(newTmpBuf1->pBuffer);
			free(newTmpBuf1);
			tsem_timed_down(codec_config->sharedH264QueueSem, 2*1000);
		}
	}
#endif

	qbuf = pDec->pH264DecInV4L2Buffer[i];
	memcpy(pDec->mmap_virtual_input[i], tmpBuf->pBuffer, tmpBuf->nFilledLen);
	qbuf.m.planes[0].bytesused = (uint32_t) (tmpBuf->nFilledLen);	/* enqueue it with data */
	pDec->bH264DecInBufferWithUs[i] = false;

	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_QBUF, &qbuf);
	if (ret < 0)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_QBUF output Failed\n", __func__,__LINE__);
		goto err;
	}

	free(tmpBuf->pBuffer);
	free(tmpBuf);
	if (pDec->bH264DecoderInitialized == false)
	{
		//check if res change event received before output setup
		tsem_down(pDec->videoH264DecoderEventSem);

		if(SetupHWV4L2H264DecoderOutput(pCfg) < 0)
		{
			LOG_ERR("SetupHWV4L2H264DecoderOutput failed: %x @(%d)\n", ret, __LINE__);
			goto err;
		}
		pDec->bH264DecoderInitialized = true;

		pthread_create(&pDec->dqbuf_capture_watcher_id, NULL, hw_dqbuf_capture_watcher, (void *)pCfg);
		pthread_create(&pDec->dqbuf_output_watcher_id, NULL, hw_dqbuf_output_watcher, (void *)pCfg);
	}

	if ((pCfg->src == FILE_H264)|| (pCfg->src == FILE_H265))
		SleepForMaintainingFpsAtDecoder(pCfg);

	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return 0;
err:
	return -1;
}

void StopHWV4L2H264Decoder(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;
	struct v4l2_event_subscription sub;
	struct v4l2_decoder_cmd v4l2DecCmd;

	// Send V4L2_DEC_CMD_STOP command to decoder followed by streamoff
	memset(&v4l2DecCmd, 0, sizeof v4l2DecCmd);
	v4l2DecCmd.cmd = V4L2_DEC_CMD_STOP;
	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_DECODER_CMD, &v4l2DecCmd);
	if (ret < 0)
	{
		LOG_ERR ("%s: VIDIOC_DECODER_CMD failed\n", __func__ );
		return;
	}

	//subscribe events to receive from HW v4l2 decoder driver
	memset(&sub, 0, sizeof sub);
	sub.type = V4L2_EVENT_ALL;
	ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
	if (ret < 0)
	{
		LOG_ERR ("%s: VIDIOC_UNSUBSCRIBE_EVENT failed\n", __func__ );
		return;
	}

	StreamOffHWPort(pCfg, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
	StreamOffHWPort(pCfg, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
}

void ShutdownHWV4L2H264Decoder(dec_config_t *pCfg)
{
	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;
	int index = 0;

	if (pDec->dqbuf_output_watcher_id)
	{
		pthread_join(pDec->dqbuf_output_watcher_id, NULL);
	}
	if (pDec->dqbuf_capture_watcher_id)
	{
		pthread_join(pDec->dqbuf_capture_watcher_id, NULL);
	}
	// Make sure we exit from the event watcher thread
	pDec->bH264DecoderRecievedEOS = true;

	// Free buffers if any	
	queue_deinit(pDec->gstRx.h264DecoderStreamQueue);

	for(int i=0; i<pDec->h264DecInBufCount; i++)
	{
		if (pDec->pH264DecInV4L2Buffer[i].m.planes)
		{
			free(pDec->pH264DecInV4L2Buffer[i].m.planes);
		}
	}

	free(pDec->gstRx.h264DecoderStreamQueue);
	free(pDec->gstRx.h264DecoderQueueSem);
	free(pDec->videoH264DecoderEventSem);
	free(pDec->videoH264DecoderEofSem);
	//munmap for input
	for (index = 0; index < pDec->h264DecInBufCount; index++)
	{
		if (0 != munmap(pDec->mmap_virtual_input[index], pDec->mmap_size_input[index]))
		{
			LOG_ERR("Input Munmap failed for index: %d\n", index);
			return -1;
		}
	}

	//munmap for output
	for (index = 0; index < pDec->h264DecOutBufCount; index++)
	{
		for (int k = 0; k < pDec->number_output_planes; k++)
		{
			if (0 != munmap(pDec->mmap_virtual_output[index][k], pDec->mmap_size_output[index][k]))
			{
				LOG_ERR("Output Munmap failed for index: %d\n", index);
				return -1;
			}
		}
		if (pDec->mmap_virtual_output[index])
			free(pDec->mmap_virtual_output[index]);
		if (pDec->mmap_size_output[index])
			free(pDec->mmap_size_output[index]);
	}

	if (pDec->mmap_virtual_output)
		free(pDec->mmap_virtual_output);
	if (pDec->mmap_size_output)
		free(pDec->mmap_size_output);
	free(pDec->mmap_virtual_input);
	free(pDec->mmap_size_input);

	ret = close(pDec->hVideoH264Decoder);
	if (ret < 0)
	{
		LOG_ERR("HW decoder Close Failed\n");
	}
	if (pDec->v4l2_event_watcher_id)
	{
		pthread_join(pDec->v4l2_event_watcher_id, NULL);
	}
}

void enqueBufFromUIHWDec(dec_config_t *pCfg, int shareFd)
{
	if(pCfg == NULL)
		return;

	priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;
	int i, ret;

	for(i = 0; i < pDec->h264DecOutBufCount; i++){
		//for(int k = 0; k < pDec->number_output_planes; k++){
			// handle for multi planar data
			if(pDec->mmap_virtual_output_sharedFDs[i][0] == shareFd)
				break;
		//}
	}

	if( i < pDec->h264DecOutBufCount){
		struct v4l2_buffer qbuf;
		struct v4l2_plane planes[VIDEO_MAX_PLANES];

		memset(&qbuf, 0, sizeof(struct v4l2_buffer));
		memset (&planes, 0, sizeof (planes));

		qbuf.index = i;
		qbuf.length = pDec->number_output_planes;
		qbuf.m.planes = planes;
		for (int k = 0; k < pDec->number_output_planes; k++)
		{
			qbuf.m.planes[k].bytesused = 0;
		}
		qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		qbuf.memory = V4L2_MEMORY_MMAP;

		ret = ioctl(pDec->hVideoH264Decoder, VIDIOC_QBUF, &qbuf);
		if (ret < 0) {
			LOG_ERR("ERROR: VIDIOC_QBUF, enQueueAMPBuffer Failed\n");
		}
	}else{
		LOG_ERR("enqueBufFromUI bo_name(%d) not found\n", shareFd);
	}
}
