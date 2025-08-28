// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <syna-settings-framework.h>

typedef struct {
    char rtspUrls[4][256];
} RtspSettingsData;

typedef struct {
    char formatPriorities[4][16];
} UsbFormatPrioritiesData;

class SettingsManager : public QObject {
    Q_OBJECT

public:
    explicit SettingsManager(QObject *parent = nullptr);

    Q_INVOKABLE void saveConfig(const QString &filePath);
    Q_INVOKABLE void loadConfig(const QString &filePath);

    Q_INVOKABLE void setHostIP(const QString &addr);
    Q_INVOKABLE QString getHostIP() const;
    Q_INVOKABLE bool setData(int id, const QVariantMap &data);
    Q_INVOKABLE QVariantMap getData(int id) const;
    Q_INVOKABLE QStringList formatPriorities() const;
    Q_INVOKABLE QStringList rtspUrls() const;

    Q_PROPERTY(QStringList rtspUrls READ rtspUrls NOTIFY rtspUrlsChanged)
    Q_PROPERTY(QStringList formatPriorities READ formatPriorities NOTIFY prioritiesChanged)
    Q_PROPERTY(QString hostIP READ getHostIP NOTIFY hostIPChanged)

    static void createConfig(const QString &filePath);
    static const QString CONFIG_FILE_PATH;
    bool checkData(int id);

signals:
    void prioritiesChanged();
    void rtspUrlsChanged();
    void hostIPChanged();

private:
    QString hostIP;
};

#endif // SETTINGSMANAGER_H
