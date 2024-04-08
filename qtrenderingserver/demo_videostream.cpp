#include "demo_videostream.h"
#include "demo_videoitem.h"
#include "logs.h"

#include <QtCore/QThread>

VideoStream* VideoItemUpdater::Stream[ARGB_WINDOW_MAX] = {0};
FRAMES VideoStream::m_frames_preview_failed = FRAMES();

void VideoItemUpdater::initialize()
{
	LOG_FUNC(">> Fn(VideoItemUpdater::%s)\n", __func__);
	for(int i = 0; i < ARGB_WINDOW_MAX; i++){
		Stream[i] = new VideoStream(i);
	}
	LOG_FUNC("<< Fn(VideoItemUpdater::%s)\n", __func__);
}

VideoStream *VideoItemUpdater::getVideoStream(int windowID)
{
    if( windowID >= ARGB_WINDOW_0 && windowID < ARGB_WINDOW_MAX){
	return Stream[windowID];
    }else {
        return NULL;
    }
}

VideoStream::VideoStream(int type)
    : m_videoItem(NULL)
    , m_textureId(-1)
{
    Q_UNUSED(type);
    m_frame._videoDataRef = NULL;
    m_frame._sessionId = -1;
    m_frame._videoType = INVALID_VIDEO_TYPE;
    m_frame._released = false;
}

void VideoStream::setVideoItem(VideoItem *item)
{
    LOG_FUNC(">> Fn(VideoStream::%s)\n", __func__);
    m_frameMutex.lock();
    m_videoItem = item;
    updateResolution();
    m_frameMutex.unlock();
    LOG_FUNC("<< Fn(VideoStream::%s)\n", __func__);
}

void VideoStream::clearVideoItemIfEqual(VideoItem *item)
{
    m_frameMutex.lock();
    if (item == m_videoItem) {
        m_videoItem = NULL;
    }
    m_frameMutex.unlock();
}

void VideoStream::frameFromMS()
{
    LOG_FUNC(">> Fn(VideoStream::%s)\n", __func__);
    if (m_videoItem && m_videoItem->isVisible() && m_videoItem->hasMaterial()) {
        updateFrame();
    } else {
        qDebug()<<__FUNCTION__<<":::frame come too early!";
    }
    LOG_FUNC("<< Fn(VideoStream::%s)\n", __func__);
}

void VideoStream::updateFrame()
{
    LOG_FUNC(">> Fn(VideoStream::%s)\n", __func__);
    m_frameMutex.lock();
    m_videoItem->updateFrame();
    m_frameMutex.unlock();
    LOG_FUNC("<< Fn(VideoStream::%s)\n", __func__);
}

void VideoStream::updateResolutionByFrame(int src_w, int src_h)
{
    LOG_FUNC(">> Fn(VideoStream::%s)\n", __func__);
    if (m_src_parameter.originalwidth == src_w &&
            m_src_parameter.originalheight == src_h)
        return;

    calculateResolution(src_w,
                        src_h,
                        m_videoItem->width(),
                        m_videoItem->height());
    LOG_FUNC("<< Fn(VideoStream::%s)\n", __func__);
}

void VideoStream::updateResolution()
{
    LOG_FUNC(">> Fn(VideoStream::%s)\n", __func__);
    calculateResolution(m_src_parameter.width,
                        m_src_parameter.height,
                        m_videoItem->width(),
                        m_videoItem->height());
    LOG_FUNC("<< Fn(VideoStream::%s)\n", __func__);
}

void VideoStream::calculateResolution(int src_w, int src_h, int des_w, int des_h)
{
    LOG_FUNC(">> Fn(VideoStream::%s)\n", __func__);
    if (des_w == 0 || des_h == 0)
        return;

    CropParameter src_parameter, dst_parameter;

    src_parameter.originalwidth = src_parameter.width = src_w;
    src_parameter.originalheight = src_parameter.height = src_h;

    float resolution_width = (float)src_w;
    float resolution_height = (float)src_h;

    if (resolution_width != 0 && resolution_height != 0) {
        float needed_ratio = des_w / (float)des_h;
        float src_ratio = resolution_width / resolution_height;
        if (src_ratio > needed_ratio) {
            float needed_width = resolution_height * needed_ratio;
            src_parameter.width = (int)needed_width;
            src_parameter.x_offset = (int)((resolution_width - needed_width) / 2.f);
        } else {
            float needed_height = resolution_width / needed_ratio;
            src_parameter.height = (int)needed_height;
            src_parameter.y_offset = (int)((resolution_height - needed_height) / 2.f);
        }
    }

    src_parameter.ensureEven();
    m_src_parameter.copy(src_parameter);

    dst_parameter.width = (int)des_w;
    dst_parameter.height = (int)des_h;
    dst_parameter.ensureEven();
    m_dst_parameter.copy(dst_parameter);
    LOG_FUNC("<< Fn(VideoStream::%s)\n", __func__);

}
