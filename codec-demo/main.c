// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "main.h"

#define STATS_PERIOD 3

static void mysighandler(int signum) {
	LOG_DEF("%s:%d ===== signum=%d\n", __func__, __LINE__, signum);

	main_loop = 0;
	tsem_up(mainLoopSem);
}

static int parseOptions(int argc, char **argv)
{
	int res, ret;
	char ops[30] = "c:d:e:h";

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
		case 'd':
			strncpy(dri_card, optarg, 256);
			LOG_DEF("DRI CARD : %s\n", dri_card);
			break;
		case 'e':
			srtp_enabled = atoi(optarg);
			LOG_DEF("Srtp_enabled(%d)\n", srtp_enabled);
			break;
		case 'h':
			LOG_DEF("\n\nUsage : codec-demo -c <configuration_file> -d <dri_card_path> -e <0/1>\n");
			LOG_DEF("\n\n Arguments: \n");
			LOG_DEF("-c configuration file with path for ENC/DEC use cases\n");
			LOG_DEF("-d path of dri card\n");
			LOG_DEF("-e srtp enable (0 - Disabled, 1 - Enabled)\n");
			LOG_DEF("\n\n");
			LOG_DEF("Configuration File Structure : \n");
			LOG_DEF("Usage (ENC) < ENC_TYPE SRC SRC_PATH RESOLUTION FPS BITRATE SLICE_SIZE HOST_IP HOST_PORT DISPLAY PROFILE LEVEL INIT_QP >\n");
			LOG_DEF("Usage (DEC) < DEC_TYPE SRC SRC_PATH PORT RESOLUTION FPS DISPLAY > \n");
			LOG_DEF("Example - \n");
			LOG_DEF("HWENC CAM_NV12 /dev/video0 1920x1080 30 1024 1000 0.0.0.0 65001 5 0 MAIN 4.1 10\n");
			LOG_DEF("HWDEC FILE_H264 /home/root/Files/conf1_1080p.h264 8011 1920x1080 30 1 0\n");
			LOG_DEF("\n\n");
			LOG_DEF("ENC_TYPE \t:\t Available Encoders <HWENC, SWENC>\n");
			LOG_DEF("DEC_TYPE \t:\t Available Decoders <HWDEC, SWDEC>\n");
			LOG_DEF("SRC \t\t:\t Source options for ENC/DEC\n\t\t\t <CAM_H264, CAM_NV12, CAM_YUYV, FILE_H264, FILE_H265, FILE_AV1, FILE_VP8,\n\t\t\t FILE_VP9, NW_H264, HDMI_NV12, HDMI_YUYV >\n");
			LOG_DEF("SRC_PATH \t:\t Source Path details i.e. File path or v4l2 device path or NW IP address\n");
			LOG_DEF("RESOLUTION \t:\t Resolution of video in <width>x<height> format\n");
			LOG_DEF("FPS \t\t:\t FPS of video\n");
			LOG_DEF("BITRATE \t:\t Desired bitrate of video for encoding\n");
			LOG_DEF("HOST_<IP/PORT> \t:\t IP Address and port of UDP Receiver\n");
			LOG_DEF("DISPLAY \t:\t Display window ID of QT Rendering\n");
			LOG_DEF("PROFILE, LEVEL \t:\t Encoding Profile and Level of Encoder\n");
			LOG_DEF("\n\n");
			LOG_DEF("Note:\n");
			LOG_DEF("1. Each line in configuration file will denote one configuration to run\n");
			LOG_DEF("2. To comment use '#' in the start of the line\n");
			exit(-1);
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

	ret = parseOptions(argc, argv);

	/*Init IPC*/
	ret = InitClientIPC();
	if(ret < 0){
		LOG_ERR("InitClientIPC: Terminating...\n");
		goto err;
	}

	ret=InitGBM(dri_card);
	if(ret < 0){
		LOG_ERR("InitGBM: Terminating device :%s...\n", dri_card);
		goto err;
	}


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
	free(mainLoopSem);

	LOG_DEF("main exit done\n");
	return 0;
}
