/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#ifndef __QVIDEO_TEXTURE_H__
#define __QVIDEO_TEXTURE_H__

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include "demo_videostream.h"

#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>
#include <QSGTextureProvider>
#include <QSGTexture>
#include <QQuickWindow>

#ifdef DEMO_COMPILE
#include "demo_mock.h"
#else
#include "drm/drm_fourcc.h"
//#include "gbm/gbm.h"
#include <EGL/egl.h>
#include "EGL/eglext.h"
//#include "libkms/libkms.h"
#include "QtGui/qopengles2ext.h"
#endif

const int DEFAULT_BPP_VIDEO_IMAGE = 32;

class QVideoTexture:public QObject
{
	Q_OBJECT

	public:
		QVideoTexture(int w,int h, int bo_handle, int shared_fd);
		~QVideoTexture();
		int getBoName(){return m_bo_name;}
		void createEGLImage(int nWidth, int nHeight, int shared_fd1 ,int shared_fd2, int optionType, int pixelType);   //Create the EGLImage from DMA buf
		void renderTexture();
		static void initilizeTexture();
		static void deinitilizeTexture();
		void setTextureID(int id) {m_textureid = id;}
		int textureID() {return m_textureid;}

	private:
		struct EGLImageBaseAttributeList
		{
			int m_color_format;
			int m_color_format_value;
			int m_height_format;
			int m_height_value;
			int m_width_format;
			int m_width_value;
			int m_dma_buf_format;
			int m_dma_buf_id;
			int m_dma_plan0_offset_format;
			int m_dma_plan0_offset_value;
			int m_dma_plan0_pitch_format;
			int m_dma_plan0_pitch_value;
			EGLImageBaseAttributeList();   //CTOR of EGLImageBaseAttributeList
			~EGLImageBaseAttributeList(); //DTOR of EGLImageBaseAttributeList
		};
		struct EGLImageRGBAAttributeList:public EGLImageBaseAttributeList
		{
			int m_done_format;
			EGLImageRGBAAttributeList():m_done_format(EGL_NONE){
				m_color_format_value = DRM_FORMAT_BGRA8888;
			}
		};
		static int ms_drm_fd; //Create bo_name is doen via m_drm_fd
		//static struct kms_driver *ms_kms;
		static EGLDisplay ms_eglDisplay;
		static int ms_initialized;
		static pthread_mutex_t ms_mutexLock; //lock to check for updating ms_drm_fd

		void createDMABuffer(int bo_handle, int shared_fd);  //Create DMA buffer function
		int m_width;     //The width parameter for video frame
		int m_height;    //The height parameter for video frame
		int m_bpp;       //The bpp parameter for video frame
		int m_format;    //The format parameter for video frame
		int m_dma_buf_id; //The DMA buffer id corresponding to the texture
		int m_bo_name;    //The generic global name of the DMA buffer
		void* m_eglImage; //Corresponding EGLImage created via eglCreateImageKHR
		//struct kms_bo *mp_bos;
		int m_textureid;   //OpenGL texture id
		void* m_mmamped_addr; //The mmap address of the corresonding DMA buffer
};
#endif//__QVIDEO_TEXTURE_H__
