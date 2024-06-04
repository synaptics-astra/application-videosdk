/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#ifndef __COMMON_BUFFER_H__
#define __COMMON_BUFFER_H__

#define MAX_BUFFER_SUPPORT 25
#define MAX_PLANES_SUPPORT 2
#define DUAL_DISPLAY 2

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
	SWDEC,
	SWENC,
	HWDEC,
	HWENC
}OptionType;

typedef enum{
    ARGB_WINDOW_INVALID = -1,
    ARGB_WINDOW_0 = 0,
    ARGB_WINDOW_1,
    ARGB_WINDOW_2,
    ARGB_WINDOW_3,
    ARGB_WINDOW_4,
    ARGB_WINDOW_5,
    ARGB_WINDOW_6,
    ARGB_WINDOW_7,
    ARGB_WINDOW_8,
    ARGB_WINDOW_9,
    ARGB_WINDOW_10,
    ARGB_WINDOW_11,
    ARGB_WINDOW_12,
    ARGB_WINDOW_13,
    ARGB_WINDOW_14,
    ARGB_WINDOW_15,
    ARGB_WINDOW_MAX,
}ARGBWindowID;


typedef enum{
	PIXEL_UI_NV12,
	PIXEL_UI_YUYV,
	PIXEL_UI_UYVY,
	PIXEL_UI_NV12M,
	PIXEL_UI_ARGB
}InputPixelFormatUI;

typedef enum{
    PLATFORM_DOLPHIN = 0,
    PLATFORM_PLATYPUS,
    PLATFORM_MYNA2,
    MODE_MAX
}PlatformType;

typedef struct _ARGBWindow{
	int					numBuffers;
	int					width;
	int					height;
	int					remoteFDs[MAX_BUFFER_SUPPORT];
	int					sharedFDs[MAX_BUFFER_SUPPORT][MAX_PLANES_SUPPORT];
	OptionType			optionType;
	InputPixelFormatUI	inPixelType;
}ARGBWindow;

typedef struct BufferInfo{
	int m_boName;
}BufferInfo;

typedef struct InitMessage{
	int					m_numberbuffer;
	int					m_bufferSize;
	int					m_height;
	int					m_width;
	BufferInfo			m_sbufferInfo[MAX_BUFFER_SUPPORT];
	OptionType			optionType;
	InputPixelFormatUI	m_inpixeltype;
	int					m_total_window;
}InitMessage;

typedef struct MessageFromUI{
	int	m_boName;
}MessageFromUI;

typedef struct MessageToUI{
	int m_boName1;
	int m_boName2;
}MessageToUI;

typedef struct StatusMsg{
	int			m_sessionID;
	OptionType	m_optionType;
	int			m_width;
	int			m_height;
	float		m_fps;
	int			m_total_sessions;
}StatusMsg;

typedef struct MessageStruct{
	MessageType		m_messageType;
	ARGBWindowID	m_argbWindowID;
	union{
		InitMessage		initMessage;
		MessageFromUI	uIMessage;
		MessageToUI		msMessage;
		StatusMsg		stMessage;
	}message;
}MessageStruct;

#endif
