#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "config.h"

#ifdef AMP_SUPPORT
#include "decoder_interface_v4l2.h"
#include "encoder_interface_v4l2.h"
#endif
#include "decoder_interface_sw.h"
#include "encoder_interface_sw.h"
#ifdef V4L2_SUPPORT
#include "encoder_interface_hwv4l2.h"
#include "decoder_interface_hwv4l2.h"
#endif

void startInstance(sess_config_t *sess_cfg);
void stopInstance(sess_config_t *sess_cfg);
void waitForInstanceShutdown(sess_config_t *sess_cfg);

#endif // _INSTANCE_H_
