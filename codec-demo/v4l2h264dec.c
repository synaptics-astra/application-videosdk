#include "v4l2h264dec.h"

//#define DUMP_DECODED_DATA 1
#if DUMP_DECODED_DATA
void dumpData(char *data, int data_size)
{
	static FILE *fp = NULL;
	static int iCnt;

	if(!iCnt){
		fp = fopen("decoder_output.raw","wb");
	} else {
		fp = fopen("decoder_output.raw","ab");
	}
	fwrite(data, 1, data_size, fp);
	fsync(fileno(fp));
	fclose(fp);
	iCnt++;
}
#endif

static void SleepForMaintainingFpsAtDecoder(dec_config_t *pCfg)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
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

void *v4l2_event_watcher(void *data)
{
	dec_config_t *pCfg = (dec_config_t *) data;
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;
	struct v4l2_event dqevent;

	while(pDec->bH264DecoderRecievedEOS != true)
	{
		memset(&dqevent, 0, sizeof(struct v4l2_event));
		ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_DQEVENT, &dqevent);
		if (ret != V4L2_SUCCESS)
		{
			usleep(10000);
			continue;
		}
		LOG_DEF("%s: %d VIDIOC_DQEVENT: %d\n", __func__,__LINE__, dqevent.type);

		switch (dqevent.type) {
			case V4L2_EVENT_SOURCE_CHANGE:
				LOG_DEF("%s: %d src_change %d\n", __func__,__LINE__, dqevent.u.src_change.changes);
				if (dqevent.u.src_change.changes == V4L2_EVENT_SRC_CH_RESOLUTION) {
					pDec->src_res_change++;
				}
				if (pDec->src_res_change > 1) {
					pDec->bH264DecoderConfigurationChanged = true;
					StreamOffPort(pCfg, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); //disable capture port when resolution changes
				}
				tsem_up(pDec->videoH264DecoderEventSem);
			break;

			default:
				LOG_DEF("%s: %d unknown event\n", __func__,__LINE__);
		}
	}
}

int StreamOnPort(dec_config_t *pCfg, unsigned int type)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	int ret = V4L2_SUCCESS;
	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_STREAMON, &type);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_STREAMON Failed\n", __func__,__LINE__);
		return -1;
	}
	return 0;
}

int StreamOffPort(dec_config_t *pCfg, unsigned int type)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	int ret = V4L2_SUCCESS;
	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_STREAMOFF, &type);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_STREAMOFF Failed\n", __func__,__LINE__);
		return -1;
	}
	return 0;
}

int SetupV4L2H264DecoderInput(dec_config_t *pCfg)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer querybuf, qbuf;
	unsigned int type;
	int i, ret = V4L2_SUCCESS;

	//Set stream ON for input
	ret = StreamOnPort(pCfg, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
	if (ret != 0)
	{
		LOG_ERR("%s: StreamOnPort output failed\n", __func__ );
		goto err;
	}

	/* Memory mapping for input buffers in V4L2 */
	memset (&reqbuf, 0, sizeof reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbuf.count = H264_DEC_IN_BUF_COUNT_DEF;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_REQBUFS, &reqbuf);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_REQBUFS Failed\n", __func__,__LINE__);
		goto err;
	}

	LOG_DEF("VIDIOC_REQBUFS: IN requested(%d) vs allocated(%d)\n", H264_DEC_IN_BUF_COUNT_DEF, reqbuf.count);

	pDec->mmap_virtual_input = malloc (sizeof (void *) * reqbuf.count);
	pDec->mmap_size_input = malloc (sizeof (void *) * reqbuf.count);
	pDec->h264DecInBufCount = reqbuf.count;

	for (i = 0; i < reqbuf.count; i++)
	{
		void *m_input_ptr;
		memset(&querybuf, 0, sizeof querybuf);

		querybuf.index = i;
		querybuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		querybuf.memory = V4L2_MEMORY_MMAP;
		querybuf.m.planes = (struct v4l2_plane *) malloc(sizeof(struct v4l2_plane) * VIDEO_MAX_PLANES);

		pDec->current_nb_buf_input++;

		/* Memory mapping for input buffers */
		ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_QUERYBUF, &querybuf);
		if (ret != V4L2_SUCCESS)
		{
			LOG_ERR("%s: %d ERROR: VIDIOC_QUERYBUF Failed\n", __func__,__LINE__);
			goto err;
		}

		pDec->pH264DecInV4L2Buffer[i] = querybuf;
		m_input_ptr = SynaV4L2_Mmap(pDec->hVideoH264Decoder, NULL, querybuf.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, querybuf.m.planes[0].m.mem_offset);
		if (m_input_ptr == NULL)
		{
			LOG_ERR("%s: %d ERROR: SynaV4L2_Mmap Failed\n", __func__,__LINE__);
			goto err;
		}

		LOG_DEF("%s: input memory mapped [%d]: ptr[%p] length[%d]\n", __func__, i, m_input_ptr, querybuf.m.planes[0].length);

		/* Queue V4L2 buffer */
		pDec->mmap_virtual_input[i] = m_input_ptr;
		pDec->mmap_size_input[i] = querybuf.m.planes[0].length;
		pDec->bH264DecInBufferWithUs[i] = true;
	}

	return 0;
err:
	return -1;
}

void *dqbuf_capture_watcher(void *data)
{
	dec_config_t *pCfg = (dec_config_t *) data;
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
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
		dqbuf.memory = V4L2_MEMORY_MMAP;
		dqbuf.m.planes = planes;

		ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_DQBUF, &dqbuf);
		if (ret != V4L2_SUCCESS)
		{
			if(ret != ERROR_EAGAIN)
			{
				LOG_ERR("ERROR: VIDIOC_DQBUF Failed on capture\n");
				break;
			}
			usleep(10000);
			continue;
		}
		else
		{
			if ((dqbuf.flags & V4L2_BUF_FLAG_LAST)) {
				LOG_DEF("VIDIOC_DQBUF, capture: EOS received\n");
				pDec->bH264DecoderRecievedEOS = true;
				pDec->is_running = 0;
				tsem_up(pDec->gstRx.h264DecoderQueueSem);
				break;
			}

			if (dqbuf.m.planes[0].bytesused > 0) {
#if DUMP_DECODED_DATA
				if(codec_config->num_of_sessions < 2 && pDec->total_frames < 200) //dump data only when one session running
					dumpData(pDec->mmap_virtual_output[dqbuf.index], dqbuf.m.planes[0].bytesused);
#endif
				pDec->total_frames++;
			}
		}

		qbuf = dqbuf;
		qbuf.m.planes[0].bytesused = 0;

		if(pCfg->display_window){
			sendProcessDMABufId(pCfg->display_window, pDec->mmap_virtual_output_sharedFDs[dqbuf.index], NULL);
		}else{
			ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder,VIDIOC_QBUF, &qbuf);
			if (ret != V4L2_SUCCESS)
			{
				LOG_ERR("ERROR: VIDIOC_QBUF, capture Failed\n");
				break;
			}
		}
	}
}

void *dqbuf_output_watcher(void *data)
{
	dec_config_t *pCfg = (dec_config_t *) data;
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;
	struct v4l2_buffer dqbuf;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];

	while(pDec->is_running)
	{
		memset (&dqbuf, 0, sizeof dqbuf);
		memset(&planes, 0, sizeof(planes));

		dqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		dqbuf.memory = V4L2_MEMORY_MMAP;
		dqbuf.m.planes = planes;

		ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_DQBUF, &dqbuf);
		if (ret != V4L2_SUCCESS)
		{
			if(ret != ERROR_EAGAIN)
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


int SetupV4L2H264DecoderOutput(dec_config_t *pCfg)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	struct v4l2_buffer qbuf, querybuf, dqbuf;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_control ctrl;
	int pixelType = 0;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];

	memset (&planes, 0, sizeof (planes));
	int i, ret = V4L2_SUCCESS;

	//Set stream ON for capture
	ret = StreamOnPort(pCfg, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
	if (ret != 0)
	{
		LOG_ERR("%s: StreamOnPort capture failed\n", __func__ );
		goto err;
	}

	memset (&ctrl, 0, sizeof ctrl);
	ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;
	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_G_CTRL, (void*) &ctrl);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Ioctl for Capture VIDIOC_G_CTRL failed = %d\n", ret);
		goto err;
	}
	LOG_DEF("Min buffer required for Capture: %d\n", ctrl.value);

	//Request for n count buffers
	memset (&reqbuf, 0, sizeof reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbuf.count = (ctrl.value > H264_DEC_OUT_BUF_COUNT_DEF ? ctrl.value : H264_DEC_OUT_BUF_COUNT_DEF);
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_REQBUFS, &reqbuf);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR("%s: %d ERROR: VIDIOC_REQBUFS Failed\n", __func__, __LINE__);
		goto err;
	}

	pDec->mmap_virtual_output = malloc(sizeof (void *) * reqbuf.count);
	pDec->mmap_size_output = malloc(sizeof (void *) * reqbuf.count);
	pDec->h264DecOutBufCount = reqbuf.count;

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

		/* Memory mapping for input buffers */
		ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_QUERYBUF, &querybuf);
		if (ret != V4L2_SUCCESS)
		{
			LOG_ERR("%s: %d ERROR: VIDIOC_QUERYBUF Failed\n",__func__, __LINE__);
			goto err;
		}

		m_ptr = SynaV4L2_Mmap(pDec->hVideoH264Decoder, NULL, querybuf.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, querybuf.m.planes[0].m.mem_offset);
		if (m_ptr == NULL)
		{
			LOG_ERR("%s: %d ERROR: SynaV4L2_Mmap Failed\n",__func__, __LINE__);
			goto err;
		}
		LOG_DEF("%s: output memory mapped [%d]: ptr[%p] length[%d]\n", __func__, i, m_ptr, querybuf.m.planes[0].length);

		pDec->mmap_virtual_output[i] = m_ptr;
		pDec->mmap_size_output[i] = querybuf.m.planes[0].length;

		qbuf = querybuf;						/* index from querybuf */
		qbuf.m.planes[0].bytesused = 0;			/* enqueue it with no data */
		ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_QBUF, &qbuf);
		if (ret != V4L2_SUCCESS)
		{
			LOG_ERR("%s: %d ERROR: VIDIOC_QBUF Failed\n",__func__, __LINE__);
			goto err;
		}

		/*Get shared_fd*/
		if(pCfg->display_window)
			SynaV4L2_GetSharedFD(pDec->hVideoH264Decoder,
					querybuf.m.planes[0].m.mem_offset,
					V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE,
					&pDec->mmap_virtual_output_sharedFDs[i]);
	}

	/*Send Init msg to UI about this ARGB window */
	if(pCfg->display_window)
	{
		pixelType = PIXEL_NV12;
		SendInitMessageToUI(SESS_AMPDEC,
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
int SetupV4L2H264DecoderPorts(dec_config_t *pCfg)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;

	unsigned int ret = V4L2_SUCCESS;
	struct v4l2_format v4l2dec_s_fmt, v4l2dec_g_fmt;
	struct v4l2_event_subscription sub;

	//Set Format for input
	memset (&v4l2dec_s_fmt, 0, sizeof v4l2dec_s_fmt);
	v4l2dec_s_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	v4l2dec_s_fmt.fmt.pix_mp.field = V4L2_FIELD_INTERLACED;
	v4l2dec_s_fmt.fmt.pix_mp.colorspace = V4L2_COLORSPACE_REC709;
	v4l2dec_s_fmt.fmt.pix_mp.width = pCfg->width;
	v4l2dec_s_fmt.fmt.pix_mp.height = pCfg->height;
	v4l2dec_s_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	v4l2dec_s_fmt.fmt.pix_mp.num_planes = 1;
	/* Considering at least 50% compression of NV12 frame */
	v4l2dec_s_fmt.fmt.pix_mp.plane_fmt[0].sizeimage = (pCfg->width * pCfg->height * 3) >> 2;
	v4l2dec_s_fmt.fmt.pix_mp.plane_fmt[0].bytesperline = 0;

	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_S_FMT, &v4l2dec_s_fmt);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR("%s: Output VIDIOC_S_FMT failed\n", __func__);
		goto err;
	}

	memset (&v4l2dec_g_fmt, 0, sizeof v4l2dec_g_fmt);
	v4l2dec_g_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_G_FMT, &v4l2dec_g_fmt);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR ("%s: Output VIDIOC_G_FMT failed\n", __func__ );
		goto err;
	}

	LOG_DEF("Output VIDIOC_G_FMT: (%dx%d), buffersize(%d)\n", v4l2dec_g_fmt.fmt.pix_mp.width, v4l2dec_g_fmt.fmt.pix_mp.height, v4l2dec_g_fmt.fmt.pix_mp.plane_fmt[0].sizeimage);

	//Set Format for output
	v4l2dec_s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	v4l2dec_s_fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;

	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_S_FMT, &v4l2dec_s_fmt);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR ("%s: Capture VIDIOC_S_FMT failed\n", __func__);
		goto err;
	}

	memset (&v4l2dec_g_fmt, 0, sizeof v4l2dec_g_fmt);
	v4l2dec_g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_G_FMT, &v4l2dec_g_fmt);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR ("%s: Capture VIDIOC_G_FMT failed\n", __func__ );
		goto err;
	}

	LOG_DEF("Capture VIDIOC_G_FMT: (%dx%d), buffersize(%d)\n", v4l2dec_g_fmt.fmt.pix_mp.width, v4l2dec_g_fmt.fmt.pix_mp.height, v4l2dec_g_fmt.fmt.pix_mp.plane_fmt[0].sizeimage);

	//subscribe events to receive from libsynav4l2 layer
	memset(&sub, 0, sizeof sub);
	sub.type = V4L2_EVENT_SOURCE_CHANGE;
	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_SUBSCRIBE_EVENT, &sub);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR ("%s: VIDIOC_SUBSCRIBE_EVENT failed\n", __func__ );
		return -1;
	}
	//create event watcher thread to look for event receive from libsynav4l2
	pthread_create(&pDec->v4l2_event_watcher_id, NULL, v4l2_event_watcher, (void *)pCfg);

	return 0;
err:
	return -1;
}

int SetupV4L2H264Dec(dec_config_t *pCfg)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;

	pDec->bH264DecoderInitialized = false;
	//pDec->window_displaying = 0;
	pDec->videoH264DecoderEventSem = malloc(sizeof(tsem_t));
	pDec->videoH264DecoderEofSem = malloc(sizeof(tsem_t));

	tsem_init(pDec->videoH264DecoderEventSem, 0);
	tsem_init(pDec->videoH264DecoderEofSem, 0);


	ret = SynaV4L2_Open(&pDec->hVideoH264Decoder, LIBSYNAV4L2_DECODER_INDEX, V4L2_PIX_FMT_H264, O_NONBLOCK);
	if (ret == V4L2_SUCCESS)
		LOG_DEF("SynaV4L2_Open handle = 0x%x\n", (int)pDec->hVideoH264Decoder);
	else {
		LOG_ERR("SynaV4L2_Open failed: %x\n", ret);
		goto err;
	}

	if(SetupV4L2H264DecoderPorts(pCfg) < 0)
	{
		LOG_ERR("SetupV4L2H264DecoderPorts failed: %x\n", ret);
		goto err;
	}

	if(SetupV4L2H264DecoderInput(pCfg) < 0)
	{
		LOG_ERR("SetupV4L2H264DecoderInput failed: %x\n", ret);
		goto err;
	}

	pDec->gstRx.h264DecoderQueueSem = calloc(1,sizeof(tsem_t));
	tsem_init(pDec->gstRx.h264DecoderQueueSem, 0);

	pDec->gstRx.h264DecoderStreamQueue = calloc(1,sizeof(queue_t));
	queue_init(pDec->gstRx.h264DecoderStreamQueue);

	pDec->h264DecInBufCurrCount = 0;

	if (pCfg->src == FILE_H264) {
		pDec->frame_duration_us = (1000*1000)/pCfg->fps;
		gettimeofday(&pDec->time_stamp, NULL);
	}

	return 0;
err:
	return -1;
}

int GetV4L2H264DecFreeInputBuffer(dec_config_t *pCfg, int *index)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;

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

int FeedBufferToV4L2H264Decoder(dec_config_t *pCfg)
{

	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	queue_element_t *tmpBuf = NULL;
	struct v4l2_buffer qbuf, querybuf;
	int i = 0;
	int ret = 0;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);

	if (pDec->bH264DecoderConfigurationChanged)
	{
		LOG_DEF("%s: bH264DecoderConfigurationChanged: %d\n", __func__, pDec->bH264DecoderConfigurationChanged );

		tsem_down(pDec->videoH264DecoderEventSem);

		if (pDec->mmap_virtual_output) {
			free(pDec->mmap_virtual_output);
			free(pDec->mmap_size_output);
		}

		if(SetupV4L2H264DecoderOutput(pCfg) < 0)
		{
			LOG_ERR("SetupV4L2H264DecoderOutput failed: %x\n", ret);
			goto err;
		}
		pDec->bH264DecoderConfigurationChanged = false;
		LOG_DEF("%s: Configured decoder\n", __func__);
	}
#if 0
TODO: To be enabled for NW usecase where we may see corrupted data
	if (pDec->bH264DecoderInitialized == true)
	{

		while (GetV4L2H264DecFreeInputBuffer(pCfg, &i) < 0)
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
		if(GetV4L2H264DecFreeInputBuffer(pCfg, &i) < 0)
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

	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_QBUF, &qbuf);
	if (ret != V4L2_SUCCESS)
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

		if(SetupV4L2H264DecoderOutput(pCfg) < 0)
		{
			LOG_ERR("SetupV4L2H264DecoderOutput failed: %x\n", ret);
			goto err;
		}
		pDec->bH264DecoderInitialized = true;

		pthread_create(&pDec->dqbuf_capture_watcher_id, NULL, dqbuf_capture_watcher, (void *)pCfg);
		pthread_create(&pDec->dqbuf_output_watcher_id, NULL, dqbuf_output_watcher, (void *)pCfg);
	}

	if (pCfg->src == FILE_H264)
		SleepForMaintainingFpsAtDecoder(pCfg);

	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return 0;
err:
	return -1;
}

void StopV4L2H264Decoder(dec_config_t *pCfg)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;

	//subscribe events to receive from libsynav4l2 layer
	struct v4l2_event_subscription sub;
	memset(&sub, 0, sizeof sub);
	sub.type = V4L2_EVENT_SOURCE_CHANGE;
	ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR ("%s: VIDIOC_UNSUBSCRIBE_EVENT failed\n", __func__ );
		return;
	}

	StreamOffPort(pCfg, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
	StreamOffPort(pCfg, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
}

void ShutdownV4L2H264Decoder(dec_config_t *pCfg)
{
	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	int ret = 0;

	if (pDec->dqbuf_output_watcher_id)
		pthread_join(pDec->dqbuf_output_watcher_id, NULL);
	if (pDec->dqbuf_capture_watcher_id)
		pthread_join(pDec->dqbuf_capture_watcher_id, NULL);
	// Make sure we exit from the event watcher thread
	pDec->bH264DecoderRecievedEOS = true;
	if (pDec->v4l2_event_watcher_id)
		pthread_join(pDec->v4l2_event_watcher_id, NULL);

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
	free(pDec->mmap_virtual_input);
	free(pDec->mmap_size_input);
	free(pDec->mmap_virtual_output);
	free(pDec->mmap_size_output);

	ret = SynaV4L2_Close(pDec->hVideoH264Decoder);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR("SynaV4L2_Close Failed\n");
	}
}

void enqueBufFromUIDec(dec_config_t *pCfg, int shareFd)
{
	if(pCfg == NULL)
		return;

	priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;
	int i, ret;

	for(i = 0; i < pDec->h264DecOutBufCount; i++){
		if(pDec->mmap_virtual_output_sharedFDs[i] == shareFd)
			break;
	}

	if( i < pDec->h264DecOutBufCount){
		struct v4l2_buffer qbuf;
		struct v4l2_plane planes[VIDEO_MAX_PLANES];

		memset(&qbuf, 0, sizeof(struct v4l2_buffer));
		memset (&planes, 0, sizeof (planes));

		qbuf.index = i;
		qbuf.m.planes = planes;
		qbuf.m.planes[0].bytesused = 0;
		qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		qbuf.memory = V4L2_MEMORY_MMAP;

		ret = SynaV4L2_Ioctl(pDec->hVideoH264Decoder, VIDIOC_QBUF, &qbuf);
		if (ret != V4L2_SUCCESS) {
			LOG_ERR("ERROR: VIDIOC_QBUF, enQueueAMPBuffer Failed\n");
		}
	}else{
		LOG_ERR("enqueBufFromUI bo_name(%d) not found\n", shareFd);
	}
}
