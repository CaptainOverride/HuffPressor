#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QString>
#include "compressor.h"
#include "decompressor.h"
#include "huffmanTree.h"

class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = nullptr);

public slots:
    void processCompression(const QString& inputFile, const QString& outputFile);
    void processDecompression(const QString& inputFile, const QString& outputFile);

signals:
    void progressUpdated(float percentage);
    void logMessage(const QString& message);
    void operationFinished(bool success, const QString& message);
};

#endif // WORKER_H
