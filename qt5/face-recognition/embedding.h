// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#ifndef EMBEDDING_H
#define EMBEDDING_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include <QtMath>
#include <QVector>
#include <QString>
#include <QMap>
#include <QtAlgorithms>

using namespace std;

struct NameInfo {
    QString name;
    uint32_t index;
    double distance;
    uint32_t recognizeTimes;
};

class Embedding {
    public:
        Embedding();
        ~Embedding();
        double computeDistance(const cv::Mat& matrix1, const cv::Mat& matrix2, bool isNormalized);
        cv::Mat getNormVector(const cv::Mat& matrix);
        QString getMatchedName(cv::Mat& matrix);
        bool registerFace(const std::vector<QString>& nameCollection, const std::vector<cv::Mat>& faceEmbeddingCollection);

    private:
        void saveDatabase();
        void loadDatabase();

        QVector<QPair<QStringList, cv::Mat>> registeredFaces;
        QVector<cv::Mat> unrecognizedFaces;
        QStringList recognizedNames;
        QVector<uint32_t> recognizedIndices;
        double distanceThreshold = 0.025;
};
#endif // EMBEDDING_H
