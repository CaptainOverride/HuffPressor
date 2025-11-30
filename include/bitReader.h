#ifndef BITREADER_H
#define BITREADER_H

#include <istream>
#include <vector>

// BitReader is a utility class for reading individual bits or bytes from an input stream.
// It buffers data byte-by-byte and provides bitwise access for decoding purposes.
class BitReader {
public:
    // Constructor binds the BitReader to an existing input stream
    explicit BitReader(std::istream& input);

    // Reads the next single bit from the input stream.
    // Returns true if a bit was successfully read, false on failure (EOF or error).
    bool readBit(bool& bit);

    // Reads the next byte (8 bits) from the stream directly.
    // Useful for reading characters during tree deserialization.
    bool readByte(unsigned char& byte);

    // Aligns the bit reader to the next full byte boundary by discarding leftover bits
    void alignToByte();

private:
    std::istream& inputStream;     // Reference to the input file/stream
    unsigned char currentByte = 0; // Buffer to store a byte being read bit-by-bit
    int bitsRemaining = 0;         // How many bits are left to read from the current buffer

    // Buffer for bulk reading
    std::vector<char> fileBuffer;
    size_t bufferIndex = 0;
    size_t bufferSize = 0;

    bool refillBuffer();
};

#endif // BITREADER_H
