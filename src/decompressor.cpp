#include "decompressor.h"
#include "huffmanTree.h"
#include "config.h"

#include <fstream>
#include <iostream>
#include <cstdint>

// Destructor: Frees the dynamically allocated Huffman tree
Decompressor::~Decompressor() {
    freeTree(root);
}

// Recursively deletes nodes of the Huffman tree
void Decompressor::freeTree(HuffmanNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// Main decompression routine
bool Decompressor::decompressFile(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream input(inputFilename, std::ios::binary);
    if (!input.is_open()) {
#if ENABLE_LOGGING
        std::cerr << "Failed to open compressed input file: " << inputFilename << "\n";
#endif
        return false;
    }

    std::ofstream output(outputFilename, std::ios::binary);
    if (!output.is_open()) {
#if ENABLE_LOGGING
        std::cerr << "Failed to open output file: " << outputFilename << "\n";
#endif
        input.close();
        return false;
    }

    BitReader reader(input);

    // Step 1: Reconstruct the Huffman tree
    root = deserializeTree(reader);
    if (!root) {
#if ENABLE_LOGGING
        std::cerr << "Tree deserialization failed. Possibly corrupted input.\n";
#endif
        input.close();
        output.close();
        return false;
    }

#if ENABLE_LOGGING
    std::cout << "Huffman Tree deserialized successfully.\n";
#endif

    // Step 2: Align to byte boundary before reading file size
    reader.alignToByte();

    // Step 3: Read original file size from metadata
    uint64_t originalSize = 0;
    for (int i = 7; i >= 0; --i) {
        unsigned char sizeByte;
        if (!reader.readByte(sizeByte)) {
#if ENABLE_LOGGING
            std::cerr << "Failed to read file size metadata.\n";
#endif
            input.close();
            output.close();
            return false;
        }
        originalSize |= static_cast<uint64_t>(sizeByte) << (8 * i);
    }

    originalFileSize = originalSize;

#if ENABLE_LOGGING
    std::cout << "Original file size to decode: " << originalFileSize << " bytes\n";
#endif

    // Step 4: Decode using Huffman tree
    decode(reader, output, root, originalFileSize);

#if ENABLE_LOGGING
    std::cout << "Decompression complete. Output saved at: " << outputFilename << "\n";
#endif

    input.close();
    output.close();
    return true;
}

// Recursively rebuilds Huffman tree from bitstream
HuffmanNode* Decompressor::deserializeTree(BitReader& reader) {
    bool bit;
    if (!reader.readBit(bit)) {
#if ENABLE_LOGGING
        std::cerr << "Failed to read bit while deserializing tree.\n";
#endif
        return nullptr;
    }

    if (bit) {
        // Leaf node
        unsigned char byte;
        if (!reader.readByte(byte)) {
#if ENABLE_LOGGING
            std::cerr << "Failed to read byte for leaf node.\n";
#endif
            return nullptr;
        }
        return new HuffmanNode(byte, 0);
    }

    // Internal node
    HuffmanNode* left = deserializeTree(reader);
    HuffmanNode* right = deserializeTree(reader);

    if (!left || !right) {
#if ENABLE_LOGGING
        std::cerr << "Incomplete subtree during tree reconstruction.\n";
#endif
        return nullptr;
    }

    return new HuffmanNode(0, left, right);
}

// Decodes compressed bitstream using the reconstructed Huffman tree
void Decompressor::decode(BitReader& reader, std::ostream& output, HuffmanNode* root, uint64_t originalSize) {
    HuffmanNode* current = root;
    bool bit;
    uint64_t bytesWritten = 0;

    while (bytesWritten < originalSize && reader.readBit(bit)) {
        current = bit ? current->right : current->left;

        if (current->isLeaf()) {
            output.put(current->byte);
            ++bytesWritten;
            current = root;
        }
    }

    if (bytesWritten < originalSize) {
#if ENABLE_LOGGING
        std::cerr << "Warning: Expected " << originalSize
                  << " bytes, but only decoded " << bytesWritten << " bytes.\n";
#endif
    }
}

// Returns file size read from compressed metadata
uint64_t Decompressor::getOriginalFileSize() const {
    return originalFileSize;
}
