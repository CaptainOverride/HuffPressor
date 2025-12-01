#include "bitWriter.h"
#include "huffmanTree.h"
#include "config.h"

#include <iostream>
#include <string>
#include <bitset>  // Needed for std::bitset


// Constructor binds the writer to an output stream
BitWriter::BitWriter(std::ostream& outputStream) : out(outputStream) {}

// Destructor ensures that any remaining bits in the buffer are flushed
BitWriter::~BitWriter() {
    flush();
}

// Writes a single bit into the buffer. When 8 bits are collected, writes a byte to the stream.
void BitWriter::writeBit(bool bit) {
    buffer = (buffer << 1) | bit;  // Shift buffer and insert new bit
    bitCount++;

#if ENABLE_LOGGING
    std::cout << "Bit Written: " << bit
              << " | Buffer: " << std::bitset<8>(static_cast<int>(buffer))
              << " | Bit Count: " << bitCount << "\n";
#endif

    // Write byte when buffer is full
    if (bitCount == 8) {
        out.put(buffer);
        buffer = 0;
        bitCount = 0;
    }
}

// Writes a string of '0' and '1' characters to the stream as actual bits
void BitWriter::writeBits(const std::string& bits) {
    for (char c : bits) {
        writeBit(c == '1');
    }
}

// Writes a raw byte directly (used for writing file size, etc.)
void BitWriter::writeByte(unsigned char byte) {
    // Flush buffer if it's not aligned to a byte boundary
    if (bitCount != 0) flush();

    out.put(byte);

#if ENABLE_LOGGING
    std::cout << "Raw Byte Written: " << std::bitset<8>(static_cast<int>(byte))
              << " (" << static_cast<int>(byte) << ")\n";
#endif
}

// Flushes any bits left in the buffer by padding with 0s and writing the final byte
void BitWriter::flush() {
    if (bitCount > 0) {
        buffer <<= (8 - bitCount);  // Pad remaining bits with 0s

#if ENABLE_LOGGING
        std::cout << "Flushing incomplete byte: " << std::bitset<8>(static_cast<int>(buffer))
                  << " (" << static_cast<int>(buffer) << ")\n";
#endif

        out.put(buffer);
        buffer = 0;
        bitCount = 0;
    }
}

// Begins serialization of the Huffman tree
void BitWriter::writeTree(HuffmanNode* root) {
    serializeTree(root);
}

// Recursively serializes the Huffman tree in pre-order
void BitWriter::serializeTree(HuffmanNode* node) {
    if (!node) return;

    if (node->isLeaf()) {
        writeBit(1);  // Leaf marker
        for (int i = 7; i >= 0; --i) {
            writeBit((node->byte >> i) & 1);  // Write byte as 8 bits (MSB first)
        }
    } else {
        writeBit(0);  // Internal node marker
        serializeTree(node->left);
        serializeTree(node->right);
    }
}
