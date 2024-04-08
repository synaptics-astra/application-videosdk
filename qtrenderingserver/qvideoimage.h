#ifndef _QQUICKVIDEOIMAGE_H
#define _QQUICKVIDEOIMAGE_H

#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>
#include <QSGTextureProvider>
#include <QSGTexture>
#include <QQuickWindow>
#include "iostream"
#include "map"
#include "queue"
#include "pthread.h"
#ifdef DEMO_COMPILE
#include "demo_mock.h"
#else
#include <EGL/egl.h>
#include "EGL/eglext.h"
#endif
#include "commonBuffer.h"

using namespace std;

QT_BEGIN_NAMESPACE
class QVideoTexture;
class QVideoImageStorage;
//enum EVideoType{ Preview, Main };

class QQuickVideoImage : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(int videotype READ getWindowID WRITE setvideotype NOTIFY videotypeChanged)
	//Q_PROPERTY(int width READ width WRITE setwidth NOTIFY widthChanged)
	//Q_PROPERTY(qreal height READ height WRITE setheight NOTIFY heightChanged)
public:
	/*int width()const{
		return m_width;
	}
	qreal height()const{
		return m_height;
	}
	void setwidth(int width){
		m_width = width;
		QQuickItem::setWidth(width);
		emit widthChanged();
	}
	void setheight(qreal height){
		printf("The request for height is %f\n",height);
		m_height= height;
		QQuickItem::setHeight(height);
		printf("The height from QQuickItem is %d\n",QQuickItem::height());
		emit heightChanged();
	}*/
	void updateEGLDisplay();
	int getWindowID()const{
		//return (int)m_videotype;
		return (int)m_windowID;
	}
	void setvideotype(int windowID){
		m_windowID = (ARGBWindowID) windowID;
		emit videotypeChanged();
	}
	int getNumberBuffer(ARGBWindowID windowID);
	void renderNext();
	//void renderNext(int id);
	int renderNext(int windowID);
	void getDMABufIds(int* boName);

	QQuickVideoImage(QQuickItem *parent=0);
	~QQuickVideoImage(){}
	void processMessage(int boName);
	ARGBWindowID m_windowID;
	bool isImageReady(){return m_bIsImageReady;}
	bool isBufferAvailable();
Q_SIGNALS:
	void videotypeChanged();
	//void widthChanged();
	//void heightChanged();
private Q_SLOTS:
	void videoImageUpdated();
protected:

private:
	pthread_mutex_t m_mutex;
	static int m_number_dma_buf_id;
	int m_previousBoName;
	map<int,QVideoTexture*> m_textureMap;
	queue<QVideoTexture*>   m_pendingQueue;
	QVideoImageStorage*     m_pVideoImageStorage;
	void updatePreviousboName(int boName);
	bool m_bIsImageReady;
	int m_width;
	qreal m_height;
	//Q_DISABLE_COPY(QQuickVideoImage)
};

typedef void (QQuickVideoImage::*processMessageHandlerPtr)(int boName);

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickVideoImage)
//QML_DECLARE_TYPEINFO(QQuickVideoImage,QML_HAS_ATTACHED_PROPERTIES)

#endif // _QQUICKVIDEOIMAGE_H

