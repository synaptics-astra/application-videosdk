#ifndef _V4L2H264ENC_H_
#define _V4L2H264ENC_H_

#include "config.h"
#include "gsttxpipeline.h"
#include "encoder_interface_v4l2.h"

#define ENCODER 1
// Number of input buffers
#define NB_BUF_INPUT	8
// Number of output buffers
#define NB_BUF_OUTPUT	4
#define FRAME_BUF_SIZE (1920*1080*4)

#define ENC_EXT_CONTROLS_PARAM_NUM 10

struct v4l2_enc {
	// Global handle for the application to interact with SynaV4l2 library
	V4L2_HANDLE		*handle;
	FILE			*fpo;
	FILE			*fpo_sim[MAX_EXT_ENC_SESSIONS];
	void			**mmap_virt_inp;	 /* Pointer to input frames pointers */
	int				*mmap_size_inp;

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
	v4l2_simulcast *simulcast;
	int 			out_buf_count;
};

int setupEncEXt(enc_config_t *pCfg);
int SetupV4L2H264Enc(enc_config_t *pCfg);
int v4l2h264enc_stop(enc_config_t *pCfg);
int v4l2h264enc_encode (int frame_size, int index, struct v4l2_enc *v4l2_enc, enc_config_t *pCfg);
int v4l2enc_get_input_handle(enc_config_t *pCfg, int32_t *index);

void setV4L2EncBitrate (enc_config_t *pCfg, unsigned int rate);
void setV4L2EncFramerate (enc_config_t *pCfg, unsigned int fps_n, unsigned int fps_d);
void setV4L2EncIDRFrameRequest (enc_config_t *pCfg);

#endif //_V4L2H264ENC_H_
