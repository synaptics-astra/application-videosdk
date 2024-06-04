// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "v4l2camera.h"

#define v4l2_ioctl ioctl
#define v4l2_close close
#define v4l2_mmap mmap

#define NB_BUF_OUTPUT (8)

static int setCameraFormat(v4l2cam_t *pV4l2Cam)
{
	if(-1 != pV4l2Cam->cam_fd){

		struct v4l2_format fmt;
		int ret = 0;
		memset(&fmt, 0, sizeof fmt);
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.pixelformat =  pV4l2Cam->cam_v4l2_fmt;
		ret = v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_G_FMT, &fmt);
		if(ret <0){
			LOG_ERR("Error: VIDIOC_G_FMT has failed \n");
			goto err;
		}
		//LOG_DEF("G_FMT(start): width = %u, height = %u, 4cc = %.4s\n", fmt.fmt.pix.width, fmt.fmt.pix.height, (char*)&fmt.fmt.pix.pixelformat);

		fmt.fmt.pix.width = pV4l2Cam->width;
		fmt.fmt.pix.height = pV4l2Cam->height;
		fmt.fmt.pix.pixelformat = pV4l2Cam->cam_v4l2_fmt;

		LOG_DEF("S_FMT(final): width = %u, height = %u, 4cc = %.4s\n", fmt.fmt.pix.width, fmt.fmt.pix.height, (char*)&fmt.fmt.pix.pixelformat);
		ret = v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_S_FMT , &fmt);
		if(ret <0){
			LOG_ERR("Error: VIDIOC_S_FMT has failed \n");
			goto err;
		}

		ret = v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_G_FMT, &fmt);
		if(ret <0){
			LOG_ERR("Error: VIDIOC_G_FMT has failed \n");
			goto err;
		}
		LOG_DEF("G_FMT(final): width = %u, height = %u, 4cc = %.4s\n", fmt.fmt.pix.width, fmt.fmt.pix.height, (char*)&fmt.fmt.pix.pixelformat);

	}else{
		LOG_ERR("The camera fd is not configured \n");
		goto err;
	}
	return 0;
err:
	return -1;
}

static int openCameraDevice(v4l2cam_t *pV4l2Cam)
{
	int ret =-1;
	int found;
	char path[100];
	struct v4l2_capability cap;
	int fd;
	int i =0;
	int curr_index = 0;

	/* Start with userdefined path */
	snprintf (path, sizeof (path), "%s", pV4l2Cam->path);
retry:
	found = 0;
	{
		fd = open (path, O_RDWR, 0);
		if (fd < 0)
			goto err;

		ret = v4l2_ioctl (fd, VIDIOC_QUERYCAP, &cap);
		if (ret < 0) {
			v4l2_close(fd);
			goto err;
		}

		//check for video capture devices
		if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
			v4l2_close(fd);
			snprintf (path, sizeof (path), "%s%d", "/dev/video", curr_index++);
			LOG_DEF("V4L2Cam: Restarting search:%s \n",path);
			goto retry;
		}

		//check that the device is not a decoder one
		if ((cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) != 0) {
			v4l2_close(fd);
			snprintf (path, sizeof (path), "%s%d", "/dev/video", curr_index++);
			LOG_DEF("V4L2Cam: Restarting search:%s \n",path);
			goto retry;
		}

		if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)){
			LOG_DEF("It supports  V4L2_CAP_VIDEO_CAPTURE_MPLANE \n");
		}

		pV4l2Cam->cam_fd = fd;
		if(setCameraFormat(pV4l2Cam) ==-1){
			LOG_ERR("Failed to set camera format \n");
			close(pV4l2Cam->cam_fd);
			snprintf (path, sizeof (path), "%s%d", "/dev/video", curr_index++);
			LOG_DEF("V4L2Cam: Restarting search:%s \n",path);
			goto retry;
		}
		//Found the camera fd
		found = 1;
		ret = 0;
	}

	if(1 == found){
		LOG_DEF("Found the camera in location %s\n",path);
	}
	return ret;

err:
	ret = -1;
	return ret;
}

static int setupCameraOutputMemory(v4l2cam_t *pV4l2Cam)
{
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer querybuf;
	int i;

	/* Memory mapping for input buffers in V4L2 */
	memset (&reqbuf, 0, sizeof reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.count = NB_BUF_OUTPUT;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if (v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
		LOG_ERR("ERROR: VIDIOC_REQBUFS Failed\n");
		goto err;
	}

	pV4l2Cam->mmap_virtual_input = malloc (sizeof (void *) * reqbuf.count);
	pV4l2Cam->mmap_size_input = malloc (sizeof (int) * reqbuf.count);
	pV4l2Cam->number_output_buffer =  reqbuf.count;

	memset(&querybuf, 0, sizeof querybuf);
	querybuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	querybuf.memory = V4L2_MEMORY_MMAP;
	LOG_DEF("The created request buffer count %d\n", reqbuf.count);
	for(i = 0; i < reqbuf.count; i++) {
		void *ptr;
		querybuf.index = i;

		/* Memory mapping for input buffers */
		if(v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_QUERYBUF, &querybuf) < 0) {
			LOG_ERR("ERROR: VIDIOC_QUERYBUF Failed : %d and str %s\n",errno,strerror(errno));
			goto err;
		}

		ptr = v4l2_mmap(NULL, querybuf.length, PROT_READ | PROT_WRITE, MAP_SHARED, pV4l2Cam->cam_fd, querybuf.m.offset);
		if (ptr == MAP_FAILED) {
			LOG_ERR("ERROR: v4l2_mmap Failed\n");
			goto err;
		}

		pV4l2Cam->mmap_virtual_input[i] = ptr;
		pV4l2Cam->mmap_size_input[i] = querybuf.length;
	}

	return 0;

err:
	return -1;
}

// Function to display the FRAMEINTERVALS supported by the Camera Device
void log_frameval(const struct v4l2_frmivalenum *frameval)
{
	LOG_DEF("Interval: %d ", frameval->type);
	if (frameval->type == V4L2_FRMIVAL_TYPE_DISCRETE) {
		LOG_DEF("Time: %.3f , FPS : %.3f\n", ((1.0 * frameval->discrete.numerator) / frameval->discrete.denominator),
				((1.0 * frameval->discrete.denominator) / frameval->discrete.numerator));
	} else if (frameval->type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
		LOG_DEF("MINIMUM::Time: %.3f , FPS : %.3f\n", ((1.0 * frameval->stepwise.min.numerator) / frameval->stepwise.min.denominator),
				((1.0 * frameval->stepwise.min.denominator) / frameval->stepwise.min.numerator));
		LOG_DEF("MAXIMUM::Time: %.3f , FPS : %.3f\n", ((1.0 * frameval->stepwise.max.numerator) / frameval->stepwise.max.denominator),
				((1.0 * frameval->stepwise.max.denominator) / frameval->stepwise.max.numerator));
	} else if (frameval->type == V4L2_FRMIVAL_TYPE_STEPWISE) {
		LOG_DEF("MINIMUM::Time: %.3f , FPS : %.3f\n", ((1.0 * frameval->stepwise.min.numerator) / frameval->stepwise.min.denominator),
				((1.0 * frameval->stepwise.min.denominator) / frameval->stepwise.min.numerator));
		LOG_DEF("MAXIMUM::Time: %.3f , FPS : %.3f\n", ((1.0 * frameval->stepwise.max.numerator) / frameval->stepwise.max.denominator),
				((1.0 * frameval->stepwise.max.denominator) / frameval->stepwise.max.numerator));
		LOG_DEF("STEPWISE::Time: %.3f , FPS : %.3f\n", ((1.0 * frameval->stepwise.step.numerator) / frameval->stepwise.step.denominator),
				((1.0 * frameval->stepwise.step.denominator) / frameval->stepwise.step.numerator));
	}
}

// Function to set the FPS of the Camera Device
int SetupCameraFps(v4l2cam_t *pV4l2Cam, int num, int den)
{
	struct v4l2_streamparm streamParams;
	struct v4l2_frmivalenum frmval;
	uint32_t ret;
	int bFound = 0;
	struct v4l2_buffer querybuf;
	int i;

	LOG_DEF("Target FPS : %d/%d\n", num, den);
	memset (&streamParams, 0, sizeof streamParams);
	// Just to validate if the driver allows V4L2_CAP_TIMEPERFRAME capability
	streamParams.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_G_PARM, &streamParams);
	if (ret < 0) {
		LOG_ERR("ERROR: VIDIOC_S_PARM failed on input\n");
		close(pV4l2Cam->cam_fd);
		return -1;
	}

	if ((streamParams.parm.capture.capability == V4L2_CAP_TIMEPERFRAME) ||
			(streamParams.parm.capture.capturemode == V4L2_CAP_TIMEPERFRAME))
	{
		// Find out all the supported frame intervals
		memset (&frmval, 0, sizeof (struct v4l2_frmivalenum));
		frmval.index = 0;
		frmval.width = pV4l2Cam->width;
		frmval.height = pV4l2Cam->height;
		frmval.pixel_format = pV4l2Cam->cam_v4l2_fmt;
		while (v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmval) >= 0) {
			log_frameval(&frmval);
			if (frmval.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
				if (frmval.discrete.denominator == num)
				{
					LOG_DEF("Found the FPS\n");
					bFound = 1;
					break;
				}
			}
			frmval.index++;
		}

		if (!bFound)
		{
			LOG_ERR("Target Frame Rate to be set not supported for source\n");
			return -1;
		}

		uint32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_STREAMOFF, &type) < 0) {
			LOG_ERR("ERROR: Stream ON filed on input\n");
			close(pV4l2Cam->cam_fd);
			return -1;
		}
		// Set FPS as 1/fps
		streamParams.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		streamParams.parm.capture.capability |= V4L2_CAP_TIMEPERFRAME;
		streamParams.parm.capture.capturemode |= V4L2_CAP_TIMEPERFRAME;
		streamParams.parm.capture.timeperframe.denominator = frmval.discrete.denominator;
		streamParams.parm.capture.timeperframe.numerator= frmval.discrete.numerator;
		ret = v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_S_PARM, &streamParams);
		if (ret < 0) {
			LOG_ERR("ERROR: VIDIOC_S_PARM failed on input\n");
			close(pV4l2Cam->cam_fd);
			return -1;
		}

		// Enqueue empty buffers so as to make sure when we set FPS we have all
		// empty buffers in the Queue
		memset(&querybuf, 0, sizeof querybuf);
		querybuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		querybuf.memory = V4L2_MEMORY_MMAP;
		for(i = 0; i < pV4l2Cam->number_output_buffer; i++) {
			querybuf.index = i;
			querybuf.bytesused = 0;         /* enqueue it with no data */
			if (v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_QBUF, &querybuf) < 0) {
				LOG_ERR("Empty QBUF Failed\n");
			}
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_STREAMON, &type) < 0) {
			LOG_ERR("ERROR: Stream ON failed on input\n");
			close(pV4l2Cam->cam_fd);
			return -1;
		}

		memset (&streamParams, 0, sizeof streamParams);
		streamParams.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ret = v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_G_PARM, &streamParams);
		if (ret < 0) {
			LOG_ERR("ERROR: VIDIOC_S_PARM failed on input\n");
			close(pV4l2Cam->cam_fd);
			return -1;
		}
	}

	return 0;
}

int SetupCamera(v4l2cam_t *pV4l2Cam)
{
	int width =  pV4l2Cam->width;
	int height = pV4l2Cam->height;

	struct v4l2_streamparm streamParams;
	uint32_t type, ret;
	struct v4l2_buffer querybuf;
	struct v4l2_buffer qbuf;
	int i;

	if(openCameraDevice(pV4l2Cam) == -1){
		LOG_ERR("Failed to open camera device \n");
		return -1;
	}

	if(setupCameraOutputMemory(pV4l2Cam) ==-1){
		LOG_ERR("Failed to create memory output \n");
		close(pV4l2Cam->cam_fd);
		return -1;
	}
	//configure for preview size and pixel format.
	//set frame rate
	memset (&streamParams, 0, sizeof streamParams);
	streamParams.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	streamParams.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	streamParams.parm.capture.capturemode = (V4L2_MODE_HIGHQUALITY | V4L2_CAP_TIMEPERFRAME);
	streamParams.parm.capture.timeperframe.denominator = pV4l2Cam->fps;
	streamParams.parm.capture.timeperframe.numerator= 1;
	ret = v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_S_PARM, &streamParams);
	if (ret < 0) {
		LOG_ERR("ERROR: VIDIOC_S_PARM filed on input\n");
		close(pV4l2Cam->cam_fd);
		return -1;
	}
	memset(&querybuf, 0, sizeof querybuf);
	querybuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	querybuf.memory = V4L2_MEMORY_MMAP;
	for(i = 0; i < pV4l2Cam->number_output_buffer; i++) {
		querybuf.index = i;
		qbuf = querybuf;            /* index from querybuf */
		qbuf.bytesused = 0;         /* enqueue it with no data */
		if (v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_QBUF, &qbuf) < 0) {
			close(pV4l2Cam->cam_fd);
			return -1;
		}
	}
	/* Start stream on input */
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_STREAMON, &type) < 0) {
		LOG_ERR("ERROR: Stream ON filed on input\n");
		close(pV4l2Cam->cam_fd);
		return -1;
	}

	return 0;
}

void ReadCameraInput(v4l2cam_t *pV4l2Cam, void** ptr, int* size, int* fb_index)
{
	//We reached here where camera is started
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	struct v4l2_buffer dqbuf;
	int index  = -1;
	memset (&dqbuf, 0, sizeof dqbuf);
	dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dqbuf.memory = V4L2_MEMORY_MMAP;
	if (v4l2_ioctl (pV4l2Cam->cam_fd, VIDIOC_DQBUF, &dqbuf) < 0) {
		LOG_ERR("ERROR: VIDIOC_DQBUF Failed \n");
		usleep (50 * 1000);
	}else {
		index = dqbuf.index;
		//LOG_DEF("The recv buffer size is %d\n",dqbuf.bytesused);
		*ptr =  pV4l2Cam->mmap_virtual_input[index];
		*size  = dqbuf.bytesused;
		*fb_index = index;
	}
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
}

void EnqueCameraBuffer(v4l2cam_t *pV4l2Cam, int index)
{
	//LOG_DEF("Enqueu amera output now\n");
	struct v4l2_buffer qbuf;
	memset (&qbuf, 0, sizeof qbuf);
	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	qbuf.memory = V4L2_MEMORY_MMAP;
	qbuf.index = index;
	qbuf.bytesused = 0;         /* enqueue it with no data */

	if (v4l2_ioctl (pV4l2Cam->cam_fd, VIDIOC_QBUF, &qbuf) < 0) {
		LOG_ERR("ERROR: VIDIOC_QBUF Failed \n");
		usleep (50 * 1000);
	}
}

void ShutdownCamera(v4l2cam_t *pV4l2Cam)
{
	if(-1 != pV4l2Cam->cam_fd)
	{
		/* Stop stream on input */
		uint32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(v4l2_ioctl(pV4l2Cam->cam_fd, VIDIOC_STREAMOFF, &type) < 0) {
			LOG_ERR("ERROR: Stream ON filed on input\n");
			close(pV4l2Cam->cam_fd);
			return;
		}

		if(pV4l2Cam->mmap_virtual_input)
			free(pV4l2Cam->mmap_virtual_input);

		if(pV4l2Cam->mmap_size_input)
			free(pV4l2Cam->mmap_size_input);

		v4l2_close(pV4l2Cam->cam_fd);
		pV4l2Cam->cam_fd = -1;
	}
}
