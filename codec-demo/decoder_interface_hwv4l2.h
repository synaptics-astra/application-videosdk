#ifndef _DECODER_INTERFACE_HWV4L2_H_
#define _DECODER_INTERFACE_HWV4L2_H_

#include "config.h"
#include "gstrxpipeline.h"

void startHWV4L2Decoder(sess_config_t *sess_cfg);
void stopHWV4L2Decoder(sess_config_t *sess_cfg);
void waitForHWV4L2DecoderShutdown(sess_config_t *sess_cfg);


#define H264_DEC_OUT_BUF_COUNT_DEF 7 /*Default*/
#define H264_DEC_IN_BUF_COUNT_DEF 8

/* Increase H264_DEC_OUT_BUF_COUNT_MAX to 25 to accomodate for files requires higher number of buffers */
#define H264_DEC_OUT_BUF_COUNT_MAX 25 /*Maximun*/
#define H264_DEC_IN_BUF_COUNT_MAX 20

typedef	void(*hw_sample_received_fn)(void *pCfg, void *ptr, int size);

typedef struct
{
	sess_config_t		*sess_cfg;
	int					is_running;

	pthread_t			feed_loop_id;
	pthread_t			v4l2_event_watcher_id;
	pthread_t			dqbuf_capture_watcher_id;
	pthread_t			dqbuf_output_watcher_id;

	// V4L2 H264 Decoder data
	int					hVideoH264Decoder;

	void				**mmap_virtual_input;    /* Pointer tab of input AUs */
	int					*mmap_size_input;

	void				***mmap_virtual_output;    /* Pointer tab of output frames */
	int					**mmap_size_output;

	int					current_nb_buf_input;
	int					current_nb_buf_output;
	unsigned int		src_res_change;  /*To mark if resolution change is detected*/

	tsem_t				*feed_loop_start_sem;
	tsem_t				*videoH264DecoderEventSem;
	tsem_t				*videoH264DecoderEofSem;

	bool				bH264DecoderRecievedEOS;
	bool				bH264DecoderInitialized;
	bool				bH264DecoderConfigurationChanged;

	bool				bH264DecInBufferWithUs[H264_DEC_IN_BUF_COUNT_MAX];
	struct v4l2_buffer	pH264DecInV4L2Buffer[H264_DEC_IN_BUF_COUNT_MAX];
	int					h264DecInBufCount;
	int					h264DecOutBufCount;
	int					h264DecInBufCurrCount;

	// Gstreamer Data
	gst_rx_t			gstRx;

	int					SPS_FOUND;
	int					PPS_FOUND;

	long				frame_duration_us;
	struct timeval		time_stamp;

	/* Statistics */
	unsigned long		current_fps;
	unsigned long		total_frames;
	unsigned long		prev_frames;

	/* Display */
	//VOUT_BUFFER		voutBuf[H264_DEC_OUT_BUF_COUNT_MAX];
	//int				window_displaying;
	pthread_t			feed_se_loop_id;
	pthread_mutex_t		qt_rendering_buffer_mutex;
	int					qt_rendering_buffer_index;
	//tsem_t			*se_feed_loop_start_sem;
	tsem_t				*h264DecoderSeQueueSem;
	int					mmap_virtual_output_sharedFDs[H264_DEC_OUT_BUF_COUNT_MAX][VIDEO_MAX_PLANES];
	/* Hold the total number of planes from the driver during S_FMT iotcl */
	int					number_output_planes;
}priv_hw_v4l2_decoder_t;

#endif //_DECODER_INTERFACE_HWV4L2_H_
