// SPDX-License-Identifier: Apache-2.0
// Copyright 2025 Synaptics Incorporated

#include "embedding.h"

Embedding::Embedding()
{
    loadDatabase();
}

Embedding::~Embedding()
{
    saveDatabase();
}

void Embedding::saveDatabase()
{
    cv::FileStorage fs("/home/root/demos/configs/face_id_data.yml", cv::FileStorage::WRITE);
    fs << "pairs" << "[";
    for (const auto& pair : registeredFaces) {
        fs << "{";
        fs << "names" << "[";
        for (const auto& name : pair.first) {
            fs << name.toStdString();
        }
        fs << "]";
        fs << "face" << pair.second;
        fs << "}";
    }
    fs << "]";
    fs.release();
}

void Embedding::loadDatabase()
{
    cv::FileStorage fs("/home/root/demos/configs/face_id_data.yml", cv::FileStorage::READ);
    if (fs.isOpened()) {
        cv::FileNode pairs = fs["pairs"];
        for (cv::FileNodeIterator it = pairs.begin(); it != pairs.end(); ++it) {
            cv::FileNode pair = *it;
            QStringList names;
            cv::FileNode namesNode = pair["names"];
            for (cv::FileNodeIterator nameIt = namesNode.begin(); nameIt != namesNode.end(); ++nameIt) {
                names << QString::fromStdString((std::string)*nameIt);
            }
            cv::Mat face;
            pair["face"] >> face;
            registeredFaces.append(qMakePair(names, face));
        }
        fs.release();
    }
}

cv::Mat Embedding::getNormVector(const cv::Mat& matrix)
{
    cv::Mat floatMatrix;
    matrix.convertTo(floatMatrix, CV_32F);

    cv::Mat normalizedMatrix = floatMatrix / cv::norm(floatMatrix);
    return normalizedMatrix.clone();
}

double Embedding::computeDistance(const cv::Mat& matrix1, const cv::Mat& matrix2, bool isNormalized)
{
    cv::Mat difference;
    if (!isNormalized) {
        difference = getNormVector(matrix1) - getNormVector(matrix2);
    } else {
        difference = matrix1 - matrix2;
    }

    cv::Mat squaredDifference;
    cv::pow(difference, 2, squaredDifference);
    return cv::sum(squaredDifference)[0];
}

QString Embedding::getMatchedName(cv::Mat& matrix)
{
    QVector<QPair<uint32_t, double>> indexVector;
    QVector<NameInfo> matchedNamesInfo;
    recognizedNames.clear();
    recognizedIndices.clear();

    for (const auto& embeddingPair : registeredFaces) {
        QStringList dbPairName = embeddingPair.first;
        cv::Mat dbEmbeddingsMat = embeddingPair.second;

        for (uint32_t idx = 0; idx < dbEmbeddingsMat.rows; idx++) {
            cv::Mat currentRow = dbEmbeddingsMat.row(idx);
            double distance = computeDistance(matrix, currentRow, true);

            if (distance < distanceThreshold) {
                indexVector.append(qMakePair(idx, distance));
            }
        }

        for (const auto& indexPair : indexVector) {
            uint32_t nameIndex = indexPair.first;
            bool isFound = false;

            for (auto& matchedInfo : matchedNamesInfo) {
                if (matchedInfo.name == dbPairName[nameIndex]) {
                    matchedInfo.recognizeTimes++;
                    isFound = true;
                    break;
                }
            }

            if (!isFound) {
                NameInfo nameInfo = {dbPairName[nameIndex], nameIndex, indexPair.second, 1};
                matchedNamesInfo.append(nameInfo);
            }
        }

        if (!matchedNamesInfo.isEmpty()) {
            std::sort(matchedNamesInfo.begin(), matchedNamesInfo.end(),
                    [](const NameInfo& n1, const NameInfo& n2) {
                    if (n1.recognizeTimes == n2.recognizeTimes) {
                        return n1.distance < n2.distance;
                    }
                        return n1.recognizeTimes > n2.recognizeTimes;
                    });

            recognizedNames.append(matchedNamesInfo.first().name);
            recognizedIndices.append(matchedNamesInfo.first().index);
            break;
        }

        indexVector.clear();
    }

    if (matchedNamesInfo.isEmpty()) {
        unrecognizedFaces.append(matrix);
        recognizedNames.append("Unknown");
        recognizedIndices.append(0);
    }

    if (!recognizedNames.isEmpty()) {
        return recognizedNames.at(0);
    } else {
        return "Unknown face!!!";
    }
}

bool Embedding::registerFace(const QString& name, const cv::Mat& faceEmbedding)
{
    int matchCount = 0;
    for (const auto& unknownFace : unrecognizedFaces) {
        double distance = computeDistance(faceEmbedding, unknownFace, true);
        if (distance < distanceThreshold) {
            matchCount++;
        }
    }

    if (matchCount >= 8) {
        QStringList faceName;
        if (!name.isEmpty()) {
            faceName.append(name);
            registeredFaces.append(qMakePair(faceName, faceEmbedding));

            unrecognizedFaces.erase(std::remove_if(unrecognizedFaces.begin(), unrecognizedFaces.end(),
                        [this, &faceEmbedding](const cv::Mat& face) {
                            return computeDistance(faceEmbedding, face, true) < this->distanceThreshold;
                        }), unrecognizedFaces.end());
        }
    }

    return true;
}
