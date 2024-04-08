#include "v4l2hdmi.h"

//pthread_t    v4l2_event_watcher_id;

#if 0
void *v4l2_hdmi_event_watcher(void *data)
{
	int ret = 0;
	struct v4l2_event dqevent;
	enc_config_t *pCfg = (enc_config_t *) data;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	while(pEnc->is_running)
	{
		memset(&dqevent, 0, sizeof(struct v4l2_event));
		ret = SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_DQEVENT, &dqevent);
		if (ret != V4L2_SUCCESS)
		{
			usleep(10000);
			continue;
		}
		LOG_DEF("%s: %d VIDIOC_DQEVENT: %d\n", __func__,__LINE__, dqevent.type);
		tsem_up(pEnc->videoH264EncoderEventSem);
	}
}
#endif

void GetHdmiInVideoConnStatus(int *status)
{
	*status = 1;
}

void RetrieveHdmiRxVideoInfo()
{
	return;
}

int OpenHdmiIn(enc_config_t *pCfg)
{
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	ret = SynaV4L2_Open((void *)&(pEnc->hdmi_rx_handle), HDMI_RX, pEnc->hdmi_rx_v4l2_fmt, O_NONBLOCK);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Open failed = %d\n", ret);
		return -1;
	}
	else
	{
		LOG_DEF("SynaV4L2_Open Success for HDMI_RX\n");
	}

	return 0;
}

int SetHdmiInFormat(enc_config_t *pCfg)
{
	struct v4l2_format fmt;
	int ret = 0;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	if(NULL != pEnc->hdmi_rx_handle) {
		memset(&fmt, 0, sizeof fmt);
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		if (pCfg->src == HDMI_UYVY)
		{
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
		}
		else
		{
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
		}
		fmt.fmt.pix.width = pCfg->width;
		fmt.fmt.pix.height = pCfg->height;
		//LOG_DEF("Set HDMI FMT : %s\n", ((pEnc->hdmi_rx_v4l2_fmt == V4L2_PIX_FMT_UYVY) ? "V4L2_PIX_FMT_UYVY" : ((pEnc->hdmi_rx_v4l2_fmt == V4L2_PIX_FMT_NV12) ? "pEnc->hdmi_rx_v4l2_fmt == V4L2_PIX_FMT_NV12" : "FORMAT_UNKNOWN")));

		ret = SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_S_FMT , &fmt);
		if(ret != 0){
			LOG_ERR("Error: VIDIOC_S_FMT has failed \n");
			goto err;
		}
	}else{
		LOG_ERR("The HDMI-in handle is not configured \n");
		goto err;
	}
	return 0;
err:
	return -1;
}

int SetupHdmiRxOutput(enc_config_t *pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	struct v4l2_streamparm streamParams;
	uint32_t type, ret;
	struct v4l2_format avin_ingfmt;
	struct v4l2_buffer qryBuff;
	struct v4l2_buffer qbuf;
	struct v4l2_plane queryplanes[VIDEO_MAX_PLANES];
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_event_subscription subEvent;
	void *mmap_ptr = NULL;

	int i;

	if(OpenHdmiIn(pCfg) == -1){
		LOG_ERR("Failed to open HDMI-in device \n");
		goto err;
	}
	if(SetHdmiInFormat(pCfg) != 0){
		LOG_ERR("Failed to set HDMI format \n");
		//goto err;
	}

	memset (&avin_ingfmt, 0, sizeof avin_ingfmt);
	avin_ingfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_G_FMT, (void*) &avin_ingfmt) < 0)
	{
		LOG_ERR("SynaV4L2_Ioctl for Capture VIDIOC_G_FMT failed\n");
	}

	//subscribe events to receive from libsynav4l2 layer
#if 0
	memset(&subEvent, 0, sizeof subEvent);
	subEvent.type = V4L2_EVENT_FRAME_SYNC;
	ret = SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_SUBSCRIBE_EVENT, &subEvent);
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR ("%s: VIDIOC_SUBSCRIBE_EVENT failed\n", __func__ );
		return -1;
	}

	//pthread_create(&v4l2_event_watcher_id, NULL, v4l2_hdmi_event_watcher, (void *)pCfg);
	//tsem_down(pEnc->videoH264EncoderEventSem);
#endif

	/* Memory mapping for input buffers in V4L2 */
	memset (&reqbuf, 0, sizeof reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbuf.count = HDMI_RX_IN_BUF_COUNT;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if (SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_REQBUFS, &reqbuf) < 0) {
		LOG_ERR("ERROR: VIDIOC_REQBUFS Failed\n");
		goto err;
	}

	pEnc->v4l2_enc->mmap_virt_hdmi_out = malloc (sizeof (void *) * reqbuf.count);
	pEnc->v4l2_enc->mmap_size_hdmi_out = malloc (sizeof (void *) * reqbuf.count);
	pEnc->hdmi_rx_out_buf_count = reqbuf.count;

	for(i = 0; i < reqbuf.count; i++)
	{
		memset (&qryBuff, 0, sizeof (qryBuff));
		qryBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		qryBuff.memory = V4L2_MEMORY_MMAP;
		qryBuff.m.planes = queryplanes;
		qryBuff.index = i;

		ret = SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_QUERYBUF, &qryBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QUERYBUF failed = %d @ %d\n", ret, __LINE__);
			goto err;
		}

		mmap_ptr = SynaV4L2_Mmap(pEnc->hdmi_rx_handle, NULL, qryBuff.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, qryBuff.m.planes[0].m.mem_offset);
		if (mmap_ptr == NULL)
		{
			LOG_ERR("SynaV4L2_Mmap FAILED\n");
			goto err;
		}

		pEnc->v4l2_enc->mmap_virt_hdmi_out[i] = mmap_ptr;
		pEnc->v4l2_enc->mmap_size_hdmi_out[i] = qryBuff.m.planes[0].length;
		//LOG_ERR("Bufflength [%d] = %d\n", i, qryBuff.m.planes[0].length);
		//LOG_ERR("CODEC-DEMO: VA[%d] : %p\n", i, pEnc->v4l2_enc->mmap_virt_hdmi_out[i]);

		// Queue empty buffers
		qbuf = qryBuff;
		qbuf.m.planes[0].bytesused = 0;
		qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

		if (SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_QBUF, &qbuf) < 0) {
			LOG_ERR("ERROR: VIDIOC_QBUF Failed :%d and %s:\n",errno,strerror(errno));
			goto err;
		}
	}

	/* Start stream on input */
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if(SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_STREAMON, &type) < 0) {
		LOG_ERR("ERROR: Stream ON filed on input\n");
		goto err;
	}

	return 0;
err:
	return -1;
}

int ReadHdmiRxFrame(enc_config_t *pCfg, int* fb_index, int* size)
{
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	struct v4l2_buffer dqbuf;
	int index  = -1;
	memset (&dqbuf, 0, sizeof dqbuf);
	dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	dqbuf.memory = V4L2_MEMORY_MMAP;

	struct v4l2_buffer qbuf;
	memset (&qbuf, 0, sizeof qbuf);

	ret = SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_DQBUF, &dqbuf);
	if (ret != 0) {
		//LOG_DEF("HDMI ERROR: VIDIOC_DQBUF Failed \n");
		usleep(10000);
		return ret;
	}else {
		int v4l2_index;
		index = dqbuf.index;
		*fb_index = index;
		*size  = dqbuf.m.planes[0].bytesused;

		//dump HDMI RX data to a file
#if 0
		static int iCnt = 0;
		if (iCnt < 120)
		{
			FILE *fp;
			LOG_ERR("HDMI WRITE of size#: %d VA: %p\n", *size, pEnc->v4l2_enc->mmap_virt_hdmi_out[index]);
			fp = fopen("/home/root/hdmi_rx_output.raw","ab");
			if (NULL != fp)
			{
				char *buf = malloc((*size));
				memcpy(buf, pEnc->v4l2_enc->mmap_virt_hdmi_out[index], (*size));
				fwrite(buf, 1, (*size), fp);
				perror("fwrite");
				fclose(fp);
				fp = NULL;
			}
			iCnt++;
		}
#else
		// Push the above data to Encoder as well
		ret = v4l2enc_get_input_handle(pCfg, &v4l2_index);

		// Below memcpy is done so as to copy the data from the AVIN component
		// to Encoder component due to the limitation of AVIN component in
		// TUNNEL mode
		if (ret == 0)
		{
			memcpy(pEnc->v4l2_enc->mmap_virt_inp[v4l2_index], pEnc->v4l2_enc->mmap_virt_hdmi_out[index],*size);
			ret = v4l2h264enc_encode (*size, v4l2_index, pEnc->v4l2_enc, pCfg);
			if (ret != 0 && ret != ERROR_EAGAIN) {
				LOG_ERR("Encoding failed..\n");
			}
			else if (ret == ERROR_EAGAIN) {
				//LOG_ERR("Encode failed with ERROR_EAGAIN\n");
			}
		}
#endif

		//enqueue the buffer back
		qbuf = dqbuf;
		qbuf.m.planes[0].bytesused = 0;
		ret = SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_QBUF, &qbuf);
		if (ret != 0) {
			LOG_ERR("ERROR: VIDIOC_QBUF Failed \n");
			return ret;
		}
	}
	return 0;
}

int EnqueHdmiRxBuffer(enc_config_t *pCfg, int index)
{
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	struct v4l2_buffer qbuf;
	memset (&qbuf, 0, sizeof qbuf);

	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	qbuf.memory = V4L2_MEMORY_MMAP;
	qbuf.index = index;
	qbuf.m.planes[0].bytesused = 0;         /* enqueue it with no data */

	ret = SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_QBUF, &qbuf);
	if (ret != 0) {
		LOG_ERR("ERROR: VIDIOC_QBUF Failed \n");
		return -1;
	}
	return 0;
}

void StopHdmiRX(enc_config_t *pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	int ret = -1;
	int index;
	enum v4l2_buf_type buf_type;

	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = SynaV4L2_Ioctl(pEnc->hdmi_rx_handle, VIDIOC_STREAMOFF, (void*) &buf_type);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_STREAMOFF failed = %d @ %d\n", ret, __LINE__);
	}
	//munmap for output
	for (index = 0; index < HDMI_RX_IN_BUF_COUNT; index++)
	{
		if (0 != SynaV4L2_Munmap(pEnc->hdmi_rx_handle, pEnc->v4l2_enc->mmap_virt_hdmi_out[index], pEnc->v4l2_enc->mmap_size_hdmi_out[index]))
		{
			LOG_ERR("Output Munmap failed for index: %d\n", index);
		}
	}

	free (pEnc->v4l2_enc->mmap_virt_hdmi_out);
	pEnc->v4l2_enc->mmap_virt_hdmi_out = NULL;
	free (pEnc->v4l2_enc->mmap_size_hdmi_out);
	pEnc->v4l2_enc->mmap_size_hdmi_out = NULL;
	if(NULL != pEnc->hdmi_rx_handle)
	{
		SynaV4L2_Close(pEnc->hdmi_rx_handle);
		pEnc->hdmi_rx_handle = NULL;
	}
}

void *hdmiRxMonitorThreadFunc (void *data)
{
	enc_config_t *pCfg = (enc_config_t *)data;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	pEnc->hdmiRxStatus = 0;
	pEnc->hdmi_rx_streaming = 0;
	enum v4l2_buf_type buf_type;
	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	LOG_DEF("In %s:Thread Created for HDMI Event Handling!!!!!!!!\n",__func__);

	tsem_down(pEnc->hdmi_rx_start_sem);

	while(pEnc->is_running == 1)
	{
		int new_status;
		usleep(50000);
		GetHdmiInVideoConnStatus(&new_status);

		if (new_status != pEnc->hdmiRxStatus) {
			LOG_DEF("hdmiRxMonitorThreadFunc status changed to [%d] \n", new_status);
			pEnc->hdmiRxStatus = new_status;

			if (new_status == 1)
			{
				RetrieveHdmiRxVideoInfo();
				int ret = SetupHdmiRxOutput(pCfg);
				if(ret) {
					LOG_ERR("In %s :unable to create thread for HDMI-in\n",__func__);
				}
				else
				{
					pEnc->hdmi_rx_streaming = 1;
				}
			}
			else
			{
				if (pEnc->hdmi_rx_streaming == 1)
				{
					pEnc->hdmi_rx_streaming = 0;
				}
			}
		}
	}

	if (pEnc->hdmi_rx_streaming == 1)
	{
		pEnc->hdmi_rx_streaming = 0;
	}

	LOG_DEF("Out of HDMI Event\n");
	return NULL;
}

int SetupHdmiRx(enc_config_t *pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

	pEnc->hdmi_rx_start_sem = malloc(sizeof(tsem_t));
	tsem_init(pEnc->hdmi_rx_start_sem, 0);

	int ret = pthread_create(&pEnc->hdmiRxMonitorThread, NULL, hdmiRxMonitorThreadFunc, (void *)pCfg);
	if(ret) {
		LOG_ERR("In %s :unable to create thread for HDMI-in Monitor\n",__func__);
		return -1;
	}
}

int ShutdownHdmiRx(enc_config_t *pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	if (!pEnc->hdmiRxMonitorThread)
	{
		pthread_join(pEnc->hdmiRxMonitorThread, NULL);
		pEnc->hdmiRxMonitorThread = NULL;
	}
	//pthread_join(v4l2_event_watcher_id, NULL);

	if (NULL != pEnc->hdmi_rx_start_sem)
	{
		free(pEnc->hdmi_rx_start_sem);
		pEnc->hdmi_rx_start_sem = NULL;
	}
}
