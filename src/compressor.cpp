#include "compressor.h"
#include "bitWriter.h"
#include "config.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

void Compressor::setLogger(LogCallback logCallback) {
    logger = logCallback;
}

void Compressor::setProgressCallback(ProgressCallback progCallback) {
    progress = progCallback;
}

bool Compressor::readFileAndBuildFrequency(const std::string& filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input.is_open()) {
        if (logger) logger("Error: Could not open file " + filename + "\n");
        return false;
    }

    freqMap.clear();
    originalFileSize = 0;

    const size_t BUFFER_SIZE = 64 * 1024; // 64KB
    std::vector<char> buffer(BUFFER_SIZE);

    while (input) {
        input.read(buffer.data(), BUFFER_SIZE);
        std::streamsize bytesRead = input.gcount();
        if (bytesRead == 0) break;

        for (std::streamsize i = 0; i < bytesRead; ++i) {
            unsigned char byte = static_cast<unsigned char>(buffer[i]);
            freqMap[byte]++;
            originalFileSize++;
        }
    }

    input.close();

    if (originalFileSize == 0) {
        if (logger) logger("Error: Input file is empty.\n");
        return false;
    }

    return true;
}

const std::unordered_map<unsigned char, int>& Compressor::getFrequencyMap() const {
    return freqMap;
}

uint64_t Compressor::getOriginalFileSize() const {
    return originalFileSize;
}

bool Compressor::compressFile(const std::string& inputFilename,
                              const std::string& outputFilename,
                              const std::unordered_map<unsigned char, std::string>& codes,
                              HuffmanNode* root) {
    if (!root) {
        if (logger) logger("Error: Cannot compress because Huffman tree root is null.\n");
        return false;
    }

    std::ifstream input(inputFilename, std::ios::binary);
    if (!input.is_open()) {
        if (logger) logger("Error: Cannot open input file: " + inputFilename + "\n");
        return false;
    }

    std::ofstream output(outputFilename, std::ios::binary);
    if (!output.is_open()) {
        if (logger) logger("Error: Cannot create output file: " + outputFilename + "\n");
        input.close();
        return false;
    }

    BitWriter writer(output);

    // Write Huffman Tree
    writer.writeTree(root);

    // Write original file size (64-bit big-endian)
    if (logger) {
        std::stringstream ss;
        ss << "Writing original file size: " << originalFileSize << " bytes\n";
        logger(ss.str());
    }

    for (int i = 7; i >= 0; --i) {
        unsigned char sizeByte = static_cast<unsigned char>((originalFileSize >> (i * 8)) & 0xFF);
        writer.writeByte(sizeByte);
    }

    // Encode input file using Huffman codes
    const size_t BUFFER_SIZE = 64 * 1024; // 64KB
    std::vector<char> buffer(BUFFER_SIZE);
    uint64_t bytesProcessed = 0;

    while (input) {
        input.read(buffer.data(), BUFFER_SIZE);
        std::streamsize bytesRead = input.gcount();
        if (bytesRead == 0) break;

        for (std::streamsize i = 0; i < bytesRead; ++i) {
            unsigned char byte = static_cast<unsigned char>(buffer[i]);
            auto it = codes.find(byte);
            if (it == codes.end()) {
                if (logger) {
                    std::stringstream ss;
                    ss << "Error: No Huffman code found for byte: " << static_cast<int>(byte) << "\n";
                    logger(ss.str());
                }
                input.close();
                output.close();
                return false;
            }

            const std::string& code = it->second;
            writer.writeBits(code);
        }
        
        bytesProcessed += bytesRead;
        if (progress && originalFileSize > 0) {
            progress(static_cast<float>(bytesProcessed) / originalFileSize * 100.0f);
        }
    }

    writer.flush();

    input.close();
    output.close();

    if (logger) {
        logger("Compression complete. Output: " + outputFilename + "\n");
    }
    return true;
}