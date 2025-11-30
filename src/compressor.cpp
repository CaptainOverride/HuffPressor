#include "compressor.h"
#include "bitWriter.h"
#include "config.h"

#include <fstream>
#include <iostream>

bool Compressor::readFileAndBuildFrequency(const std::string& filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input.is_open()) {
#if ENABLE_LOGGING
        std::cerr << "Error: Could not open file " << filename << "\n";
#endif
        return false;
    }

    freqMap.clear();
    originalFileSize = 0;

    unsigned char byte;
    while (input.read(reinterpret_cast<char*>(&byte), 1)) {
        freqMap[byte]++;
        originalFileSize++;
    }

    input.close();

    if (originalFileSize == 0) {
#if ENABLE_LOGGING
        std::cerr << "Error: Input file is empty.\n";
#endif
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
#if ENABLE_LOGGING
        std::cerr << "Error: Cannot compress because Huffman tree root is null.\n";
#endif
        return false;
    }

    std::ifstream input(inputFilename, std::ios::binary);
    if (!input.is_open()) {
#if ENABLE_LOGGING
        std::cerr << "Error: Cannot open input file: " << inputFilename << "\n";
#endif
        return false;
    }

    std::ofstream output(outputFilename, std::ios::binary);
    if (!output.is_open()) {
#if ENABLE_LOGGING
        std::cerr << "Error: Cannot create output file: " << outputFilename << "\n";
#endif
        input.close();
        return false;
    }

    BitWriter writer(output);

    // Write Huffman Tree
    writer.writeTree(root);

    // Write original file size (64-bit big-endian)
#if ENABLE_LOGGING
    std::cout << "Writing original file size: " << originalFileSize << " bytes\n";
#endif
    for (int i = 7; i >= 0; --i) {
        unsigned char sizeByte = static_cast<unsigned char>((originalFileSize >> (i * 8)) & 0xFF);
        writer.writeByte(sizeByte);
#if ENABLE_LOGGING
        std::cout << "Size Byte[" << (7 - i) << "]: " << static_cast<int>(sizeByte) << "\n";
#endif
    }

    // Encode input file using Huffman codes
    unsigned char byte;
    while (input.read(reinterpret_cast<char*>(&byte), 1)) {
        auto it = codes.find(byte);
        if (it == codes.end()) {
#if ENABLE_LOGGING
            std::cerr << "Error: No Huffman code found for byte: " << static_cast<int>(byte) << "\n";
#endif
            input.close();
            output.close();
            return false;
        }

        const std::string& code = it->second;
        writer.writeBits(code);
#if ENABLE_LOGGING
        std::cout << "Encoding '" << byte << "' → " << code << " (" << code.length() << " bits)\n";
#endif
    }

    writer.flush();

    input.close();
    output.close();

#if ENABLE_LOGGING
    std::cout << "Compression complete. Output: " << outputFilename << "\n";
#endif
    return true;
}
 