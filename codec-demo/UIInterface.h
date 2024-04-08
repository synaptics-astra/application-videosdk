#ifndef __UI_INTERFACE_H__
#define __UI_INTERFACE_H__

#include "syna_std_headers.h"

/* Increase MAX_BUFFER_SUPPORT to 25 to accomodate for files requires higher number of buffers */
#define MAX_BUFFER_SUPPORT 25
#define MAX_PATH_SIZE 25
#define SOCK_DIR "/home/root/"
#define SOCK_PATH "/home/root/echo_socket"

typedef enum{
	INIT_MESSAGE,
	MESSAGE_FROM_UI,
	MESSAGE_TO_UI,
	ALL_INIT_MSG_DONE_FROM_UI,
	ALL_INIT_MSG_DONE_TO_UI,
	STATUS_INIT_MSG,
	STATUS_MSG_FROM_UI,
	STATUS_MSG_TO_UI,
	TERMINATE_FROM_UI,
	TERMINATE_TO_UI,
	ACK_FROM_UI,
}MessageType;

typedef enum{
	AMPENC,
	AMPDEC,
	SWDEC,
	SWENC,
	HWDEC,
	HWENC
}OptionType;

typedef enum{
	PIXEL_UI_NV12,
	PIXEL_UI_YUYV,
	PIXEL_UI_UYVY,
	PIXEL_UI_NV12M,
	PIXEL_UI_ARGB
}InputPixelFormatUI;

typedef struct BufferInfo{
	int m_boName;
}BufferInfo;

typedef struct InitMessage{
	int m_numberbuffer;
	int m_bufferSize;
	int m_height;
	int m_width;
	BufferInfo m_sbufferInfo[MAX_BUFFER_SUPPORT];
	OptionType optionType;
	InputPixelFormatUI m_inpixeltype;
	int m_total_window;
}InitMessage;

typedef struct MessageFromUI{
	int m_boName;
}MessageFromUI;


typedef struct MessageToUI{
	int m_boName1;
	int m_boName2;
}MessageToUI;

typedef struct StatusMsg{
	int		m_sessionID;
	int		m_optionType;
	int		m_width;
	int		m_height;
	float		m_fps;
	int		m_total_sessions;
}StatusMsg;

typedef struct MessageStruct{
	MessageType m_messageType;
	int m_windowID;
	union{
		InitMessage	initMessage;
		MessageFromUI	uIMessage;
		MessageToUI	msMessage;
		StatusMsg	stMessage;
	}message;
}MessageStruct;

typedef struct{
	int m_dma_buf_id;
	int m_bo_name;
}DMABufferInfoStruct;

typedef struct{
	int width;
	int height;
	int number_buffer;
	DMABufferInfoStruct bufferInfo[MAX_BUFFER_SUPPORT];
}DMAInfoStruct;

int InitClientIPC(void);
void DeInitClientIPC(void);
int   sendProcessDMABufId(int windowID, int sharedFD1, int sharedFD2);
void* listening_thread(void* arg);
int sendAllInitDoneMsgToUI(void);
int registerClientWindow(void);
int sendTerminateMsgToUI(void);
void SendInitMessageToUI(int instType, void *pCfg, int windowID, int numBuffer, int width, int height, int pixelType);
void SendStatusInitMessageToUI(StatusMsg *stMessage);
void SendStatusMessageToUI(StatusMsg *stMessage);


#endif
