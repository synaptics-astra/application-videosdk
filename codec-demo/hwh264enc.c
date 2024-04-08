#include "hwh264enc.h"
//#define DEBUG_V4L2ENC

level_MB_map level_MB_Map[] = {
	{"1.0",99},
	{"1.1",396},
	{"1.2",396},
	{"1.3",396},
	{"2.0",396},
	{"2.1",792},
	{"2.2",1620},
	{"3.0",1620},
	{"3.1",3600},
	{"3.2",5120},
	{"4.0",8192},
	{"4.1",8192},
};

int hw_v4l2enc_test_streamon (void *handle, enum v4l2_buf_type buf_type)
{
	int ret = -1;
	ret = ioctl(handle, VIDIOC_STREAMON, (void*) &buf_type);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_STREAMON failed = %d @ %d\n", ret, __LINE__);
	}
	return ret;
}

int hw_v4l2enc_setup_input(enc_config_t *pCfg)
{
	struct v4l2_requestbuffers rqBuff;
	struct v4l2_exportbuffer expBuff;
	struct v4l2_buffer qryBuff, queBuff;
	int i, j, pixelType;
	void *mmap_ptr = NULL;
	int ret = -1;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2enc = pEnc->v4l2_enc;
	enum v4l2_buf_type buf_type;

	memset (&expBuff, 0, sizeof (expBuff));
	expBuff.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	expBuff.plane = 0;

	memset (&rqBuff, 0, sizeof (rqBuff));

	rqBuff.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	rqBuff.count = NB_BUF_INPUT;
	rqBuff.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(v4l2enc->handle, VIDIOC_REQBUFS, (void*) &rqBuff);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_REQBUFS failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	LOG_PARAM("InBuf: Requested %d  and got %d \n",NB_BUF_INPUT,rqBuff.count);

	v4l2enc->mmap_virt_inp = malloc (sizeof (void *) * rqBuff.count);
	v4l2enc->mmap_size_inp = malloc (sizeof (void *) * rqBuff.count);
	for (i = 0; i < rqBuff.count; i++) {
		v4l2enc->mmap_virt_inp[i] = malloc(sizeof (void *) * v4l2enc->num_planes);
		v4l2enc->mmap_size_inp[i] = malloc(sizeof (int) * v4l2enc->num_planes);
	}

	pEnc->h264EncInBufCount = rqBuff.count;

	v4l2enc->session_active = 1;
	for(i = 0; i < rqBuff.count; i++){
		memset (&qryBuff, 0, sizeof (qryBuff));
		qryBuff.index = i;
		expBuff.index = i;
		qryBuff.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		qryBuff.length = v4l2enc->num_planes;
		qryBuff.memory = V4L2_MEMORY_MMAP;
		qryBuff.m.planes = (struct v4l2_plane *) malloc(sizeof(struct v4l2_plane) * VIDEO_MAX_PLANES);;

		ret = ioctl (v4l2enc->handle, VIDIOC_QUERYBUF, &qryBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QUERYBUF failed = %d @ %d\n", ret, __LINE__);
			return -1;
		}

		pEnc->pH264EncInV4l2Buf[i] = qryBuff;

		for (j =0; j<v4l2enc->num_planes; j++) {
			//LOG_DEF("qryBuff.length = %d\t\tqryBuff.m.offset = %x\n", qryBuff.m.planes[0].length, qryBuff.m.planes[0].m.mem_offset);
			mmap_ptr = mmap(NULL, qryBuff.m.planes[j].length, PROT_READ | PROT_WRITE, MAP_SHARED, v4l2enc->handle, qryBuff.m.planes[j].m.mem_offset);
			//LOG_DEF("ret for mmap = %p\n", mmap_ptr);
			if (mmap_ptr == MAP_FAILED)
			{
				LOG_ERR("mmap FAILED\n");
				return -1;
			}

			/*Get shared_fd*/
			if(pCfg->display_window){
				expBuff.plane = j;
				ret = ioctl(v4l2enc->handle, VIDIOC_EXPBUF, &expBuff);
				if (ret != 0)
				{
					LOG_ERR("Unexpected Error: VIDIOC_EXPBUF failed = %d @ %d\n", ret, __LINE__);
					pEnc->mmap_virtual_input_sharedFDs[i][j] = 0;
				}
				else
				{
					//LOG_DEF("Output side SharedFD(%x)\n", expBuff.fd);
					pEnc->mmap_virtual_input_sharedFDs[i][j] = expBuff.fd;
				}
			}

			v4l2enc->mmap_virt_inp[i][j] = mmap_ptr;
			v4l2enc->mmap_size_inp[i][j] = qryBuff.m.planes[j].length;
			//LOG_DEF("mmap_virt_inp[%d][%d] :  %p \t mmap_size_inp[%d][%d] : %d  \n",i,j,v4l2enc->mmap_virt_inp[i][j],i,j,v4l2enc->mmap_size_inp[i][j]);
		}
		pEnc->bH264InBufWithEnc[i] = false;
		pEnc->bH264InBufWithQtRdS[i] = false;



#if 0
		/* Queue empty V4L2 buffer */
		queBuff = qryBuff;
		queBuff.m.planes[0].bytesused = v4l2enc->frame_read_size;
		ret = ioctl (v4l2enc->handle, VIDIOC_QBUF, &queBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QBUF failed = %d @ %d\n", ret, __LINE__);
			return -1;
		}
#endif
	}
	buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	ret = hw_v4l2enc_test_streamon (v4l2enc->handle, buf_type);
	if (ret != 0)
	{
		LOG_ERR("hw_v4l2enc_test_streamon failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	/*Send Init msg to UI about this ARGB window */
	if(pCfg->display_window) {
		// Note: Check for appropriate input source type

		// Set pixel format to NV12M as we are converting YUYV to NV12
		pixelType = PIXEL_NV12M;
		SendInitMessageToUI(SESS_HWENC,
				pCfg,
				(pCfg->display_window - 1),
				rqBuff.count,
				pCfg->width,
				pCfg->height,
				pixelType);
	}

	return ret;
}

int hw_v4l2enc_setup_output(struct v4l2_hwenc *v4l2enc)
{
	struct v4l2_requestbuffers rqBuff;
	int i,j;
	struct v4l2_buffer qryBuff, queBuff;
	void *mmap_ptr = NULL;
	int ret = -1;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	enum v4l2_buf_type buf_type;
	int req_out_buff;

	memset (&rqBuff, 0, sizeof (rqBuff));
	rqBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	rqBuff.count = NB_BUF_OUTPUT;
	rqBuff.memory = V4L2_MEMORY_MMAP;

	req_out_buff = rqBuff.count;

	ret = ioctl(v4l2enc->handle, VIDIOC_REQBUFS, (void*) &rqBuff);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_REQBUFS failed = %d @ %d errno(%d)\n", ret, __LINE__,errno);
		return -1;
	}

	v4l2enc->out_buf_count = rqBuff.count;
	if(v4l2enc->out_buf_count < req_out_buff)
	{
		LOG_ERR("Requested buf is %d but got only %d Returning from %s",req_out_buff,v4l2enc->out_buf_count,__func__);
		return -1;
	}
	LOG_PARAM("Requested buf is %d And %d Returning from %s \n",req_out_buff,v4l2enc->out_buf_count,__func__);
	//check with req buff

	v4l2enc->mmap_virt_out = malloc (sizeof (void *) * rqBuff.count);
	v4l2enc->mmap_size_out = malloc (sizeof (int) * rqBuff.count);

	memset (&qryBuff, 0, sizeof (qryBuff));
	memset (&planes, 0, sizeof (planes));
	qryBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	qryBuff.memory = V4L2_MEMORY_MMAP;
	qryBuff.m.planes = planes;

	for(i = 0; i < rqBuff.count; i++){
		qryBuff.index = i;
		qryBuff.flags = 0;
		qryBuff.length = 1;

		ret = ioctl (v4l2enc->handle, VIDIOC_QUERYBUF, &qryBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QUERYBUF failed = %d @ %d\n", ret, __LINE__);
			return -1;
		}

		//LOG_DEF("qryBuff.length = %d\t\tqryBuff.m.offset = %x\n", qryBuff.m.planes[0].length, qryBuff.m.planes[0].m.mem_offset);
		mmap_ptr = mmap( NULL, qryBuff.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED,v4l2enc->handle, qryBuff.m.planes[0].m.mem_offset);
		//LOG_DEF("ret for mmap = %p\n", mmap_ptr);
		if (mmap_ptr == MAP_FAILED)
		{
			LOG_ERR("mmap FAILED\n");
			return -1;
		}

		v4l2enc->mmap_virt_out[i] = mmap_ptr;
		v4l2enc->mmap_size_out[i] = qryBuff.m.planes[0].length;

		// Queue empty buffers
		queBuff = qryBuff;
		queBuff.m.planes[0].bytesused = 0;
		queBuff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		ret = ioctl (v4l2enc->handle, VIDIOC_QBUF, &queBuff);
		if (ret != 0)
		{
			LOG_ERR("VIDIOC_QBUF failed = %d @ %d\n", ret, __LINE__);
			return -1;
		}
	}

	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = hw_v4l2enc_test_streamon (v4l2enc->handle, buf_type);
	if (ret != 0)
	{
		LOG_ERR("hw_v4l2enc_test_streamon failed = %d @ %d\n", ret, __LINE__);
	}

	return ret;
}

void hw_setProfile(enc_config_t *pCfg, int *profile)
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
void set_level_compatible(enc_config_t *pCfg,int *level)
{
	int i,ret = -1,cmp = 0;

	LOG_SEQ("Requesting level : %s \n",pCfg->level);
	for (i = 0; i < ARRAY_SIZE(level_MB_Map); i++) {
		if(!cmp) {
			if (strcmp(level_MB_Map[i].level,pCfg->level) == 0) {
				cmp =1;
			}
		}
		if((level_MB_Map[i].MB_Size > ((pCfg->width/16) * (pCfg->height/16))) && (cmp ==1)) {
			strcpy(pCfg->level,level_MB_Map[i].level);
			LOG_SEQ("Setting level : %s \n",pCfg->level);
			hw_setLevel(pCfg,level);
			return;
		}
	}

	strcpy(pCfg->level,"4.1");
	LOG_SEQ("Setting default level : %s \n",pCfg->level);
	hw_setLevel(pCfg,level);
	return;
}

void hw_setLevel(enc_config_t *pCfg, int *level)
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

int hw_v4l2enc_set_formats(enc_config_t *pCfg)
{
	int ret = -1;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2enc = pEnc->v4l2_enc;
	int width, height, fps, bitrate;
	int enc_profile, enc_level;
	struct v4l2_format v4l2enc_infmt,try_fmt;
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
	hw_setProfile(pCfg, &enc_profile);

	// set the right level value
	set_level_compatible(pCfg, &enc_level);

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

	ret = ioctl(v4l2enc->handle, VIDIOC_S_EXT_CTRLS, (void*) ext_controls);
	if (ret != 0)
	{
		LOG_ERR("ioctl for Capture VIDIOC_S_EXT_CTRLS failed = %d\n", ret);
		return -1;
	}
	LOG_ERR("ioctl for Capture VIDIOC_S_EXT_CTRLS Success = %d\n", ret);
	//Output Set Format for Output


	memset (&v4l2enc_infmt, 0, sizeof v4l2enc_infmt);
	v4l2enc_infmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	v4l2enc_infmt.fmt.pix_mp.width = width;
	v4l2enc_infmt.fmt.pix_mp.height = height;
	// Note: Check for appropriate input source type
	// Set pixel format to NV12M as we are converting YUYV to NV12
	v4l2enc_infmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M;

	ret = ioctl(v4l2enc->handle, VIDIOC_S_FMT, (void*) &v4l2enc_infmt);
	if (ret != 0)
	{
		LOG_ERR("ioctl for Output VIDIOC_S_FMT failed = %d\n", ret);
		return -1;
	}

	LOG_PARAM("OUTPUT: Set format %ux%u, sizeimage %u %u, bpl %u %u \n",
         v4l2enc_infmt.fmt.pix_mp.width, v4l2enc_infmt.fmt.pix_mp.height,
         v4l2enc_infmt.fmt.pix_mp.plane_fmt[0].sizeimage, v4l2enc_infmt.fmt.pix_mp.plane_fmt[1].sizeimage,
		v4l2enc_infmt.fmt.pix_mp.plane_fmt[0].bytesperline, v4l2enc_infmt.fmt.pix_mp.plane_fmt[1].bytesperline);

	//LOG_DEF("ioctl for Output VIDIOC_S_FMT Success = %d  num_planes = %d\n", ret,v4l2enc_infmt.fmt.pix_mp.num_planes);
	// check no of planes
	pEnc->v4l2_enc->num_planes = v4l2enc_infmt.fmt.pix_mp.num_planes;

	// Set Frame-rate Param
	memset (&param, 0, sizeof param);
	param.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	param.parm.output.timeperframe.numerator = 1;
	param.parm.output.timeperframe.denominator = fps;
	ret = ioctl(v4l2enc->handle, VIDIOC_S_PARM, (void*) &param);
	if (ret != 0)
	{
		LOG_ERR("ioctl for Output VIDIOC_S_PARM failed = %d\n", ret);
		return -1;
	}

	memset (&v4l2enc_ingfmt, 0, sizeof v4l2enc_ingfmt);
	v4l2enc_ingfmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	ret = ioctl(v4l2enc->handle, VIDIOC_G_FMT, (void*) &v4l2enc_ingfmt);
	if (ret != 0)
	{
		LOG_ERR("ioctl for Output VIDIOC_G_FMT failed = %d\n", ret);
		return -1;
	}

	//Output Set Format for Capture
	memset (&v4l2enc_outfmt, 0, sizeof v4l2enc_outfmt);
	v4l2enc_outfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	v4l2enc_outfmt.fmt.pix_mp.width = width;
	v4l2enc_outfmt.fmt.pix_mp.height = height;
	v4l2enc_outfmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	v4l2enc_outfmt.fmt.pix_mp.plane_fmt[0].sizeimage = 1024 * 1024;

	ret = ioctl(v4l2enc->handle, VIDIOC_S_FMT, (void*) &v4l2enc_outfmt);
	if (ret != 0)
	{
		LOG_ERR("ioctl for Capture VIDIOC_S_FMT failed = %d\n", ret);
		return -1;
	}
	//LOG_DEF("ioctl for Capture VIDIOC_S_FMT Success = %d  num_planes = %d\n", ret,v4l2enc_outfmt.fmt.pix_mp.num_planes);
	memset (&v4l2enc_outgfmt, 0, sizeof v4l2enc_outgfmt);
	v4l2enc_outgfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = ioctl(v4l2enc->handle, VIDIOC_G_FMT, (void*) &v4l2enc_outgfmt);
	if (ret != 0)
	{
		LOG_ERR("ioctl for Capture VIDIOC_G_FMT failed = %d\n", ret);
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


	return ret;
}

// Use this function in order to write the encoded output to a FILE at
// /home/root/outLocal.h264
static int write_out_data_to_file (void *data_ptr, int data_size, void *cb_data)
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

void hw_v4l2enc_data_out(enc_config_t *pCfg)
{
	struct v4l2_buffer dqbuf;
	struct v4l2_buffer qbuf;
	uint8_t *v4l2_data;
	uint32_t v4l2_size;
	int gstRet = 0;
	int ret = -1;
	int header = 0;
	int i;

	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2enc = pEnc->v4l2_enc;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	enc_config_t *ptr = (enc_config_t *)(v4l2enc->encCfg);
	codec_config_t *codec_config = (codec_config_t *)(ptr->codec_config);

	while (1)
	{
try_again2:
		if (v4l2enc->eos)
		{
			//LOG_DEF("[%s] : %d \n",__func__,v4l2enc->eos);
			return;
		}
		memset (&dqbuf, 0, sizeof dqbuf);
		//memset (&planes, 0, sizeof(planes));

		dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		dqbuf.memory = V4L2_MEMORY_MMAP;
		dqbuf.m.planes = planes;
		dqbuf.length = 1;

		//LOG_DEF("waiting for output--->\n");
		ret = ioctl (v4l2enc->handle, VIDIOC_DQBUF, &dqbuf);
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
			else if (ret == EAGAIN) {
			//	usleep (10*1000);
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
						LOG_PARAM("In %s :Queueed Encoded data len[%d]\n", __func__, v4l2_size);
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
					v4l2enc->data_cb(v4l2_data, v4l2_size, v4l2enc->cb_data);
					pEnc->total_bitrate += v4l2_size;
					pEnc->total_frames++;
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
				else
#else
				{
					enc_config_t *pEncCfg = (enc_config_t *)(v4l2enc->encCfg);
					priv_hw_v4l2_encoder_t *pEncPriv = (priv_hw_v4l2_encoder_t *)pEncCfg->priv;
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
			ret = ioctl (v4l2enc->handle, VIDIOC_QBUF, &qbuf);
			if (ret != 0) {
				LOG_ERR("ERROR: VIDIOC_QBUF Failed during encode process %d\n", ret);
			}
		}
		//usleep (10*1000);
	}
}

int hw_v4l2enc_get_input_handle(enc_config_t *pCfg, int32_t *index)
{
	struct v4l2_buffer dqbuf;
	int ret = -1;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2enc = pEnc->v4l2_enc;
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

static int hw_v4l2enc_test_encode(uint32_t frame_size, int index, struct v4l2_hwenc *v4l2enc, enc_config_t *pCfg)
{
	int ret = -1;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
//	LOG_DEF("Inside %s at# %d\n", __func__, __LINE__);

	struct v4l2_buffer qbuf;
	qbuf = pEnc->pH264EncInV4l2Buf[index];
	memset(planes, 0, sizeof(planes));
	qbuf.m.planes = planes;
	qbuf.m.planes[0].bytesused = pCfg->width *pCfg ->height;
	qbuf.m.planes[1].bytesused = (pCfg->width *pCfg ->height) /2;

	pEnc->bH264InBufWithEnc[index] = true;
	ret = ioctl (v4l2enc->handle, VIDIOC_QBUF, &(qbuf));
	if (ret != 0) {
		LOG_ERR("ERROR: VIDIOC_QBUF Failed %d\n", ret);
	}
	if(pCfg->display_window){
		//LOG_ERR("QTRDS: IDX %d\n", index);
		pEnc->bH264InBufWithQtRdS[index] = true;
		sendProcessDMABufId(pCfg->display_window, pEnc->mmap_virtual_input_sharedFDs[index][0], pEnc->mmap_virtual_input_sharedFDs[index][1]);
	}
err:
	//LOG_DEF("returning from %s with ret = %d\n", __func__, ret);
	return ret;
}

void *dqbufHWEnc_output_watcher(void *data)
{
	int ret = -1;
	struct v4l2_buffer dqbuf;
	enc_config_t *pCfg = (enc_config_t *) data;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2enc = pEnc->v4l2_enc;
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
		dqbuf.length = v4l2enc->num_planes;

		ret = ioctl(v4l2enc->handle, VIDIOC_DQBUF, &dqbuf);
		if (ret != 0)
		{
			if(ret != EAGAIN)
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

static int v4l2enc_open_device(enc_config_t *pCfg)
{
    int fd = -1;
    int ret;
    int i = 0;
    short found;
    char path[100];
    struct v4l2_format try_fmt;
    struct v4l2_format s_fmt,g_fmt;
    int libv4l2_fd;
    uint32_t sfmt, pfmt, width, height, buffer_in_size, buffer_out_size;

    // Get data from the params passed by application
    width = pCfg->width;
    height = pCfg->height;
    pfmt = V4L2_PIX_FMT_NV12M;
    sfmt = V4L2_PIX_FMT_H264;

    //TODO: Calculate based on format
    buffer_in_size = (width * height * 3 / 2);
    buffer_out_size = (width * height * 3 / 4);

    for (i = 0; i < MAX_DEVICES; i++) {
        snprintf (path, sizeof (path), "/dev/video%d", i);

        fd = open (path, O_RDWR, 0);
        if (fd < 0) {
            LOG_ERR("Couldn't open path:%s continue\n",path);
            continue;
        }

        memset (&try_fmt, 0, sizeof try_fmt);
        try_fmt.fmt.pix.width = width;
        try_fmt.fmt.pix.height = height;
        try_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        try_fmt.fmt.pix.sizeimage = buffer_in_size;
        try_fmt.fmt.pix.pixelformat = pfmt;

        ret = ioctl (fd, VIDIOC_TRY_FMT, &try_fmt);
        if (ret < 0) {
            LOG_ERR("Couldn't VIDIOC_TRY_FMT for V4L2_BUF_TYPE_VIDEO_OUTPUT continue ret: %d\n",ret);
            close (fd);
            continue;
        }

        memset (&try_fmt, 0, sizeof try_fmt);
        try_fmt.fmt.pix.width = width;
        try_fmt.fmt.pix.height = height;
        try_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        try_fmt.fmt.pix.sizeimage = buffer_out_size;
        try_fmt.fmt.pix.pixelformat = sfmt;

        ret = ioctl (fd, VIDIOC_TRY_FMT, &try_fmt);
        if (ret < 0) {
            LOG_ERR("Couldn't VIDIOC_TRY_FMT for V4L2_BUF_TYPE_VIDEO_CAPTURE continue\n");
            close (fd);
            continue;
        }
        found = TRUE;
        break;
    }

    if (!found) {
        LOG_ERR("No device found matching format V4L2_PIX_FMT_NV12M and resolution %dx%d\n", width, height);
        return -1;
    }

    LOG_DEF("Device %s opened for format V4L2_PIX_FMT_NV12M %dx%d resolution\n", path, width, height);

    return fd;
}


int SetupHWV4L2H264Enc(enc_config_t *pCfg)
{
	int ret = -1;
	int i;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2_enc = pEnc->v4l2_enc;
	int width, height, fps, disp_window;
	char path[25];
	v4l2_enc->fpo = NULL;
	codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;

	if(pCfg->src ==ENC_EXT)
	{
		LOG_ERR("No Init required on ENC_EXT since it will be doing in next CAM/FILE ENC session \n");
		return 0;
	}
	//pEnc->videoH264EncoderEventSem = malloc(sizeof(tsem_t));
	//tsem_init(pEnc->videoH264EncoderEventSem, 0);

	width = pCfg->width;
	height = pCfg->height;
	fps = pCfg->fps;
	disp_window = pCfg->display_window;
	//TBD: add code for checking all video endpoints and take the appropriate enc


	v4l2_enc->handle = v4l2enc_open_device(pCfg);
	if (v4l2_enc->handle < 0)
	{
		LOG_ERR("open failed = %d \n", v4l2_enc->handle);
		return -1;
	}

	v4l2_enc->display_window_id = disp_window;
	v4l2_enc->pkt_duration = (1000000000 / fps);
	// Set the PTS of the first buffer to "0"
	v4l2_enc->pkt_pts = 0;
	v4l2_enc->encCfg = (void *)pCfg;

	ret = hw_v4l2enc_set_formats(pCfg);
	if (ret != 0)
	{
		LOG_ERR("hw_v4l2enc_set_formats failed = %d\n", ret);
		return -1;
	}

	v4l2_enc->frame_read_size = (width * height * 3 / 2);
	v4l2_enc->cb_data = NULL;

	pEnc->last_index = -1;
#ifdef DEBUG_V4L2ENC
	// If display_window_id is "0", then fall back to local write back of the
	// encoded data
	if ((v4l2_enc->display_window_id == 0) && (v4l2_enc->fpo == NULL))
	{
		LOG_SEQ("Display Window Id Set to 0 in configuration !!\n");
		v4l2_enc->fpo = fopen("/home/root/outLocal.h264","wb");
		if (NULL == v4l2_enc->fpo)
		{
			LOG_ERR("Invalid output file pointer\n");
			return -1;
		}
	}
#endif

	ret = hw_v4l2enc_setup_input(pCfg);
	ret = hw_v4l2enc_setup_output(v4l2_enc);
	if (v4l2_enc->cb_data == NULL)
	{
#ifdef DEBUG_V4L2ENC
		if (v4l2_enc->display_window_id == 0)
		{
			v4l2_enc->data_cb =  write_out_data_to_file;
			v4l2_enc->cb_data = (void *) v4l2_enc->fpo;
		}
#endif
		pthread_create(&(v4l2_enc->thread_id), NULL, (void *)hw_v4l2enc_data_out, (void *) (pCfg));
		pthread_create(&pEnc->dqEnc_output_watcher_id, NULL, dqbufHWEnc_output_watcher, (void *)pCfg);
	}
	return 0;
}

int hw_v4l2h264enc_stop(enc_config_t *pCfg)
{
	int index,i,plane;
	enum v4l2_buf_type buf_type;
	//LOG_DEF("In %s\n", __func__);
	int ret = -1;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2_enc = pEnc->v4l2_enc;
	//set the session to be closed
	v4l2_enc->eos = 1;
	v4l2_enc->session_active = 0;

	if (v4l2_enc->fpo != NULL)
	{
		fclose(v4l2_enc->fpo);
		v4l2_enc->fpo = NULL;
	}
	//Stream OFF for output and capture streams
	buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	ret = ioctl(v4l2_enc->handle, VIDIOC_STREAMOFF, (void*) &buf_type);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_STREAMOFF failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	ret = ioctl(v4l2_enc->handle, VIDIOC_STREAMOFF, (void*) &buf_type);
	if (ret != 0)
	{
		LOG_ERR("VIDIOC_STREAMOFF failed = %d @ %d\n", ret, __LINE__);
		return -1;
	}

	//close the thread
	pthread_join(pEnc->dqEnc_output_watcher_id, NULL);
	pthread_join(v4l2_enc->thread_id, NULL);

	//munmap for input
	for (index = 0; index < pEnc->h264EncInBufCount; index++)
	{
		for(plane = 0; plane < v4l2_enc->num_planes; plane++) {
			if (0 != munmap(v4l2_enc->mmap_virt_inp[index][plane], v4l2_enc->mmap_size_inp[index][plane]))
			{
				LOG_ERR("Input Munmap failed for index: %d, plane : %d\n", index,plane);
				return -1;
			}
		}
	}

	//munmap for output
	for (index = 0; index < v4l2_enc->out_buf_count; index++)
	{
		if (0 != munmap(v4l2_enc->mmap_virt_out[index], v4l2_enc->mmap_size_out[index]))
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

	for (index = 0; index < pEnc->h264EncInBufCount; index++) {
		free (v4l2_enc->mmap_virt_inp[index]);
		free (v4l2_enc->mmap_size_inp[index]);
		v4l2_enc->mmap_virt_inp[index] = NULL;
		v4l2_enc->mmap_size_inp[index] = NULL;
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
	ret = close(v4l2_enc->handle);
	LOG_DEF("close returned = %d\n", ret);

	return 0;
}

int hw_h264enc_encode (int frame_size, int index, struct v4l2_hwenc *v4l2_enc, enc_config_t *pCfg)
{
	int ret = -1;
	ret = hw_v4l2enc_test_encode(frame_size, index, v4l2_enc, pCfg);
	return ret;
}

void setV4L2HWEncBitrate (enc_config_t *pCfg, unsigned int rate)
{
	int ret = 0;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2_enc = pEnc->v4l2_enc;
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
	//ret = ioctl(v4l2_enc->handle, VIDIOC_SET_BITRATE, (void*) &rate);
	ret = ioctl(v4l2_enc->handle, VIDIOC_S_EXT_CTRLS, (void*) ext_controls);
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

void setV4L2HWEncFramerate (enc_config_t *pCfg, unsigned int fps_n, unsigned int fps_d)
{
	int ret = 0;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2_enc = pEnc->v4l2_enc;
	struct v4l2_streamparm param;
	// Set Frame-rate Param
	memset (&param, 0, sizeof param);
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	param.parm.output.timeperframe.numerator = fps_n;
	param.parm.output.timeperframe.denominator = fps_d;
	param.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	//ret = SynaV4L2_SetFrameRate (v4l2_enc->handle, fps_n, fps_d);
	//ret = ioctl(v4l2_enc->handle, VIDIOC_SET_FRAMERATE, ((void*)data));
	ret = ioctl(v4l2_enc->handle, VIDIOC_S_PARM, (void*) &param);
	if (ret != 0)
	{
		LOG_ERR("Set Framerate failed = %d\n", ret);
	}
}

void setV4L2HWEncIDRFrameRequest (enc_config_t *pCfg)
{
	int ret = 0;
	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	struct v4l2_hwenc *v4l2_enc = pEnc->v4l2_enc;
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
	//ret = ioctl(v4l2_enc->handle, VIDIOC_SET_IDR_REQ, NULL);
	ret = ioctl(v4l2_enc->handle, VIDIOC_S_EXT_CTRLS, (void*) ext_controls);
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

void enqueBufFromUIHWEnc(enc_config_t *pCfg, int shareFd)
{
	if(pCfg == NULL)
		return;

	priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;
	int i;

	/* Set the QtRenderingServer flag to FALSE */
	for(i = 0; i < pEnc->h264EncInBufCount; i++){
		if(pEnc->mmap_virtual_input_sharedFDs[i][0] == shareFd){
			pEnc->bH264InBufWithQtRdS[i] = false;
			break;
		}
	}

	if (i == pEnc->h264EncInBufCount)
	{
		LOG_ERR("QTRDS: shareFd[%d] notfound\n", shareFd);
	}
}

