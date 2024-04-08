#ifndef _HWH264ENC_H_
#define _HWH264ENC_H_

#include "config.h"
#include "gsttxpipeline.h"
#include "encoder_interface_hwv4l2.h"

#define ENCODER 1
// Number of input buffers
#define NB_BUF_INPUT	8
// Number of output buffers
#define NB_BUF_OUTPUT	4
#define FRAME_BUF_SIZE (1920*1080*4)

#define ENC_EXT_CONTROLS_PARAM_NUM 10
#define MAX_DEVICES 8
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

struct v4l2_hwenc {
	// Global handle for the application to interact with SynaV4l2 library
	int				handle;
	FILE			*fpo;
	FILE			*fpo_sim[MAX_EXT_ENC_SESSIONS];
	void			***mmap_virt_inp;	 /* Pointer to input frames pointers */
	int				**mmap_size_inp;

	void			**mmap_virt_out;	 /* Pointer to output frame pointers */
	int				*mmap_size_out;

	pthread_t		thread_id;
	void			*cb_data;
	void			*cb_data_sim[MAX_EXT_ENC_SESSIONS];
	int (*data_cb)(void *data_ptr, int data_size, void *cb_data);

	int				eos;
	int				session_active;
	int				frame_read_size;
	int				display_window_id;
	int				pkt_duration;
	long long int	pkt_pts;
	void			*encCfg; /* pointer to hold the encoder config handle */

	void			**mmap_virt_hdmi_out;	 /* Pointer to output frame pointers */
	int				*mmap_size_hdmi_out;
	int 			out_buf_count;
//	int 			in_buf_count;
	int 			num_planes;
};

typedef struct __level_MB_map
{
	char	level[5];
	int		MB_Size;
}level_MB_map;

int SetupHWV4L2H264Enc(enc_config_t *pCfg);
int hw_v4l2h264enc_stop(enc_config_t *pCfg);
int hw_h264enc_encode (int frame_size, int index, struct v4l2_hwenc *v4l2_enc, enc_config_t *pCfg);
int hw_v4l2enc_get_input_handle(enc_config_t *pCfg, int32_t *index);

void setHWV4L2EncBitrate (enc_config_t *pCfg, unsigned int rate);
void setHWV4L2EncFramerate (enc_config_t *pCfg, unsigned int fps_n, unsigned int fps_d);
void setHWV4L2EncIDRFrameRequest (enc_config_t *pCfg);

#endif //_HWH264ENC_H_
