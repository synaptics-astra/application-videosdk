#ifndef __VIDEO_TUNER_SERVER_H__
#define __VIDEO_TUNER_SERVER_H__

#include "config.h"
#include "encoder_interface_v4l2.h"

#ifdef VIDEO_TUNER_C
#define CODEC_DEMO_EXTERN
#else
#define CODEC_DEMO_EXTERN extern
#endif

CODEC_DEMO_EXTERN pthread_t serverlistenThread;
CODEC_DEMO_EXTERN int fServerIPCInitDone;

CODEC_DEMO_EXTERN int server_sock;
CODEC_DEMO_EXTERN int client_sock;

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

int InitServerIPC();
int DeInitServerIPC();
#endif
