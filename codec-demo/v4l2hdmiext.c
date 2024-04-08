#include "v4l2hdmiext.h"

void
GetHdmiRxExtVideoConnStatus (int *status)
{
	*status = 1;
}

void
RetrieveHdmiRxExtVideoInfo ()
{
	return;
}

int
OpenHdmiRxExt (enc_config_t * pCfg)
{
	int ret = 0;
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;

	ret = SynaV4L2_Open ((void *) &(pEnc->hdmi_rx_handle), HDMI_RX_EXT,
	pEnc->hdmi_rx_v4l2_fmt, O_NONBLOCK);
	if (ret != 0) {
		LOG_ERR ("SynaV4L2_Open failed for HDMI_RX_EXT (%d)\n", ret);
	} else {
		LOG_DEF ("SynaV4L2_Open Success for HDMI_RX_EXT\n");
	}

	return ret;
}

int
SetHdmiRxExtOutputFormat (enc_config_t * pCfg)
{
	struct v4l2_format fmt;
	int ret = 0;
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;
	V4L2_HANDLE handle = pEnc->hdmi_rx_handle;
	LOG_DEF (">>>>>Fn(%s)\n", __func__);

	if (handle) {
		//Set Format for output
		memset (&fmt, 0, sizeof (fmt));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		fmt.fmt.pix_mp.width = pCfg->width;
		fmt.fmt.pix_mp.height = pCfg->height;
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
		LOG_DEF ("In Fn(%s): Setting width(%d) height(%d) pixelformat(%d)\n",
			__func__, fmt.fmt.pix_mp.width, fmt.fmt.pix_mp.height,
			fmt.fmt.pix_mp.pixelformat);

		ret = SynaV4L2_Ioctl (handle, VIDIOC_S_FMT, &fmt);
		if (ret != V4L2_SUCCESS) {
			LOG_DEF ("Error: %s: Output VIDIOC_S_FMT failed\n", __func__);
			goto error;
		}

		memset (&fmt, 0, sizeof (fmt));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		ret = SynaV4L2_Ioctl (handle, VIDIOC_G_FMT, &fmt);
		if (ret != V4L2_SUCCESS) {
			LOG_DEF ("Error: %s: Output VIDIOC_G_FMT failed\n", __func__);
		}
	} else {
		LOG_ERR ("Error: The HDMI-Rx-EXT handle is not configured\n");
		ret = -1;
	}
  error:
	LOG_DEF ("<<<<<Fn(%s)\n", __func__);
	return ret;
}


int
SetupHdmiRxExtOutput (enc_config_t * pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;

	uint32_t type;
	int ret = 0, i;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer qryBuff;
	struct v4l2_plane queryplanes[VIDEO_MAX_PLANES];
	void *mmap_ptr = NULL;

	if (OpenHdmiRxExt (pCfg) == -1) {
		LOG_ERR ("Failed to open HDMI-RX-EXT device \n");
		ret = -1;
		goto err;
	}
	if (SetHdmiRxExtOutputFormat (pCfg) != 0) {
		LOG_ERR ("Failed to SetHdmiRxExtOutputFormat\n");
		ret = -1;
		goto err;
	}

	/* Memory mapping for Hdmi-Rx-Ext CAPTURE buffers */
	memset (&reqbuf, 0, sizeof reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	//reqbuf.count = HDMI_RX_EXT_BUF_COUNT; //Lower layer returns actual numbers of buf count, so passing this arg is absolute.
	if (SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_REQBUFS, &reqbuf) < 0) {
		LOG_ERR ("ERROR: VIDIOC_REQBUFS Failed\n");
		ret = -1;
		goto err;
	}
	LOG_DEF ("VIDIOC_REQBUFS: reqbuf.count(%d)\n", reqbuf.count);

	pEnc->v4l2_enc->mmap_virt_hdmi_out =
		malloc (sizeof (void *) * reqbuf.count);
	pEnc->v4l2_enc->mmap_size_hdmi_out =
		malloc (sizeof (void *) * reqbuf.count);
	pEnc->hdmi_rx_out_buf_count = reqbuf.count;


	for (i = 0; i < reqbuf.count; i++) {

		memset (&qryBuff, 0, sizeof (qryBuff));
		memset (&queryplanes, 0, sizeof (queryplanes));

		qryBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		qryBuff.memory = V4L2_MEMORY_MMAP;
		qryBuff.m.planes = queryplanes;
		qryBuff.index = i;
		ret = SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_QUERYBUF, &qryBuff);
		if (ret != 0) {
			LOG_ERR ("VIDIOC_QUERYBUF failed = %d @ %d\n", ret, __LINE__);
			goto err;
		}

		mmap_ptr =
			SynaV4L2_Mmap (pEnc->hdmi_rx_handle, NULL,
			qryBuff.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED,
			qryBuff.m.planes[0].m.mem_offset);
		if (mmap_ptr == NULL) {
			LOG_ERR ("Fn(%s): SynaV4L2_Mmap FAILED\n", __func__);
			ret = -1;
			goto err;
		} else {
			LOG_DEF ("%s: VIDIOC_QUERYBUF: for index(%d) VA(%p) Len(%d)\n",
				__func__, i, mmap_ptr, qryBuff.m.planes[0].length);
			pEnc->v4l2_enc->mmap_virt_hdmi_out[i] = mmap_ptr;
			pEnc->v4l2_enc->mmap_size_hdmi_out[i] = qryBuff.m.planes[0].length;
		}
	}

	/* Start stream on CAPTURE */
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_STREAMON, &type) < 0) {
		LOG_ERR ("ERROR: Stream ON filed on input\n");
		ret = -1;
	}

  err:
	return ret;
}

int
ReadHdmiRxExtFrame (enc_config_t * pCfg, int *fb_index, int *size)
{
	int ret = -1;
	int v4l2_index;
	struct v4l2_buffer buf;
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;
	LOG_FUNC (">>>>>Fn(%s)\n", __func__);

	memset (&buf, 0, sizeof buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	buf.memory = V4L2_MEMORY_MMAP;


	ret = SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_DQBUF, &buf);
	if (ret != 0) {
		LOG_FUNC ("HDMI ERROR: VIDIOC_DQBUF Failed\n");
		usleep (10000);
	} else {
		LOG_FUNC ("VIDIOC_DQBUF: buf.index(%d) buf.bytesused(%d)\n", buf.index,
			buf.bytesused);
		*fb_index = buf.index;
		*size = buf.bytesused;

		//dump HDMI RX data to a file
		// Push the above data to Encoder as well
		ret = v4l2enc_get_input_handle (pCfg, &v4l2_index);
		// TODO: Optimize the memcpy
		if (ret == 0) {
			memcpy (pEnc->v4l2_enc->mmap_virt_inp[v4l2_index],
				pEnc->v4l2_enc->mmap_virt_hdmi_out[buf.index], *size);
			ret = v4l2h264enc_encode (*size, v4l2_index, pEnc->v4l2_enc, pCfg);
			if (ret != 0 && ret != ERROR_EAGAIN) {
				LOG_ERR ("Encoding failed..\n");
			} else if (ret == ERROR_EAGAIN) {
				LOG_FUNC ("Encode failed with ERROR_EAGAIN\n");
			}
		}else{
			LOG_ERR ("v4l2enc_get_input_handle ret(%d)..\n", ret);
		}
		//enqueue the buffer back
		buf.bytesused = 0;
		ret = SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_QBUF, &buf);
		if (ret != 0) {
			LOG_ERR ("ERROR: VIDIOC_QBUF Failed \n");
		}
	}
	LOG_FUNC ("<<<<<Fn(%s)\n", __func__);
	return ret;
}

int
EnqueHdmiRxExtBuffer (enc_config_t * pCfg, int index)
{
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;

	struct v4l2_buffer qbuf;
	memset (&qbuf, 0, sizeof qbuf);

	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	qbuf.memory = V4L2_MEMORY_MMAP;
	qbuf.index = index;
	qbuf.m.planes[0].bytesused = 0;     /* enqueue it with no data */

	ret = SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_QBUF, &qbuf);
	if (ret != 0) {
		LOG_ERR ("ERROR: VIDIOC_QBUF Failed \n");
		return -1;
	}
	return 0;
}

void
StopHdmiRxExt (enc_config_t * pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;
	int ret = -1;
	int index;
	enum v4l2_buf_type buf_type;

	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret =
		SynaV4L2_Ioctl (pEnc->hdmi_rx_handle, VIDIOC_STREAMOFF,
		(void *) &buf_type);
	if (ret != 0) {
		LOG_ERR ("VIDIOC_STREAMOFF failed = %d @ %d\n", ret, __LINE__);
	}
	//munmap for output
	for (index = 0; index < HDMI_RX_EXT_BUF_COUNT; index++) {
		if (0 != SynaV4L2_Munmap (pEnc->hdmi_rx_handle,
				pEnc->v4l2_enc->mmap_virt_hdmi_out[index],
				pEnc->v4l2_enc->mmap_size_hdmi_out[index])) {
			LOG_ERR ("Output Munmap failed for index: %d\n", index);
		}
	}

	free (pEnc->v4l2_enc->mmap_virt_hdmi_out);
	pEnc->v4l2_enc->mmap_virt_hdmi_out = NULL;
	free (pEnc->v4l2_enc->mmap_size_hdmi_out);
	pEnc->v4l2_enc->mmap_size_hdmi_out = NULL;
	if (NULL != pEnc->hdmi_rx_handle) {
		SynaV4L2_Close (pEnc->hdmi_rx_handle);
		pEnc->hdmi_rx_handle = NULL;
	}
}

void *
hdmiRxExtMonitorThreadFunc (void *data)
{
	enc_config_t *pCfg = (enc_config_t *) data;
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;
	pEnc->hdmiRxStatus = 0;
	pEnc->hdmi_rx_streaming = 0;
	enum v4l2_buf_type buf_type;
	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

	LOG_DEF ("In %s:Thread Created for HDMI Event Handling!!!!!!!!\n",
		__func__);

	tsem_down (pEnc->hdmi_rx_start_sem);

	while (pEnc->is_running == 1) {
		int new_status;
		usleep (50000);
		GetHdmiRxExtVideoConnStatus (&new_status);

		if (new_status != pEnc->hdmiRxStatus) {
			LOG_DEF ("hdmiRxExtMonitorThreadFunc status changed to [%d] \n",
				new_status);
			pEnc->hdmiRxStatus = new_status;

			if (new_status == 1) {
				//RetrieveHdmiRxExtVideoInfo();
				int ret = SetupHdmiRxExtOutput (pCfg);
				if (ret) {
					LOG_ERR ("In %s :unable to create thread for HDMI-in\n",
						__func__);
				} else {
					pEnc->hdmi_rx_streaming = 1;
					LOG_DEF ("In %s: Set hdmi_rx_streaming(%d)\n", __func__,
						pEnc->hdmi_rx_streaming);
				}
			} else {
				if (pEnc->hdmi_rx_streaming == 1) {
					pEnc->hdmi_rx_streaming = 0;
				}
			}
		}
	}

	if (pEnc->hdmi_rx_streaming == 1) {
		pEnc->hdmi_rx_streaming = 0;
	}

	LOG_DEF ("Out of HDMI Event\n");
	return NULL;
}

int
SetupHdmiRxExt (enc_config_t * pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;

	pEnc->hdmi_rx_start_sem = malloc (sizeof (tsem_t));
	tsem_init (pEnc->hdmi_rx_start_sem, 0);

	int ret =
		pthread_create (&pEnc->hdmiRxMonitorThread, NULL,
		hdmiRxExtMonitorThreadFunc, (void *) pCfg);
	if (ret) {
		LOG_ERR ("In %s :unable to create thread for HDMI-in Monitor\n",
			__func__);
		return -1;
	}
}

int
ShutdownHdmiRxExt (enc_config_t * pCfg)
{
	priv_encoder_t *pEnc = (priv_encoder_t *) pCfg->priv;
	if (!pEnc->hdmiRxMonitorThread) {
		pthread_join (pEnc->hdmiRxMonitorThread, NULL);
		pEnc->hdmiRxMonitorThread = NULL;
	}

	if (NULL != pEnc->hdmi_rx_start_sem) {
		free (pEnc->hdmi_rx_start_sem);
		pEnc->hdmi_rx_start_sem = NULL;
	}
}
