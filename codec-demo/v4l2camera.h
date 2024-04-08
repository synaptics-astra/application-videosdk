#ifndef _V4L2CAMERA_H_
#define _V4L2CAMERA_H_

#include "syna_std_headers.h"
#include "config.h"

typedef struct v4l2cam_t
{
	int	width;
	int	height;
	int	fps;
	char	path[MAX_FILE_PATH_STR];
	int	cam_fd;
	int	cam_v4l2_fmt;
	void	**mmap_virtual_input;
	int	*mmap_size_input;
	int	number_output_buffer;
}v4l2cam_t;

int SetupCamera(v4l2cam_t *pV4l2Cam);
void ReadCameraInput(v4l2cam_t *pV4l2Cam, void** ptr, int* size,int* fb_index);
void EnqueCameraBuffer(v4l2cam_t *pV4l2Cam, int index);
void ShutdownCamera(v4l2cam_t *pV4l2Cam);
int SetupCameraFps(v4l2cam_t *pV4l2Cam, int num, int den);

#endif //_V4L2CAMERA_H_
