/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <unistd.h>
#include "commonBuffer.h"
#include "qvideoimagestorage.h"
#include "pthread.h"
#include "qmsinterface.h"
#include "logs.h"

#ifdef DEMO_COMPILE
#include "demo_mock.h"
#else
#include <EGL/egl.h>
#include "EGL/eglext.h"
#endif


#define SOCK_PATH "/home/root/echo_socket"
#define SOCK_DIR "/home/root/"
extern int buffers_received;
extern int fClientReady;
extern int iNumDisplayAvail;
extern int fVisible[];
QString optionTypeEnumtoString(OptionType type);

void updateStatus(int inst,char *type,char *res,char *fps);
void initStatus(int total_sessions);
pthread_t QMSInterface::ms_tid = 0;
pthread_mutex_t QMSInterface::ms_mutex = PTHREAD_MUTEX_INITIALIZER;
int QMSInterface::ms_var = 0;
QMSInterface* QMSInterface::ms_instance = NULL;
/* Structure to hold the ARGB windows and associated shared FD's */
ARGBWindow argbWindows[ARGB_WINDOW_MAX];


void sendInitMessageToMs(ARGBWindowID windowID, int numBuffer,int height,int width,int pixelType, OptionType optionType,int* boInfo)
{
	QMSInterface::getInstance()->sendInitMessageToMs(windowID,numBuffer,height,width,(InputPixelFormatUI)pixelType,optionType,boInfo);
}

void sendProcessedMessageToMs(ARGBWindowID windowID,int boName)
{
	QMSInterface::getInstance()->sendProcessedMessageToMs(windowID,boName);
}

QMSInterface::QMSInterface():m_bIsThreadRunning(false),m_listenningSocket(-1),m_acceptedSocket(-1)
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	m_qvideoImageStorageInstance = QVideoImageStorage::getInstance();
	m_qvideoImageStorageInstance->setInitMessageToMsPtr(&(::sendInitMessageToMs));
	m_qvideoImageStorageInstance->setProcessedMessagedToMsPtr(&(::sendProcessedMessageToMs));
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}

QMSInterface::~QMSInterface()
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}

QMSInterface*  QMSInterface::getInstance()
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	pthread_mutex_lock(&ms_mutex);
	if(0 == ms_var){
		ms_instance = new QMSInterface();
		ms_var = 1;
	}
	pthread_mutex_unlock(&ms_mutex);
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
	return ms_instance;
}
void QMSInterface::destroyInstance()
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	pthread_mutex_lock(&ms_mutex);
	if(ms_tid != 0){
		pthread_cancel(ms_tid);
		pthread_join(ms_tid,(void**)NULL);
		ms_tid = 0;
	}

	QMSInterface::getInstance()->stopThread();

	if(1 == ms_var){
		delete ms_instance;
		ms_instance = NULL;
		ms_var = 0;
	}
	pthread_mutex_unlock(&ms_mutex);
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}
void* QMSInterface::runThread(void *arg)
{
	QMSInterface* instance = QMSInterface::getInstance();
	arg  = instance;
	return ((QMSInterface::getInstance())->threadRunnig(arg));
}

void QMSInterface::stopThread()
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	m_bIsThreadRunning = false;
	if(-1 != m_acceptedSocket)
		close(m_acceptedSocket);
	if(-1 != m_listenningSocket)
		close(m_listenningSocket);
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}

void QMSInterface::HandleStatusMessage(MessageStruct* pMessage)
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);

	int m_sessionID = pMessage->message.stMessage.m_sessionID;
	OptionType m_optionType = pMessage->message.stMessage.m_optionType;
	int m_width = pMessage->message.stMessage.m_width;
	int m_height = pMessage->message.stMessage.m_height;
	float m_fps = pMessage->message.stMessage.m_fps;
	QString res = QVariant(m_width).toString()+"x" +QVariant(m_height).toString();

	//LOG_DEF("The INIT: Here Window_%d Resolution[%dx%d] optionType : %d fps :%f\n",m_argbWindowID, m_width, m_height, m_optionType,m_fps);
	updateStatus(m_sessionID,optionTypeEnumtoString(m_optionType).toLocal8Bit().data(),
			res.toLocal8Bit().data(),QString::number(m_fps, 'f', 2).toLocal8Bit().data());
	return;
}

void QMSInterface::HandleStatusInitMessage(MessageStruct* pMessage)
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);

	int m_total_sessions = pMessage->message.stMessage.m_total_sessions;
	LOG_DEF("m_total_sessions %d\n", m_total_sessions);
	initStatus(m_total_sessions);
	return;
}

int QMSInterface::HandleInitMessage(MessageStruct* pMessage)
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	int windowID = pMessage->m_argbWindowID;
	LOG_DEF("INIT message has arrived from MS for Window %d\n", windowID);
	if( windowID >= ARGB_WINDOW_MAX){
		LOG_DEF("ERROR: max argb window supported(%d) but requested(%d)\n", ARGB_WINDOW_MAX, windowID);
		return -1;
	}
	ARGBWindow *window = &argbWindows[windowID];

	if(window->numBuffers){
		LOG_DEF("WARN: Requested ARGB Window_%d is already in use\n", windowID);
		return -1;
	}else{
		window->numBuffers =  pMessage->message.initMessage.m_numberbuffer;
		window->width = pMessage->message.initMessage.m_width;
		window->height = pMessage->message.initMessage.m_height;
		window->optionType = pMessage->message.initMessage.optionType;
		window->inPixelType = pMessage->message.initMessage.m_inpixeltype;
		for(int i=0;i<window->numBuffers;++i){
			window->remoteFDs[i] = pMessage->message.initMessage.m_sbufferInfo[i].m_boName;
		}
		fVisible[windowID] = 1;
		LOG_DEF("The INIT: Window_%d bufs[%d] Resolution[%dx%d] optionType[%d] pixeltype[%d]\n", windowID, window->numBuffers, window->width, window->height,window->optionType, window->inPixelType);
	}
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);

	return 0;
}

int QMSInterface::receivefromsocket(int sockfd, int *bo_handle, int *fd)
{
	int ret, buffd;
	unsigned int len = 0;
	char cmsg_b[CMSG_SPACE(sizeof(int))];
	struct cmsghdr *cmsg;
	struct msghdr msgh;
	struct iovec iov;
	fd_set recvFDs;
	char data[32];
	char* msgBuffer = NULL;
	MessageStruct ackMessage;
	bzero(&ackMessage,sizeof(MessageStruct));
	ackMessage.m_messageType = ACK_FROM_UI;

	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
retry:
	FD_ZERO(&recvFDs);
	FD_SET(0, &recvFDs);
	FD_SET(sockfd, &recvFDs);
	ret = select(sockfd+1, &recvFDs, NULL, NULL, NULL);
	if (ret < 0) {
		fprintf(stderr, "<%s>: Failed select: <%s>\n",
		__func__, strerror(errno));
		return -1;
	}
	else if (ret == 0)
	{
		usleep(1000);
		goto retry;
	}

	if (FD_ISSET(sockfd, &recvFDs)) {
		len = sizeof(buffd);
		memset(&msgh, 0, sizeof(msgh));
		msgh.msg_control = &cmsg_b;
		msgh.msg_controllen = CMSG_LEN(len);
		iov.iov_base = data;
		iov.iov_len = sizeof(data)-1;
		msgh.msg_iov = &iov;
		msgh.msg_iovlen = 1;
		cmsg = CMSG_FIRSTHDR(&msgh);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN(len);

		ret = recvmsg(sockfd, &msgh, MSG_DONTWAIT);
		if (ret < 0) {
			fprintf(stderr, "<%s>: Failed recvmsg: <%s>\n",
			__func__, strerror(errno));
			return -1;
		}
		sendMessageToMs(ackMessage);
		int handle;
		sscanf((char *)iov.iov_base, "%d\n", &handle);

		memcpy(&buffd, CMSG_DATA(cmsg), len);
		*fd = buffd;
		*bo_handle = handle;
	}
	else
	{
		usleep(1000);
		goto retry;
	}
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
	return 0;
}

void QMSInterface::getARGBWindowSharedFDs(ARGBWindowID windowID)
{
	int fd;
	int bo_handle;
	ARGBWindow *window = &argbWindows[windowID];
	LOG_FUNC(">> Fn(QMSInterface::%s)", __func__);
	for(int i=0;i<window->numBuffers;++i){
		receivefromsocket(m_acceptedSocket, &bo_handle, &fd);
		if (window->remoteFDs[i] == bo_handle)
		{
			LOG_SEQ("FD received [%d], bo_handle[%d]\n", fd, bo_handle);
			// TODO: To handle generically for multi plane data
			window->sharedFDs[i][0] = fd;
		}
		else
		{
			LOG_ERR("Sequence mismatch bo_handle expected[%d] vs received[%d]\n", window->remoteFDs[i], bo_handle);
		}
		// TODO: Handle this for number of planes
		//Receive & Store the second plane SharedFd
		receivefromsocket(m_acceptedSocket, &bo_handle, &fd);
		window->sharedFDs[i][1] = fd;
	}
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}

void* QMSInterface::threadRunnig(void* arg)
{
	m_bIsThreadRunning = true;
	QMSInterface* instance = (QMSInterface*)arg;
	MessageStruct ackMessage;
	bzero(&ackMessage,sizeof(MessageStruct));
	ackMessage.m_messageType = ACK_FROM_UI;
#ifndef DEMO_COMPILE
	pthread_setname_np(pthread_self(), "IPCThread");
    	//LOG_DEF("The thread is running for instance %x\n",(unsigned int)instance);
#endif
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	initServerIPC();
	//m_qvideoImageStorageInstance->triggerInitMessage();
	MessageStruct msg;
	while(true == m_bIsThreadRunning){
		readNetworkMessage(msg);
		switch(msg.m_messageType){
			case INIT_MESSAGE:
				LOG_DEF("INIT message received\n");
				HandleInitMessage(&msg);
				sendMessageToMs(ackMessage);
				getARGBWindowSharedFDs(msg.m_argbWindowID);
				buffers_received++;
				break;
			case MESSAGE_FROM_UI:
				LOG_DEF("Wrong message From UI to MS dropping \n");
				break;
			case MESSAGE_TO_UI:
				LOG_FUNC("New msg from MS has arrived for window %d and boName %d\n",msg.m_argbWindowID, msg.message.msMessage.m_boName1);
				m_qvideoImageStorageInstance->handleMessageFromMs(msg.m_argbWindowID,msg.message.msMessage.m_boName1);
				break;
			case ALL_INIT_MSG_DONE_TO_UI:
				LOG_DEF("DSPGI: In Server ALL_INIT_MSG_DONE_TO_UI\n");
				sendMessageToMs(ackMessage);
				fClientReady = 1;
				break;
			case STATUS_INIT_MSG:
				if (iNumDisplayAvail == DUAL_DISPLAY)
				{
					LOG_DEF("UIServer:Received Message To UI\n");
					HandleStatusInitMessage(&msg);
				}
				break;
			case STATUS_MSG_FROM_UI:
				LOG_DEF("UIServer: wrong message\n");
				break;
			case STATUS_MSG_TO_UI:
				if (iNumDisplayAvail == DUAL_DISPLAY)
				{
					LOG_DEF("UIServer:Received Message To UI\n");
					HandleStatusMessage(&msg);
				}
				break;
			case TERMINATE_TO_UI:
				LOG_DEF("Server received terminate commnd\n");
				exit(-1);
			default:
				LOG_DEF("Dropped unknown message type %d\n",msg.m_messageType);
				break;
		}
	}
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
	return (void*)NULL;
}
void  QMSInterface::initServerIPC()
{
        int  local_sock_len, remote_sock_len;
        struct sockaddr_un local, remote;
	char cmd[32] = {0};
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
        if (( m_listenningSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
                perror("socket");
                exit(1);
        }
	sprintf(cmd, "mkdir -p %s", SOCK_DIR);
	system(cmd);

        local.sun_family = AF_UNIX;
        bzero(local.sun_path, sizeof(local.sun_path));
        strcpy(local.sun_path, SOCK_PATH);
        local_sock_len = strlen(local.sun_path) + sizeof(local.sun_family);
        unlink(local.sun_path);
#ifndef DEMO_COMPILE
        if (bind(m_listenningSocket, (struct sockaddr *)&local, local_sock_len) == -1) {
                perror("bind");
                exit(1);
        }
#endif
        if (listen(m_listenningSocket, 5) == -1) {
                perror("listen");
                exit(1);
        }

        remote_sock_len = sizeof(remote);
        printf("Waiting for a connection...\n");
        if ((m_acceptedSocket = accept(m_listenningSocket, (struct sockaddr *)&remote,(socklen_t *) &remote_sock_len)) == -1) {
                perror("accept");
                exit(1);
        }
        LOG_DEF("Connected...\n");
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}
void QMSInterface::sendProcessedMessageToMs(ARGBWindowID windowID,int boName)
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	LOG_FUNC("sendMessageProcessedDMABufIdToMs is called with m_boName :%d\n",boName);
	MessageStruct lsendMessage;
	bzero(&lsendMessage,sizeof(MessageStruct));
	lsendMessage.m_messageType =  MESSAGE_FROM_UI;
	lsendMessage.m_argbWindowID   = windowID;
	lsendMessage.message.msMessage.m_boName1 = boName;
	sendMessageToMs(lsendMessage);
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}

void QMSInterface::sendAllInitDoneMsgToMs(void){
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	MessageStruct sndMsg;
	bzero(&sndMsg, sizeof(MessageStruct));
	sndMsg.m_messageType = ALL_INIT_MSG_DONE_FROM_UI;
	sendMessageToMs(sndMsg);
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}

void QMSInterface::sendInitMessageToMs(ARGBWindowID windowID,int numBuffer,int height,int width,InputPixelFormatUI pixelType,OptionType optionType,int* boInfo)
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	LOG_FUNC("%s : windowID %d numBuffer %d height %d width %d and pixelType %d \n",__FUNCTION__,windowID,numBuffer,height,width,pixelType);
	MessageStruct lsendMessage;
	bzero(&lsendMessage,sizeof(MessageStruct));
	lsendMessage.m_messageType  = INIT_MESSAGE;
	lsendMessage.m_argbWindowID = windowID;
	InitMessage* initMessage    = &lsendMessage.message.initMessage;
	initMessage->m_height       = height;
	initMessage->m_width        = width;
	initMessage->m_inpixeltype   = pixelType;
	initMessage->m_numberbuffer = numBuffer;
	initMessage->optionType = optionType;
	for(int i=0;i<numBuffer;++i){
		initMessage->m_sbufferInfo[i].m_boName = boInfo[i];
	}
	sendMessageToMs(lsendMessage);
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}

void QMSInterface::sendMessageToMs(const MessageStruct &lsendMessage)
{
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	static int socketId = QMSInterface::getInstance()->getConnectedSocket();
	int sent = 0, remaining = sizeof(MessageStruct),sent_step = 0;
	unsigned char* psendMessage = ( unsigned char*) &lsendMessage;
	while(sent!=sizeof(MessageStruct)){
		sent_step = send(socketId,psendMessage+sent,remaining,0);
		if(sent_step <0){
			LOG_FUNC("Error sedning message errno %d and strerror %s\n",errno,strerror(errno));
			exit(-1);
		}
		sent += sent_step;
		remaining -= sent_step;
		LOG_FUNC("The sent is %d\n",sent);
	}
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}
void QMSInterface::readNetworkMessage(MessageStruct &msg)
{
	bzero(&msg,sizeof(msg));
	int msg_len =  sizeof(struct MessageStruct);
	char* bufferPtr = (char*)&msg;
	int expected_len = msg_len;
	int len = 0 , recv_len = 0;
	LOG_FUNC(">> Fn(QMSInterface::%s)\n", __func__);
	while(expected_len !=0){
		len = recv (m_acceptedSocket, (bufferPtr+recv_len), (msg_len-recv_len),0);
		recv_len += len;
		expected_len -= len;
	}
	LOG_FUNC("<< Fn(QMSInterface::%s)\n", __func__);
}
