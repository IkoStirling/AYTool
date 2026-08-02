#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "aystorage/IPackageWriter.h"

using namespace ayt::storage;

static void printUsage(const char* exeName) {
    std::cerr << "Usage: " << exeName << " [options] <output.pak>\n"
              << "Options:\n"
              << "  -a <file>:<entry>   Add file to package (file path : entry name in package)\n"
              << "  -c <algo>          Compression algorithm: none, lz4, zstd (default: zstd)\n"
              << "  -s <max>           Max entries per segment (default: 1000)\n"
              << "  -h                 Show this help\n"
              << "\nExample:\n"
              << "  " << exeName << " -a textures/player.png:textures/player.png output.pak\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::vector<std::pair<std::string, std::string>> filesToAdd;
    std::string outputPath;
    CompressionAlgo algo = CompressionAlgo::Zstd;
    size_t segmentSize = 1000;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -a requires an argument\n";
                return 1;
            }
            ++i;
            std::string arg = argv[i];
            size_t colonPos = arg.find(':');
            if (colonPos == std::string::npos) {
                std::cerr << "Error: -a argument must be in format 'filePath:entryName'\n";
                return 1;
            }
            std::string filePath = arg.substr(0, colonPos);
            std::string entryName = arg.substr(colonPos + 1);
            filesToAdd.emplace_back(filePath, entryName);
        } else if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -c requires an argument\n";
                return 1;
            }
            ++i;
            std::string algoStr = argv[i];
            if (algoStr == "none") algo = CompressionAlgo::None;
            else if (algoStr == "lz4") algo = CompressionAlgo::Lz4;
            else if (algoStr == "zstd") algo = CompressionAlgo::Zstd;
            else {
                std::cerr << "Error: unknown compression algorithm '" << algoStr << "'\n";
                return 1;
            }
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -s requires an argument\n";
                return 1;
            }
            ++i;
            segmentSize = std::stoul(argv[i]);
        } else {
            outputPath = argv[i];
        }
    }

    if (outputPath.empty()) {
        std::cerr << "Error: no output path specified\n";
        return 1;
    }

    if (filesToAdd.empty()) {
        std::cerr << "Error: no files to add (use -a option)\n";
        return 1;
    }

    auto writer = IPackageWriter::create(outputPath);
    if (!writer) {
        std::cerr << "Error: failed to create package writer\n";
        return 1;
    }

    writer->setCompressionAlgo(algo);
    writer->setSegmentSize(segmentSize);

    int addedCount = 0;
    for (const auto& [filePath, entryName] : filesToAdd) {
        if (!writer->addFile(filePath, entryName)) {
            std::cerr << "Warning: failed to add '" << filePath << "'\n";
        } else {
            std::cout << "Added: " << entryName << "\n";
            ++addedCount;
        }
    }

    if (addedCount == 0) {
        std::cerr << "Error: no files were added\n";
        return 1;
    }

    if (!writer->flush()) {
        std::cerr << "Error: failed to write package\n";
        return 1;
    }

    std::cout << "Created: " << outputPath << " (" << addedCount << " files)\n";
    return 0;
}