#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "aystorage/IResourceMetaIndex.h"

using namespace ayt::storage;

static void printUsage(const char* exeName) {
    std::cerr << "Usage: " << exeName << " [options] <output.db>\n"
              << "Options:\n"
              << "  -s <dir>          Scan directory (can be used multiple times)\n"
              << "  -r                Recursive scan (default: non-recursive)\n"
              << "  -a <path>:<type>  Add single resource (path:type)\n"
              << "  -h               Show this help\n"
              << "\nExample:\n"
              << "  " << exeName << " -s textures/ -s audio/ -r resources.db\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::vector<std::string> dirsToScan;
    std::vector<std::pair<std::string, std::string>> resourcesToAdd;
    std::string outputPath;
    bool recursive = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -s requires an argument\n";
                return 1;
            }
            ++i;
            dirsToScan.push_back(argv[i]);
        } else if (strcmp(argv[i], "-r") == 0) {
            recursive = true;
        } else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -a requires an argument\n";
                return 1;
            }
            ++i;
            std::string arg = argv[i];
            size_t colonPos = arg.find(':');
            if (colonPos == std::string::npos) {
                std::cerr << "Error: -a argument must be in format 'path:type'\n";
                return 1;
            }
            std::string path = arg.substr(0, colonPos);
            std::string type = arg.substr(colonPos + 1);
            resourcesToAdd.emplace_back(path, type);
        } else {
            outputPath = argv[i];
        }
    }

    if (outputPath.empty()) {
        std::cerr << "Error: no output database path specified\n";
        return 1;
    }

    if (dirsToScan.empty() && resourcesToAdd.empty()) {
        std::cerr << "Error: no directories to scan and no resources to add\n";
        return 1;
    }

    // Create resource meta index
    auto index = IResourceMetaIndex::create(outputPath);
    if (!index) {
        std::cerr << "Error: failed to create index\n";
        return 1;
    }

    // Scan directories
    for (const auto& dir : dirsToScan) {
        std::cout << "Scanning: " << dir << " (recursive=" << recursive << ")\n";
        index->scanDirectory(dir, recursive);
    }

    // Add individual resources
    for (const auto& [path, type] : resourcesToAdd) {
        index->addResource(path, type, 0, 0);
        std::cout << "Added: " << path << " (" << type << ")\n";
    }

    std::cout << "Created index: " << outputPath << "\n";
    return 0;
}