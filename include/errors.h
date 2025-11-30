#ifndef ERRORS_H
#define ERRORS_H

#include <string>

enum class ErrorCode {
    Success = 0,
    FileNotFound,
    FileEmpty,
    FileCreateError,
    FileReadError,
    FileWriteError,
    InvalidFormat,
    TreeSerializationError,
    TreeDeserializationError,
    CompressionFailed,
    DecompressionFailed,
    UnknownError
};

inline std::string getErrorMessage(ErrorCode code) {
    switch (code) {
        case ErrorCode::Success: return "Operation successful.";
        case ErrorCode::FileNotFound: return "File not found.";
        case ErrorCode::FileEmpty: return "Input file is empty.";
        case ErrorCode::FileCreateError: return "Could not create output file.";
        case ErrorCode::FileReadError: return "Error reading from file.";
        case ErrorCode::FileWriteError: return "Error writing to file.";
        case ErrorCode::InvalidFormat: return "Invalid file format or corrupted data.";
        case ErrorCode::TreeSerializationError: return "Failed to serialize Huffman tree.";
        case ErrorCode::TreeDeserializationError: return "Failed to deserialize Huffman tree.";
        case ErrorCode::CompressionFailed: return "Compression process failed.";
        case ErrorCode::DecompressionFailed: return "Decompression process failed.";
        default: return "Unknown error occurred.";
    }
}

#endif // ERRORS_H
