#define VIDEO_TUNER_C
#include "video_tuner_server.h"
#undef VIDEO_TUNER_C

extern int stats_enable_flag;
codec_config_t *pCodecCfg;

#define CODEC_DEMO_SERVER_DIR "/home/root/"
#define CODEC_DEMO_SERVER_SOCKET_PATH "/home/root/codec_demo_server_socket"
sess_config_t * checkValidSession (unsigned int session_id)
{
	sess_config_t *valid_sess_cfg = NULL;
	sess_config_t *sess_cfg = NULL;
	if (session_id < pCodecCfg->num_of_sessions) {
		sess_cfg = &(pCodecCfg->sess_cfg[session_id]);
		if (!sess_cfg)
		{
			LOG_ERR("Selected Invalid Session\n");
		}
		else {
			switch(sess_cfg->type)
			{
				case SESS_AMPENC:
					{
						valid_sess_cfg = sess_cfg;
					}
					break;

				case SESS_AMPDEC:
					LOG_ERR("Unsupported feature for Decoder\n");
					break;

				case SESS_SWDEC:
					LOG_ERR("Unsupported feature for Software Decoder\n");
					break;

				default:
					break;
			}
		}
	}
	return valid_sess_cfg;
}

int HandleBitrateMessage(Message_Struct* pMessage)
{
	LOG_FUNC(">> Fn(%s)\n", __func__);
	unsigned int session_id = pMessage->session_id;
	sess_config_t *sess_cfg = NULL;
	LOG_FUNC("Bitrate message has arrived for session ID %d\n", session_id);
	sess_cfg = checkValidSession(session_id);

	if (!sess_cfg)
	{
		LOG_ERR("Invalid Session ID\n");
		return -1;
	}
	unsigned int bitrate = pMessage->message.bitrate_msg.iBitrate;

	setBitrate (sess_cfg, bitrate);
	LOG_FUNC("<< Fn(%s)\n", __func__);

	return 0;
}

int HandleFramerateMessage(Message_Struct* pMessage)
{
	LOG_FUNC(">> Fn(%s)\n", __func__);
	unsigned int session_id = pMessage->session_id;
	sess_config_t *sess_cfg = NULL;
	LOG_FUNC("Framerate message has arrived for session ID %d\n", session_id);
	sess_cfg = checkValidSession(session_id);

	if (!sess_cfg)
	{
		LOG_ERR("Invalid Session ID\n");
		return -1;
	}
	unsigned int fps = pMessage->message.framerate_msg.iFpsNum;

	setFramerate (sess_cfg, fps, 1);
	LOG_FUNC("<< Fn(%s)\n", __func__);

	return 0;
}

int HandleIDRReqMessage(Message_Struct* pMessage)
{
	LOG_FUNC(">> Fn(%s)\n", __func__);
	unsigned int session_id = pMessage->session_id;
	sess_config_t *sess_cfg = NULL;
	LOG_FUNC("Framerate message has arrived for session ID %d\n", session_id);
	sess_cfg = checkValidSession(session_id);

	if (!sess_cfg)
	{
		LOG_ERR("Invalid Session ID\n");
		return -1;
	}

	setIDRFrameRequest (sess_cfg);
	LOG_FUNC("<< Fn(%s)\n", __func__);

	return 0;
}

int ReadMessageFromClient(Message_Struct *pmsg)
{
	bzero(pmsg,sizeof(Message_Struct));
	int msg_len =  sizeof(struct Message_Struct);
	char* bufferPtr = (char*)pmsg;
	int expected_len = msg_len;
	int len = 0;
	int recv_len = 0;
	fd_set ready;
	struct timeval to;
	int ret;
	LOG_FUNC(">> Fn(%s)\n", __func__);
	while(expected_len !=0){
		len = recv (client_sock,bufferPtr+len,(msg_len-recv_len),0);
		if (len == 0)
		{
			LOG_ERR("Remote end closed unexpetedly..!!\n");
			return -1;
		}
		LOG_DEF("The message received with expected len %d and incoming mes lenghth %d\n",expected_len,len);
		recv_len += len;
		expected_len -= len;
	}
	LOG_FUNC("<< Fn(%s)\n", __func__);
	return 0;
}

void* server_listening_thread(void* arg)
{
	socklen_t remote_len, local_len;
	int ret = 0;
	char cmd[256] = {0};
	struct sockaddr_un remote, local;
	Message_Struct msg;
	pthread_setname_np(pthread_self(), "CodecDemoServer");
	LOG_FUNC(">> Fn(%s)\n", __func__);
	if(!fServerIPCInitDone){

		if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			ret = -1;
			pthread_exit(0);
		}

		LOG_DEF("Trying to connect to a client...\n");
		snprintf(cmd, 256, "mkdir -p %s", CODEC_DEMO_SERVER_DIR);
		system(cmd);
		local.sun_family = AF_UNIX;
		bzero(local.sun_path, sizeof(local.sun_path));
		strcpy(local.sun_path, CODEC_DEMO_SERVER_SOCKET_PATH);
		local_len = strlen(local.sun_path) + sizeof(local.sun_family);
		unlink(local.sun_path);
		if (bind(server_sock, (struct sockaddr *)&local, local_len) == -1) {
			perror("bind");
			pthread_exit(0);
		}
		if (listen(server_sock, 5) == -1) {
			perror("listen");
			pthread_exit(0);
		}
		fServerIPCInitDone = 1;
	}

reconnect_client:
	remote_len = sizeof(remote);
	LOG_DEF("Waiting for a connection...\n");
	if ((client_sock = accept(server_sock, (struct sockaddr *)&remote,(socklen_t *) &remote_len)) == -1) {
		LOG_DEF("No Client to Connect ....\n");
		perror("accept");
		//pthread_exit(0);
	}
	LOG_DEF("Connected to Client....\n");

	while(fServerIPCInitDone != 0) {
		if (ReadMessageFromClient(&msg) < 0)
		{
			LOG_ERR("ReadMessageFromClient has an error. %s\n", __func__);
			LOG_ERR("Trying to wait for other client to connect!!!\n");
			close(client_sock);
			goto reconnect_client;
		}

		LOG_FUNC("Received message with m_messageType %d\n",msg.m_messageType);
		switch(msg.m_messageType){
			case SET_BITRATE_MSG:
				ret = HandleBitrateMessage(&msg);
				LOG_DEF("Set Bitrate returned : %d\n", ret);
				break;
			case SET_FRAMERATE_MSG:
				ret = HandleFramerateMessage(&msg);
				LOG_DEF("Set Framerate returned : %d\n", ret);
				break;
			case SET_FORCE_IDR_MSG:
				ret = HandleIDRReqMessage(&msg);
				LOG_DEF("Set IDR Request returned : %d\n", ret);
				break;
			case SET_RESOLUTION_MSG:
				//ret = msg.params[0]; // store the return value from the received msg
				LOG_ERR("Set Resolution Not Supported:\n");
				break;
			case SET_STATISTICS_FLAG_MSG:
				stats_enable_flag ^= 1;
				if (stats_enable_flag)
					LOG_ERR("Statistics Enabled:\n");
				else
					LOG_ERR("Statistics Disabled:\n");
				break;
			default:
				LOG_ERR("Dropped unknown message type %d\n",msg.m_messageType);
				break;
		}
	}
	LOG_FUNC("<< Fn(%s)\n", __func__);
}

int InitServerIPC(codec_config_t *p_codec_config)
{
	int ret;
	fServerIPCInitDone = 0;
	server_sock = -1;
	client_sock = -1;
	pCodecCfg = p_codec_config;

	LOG_FUNC(">> Fn(%s)\n", __func__);
	// Create Thread for creating socket and listening msg from Rendering Server.
	ret = pthread_create(&serverlistenThread, NULL, server_listening_thread, (void*)pCodecCfg);
	if (ret) {
		LOG_ERR("Thread creating (server_listening_thread) Error\n");
		ret = -1;
		goto out;
	}
out:
	LOG_FUNC("<< Fn(%s)\n", __func__);
	return ret;
}

int DeInitServerIPC()
{
	fServerIPCInitDone = 0;
	pthread_cancel(serverlistenThread);

	close(server_sock);
	close(client_sock);

	pthread_join(serverlistenThread, NULL);
	return 0;
}