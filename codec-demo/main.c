#include "main.h"

#define STATS_PERIOD 3
#define DRM_DEV "/dev/dri/card0"

static void mysighandler(int signum) {
	LOG_DEF("%s:%d ===== signum=%d\n", __func__, __LINE__, signum);

	main_loop = 0;
	tsem_up(mainLoopSem);
}

static int parseOptions(int argc, char **argv)
{
	int res, ret;
	char ops[30] = "c:e:";

	while ((res = getopt(argc, argv, ops)) >= 0) {
		switch (res) {
		case 'c':
			strncpy(config_file, optarg, 256);
			LOG_DEF("Config file: %s\n", config_file);
			ret = parseConfigFile(&codec_config, config_file);
			if (ret < 0)
				return -1;
			printConfig(&codec_config);
			break;
		case 'e':
			srtp_enabled = atoi(optarg);
			LOG_DEF("Srtp_enabled(%d)\n", srtp_enabled);
			break;
		default:
			LOG_ERR("unsupport arg %c\n",res);
			return -1;
			break;
		}
	}
	return 0;
}

void dumpStats (int signum) {

	if (main_loop == 0)
		return;
	static int init =0;
	StatusMsg stMessage;

	if(init == 0) {

		stMessage.m_total_sessions = 0;
		for(int i = 0; i < codec_config.num_of_sessions; i++)
		{
			sess_config_t *sess_cfg = &codec_config.sess_cfg[i];
			dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;
			if((sess_cfg->type == SESS_SWDEC) && (pCfg->src == CAM_H264))
				continue;
			else
				stMessage.m_total_sessions++;
		}
		SendStatusInitMessageToUI(&stMessage);
		init =1;
	}

	for(int i = 0; i < codec_config.num_of_sessions; i++)
	{
		sess_config_t *sess_cfg = &codec_config.sess_cfg[i];

		switch(sess_cfg->type)
		{
#ifdef AMP_SUPPORT
			case SESS_AMPENC:
				{
					enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
					priv_encoder_t *pEnc = (priv_encoder_t *)pCfg->priv;

					if (pEnc != NULL) {
						pEnc->current_fps = pEnc->total_frames - pEnc->prev_frames;
						pEnc->prev_frames = pEnc->total_frames;
						pEnc->current_bitrate = (pEnc->total_bitrate - pEnc->prev_bitrate) * 8 / 1000;
						pEnc->prev_bitrate = pEnc->total_bitrate;

						if (stats_enable_flag) {
							LOG_DEF("\n====AMP Encoder Session [%d] ========\n", i);
							LOG_DEF("Resolution  : %d x %d\n", pCfg->width, pCfg->height);
							LOG_DEF("FPS         : %f FPS\n", pEnc->current_fps/(float)STATS_PERIOD);
							LOG_DEF("Bitrate     : %llu kbps\n", pEnc->current_bitrate/STATS_PERIOD);
							LOG_DEF("Total frames: %lu\n", pEnc->total_frames);
							LOG_DEF("=======================================\n");
						}
					}
					stMessage.m_sessionID = (pCfg->instance_id+1);
					stMessage.m_optionType = AMPENC;
					stMessage.m_width = pCfg->width;
					stMessage.m_height = pCfg->height;
					stMessage.m_fps = pEnc->current_fps/(float)STATS_PERIOD;
					SendStatusMessageToUI(&stMessage);
				}
				break;

			case SESS_AMPDEC:
				{
					dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;
					priv_v4l2_decoder_t *pDec = (priv_v4l2_decoder_t *)pCfg->priv;

					if (pDec != NULL) {
						pDec->current_fps = pDec->total_frames - pDec->prev_frames;
						pDec->prev_frames = pDec->total_frames;

						if (stats_enable_flag) {
							LOG_DEF("\n====AMP Decoder Session [%d] ========\n", i);
							LOG_DEF("Resolution  : %d x %d\n", pCfg->width, pCfg->height);
							LOG_DEF("FPS         : %f FPS\n", pDec->current_fps/(float)STATS_PERIOD);
							LOG_DEF("Total frames: %lu\n", pDec->total_frames);
							LOG_DEF("=======================================\n");
						}
					}
					stMessage.m_sessionID = (pCfg->instance_id+1);
					stMessage.m_optionType = AMPDEC;
					stMessage.m_width = pCfg->width;
					stMessage.m_height = pCfg->height;
					stMessage.m_fps = pDec->current_fps/(float)STATS_PERIOD;
					SendStatusMessageToUI(&stMessage);
				}
				break;
#endif

			case SESS_SWDEC:
				{
					dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;
					if(pCfg->src == CAM_H264)
						break;			// Don't send Stats as this Decoder Session is part of CAM_H264 Encoder Session
					priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;

					pDec->current_fps = pDec->total_frames - pDec->prev_frames;
					pDec->prev_frames = pDec->total_frames;

					if (stats_enable_flag) {
						LOG_DEF("\n==== SW Decoder Session [%d] ========\n", i);
						LOG_DEF("Resolution  : %d x %d\n", pCfg->width, pCfg->height);
						LOG_DEF("FPS         : %f FPS\n", pDec->current_fps/(float)STATS_PERIOD);
						LOG_DEF("Total frames: %lu\n", pDec->total_frames);
						LOG_DEF("=======================================\n");
					}
					stMessage.m_sessionID = (pCfg->instance_id+1);
					stMessage.m_optionType = SWDEC;
					stMessage.m_width = pCfg->width;
					stMessage.m_height = pCfg->height;
					stMessage.m_fps = pDec->current_fps/(float)STATS_PERIOD;
					SendStatusMessageToUI(&stMessage);
				}
				break;

			case SESS_SWENC:
				{
					enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
					priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;

					if (pEnc != NULL) {
						pEnc->current_fps = pEnc->total_frames - pEnc->prev_frames;
						pEnc->prev_frames = pEnc->total_frames;
						pEnc->current_bitrate = (pEnc->total_bitrate - pEnc->prev_bitrate) * 8 / 1000;
						pEnc->prev_bitrate = pEnc->total_bitrate;

						if (stats_enable_flag) {
							LOG_DEF("\n==== SW Encoder Session [%d] ========\n", i);
							LOG_DEF("Resolution  : %d x %d\n", pCfg->width, pCfg->height);
							LOG_DEF("FPS         : %f FPS\n", pEnc->current_fps/(float)STATS_PERIOD);
							LOG_DEF("Bitrate     : %llu kbps\n", pEnc->current_bitrate/STATS_PERIOD);
							LOG_DEF("Total frames: %lu\n", pEnc->total_frames);
							LOG_DEF("=======================================\n");
						}
					}
					stMessage.m_sessionID = (pCfg->instance_id+1);
					stMessage.m_optionType = SWENC;
					stMessage.m_width = pCfg->width;
					stMessage.m_height = pCfg->height;
					stMessage.m_fps = pEnc->current_fps/(float)STATS_PERIOD;
					SendStatusMessageToUI(&stMessage);
				}
				break;

#ifdef V4L2_SUPPORT
			case SESS_HWENC:
				{
					enc_config_t *pCfg = &sess_cfg->sess.enc_cfg;
					priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfg->priv;

					if (pEnc != NULL) {
						pEnc->current_fps = pEnc->total_frames - pEnc->prev_frames;
						pEnc->prev_frames = pEnc->total_frames;
						pEnc->current_bitrate = (pEnc->total_bitrate - pEnc->prev_bitrate) * 8 / 1000;
						pEnc->prev_bitrate = pEnc->total_bitrate;

						if (stats_enable_flag) {
							LOG_DEF("\n==== HW Encoder Session [%d] ========\n", i);
							LOG_DEF("Resolution  : %d x %d\n", pCfg->width, pCfg->height);
							LOG_DEF("FPS         : %f FPS\n", pEnc->current_fps/(float)STATS_PERIOD);
							LOG_DEF("Bitrate     : %llu kbps\n", pEnc->current_bitrate/STATS_PERIOD);
							LOG_DEF("Total frames: %lu\n", pEnc->total_frames);
							LOG_DEF("=======================================\n");
						}
					}
					stMessage.m_sessionID = (pCfg->instance_id+1);
					stMessage.m_optionType = HWENC;
					stMessage.m_width = pCfg->width;
					stMessage.m_height = pCfg->height;
					stMessage.m_fps = pEnc->current_fps/(float)STATS_PERIOD;
					SendStatusMessageToUI(&stMessage);
				}
				break;

			case SESS_HWDEC:
				{
					dec_config_t *pCfg = &sess_cfg->sess.dec_cfg;
					priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfg->priv;

					if (pDec != NULL) {
						pDec->current_fps = pDec->total_frames - pDec->prev_frames;
						pDec->prev_frames = pDec->total_frames;

						if (stats_enable_flag) {
							LOG_DEF("\n==== HW Decoder Session [%d] ========\n", i);
							LOG_DEF("Resolution  : %d x %d\n", pCfg->width, pCfg->height);
							LOG_DEF("FPS         : %f FPS\n", pDec->current_fps/(float)STATS_PERIOD);
							LOG_DEF("Total frames: %lu\n", pDec->total_frames);
							LOG_DEF("=======================================\n");
						}
					}
					stMessage.m_sessionID = (pCfg->instance_id+1);
					stMessage.m_optionType = HWDEC;
					stMessage.m_width = pCfg->width;
					stMessage.m_height = pCfg->height;
					stMessage.m_fps = pDec->current_fps/(float)STATS_PERIOD;
					SendStatusMessageToUI(&stMessage);
				}
				break;
#endif
			default:
				break;
		}
	}
}

void setupStats()
{
	struct itimerval val;

	/* Setup an alarm for 1sec */
	signal (SIGALRM, dumpStats);

	val.it_interval.tv_sec  = STATS_PERIOD;      /* ie. every second */
	val.it_interval.tv_usec = 0;
	val.it_value.tv_sec     = 1;      /* initialise counter */
	val.it_value.tv_usec    = 0;

	setitimer(ITIMER_REAL, &val, 0);
}

int main(int argc, char** argv)
{
	LOG_DEF("lets start\n");

	int ret = 0;

	mainLoopSem = (tsem_t*) malloc(sizeof(tsem_t));
	tsem_init(mainLoopSem, 0);

	// Initialize signal handler for QUIT signal
	signal(SIGINT, mysighandler);

	/*Init IPC*/
	ret = InitClientIPC();
	if(ret < 0){
		LOG_ERR("InitClientIPC: Terminating...\n");
		goto err;
	}

	ret=InitGBM(DRM_DEV);
	if(ret < 0){
		LOG_ERR("InitGBM: Terminating device :%s...\n",DRM_DEV);
		goto err;
	}

#ifdef AMP_SUPPORT
	ret = InitServerIPC(&codec_config);
	if (ret < 0){
		LOG_ERR("InitServerIPC: Terminating...\n");
		goto err;
	}
#endif

	ret = parseOptions(argc, argv);

#ifdef LOCAL_LOOPBACK_MODE
	codec_config.sharedH264QueueSem = (tsem_t *) calloc(1,sizeof(tsem_t));
	tsem_init(codec_config.sharedH264QueueSem, 0);
	LOG_SEQ("Fn(%s): sharedH264QueueSem(%p)\n", __func__, codec_config.sharedH264QueueSem);
	codec_config.sharedH264Queue = (queue_t *) calloc(1,sizeof(queue_t));
	queue_init(codec_config.sharedH264Queue);
#endif

	codec_config.sharedH264EncoderQueueSem = (tsem_t *) calloc(1,sizeof(tsem_t));
	tsem_init(codec_config.sharedH264EncoderQueueSem, 0);
	LOG_SEQ("Fn(%s): sharedH264EncoderQueueSem(%p)\n", __func__, codec_config.sharedH264EncoderQueueSem);
	codec_config.sharedH264EncoderQueue = (queue_t *) calloc(1,sizeof(queue_t));
	queue_init(codec_config.sharedH264EncoderQueue);

	if (ret == 0 && codec_config.config_parsed)
	{
#ifdef AMP_SUPPORT
		ret = SynaV4L2_Init();
		if (ret != V4L2_SUCCESS) {
			LOG_ERR("Error in v4l2 init\n");
			goto err;
		}
#endif
		for(int i=0; i<codec_config.num_of_sessions; i++)
		{
			startInstance(&codec_config.sess_cfg[i]);
		}

		setupStats();

		main_loop = 1;
		// Run till we get QUIT signal from command prompt
		tsem_down(mainLoopSem);

		for(int i=0; i<codec_config.num_of_sessions; i++)
		{
			stopInstance(&codec_config.sess_cfg[i]);
			waitForInstanceShutdown(&codec_config.sess_cfg[i]);
		}
	}

err:

#ifdef LOCAL_LOOPBACK_MODE
	queue_deinit(codec_config.sharedH264Queue);
	free(codec_config.sharedH264QueueSem);
	free(codec_config.sharedH264Queue);
#endif

	queue_deinit(codec_config.sharedH264EncoderQueue);
	free(codec_config.sharedH264EncoderQueueSem);
	free(codec_config.sharedH264EncoderQueue);

	/*DeInit QT Rendering Client IPC &
	Send deinit message to QRS*/
	DeInitClientIPC();
	DeInitGBM();
#ifdef AMP_SUPPORT
	/*DeInit Video tuner Server IPC*/
	DeInitServerIPC();

	ret = SynaV4L2_Deinit();
	if (ret != V4L2_SUCCESS)
	{
		LOG_ERR("SynaV4L2_Deinit Failed\n");
	}
#endif
	free(mainLoopSem);

	LOG_DEF("main exit done\n");
	return 0;
}
