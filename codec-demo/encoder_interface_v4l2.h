#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "syna_std_headers.h"
#include "config.h"
#include "filereader.h"
#include "v4l2h264enc.h"
#include "v4l2hdmi.h"
#include "v4l2camera.h"
#include "neon_utils.h"

void startV4L2Encoder(sess_config_t *sess_cfg);
void stopV4L2Encoder(sess_config_t *sess_cfg);
void waitForV4L2EncoderShutdown(sess_config_t *sess_cfg);

// Set Feature Functions
void setBitrate (sess_config_t *sess_cfg, unsigned int rate);
void setFramerate (sess_config_t *sess_cfg, unsigned int fps_n, unsigned int fps_d);
void setIDRFrameRequest (sess_config_t *sess_cfg);

#define H264_ENC_IN_BUF_COUNT	MJPEG_OUT_BUF_COUNT
#define H264_ENC_OUT_BUF_COUNT	8

#define HDMI_IN_BUF_COUNT	MJPEG_OUT_BUF_COUNT

#define H264_ENC_IN_BUF_COUNT_MAX      20

typedef struct
{
	sess_config_t	*sess_cfg;
	int				is_running;
	pthread_t		feed_loop_id;
	tsem_t			*feed_loop_start_sem;
	int				pts;
	int				inc_pts;

	// Camera data
	v4l2cam_t		 v4l2Cam;

	tsem_t			*videoH264EncoderEventSem;
	tsem_t			*videoH264EncoderEofSem;

	int						h264EncInBufCount;
	int						h264EncOutBufCount;

	// GST TX pipeline data
	gst_tx_t gstTx;

	// HDMI RX data
	V4L2_HANDLE     *hdmi_rx_handle; //handle to interact with synav4l2 layer
	int hdmi_rx_fd;
	int hdmi_rx_v4l2_fmt;
	pthread_t hdmiRxMonitorThread;
	pthread_t hdmiRxThread;
	int hdmiRxStatus;
	int hdmi_rx_streaming;
	int	hdmi_rx_out_buf_count;
	tsem_t *hdmi_rx_start_sem;

	// FileReader
	filereader_t fileReader;

	// Statistics
	unsigned long current_fps;
	unsigned long total_frames;
	unsigned long prev_frames;

	pthread_t		feed_se_loop_id;
	pthread_mutex_t qt_rendering_buffer_mutex;
	int				qt_rendering_buffer_index;
	//tsem_t			*se_feed_loop_start_sem;
	tsem_t			*h264EncoderSeQueueSem;
	struct v4l2_enc *v4l2_enc;
	int mmap_virtual_input_sharedFDs[H264_ENC_IN_BUF_COUNT_MAX];
	/* To differentiate if the buffer is with us or not */
	bool bH264InBufWithEnc[H264_ENC_IN_BUF_COUNT_MAX];
	/* Input queried v4l2 buffer handle array */
	struct v4l2_buffer	pH264EncInV4l2Buf[H264_ENC_IN_BUF_COUNT_MAX];
	/* To differentiate if the buffer is with QTRenderingServer or not */
	bool bH264InBufWithQtRdS[H264_ENC_IN_BUF_COUNT_MAX];
	pthread_t	dqEnc_output_watcher_id;
	tsem_t			*dqbuf_enc_start_sem;

	// Bitrate Statistics
	unsigned long long current_bitrate;
	unsigned long long total_bitrate;
	unsigned long long prev_bitrate;

	// Flag for FPS Change request
	bool fpsChangeRequest;
	int fps_num;
	int fps_den;
	int last_index;

}priv_encoder_t;
#endif //_ENCODER_H_
