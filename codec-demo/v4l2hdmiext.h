#ifndef _V4L2HDMIRXEXT_H_
#define _V4L2HDMIRXEXT_H_

#include "encoder_interface_v4l2.h"

#define HDMI_RX_EXT    4
#define HDMI_RX_EXT_BUF_COUNT 4

int SetupHdmiRxExt (enc_config_t * pCfg);
int ShutdownHdmiRxExt (enc_config_t * pCfg);
int ReadHdmiRxExtFrame (enc_config_t * pCfg, int *fb_index, int *size);
int EnqueHdmiRxExtBuffer (enc_config_t * pCfg, int index);
void StopHdmiRXExt (enc_config_t * pCfg);

#endif //_V4L2HDMIRXEXT_H_
