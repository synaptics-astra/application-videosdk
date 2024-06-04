/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

//#include "ion/ion.h"
#include <unistd.h>
#include "commonBuffer.h"
#include <sys/time.h>
#include "errno.h"
#include <sys/ioctl.h>
#include "unistd.h"
#include <sys/mman.h>
#include "qvideotexture.h"
#include "logs.h"
//#include "system/window.h"
#include <qpa/qplatformnativeinterface.h>

#ifdef DEMO_COMPILE
#include "demo_mock.h"
#else
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gbm.h>
//#include <drm/drm.h>
//#include <drm/xf86drm.h>
#endif

#define ROUNDUP(x, base)        ((x + base - 1) & ~(base - 1))
#define ROUNDUP128(x, y)           (((y) == 1) ? ROUNDUP(x, 128):x)
/*
#ifndef DEFAULT_IMAGE_SIZE
#define DEFAULT_IMAGE_SIZE 0
#define DEFAULT_WIDTH   800
#define DEFAULT_HEIGHT   480
#endif
*/
/*****The DRM Initilaization for BO Creation *****/
int QVideoTexture:: ms_drm_fd = -1;
EGLDisplay QVideoTexture:: ms_eglDisplay  = 0;
int QVideoTexture::ms_initialized = 0;
pthread_mutex_t QVideoTexture::ms_mutexLock = PTHREAD_MUTEX_INITIALIZER;
//kms_driver *QVideoTexture ::ms_kms = NULL;
/***************************************************/

void checkEglError(const char* op, EGLBoolean returnVal = EGL_TRUE) {
    if (returnVal != EGL_TRUE) {
        fprintf(stderr, "%s() returned %d\n", op, returnVal);
    }

    for (EGLint error = eglGetError(); error != EGL_SUCCESS; error
            = eglGetError()) {
        fprintf(stderr, "after %s() eglError %s (0x%x)\n", op, strerror(error),
                error);
    }
}

void printEGLConfiguration(EGLDisplay dpy, EGLConfig config) {

#define X(VAL) {VAL, #VAL}
    struct {EGLint attribute; const char* name;} names[] = {
    X(EGL_BUFFER_SIZE),
    X(EGL_ALPHA_SIZE),
    X(EGL_BLUE_SIZE),
    X(EGL_GREEN_SIZE),
    X(EGL_RED_SIZE),
    X(EGL_DEPTH_SIZE),
    X(EGL_STENCIL_SIZE),
    X(EGL_CONFIG_CAVEAT),
    X(EGL_CONFIG_ID),
    X(EGL_LEVEL),
    X(EGL_MAX_PBUFFER_HEIGHT),
    X(EGL_MAX_PBUFFER_PIXELS),
    X(EGL_MAX_PBUFFER_WIDTH),
    X(EGL_NATIVE_RENDERABLE),
    X(EGL_NATIVE_VISUAL_ID),
    X(EGL_NATIVE_VISUAL_TYPE),
    X(EGL_SAMPLES),
    X(EGL_SAMPLE_BUFFERS),
    X(EGL_SURFACE_TYPE),
    X(EGL_TRANSPARENT_TYPE),
    X(EGL_TRANSPARENT_RED_VALUE),
    X(EGL_TRANSPARENT_GREEN_VALUE),
    X(EGL_TRANSPARENT_BLUE_VALUE),
    X(EGL_BIND_TO_TEXTURE_RGB),
    X(EGL_BIND_TO_TEXTURE_RGBA),
    X(EGL_MIN_SWAP_INTERVAL),
    X(EGL_MAX_SWAP_INTERVAL),
    X(EGL_LUMINANCE_SIZE),
    X(EGL_ALPHA_MASK_SIZE),
    X(EGL_COLOR_BUFFER_TYPE),
    X(EGL_RENDERABLE_TYPE),
    X(EGL_CONFORMANT),
   };
#undef X

    for (size_t j = 0; j < sizeof(names) / sizeof(names[0]); j++) {
        EGLint value = -1;
        EGLint returnVal = eglGetConfigAttrib(dpy, config, names[j].attribute, &value);
        EGLint error = eglGetError();
        if (returnVal && error == EGL_SUCCESS) {
printf("\t");
            printf(" %s: ", names[j].name);
            printf("%d (0x%x)", value, value);
printf("\n");
        }
    }
    printf("\n");
}

int printEGLConfigurations(EGLDisplay dpy) {
    EGLint numConfig = 0;
    EGLint returnVal = eglGetConfigs(dpy, NULL, 0, &numConfig);
    checkEglError("eglGetConfigs", returnVal);
    if (!returnVal) {
        return false;
    }

    LOG_DEF("Number of EGL configuration: %d\n", numConfig);

    EGLConfig* configs = (EGLConfig*) malloc(sizeof(EGLConfig) * numConfig);
    if (! configs) {
        LOG_DEF("Could not allocate configs.\n");
        return false;
    }

    returnVal = eglGetConfigs(dpy, configs, numConfig, &numConfig);
    checkEglError("eglGetConfigs", returnVal);
    if (!returnVal) {
        free(configs);
        return false;
    }

    for(int i = 0; i < numConfig; i++) {
        LOG_FUNC("Configuration %d\n", i);
        printEGLConfiguration(dpy, configs[i]);
    }

    free(configs);
    return true;
}



void QVideoTexture::initilizeTexture()
{
	LOG_FUNC(">> Fn(QVideoTexture::%s)\n", __func__);
	pthread_mutex_lock(&ms_mutexLock);
	if(0 == ms_initialized){

#ifdef DVF2300
		ms_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		//LOG_DEF("ms_eglDisplay [%x]\n", (unsigned int)ms_eglDisplay);
	    	if (ms_eglDisplay == EGL_NO_DISPLAY) {
		    LOG_DEF("eglGetDisplay failed: %#x\n", eglGetError());
		    return;
		}
		
		EGLint majorVersion, minorVersion;
		int result = eglInitialize(ms_eglDisplay, &majorVersion, &minorVersion);
		if (result != EGL_TRUE) {
		    printf("eglInitialize failed: %#x\n", eglGetError());
		    return;
		}
		LOG_DEF("Initialized EGL v%d.%d\n", majorVersion, minorVersion);
		//printEGLConfigurations(ms_eglDisplay);

		const char *extensions;
		extensions = eglQueryString(ms_eglDisplay, EGL_EXTENSIONS);
		LOG_DEF("EGL extensions: (%s) \n", extensions);
#else
		QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
                ms_eglDisplay = static_cast<EGLDisplay>(nativeInterface->nativeResourceForIntegration("EGLDisplay"));
                if (ms_eglDisplay) {
                        LOG_PARAM("egl Display is 0x%lx\n", ms_eglDisplay);
                } else {
                        LOG_ERR("ERROR: no egl Display \n");
                }

                const char *extensions;
                extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
                LOG_PARAM("EGL generic extensions: (%s) \n", extensions);
                extensions = eglQueryString(ms_eglDisplay, EGL_EXTENSIONS);
                LOG_PARAM("EGL extensions: (%s) \n", extensions);
#endif
		ms_initialized = 1;
	}

	pthread_mutex_unlock(&ms_mutexLock);
	LOG_FUNC("<< Fn(QVideoTexture::%s)\n", __func__);
}

void QVideoTexture::deinitilizeTexture()
{
	LOG_FUNC(">> Fn(QVideoTexture::%s)\n", __func__);
	pthread_mutex_lock(&ms_mutexLock);
	if(1 == ms_initialized){
		ms_initialized = 0;
	}
	pthread_mutex_unlock(&ms_mutexLock);
	LOG_FUNC("<< Fn(QVideoTexture::%s)\n", __func__);
}

inline QVideoTexture::EGLImageBaseAttributeList::EGLImageBaseAttributeList(): m_color_format(EGL_LINUX_DRM_FOURCC_EXT),
							m_color_format_value(0),m_height_format(EGL_HEIGHT),m_height_value(0),
							m_width_format(EGL_WIDTH),m_width_value(0),
							m_dma_buf_format(EGL_DMA_BUF_PLANE0_FD_EXT),m_dma_buf_id(-1),
							m_dma_plan0_offset_format(EGL_DMA_BUF_PLANE0_OFFSET_EXT),m_dma_plan0_offset_value(0),
							m_dma_plan0_pitch_format(EGL_DMA_BUF_PLANE0_PITCH_EXT),m_dma_plan0_pitch_value(0){
}
inline QVideoTexture::EGLImageBaseAttributeList:: ~EGLImageBaseAttributeList()
{
}
//The CTOR
QVideoTexture::QVideoTexture(int w,int h, int bo, int shared_fd):m_width(w),m_height(h),m_bpp(12),
			m_format(),m_dma_buf_id(shared_fd),m_bo_name(bo), m_eglImage(0),
			m_textureid(-1),m_mmamped_addr(NULL)
{
	LOG_FUNC(">> Fn(QVideoTexture::%s)\n", __func__);
	LOG_FUNC("QVideoTexture is created with width %d and height %d, bo_handle [%d] shared_fd[%d]\n",m_width,m_height, m_bo_name, m_dma_buf_id);
	LOG_FUNC("<< Fn(QVideoTexture::%s)\n", __func__);
}

//The DTOR
QVideoTexture::~QVideoTexture()
{
}

PFNEGLCREATEIMAGEKHRPROC pfneglCreateImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC pfnglEGLImageTargetTexture2DOES = NULL;

#include "demo_videostream.h"
void QVideoTexture::createEGLImage(int nWidth, int nHeight, int shared_fd1 ,int shared_fd2, int optionType, int pixelType)
{
	LOG_FUNC(">> Fn(QVideoTexture::%s)\n", __func__);
	LOG_FUNC("GL error after line %d is %d\n",__LINE__,glGetError());
	EGLint egl_error_code;
	EGLint *img_attrs;

	EGLint img_attrs_uyvy[] = {
		EGL_WIDTH, nWidth,
		EGL_HEIGHT, nHeight,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc_code('U', 'Y', 'V', 'Y'),
		EGL_DMA_BUF_PLANE0_FD_EXT, shared_fd1,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, ROUNDUP128((2 * nWidth),optionType),
		EGL_NONE
	};
	EGLint img_attrs_yuyv[] = {
		EGL_WIDTH, nWidth,
		EGL_HEIGHT, nHeight,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc_code('Y', 'U', 'Y', 'V'),
		EGL_DMA_BUF_PLANE0_FD_EXT, shared_fd1,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, ROUNDUP128((2 * nWidth),optionType),
		EGL_NONE
	};
	EGLint img_attrs_nv12[] = {
		EGL_WIDTH, nWidth,
		EGL_HEIGHT, nHeight,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc_code('N', 'V', '1', '2'),
		EGL_DMA_BUF_PLANE0_FD_EXT, shared_fd1,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, ROUNDUP128(nWidth,optionType),
		EGL_DMA_BUF_PLANE1_FD_EXT, shared_fd1,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, ROUNDUP128(nWidth,optionType)*ROUNDUP128(nHeight,optionType),
		EGL_DMA_BUF_PLANE1_PITCH_EXT, ROUNDUP128(nWidth,optionType),
		EGL_NONE
	};
	//NM12 is treated as NV12 with 2 different planes on the input-side
	EGLint img_attrs_nv12m[] = {
		EGL_WIDTH, nWidth,
		EGL_HEIGHT, nHeight,
		EGL_LINUX_DRM_FOURCC_EXT, fourcc_code('N', 'V', '1', '2'),
		EGL_DMA_BUF_PLANE0_FD_EXT, shared_fd1,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, ROUNDUP128(nWidth,optionType),
		EGL_DMA_BUF_PLANE1_FD_EXT, shared_fd2,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE1_PITCH_EXT, ROUNDUP128(nWidth,optionType),
		EGL_NONE
	};

	if (pixelType == PIXEL_UI_YUYV)
	{
		img_attrs = img_attrs_yuyv;
	}
	else if (pixelType == PIXEL_UI_UYVY)
	{
		img_attrs = img_attrs_uyvy;
	}
	else if (pixelType == PIXEL_UI_NV12M)
	{
		img_attrs = img_attrs_nv12m;
	}
	else
	{
		img_attrs = img_attrs_nv12;
	}

	LOG_FUNC("Fn(QVideoTexture::%s) ms_eglDisplay(%x) shared_fd(%d)\n", __func__, ms_eglDisplay, shared_fd1);
#if 0
	m_eglImage = (void*)eglCreateImageKHR(ms_eglDisplay,EGL_NO_CONTEXT,
                        EGL_LINUX_DMA_BUF_EXT,(void*)NULL,
                        (EGLint*)img_attrs);
#endif
	pfneglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
	if (!pfneglCreateImageKHR)
        {
                LOG_ERR("Error for pfneglCreateImageKHR->eglGetProcAddress");
        }

	m_eglImage = pfneglCreateImageKHR(ms_eglDisplay, EGL_NO_CONTEXT,
			EGL_LINUX_DMA_BUF_EXT, (void*)NULL,
			(EGLint*)img_attrs);

	if(m_eglImage == EGL_NO_IMAGE_KHR){
		EGLint eglErrorCode = eglGetError();
		LOG_FUNC("Failed to create eglCreateImageKHR image %x\n",(int)eglErrorCode);
		exit(-1);

	}else{
		LOG_SEQ("Successfully created eglIMAGE########################\n");
	}

	pfnglEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
        if (!pfnglEGLImageTargetTexture2DOES)
        {
                LOG_ERR("Error for pfnglEGLImageTargetTexture2DOES->eglGetProcAddress");
        }

	glGenTextures(1, (GLuint*) &m_textureid);
	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, m_eglImage);
	if(pfnglEGLImageTargetTexture2DOES) {
		pfnglEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, m_eglImage);
                egl_error_code = eglGetError();
                if (egl_error_code != EGL_SUCCESS)
                {
                        LOG_ERR("ERROR: (%s): failed to create a texture backed by egl image, error=%x\n",
                                        __func__, egl_error_code);
                } else {
                        LOG_SEQ("---success in creating tex2does\n");
                }
	}
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, (GLuint)m_textureid);
	LOG_FUNC("<< Fn(QVideoTexture::%s)\n", __func__);
}

void QVideoTexture::renderTexture()
{
	LOG_FUNC(">> Fn(QVideoTexture::%s)\n", __func__);
	//qDebug()<<__FUNCTION__<<":::m_textureid"<<m_textureid;
	glActiveTexture(GL_TEXTURE0+m_textureid);

	if(pfnglEGLImageTargetTexture2DOES == NULL){
		pfnglEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
	}

	if(pfnglEGLImageTargetTexture2DOES)
		pfnglEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, m_eglImage);
	//glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, m_eglImage);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_textureid);
	LOG_FUNC("<< Fn(QVideoTexture::%s)\n", __func__);
}

void QVideoTexture::createDMABuffer(int bo_handle, int shared_fd)
{
	LOG_FUNC(">> Fn(QVideoTexture::%s)\n", __func__);
	m_dma_buf_id = shared_fd;
	m_bo_name    = bo_handle;
	LOG_FUNC("<< Fn(QVideoTexture::%s)\n", __func__);
}
