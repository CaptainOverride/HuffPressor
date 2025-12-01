#include "utils.h"
#include "config.h"

#include <fstream>
#include <iostream>
#include <cstring>  // Required for std::memcmp
#include <cstddef>  // for size_t


bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, std::ios::binary | std::ios::ate);
    std::ifstream f2(file2, std::ios::binary | std::ios::ate);

    if (!f1.is_open() || !f2.is_open()) {
#if ENABLE_LOGGING
        std::cerr << "Error: One or both files could not be opened.\n";
#endif
        return false;
    }

    std::streamsize size1 = f1.tellg();
    std::streamsize size2 = f2.tellg();

    if (size1 != size2) {
#if ENABLE_LOGGING
        std::cerr << "File sizes differ: " << size1 << " vs " << size2 << "\n";
#endif
        return false;
    }

    f1.seekg(0);
    f2.seekg(0);

    constexpr size_t bufferSize = 4096;
    char buffer1[bufferSize], buffer2[bufferSize];

    while (f1 && f2) {
        f1.read(buffer1, bufferSize);
        f2.read(buffer2, bufferSize);

        std::streamsize bytesRead1 = f1.gcount();
        std::streamsize bytesRead2 = f2.gcount();

        if (bytesRead1 != bytesRead2 || std::memcmp(buffer1, buffer2, bytesRead1) != 0) {
#if ENABLE_LOGGING
            std::cerr << "Mismatch found in file content.\n";
#endif
            return false;
        }
    }

    return true;
}
