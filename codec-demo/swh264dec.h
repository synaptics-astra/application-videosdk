// SPDX-License-Identifier: MIT
// Original work Copyright 2001 Fabrice Bellard
// Modified work Copyright 2024 Synaptics Incorporated

#ifndef _SWH264DEC_H_
#define _SWH264DEC_H_

#include "config.h"
#include "neon_utils.h"

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/avassert.h>
#include <libavutil/intreadwrite.h>

#define MAX_DEC_INSTANCE 8
#define H264_SWDEC_OUT_BUF_COUNT 8 /*Default*/

typedef struct {
	int dec_inst;
	const AVCodec *codec;
	AVCodecContext *c;
	AVFrame *frame;
	AVCodecParserContext *parser;
	AVPacket *pkt;
	/*Decoder internal*/
	const AVPixFmtDescriptor *desc;
	int nb_planes;
	int linesize[H264_SWDEC_OUT_BUF_COUNT];
}ffmpeg_dec;

int addToPacket(void *arg);
int SetupSWH264Dec(dec_config_t *pCfg);
int GetSWH264DecFreeInputBuffer(dec_config_t *pCfg, int *index);
int FeedBufferToSWH264Decoder(dec_config_t *pCfg);
void StopSWH264Decoder(dec_config_t *pCfg);
void ShutdownSWH264Decoder(dec_config_t *pCfg);
void enqueBufFromUISWDec(dec_config_t *pCfg, int shareFd);
//int StreamOffPort(dec_config_t *pCfg, unsigned int type);

#endif //_SW2H264DEC_H_
