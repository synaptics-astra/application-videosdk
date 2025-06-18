/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#ifndef __QVIDEO_IMAGESTORAGE_H__
#define __QVIDEO_IMAGESTORAGE_H__

#include "commonBuffer.h"
#include "qvideoimage.h"

class QQuickVideoImage;

typedef void (*sendInitMessageToMsPtr)(ARGBWindowID windowID,int numBuffer,int height,int width,int size,OptionType optionType, int* boInfo);
typedef void (*sendProcessedMessagedToMsPtr)(ARGBWindowID windowID,int boName);

typedef struct QVideoImageInfo{
	QQuickVideoImage*         m_imageInstance;
	ARGBWindowID 		  m_windowID;
	processMessageHandlerPtr  m_messageHandler;
	QVideoImageInfo():m_imageInstance(NULL),m_messageHandler(NULL){}
}QVideoImageInfo;

static const int m_maxSupportedQVideoImage = ARGB_WINDOW_MAX; //DSPG: 
class QVideoImageStorage: public QObject
{
	public:
		static QVideoImageStorage* getInstance();
		void   setInitMessageToMsPtr(sendInitMessageToMsPtr fPtr){m_fptrSendInitMessageToMs = fPtr;}
		void   setProcessedMessagedToMsPtr(sendProcessedMessagedToMsPtr fPtr){m_fptrSendProcessedMessagedToMs = fPtr;}
		void   registerQVideoImage(QQuickVideoImage* qImage,const ARGBWindowID& windowID,processMessageHandlerPtr handler);
		void   handleMessageFromMs(const ARGBWindowID& windowID,int boName);
		int    renderNextFrame(int type);
		void   sendBoName(const ARGBWindowID& windowID,int boName);
		void   triggerInitMessage();
		bool   IsImageRegistartionDone(int windowID);
        QVideoImageInfo m_videoImages[m_maxSupportedQVideoImage];
	public slots:
		void   updateEGLDisplay(int windowID);
	private:
		static pthread_mutex_t ms_mutex;
		static int ms_var;
		static QVideoImageStorage* ms_instance;

		QVideoImageStorage(QObject *parent = 0);
		~QVideoImageStorage();
		sendInitMessageToMsPtr m_fptrSendInitMessageToMs;
		sendProcessedMessagedToMsPtr  m_fptrSendProcessedMessagedToMs;

		EGLDisplay m_EGLDisplay;
		bool bIsEGLImageCreationPending;
};
#endif//__QVIDEO_IMAGESTORAGE_H__
