#ifndef _V4L2HDMI_H_
#define _V4L2HDMI_H_

#include "encoder_interface_v4l2.h"

#define HDMI_RX    3

#define HDMI_RX_IN_BUF_COUNT 9
#if 0
#define HDMI_VIDEO_DEVICE "/dev/video250"

#define v4l2_ioctl ioctl
#define v4l2_close close
#define v4l2_mmap mmap

#define HDMIRX_VIDEO_INFO   "/sys/devices/virtual/video4linux/video250/hdmirx_video_info"
#define HDMIRX_STATUS   	"/sys/class/switch/rx_video/state"
#endif

int SetupHdmiRx(enc_config_t *pCfg);
int ShutdownHdmiRx(enc_config_t *pCfg);
int ReadHdmiRxFrame(enc_config_t *pCfg, int* fb_index, int* size);
int EnqueHdmiRxBuffer(enc_config_t *pCfg, int index);
void StopHdmiRX(enc_config_t *pCfg);


#endif //_V4L2HDMI_H_
