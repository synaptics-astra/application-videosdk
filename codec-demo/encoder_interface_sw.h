#ifndef _ENCODER_INTERFACE_SW_H_
#define _ENCODER_INTERFACE_SW_H_

#include "config.h"
#include "filereader.h"
#include "v4l2camera.h"
#include "swh264enc.h"
#include "gsttxpipeline.h"
#include "gbm-mem-utils.h"
#include "neon_utils.h"

void startSWEncoder(sess_config_t *sess_cfg);
void stopSWEncoder(sess_config_t *sess_cfg);
void waitForSWEncoderShutdown(sess_config_t *sess_cfg);

#define H264_SWENC_IN_BUF_COUNT	4

typedef struct
{
	sess_config_t		*sess_cfg;
	int			is_running;
	pthread_t		feed_loop_id;
	tsem_t			*feed_loop_start_sem;
	int			pts;
	int			inc_pts;

	// Camera data
	v4l2cam_t		v4l2Cam;

	// GST TX pipeline data
	gst_tx_t		gstTx;

	// FileReader
	filereader_t		fileReader;

	// Statistics
	unsigned long		current_fps;
	unsigned long		total_frames;
	unsigned long		prev_frames;

	pthread_mutex_t		qt_rendering_buffer_mutex;
	int			qt_rendering_buffer_index;
	tsem_t			*h264EncoderSeQueueSem;
	struct ffmpeg_enc *ffmpeg_enc;
	/* To differentiate if the buffer is with us or not */
	bool			bH264InBufWithEnc[H264_SWENC_IN_BUF_COUNT];
	/* Input queried Amp buffer handle array */
	GBMBufInfo		*pH264EncInSWBuffer[H264_SWENC_IN_BUF_COUNT]; //TODO: Need to avoid memcpy in tmpBuf.
	/* To differentiate if the buffer is with QTRenderingServer or not */
	bool			bH264InBufWithQtRdS[H264_SWENC_IN_BUF_COUNT];

	// Bitrate Statistics
	unsigned long long	current_bitrate;
	unsigned long long	total_bitrate;
	unsigned long long	prev_bitrate;

	int last_index;

}priv_sw_encoder_t;
#endif //_ENCODER_INTERACE_SW_H_
