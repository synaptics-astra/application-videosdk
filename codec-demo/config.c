// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "config.h"
#include "UIInterface.h"

int parseConfigFile(codec_config_t *codec_config, char *config_file)
{
	codec_config->config_parsed = 0;
	int init=0;

	if(access(config_file, R_OK) != -1) {
		// file exists
		LOG_DEF("Parsing configuration file: %s\n", config_file);
	} else {
		// file doesn't exist
		LOG_ERR("No configuration file found: %s\n", config_file);
		return -1;
	}

	FILE* fp = fopen(config_file, "r");

	if(fp == NULL)
	{
		LOG_ERR("Cannot open configuration file: %s\n", config_file);
		return -1;
	}

	FILE* fPtr = NULL;
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		LOG_SEQ("%s\n", line);

		if(line != NULL)
		{
			//char delim[] = " ";
			//char *ptr = strtok(line, delim);

			if (codec_config->num_of_sessions >= MAX_SESSIONS)
			{
				LOG_ERR("Sessions are more than maximum allowed instances - %d\n", MAX_SESSIONS);
				break;
			}

			if(strncmp(line, "SWDEC", 5) == 0)
			{
				char src[10] = {0};
				char path[MAX_FILE_PATH_STR] = {0};
				int width;
				int height;
				int fps;
				int port;
				int display;

				//SWDEC NW_H264 172.28.5.9 8006 1280x720 30 (3 to 10)
				sscanf(line, "SWDEC %s %s %d %dx%d %d %d", src, path, &port, &width, &height, &fps, &display);

				/*TODO: Vadidation of data */
				codec_config->sess_cfg[codec_config->num_of_sessions].type = SESS_SWDEC;
				dec_config_t *pCfg = &codec_config->sess_cfg[codec_config->num_of_sessions].sess.dec_cfg;

				memset(pCfg, 0, sizeof(dec_config_t));

				if(strcmp(src, "NW_H264") == 0)
					pCfg->src = NW_H264;
				else if(strcmp(src, "FILE_H264") == 0)
					pCfg->src = FILE_H264;
				else
				{
					LOG_ERR("Invalid decoder src entry: - %s\n", line);
					continue;
				}

				/* Supporting max 8 display windows, windows 3 onwards on OSD */
				if (display < 0 || display > MAX_SESSIONS)
				{
					pCfg->display_window = 0;
				}
				else
					pCfg->display_window = display;

				pCfg->width = width;
				pCfg->height = height;
				pCfg->fps = fps;
				pCfg->port = port;
				strncpy(pCfg->path, path, MAX_FILE_PATH_STR);

				pCfg->instance_id = codec_config->num_of_sessions;
				codec_config->num_of_sessions++;
				pCfg->codec_config = codec_config;

				if(pCfg->display_window > 0 && pCfg->display_window < MAX_SESSIONS){
					registerClientWindow();
				}
			}
			else if(strncmp(line, "SWENC", 5) == 0)
			{
				char src[10];
				char path[MAX_FILE_PATH_STR];
				char ip[MAX_IP_ADDR_STR];
				int width;
				int height;
				int fps;
				int bitrate;
				int port;
				int slice_size_bytes;
				int display;
				char profile[MAX_H264_ENC_PROFILE_LENGTH];
				char level[MAX_H264_ENC_LEVEL_LENGTH];
				int InitQP;
				int simcast_en = 0;

				//ENC CAM_NV12 /dev/video0 1920x1080 30 4096 1000 172.28.5.9 8001 0 0
				sscanf(line, "SWENC %s %s %dx%d %d %d %d %s %d %d %s %s %d", src, path, &width, &height, &fps, &bitrate, &slice_size_bytes, ip, &port, &display, profile, level, &InitQP);

				/*TODO: Vadidation of data */
				codec_config->sess_cfg[codec_config->num_of_sessions].type = SESS_SWENC;
				enc_config_t *pCfg = &codec_config->sess_cfg[codec_config->num_of_sessions].sess.enc_cfg;
				memset(pCfg, 0, sizeof(enc_config_t));

				if(strcmp(src, "CAM_H264") == 0)
					pCfg->src = CAM_H264;
				else if(strcmp(src, "CAM_NV12") == 0)
					pCfg->src = CAM_NV12;
				else if(strcmp(src, "CAM_YUYV") == 0)
					pCfg->src = CAM_YUYV;
				else if ((strcmp(src, "FILE_NV12") == 0) || (strcmp(src, "FILE_YUYV") == 0))
				{
					fPtr = fopen(path, "r");
					if( fPtr == NULL)
					{
						LOG_ERR("Cannot open encoder input file: %s\n", path);
						exit(-1);
					} else
						fclose(fPtr);
					if(strcmp(src, "FILE_NV12") == 0)
						pCfg->src = FILE_NV12;
					else
						pCfg->src = FILE_YUYV;
				}
				else
				{
					LOG_ERR("SW encoder does not support src:%s entry: - %s\n",src, line);
					continue;
				}

				if (display < 0 || display > MAX_SESSIONS || pCfg->src == CAM_H264)
				{
					pCfg->display_window = 0;
				}
				else
					pCfg->display_window = display;

				pCfg->width = width;
				pCfg->height = height;
				pCfg->fps = fps;
				pCfg->bitrate = bitrate;
				pCfg->slice_size_bytes = slice_size_bytes;
				pCfg->port = port;
				strncpy(pCfg->path, path, MAX_FILE_PATH_STR);
				strncpy(pCfg->ip, ip, MAX_IP_ADDR_STR);
				pCfg->instance_id = codec_config->num_of_sessions;
				strncpy(pCfg->profile, profile, MAX_H264_ENC_PROFILE_LENGTH);
				strncpy(pCfg->level, level, MAX_H264_ENC_LEVEL_LENGTH);
				pCfg->encInitQP = InitQP;
				codec_config->num_of_sessions++;
				pCfg->codec_config = codec_config;
				pCfg->simcast_en = simcast_en;


				if(pCfg->display_window > 0 && pCfg->display_window < MAX_SESSIONS && pCfg->src != CAM_H264){
					registerClientWindow();
				}

				if(pCfg->src == CAM_H264)
				{
					codec_config->sess_cfg[codec_config->num_of_sessions].type = SESS_SWDEC;
					dec_config_t *pDCfg = &codec_config->sess_cfg[codec_config->num_of_sessions].sess.dec_cfg;

					memset(pDCfg, 0, sizeof(dec_config_t));
					pDCfg->src = CAM_H264;

					if (display < 0 || display > MAX_SESSIONS)
					{
						pDCfg->display_window = 0;
					}
					else
						pDCfg->display_window = display;

					pDCfg->width = width;
					pDCfg->height = height;
					pDCfg->fps = fps;
					pDCfg->port = port;
					strncpy(pDCfg->path, path, MAX_FILE_PATH_STR);

					pDCfg->instance_id = codec_config->num_of_sessions;
					codec_config->num_of_sessions++;
					pDCfg->codec_config = codec_config;

					if(pDCfg->display_window > 0 && pDCfg->display_window < MAX_SESSIONS){
						registerClientWindow();
					}
				}

			}
            else if(strncmp(line, "HWENC", 5) == 0)
            {
                char src[10];
                char path[MAX_FILE_PATH_STR];
                char ip[MAX_IP_ADDR_STR];
                int width;
                int height;
                int fps;
                int bitrate;
                int port;
                int slice_size_bytes;
                int display;
                char profile[MAX_H264_ENC_PROFILE_LENGTH];
                char level[MAX_H264_ENC_LEVEL_LENGTH];
                int InitQP;
                int simcast_en = 0;

                //HWENC CAM_NV12 /dev/video0 1920x1080 30 4096 1000 172.28.5.9 8001 0 0
                sscanf(line, "HWENC %s %s %dx%d %d %d %d %s %d %d %s %s %d", src, path, &width, &height, &fps, &bitrate, &slice_size_bytes, ip, &port, &display, profile, level, &InitQP);

#ifdef CURR_MACH_MYNA2
				LOG_ERR("HWENC is not supported\n");
				continue;
#endif
                /*TODO: Vadidation of data */
                codec_config->sess_cfg[codec_config->num_of_sessions].type = SESS_HWENC;
                enc_config_t *pCfg = &codec_config->sess_cfg[codec_config->num_of_sessions].sess.enc_cfg;
                memset(pCfg, 0, sizeof(enc_config_t));

                if(strcmp(src, "CAM_NV12") == 0)
                    pCfg->src = CAM_NV12;
                else if(strcmp(src, "CAM_YUYV") == 0)
                    pCfg->src = CAM_YUYV;
                else if(strcmp(src, "HDMI_UYVY") == 0)
                    pCfg->src = HDMI_UYVY;
                else if(strcmp(src, "HDMI_NV12") == 0)
                    pCfg->src = HDMI_NV12;
                else if ((strcmp(src, "FILE_NV12") == 0) || (strcmp(src, "FILE_YUYV") == 0))
                {
                    fPtr = fopen(path, "r");
                    if( fPtr == NULL)
                    {
                        LOG_ERR("Cannot open encoder input file: %s\n", path);
                        exit(-1);
                    } else
                        fclose(fPtr);
                    if(strcmp(src, "FILE_NV12") == 0)
                        pCfg->src = FILE_NV12;
                    else
                        pCfg->src = FILE_YUYV;
                }
                else
                {
                    LOG_ERR("HW encoder does not support src:%s entry: - %s\n",src, line);
                    continue;
                }

                if (display < 0 || display > MAX_SESSIONS)
                {
                    pCfg->display_window = 0;
                }
                else
                    pCfg->display_window = display;

                pCfg->width = width;
                pCfg->height = height;
                pCfg->fps = fps;
                pCfg->bitrate = bitrate;
                pCfg->slice_size_bytes = slice_size_bytes;
                pCfg->port = port;
                strncpy(pCfg->path, path, MAX_FILE_PATH_STR);
                strncpy(pCfg->ip, ip, MAX_IP_ADDR_STR);
                pCfg->instance_id = codec_config->num_of_sessions;
                strncpy(pCfg->profile, profile, MAX_H264_ENC_PROFILE_LENGTH);
                strncpy(pCfg->level, level, MAX_H264_ENC_LEVEL_LENGTH);
                pCfg->encInitQP = InitQP;
                codec_config->num_of_sessions++;
                pCfg->codec_config = codec_config;
                pCfg->simcast_en = simcast_en;


                if(pCfg->display_window > 0 && pCfg->display_window < MAX_SESSIONS){
                    registerClientWindow();
                }

            }
            else if(strncmp(line, "HWDEC", 5) == 0)
			{
				char src[10];
				char path[MAX_FILE_PATH_STR];
				int width;
				int height;
				int fps;
				int port;
				int display;

				//HWDEC NW_H264 172.28.5.9 8006 1280x720 30 0 0
				sscanf(line, "HWDEC %s %s %d %dx%d %d %d", src, path, &port, &width, &height, &fps, &display);

#ifdef CURR_MACH_MYNA2
				LOG_ERR("HWDEC is not supported\n");
				continue;
#endif
				/*TODO: Vadidation of data */
				codec_config->sess_cfg[codec_config->num_of_sessions].type = SESS_HWDEC;
				dec_config_t *pCfg = &codec_config->sess_cfg[codec_config->num_of_sessions].sess.dec_cfg;

				memset(pCfg, 0, sizeof(dec_config_t));

				if(strcmp(src, "NW_H264") == 0)
					pCfg->src = NW_H264;	
				else if(strcmp(src, "FILE_H264") == 0)
				{
					fPtr = fopen(path, "r");
					if( fPtr == NULL)
					{
						LOG_ERR("Cannot open Decoder input file: %s\n", path);
						exit(-1);
					} else
						fclose(fPtr);
					pCfg->src = FILE_H264;
				}
				else if(strcmp(src, "FILE_H265") == 0)
				{
					fPtr = fopen(path, "r");
					if( fPtr == NULL)
					{
						LOG_ERR("Cannot open Decoder input file: %s\n", path);
						exit(-1);
					} else
						fclose(fPtr);
					pCfg->src = FILE_H265;
				}
				else if(strcmp(src, "FILE_VP8") == 0)
				{
					fPtr = fopen(path, "r");
					if( fPtr == NULL)
					{
						LOG_ERR("Cannot open Decoder input file: %s\n", path);
						exit(-1);
					} else
						fclose(fPtr);
					pCfg->src = FILE_VP8;
				}
				else if(strcmp(src, "FILE_VP9") == 0)
				{
					fPtr = fopen(path, "r");
					if( fPtr == NULL)
					{
						LOG_ERR("Cannot open Decoder input file: %s\n", path);
						exit(-1);
					} else
						fclose(fPtr);
					pCfg->src = FILE_VP9;
				}
				else if(strcmp(src, "FILE_AV1") == 0)
				{
					fPtr = fopen(path, "r");
					if( fPtr == NULL)
					{
						LOG_ERR("Cannot open Decoder input file: %s\n", path);
						exit(-1);
					} else
						fclose(fPtr);
					pCfg->src = FILE_AV1;
				}
				else
				{
					LOG_ERR("Invalid HW decoder src entry: - %s\n", line);
					continue;
				}

				/* Supporting max 8 display windows, windows 3 onwards on OSD */
				if (display < 0 || display > MAX_SESSIONS)
				{
					pCfg->display_window = 0;
				}
				else
					pCfg->display_window = display;

				pCfg->width = width;
				pCfg->height = height;
				pCfg->fps = fps;
				pCfg->port = port;
				strncpy(pCfg->path, path, MAX_FILE_PATH_STR);
				pCfg->instance_id = codec_config->num_of_sessions;
				codec_config->num_of_sessions++;
				pCfg->codec_config = codec_config;

				if(pCfg->display_window > 0 && pCfg->display_window < MAX_SESSIONS){
					registerClientWindow();
				}
			}
			else if(strncmp(line, "#", 1) == 0)
			{
				// Comment line
				continue;
			}
			else if(strlen(line) == 0)
			{
				//Blank Line
				continue;
			}
			else
			{
				// Invalid entry
				LOG_ERR("Wrong entry in configuration file: %s\n", line);
				continue;
			}
		}
	}

	fclose(fp);

	codec_config->config_parsed = 1;

	return 0;
}

int printConfig(codec_config_t *codec_config)
{
	LOG_DEF("Printing configurations:\n\n");
	for(int i=0; i<codec_config->num_of_sessions; i++)
	{
		switch (codec_config->sess_cfg[i].type)
		{
			case SESS_SWENC:
			{
				enc_config_t *pCfg = &codec_config->sess_cfg[i].sess.enc_cfg;

				LOG_DEF("SWENC-%d:\n"
					"%d <0: CAM_MJPEG, 1:CAM_NV12, 2:HDMI_NV12, 3:FILE_NV12, 4:ENC_EXT, 5:CAM_YUYV, 6:FILE_YUYV, 7:HDMI_UYVY, 8:HDMI_EXT >\n"
					"%s\n"
					"%dx%d@%d\n"
					"%d kbps\n"
					"%d slice_size_bytes\n"
					"%s:%d\n"
					"Window:%d\n"
					"%s Profile\n"
					"%s Level\n"
					"%d InitQP\n\n",
					i, pCfg->src,
					pCfg->path,
					pCfg->width, pCfg->height, pCfg->fps,
					pCfg->bitrate,
					pCfg->slice_size_bytes,
					pCfg->ip, pCfg->port,
					pCfg->display_window,
					pCfg->profile,
					pCfg->level,
					pCfg->encInitQP
					);
			}
			break;

			case SESS_SWDEC:
			{
				dec_config_t *pCfg = &codec_config->sess_cfg[i].sess.dec_cfg;

				LOG_DEF("SWDEC-%d:\n"
					"%d <0: NW_H264, 1:FILE_H264>\n"
					"%s:%d\n"
					"%dx%d@%d\n"
					"Window:%d\n\n",
					i, pCfg->src,
					pCfg->path, pCfg->port,
					pCfg->width, pCfg->height, pCfg->fps,
					pCfg->display_window
				);
			}
			break;
			case SESS_HWENC:
			{
				enc_config_t *pCfg = &codec_config->sess_cfg[i].sess.enc_cfg;

				LOG_DEF("HWENC-%d:\n"
					"%d <1:CAM_NV12, 3:FILE_NV12, 5:CAM_YUYV, 6:FILE_YUYV >\n"
					"%s\n"
					"%dx%d@%d\n"
					"%d kbps\n"
					"%d slice_size_bytes\n"
					"%s:%d\n"
					"Window:%d\n"
					"%s Profile\n"
					"%s Level\n"
					"%d InitQP\n\n",
					i, pCfg->src,
					pCfg->path,
					pCfg->width, pCfg->height, pCfg->fps,
					pCfg->bitrate,
					pCfg->slice_size_bytes,
					pCfg->ip, pCfg->port,
					pCfg->display_window,
					pCfg->profile,
					pCfg->level,
					pCfg->encInitQP
					);
			}
			break;
			case SESS_HWDEC:
			{
				dec_config_t *pCfg = &codec_config->sess_cfg[i].sess.dec_cfg;

				LOG_DEF("SESS_HWDEC-%d:\n"
					"%d <0: NW_H264, 1:FILE_H264>\n"
					"%s:%d\n"
					"%dx%d@%d\n"
					"Window:%d\n\n",
					i, pCfg->src,
					pCfg->path, pCfg->port,
					pCfg->width, pCfg->height, pCfg->fps,
					pCfg->display_window
				);
			}
			break;

			default:
				break;
		}
	}

	return 0;
}
