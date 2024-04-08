#ifndef _DECODER_INTERFACE_SW_H_
#define _DECODER_INTERFACE_SW_H_

#include "config.h"
#include "gbm-mem-utils.h"
#include "gstrxpipeline.h"
#include "swh264dec.h"

void startSWDecoder(sess_config_t *sess_cfg);
void stopSWDecoder(sess_config_t *sess_cfg);
void waitForSWDecoderShutdown(sess_config_t *sess_cfg);

typedef	void(*sample_received_fn)(void *pCfg, void *ptr, int size);

typedef struct
{
	sess_config_t		*sess_cfg;
	int 			is_running;

	pthread_t		feed_loop_id;
	struct ffmpeg_dec	*ffmpeg_dec;

	tsem_t			*feed_loop_start_sem;
	tsem_t			*videoH264DecoderEventSem;
	tsem_t			*videoH264DecoderEofSem;

	bool			bH264DecoderRecievedEOS;
	bool			bH264DecoderInitialized;
	bool			bH264DecoderConfigurationChanged;

	bool			bH264SWDecBufferWithUs[H264_SWDEC_OUT_BUF_COUNT];
	GBMBufInfo		*pH264DecOutSWBuffer[H264_SWDEC_OUT_BUF_COUNT];


	int			h264DecInBufCount;
	int			h264DecOutBufCount;
	int			h264DecInBufCurrCount;

	// Gstreamer Data
	gst_rx_t 		gstRx;

	long frame_duration_us;
	struct timeval 		time_stamp;

	/* Statistics */
	unsigned long		current_fps;
	unsigned long		total_frames;
	unsigned long		prev_frames;

	/* Display */
	//int window_displaying;
	pthread_t		feed_se_loop_id;
	pthread_mutex_t		qt_rendering_buffer_mutex;
	int			qt_rendering_buffer_index;
	tsem_t			*h264DecoderSeQueueSem;
}priv_sw_decoder_t;

#endif //_DECODER_INTERFACE_SW_H_
