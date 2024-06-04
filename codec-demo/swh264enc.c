// SPDX-License-Identifier: MIT
// Original work Copyright 2001 Fabrice Bellard
// Modified work Copyright 2024 Synaptics Incorporated

/**
 * @file
 * video encoding with libavcodec API.
 *
 * @example ffmpeg_dspg_enc.c
 */
#include "swh264enc.h"
#include "UIInterface.h"
#include "encoder_interface_sw.h"

//#define DUMP_DECODED_DATA 1

#if DUMP_DECODED_DATA
static void dumpData(char *data, int data_size,int num_frames)
{
	static FILE *fp = NULL;
	static int iCnt;
	if (iCnt < num_frames) {
		if(!iCnt){
			fp = fopen("/tmp/encoder_output.264","wb");
		} else {
			fp = fopen("/tmp/encoder_output.264","ab");
		}
		fwrite(data, 1, data_size, fp);
		fsync(fileno(fp));
		fclose(fp);
		iCnt++;
	}
}
#endif

int RenderSWEncDisplayBufferRemote(enc_config_t *pCfg, int i) {
	int ret;
  	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t*)pCfg->priv;
	ffmpeg_enc *ffmpegEnc = (ffmpeg_enc *)pEnc->ffmpeg_enc;
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);

	pEnc->bH264InBufWithQtRdS[i] = true;
	sendProcessDMABufId(pCfg->display_window, pEnc->pH264EncInSWBuffer[i]->mShareFd, NULL);
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return 0;
}

int PrepareENCDecDisplayBufferRemote(void *arg) {

	int ret = -1;
	int width, height, size, pixelType, format;
	enc_config_t *pCfg = (enc_config_t*)arg;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	if(pCfg == NULL) {
		LOG_ERR("Fn(%s): ERROR: pCfg(%p)\n", __func__, pCfg);
		goto out;
	}
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t*)pCfg->priv;
	ffmpeg_enc *ffmpegEnc = (ffmpeg_enc *)pEnc->ffmpeg_enc;
	if ((pCfg->src == FILE_NV12)||(pCfg->src == CAM_NV12)||(pCfg->src == FILE_YUYV)||(pCfg->src == CAM_YUYV)||(pCfg->src == CAM_H264))
	{
		pixelType = PIXEL_NV12;
	}
	else
		LOG_ERR("SWH264ENC: pix fmt not supported\n");

//TBD:: check why "format = GBM_FORMAT_NV12" not supported
	format = GBM_FORMAT_ARGB8888;

	if(ffmpegEnc->img_buf_size > 0){
		ret = AllocateDRMBuf(pEnc->pH264EncInSWBuffer, H264_SWENC_IN_BUF_COUNT, pCfg->width, pCfg->height,format);
	}else{
		LOG_ERR("ERROR: AllocateDRMBuf size(%d)\n", ffmpegEnc->img_buf_size);
		goto out;
	}

	/*Send Init msg to UI about this ARGB window */
	if(pCfg->display_window) {
		SendInitMessageToUI(SESS_SWENC,
				pCfg,
				(pCfg->display_window-1),
				H264_SWENC_IN_BUF_COUNT,
				pCfg->width,
				pCfg->height,
				pixelType);
	}
	ret = 0;

out:
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return ret;
}

int swenc_get_input_handle(enc_config_t *pCfg, int32_t *index)
{
	int ret = -1;
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;
	struct ffmpeg_enc *ffmpegenc = pEnc->ffmpeg_enc;
	int i;
	int j = pEnc->last_index;

//	LOG_DEF("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	*index = -1;
	for (i = 0; i < H264_SWENC_IN_BUF_COUNT; i++)
	{
		if(++j == 4)
			j=0;

		if ((pEnc->bH264InBufWithEnc[j] == false) && (pEnc->bH264InBufWithQtRdS[j] == false))
		{
			*index = j;
			pEnc->last_index = j;
			ret = 0;
			return ret;
		}
	}

	return ret;
}

int swh264enc_encode(int frame_size, int index, ffmpeg_enc *ffmpeg_enc, enc_config_t *pCfg )
{
	int ret =0 ,i,size,header = 0,pts;
	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;

	pEnc->bH264InBufWithEnc[index] = true;

	if((pCfg->display_window < MAX_ENC_INSTANCE) && (pCfg->display_window > 0))
		RenderSWEncDisplayBufferRemote(pCfg, index);

	writeToFrame((void *)pCfg,index);

	if (ffmpeg_enc->frame) {
		ffmpeg_enc->frame->pts++;
	}

	ret = avcodec_send_frame(ffmpeg_enc->c, ffmpeg_enc->frame);
	if (ret < 0) {
		LOG_ERR("SWH264ENC: Error sending a packet for encoding\n");
		goto out;
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(ffmpeg_enc->c, ffmpeg_enc->pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			goto out;
		else if (ret < 0) {
			LOG_ERR("SWH264ENC: Error during encoding\n");
			goto out;
		}

		/*Count the encoded frames*/
		pEnc->total_bitrate += ffmpeg_enc->pkt->size;
		pEnc->total_frames = ffmpeg_enc->pkt->pts;
		size = ffmpeg_enc->pkt->size;
		pts = ffmpeg_enc->pkt->pts;

		if(pCfg->display_window == 0) {
#if DUMP_DECODED_DATA
			dumpData(ffmpeg_enc->pkt->data,size,100);
#endif
		}
		else if (pCfg->display_window < MAX_ENC_INSTANCE)
		{
			//LOG_DEF("SYNA: codec-demo: Sending FD index(%d)\n", i );
		}
		else
			LOG_ERR("SWH264ENC:Error: Invalid Display ID(%d)\n", pCfg->display_window);

try_again:
		if (!pEnc->is_running)
		{
			goto out;
		}

		if(strcmp(pCfg->ip, "0.0.0.0") != 0)
		{
			gst_tx_t *pGstTx = &(pEnc->gstTx);
			ret = FeedGstTx(pGstTx, ffmpeg_enc->pkt->data, size ,pts);
			if (ret != 0 && (!header))
			{
				usleep(5*1000);
				goto try_again;
			}
			else
			{
				header = 1;
			}
		}
	}


#if 0 /* if YV12toNV12 Required */
	convertYV12toNV12(pEnc->pTmpBuf[0]->vAddr, pEnc->video_dst_data[0],
			pEnc->frame->width * pEnc->frame->height);
#endif
out:
	return 0;
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
}
int SetupSWH264Enc(enc_config_t *pCfg) {

	int ret = -1;
	int i;
	priv_sw_encoder_t *pEnc;
	int size;

	pEnc = (priv_sw_encoder_t*)pCfg->priv;
	ffmpeg_enc *ffmpegEnc = (ffmpeg_enc *)pEnc->ffmpeg_enc;

	memset(ffmpegEnc, 0, sizeof(ffmpeg_enc));
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);

	if(pCfg->src != CAM_H264)
	{
		ffmpegEnc->pkt = av_packet_alloc();
		if (!ffmpegEnc->pkt){
			LOG_ERR("SWH264ENC: av_packet_alloc() failed\n");
			goto out;
		}

		ffmpegEnc->pkt->data = NULL;
		ffmpegEnc->pkt->size =0;

		ffmpegEnc->codec = avcodec_find_encoder_by_name("libx264");
		if (!ffmpegEnc->codec) {
			LOG_ERR("SWH264ENC: avcodec_find_encoder() failed\n");
			goto out;
		}

		usleep(10000); /*Needed before parser init when multi-session*/

		ffmpegEnc->c = avcodec_alloc_context3(ffmpegEnc->codec);
		if (!ffmpegEnc->c) {
			LOG_ERR("SWH264ENC:Could not allocate video codec context\n");
			goto out;

		}

		/* put sample parameters */
		ffmpegEnc->c->bit_rate =  pCfg->bitrate*1000;
		/* resolution must be a multiple of two */
		ffmpegEnc->c->width = pCfg->width;
		ffmpegEnc->c->height = pCfg->height;
		/* frames per second */
		ffmpegEnc->c->time_base= (AVRational){1,pCfg->fps};
		ffmpegEnc->c->framerate = (AVRational){pCfg->fps, 1};
		ffmpegEnc->c->profile = FF_PROFILE_H264_HIGH;
		ffmpegEnc->c->level = 40;
		ffmpegEnc->c->gop_size = 1000; /* emit one intra frame every 1000 frames */
		ffmpegEnc->c->max_b_frames= 0;

		if ((pCfg->src == FILE_NV12)||(pCfg->src == CAM_NV12)||(pCfg->src == FILE_YUYV)||(pCfg->src == CAM_YUYV)||(pCfg->src == CAM_H264))
		{
			ffmpegEnc->c->pix_fmt = AV_PIX_FMT_NV12;
		}
		else
			LOG_ERR("SWH264ENC: pix fmt not supported\n");

		av_opt_set(ffmpegEnc->c->priv_data, "preset", "ultrafast", 0);

		ffmpegEnc->c->thread_count = 2;  // Change Thread_count to enable multithreading
		if (ffmpegEnc->codec->capabilities & AV_CODEC_CAP_FRAME_THREADS)
			ffmpegEnc->c->thread_type = FF_THREAD_FRAME;
		else if (ffmpegEnc->codec->capabilities & AV_CODEC_CAP_SLICE_THREADS)
			ffmpegEnc->c->thread_type = FF_THREAD_SLICE;
		else
			ffmpegEnc->c->thread_count = 1; //don't use multithreading*/

		/* open it */
		if (avcodec_open2(ffmpegEnc->c, ffmpegEnc->codec, NULL) < 0) {
			LOG_ERR("SWH264ENC: Could not open codec\n");
			goto out;
		}
		ffmpegEnc->frame = av_frame_alloc();
		if (!ffmpegEnc->frame) {
			LOG_ERR("SWH264ENC: Could not allocate video frame\n");
			goto out;
		}

		ffmpegEnc->frame->format = ffmpegEnc->c->pix_fmt;
		ffmpegEnc->frame->width  = ffmpegEnc->c->width;
		ffmpegEnc->frame->height = ffmpegEnc->c->height;

		ret = av_frame_get_buffer(ffmpegEnc->frame,0);
		if (ret < 0) {
			LOG_ERR("SWH264ENC: Could not allocate raw picture buffer\n");
			exit(1);
		}

		ffmpegEnc->img_buf_size = av_image_get_buffer_size(ffmpegEnc->c->pix_fmt,ffmpegEnc->c->width,ffmpegEnc->c->height,1);

		PrepareENCDecDisplayBufferRemote(pCfg);

		for (i =0; i< H264_SWENC_IN_BUF_COUNT; i++)
		{
			if(pEnc->pH264EncInSWBuffer[i] != NULL) {
				pEnc->bH264InBufWithEnc[i] = false;
				pEnc->bH264InBufWithQtRdS[i] = false;
			}
			else
				LOG_ERR("Failed input allocation  \n");
		}

		LOG_DEF(" \n ffmpegEnc->img_buf_size : %d \n",ffmpegEnc->img_buf_size);

		ffmpegEnc->frame->pts = 0;
	}
	pEnc->last_index = -1;

	ret = 0;

out:
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return ret;

}

int swh264enc_stop(enc_config_t *pCfg){

	priv_sw_encoder_t *pEnc;
	int enc_inst,i;

	pEnc = (priv_sw_encoder_t *)pCfg->priv;
	ffmpeg_enc *ffmpegEnc = (ffmpeg_enc *)pEnc->ffmpeg_enc;

	enc_inst = ffmpegEnc->enc_inst;

	/* flush the encoder */
	//LOG_DEF("Flush encoder\n");
	//encode();

	/*close dec inst*/
	avcodec_free_context(&ffmpegEnc->c);
	av_frame_free(&ffmpegEnc->frame);
	av_packet_free(&ffmpegEnc->pkt);

	FreeDRMBuf(pEnc->pH264EncInSWBuffer,H264_SWENC_IN_BUF_COUNT,pCfg->width,pCfg->height,GBM_FORMAT_ARGB8888);

	memset(pEnc, 0,sizeof(ffmpegEnc));

	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return 0;
}

int writeToFrame(void *arg,int buf_index){
	int ret = 0,i;
	enc_config_t *pCfg = (enc_config_t*)arg;
	priv_sw_encoder_t *pEnc =(priv_sw_encoder_t *)pCfg->priv;
	ffmpeg_enc *ffmpegEnc = (ffmpeg_enc *)pEnc->ffmpeg_enc;

	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);

	ret = av_frame_make_writable(ffmpegEnc->frame);
	if (ret < 0){
		LOG_ERR("SWH264ENC: Could not make the frame writable\n");
    }

	ret = av_image_fill_arrays(ffmpegEnc->frame->data,ffmpegEnc->frame->linesize,pEnc->pH264EncInSWBuffer[buf_index]->vAddr,
		ffmpegEnc->c->pix_fmt,ffmpegEnc->c->width,ffmpegEnc->c->height,1);

   	if(ret != ffmpegEnc->img_buf_size)
	{
		LOG_ERR("SWH264ENC: av_image_fill_arrays failed \n");
		ret = -1;
	}
	else {
		pEnc->bH264InBufWithEnc[buf_index] = false;
	}

	return ret;
}
void enqueBufFromUISWEnc(enc_config_t *pCfg, int shareFd)
{
	if(pCfg == NULL)
		return;

	priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfg->priv;
	ffmpeg_enc *ffmpegEnc = (ffmpeg_enc *)pEnc->ffmpeg_enc;

	int i;

	for(i = 0; i < H264_SWENC_IN_BUF_COUNT; i++){
		if(pEnc->pH264EncInSWBuffer[i]->mShareFd == shareFd){
			pEnc->bH264InBufWithQtRdS[i] = false;
			break;
		}
	}

	if( i == H264_SWENC_IN_BUF_COUNT){
		LOG_ERR("QTRDS: shareFd[%d] notfound\n", shareFd);
	}
}
