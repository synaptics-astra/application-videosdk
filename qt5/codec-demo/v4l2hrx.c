// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include "v4l2hrx.h"

#define v4l2_ioctl ioctl
#define v4l2_close close
#define v4l2_mmap mmap

#define NB_BUF_OUTPUT (8)

static int setupHRxFormat(v4l2hrx_t *pV4l2HRx)
{
	if(-1 != pV4l2HRx->hdmi_rx_fd){

		struct v4l2_format fmt;
		int ret = 0;
		memset(&fmt, 0, sizeof fmt);

		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		fmt.fmt.pix.width = pV4l2HRx->width;
		fmt.fmt.pix.height = pV4l2HRx->height;
		fmt.fmt.pix.pixelformat = pV4l2HRx->hdmi_rx_fmt;

		LOG_DEF("S_FMT(final): width = %u, height = %u, 4cc = %.4s\n", fmt.fmt.pix.width, fmt.fmt.pix.height, (char*)&fmt.fmt.pix.pixelformat);
		ret = v4l2_ioctl(pV4l2HRx->hdmi_rx_fd, VIDIOC_S_FMT , &fmt);
		if(ret <0){
			LOG_ERR("Error: VIDIOC_S_FMT has failed \n");
			goto err;
		}

		ret = v4l2_ioctl(pV4l2HRx->hdmi_rx_fd, VIDIOC_G_FMT, &fmt);
		if(ret <0){
			LOG_ERR("Error: VIDIOC_G_FMT has failed \n");
			goto err;
		}
		LOG_DEF("G_FMT(final): width = %u, height = %u, 4cc = %.4s\n", fmt.fmt.pix.width, fmt.fmt.pix.height, (char*)&fmt.fmt.pix.pixelformat);

	}else{
		LOG_ERR("The hdmi rx fd is not configured \n");
		goto err;
	}
	return 0;
err:
	return -1;
}

static int openHRxDevice(v4l2hrx_t* pV4l2HRx)
{
	int ret =-1;
	int found;
	char path[100];
	struct v4l2_capability cap;
	int fd;
	int i =0;
	int curr_index = 0;

	/* Start with userdefined path */
	snprintf (path, sizeof (path), "%s", pV4l2HRx->path);
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
			LOG_DEF("V4L2HRx: Restarting search:%s \n",path);
			goto retry;
		}

		//check that the device is not a decoder one
		if ((cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) != 0) {
			v4l2_close(fd);
			snprintf (path, sizeof (path), "%s%d", "/dev/video", curr_index++);
			LOG_DEF("V4L2HRx: Restarting search:%s \n",path);
			goto retry;
		}

		if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)){
			LOG_DEF("It supports  V4L2_CAP_VIDEO_CAPTURE_MPLANE \n");
		}

		pV4l2HRx->hdmi_rx_fd = fd;
		if(setupHRxFormat(pV4l2HRx) ==-1){
			LOG_ERR("Failed to set HRx format \n");
			close(pV4l2HRx->hdmi_rx_fd);
			snprintf (path, sizeof (path), "%s%d", "/dev/video", curr_index++);
			LOG_DEF("V4L2HRx: Restarting search:%s \n",path);
			goto retry;
		}
		found = 1;
		ret = 0;
	}

	if(1 == found){
		LOG_DEF("Found the HRx device in location %s\n",path);
	}
	return ret;

err:
	ret = -1;
	return ret;
}

static int setupHRxOutputMemory(v4l2hrx_t *pV4l2HRx)
{
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer querybuf, qbuf;
	struct v4l2_plane queryplanes[VIDEO_MAX_PLANES];
	int i, ret;
	int type;

	/* Memory mapping for input buffers in V4L2 */
	memset (&reqbuf, 0, sizeof reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbuf.count = 4;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	if (v4l2_ioctl(pV4l2HRx->hdmi_rx_fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
		LOG_ERR("ERROR: VIDIOC_REQBUFS Failed\n");
		goto err;
	}

	pV4l2HRx->mmap_virtual_input = malloc (sizeof (void *) * reqbuf.count);
	pV4l2HRx->mmap_size_input = malloc (sizeof (int) * reqbuf.count);
	pV4l2HRx->number_output_buffer =  reqbuf.count;

	memset(&querybuf, 0, sizeof querybuf);
	querybuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	querybuf.memory = V4L2_MEMORY_MMAP;
	LOG_DEF("The created request buffer count %d\n", reqbuf.count);
	for(i = 0; i < reqbuf.count; i++) {
		void *ptr;
		memset (&querybuf, 0, sizeof (querybuf));
		memset (&queryplanes, 0, sizeof (queryplanes));
		querybuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		querybuf.memory = V4L2_MEMORY_MMAP;
		querybuf.m.planes = queryplanes;
		querybuf.length = 1;
		querybuf.index = i;

		/* Memory mapping for input buffers */
		if(v4l2_ioctl(pV4l2HRx->hdmi_rx_fd, VIDIOC_QUERYBUF, &querybuf) < 0) {
			LOG_ERR("ERROR: VIDIOC_QUERYBUF Failed : %d and str %s\n",errno,strerror(errno));
			goto err;
		}

		ptr = v4l2_mmap(NULL, querybuf.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, pV4l2HRx->hdmi_rx_fd, querybuf.m.planes[0].m.mem_offset);
		if (ptr == MAP_FAILED) {
			LOG_ERR("ERROR: v4l2_mmap Failed\n");
			goto err;
		}

		pV4l2HRx->mmap_virtual_input[i] = ptr;
		pV4l2HRx->mmap_size_input[i] = querybuf.length;
		qbuf = querybuf;						/* index from querybuf */
		qbuf.m.planes[0].bytesused = 0;		 /* enqueue it with no data */

		ret = ioctl (pV4l2HRx->hdmi_rx_fd, VIDIOC_QBUF, &qbuf);
		if (ret != 0) {
			printf ("VIDIOC_QBUF failed = (%d) @Line(%d) in Fn(%s)\n", ret,__LINE__, __func__);
			ret = -1;
		}
	}
	return 0;

err:
	LOG_ERR("[%s::%s] ERROR ... ERROR ... ERROR\n", __FILE__, __func__);
	return -1;
}



int SetupHRx(v4l2hrx_t* pV4l2HRx)
{

	int width =  pV4l2HRx->width;
	int height = pV4l2HRx->height;

	struct v4l2_streamparm streamParams;
	uint32_t type, ret;
	struct v4l2_buffer querybuf;
	struct v4l2_buffer qbuf;
	int i;

	if(openHRxDevice(pV4l2HRx) == -1){
		LOG_ERR("Failed to open HRX device \n");
		return -1;
	}

	if(setupHRxOutputMemory(pV4l2HRx) ==-1){
		LOG_ERR("Failed to create memory output \n");
		close(pV4l2HRx->hdmi_rx_fd);
		return -1;
	}
}


void ReadHRxInput(v4l2hrx_t *pV4l2HRx, void** ptr, int* size, int* fb_index)
{
	//We reached here where camera is started
	struct v4l2_buffer buf;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	int index  = -1;
	memset (&buf, 0, sizeof (buf));
	memset (&planes, 0, sizeof(planes));
	buf.m.planes = planes;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.length = 1;
	if (v4l2_ioctl (pV4l2HRx->hdmi_rx_fd, VIDIOC_DQBUF, &buf) < 0) {
		LOG_ERR("ERROR: VIDIOC_DQBUF Failed, errno = %d \n", errno);
		usleep (50 * 1000);
	}else {
		index = buf.index;
		*ptr =  pV4l2HRx->mmap_virtual_input[index];
		*size  = buf.bytesused;
		*fb_index = index;
	}
}

void EnqueHRxBuffer(v4l2hrx_t *pV4l2HRx, int index)
{
	int ret = 0;
	int handle = pV4l2HRx->hdmi_rx_fd;
	struct v4l2_buffer qbuf;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	memset (&qbuf, 0, sizeof (qbuf));
	memset (&planes, 0, sizeof(planes));
	qbuf.m.planes = planes;
	qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	qbuf.memory = V4L2_MEMORY_MMAP;
	qbuf.m.planes[0].bytesused = 0;
	qbuf.index = index;
	qbuf.length = 1;
	ret = ioctl (handle, VIDIOC_QBUF, &qbuf);
	if (ret != 0) {
		printf ("VIDIOC_QBUF failed = (%d) @Line(%d) in Fn(%s)\n", errno,
				__LINE__, __func__);
		ret = -1;
	}
}

void ShutdownHRx(v4l2hrx_t *pV4l2HRx)
{
	if(-1 != pV4l2HRx->hdmi_rx_fd)
	{
		/* Stop stream on input */
		uint32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(v4l2_ioctl(pV4l2HRx->hdmi_rx_fd, VIDIOC_STREAMOFF, &type) < 0) {
			LOG_ERR("ERROR: Stream ON filed on input\n");
			close(pV4l2HRx->hdmi_rx_fd);
			return;
		}

		if(pV4l2HRx->mmap_virtual_input)
			free(pV4l2HRx->mmap_virtual_input);

		if(pV4l2HRx->mmap_size_input)
			free(pV4l2HRx->mmap_size_input);

		v4l2_close(pV4l2HRx->hdmi_rx_fd);
		pV4l2HRx->hdmi_rx_fd = -1;
	}
}

int StreamOnHRx(v4l2hrx_t* pV4l2HRx)
{
	int type;
	/* Start stream on input */
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if(v4l2_ioctl(pV4l2HRx->hdmi_rx_fd, VIDIOC_STREAMON, &type) < 0) {
		LOG_ERR("ERROR: Stream ON failed on input\n");
		close(pV4l2HRx->hdmi_rx_fd);
		goto err;
	}

	return 0;

err:
	LOG_ERR("[%s::%s] ERROR ... ERROR ... ERROR\n", __FILE__, __func__);
	return -1;
}
