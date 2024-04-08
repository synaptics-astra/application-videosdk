#ifndef __QMSINTERFACE_H__
#define __QMSINTERFACE_H__

#include "pthread.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <unistd.h>
#include "commonBuffer.h"

class QVideoImageStorage;

void sendInitMessageToMs(ARGBWindowID  windowID, int numBuffer,int height,int width,int pixelType, OptionType optionType,int* boInfo);
void sendProcessedMessageToMs(ARGBWindowID windowID,int boName);

class QMSInterface:public QObject
{
	public:
		static QMSInterface* getInstance();
		static void  destroyInstance();
		static void* runThread(void*);
		static pthread_t ms_tid;
		static void sendMessageProcessedDMABufIdToMs(int boName, ARGBWindowID windowID);
		int    getConnectedSocket(){return m_acceptedSocket;}
		void   sendInitMessageToMs(ARGBWindowID windowID,int numBuffer,int height,int width,InputPixelFormatUI pixelType, OptionType optionType, int* boInfo);
		void   sendProcessedMessageToMs(ARGBWindowID windowID, int boName);
		void   stopThread();
		int    HandleInitMessage(MessageStruct* pMessage);
		void   sendAllInitDoneMsgToMs(void);
		void   HandleStatusInitMessage(MessageStruct* pMessage);
		void   HandleStatusMessage(MessageStruct* pMessage);
	private:
		void*  threadRunnig(void*);
		void   initServerIPC();
		void   readNetworkMessage(MessageStruct &msg);
		void getARGBWindowSharedFDs(ARGBWindowID windowID);
		int    receivefromsocket(int sockfd, int *bo_handle, int *fd);
		QMSInterface();
		~QMSInterface();
		//Disable copy and assignment operator
		QMSInterface(const QMSInterface&);
		const QMSInterface & operator=(const QMSInterface &);
		void   sendMessageToMs(const MessageStruct &lsendMessage);
		static pthread_mutex_t ms_mutex;
		static int ms_var;
		static QMSInterface* ms_instance;
		//static int ms_maxSupportedVideoImage = 1;
		bool m_bIsThreadRunning;
		int m_listenningSocket;
		int m_acceptedSocket;
		//The COND/MUTEX to IPC to proceed further
		pthread_mutex_t m_syncMutex;
		pthread_cond_t  m_syncCond;
		int m_numberVideoImageRequestPending;
		//The internal instance
		QVideoImageStorage* m_qvideoImageStorageInstance;
};

#endif//__QMSINTERFACE_H__
