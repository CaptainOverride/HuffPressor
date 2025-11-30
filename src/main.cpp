#include "compressor.h"
#include "decompressor.h"
#include "huffmanTree.h"
#include "utils.h"
#include "config.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

// Simple console logger
void consoleLogger(const std::string& msg) {
    std::cout << msg;
}

// Simple console progress bar
void consoleProgress(float percentage) {
    int barWidth = 50;
    std::cout << "[";
    int pos = barWidth * percentage / 100.0;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << percentage << " %\r";
    std::cout.flush();
    if (percentage >= 100.0f) std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // Validate argument count; Expecting: program -mode input output
    if (argc != 4) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " -c <input_file> <compressed_file>\n"
                  << "  " << argv[0] << " -d <compressed_file> <output_file>\n";
        return 1;
    }

    std::string mode       = argv[1];  // -c or -d
    std::string inputFile  = argv[2];  // Input file path
    std::string outputFile = argv[3];  // Output file path

    if (mode == "-c") {
        // ===== COMPRESSION MODE =====
        Compressor compressor;
        HuffmanTree tree;

        // Set up callbacks
        compressor.setLogger(consoleLogger);
        compressor.setProgressCallback(consoleProgress);

        // Step 1: Build frequency map from input file
        if (!compressor.readFileAndBuildFrequency(inputFile)) {
            return 1;
        }

        // Step 2: Build Huffman tree from frequency map
        tree.build(compressor.getFrequencyMap());

        // Step 3: Generate Huffman codes from tree
        tree.generateCodes();

        // Step 4: Compress input using Huffman codes
        if (!compressor.compressFile(inputFile, outputFile, tree.getHuffmanCodes(), tree.getRoot())) {
            return 1;
        }

    } else if (mode == "-d") {
        // ===== DECOMPRESSION MODE =====
        Decompressor decompressor;

        // Set up callbacks
        decompressor.setLogger(consoleLogger);
        decompressor.setProgressCallback(consoleProgress);

        // Step 1: Decompress the file using Huffman decoding
        if (!decompressor.decompressFile(inputFile, outputFile)) {
            return 1;
        }

    } else {
        // Invalid operation mode
        std::cerr << "Invalid mode: " << mode << "\n";
        std::cerr << "Use -c to compress or -d to decompress.\n";
        return 1;
    }

    return 0;
}
