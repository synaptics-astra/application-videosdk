// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "syna_std_headers.h"

#define MAX_IP_ADDR_STR 	16
#define MAX_FILE_PATH_STR	100
#define MAX_SESSIONS		30
#define MAX_DISPLAY_WINDOWS	2
#define MAX_EXT_ENC_SESSIONS	2

#ifdef LOCAL_LOOPBACK_MODE
#define LOCAL_LOOPBACK_ENC_INST 0
#define LOCAL_LOOPBACK_DEC_INST 1
#endif

#define MAX_H264_ENC_PROFILE_LENGTH	24
#define MAX_H264_ENC_LEVEL_LENGTH	5

typedef enum
{
	SESS_SWDEC,
	SESS_SWENC,
	SESS_HWENC,
	SESS_HWDEC,
	SESS_MAX
}inst_type_e;

typedef enum
{
	CAM_MJPEG,
	CAM_NV12,
	HDMI_NV12,
	FILE_NV12,
	ENC_EXT,
	CAM_YUYV,
	FILE_YUYV,
	HDMI_UYVY,
	HDMI_EXT,    /* hdmi-rx-ext i.e. hdmi(4k)->vout && hdmi->downscale(1080p)->enc */
	CAM_H264,
	ENC_SRC_MAX
}enc_src_type_e;

typedef struct
{
	enc_src_type_e		src;
	int			instance_id;
	int			width;
	int			height;
	int			fps;
	char			path[MAX_FILE_PATH_STR];
	int			bitrate;
	char			ip[MAX_IP_ADDR_STR];
	int			port;
	int			slice_size_bytes;
	int			display_window;
	void			*priv;
	void			*codec_config;
	char			profile[MAX_H264_ENC_PROFILE_LENGTH];
	char			level[MAX_H264_ENC_LEVEL_LENGTH];
	int			encInitQP;
	int 			simcast_en;
}enc_config_t;

typedef enum
{
	NW_H264,
	FILE_H264,
	FILE_H265,
	FILE_VP8,
	FILE_VP9,
	FILE_AV1,
}dec_src_type_e;

typedef enum{
	PIXEL_NV12,
	PIXEL_YUYV,
	PIXEL_UYVY,
	PIXEL_NV12M,
	PIXEL_ARGB
}InputPixelFormat;

typedef struct
{
	dec_src_type_e		src;
	int			instance_id;
	int			width;
	int			height;
	int			format;
	int			fps;
	char			path[MAX_FILE_PATH_STR];
	int			port;
	int			display_window;
	void			*priv;
	void			*codec_config;
}dec_config_t;

typedef struct
{
	inst_type_e	type;
	union
	{
		enc_config_t	enc_cfg;
		dec_config_t	dec_cfg;
	}sess;
}sess_config_t;

typedef struct
{
	int 			config_parsed;
	int 			num_of_sessions;
	sess_config_t 		sess_cfg[MAX_SESSIONS];
	void 			*ext_enc_sess_cfg[MAX_EXT_ENC_SESSIONS];
	int 			num_ext_enc_sessions;
	int 			num_ext_enc_sessions_initialized;
	tsem_t 			*sharedH264EncoderQueueSem;
	queue_t 		*sharedH264EncoderQueue;
#ifdef LOCAL_LOOPBACK_MODE
	tsem_t 			*sharedH264QueueSem;
	queue_t 		*sharedH264Queue;
#endif
}codec_config_t;

typedef	void(*omx_callback_fn)(void *pCfg, void *pBuffer);
int parseConfigFile(codec_config_t *codec_config, char *config_file);
int printConfig(codec_config_t *codec_config);
#endif //_CONFIG_H_
