#ifndef FRAMECONTROLLER_H
#define FRAMECONTROLLER_H

#include <QObject>
#include <QVariantList>
#include <QStringList>
#include <QMutex>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class FrameProvider;

class FrameController : public QObject
{
    Q_OBJECT
public:
    explicit FrameController(FrameProvider *provider,
                             QObject *parent = nullptr);
    ~FrameController();

    Q_INVOKABLE void startWithDefaultFiles();
    Q_INVOKABLE void addUserFiles(const QVariantList &fileUrls);
    Q_INVOKABLE void removeUserFiles(const QVariantList &fileUrls);
    Q_INVOKABLE void playUserFiles();
    Q_INVOKABLE void playSingleFile(const QString &filePath);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void nextFrame();

signals:
    void imageChanged();

private:
    void init();
    void createPipeline();
    void extractHalfFrame(const uint8_t *originalFrame,
                          int originalWidth,
                          int originalHeight,
                          uint8_t *halfFrame,
                          bool left);
    void updateImage(uint8_t *data, int len);

    static GstFlowReturn newBuffer(GstAppSink *appsink,
                                  gpointer user_data);

private:
    FrameProvider *provider;

    QString modelPath;
    QString gstPipeline;
    QString gstPipelineTemplate;
    QStringList files;
    int currentIndex = 0;

    uint8_t *leftData = nullptr;
    uint8_t *rightData = nullptr;

    GstElement *pipeline = nullptr;
    GstAppSink *appsink_1 = nullptr;
    GstAppSink *appsink_2 = nullptr;

    QMutex mutex_;
    int callbackCount = 0;
    bool normalFrameProcessed = false;
    bool srFrameProcessed = false;
};

#endif // FRAMECONTROLLER_H

