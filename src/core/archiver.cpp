#include "archiver.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

// Helper to write 64-bit integer
void writeUint64(std::ofstream& out, uint64_t val) {
    for (int i = 0; i < 8; ++i) {
        out.put((val >> (i * 8)) & 0xFF);
    }
}

// Helper to read 64-bit integer
uint64_t readUint64(std::ifstream& in) {
    uint64_t val = 0;
    for (int i = 0; i < 8; ++i) {
        val |= (static_cast<uint64_t>(static_cast<unsigned char>(in.get())) << (i * 8));
    }
    return val;
}

ErrorCode Archiver::archiveDirectory(const std::string& directoryPath, const std::string& outputFilename) {
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        return ErrorCode::FileNotFound;
    }

    std::ofstream out(outputFilename, std::ios::binary);
    if (!out.is_open()) return ErrorCode::FileCreateError;

    // Collect all files
    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path());
        }
    }

    // Write Magic Header
    out.write("HUFFARCH", 8);

    // Write file count
    writeUint64(out, files.size());

    for (const auto& filePath : files) {
        // Get relative path
        std::string relPath = fs::relative(filePath, directoryPath).generic_string();
        uint64_t pathLen = relPath.size();
        uint64_t fileSize = fs::file_size(filePath);

        // Write path length and path
        writeUint64(out, pathLen);
        out.write(relPath.c_str(), pathLen);

        // Write file size
        writeUint64(out, fileSize);

        // Write content
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile.is_open()) return ErrorCode::FileNotFound;
        out << inFile.rdbuf();
    }

    return ErrorCode::Success;
}

ErrorCode Archiver::extractArchive(const std::string& archiveFilename, const std::string& outputDirectory) {
    std::ifstream in(archiveFilename, std::ios::binary);
    if (!in.is_open()) return ErrorCode::FileNotFound;

    fs::create_directories(outputDirectory);

    char magic[8];
    in.read(magic, 8);
    if (std::string(magic, 8) != "HUFFARCH") {
        return ErrorCode::UnknownError; // Not an archive
    }

    uint64_t fileCount = readUint64(in);

    for (uint64_t i = 0; i < fileCount; ++i) {
        // Read path
        uint64_t pathLen = readUint64(in);
        std::string relPath(pathLen, '\0');
        in.read(&relPath[0], pathLen);

        // Read size
        uint64_t fileSize = readUint64(in);

        // Prepare output file
        fs::path outPath = fs::path(outputDirectory) / relPath;
        fs::create_directories(outPath.parent_path());

        std::ofstream outFile(outPath, std::ios::binary);
        if (!outFile.is_open()) return ErrorCode::FileCreateError;

        // Copy content
        // We can't use rdbuf directly because we need to read exactly fileSize bytes
        // and the stream continues after that.
        const size_t BUFFER_SIZE = 4096;
        char buffer[BUFFER_SIZE];
        uint64_t remaining = fileSize;
        while (remaining > 0) {
            size_t toRead = std::min(static_cast<uint64_t>(BUFFER_SIZE), remaining);
            in.read(buffer, toRead);
            outFile.write(buffer, toRead);
            remaining -= toRead;
        }
    }

    return ErrorCode::Success;
}
