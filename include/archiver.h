#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <string>
#include <vector>
#include "errors.h"

class Archiver {
public:
    // Bundles a directory into a single output file
    static ErrorCode archiveDirectory(const std::string& directoryPath, const std::string& outputFilename);

    // Extracts an archive file to a directory
    static ErrorCode extractArchive(const std::string& archiveFilename, const std::string& outputDirectory);
};

#endif // ARCHIVER_H
