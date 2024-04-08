#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include "qvideoimage.h"
#include "pthread.h"
#include "map"
#include "qvideoimagestorage.h"
#include "qvideotexture.h"
#include "logs.h"

#ifdef DEMO_COMPILE
#include "demo_mock.h"
#else
#include <EGL/egl.h>
#include "EGL/eglext.h"
#endif

extern ARGBWindow argbWindows[ARGB_WINDOW_MAX];

QQuickVideoImage::QQuickVideoImage(QQuickItem *parent):m_bIsImageReady(false)
{
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
	Q_UNUSED(parent);
	pthread_mutex_init(&m_mutex,NULL);
	m_previousBoName = -1;
	m_pVideoImageStorage = QVideoImageStorage::getInstance();
	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
}

int QQuickVideoImage::getNumberBuffer(ARGBWindowID windowID)
{
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
        ARGBWindow *window = &argbWindows[windowID];
	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
	return window->numBuffers;
}

void QQuickVideoImage::getDMABufIds(int* boName)
{
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
	if(NULL != boName){
		map<int,QVideoTexture*>::iterator it = m_textureMap.begin();
		map<int,QVideoTexture*>::iterator it_end  = m_textureMap.end();
		for(  int i = 0; it != it_end; ++it ){
			boName[i++] = it->first;
		}
	}
	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
}

void QQuickVideoImage::videoImageUpdated()
{
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
	if(width() && height()){
		//Image is ready for registrattion
		LOG_DEF("The value of width %f and height %f\n",width(),height());
		m_pVideoImageStorage->registerQVideoImage(this, (ARGBWindowID) m_windowID,&QQuickVideoImage::processMessage);
		exit(-1);
	}
	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
}

void QQuickVideoImage::updateEGLDisplay()
{
	//EGL Display is set now we can start
	int windowID = QQuickVideoImage::getWindowID();
	ARGBWindow *window = &argbWindows[windowID];
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
	LOG_PARAM("m_number_dma_buf_id(%d)\n", window->numBuffers);
	for(int i =0 ;i< window->numBuffers ;++i){
		QVideoTexture* texture = new QVideoTexture(window->width, window->height, window->remoteFDs[i], window->sharedFDs[i][0]);
		//TODO: Handle this generically for multi planar data
		texture->createEGLImage(window->width, window->height, window->sharedFDs[i][0], window->sharedFDs[i][1],window->optionType, window->inPixelType);
		m_textureMap.insert(std::pair<int,QVideoTexture*>(texture->getBoName(),texture));
	}
	if(window->numBuffers){
		QVideoImageStorage* qvideoImageStorageInstance = QVideoImageStorage::getInstance();
		if(!qvideoImageStorageInstance->m_videoImages[windowID].m_imageInstance->isVisible()){
			LOG_PARAM("Fn(%s): window_%d setting visible\n", __func__, windowID);
			qvideoImageStorageInstance->m_videoImages[windowID].m_imageInstance->setVisible(true);
		}
	}

	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
}

#include "demo_videostream.h"
void QQuickVideoImage::processMessage(int boName)
{
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
	pthread_mutex_lock(&m_mutex);

	map<int,QVideoTexture*>::iterator it;
	it = m_textureMap.find(boName);
	if(it==m_textureMap.end()){
		LOG_SEQ("Failed to find the bo_name in the map with bo_name %d\n",boName);
		m_pVideoImageStorage->sendBoName((ARGBWindowID)m_windowID, boName);
		pthread_mutex_unlock(&m_mutex);
		return;
	}
	m_pendingQueue.push(it->second);
	m_bIsImageReady =  true;
	LOG_SEQ("The bo_name %d is sent from ms and inserted into Queue\n",boName);
	//qDebug()<<__FUNCTION__<<"ready?"<<m_bIsImageReady;
	LOG_SEQ("for m_windowID(%d)\n", m_windowID);
	VideoItemUpdater::Stream[m_windowID]->frameFromMS();
	pthread_mutex_unlock(&m_mutex);
	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
}

int QQuickVideoImage::renderNext(int windowID)
{
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
	QVideoTexture* texture = NULL;
	int texture_id = -1;
	pthread_mutex_lock(&m_mutex);
	if(m_pendingQueue.empty()){
		LOG_SEQ("No Frame available\n");
		pthread_mutex_unlock(&m_mutex);
	}else{
		/* Donot pile up buffer, display them immediately or discard */
		while(m_pendingQueue.size() > 2)
		{
			texture =  m_pendingQueue.front();
			m_pendingQueue.pop();
			m_pVideoImageStorage->sendBoName((ARGBWindowID)m_windowID, texture->getBoName());
			LOG_SEQ("[%d]: Discarding frame\n", m_windowID);
		}
		texture =  m_pendingQueue.front();
		m_pendingQueue.pop();
		texture->renderTexture();
		texture_id = texture->textureID();
		LOG_FUNC("Got the new frame \n");
		pthread_mutex_unlock(&m_mutex);
		VideoItemUpdater::Stream[windowID]->setTextureID(texture_id);
		updatePreviousboName(texture->getBoName());
	}
	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
	return texture_id;
}
bool QQuickVideoImage::isBufferAvailable()
{
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
	bool bReturn = true;
	if(m_pendingQueue.empty())
		bReturn = false;
	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
	return bReturn;
}
void QQuickVideoImage::updatePreviousboName(int boName)
{
	LOG_FUNC(">> Fn(QQuickVideoImage::%s)\n", __func__);
	LOG_FUNC("The old boName is %d and new Bo Name is %d\n",m_previousBoName,boName);
	if(-1 == m_previousBoName){
		m_previousBoName = boName;
	}else if(m_previousBoName != boName){
		m_pVideoImageStorage->sendBoName((ARGBWindowID)m_windowID,m_previousBoName);
		m_previousBoName = boName;
	}
	LOG_FUNC("<< Fn(QQuickVideoImage::%s)\n", __func__);
}
