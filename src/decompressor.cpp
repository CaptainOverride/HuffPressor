#include "decompressor.h"
#include "huffmanTree.h"
#include "config.h"

#include <fstream>
#include <iostream>
#include <cstdint>
#include <sstream>

void Decompressor::setLogger(LogCallback logCallback) {
    logger = logCallback;
}

void Decompressor::setProgressCallback(ProgressCallback progCallback) {
    progress = progCallback;
}

Decompressor::~Decompressor() {
    freeTree(root);
}

void Decompressor::freeTree(HuffmanNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

ErrorCode Decompressor::decompressFile(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream input(inputFilename, std::ios::binary);
    if (!input.is_open()) {
        if (logger) logger("Failed to open compressed input file: " + inputFilename + "\n");
        return ErrorCode::FileNotFound;
    }

    std::ofstream output(outputFilename, std::ios::binary);
    if (!output.is_open()) {
        if (logger) logger("Failed to open output file: " + outputFilename + "\n");
        return ErrorCode::FileCreateError;
    }

    BitReader reader(input);

    // Step 1: Deserialize Huffman Tree
    root = deserializeTree(reader);
    if (!root) {
        if (logger) logger("Tree deserialization failed. Possibly corrupted input.\n");
        return ErrorCode::TreeDeserializationError;
    }

    if (logger) logger("Huffman Tree deserialized successfully.\n");

    // Step 2: Align to byte boundary
    reader.alignToByte();

    // Step 3: Read original file size
    uint64_t originalSize = 0;
    for (int i = 7; i >= 0; --i) {
        unsigned char sizeByte;
        if (!reader.readByte(sizeByte)) {
            if (logger) logger("Failed to read file size metadata.\n");
            return ErrorCode::FileReadError;
        }
        originalSize |= static_cast<uint64_t>(sizeByte) << (8 * i);
    }

    originalFileSize = originalSize;

    if (logger) {
        std::stringstream ss;
        ss << "Original file size to decode: " << originalFileSize << " bytes\n";
        logger(ss.str());
    }

    // Step 4: Decode
    decode(reader, output, root, originalFileSize);

    if (logger) logger("Decompression complete. Output saved at: " + outputFilename + "\n");
    return ErrorCode::Success;
}

HuffmanNode* Decompressor::deserializeTree(BitReader& reader) {
    bool bit;
    if (!reader.readBit(bit)) {
        if (logger) logger("Failed to read bit while deserializing tree.\n");
        return nullptr;
    }

    if (bit) {
        unsigned char byte;
        if (!reader.readByte(byte)) {
            if (logger) logger("Failed to read byte for leaf node.\n");
            return nullptr;
        }
        return new HuffmanNode(byte, 0);
    }

    HuffmanNode* left = deserializeTree(reader);
    HuffmanNode* right = deserializeTree(reader);

    if (!left || !right) {
        if (logger) logger("Incomplete subtree during tree reconstruction.\n");
        return nullptr;
    }

    return new HuffmanNode(0, left, right);
}

void Decompressor::decode(BitReader& reader, std::ostream& output, HuffmanNode* root, uint64_t originalSize) {
    HuffmanNode* current = root;
    bool bit;
    uint64_t bytesWritten = 0;

    // For progress reporting
    uint64_t lastReported = 0;
    const uint64_t reportInterval = originalSize / 100; // Report every 1% roughly

    while (bytesWritten < originalSize && reader.readBit(bit)) {
        current = bit ? current->right : current->left;

        if (current->isLeaf()) {
            output.put(current->byte);
            ++bytesWritten;
            current = root;

            if (progress && originalSize > 0) {
                if (bytesWritten - lastReported >= reportInterval || bytesWritten == originalSize) {
                    progress(static_cast<float>(bytesWritten) / originalSize * 100.0f);
                    lastReported = bytesWritten;
                }
            }
        }
    }

    if (bytesWritten < originalSize) {
        if (logger) {
            std::stringstream ss;
            ss << "Warning: Expected " << originalSize
               << " bytes, but only decoded " << bytesWritten << " bytes.\n";
            logger(ss.str());
        }
    }
}

uint64_t Decompressor::getOriginalFileSize() const {
    return originalFileSize;
}
