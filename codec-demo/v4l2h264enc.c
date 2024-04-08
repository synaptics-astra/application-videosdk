#include "v4l2h264enc.h"
//#define DEBUG_V4L2ENC
int setupEncEXt(enc_config_t *pCfg)
{
	int ret =0;
	pCfg->width = ROUNDUP16(pCfg->width);
	pCfg->height = ROUNDUP16(pCfg->height);

	LOG_ERR("%s done \n",__func__);
	return ret;
}
int v4l2enc_test_streamon (void *handle, enum v4l2_buf_type buf_type)
{
	int ret = -1;
	ret = SynaV4L2_Ioctl(handle, VIDIOC_STREAMON, (void*) &buf_type);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_STREAMON failed = %d @ %d\n", ret, __LINE__);
	}
	return ret;
}

int v4l2enc_setup_input(enc_config_t *pCfg)
{
	struct v4l2_requestbuffers rqBuff;
	struct v4l2_buffer qryBuff, queBuff;
	int i, pixelType;
	void *mmap_ptr = NULL;
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2enc = pEnc->v4l2_enc;

	memset (&rqBuff, 0, sizeof (rqBuff));

	rqBuff.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	rqBuff.count = NB_BUF_INPUT;
	rqBuff.memory = V4L2_MEMORY_MMAP;

	ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_REQBUFS, (void*) &rqBuff);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_REQBUFS failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	v4l2enc->mmap_virt_inp = malloc (sizeof (void *) * rqBuff.count);
	v4l2enc->mmap_size_inp = malloc (sizeof (void *) * rqBuff.count);
	pEnc->h264EncInBufCount = rqBuff.count;

	v4l2enc->session_active = 1;
	for(i = 0; i < rqBuff.count; i++){
		memset (&qryBuff, 0, sizeof (qryBuff));
		qryBuff.index = i;
		qryBuff.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		qryBuff.memory = V4L2_MEMORY_MMAP;
		qryBuff.m.planes = (struct v4l2_plane *) malloc(sizeof(struct v4l2_plane) * VIDEO_MAX_PLANES);;

		ret = SynaV4L2_Ioctl (v4l2enc->handle, VIDIOC_QUERYBUF, &qryBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QUERYBUF failed = %d @ %d\n", ret, __LINE__);
			return -1;
		}

		pEnc->pH264EncInV4l2Buf[i] = qryBuff;
		//LOG_DEF("qryBuff.length = %d\t\tqryBuff.m.offset = %x\n", qryBuff.m.planes[0].length, qryBuff.m.planes[0].m.mem_offset);
		mmap_ptr = SynaV4L2_Mmap(v4l2enc->handle, NULL, qryBuff.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, qryBuff.m.planes[0].m.mem_offset);
		//LOG_DEF("ret for SynaV4L2_Mmap = %p\n", mmap_ptr);
		if (mmap_ptr == NULL)
		{
			LOG_ERR("SynaV4L2_Mmap FAILED\n");
			return -1;
		}

		/*Get shared_fd*/
		if(pCfg->display_window){
			SynaV4L2_GetSharedFD(v4l2enc->handle,
					qryBuff.m.planes[0].m.mem_offset,
					V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE,
					&pEnc->mmap_virtual_input_sharedFDs[i]);
			LOG_DEF("SYNA_DEBUG: Encoder mmap_virtual_input_sharedFD(%d)\n", pEnc->mmap_virtual_input_sharedFDs[i]);
		}

		v4l2enc->mmap_virt_inp[i] = mmap_ptr;
		v4l2enc->mmap_size_inp[i] = qryBuff.m.planes[0].length;
		pEnc->bH264InBufWithEnc[i] = false;
		pEnc->bH264InBufWithQtRdS[i] = false;

#if 0
		/* Queue empty V4L2 buffer */
		queBuff = qryBuff;
		queBuff.m.planes[0].bytesused = v4l2enc->frame_read_size;
		ret = SynaV4L2_Ioctl (v4l2enc->handle, VIDIOC_QBUF, &queBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QBUF failed = %d @ %d\n", ret, __LINE__);
			return -1;
		}
#endif
	}

	/*Send Init msg to UI about this ARGB window */
	if(pCfg->display_window) {
		if ((pCfg->src == CAM_YUYV) || (pCfg->src == FILE_YUYV))
		{
			pixelType = PIXEL_YUYV;
			SendInitMessageToUI(SESS_AMPENC,
					pCfg,
					(pCfg->display_window - 1),
					rqBuff.count,
					pCfg->width,
					pCfg->height,
					pixelType);
		}
		else if (pCfg->src == HDMI_UYVY)
		{
			pixelType = PIXEL_UYVY;
			SendInitMessageToUI(SESS_AMPENC,
					pCfg,
					(pCfg->display_window - 1),
					rqBuff.count,
					pCfg->width,
					pCfg->height,
					pixelType);
		}
		else
		{
			pixelType = PIXEL_NV12;
			SendInitMessageToUI(SESS_AMPENC,
					pCfg,
					(pCfg->display_window - 1),
					rqBuff.count,
					pCfg->width,
					pCfg->height,
					pixelType);
		}
	}

	return ret;
}

int v4l2enc_setup_output(struct v4l2_enc *v4l2enc)
{
	struct v4l2_requestbuffers rqBuff;
	int i;
	struct v4l2_buffer qryBuff, queBuff;
	void *mmap_ptr = NULL;
	int ret = -1;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];

	memset (&planes, 0, sizeof (planes));
	int req_out_buff;

	memset (&rqBuff, 0, sizeof (rqBuff));
	rqBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
//	rqBuff.count = NB_BUF_OUTPUT;
	rqBuff.memory = V4L2_MEMORY_MMAP;

	switch(v4l2enc->simulcast->simulcast_num_total)
	{
		case 1:
			rqBuff.count = NB_BUF_OUTPUT*2;
			break;
		case 2:
			rqBuff.count = NB_BUF_OUTPUT*3;
			break;
		default:
			rqBuff.count = NB_BUF_OUTPUT;
			break;
	}
	req_out_buff = rqBuff.count;

	ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_REQBUFS, (void*) &rqBuff);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_REQBUFS failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	v4l2enc->out_buf_count = rqBuff.count;
	if(v4l2enc->out_buf_count < req_out_buff)
	{
		LOG_ERR("Requested buf is %d but got only %d Returning from %s",req_out_buff,v4l2enc->out_buf_count,__func__);
		return -1;
	}
	LOG_ERR("Requested buf is %d And %d Returning from %s",req_out_buff,v4l2enc->out_buf_count,__func__);
	//check with req buff

	v4l2enc->mmap_virt_out = malloc (sizeof (void *) * rqBuff.count);
	v4l2enc->mmap_size_out = malloc (sizeof (void *) * rqBuff.count);

	memset (&qryBuff, 0, sizeof (qryBuff));
	qryBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	qryBuff.memory = V4L2_MEMORY_MMAP;
	qryBuff.m.planes = planes;

	for(i = 0; i < rqBuff.count; i++){
		qryBuff.index = i;
		qryBuff.flags = 0;

		if(v4l2enc->simulcast->simulcast_num_total) {
			if((qryBuff.index % (v4l2enc->simulcast->simulcast_num_total+1)) == 2 )
				qryBuff.flags |= V4L2_BUF_FLAG_VENC_OUTPUT_PORT_SIMULCAST_CH2;
			else if ((qryBuff.index % (v4l2enc->simulcast->simulcast_num_total+1)) == 1)
				qryBuff.flags |= V4L2_BUF_FLAG_VENC_OUTPUT_PORT_SIMULCAST_CH1;
			else
				qryBuff.flags |= V4L2_BUF_FLAG_VENC_OUTPUT_PORT_INDEX;
		}
		else
			qryBuff.flags |= V4L2_BUF_FLAG_VENC_OUTPUT_PORT_INDEX;

		ret = SynaV4L2_Ioctl (v4l2enc->handle, VIDIOC_QUERYBUF, &qryBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QUERYBUF failed = %d @ %d\n", ret, __LINE__);
			return -1;
		}

		//LOG_DEF("qryBuff.length = %d\t\tqryBuff.m.offset = %x\n", qryBuff.m.planes[0].length, qryBuff.m.planes[0].m.mem_offset);
		mmap_ptr = SynaV4L2_Mmap(v4l2enc->handle, NULL, qryBuff.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, qryBuff.m.planes[0].m.mem_offset);
		//LOG_DEF("ret for SynaV4L2_Mmap = %p\n", mmap_ptr);
		if (mmap_ptr == NULL)
		{
			LOG_ERR("SynaV4L2_Mmap FAILED\n");
			return -1;
		}

		v4l2enc->mmap_virt_out[i] = mmap_ptr;
		v4l2enc->mmap_size_out[i] = qryBuff.m.planes[0].length;

		// Queue empty buffers
		queBuff = qryBuff;
		queBuff.m.planes[0].bytesused = 0;
		queBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		ret = SynaV4L2_Ioctl (v4l2enc->handle, VIDIOC_QBUF, &queBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QBUF failed = %d @ %d\n", ret, __LINE__);
			return -1;
		}
	}

	return ret;
}

void setProfile(enc_config_t *pCfg, int *profile)
{
	if (strcmp(pCfg->profile, "MAIN") == 0) {
		*profile = V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;
	}
	else if (strcmp(pCfg->profile, "HIGH") == 0) {
		*profile = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
	}
	else if (strcmp(pCfg->profile, "BASELINE") == 0) {
		*profile = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
	}
	else {
		// default to High Profile
		*profile = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
	}
}

void setLevel(enc_config_t *pCfg, int *level)
{
	if (strcmp(pCfg->level, "1.0") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_1_0;
	}
	else if (strcmp(pCfg->level, "1.1") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_1_1;
	}
	else if (strcmp(pCfg->level, "1.2") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_1_2;
	}
	else if (strcmp(pCfg->level, "1.3") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_1_3;
	}
	else if (strcmp(pCfg->level, "2.0") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_2_0;
	}
	else if (strcmp(pCfg->level, "2.1") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_2_1;
	}
	else if (strcmp(pCfg->level, "2.2") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_2_2;
	}
	else if (strcmp(pCfg->level, "3.0") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_3_0;
	}
	else if (strcmp(pCfg->level, "3.1") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
	}
	else if (strcmp(pCfg->level, "3.2") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_3_2;
	}
	else if (strcmp(pCfg->level, "4.0") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_4_0;
	}
	else if (strcmp(pCfg->level, "4.1") == 0) {
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
	}
	else {
		// TODO : Supported Upto Level 4.2 in case of VS680 - Handle the same
		// For now default to 4.1 Level
		*level = V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
	}
}

int v4l2enc_set_formats(enc_config_t *pCfg)
{
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2enc = pEnc->v4l2_enc;
	int width, height, fps, bitrate;
	int enc_profile, enc_level;
	struct v4l2_format v4l2enc_infmt;
	struct v4l2_format v4l2enc_ingfmt;
	struct v4l2_format v4l2enc_outfmt;
	struct v4l2_format v4l2enc_outgfmt;
	enum v4l2_buf_type buf_type;
	struct v4l2_streamparm param;
	struct v4l2_ext_controls *ext_controls;
	struct v4l2_ext_control *ext_control;

	width = pCfg->width;
	height = pCfg->height;
	fps = pCfg->fps;
	bitrate = pCfg->bitrate;

	// set the right profile value
	setProfile(pCfg, &enc_profile);

	// set the right level value
	setLevel(pCfg, &enc_level);

	ext_controls = (struct v4l2_ext_controls *)malloc(sizeof(struct v4l2_ext_controls));
	memset (ext_controls, 0, sizeof (struct v4l2_ext_controls));
	ext_controls->controls = (struct v4l2_ext_control *)malloc(ENC_EXT_CONTROLS_PARAM_NUM * sizeof(struct v4l2_ext_control));

	// Set the initial configured Bit-rate
	ext_controls->count++;
	ext_control = &ext_controls->controls[0];
	ext_control->id = V4L2_CID_MPEG_VIDEO_BITRATE;
	ext_control->value = (bitrate * 1000); //to set as bits per second

	// Set the configured Profile
	ext_controls->count++;
	ext_control = &ext_controls->controls[1]; // to set the profile
	ext_control->id = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
	ext_control->value = enc_profile;

	// Set the configured Level
	ext_controls->count++;
	ext_control = &ext_controls->controls[2]; // to set the level
	ext_control->id = V4L2_CID_MPEG_VIDEO_H264_LEVEL;
	ext_control->value = enc_level;

	// Set the configured initial QP
	ext_controls->count++;
	ext_control = &ext_controls->controls[3]; // to set the initial QP value
	ext_control->id = V4L2_CID_MPEG_VIDEO_H264_MAX_QP;
	ext_control->value = pCfg->encInitQP;
#if 0
	// Set the Bitrate Mode - CBR / VBR
	ext_control = &ext_controls->controls[1];
	ext_control->id = V4L2_CID_MPEG_VIDEO_BITRATE_MODE;
	ext_control->value = V4L2_MPEG_VIDEO_BITRATE_MODE_CBR;
	//ext_control->value = V4L2_MPEG_VIDEO_BITRATE_MODE_VBR;
#endif
	// set the simulcast

	ext_controls->count++;
	ext_control = &ext_controls->controls[4];
	ext_control->id = V4L2_CID_MPEG_VIDEO_H264_SIMULCAST_ENABLE;
	ext_control->ptr = v4l2enc->simulcast;

	ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_S_EXT_CTRLS, (void*) ext_controls);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Ioctl for Capture VIDIOC_S_EXT_CTRLS failed = %d\n", ret);
		return -1;
	}
	//Output Set Format for Output
	memset (&v4l2enc_infmt, 0, sizeof v4l2enc_infmt);
	v4l2enc_infmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	v4l2enc_infmt.fmt.pix_mp.width = width;
	v4l2enc_infmt.fmt.pix_mp.height = height;
	if ((pCfg->src == CAM_YUYV) || (pCfg->src == FILE_YUYV))
	{
		v4l2enc_infmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUYV;
	}
	else if (pCfg->src == HDMI_UYVY)
	{
		v4l2enc_infmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_UYVY;
	}
	else
	{
		v4l2enc_infmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12;
	}
	ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_S_FMT, (void*) &v4l2enc_infmt);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Ioctl for Output VIDIOC_S_FMT failed = %d\n", ret);
		return -1;
	}

	// Set Frame-rate Param
	memset (&param, 0, sizeof param);
	param.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	param.parm.output.timeperframe.numerator = fps;
	param.parm.output.timeperframe.denominator = 1;
	ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_S_PARM, (void*) &param);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Ioctl for Output VIDIOC_S_PARM failed = %d\n", ret);
		return -1;
	}

	memset (&v4l2enc_ingfmt, 0, sizeof v4l2enc_ingfmt);
	v4l2enc_ingfmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_G_FMT, (void*) &v4l2enc_ingfmt);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Ioctl for Output VIDIOC_G_FMT failed = %d\n", ret);
		return -1;
	}

	buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	ret = v4l2enc_test_streamon (v4l2enc->handle, buf_type);
	if (ret != 0)
	{
		LOG_ERR("v4l2enc_test_streamon failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	//Output Set Format for Capture
	memset (&v4l2enc_outfmt, 0, sizeof v4l2enc_outfmt);
	v4l2enc_outfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	v4l2enc_outfmt.fmt.pix_mp.width = width;
	v4l2enc_outfmt.fmt.pix_mp.height = height;
	v4l2enc_outfmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_S_FMT, (void*) &v4l2enc_outfmt);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Ioctl for Capture VIDIOC_S_FMT failed = %d\n", ret);
		return -1;
	}
	memset (&v4l2enc_outgfmt, 0, sizeof v4l2enc_outgfmt);
	v4l2enc_outgfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_G_FMT, (void*) &v4l2enc_outgfmt);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Ioctl for Capture VIDIOC_G_FMT failed = %d\n", ret);
		return -1;
	}

	if (ext_controls->controls != NULL)
	{
		free (ext_controls->controls);
		ext_controls->controls = NULL;
	}
	if (ext_controls != NULL)
	{
		free (ext_controls);
		ext_controls = NULL;
	}

	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = v4l2enc_test_streamon (v4l2enc->handle, buf_type);
	if (ret != 0)
	{
		LOG_ERR("v4l2enc_test_streamon failed = %d @ %d\n", ret, __LINE__);
	}

	return ret;
}

// Use this function in order to write the encoded output to a FILE at
// /home/root/outLocal.h264
int write_out_data_to_file (void *data_ptr, int data_size, void *cb_data)
{
	//LOG_DEF("Received data from encoder of size = %d\n", data_size);
	if (NULL != cb_data)
	{
		FILE *fpOp = (FILE *)cb_data;
		return fwrite(data_ptr, 1, data_size, fpOp);
	}
	else
		return -1;
}

void v4l2enc_data_out(enc_config_t *pCfg)
{
	struct v4l2_buffer dqbuf;
	struct v4l2_buffer qbuf;
	uint8_t *v4l2_data;
	uint32_t v4l2_size;
	int gstRet = 0;
	int ret = -1;
	int header = 0;
	int i;
	enc_config_t *pCfg_sim[MAX_EXT_ENC_SESSIONS];
	priv_encoder_t *pEnc_sim[MAX_EXT_ENC_SESSIONS];

	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2enc = pEnc->v4l2_enc;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	enc_config_t *ptr = (enc_config_t *)(v4l2enc->encCfg);
	codec_config_t *codec_config = (codec_config_t *)(ptr->codec_config);

	if(ptr->simcast_en) {
		for(i=0; i< codec_config->num_ext_enc_sessions; i++)
		{
			pCfg_sim[i] = (enc_config_t *)codec_config->ext_enc_sess_cfg[i];
			pEnc_sim[i] = pCfg_sim[i]->priv;
		}
	}

	while (1)
	{
try_again2:
		if (v4l2enc->eos)
		{
			return;
		}
		memset (&dqbuf, 0, sizeof dqbuf);
		//memset (&planes, 0, sizeof(planes));

		dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		dqbuf.memory = V4L2_MEMORY_MMAP;
		dqbuf.m.planes = planes;

		//LOG_DEF("waiting for output--->\n");
		ret = SynaV4L2_Ioctl (v4l2enc->handle, VIDIOC_DQBUF, &dqbuf);
		if (ret != 0) {
			//LOG_DEF("Error: VIDIOC_DQBUF Failed, while encoding %d\n", ret);
			if((!v4l2enc->session_active) && !(v4l2enc->eos))
			{
				LOG_DEF("Session is still not active\n");
				usleep(20*1000);
				continue;
			}
			else if (v4l2enc->eos)
			{
				return;
			}
			else if (ret == ERROR_EAGAIN) {
				usleep (10*1000);
				goto try_again2;
			}
		}
		else {
			v4l2_data = v4l2enc->mmap_virt_out[dqbuf.index];
			v4l2_size = dqbuf.m.planes[0].bytesused;

#ifdef LOCAL_LOOPBACK_MODE
			if(pCfg->instance_id == LOCAL_LOOPBACK_ENC_INST){
				/*enque the encoded data here for local loopback only*/
				codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
				queue_element_t* tmpBuf = ( queue_element_t*) malloc(sizeof(queue_element_t));
				if(tmpBuf){
					tmpBuf->nFilledLen = v4l2_size;
					tmpBuf->pBuffer = (uint8_t *)malloc(v4l2_size);
					memcpy(tmpBuf->pBuffer, v4l2_data, v4l2_size);
					int eError = queue(codec_config->sharedH264Queue,tmpBuf);
					if(!eError) {
						LOG_FUNC("In %s :Queueed Encoded data len[%d]\n", __func__, v4l2_size);
						tsem_up(codec_config->sharedH264QueueSem);
					}
				}
			}
#endif
			//LOG_DEF("%s -> length %d -> bytesused %x -> mem_offset %x -> data_offset %x\n", __func__, dqbuf.m.planes[0].length, dqbuf.m.planes[0].bytesused, dqbuf.m.planes[0].m.mem_offset, dqbuf.m.planes[0].data_offset);
			//send access unit to application
			//LOG_DEF("data of %d copied to output mmap memory\n", v4l2_size);
#ifdef DEBUG_V4L2ENC
			if ((v4l2enc->display_window_id == 0) && (v4l2enc->data_cb != NULL))
			{
				if (dqbuf.flags & V4L2_BUF_FLAG_VENC_OUTPUT_PORT_SIMULCAST_CH2) {
					v4l2enc->data_cb(v4l2_data, v4l2_size, v4l2enc->cb_data_sim[1]);
					pEnc_sim[1]->total_bitrate += v4l2_size;
					pEnc_sim[1]->total_frames++;
				}
				else if (dqbuf.flags & V4L2_BUF_FLAG_VENC_OUTPUT_PORT_SIMULCAST_CH1) {
					v4l2enc->data_cb(v4l2_data, v4l2_size, v4l2enc->cb_data_sim[0]);
					pEnc_sim[0]->total_bitrate += v4l2_size;
					pEnc_sim[0]->total_frames++;
				}
				else {
					v4l2enc->data_cb(v4l2_data, v4l2_size, v4l2enc->cb_data);
					pEnc->total_bitrate += v4l2_size;
					pEnc->total_frames++;
				}
			}
			else
#endif
			{
try_again:
#ifdef LOCAL_LOOPBACK_MODE
				if(pCfg->instance_id != LOCAL_LOOPBACK_ENC_INST){
					if(strcmp(pCfg->ip, "0.0.0.0") != 0)
					{
						gst_tx_t *pGstTx = &(pEnc->gstTx);
						gstRet = FeedGstTx(pGstTx, v4l2_data, v4l2_size, v4l2enc->pkt_pts);
						if (gstRet != 0 && (!header))
						{
							usleep(5*1000);
							goto try_again;
						}
						else
						{
							header = 1;
						}
					}
				}
#else
				if (dqbuf.flags & V4L2_BUF_FLAG_VENC_OUTPUT_PORT_SIMULCAST_CH2)
				{
					enc_config_t *pEncCfg = (enc_config_t *)(codec_config->ext_enc_sess_cfg[1]);
					priv_encoder_t *pEncPriv = (priv_encoder_t *)pEncCfg->priv;
					if(strcmp(pCfg->ip, "0.0.0.0") != 0)
					{
						gst_tx_t *pGstTx = &(pEncPriv->gstTx);

						gstRet = FeedGstTx(pGstTx, v4l2_data, v4l2_size, v4l2enc->pkt_pts);
						if (gstRet != 0 && (!header))
						{
							usleep(5*1000);
							goto try_again;
						}
						else
						{
							header = 1;
						}
					}
					pEnc_sim[1]->total_bitrate += v4l2_size;
					pEnc_sim[1]->total_frames++;
				}
				else if (dqbuf.flags & V4L2_BUF_FLAG_VENC_OUTPUT_PORT_SIMULCAST_CH1)
				{
					enc_config_t *pEncCfg = (enc_config_t *)(codec_config->ext_enc_sess_cfg[0]);
					priv_encoder_t *pEncPriv = (priv_encoder_t *)pEncCfg->priv;
					if(strcmp(pCfg->ip, "0.0.0.0") != 0)
					{
						gst_tx_t *pGstTx = &(pEncPriv->gstTx);

						gstRet = FeedGstTx(pGstTx, v4l2_data, v4l2_size, v4l2enc->pkt_pts);
						if (gstRet != 0 && (!header))
						{
							usleep(5*1000);
							goto try_again;
						}
						else
						{
							header = 1;
						}
					}
					pEnc_sim[0]->total_bitrate += v4l2_size;
					pEnc_sim[0]->total_frames++;
				}
				else
				{
					enc_config_t *pEncCfg = (enc_config_t *)(v4l2enc->encCfg);
					priv_encoder_t *pEncPriv = (priv_encoder_t *)pEncCfg->priv;
					if(strcmp(pCfg->ip, "0.0.0.0") != 0)
					{
						gst_tx_t *pGstTx = &(pEncPriv->gstTx);

						gstRet = FeedGstTx(pGstTx, v4l2_data, v4l2_size, v4l2enc->pkt_pts);
						if (gstRet != 0 && (!header))
						{
							usleep(5*1000);
							goto try_again;
						}
						else
						{
							header = 1;
						}
					}
					pEnc->total_bitrate += v4l2_size;
					pEnc->total_frames++;
					v4l2enc->pkt_pts += v4l2enc->pkt_duration;

				}
#endif //LOCAL_LOOPBACK_MODE

			}

			qbuf = dqbuf;			 /* index from querybuf */
			qbuf.m.planes[0].bytesused = 0;			/* enqueue it with no data */
			ret = SynaV4L2_Ioctl (v4l2enc->handle, VIDIOC_QBUF, &qbuf);
			if (ret != 0) {
				LOG_ERR("ERROR: VIDIOC_QBUF Failed during encode process %d\n", ret);
			}
		}
		//usleep (10*1000);
	}
}

int v4l2enc_get_input_handle(enc_config_t *pCfg, int32_t *index)
{
	struct v4l2_buffer dqbuf;
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2enc = pEnc->v4l2_enc;
	int i;
	int j = pEnc->last_index;

	*index = -1;
	for (i = 0; i < pEnc->h264EncInBufCount; i++)
	{
		if(++j == pEnc->h264EncInBufCount)
			j=0;

		if ((pEnc->bH264InBufWithEnc[j] == false) && (pEnc->bH264InBufWithQtRdS[j] == false))
		{
			*index = j;
			pEnc->last_index = j;
			ret = 0;
			return ret;
		}
	}
#if 0
	// Only for debugging - not expected to enter in this flow
	for (i = 0; i < pEnc->h264EncInBufCount; i++)
	{
		LOG_ERR("QTRDS: [%d] = E[%d] Q[%d]\n", i, pEnc->bH264InBufWithEnc[i], pEnc->bH264InBufWithQtRdS[i]);
	}
#endif

	return ret;
}

int v4l2enc_test_encode(uint32_t frame_size, int index, struct v4l2_enc *v4l2enc, enc_config_t *pCfg)
{
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	//LOG_DEF("Inside %s at# %d\n", __func__, __LINE__);

	struct v4l2_buffer qbuf;
	qbuf = pEnc->pH264EncInV4l2Buf[index];
	memset(planes, 0, sizeof(planes));
	qbuf.m.planes = planes;
	qbuf.m.planes[0].bytesused = frame_size;

	pEnc->bH264InBufWithEnc[index] = true;
	ret = SynaV4L2_Ioctl (v4l2enc->handle, VIDIOC_QBUF, &(qbuf));
	if (ret != 0) {
		LOG_ERR("ERROR: VIDIOC_QBUF Failed %d\n", ret);
	}
	if(pCfg->display_window){
		//LOG_ERR("QTRDS: IDX %d\n", index);
		pEnc->bH264InBufWithQtRdS[index] = true;
		sendProcessDMABufId(pCfg->display_window, pEnc->mmap_virtual_input_sharedFDs[index], NULL);
	}
err:
	//LOG_DEF("returning from %s with ret = %d\n", __func__, ret);
	return ret;
}

void *dqbufEnc_output_watcher(void *data)
{
	int ret = -1;
	struct v4l2_buffer dqbuf;
	enc_config_t *pCfg = (enc_config_t *) data;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2enc = pEnc->v4l2_enc;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];

	// Wait for encoder to start
	tsem_down(pEnc->dqbuf_enc_start_sem);

	while(pEnc->is_running)
	{
		memset (&dqbuf, 0, sizeof dqbuf);
		memset (&planes, 0, sizeof(planes));
		dqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		dqbuf.memory = V4L2_MEMORY_MMAP;
		dqbuf.m.planes = planes;

		ret = SynaV4L2_Ioctl(v4l2enc->handle, VIDIOC_DQBUF, &dqbuf);
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
			pEnc->bH264InBufWithEnc[dqbuf.index] = false;
		}
	}
}

int SetupV4L2H264Enc(enc_config_t *pCfg)
{
	int ret = -1;
	int i;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2_enc = pEnc->v4l2_enc;
	int width, height, fps, disp_window;
	v4l2_enc->fpo = NULL;
	v4l2_enc->fpo_sim[0] = NULL;
	v4l2_enc->fpo_sim[1] = NULL;
	codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;

	if(pCfg->src ==ENC_EXT)
	{
		LOG_ERR("No Init required on ENC_EXT since it will be doing in next CAM/FILE ENC session \n");
		return 0;
	}
	pEnc->videoH264EncoderEventSem = malloc(sizeof(tsem_t));
	tsem_init(pEnc->videoH264EncoderEventSem, 0);


	v4l2_enc->simulcast = (v4l2_simulcast *) malloc(sizeof (v4l2_simulcast));
	memset(v4l2_enc->simulcast,0,sizeof(struct _v4l2_simulcast));

	LOG_ERR("encoder simcast enable : %d \n",pCfg->simcast_en);

	if(pCfg->simcast_en)
		v4l2_enc->simulcast->simulcast_num_total = codec_config->num_ext_enc_sessions;

	for ( i=0; i<v4l2_enc->simulcast->simulcast_num_total;i++)
	{
		v4l2_enc->simulcast->sim_width[i] =((enc_config_t *)codec_config->ext_enc_sess_cfg[i])->width;
		v4l2_enc->simulcast->sim_height[i] = ((enc_config_t *) codec_config->ext_enc_sess_cfg[i])->height;
		if(i >0) {
			if((v4l2_enc->simulcast->sim_width[i]>v4l2_enc->simulcast->sim_width[i-1]) ||
					(v4l2_enc->simulcast->sim_height[i] > v4l2_enc->simulcast->sim_height[i -1]))
			{
				LOG_ERR("width/height of simulcast chan %d should be less than chan %d \n",i,i-1);
				return -1;
			}
		}
	}

	width = pCfg->width;
	height = pCfg->height;
	fps = pCfg->fps;
	disp_window = pCfg->display_window;
	ret = SynaV4L2_Open((void *)&(v4l2_enc->handle), ENCODER, V4L2_PIX_FMT_H264, 0);
	if (ret != 0)
	{
		LOG_ERR("SynaV4L2_Open failed = %d\n", ret);
		return -1;
	}

	v4l2_enc->display_window_id = disp_window;
	v4l2_enc->pkt_duration = (1000000000 / fps);
	// Set the PTS of the first buffer to "0"
	v4l2_enc->pkt_pts = 0;
	v4l2_enc->encCfg = (void *)pCfg;

	ret = v4l2enc_set_formats(pCfg);
	if (ret != 0)
	{
		LOG_ERR("v4l2enc_set_formats failed = %d\n", ret);
		return -1;
	}

	v4l2_enc->frame_read_size = (width * height * 3 / 2);
	v4l2_enc->cb_data = NULL;
	for ( i=0; i<v4l2_enc->simulcast->simulcast_num_total;i++)
	{
		v4l2_enc->cb_data_sim[i] = NULL;
	}

	pEnc->last_index = -1;
#ifdef DEBUG_V4L2ENC
	// If display_window_id is "0", then fall back to local write back of the
	// encoded data
	if ((v4l2_enc->display_window_id == 0) && (v4l2_enc->fpo == NULL))
	{
		LOG_DEF("Display Window Id Set to 0 in configuration !!\n");
		v4l2_enc->fpo = fopen("/home/root/outLocal.h264","wb");
		if (NULL == v4l2_enc->fpo)
		{
			LOG_ERR("Invalid output file pointer\n");
			return -1;
		}
	}
	for ( i=0; i<v4l2_enc->simulcast->simulcast_num_total;i++)
	{
		if ((v4l2_enc->display_window_id == 0) && (v4l2_enc->fpo_sim[i] == NULL))
		{
			LOG_DEF("Display Window Id Set to 0 in configuration !!\n");
			v4l2_enc->fpo_sim[i] = fopen("/home/root/outLocal_sim1.h264","wb");
			if (NULL == v4l2_enc->fpo_sim[i])
			{
				LOG_ERR("Invalid output file pointer\n");
				return -1;
			}
		}
	}
#endif

	ret = v4l2enc_setup_input(pCfg);
	LOG_DEF("set up inp rets = %d\n", ret);
	ret = v4l2enc_setup_output(v4l2_enc);
	LOG_DEF("set up out rets = %d\n", ret);
	if (v4l2_enc->cb_data == NULL)
	{
#ifdef DEBUG_V4L2ENC
		if (v4l2_enc->display_window_id == 0)
		{
			v4l2_enc->data_cb =  write_out_data_to_file;
			v4l2_enc->cb_data = (void *) v4l2_enc->fpo;
			for ( i=0; i<v4l2_enc->simulcast->simulcast_num_total;i++)
			{
				if (v4l2_enc->cb_data_sim[i] == NULL)
					v4l2_enc->cb_data_sim[i] = (void *) v4l2_enc->fpo_sim[i];
			}
		}
#endif
		pthread_create(&(v4l2_enc->thread_id), NULL, (void *)v4l2enc_data_out, (void *) (pCfg));
		pthread_create(&pEnc->dqEnc_output_watcher_id, NULL, dqbufEnc_output_watcher, (void *)pCfg);
	}
	return 0;
}

int v4l2h264enc_stop(enc_config_t *pCfg)
{
	int index,i;
	enum v4l2_buf_type buf_type;
	LOG_DEF("In %s\n", __func__);
	int ret = -1;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2_enc = pEnc->v4l2_enc;
	//set the session to be closed
	v4l2_enc->eos = 1;
	v4l2_enc->session_active = 0;

	// FIXME : The below semaphore_down call is masked as it is not required
	// for the current HDMI Rx implementation. It maybe required once we add
	// support for non-tunnel mode.
	//tsem_down(pEnc->videoH264EncoderEventSem);
	free(pEnc->videoH264EncoderEventSem);

	//close the thread
	pthread_join(pEnc->dqEnc_output_watcher_id, NULL);
	pthread_join(v4l2_enc->thread_id, NULL);

	if (v4l2_enc->fpo != NULL)
	{
		fclose(v4l2_enc->fpo);
		v4l2_enc->fpo = NULL;
	}
	for ( i=0; i<v4l2_enc->simulcast->simulcast_num_total;i++)
	{
		if (v4l2_enc->fpo_sim[i] != NULL)
		{
			fclose(v4l2_enc->fpo_sim[i]);
			v4l2_enc->fpo_sim[i] = NULL;
		}
	}
	//Stream OFF for output and capture streams
	buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	ret = SynaV4L2_Ioctl(v4l2_enc->handle, VIDIOC_STREAMOFF, (void*) &buf_type);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_STREAMOFF failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = SynaV4L2_Ioctl(v4l2_enc->handle, VIDIOC_STREAMOFF, (void*) &buf_type);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_STREAMOFF failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	//munmap for input
	for (index = 0; index < NB_BUF_INPUT; index++)
	{
		if (0 != SynaV4L2_Munmap(v4l2_enc->handle, v4l2_enc->mmap_virt_inp[index], v4l2_enc->mmap_size_inp[index]))
		{
			LOG_ERR("Input Munmap failed for index: %d\n", index);
			return -1;
		}
	}

	//munmap for output
	for (index = 0; index < v4l2_enc->out_buf_count; index++)
	{
		if (0 != SynaV4L2_Munmap(v4l2_enc->handle, v4l2_enc->mmap_virt_out[index], v4l2_enc->mmap_size_out[index]))
		{
			LOG_ERR("Output Munmap failed for index: %d\n", index);
			return -1;
		}
	}

	for(int i=0; i<pEnc->h264EncInBufCount; i++)
	{
		if (pEnc->pH264EncInV4l2Buf[i].m.planes)
		{
			free(pEnc->pH264EncInV4l2Buf[i].m.planes);
		}
	}

	free (v4l2_enc->mmap_virt_inp);
	v4l2_enc->mmap_virt_inp = NULL;
	free (v4l2_enc->mmap_size_inp);
	v4l2_enc->mmap_size_inp = NULL;
	free (v4l2_enc->mmap_virt_out);
	v4l2_enc->mmap_virt_out = NULL;
	free (v4l2_enc->mmap_size_out);
	v4l2_enc->mmap_size_out = NULL;

	//Close SynaV4L2
	ret = SynaV4L2_Close(v4l2_enc->handle);
	LOG_DEF("SynaV4L2_Close returned = %d\n", ret);

	return 0;
}

int v4l2h264enc_encode (int frame_size, int index, struct v4l2_enc *v4l2_enc, enc_config_t *pCfg)
{
	int ret = -1;
	ret = v4l2enc_test_encode(frame_size, index, v4l2_enc, pCfg);
	return ret;
}

void setV4L2EncBitrate (enc_config_t *pCfg, unsigned int rate)
{
	int ret = 0;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2_enc = pEnc->v4l2_enc;
	struct v4l2_ext_controls *ext_controls;
	struct v4l2_ext_control *ext_control;

	ext_controls = (struct v4l2_ext_controls *)malloc(sizeof(struct v4l2_ext_controls));
	memset (ext_controls, 0, sizeof (struct v4l2_ext_controls));
	ext_controls->count = 1;
	ext_controls->controls = (struct v4l2_ext_control *)malloc(sizeof(struct v4l2_ext_control));
	ext_control = &ext_controls->controls[0];
	ext_control->id = V4L2_CID_MPEG_VIDEO_BITRATE;
	ext_control->value = rate;
	//ret = SynaV4L2_SetBitRate (v4l2_enc->handle, rate);
	//ret = SynaV4L2_Ioctl(v4l2_enc->handle, VIDIOC_SET_BITRATE, (void*) &rate);
	ret = SynaV4L2_Ioctl(v4l2_enc->handle, VIDIOC_S_EXT_CTRLS, (void*) ext_controls);
	if (ret != 0)
	{
		LOG_ERR("Set Bitrate failed : %d\n", ret);
	}

	if (ext_controls->controls != NULL)
	{
		free (ext_controls->controls);
		ext_controls->controls = NULL;
	}
	if (ext_controls != NULL)
	{
		free (ext_controls);
		ext_controls = NULL;
	}
}

void setV4L2EncFramerate (enc_config_t *pCfg, unsigned int fps_n, unsigned int fps_d)
{
	int ret = 0;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2_enc = pEnc->v4l2_enc;
	struct v4l2_streamparm param;
	// Set Frame-rate Param
	memset (&param, 0, sizeof param);
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	param.parm.output.timeperframe.numerator = fps_n;
	param.parm.output.timeperframe.denominator = fps_d;
	param.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	//ret = SynaV4L2_SetFrameRate (v4l2_enc->handle, fps_n, fps_d);
	//ret = SynaV4L2_Ioctl(v4l2_enc->handle, VIDIOC_SET_FRAMERATE, ((void*)data));
	ret = SynaV4L2_Ioctl(v4l2_enc->handle, VIDIOC_S_PARM, (void*) &param);
	if (ret != 0)
	{
		LOG_ERR("Set Framerate failed = %d\n", ret);
	}
}

void setV4L2EncIDRFrameRequest (enc_config_t *pCfg)
{
	int ret = 0;
	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	struct v4l2_enc *v4l2_enc = pEnc->v4l2_enc;
	struct v4l2_ext_controls *ext_controls;
	struct v4l2_ext_control *ext_control;

	ext_controls = (struct v4l2_ext_controls *)malloc(sizeof(struct v4l2_ext_controls));
	memset (ext_controls, 0, sizeof (struct v4l2_ext_controls));
	ext_controls->count = 1;
	ext_controls->controls = (struct v4l2_ext_control *)malloc(sizeof(struct v4l2_ext_control));
	ext_control = &ext_controls->controls[0];
	ext_control->id = V4L2_CID_MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE;
	ext_control->value = V4L2_MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE_I_FRAME;
	//ret = SynaV4L2_SetForceIDR (v4l2_enc->handle);
	//ret = SynaV4L2_Ioctl(v4l2_enc->handle, VIDIOC_SET_IDR_REQ, NULL);
	ret = SynaV4L2_Ioctl(v4l2_enc->handle, VIDIOC_S_EXT_CTRLS, (void*) ext_controls);
	if (ret != 0)
	{
		LOG_ERR("IDR Request failed = %d\n", ret);
	}

	if (ext_controls->controls != NULL)
	{
		free (ext_controls->controls);
		ext_controls->controls = NULL;
	}
	if (ext_controls != NULL)
	{
		free (ext_controls);
		ext_controls = NULL;
	}
}

void enqueBufFromUIEnc(enc_config_t *pCfg, int shareFd)
{
	if(pCfg == NULL)
		return;

	priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;
	int i;

	/* Set the QtRenderingServer flag to FALSE */
	for(i = 0; i < pEnc->h264EncInBufCount; i++){
		if(pEnc->mmap_virtual_input_sharedFDs[i] == shareFd){
			pEnc->bH264InBufWithQtRdS[i] = false;
			break;
		}
	}

	if (i == pEnc->h264EncInBufCount)
	{
		LOG_ERR("QTRDS: shareFd[%d] notfound\n", shareFd);
	}
}

