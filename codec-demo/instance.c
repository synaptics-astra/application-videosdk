// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "instance.h"

void startInstance(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);

	switch(sess_cfg->type)
	{
		case SESS_SWDEC:
			startSWDecoder(sess_cfg);
			break;

		case SESS_SWENC:
			startSWEncoder(sess_cfg);
			break;

#ifdef V4L2_SUPPORT
		case SESS_HWENC:
			startHWV4L2Encoder(sess_cfg);
			break;

		case SESS_HWDEC:
			startHWV4L2Decoder(sess_cfg);
			break;
#endif
		default:
			break;
	}
}

void stopInstance(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);

	switch(sess_cfg->type)
	{
		case SESS_SWDEC:
			stopSWDecoder(sess_cfg);
			break;

		case SESS_SWENC:
			stopSWEncoder(sess_cfg);
			break;

#ifdef V4L2_SUPPORT
		case SESS_HWENC:
			stopHWV4L2Encoder(sess_cfg);
			break;

		case SESS_HWDEC:
			stopHWV4L2Decoder(sess_cfg);
			break;
#endif
		default:
			break;
	}
}

void waitForInstanceShutdown(sess_config_t *sess_cfg)
{
	LOG_DEF("%s: \n", __func__);

	switch(sess_cfg->type)
	{

		case SESS_SWENC:
			waitForSWEncoderShutdown(sess_cfg);
		break;

		case SESS_SWDEC:
			waitForSWDecoderShutdown(sess_cfg);
		break;

#ifdef V4L2_SUPPORT
		case SESS_HWENC:
			waitForHWV4L2EncoderShutdown(sess_cfg);
		break;

		case SESS_HWDEC:
			waitForHWV4L2DecoderShutdown(sess_cfg);
		break;
#endif
		default:
			break;
	}
}
