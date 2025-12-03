#include "worker.h"
#include "errors.h"
#include "archiver.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

Worker::Worker(QObject *parent) : QObject(parent) {}

void Worker::processCompression(const QString& inputFile, const QString& outputFile) {
    try {
        std::string inputPath = inputFile.toStdString();
        std::string finalInputPath = inputPath;
        std::string tempArchivePath;
        bool isDirectory = fs::is_directory(inputPath);

        if (isDirectory) {
            emit logMessage("Worker: Input is a directory. Archiving...");
            tempArchivePath = inputPath + ".arch_temp";
            ErrorCode archResult = Archiver::archiveDirectory(inputPath, tempArchivePath);
            if (archResult != ErrorCode::Success) {
                emit operationFinished(false, "Failed to archive directory.");
                return;
            }
            finalInputPath = tempArchivePath;
            emit logMessage("Worker: Archive created.");
        }

        Compressor compressor;
        HuffmanTree tree;

        compressor.setLogger([this](const std::string& msg) {
            emit logMessage(QString::fromStdString(msg));
        });

        compressor.setProgressCallback([this](float p) {
            emit progressUpdated(p);
        });

        emit logMessage("Worker: Starting compression task...");
        
        if (compressor.readFileAndBuildFrequency(finalInputPath) != ErrorCode::Success) {
            if (isDirectory) fs::remove(tempArchivePath);
            emit operationFinished(false, "Failed to read input file.");
            return;
        }

        tree.build(compressor.getFrequencyMap());
        tree.generateCodes();

        ErrorCode result = compressor.compressFile(finalInputPath, 
                                                   outputFile.toStdString(), 
                                                   tree.getHuffmanCodes(), 
                                                   tree.getRoot());

        if (isDirectory) {
            fs::remove(tempArchivePath);
        }

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

        decompressor.setLogger([this](const std::string& msg) {
            emit logMessage(QString::fromStdString(msg));
        });

        decompressor.setProgressCallback([this](float p) {
            emit progressUpdated(p);
        });

        emit logMessage("Worker: Starting decompression task...");
        
        // Decompress to a temp location first to check if it's an archive
        std::string tempDecompPath = outputFile.toStdString() + ".tmp";
        
        ErrorCode result = decompressor.decompressFile(inputFile.toStdString(), tempDecompPath);

        if (result != ErrorCode::Success) {
            emit operationFinished(false, "Decompression failed with error code: " + QString::number((int)result));
            return;
        }

        // Check if it's an archive
        std::ifstream check(tempDecompPath, std::ios::binary);
        char magic[8];
        check.read(magic, 8);
        check.close();

        if (std::string(magic, 8) == "HUFFARCH") {
            emit logMessage("Worker: Detected archive. Extracting...");
            // It's an archive, extract it
            // For extraction, outputFile should be treated as a directory?
            // The current UI flow expects a single file output. 
            // We will extract to a folder named 'outputFile' (without extension if possible)
            
            // However, the UI passes a specific temp file path.
            // Let's extract to a folder with that name.
            
            // Ensure output directory is clean
            std::string outPath = outputFile.toStdString();
            if (fs::exists(outPath)) {
                fs::remove_all(outPath);
            }

            ErrorCode extractResult = Archiver::extractArchive(tempDecompPath, outPath);
            fs::remove(tempDecompPath); // Clean up temp

            if (extractResult == ErrorCode::Success) {
                emit operationFinished(true, "Extraction successful! Ready to save.");
            } else {
                emit operationFinished(false, "Extraction failed.");
            }
        } else {
            // Not an archive, just rename temp to final
            std::string outPath = outputFile.toStdString();
            if (fs::exists(outPath)) {
                fs::remove_all(outPath);
            }
            fs::rename(tempDecompPath, outPath);
            emit operationFinished(true, "Decompression successful! Ready to save.");
        }

    } catch (const std::exception& e) {
        emit operationFinished(false, QString("Critical Error: ") + e.what());
    } catch (...) {
        emit operationFinished(false, "Critical Error: Unknown exception occurred.");
    }
}
