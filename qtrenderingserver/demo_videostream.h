/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#ifndef DEMO_VIDEOSTREAM_H
#define DEMO_VIDEOSTREAM_H

#include "commonBuffer.h"
#ifdef DEMO_COMPILE
#include "demo_mock.h"
#endif
#include <QQueue>
#include <QtCore/QMutex>

#include "qvideoimagestorage.h"

#define INVALID_VIDEO_TYPE  -1
//#define PREVIEW_VIDEO_TYPE  0
//#define DECODE_VIDEO_TYPE   1

typedef void* VIDEO_DATA_REF;

typedef struct {
    VIDEO_DATA_REF _videoDataRef;
    int _sessionId;
    int _videoType;
    bool _released;
} VIDEO_DATA_COLL;

typedef QQueue<VIDEO_DATA_COLL> FRAMES;

class QMutex;
class VideoItem;

struct CropParameter {
    int originalwidth;
    int originalheight;
    int x_offset;
    int y_offset;
    int width;
    int height;
    CropParameter() : originalwidth(0), originalheight(0), x_offset(0), y_offset(0), width(0), height(0) {
    }

    void copy(const CropParameter& obj) {
        originalwidth = obj.originalwidth;
        originalheight = obj.originalheight;
        x_offset = obj.x_offset;
        y_offset = obj.y_offset;
        width = obj.width;
        height = obj.height;
    }

    // HVS:(eglCreateImageKHR) source/destination resolution and offsets must be even numbers!
    void ensureEven() {
        if (x_offset % 2 != 0)
            x_offset--;

        if (y_offset % 2 != 0)
            y_offset--;

        if (width % 2 != 0)
            width--;

        if (height % 2 != 0)
            height--;
    }
};

class VideoStream
{
public:
    VideoStream(int type);
    void frameFromMS();
    void updateFrame();
#if 0
    void frameFromMS(t_callback_event*, int type);
    void updateFrame(t_callback_event*, int type);
#endif
    void setVideoItem(VideoItem* item);
    void clearVideoItemIfEqual(VideoItem* item);

    void lock() { m_frameMutex.lock(); }
    void unlock() { m_frameMutex.unlock(); }

    VIDEO_DATA_COLL& getFrame() { return m_frame; }

    CropParameter& getSrcParameter() { return m_src_parameter; }
    CropParameter& getDesParameter() { return m_dst_parameter; }

    void setTextureID(int id) {m_textureId = id;}
    int textureID() {return m_textureId;}

private:
    void releaseFrameBuffer();
    void releaseFrameBufferAgain(int sessionId, int type);
    void updateResolutionByFrame(int w, int h);
    void updateResolution();
    void calculateResolution(int src_w, int src_h, int des_w, int des_h);

private:
    VideoItem* m_videoItem;

    QMutex m_frameMutex;
    VIDEO_DATA_COLL m_frame;
    static FRAMES m_frames_preview_failed;

    CropParameter m_src_parameter;
    CropParameter m_dst_parameter;

    QVideoImageStorage* m_qvideoImageStorageInstance;

    int m_textureId;
};

class VideoItemUpdater
{
public:
    static void initialize();
    static VideoStream* getVideoStream(int type);

    //static VideoStream* encStream;
    //static VideoStream* decStream;
    static VideoStream* Stream[ARGB_WINDOW_MAX];
};

#endif // DEMO_VIDEOSTREAM_H
