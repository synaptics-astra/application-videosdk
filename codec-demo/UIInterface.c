// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Synaptics Incorporated

#include "UIInterface.h"
#include "config.h"
#include "decoder_interface_sw.h"
#include "encoder_interface_sw.h"
#ifdef V4L2_SUPPORT
#include "encoder_interface_hwv4l2.h"
#include "decoder_interface_hwv4l2.h"
#endif

#define MAX_WINDOWS 16

static int m_client_fd = -1;
static int fClientIPCInitDone;
static pthread_t listenThread;
static int fServerReady = 0;
static int nClientWindows = 0;
pthread_mutex_t lock_UIinterface;
tsem_t sem_UIAcknowledgement;
int enQueueV4L2Buffer(uint32_t windowID, uint32_t bo_name);


static struct _windowData{
	inst_type_e instType;
	void *pCfg;
}omxHandle[MAX_WINDOWS];

int registerClientWindow(void) {
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	nClientWindows++;
	LOG_PARAM("CLIENT: Number of Client Windows updated to(%d)\n", nClientWindows);
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
}

int  InitClientIPC()
{
	int len, ret = 0;
	char cmd[256];
	struct sockaddr_un remote;
	int iCnt = 2;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	pthread_mutex_init(&(lock_UIinterface), NULL);
	sem_init(&sem_UIAcknowledgement, 0, 0);
	if(!fClientIPCInitDone){

		if ((m_client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			ret = -1;
			goto out;

		}

		LOG_DEF("Trying to connect...\n");
		sprintf(cmd, "mkdir -p %s", SOCK_DIR);
		ret = system(cmd);
		remote.sun_family = AF_UNIX;
		strcpy(remote.sun_path, SOCK_PATH);
		len = strlen(remote.sun_path) + sizeof(remote.sun_family);
		while(connect(m_client_fd, (struct sockaddr *)&remote, len) == -1) {
			if(!iCnt){
				LOG_DEF("Server not found. IPC with Server failed...\n");
				close(m_client_fd);
				goto out;
			}
			poll(0,0,1000);
			LOG_DEF("Retrying to connect... iCnt(%d)\n", iCnt);
			iCnt--;
		}
		LOG_DEF("Connected to Server....\n");

		// Create Thread for listening msg from Rendering Server.
		ret = pthread_create(&listenThread, NULL, listening_thread, (void*)NULL);
		if (ret) {
			LOG_ERR("Thread creating (listening_thread) Error\n");
			ret = -1;
			goto out;
		}
		fClientIPCInitDone = 1;
	}
out:
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return ret;
}

int sendMessageToUI(unsigned char* psendMessage)
{
	int sent = 0, remaining = sizeof(MessageStruct),sent_step = 0;
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	while(sent!=sizeof(MessageStruct)){
		sent_step = send(m_client_fd,psendMessage+sent,remaining,0);
		if(sent_step < 0)
			LOG_ERR("Send return is %d\n", sent_step);
		else
		{
			sent += sent_step;
			remaining -= sent_step;
		}
	}
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return 0;
}

int sendAllInitDoneMsgToUI(void){
	int ret = -1;
	LOG_DEF("codec-demo: File(%s): >> Fn(%s)\n", __FILE__, __func__);
	if(fClientIPCInitDone){
		MessageStruct sndMsg;
		bzero(&sndMsg, sizeof(MessageStruct));
		sndMsg.m_messageType = ALL_INIT_MSG_DONE_TO_UI;
		ret = sendMessageToUI((unsigned char *)&sndMsg);
	}else{
		LOG_ERR("Error: IPC Init not done\n");
	}
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return ret;
}


int sendTerminateMsgToUI(void){

	int ret = -1;
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	if(fClientIPCInitDone){
		MessageStruct sndMsg;
		bzero(&sndMsg, sizeof(MessageStruct));
		sndMsg.m_messageType = TERMINATE_TO_UI;
		ret = sendMessageToUI((unsigned char *)&sndMsg);
	}else{
		LOG_ERR("Error: IPC Init not done\n");
	}
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return ret;
}

void enQueueBuffer(int windowID, int sharedFd)
{
	enQueueV4L2Buffer(windowID, sharedFd);
}

int sendProcessDMABufId(int windowID, int sharedFd1, int sharedFd2)
{
	int window_id = windowID - 1;
	int ret = -1;
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	LOG_FUNC("Fn(%s): win_%d shFD_%d\n", __func__, windowID, sharedFd1);
	if(fClientIPCInitDone && fServerReady){
		MessageStruct lsendMessage;
		bzero(&lsendMessage,sizeof(MessageStruct));
		lsendMessage.m_windowID = window_id;
		lsendMessage.m_messageType = MESSAGE_TO_UI;
		lsendMessage.message.msMessage.m_boName1 = sharedFd1;
		lsendMessage.message.msMessage.m_boName2 = sharedFd2;
		ret = sendMessageToUI((unsigned char*)&lsendMessage);
	}else{
		LOG_FUNC("Error: IPC Init not done enQueue this Buffer\n");
		enQueueV4L2Buffer(window_id, sharedFd1);
	}
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return ret;
}

int ReadMessageNetwork(MessageStruct *pmsg)
{
	bzero(pmsg,sizeof(MessageStruct));
	int msg_len =  sizeof(struct MessageStruct);
	char* bufferPtr = (char*)pmsg;
	int expected_len = msg_len;
	int len = 0;
	int recv_len = 0;
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	while(expected_len !=0){
		len = recv (m_client_fd,bufferPtr+len,(msg_len-recv_len),0);
		if (len == 0)
		{
			LOG_ERR("Remote end closed unexpetedly..!! Exiting from thread..! \n");
			return -1;
		}
		LOG_FUNC("The message has come expected len %d and incoming mes lenghth %d\n",expected_len,len);
		recv_len += len;
		expected_len -= len;
	}
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	return 0;
}

void HandleInitMessage(MessageStruct* pMessage)
{
	LOG_FUNC("The INIT message has arrived from UI\n");
/*
	DMAInfoStruct bdispOutDMAInfo;
	int i = 0;
	bzero(&bdispOutDMAInfo,sizeof(DMAInfoStruct));
	bdispOutDMAInfo.number_buffer = pMessage->message.initMessage.m_numberbuffer;
	bdispOutDMAInfo.width = pMessage->message.initMessage.m_width;
	bdispOutDMAInfo.height = pMessage->message.initMessage.m_height;
	if( bdispOutDMAInfo.number_buffer > MAX_BUFFER_SUPPORT){
		LOG_DEF("Cant process no many buffer %d\n",bdispOutDMAInfo.number_buffer);
		LOG_DEF("Only supported buffers no is %d\n",MAX_BUFFER_SUPPORT);
		LOG_DEF("Exiting \n");
		exit(-1);
	}
	for(i=0;i< bdispOutDMAInfo.number_buffer;++i){
		bdispOutDMAInfo.bufferInfo[i].m_bo_name = pMessage->message.initMessage.m_sbufferInfo[i].m_boName;
		GetDMABufFromBoName(bdispOutDMAInfo.bufferInfo[i].m_bo_name,&bdispOutDMAInfo.bufferInfo[i].m_dma_buf_id);
	}
	configBdispOutput(&bdispOutDMAInfo);
*/
}

int sendtosocket(int sockfd, int fd, int length)
{
	int ret, buffd;
	unsigned int len;
	char cmsg_b[CMSG_SPACE(sizeof(int))];
	struct cmsghdr *cmsg;
	struct msghdr msgh;
	struct iovec iov;
	struct timeval timeout;
	fd_set selFDs;
	FD_ZERO(&selFDs);
	FD_SET(0, &selFDs);
	FD_SET(sockfd, &selFDs);
	timeout.tv_sec = 20;
	timeout.tv_usec = 0;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	ret = select(sockfd+1, NULL, &selFDs, NULL, &timeout);
	if (ret < 0) {
		LOG_ERR("<%s>: Failed select: <%s>\n", __func__, strerror(errno));
		return -1;
	}

	if (FD_ISSET(sockfd, &selFDs)) {
		buffd = fd;
		len = length;
		memset(&msgh, 0, sizeof(msgh));
		msgh.msg_control = &cmsg_b;
		msgh.msg_controllen = CMSG_LEN(len);
		char id[4];
		snprintf(id, 4, "%d\n", fd);
		LOG_SEQ("Sending data %s \n", id);
		iov.iov_base = id;
		iov.iov_len = 4;
		msgh.msg_iov = &iov;
		msgh.msg_iovlen = 1;
		cmsg = CMSG_FIRSTHDR(&msgh);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(len);
		memcpy(CMSG_DATA(cmsg), &buffd, len);

		ret = sendmsg(sockfd, &msgh, MSG_DONTWAIT);
		if (ret < 0) {
			LOG_ERR("<%s>: Failed sendmsg: <%s>\n", __func__, strerror(errno));
			return -1;
		}
		sem_wait(&sem_UIAcknowledgement);
	}
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);

	return 0;
}

void SendInitMessageToUI(	int instType,
				void *pCfg,
				int windowID,
				int numBuffer,
				int width,
				int height,
				int pixelType)
{
	int i;
	pthread_mutex_lock(&(lock_UIinterface));
	LOG_FUNC("File(%s): >> Fn(%s)", __FILE__, __func__);

	if(fClientIPCInitDone){
		LOG_FUNC("Fn(%s): instType(%d) winID %d nBuf %d height %d width %d pixelType %d\n",__func__,instType,windowID,numBuffer,height,width,pixelType);
		if(windowID > MAX_WINDOWS){
			LOG_ERR("Error: windowID(%d) > MAX_WINDOWS(%d)\n", windowID, MAX_WINDOWS);

			pthread_mutex_unlock(&(lock_UIinterface));
			return;
		}

		MessageStruct lsendMessage;
		bzero(&lsendMessage,sizeof(MessageStruct));
		lsendMessage.m_messageType  = INIT_MESSAGE;
		lsendMessage.m_windowID	    = windowID;
		InitMessage* initMessage    = &lsendMessage.message.initMessage;
		initMessage->m_height       = height;
		initMessage->m_width        = width;
		initMessage->m_inpixeltype   = pixelType;
		initMessage->m_numberbuffer = numBuffer;

		switch(instType){
			case SESS_SWDEC:
				{
					LOG_DEF("DSPG: SESS_SWDEC");
					dec_config_t *pCfgLocal = (dec_config_t *)pCfg;
					priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfgLocal->priv;
					initMessage->optionType = SWDEC;
					for(int i=0;i<numBuffer;++i){
						initMessage->m_sbufferInfo[i].m_boName = pDec->pH264DecOutSWBuffer[i]->mShareFd;
					}
				}
				break;

			case SESS_SWENC:
				{
					LOG_DEF("DSPG: SESS_SWENC");
					enc_config_t *pCfgLocal = (enc_config_t *)pCfg;
					priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfgLocal->priv;
					initMessage->optionType = SWENC;
					for(int i=0;i<numBuffer;++i){
						initMessage->m_sbufferInfo[i].m_boName = pEnc->pH264EncInSWBuffer[i]->mShareFd;
					}
				}
				break;

#ifdef V4L2_SUPPORT
			case SESS_HWDEC:
				{
					dec_config_t *pCfgLocal = (dec_config_t *)pCfg;
					priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfgLocal->priv;
					initMessage->optionType = HWDEC;
					for(i=0;i<numBuffer;++i){
						initMessage->m_sbufferInfo[i].m_boName = pDec->mmap_virtual_output_sharedFDs[i][0];
					}
				}
				break;

			case SESS_HWENC:
				{
					enc_config_t *pCfgLocal = (enc_config_t *)pCfg;
					priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfgLocal->priv;
					initMessage->optionType = HWENC;
					for(int i=0; i<numBuffer;++i){
						initMessage->m_sbufferInfo[i].m_boName = pEnc->mmap_virtual_input_sharedFDs[i][0];
					}
				}
				break;
#endif

			default:
				LOG_ERR("Error: Invalid instType(%d)\n", instType);
				pthread_mutex_unlock(&(lock_UIinterface));
				return;
		}

		sendMessageToUI((unsigned char*)&lsendMessage);
		sem_wait(&sem_UIAcknowledgement);
		for(i = 0; i < numBuffer; i++){
			switch(instType){
				case SESS_SWDEC:
				{
					dec_config_t *pCfgLocal = (dec_config_t *)pCfg;
					priv_sw_decoder_t *pDec = (priv_sw_decoder_t *)pCfgLocal->priv;
					sendtosocket(m_client_fd, pDec->pH264DecOutSWBuffer[i]->mShareFd, sizeof(int));
					// TODO: call the below generically for multi planes
					sendtosocket(m_client_fd, 0, sizeof(int));
				}
				break;

				case SESS_SWENC:
				{
					enc_config_t *pCfgLocal = (enc_config_t *)pCfg;
					priv_sw_encoder_t *pEnc = (priv_sw_encoder_t *)pCfgLocal->priv;
					sendtosocket(m_client_fd, pEnc->pH264EncInSWBuffer[i]->mShareFd, sizeof(int));
					// TODO: call the below generically for multi planes
					sendtosocket(m_client_fd, 0, sizeof(int));
				}
				break;

#ifdef V4L2_SUPPORT
				case SESS_HWDEC:
				{
					dec_config_t *pCfgLocal = (dec_config_t *)pCfg;
					priv_hw_v4l2_decoder_t *pDec = (priv_hw_v4l2_decoder_t *)pCfgLocal->priv;
					sendtosocket(m_client_fd, pDec->mmap_virtual_output_sharedFDs[i][0], sizeof(int));
					// TODO: call the below generically for multi planes
					sendtosocket(m_client_fd, pDec->mmap_virtual_output_sharedFDs[i][1], sizeof(int));
				}
				break;

				case SESS_HWENC:
				{
					enc_config_t *pCfgLocal = pCfg;
					priv_hw_v4l2_encoder_t *pEnc = (priv_hw_v4l2_encoder_t *)pCfgLocal->priv;
					sendtosocket(m_client_fd, pEnc->mmap_virtual_input_sharedFDs[i][0], sizeof(int));
					// TODO: call the below generically for multi planes
					sendtosocket(m_client_fd, pEnc->mmap_virtual_input_sharedFDs[i][1], sizeof(int));
				}
#endif
				break;
				default:
					break;
			}
		}

		omxHandle[windowID].instType = instType;
		if(instType == SESS_SWDEC){
			omxHandle[windowID].pCfg = (dec_config_t*)pCfg;
			LOG_FUNC("windowID(%d) belongs to DEC\n", windowID);
		}
		else if(instType == SESS_SWENC){
			omxHandle[windowID].pCfg = (enc_config_t*)pCfg;
			LOG_FUNC("windowID(%d) belongs to ENC\n", windowID);
		}
		else if((instType == SESS_HWENC) || (instType == SESS_HWDEC)){
			omxHandle[windowID].pCfg = (enc_config_t*)pCfg;
			LOG_FUNC("windowID(%d) belongs to ENC\n", windowID);
		}

		nClientWindows--;
		if(!nClientWindows){
			sendAllInitDoneMsgToUI();
			sem_wait(&sem_UIAcknowledgement);
		}
	}else{
		LOG_ERR("WARN: IPC Init not done. Display ID 0 can be used to dump the raw data.\n");
	}
	pthread_mutex_unlock(&(lock_UIinterface));
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
}

/**/
int enQueueV4L2Buffer(uint32_t windowID, uint32_t bo_name) {
	int  eError = 0;

	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	LOG_FUNC("windowID(%d) && shared_FD(%d)\n", windowID, bo_name);

	if(windowID > MAX_WINDOWS){
		LOG_ERR("Error: windowID(%u) > MAX_WINDOWS(%d)\n", windowID, MAX_WINDOWS);
		goto out;
	}

	switch(omxHandle[windowID].instType)
	{
		default:
		{
			LOG_ERR("Fn(%s): Invalid instType(%d)\n", __func__, omxHandle[windowID].instType);
		}
		break;
#ifdef V4L2_SUPPORT
		case SESS_HWENC:
		{
			enqueBufFromUIHWEnc(omxHandle[windowID].pCfg, bo_name);
		}
		break;

		case SESS_HWDEC:
		{
			enqueBufFromUIHWDec(omxHandle[windowID].pCfg, bo_name);
		}
		break;
#endif
		case SESS_SWDEC:
		{
			enqueBufFromUISWDec(omxHandle[windowID].pCfg, bo_name);
		}
		break;

		case SESS_SWENC:
		{
			enqueBufFromUISWEnc(omxHandle[windowID].pCfg, bo_name);
		}
		break;
	}

out:
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	return eError;
}

void SendStatusMessageToUI(StatusMsg *stMessage)
{
	int ret = -1;
//	LOG_DEF("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	if(fClientIPCInitDone){

		MessageStruct lsendMessage;
		bzero(&lsendMessage,sizeof(MessageStruct));
		lsendMessage.m_messageType = STATUS_MSG_TO_UI;
		lsendMessage.message.stMessage.m_sessionID = stMessage->m_sessionID;
		lsendMessage.message.stMessage.m_optionType = stMessage->m_optionType;
		lsendMessage.message.stMessage.m_width = stMessage->m_width;
		lsendMessage.message.stMessage.m_height = stMessage->m_height;
		lsendMessage.message.stMessage.m_fps = stMessage->m_fps;
		ret = sendMessageToUI((unsigned char*)&lsendMessage);
		LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	}else{
		LOG_ERR("WARN: IPC Init not done. Display ID 0 can be used to dump the raw data.\n");
	}
}

void SendStatusInitMessageToUI(StatusMsg *stMessage)
{
	int ret = -1;
//	LOG_DEF("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	if(fClientIPCInitDone){

		MessageStruct lsendMessage;
		bzero(&lsendMessage,sizeof(MessageStruct));
		lsendMessage.m_messageType = STATUS_INIT_MSG;
		lsendMessage.message.stMessage.m_total_sessions = stMessage->m_total_sessions;
		ret = sendMessageToUI((unsigned char*)&lsendMessage);
		LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
	}else{
		LOG_ERR("WARN: IPC Init not done. Display ID 0 can be used to dump the raw data.\n");
	}
}

void* listening_thread(void* arg)
{

	pthread_setname_np(pthread_self(), "IPCInput");
	char *device = NULL;

	MessageStruct msg;
	LOG_FUNC("File(%s): >> Fn(%s)\n", __FILE__, __func__);
	while(1){
		if (ReadMessageNetwork(&msg) < 0)
		{
			LOG_ERR("ReadMessageNetwork has an error.. Exiting from %s\n", __func__);
			pthread_exit(0);
		}

		LOG_FUNC("The message has come with m_messageType %d\n",msg.m_messageType);
		switch(msg.m_messageType){
			case INIT_MESSAGE:
				HandleInitMessage(&msg);
				break;
			case MESSAGE_FROM_UI:
				LOG_FUNC("Client: UIServer has sent winID(%d) sharedFD(%d) to free.\n",msg.m_windowID, msg.message.uIMessage.m_boName);
				enQueueBuffer(msg.m_windowID, msg.message.uIMessage.m_boName);
				break;
			case MESSAGE_TO_UI:
				LOG_ERR("Wrong message dropping... \n");
				break;
			case ALL_INIT_MSG_DONE_FROM_UI:
				LOG_DEF("Fn(%s) received ALL_INIT_MSG_DONE_FROM_UI\n", __func__);
				fServerReady = 1;
				break;
			case STATUS_MSG_FROM_UI:
				LOG_DEF("Client: UIServer has sent this message\n");
				break;
			case STATUS_MSG_TO_UI:
				LOG_DEF("Client: Wrong message dropping...\n");
				break;
			case ACK_FROM_UI:
				LOG_SEQ("Client: ACKOWLEDGEMENT Received...\n");
				sem_post(&sem_UIAcknowledgement);
				break;
			default:
				LOG_ERR("Dropped unknown message type %d\n",msg.m_messageType);
				break;
		}
	}
	LOG_FUNC("File(%s): << Fn(%s)\n", __FILE__, __func__);
}


void DeInitClientIPC(void) {

	//send Terminate msg to QT rendering server
	sendTerminateMsgToUI();

	if(fClientIPCInitDone){
		pthread_join(listenThread, NULL);
		fClientIPCInitDone = 0;
	}
	pthread_mutex_destroy(&(lock_UIinterface));
	sem_destroy(&sem_UIAcknowledgement);
}
