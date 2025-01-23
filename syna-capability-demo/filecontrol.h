#ifndef FILE_CONTROL_H
#define FILE_CONTROL_H

#include <QWidget>

class FileControl : public QWidget {
    Q_OBJECT

    public:
        FileControl() {}
        ~FileControl() {}
        Q_INVOKABLE QStringList readFromFile(const QString filePath);
        Q_INVOKABLE void writeToFile(const QString filePath, const QStringList data);

};

#endif /* PLAYBACK_CONTROL_H */
