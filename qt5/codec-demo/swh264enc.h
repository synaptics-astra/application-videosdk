// SPDX-License-Identifier: MIT
// Original work Copyright 2001 Fabrice Bellard
// Modified work Copyright 2024 Synaptics Incorporated

#ifndef __SW_H264_ENC_H_
#define __SW_H264_ENC_H_

#include "config.h"

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/avassert.h>
#include <libavutil/intreadwrite.h>

#define MAX_ENC_INSTANCE 8

typedef struct {
	int enc_inst;
	const AVCodec *codec;
	AVCodecContext *c;
	AVFrame *frame;
	AVPacket *pkt;
	int inbufLen;
	int is_running;
	long frame_duration_us;
	struct timeval time_stamp;
	int img_buf_size;
	int eos;
}ffmpeg_enc;

int SetupSWH264Enc(enc_config_t *pCfg);
int swh264enc_stop(enc_config_t *pCfg);
int swh264enc_encode (int frame_size, int index, ffmpeg_enc *ffmpeg_enc, enc_config_t *pCfg);
int swenc_get_input_handle(enc_config_t *pCfg, int32_t *index);

void setSWEncBitrate (enc_config_t *pCfg, unsigned int rate);
void setSWEncFramerate (enc_config_t *pCfg, unsigned int fps_n, unsigned int fps_d);
void setSWEncIDRFrameRequest (enc_config_t *pCfg);
int writeToFrame(void *arg,int buf_index);
void enqueBufFromUISWEnc(enc_config_t *pCfg, int shareFd);

#endif//__FFMPEG_DSPG_ENC__
