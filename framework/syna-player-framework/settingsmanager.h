#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QStringList>
#include <QFile>
#include <QTextStream>

class SettingsManager : public QObject {
    Q_OBJECT

public:
    explicit SettingsManager(QObject *parent = nullptr);

    Q_INVOKABLE QStringList getAvailableFormats();
    Q_INVOKABLE void reorderPriority(int fromIndex, int toIndex);
    Q_INVOKABLE void saveConfig(const QString &filePath);
    Q_INVOKABLE void loadConfig(const QString &filePath);
    Q_INVOKABLE void resetPriorities();

    Q_INVOKABLE void setRtspUrls(const QStringList &urls);
    Q_INVOKABLE QStringList getRtspUrls() const;
    Q_INVOKABLE QVariantMap validateRtspUrl(const QString &url);
    Q_INVOKABLE QVariantList validateAllRtspUrls();

    Q_INVOKABLE void setHostIP(const QString &addr);
    Q_INVOKABLE QString getHostIP() const;

    Q_PROPERTY(QStringList selectedFormats READ selectedFormats NOTIFY prioritiesChanged)
    Q_PROPERTY(QStringList rtspUrls READ getRtspUrls NOTIFY rtspUrlsChanged)
    Q_PROPERTY(QString hostIP READ getHostIP NOTIFY hostIPChanged)

    static QStringList loadFormatPriority(const QString &filePath);
    static QStringList loadRTSPUrls(const QString &filePath);
    static void createConfig(const QString &filePath);
    QStringList selectedFormats() const;
    static const QString CONFIG_FILE_PATH;
    static QStringList getDefaultFormatPriority();

signals:
    void prioritiesChanged();
    void rtspUrlsChanged();
    void hostIPChanged();

private:
    QStringList allFormats;
    QStringList selectedFormatsList;
    QStringList rtspUrls;
    QString hostIP;
};

#endif // SETTINGSMANAGER_H
