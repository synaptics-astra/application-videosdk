#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <unistd.h>
#include "commonBuffer.h"
#include "qvideoimagestorage.h"
#include "pthread.h"
#include "qmsinterface.h"
#include "qvideotexture.h"
#include "logs.h"

#ifdef DEMO_COMPILE
#include "demo_mock.h"
#else
#include <EGL/egl.h>
#include "EGL/eglext.h"
#endif


pthread_mutex_t QVideoImageStorage::ms_mutex = PTHREAD_MUTEX_INITIALIZER;
int QVideoImageStorage::ms_var = 0;
QVideoImageStorage* QVideoImageStorage::ms_instance = NULL;

QVideoImageStorage::QVideoImageStorage(QObject *parent):QObject(parent),m_EGLDisplay(0),bIsEGLImageCreationPending(false)
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
}
QVideoImageStorage::~QVideoImageStorage()
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
}
QVideoImageStorage*  QVideoImageStorage::getInstance()
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	pthread_mutex_lock(&ms_mutex);
	if(0 == ms_var){
		ms_instance = new QVideoImageStorage();
		ms_var = 1;
	}
	pthread_mutex_unlock(&ms_mutex);
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
	return ms_instance;
}
void QVideoImageStorage::updateEGLDisplay(int windowID)
{
	//Update the EGLDisplay and create EGLImage
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	LOG_FUNC("Received the updateEGLDisplay for windowID %d \n", windowID);
	if(NULL == m_videoImages[windowID].m_imageInstance){
		bIsEGLImageCreationPending =  true;
		LOG_FUNC("The  QVideo Image is not registered yet, hence pending the request\n");
	} else {
		m_videoImages[windowID].m_imageInstance->updateEGLDisplay();
	}
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
}

bool QVideoImageStorage::IsImageRegistartionDone(int windowID)
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	bool bDone =  true;
	if(NULL == m_videoImages[windowID].m_imageInstance){
		//qDebug()<<__FUNCTION__<<"1";
		bDone = false;
	}else if(false == m_videoImages[windowID].m_imageInstance->isImageReady()){
		//qDebug()<<__FUNCTION__<<"2";
		bDone = false;
	}else if(false == m_videoImages[windowID].m_imageInstance->isBufferAvailable()){
		//qDebug()<<__FUNCTION__<<"3";
		bDone = false;
	}
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
	return bDone;
}
void QVideoImageStorage::triggerInitMessage()
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
}
void QVideoImageStorage::registerQVideoImage(QQuickVideoImage* qImage,const ARGBWindowID& windowID,processMessageHandlerPtr handler)
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	//printf("SERVER: Fn(%s) is started with instance %x and windowID %d\n",__func__, (unsigned int)qImage,windowID);
	if(windowID < ARGB_WINDOW_MAX && windowID >= 0){
		m_videoImages[windowID].m_windowID = windowID;
		m_videoImages[windowID].m_imageInstance	= qImage;
		m_videoImages[windowID].m_messageHandler = handler;
	}
	LOG_FUNC("bIsEGLImageCreationPending(%d)\n", bIsEGLImageCreationPending);
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
}
void QVideoImageStorage::sendBoName(const ARGBWindowID& windowID,int boName)
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	LOG_FUNC("Returned the BoName %d\n",boName);
	m_fptrSendProcessedMessagedToMs(windowID,boName);
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
}
void QVideoImageStorage::handleMessageFromMs(const ARGBWindowID& windowID,int boName)
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	LOG_FUNC("window(%d) sharedFD(%d)\n", windowID, boName);
	if(windowID < ARGB_WINDOW_MAX && windowID >= 0){
		if(NULL != m_videoImages[windowID].m_imageInstance){
			(m_videoImages[windowID].m_imageInstance->*m_videoImages[windowID].m_messageHandler)(boName);
		}else{
			LOG_FUNC("%s: Called for windowID %d without registering the videoImage, so dropped the Message from MS\n",__FUNCTION__,windowID);
			sendBoName(windowID, boName);
		}
	}
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
}

int QVideoImageStorage::renderNextFrame(int type)
{
	LOG_FUNC(">> Fn(QVideoImageStorage::%s)\n", __func__);
	int texture_id = -1;
	if(NULL != m_videoImages[type].m_imageInstance){
		texture_id = m_videoImages[type].m_imageInstance->renderNext(type);
	}else{
		LOG_FUNC("%s: is called for windowID %d without registering the videoImage, so dropped rendering request \n",__FUNCTION__,type);
	}
	LOG_FUNC("<< Fn(QVideoImageStorage::%s)\n", __func__);
	return texture_id;
}
