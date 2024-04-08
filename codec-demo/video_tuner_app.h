#ifndef __VIDEO_TUNER_APP_H__
#define __VIDEO_TUNER_APP_H__

#include "syna_std_headers.h"

#define CODEC_DEMO_SERVER_DIR "/home/root/"
#define CODEC_DEMO_SERVER_SOCKET_PATH "/home/root/codec_demo_server_socket"
#define CODEC_DEMO_CLIENT_CLIENT_PATH "/home/root/codec_demo_client_socket"

int client_sock = -1;

typedef enum{
	SET_BITRATE_MSG,
	SET_FRAMERATE_MSG,
	SET_FORCE_IDR_MSG,
	SET_RESOLUTION_MSG,
	SET_STATISTICS_FLAG_MSG,
	SET_MAX_MSG
}Message_Type;

typedef struct SetBitrateMessage{
	unsigned int iBitrate;
}SetBitrateMessage;

typedef struct SetFramerateMessage{
	unsigned int iFpsNum; // Assumption is that user will set the FPS as a whole no, denominator will be 1 by default.
}SetFramerateMessage;

typedef struct SetResolutionMessage{
	unsigned int iWidth;
	unsigned int iHeight;
}SetResolutionMessage;

typedef struct Message_Struct{
	Message_Type m_messageType;
	unsigned int session_id;
	union {
		SetBitrateMessage bitrate_msg;
		SetFramerateMessage framerate_msg;
		SetResolutionMessage resolution_msg;
	}message;
}Message_Struct;


#endif
