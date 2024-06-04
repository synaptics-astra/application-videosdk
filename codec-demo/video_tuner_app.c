// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

/************************************************************/
/* This is a stream socket client program for codec demo	*/
/* using unix domain sockets.								*/
/* This program creates a socket, connects to a codec-demo,	*/
/* sends data, then receives and prints the result message	*/
/* from the code-demo server.								*/
/************************************************************/

#include "video_tuner_app.h"

int sendMessageToCodecDemo(unsigned char* psendMessage)
{
	int sent = 0, remaining = sizeof(Message_Struct),sent_step = 0;
	while(sent!=sizeof(Message_Struct)){
		sent_step = send(client_sock,psendMessage+sent,remaining,0);
		sent += sent_step;
		remaining -= sent_step;
	}
	return 0;
}

int main()
{
	int len, ret = 0;
	char user_cmd;
	unsigned int session_id;
	char cmd[256];
	struct sockaddr_un remote;
	int iCnt = 2;
	char buf[256];
	Message_Struct lsendMessage;

	LOG_DEF("This is the client app for interacting with the Codec-Demo app\n");
	/* Create a UNIX domain stream socket */
	if ((client_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		LOG_DEF("ERROR: socket %d : STR[errno] %s\n", errno, strerror(errno));
		ret = -1;
		goto out;
	}

	LOG_DEF("Trying to connect to Codec Demo Server...\n");
	sprintf(cmd, "mkdir -p %s", CODEC_DEMO_SERVER_DIR);
	system(cmd);
	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, CODEC_DEMO_SERVER_SOCKET_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	while(connect(client_sock, (struct sockaddr *)&remote, len) == -1) {
		if(!iCnt){
			LOG_DEF("Server not found. IPC with Server failed...\n");
			close(client_sock);
			goto out;
		}
		poll(0,0,1000);
		LOG_DEF("Retrying to connect... iCnt(%d)\n", iCnt);
		iCnt--;
	}
	LOG_DEF("Connected to Codec Demo Server....\n");

	LOG_DEF("Codec Demo Server Test Begin....\n");
	while (1)
	{
		fflush(stdin);
		puts("##############################################################\n");
		puts("##############################################################\n");
		puts("          Codec Demo Server Test Menu ");
		puts(" Options from 0-3 is not supported for DEC and SW_DEC sessions\n");
		puts("##############################################################\n");
		puts("    s) Set statistics flag [Default to Enable]");
		puts("    0) Set Bitrate");
		puts("    1) Set Framerate");
		puts("    2) Force IDR Request");
		puts("    3) Set Resolution");
		puts("    q) Quit the codec demo client app");
		puts("##############################################################");
		puts("    Select option:");
		while (scanf(" %c",&user_cmd) == EOF);
		putchar(user_cmd);
		putchar('\n');

		if (user_cmd=='q') {
			LOG_DEF("Quitting the Codec Demo Client App !!!\n");
			break;
		}

		if (user_cmd != 's') {
			LOG_DEF("Enter the session ID:");
			scanf("%d",&session_id);
			LOG_DEF("  Session ID : %d\n", session_id);
		}

		switch(user_cmd)
		{
			case '0':
				{
					// Function to Set Bitrate
					unsigned int bitrate;
					LOG_DEF("Enter bitrate value in bps: ");
					scanf("%d", &bitrate);
					bzero(&lsendMessage,sizeof(Message_Struct));
					lsendMessage.m_messageType = SET_BITRATE_MSG;
					lsendMessage.session_id = session_id;
					lsendMessage.message.bitrate_msg.iBitrate = bitrate;
					ret = sendMessageToCodecDemo((unsigned char*)&lsendMessage);
					break;
				}

			case '1':
				{
					// Function to Set Framerate
					unsigned int fps_num, fps_den;
					LOG_DEF("Enter Framerate numerator value [fps = num/1 - denominator is 1] : ");
					scanf("%d", &fps_num);
					bzero(&lsendMessage,sizeof(Message_Struct));
					lsendMessage.m_messageType = SET_FRAMERATE_MSG;
					lsendMessage.session_id = session_id;
					lsendMessage.message.framerate_msg.iFpsNum = fps_num;
					ret = sendMessageToCodecDemo((unsigned char*)&lsendMessage);
					break;
				}

			case '2':
				{
					// Function to Force a IDR Request
					bzero(&lsendMessage,sizeof(Message_Struct));
					lsendMessage.m_messageType = SET_FORCE_IDR_MSG;
					lsendMessage.session_id = session_id;
					ret = sendMessageToCodecDemo((unsigned char*)&lsendMessage);
					break;
				}

			case '3':
				{
					// Function to Set Resolution
#if 0
					unsigned int width, height;
					LOG_DEF("Enter Width value: ");
					scanf("%d", &width);
					LOG_DEF("Enter Height value: ");
					scanf("%d", &height);
					bzero(&lsendMessage,sizeof(Message_Struct));
					lsendMessage.m_messageType = SET_RESOLUTION_MSG;
					lsendMessage.session_id = session_id;
					lsendMessage.message.resolution_msg.iWidth = width;
					lsendMessage.message.resolution_msg.iHeight = height;
					ret = sendMessageToCodecDemo((unsigned char*)&lsendMessage);
#else
					LOG_DEF("!! Setting Resolution currently not supported !!\n");
#endif
					break;
				}

			case 's':
				{
					bzero(&lsendMessage,sizeof(Message_Struct));
					lsendMessage.m_messageType = SET_STATISTICS_FLAG_MSG;
					ret = sendMessageToCodecDemo((unsigned char*)&lsendMessage);
					break;
				}

			default:
				break;
		}

	}
	close(client_sock);
out:
	return 0;
}
