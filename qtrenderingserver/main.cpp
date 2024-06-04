/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/input.h>
#include "fcntl.h"
#include "pthread.h"
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QQmlEngine>
#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>
#include <QSGTextureProvider>
#include <QSGTexture>
#include <QQuickWindow>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QtGui/QOpenGLContext>
#include <qvideotexture.h>
#include <qmsinterface.h>
#include "qvideoimage.h"
#include "logs.h"

#ifdef DEMO_COMPILE
#include "demo_mock.h"
#endif

#define DEFAULT_DISPLAY_UNITS 1
#include "demo_videoitem.h"

void showBlur(int show);

pthread_t tid1;
void* keyPadEventHandler(void* arg)
{
	Q_UNUSED(arg);
	char c;
	while(1){

		printf("To Apply blur effect press b\n");
		scanf("%c",&c);
		printf("The new key is %c\n",c);
		if(c == 'b'){
			showBlur(1);
		}else if(c == 'n'){
			showBlur(0);
		}
	}
	return (void*)NULL;
}

int buffers_received = 0;
int fClientReady = 0;
int iNumDisplayAvail = 0;
int fVisible[ARGB_WINDOW_MAX] = {0};
PlatformType platform_type = PLATFORM_DOLPHIN;

QQuickView * mpView = NULL;
QQuickView * mpView_1 = NULL;
QQuickView * stat_view = NULL;
void showBlur(int show) {
	QObject*  root = mpView->rootObject();
	QQuickItem* item = root->findChild<QQuickItem *>("DemoVideoItem");
	QMetaObject::invokeMethod(item, "qmlFunc", Qt::AutoConnection, Q_ARG(QVariant, show));
}
void updateStatus(int inst,char *type,char *res,char *fps) {
	QObject*  root = stat_view->rootObject();
	QQuickItem* item = root->findChild<QQuickItem *>("statistics");
	QMetaObject::invokeMethod(item, "updateStatistics", Qt::AutoConnection,Q_ARG(QVariant,inst),Q_ARG(QVariant,type),Q_ARG(QVariant,res),Q_ARG(QVariant,fps));
}
void initStatus(int total_sessions)
{
	QObject*  root = stat_view->rootObject();
	QQuickItem* item = root->findChild<QQuickItem *>("statistics");
	QMetaObject::invokeMethod(item, "initStatistics", Qt::AutoConnection,Q_ARG(QVariant,total_sessions));
}
QString optionTypeEnumtoString(OptionType type)
{
	QString OptionType;
	switch(type)
		{
			case SWENC:
					OptionType ="SWENC";
					return OptionType;
			case SWDEC:
					OptionType ="SWDEC";
					return OptionType;
			case HWENC:
					OptionType ="HWENC";
					return OptionType;
			case HWDEC:
					OptionType ="HWDEC";
					return OptionType;
			default:
					OptionType ="";
					return OptionType;
		}
}

unsigned int counter = 0;
void updateCounter()
{
	QObject*  root = mpView->rootObject();
	QQuickItem* item = root->findChild<QQuickItem *>("DemoVideoItem");
	QMetaObject::invokeMethod(item, "updateCounter", Qt::AutoConnection, Q_ARG(QVariant, counter++));
}

int qtrendering_server_addView(QScreen *screen, int screenIdx)
{
	// screenIdx - 0 for primary display, 1 - secondary display
	if (screenIdx == 0)
	{
		mpView = new QQuickView();
		mpView->setScreen(screen);
		mpView->setColor(QColor(Qt::transparent));
		switch(platform_type) {
			case PLATFORM_DOLPHIN:
				{
#if (GRID_VAL == 55)
					mpView->setSource(QUrl(QStringLiteral("/usr/bin/dolphin_primary.qml")));
#else
					mpView->setSource(QUrl(QStringLiteral("/usr/bin/dolphin_primary_44.qml")));
#endif
				}
				break;

			case PLATFORM_PLATYPUS:
				{
#if (GRID_VAL == 55)
					mpView->setSource(QUrl(QStringLiteral("/usr/bin/platypus_primary.qml")));
#else
					mpView->setSource(QUrl(QStringLiteral("/usr/bin/platypus_primary_44.qml")));
#endif
				}
				break;

			case PLATFORM_MYNA2:
				{
					mpView->setSource(QUrl(QStringLiteral("/usr/bin/myna2_primary.qml")));
				}
				break;

			default:
				{
					LOG_ERR("Invalid mode selected falling back to dolphin platform\n");
#if (GRID_VAL == 55)
					mpView->setSource(QUrl(QStringLiteral("/usr/bin/DemoVideo.qml")));
#else
					mpView->setSource(QUrl(QStringLiteral("/usr/bin/DemoVideo_445x250.qml")));
#endif
					break;
				}
		}

	}
	else {
		mpView_1 = new QQuickView();
		mpView_1->setScreen(screen);
		if (platform_type == PLATFORM_DOLPHIN) {
			mpView_1->setSource(QUrl(QStringLiteral("/usr/bin/dolphin_secondary.qml")));
			stat_view = mpView_1; //stats display on secondary
		}
		else
		{
			// For MYNA2
			mpView_1->setSource(QUrl(QStringLiteral("/usr/bin/myna2_secondary.qml")));
			stat_view = mpView; //stats displayed on primary
		}
	}
	return 0;
}

int main(int argc, char** argv)
{
	int ret = 0;
	int qtrender_options = DEFAULT_DISPLAY_UNITS;

	{
		printf("Usage:\n");
		printf("qtrenderingserver x y &\n");
		printf("where 'x' denotes the number of displays for Qt rendering server app\n");
		printf("x is atleast '1' in case of single display\n");
		printf("x is '2' in case of dual display\n");
		printf("where 'y' denotes the platform in use\n");
		printf("0 - for Dolphin\n");
		printf("1 - for Platypus\n");
		printf("2 - for Myna2\n");
		printf("otherwise, defaults to 0\n");
	}

	QMSInterface::getInstance();
	pthread_create(&QMSInterface::ms_tid,NULL,&(QMSInterface::runThread),(void*)NULL);
	if(argc	== 1){
		LOG_DEF("No cmd line args passed. Running with keyPadEventHandler thread\n");
		pthread_create(&tid1,NULL,&keyPadEventHandler,(void*)NULL);
	}else{
		LOG_DEF("CMD line args passed. Running without keyPadEventHandler thread\n");
	}

	qtrender_options = atoi(argv[1]);
	if (qtrender_options != 1 && qtrender_options != 2)
	{
		printf("Unsupported input arguments:\n");
		printf("qtrenderingserver 1 <platformType> & (or)\n");
		printf("qtrenderingserver 2 <platformType> & (or)\n");
		return -1;
	}
	platform_type = (PlatformType)atoi(argv[2]);
	if ((platform_type < 0) || (platform_type > 2))
	{
		printf("Unsupported input arguments:\n");
		printf("qtrenderingserver 1 <platformType> & (or)\n");
		printf("qtrenderingserver 2 <platformType> & (or)\n");
		printf("platformType supported are 0 / 1 / 2:\n");
		return -1;
	}
	LOG_DEF("Waiting for Client Ready msg ....\n");
	while(!fClientReady){
		usleep(1000);
	}
	LOG_DEF("--> Buffers received for ARGB windows %d\n", buffers_received);
	LOG_DEF("QtRenderingServer: Starting RenderingService....\n");

	QGuiApplication app(argc, argv);
	QList<QScreen *> screens = app.screens();
	qmlRegisterType<VideoItem>( "DemoItem", 1, 0, "DemoVideoItem" );

	iNumDisplayAvail = qtrender_options;

	for (int i = 0; i < screens.count(); ++i) {
		if (i < qtrender_options)
			ret = qtrendering_server_addView(screens[i], i);
	}

	QVideoTexture::initilizeTexture();
	VideoItemUpdater::initialize();

	if (qtrender_options == DUAL_DISPLAY)
	{
		QObject::connect(mpView_1->engine(), &QQmlEngine::quit, qGuiApp, &QCoreApplication::quit);
		mpView_1->showFullScreen();
	}
	mpView->show();
	sleep(2);
	QMSInterface::getInstance()->sendAllInitDoneMsgToMs();
	return app.exec();
}
