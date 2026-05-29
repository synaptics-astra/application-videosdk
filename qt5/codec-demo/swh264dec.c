// SPDX-License-Identifier: MIT
// Original work Copyright 2001 Fabrice Bellard
// Modified work Copyright 2024 Synaptics Incorporated

/**
 * @file
 * video decoding with libavcodec API.
 *
 */
#include "swh264dec.h"
#include "UIInterface.h"
#include "decoder_interface_sw.h"

//#define DUMP_DECODED_DATA 1

#if DUMP_DECODED_DATA
static void dumpData(char *data, int data_size,int num_frames)
{
	static FILE *fp = NULL;
	static int iCnt;
	if (iCnt < num_frames) {
		if(!iCnt){
			fp = fopen("/tmp/decoder_output.raw","wb");
		} else {
			fp = fopen("/tmp/decoder_output.raw","ab");
		}
		fwrite(data, 1, data_size, fp);
		fsync(fileno(fp));
		fclose(fp);
		iCnt++;
	}
}
#endif

int media_image_copy_to_buffer(void *arg, int bufIndex)
{

	int i, k, align = 1, ret;
	dec_config_t *pCfg = (dec_config_t*)arg;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;
	ffmpeg_dec *ffmpegDec = (ffmpeg_dec*)pDec->ffmpeg_dec;

	uint8_t *dst = pDec->pH264DecOutSWBuffer[bufIndex]->vAddr;
	int dst_size = ffmpegDec->frame->width * ffmpegDec->frame->height * 3 / 2;//for 420-YUV
	int width = ffmpegDec->frame->width;
	int height = ffmpegDec->frame->height;

	if(dst == NULL){
		LOG_DEF("Dest add is NULL\n");
		return AVERROR(EINVAL);
	}

	if(ffmpegDec->desc == NULL){
		ffmpegDec->desc = av_pix_fmt_desc_get(ffmpegDec->frame->format);
	}

	if (dst_size < 0 || !ffmpegDec->desc)
		return AVERROR(EINVAL);

	if(ffmpegDec->nb_planes == 0){
		for (i = 0; i < ffmpegDec->desc->nb_components; i++)
			ffmpegDec->nb_planes = FFMAX(ffmpegDec->desc->comp[i].plane, ffmpegDec->nb_planes);
	}

	if(ffmpegDec->linesize[0] == 0){
		ret = av_image_fill_linesizes(ffmpegDec->linesize, ffmpegDec->frame->format, width);
		av_assert0(ret >= 0);
	}


	for (i = 0; i < ffmpegDec->nb_planes; i++) {
		int h, shift = (i == 1 ) ? ffmpegDec->desc->log2_chroma_h : 0;
		const uint8_t *src = ffmpegDec->frame->data[i];
		h = (height + (1 << shift) - 1) >> shift;

		if(FFALIGN(ffmpegDec->linesize[i], align) == ffmpegDec->frame->linesize[i]){
			LOG_FUNC("No Alignment issue:\n");
			if(i == 0) {
				memcpy_neon(dst, src, h * ffmpegDec->linesize[i]);
				//dst += h * ffmpegDec->frame->linesize[i];
				dst += (width * (height));
			}else if( i == 1 ) {
				const uint8_t *src_u = ffmpegDec->frame->data[1];
				const uint8_t *src_v = ffmpegDec->frame->data[2];
				for(k = 0; k < h * ffmpegDec->linesize[i]; k++){
					*dst++ = *src_u++;
					*dst++ = *src_v++;
				}
			}

		}else{
			LOG_DEF("ERROR: linesize Alignment issue:\n");
			dst_size = 0;
		}
	}
	return dst_size;
}

static void SleepToMaintainFps(void *arg)
{
	dec_config_t *pCfg = (dec_config_t*)arg;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t*)pCfg->priv;
	struct timeval end;

	gettimeofday(&end, NULL);
	if(pDec->time_stamp.tv_sec){
		long seconds = (end.tv_sec - pDec->time_stamp.tv_sec);
		long micros = ((seconds * 1000000) + end.tv_usec) - (pDec->time_stamp.tv_usec);

		long usec_sleep = pDec->frame_duration_us - micros;
		if (usec_sleep > 0 )
		{
			usleep(usec_sleep);
		}
	}

	gettimeofday(&pDec->time_stamp, NULL);
}
int PrepareSWDecDisplayBufferRemote(void *arg) {

	int ret = -1;
	int width, height, size, pixelType;
	dec_config_t *pCfg = (dec_config_t*)arg;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	if(pCfg == NULL) {
		LOG_ERR("Fn(%s): ERROR: pCfg(%p)\n", __func__, pCfg);
		goto out;
	}
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t*)pCfg->priv;
	ffmpeg_dec *ffmpegDec =(ffmpeg_dec*)pDec->ffmpeg_dec;
	pCfg->format = GBM_FORMAT_ARGB8888; //Default format

	width	= pCfg->width;
	height	= pCfg->height;

	switch(pCfg->format){
		case GBM_FORMAT_NV12:
			{
				size = width * height * 3 / 2;
				pixelType = PIXEL_NV12;
				break;
			}
		case GBM_FORMAT_ARGB8888:
			{
				size = width * height * 4;
				pixelType = PIXEL_ARGB;
				break;
			}
		default:
			{
				size = 0;
				pixelType = -1;
				LOG_DEF("SYNA: Error: Unsupported format(%d)\n", pCfg->format);
			}
	}

	if(size > 0){
		ret = AllocateDRMBuf(pDec->pH264DecOutSWBuffer, H264_SWDEC_OUT_BUF_COUNT, width, height, pCfg->format);
	}else{
		LOG_ERR("ERROR: AllocateDRMBuf size(%d)\n", size);
		goto out;
	}

	/*Send Init msg to UI about this ARGB window */
	if(pCfg->display_window) {
		SendInitMessageToUI(SESS_SWDEC,
				pCfg,
				(pCfg->display_window-1),
				H264_SWDEC_OUT_BUF_COUNT,
				pCfg->width,
				pCfg->height,
				PIXEL_NV12);
	}
	ret = 0;

out:
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return ret;
}

int RenderSWDecDisplayBufferRemote(dec_config_t *pCfg, int i) {

	int ret;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
//	pDec->bH264SWDecBufferWithUs[i] = true;

	sendProcessDMABufId(pCfg->display_window, pDec->pH264DecOutSWBuffer[i]->mShareFd, NULL);
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);

	return 0;
}
void convertYV12toNV12( uint8_t *destData, uint8_t *srcData, int size){

	int i;
	int qtr = size / 4;
	int vPos = size; // This is where V starts
	int uPos = size + qtr; // This is where U starts

	memcpy(destData, srcData, size); // Copy Y
	for(i = 0; i < qtr ; i++){
			destData[size + i*2 ] = srcData[vPos + i]; // For NV21, V first
			destData[size + i*2 + 1] = srcData[uPos + i]; // For Nv21, U second
	}
}

int decode(void *arg)
{
	int ret,i,size, index, count = 1;
	static int pastIndex = 0;
	dec_config_t *pCfg = (dec_config_t*)arg;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t*)pCfg->priv;

	ffmpeg_dec *ffmpegDec = (ffmpeg_dec*)pDec->ffmpeg_dec;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);

	ret = avcodec_send_packet(ffmpegDec->c, ffmpegDec->pkt);
	if (ret < 0) {
		LOG_ERR("Error sending a packet for decoding\n");
		goto out;
	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(ffmpegDec->c, ffmpegDec->frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			goto out;
		else if (ret < 0) {
			LOG_ERR("Error during decoding\n");
			goto out;
		}

		/*Count the decoded frames*/
		pDec->total_frames++;

		//pDec->bH264SWDecInBufferWithUs[i] = false;
		index = pastIndex;
		for(i = 0; i < H264_SWDEC_OUT_BUF_COUNT; i++){
			index++;
			if(index == H264_SWDEC_OUT_BUF_COUNT)
				index = 0;
			if(pDec->bH264SWDecBufferWithUs[index] == true){
				pDec->bH264SWDecBufferWithUs[index] = false;
				break;
			}
		}
		pastIndex = index;
		if(index < H264_SWDEC_OUT_BUF_COUNT){

			/* copy decoded frame to destination buffer:
				* this is required since rawvideo expects non aligned data */
			ret = media_image_copy_to_buffer(pCfg, index);


			if(pCfg->display_window == 0) {
#if DUMP_DECODED_DATA
				size = ffmpegDec->frame->width * ffmpegDec->frame->height * 3/2;
				dumpData(pDec->pH264DecOutSWBuffer[i]->vAddr,size,100);
#endif
				pDec->bH264SWDecBufferWithUs[i] = true;
			}
			else if(pCfg->display_window < MAX_DEC_INSTANCE) {
				LOG_FUNC("SYNA: codec-demo: Sending FD index(%d)\n", index );
				SleepToMaintainFps(pCfg);
				RenderSWDecDisplayBufferRemote(pCfg, index);
			}
			else
				LOG_DEF("Error: Invalid Display ID(%d)\n", pCfg->display_window);

			//SleepToMaintainFps(pCfg);
		}else{
			LOG_FUNC("No free inBuf for SWDEC\n");
		}
	}

out:
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
}


int SetupSWH264Dec(dec_config_t *pCfg) {

	int ret = -1;
	int i,buf_size;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);

	ffmpeg_dec *ffmpegDec = (ffmpeg_dec *)pDec->ffmpeg_dec;
	memset(ffmpegDec, 0, sizeof(ffmpeg_dec));

	//allocate the au buffer size considering NV12 with 50% compression
	//buf_size calculating with (width*height*3/2)/2

	buf_size = pCfg->width*pCfg->height*3/4;


	ffmpegDec->pkt = av_packet_alloc();
	if (!ffmpegDec->pkt){
		LOG_ERR("av_packet_alloc() failed\n");
		goto out;
	}

	ffmpegDec->codec = avcodec_find_decoder_by_name("h264");
	if (!ffmpegDec->codec) {
		LOG_ERR("avcodec_find_decoder() failed\n");
		goto out;
	}
		LOG_DEF("avcodec_find_decoder() success : %p\n",ffmpegDec->codec);

	usleep(10000); /*Needed before parser init when multi-session*/

	ffmpegDec->c = avcodec_alloc_context3(ffmpegDec->codec);
	if (!ffmpegDec->c) {
		LOG_ERR("Could not allocate video codec context\n");
		goto out;
	}
	LOG_DEF("avcodec_alloc_context3() success %p\n",ffmpegDec->c);

	ffmpegDec->c->thread_count = 2;  // Change Thread_count to enable multithreading
	if (ffmpegDec->codec->capabilities & AV_CODEC_CAP_FRAME_THREADS)
		ffmpegDec->c->thread_type = FF_THREAD_FRAME;
	else if (ffmpegDec->codec->capabilities & AV_CODEC_CAP_SLICE_THREADS)
		ffmpegDec->c->thread_type = FF_THREAD_SLICE;
	else
		ffmpegDec->c->thread_count = 1; //don't use multithreading*/

	/* open it */
	if (avcodec_open2(ffmpegDec->c, ffmpegDec->codec, NULL) < 0) {
		LOG_ERR("Could not open codec\n");
		goto out;
	}
	LOG_DEF("avcodec_open2() success\n");

	ffmpegDec->frame = av_frame_alloc();
	if (!ffmpegDec->frame) {
		LOG_ERR("Could not allocate video frame\n");
		goto out;
	}

	/*Allocate contegious mem Display Buf*/
	PrepareSWDecDisplayBufferRemote(pCfg);

	pDec->gstRx.h264DecoderQueueSem = calloc(1,sizeof(tsem_t));
	tsem_init(pDec->gstRx.h264DecoderQueueSem, 0);

	pDec->gstRx.h264DecoderStreamQueue = calloc(1,sizeof(queue_t));
	ret = queue_init(pDec->gstRx.h264DecoderStreamQueue);

	if(ret<0)
	{
		LOG_ERR("Could not init queue\n");
	}
	else
		LOG_DEF("gstRx.h264DecoderStreamQueue : %p \n",pDec->gstRx.h264DecoderStreamQueue);

	if (pCfg->src == FILE_H264) {
		//Reducing·frame_duration by 10%·to·catchup·w.r.t·gst·rx·pipeline
		pDec->frame_duration_us = (1000*1000 * 0.9)/pCfg->fps;
		gettimeofday(&pDec->time_stamp, NULL);
	}

	//pDec->frame_duration_us = (1000*1000)/pCfg->fps;

	for(i = 0; i < H264_SWDEC_OUT_BUF_COUNT; i++){
		pDec->bH264SWDecBufferWithUs[i] = true;
	}

	ret = 0;
	pDec->is_running = 1;
out:
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return ret;

}
void ShutdownSWH264Decoder(dec_config_t *pCfg)
{
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;
	int ret = 0;

	// Make sure we exit from the event watcher thread
	pDec->bH264DecoderRecievedEOS = true;

	// Free buffers if any
	queue_deinit(pDec->gstRx.h264DecoderStreamQueue);

	free(pDec->gstRx.h264DecoderStreamQueue);
	free(pDec->gstRx.h264DecoderQueueSem);
	free(pDec->videoH264DecoderEofSem);

}

void StopSWH264Decoder(dec_config_t *pCfg){

	priv_sw_decoder_t *pDec;
	int dec_inst,i;

	//pCfg = (dec_config_t*)arg;
	pDec = (priv_sw_decoder_t *)pCfg->priv;
	ffmpeg_dec *ffmpegDec = (ffmpeg_dec *)pDec->ffmpeg_dec;

//	dec_inst = pDec->dec_inst;
	LOG_DEF("decode deInitDec dec_inst = %d \n",dec_inst);
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);

	/* flush the decoder */
	//::TBD
//	decode(arg);

	/*close dec inst*/
	avcodec_free_context(&ffmpegDec->c);
	av_frame_free(&ffmpegDec->frame);
	av_packet_free(&ffmpegDec->pkt);

	FreeDRMBuf(pDec->pH264DecOutSWBuffer,H264_SWDEC_OUT_BUF_COUNT,pCfg->width,pCfg->height,GBM_FORMAT_ARGB8888);
	/*Close src dest files*/
#if 0
	if(pDec->fpIn){
		fclose(pDec->fpIn);
		pDec->fpIn = NULL;
	}

	if(pDec->fpOut){
		fclose(pDec->fpOut);
		pDec->fpOut = NULL;
	}
#endif
	memset(ffmpegDec, 0,sizeof(ffmpeg_dec));

	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return;
}

int FeedBufferToSWH264Decoder(dec_config_t *pCfg ) {
	int i,ret;
	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;
	ffmpeg_dec *ffmpegDec = (ffmpeg_dec *)pDec->ffmpeg_dec;
	queue_element_t *tmpBuf = NULL;

	if(pCfg->src == CAM_H264)
	{
		codec_config_t *codec_config = (codec_config_t *)pCfg->codec_config;
		tmpBuf = dequeue(codec_config->sharedH264EncoderQueue);
	}
	else
		tmpBuf = dequeue(pDec->gstRx.h264DecoderStreamQueue);

	if(tmpBuf == NULL){
		LOG_ERR("Fn(%s): dequeue returned NULL\n", __func__);
		goto err;
	}

	ffmpegDec->pkt->data = tmpBuf->pBuffer;
	ffmpegDec->pkt->size = tmpBuf->nFilledLen;

	if (ffmpegDec->pkt->size)
	decode((void *)pCfg);

	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	free(tmpBuf->pBuffer);
	free(tmpBuf);
	return 0;
err:
	return -1;
}

void enqueBufFromUISWDec(dec_config_t *pCfg, int shareFd)
{
	if(pCfg == NULL)
		return;

	priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfg->priv;
	int i;

	if((pDec == NULL) || (!(pDec->is_running)))
		return;

	for(i = 0; i < H264_SWDEC_OUT_BUF_COUNT; i++){
		if(pDec->pH264DecOutSWBuffer[i]->mShareFd == shareFd){
			LOG_FUNC("UIServer Requested: Index to free is %d\n", i);
			pDec->bH264SWDecBufferWithUs[i] = true;
			break;
		}
	}

	if( i == H264_SWDEC_OUT_BUF_COUNT){
		LOG_ERR("enqueBufFromUI shareFd(%d) not found\n", shareFd);
	}
}
