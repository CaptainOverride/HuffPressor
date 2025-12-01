#include "worker.h"
#include "errors.h"

Worker::Worker(QObject *parent) : QObject(parent) {}

void Worker::processCompression(const QString& inputFile, const QString& outputFile) {
    try {
        Compressor compressor;
        HuffmanTree tree;

        // Connect callbacks to signals
        compressor.setLogger([this](const std::string& msg) {
            emit logMessage(QString::fromStdString(msg));
        });

        compressor.setProgressCallback([this](float p) {
            emit progressUpdated(p);
        });

        emit logMessage("Worker: Starting compression task...");
        emit logMessage("Worker: Output temp file: " + outputFile);

        if (compressor.readFileAndBuildFrequency(inputFile.toStdString()) != ErrorCode::Success) {
            emit operationFinished(false, "Failed to read input file.");
            return;
        }

        tree.build(compressor.getFrequencyMap());
        tree.generateCodes();

        ErrorCode result = compressor.compressFile(inputFile.toStdString(), 
                                                   outputFile.toStdString(), 
                                                   tree.getHuffmanCodes(), 
                                                   tree.getRoot());

        if (result == ErrorCode::Success) {
            emit operationFinished(true, "Compression successful! Ready to save.");
        } else {
            emit operationFinished(false, "Compression failed with error code: " + QString::number((int)result));
        }
    } catch (const std::exception& e) {
        emit operationFinished(false, QString("Critical Error: ") + e.what());
    } catch (...) {
        emit operationFinished(false, "Critical Error: Unknown exception occurred.");
    }
}

void Worker::processDecompression(const QString& inputFile, const QString& outputFile) {
    try {
        Decompressor decompressor;

        // Connect callbacks to signals
        decompressor.setLogger([this](const std::string& msg) {
            emit logMessage(QString::fromStdString(msg));
        });

        decompressor.setProgressCallback([this](float p) {
            emit progressUpdated(p);
        });

        emit logMessage("Worker: Starting decompression task...");
        emit logMessage("Worker: Output temp file: " + outputFile);

        ErrorCode result = decompressor.decompressFile(inputFile.toStdString(), outputFile.toStdString());

        if (result == ErrorCode::Success) {
            emit operationFinished(true, "Decompression successful! Ready to save.");
        } else {
            emit operationFinished(false, "Decompression failed with error code: " + QString::number((int)result));
        }
    } catch (const std::exception& e) {
        emit operationFinished(false, QString("Critical Error: ") + e.what());
    } catch (...) {
        emit operationFinished(false, "Critical Error: Unknown exception occurred.");
    }
}
