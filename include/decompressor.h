#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include "bitReader.h"
#include "huffmanTree.h"
#include "callbacks.h"
#include <string>
#include <fstream>
#include <cstdint>

class Decompressor {
public:
    bool decompressFile(const std::string& inputFilename, const std::string& outputFilename);

    uint64_t getOriginalFileSize() const;

    void setLogger(LogCallback logCallback);
    void setProgressCallback(ProgressCallback progCallback);

    ~Decompressor();  // Destructor to free tree memory

private:
    HuffmanNode* deserializeTree(BitReader& reader);
    void decode(BitReader& reader, std::ostream& output, HuffmanNode* root, uint64_t originalSize);
    void freeTree(HuffmanNode* node);

    HuffmanNode* root = nullptr;  // Store root for cleanup
    uint64_t originalFileSize = 0;
    LogCallback logger;
    ProgressCallback progress;
};

#endif // DECOMPRESSOR_H
