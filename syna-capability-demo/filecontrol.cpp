#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "filecontrol.h"

QStringList FileControl::readFromFile(const QString filePath)
{
    QStringList data;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            data.append(line);
        }
        file.close();
    } else {
        qWarning() << "Failed to open file for reading:" << filePath;
    }
    return data;
}

void FileControl::writeToFile(const QString filePath, const QStringList data)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString &line : data) {
            out << line << "\n";
        }
        file.close();
    } else {
        qWarning() << "Failed to open file for writing:" << filePath;
    }
}
