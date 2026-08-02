// cook_tool — P4 ship builder CLI (thin argv wrapper over AYResource::cookShipPackage)
//
// Core logic lives in AYResource (AYCookShip.h). This EXE only parses args
// and prints progress / exit codes.

#include "AYCookShip.h"

#include <cstring>
#include <iostream>
#include <string>

using ayt::resource::CookShipOptions;
using ayt::resource::CookShipResult;
using ayt::resource::cookShipPackage;
using ayt::storage::CompressionAlgo;

static void printUsage(const char* exe)
{
    std::cerr
        << "Usage: " << exe << " --assets <dir> --out <shipDir> [options]\n"
        << "\n"
        << "Build a shippable content.pak + resources.db from a cooked .ay* tree.\n"
        << "\n"
        << "Required:\n"
        << "  --assets <dir>     Root directory of cooked assets (.aymesh, .aymat, ...)\n"
        << "  --out <dir>        Output ship directory\n"
        << "\n"
        << "Options:\n"
        << "  --pak <name>       Pak file name (default: content.pak)\n"
        << "  --db <name>        Database file name (default: resources.db)\n"
        << "  -c <algo>          Compression: none | lz4 | zstd (default: zstd)\n"
        << "  --no-recursive     Do not recurse into subdirectories\n"
        << "  -h, --help         Show this help\n"
        << "\n"
        << "Example:\n"
        << "  " << exe << " --assets ayeditor_cache/assets --out ship\n"
        << "  # then at runtime: ResourceManager::openDatabase(\"ship/resources.db\")\n";
}

int main(int argc, char* argv[])
{
    CookShipOptions opts;
    opts.recursive = true;

    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        auto need = [&](const char* flag) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << flag << " requires an argument\n";
                std::exit(1);
            }
            return argv[++i];
        };

        if (std::strcmp(a, "-h") == 0 || std::strcmp(a, "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        if (std::strcmp(a, "--assets") == 0) {
            opts.assetsRoot = need("--assets");
            continue;
        }
        if (std::strcmp(a, "--out") == 0) {
            opts.outputDir = need("--out");
            continue;
        }
        if (std::strcmp(a, "--pak") == 0) {
            opts.pakFileName = need("--pak");
            continue;
        }
        if (std::strcmp(a, "--db") == 0) {
            opts.dbFileName = need("--db");
            continue;
        }
        if (std::strcmp(a, "--no-recursive") == 0) {
            opts.recursive = false;
            continue;
        }
        if (std::strcmp(a, "-c") == 0) {
            const char* algo = need("-c");
            if (std::strcmp(algo, "none") == 0) {
                opts.compression = CompressionAlgo::None;
            } else if (std::strcmp(algo, "lz4") == 0) {
                opts.compression = CompressionAlgo::Lz4;
            } else if (std::strcmp(algo, "zstd") == 0) {
                opts.compression = CompressionAlgo::Zstd;
            } else {
                std::cerr << "Error: unknown compression '" << algo << "'\n";
                return 1;
            }
            continue;
        }

        std::cerr << "Error: unknown argument '" << a << "'\n";
        printUsage(argv[0]);
        return 1;
    }

    if (opts.assetsRoot.empty() || opts.outputDir.empty()) {
        printUsage(argv[0]);
        return 1;
    }

    std::cout << "[cook_tool] assets=" << opts.assetsRoot
              << " out=" << opts.outputDir << "\n";

    const CookShipResult result = cookShipPackage(opts);
    if (!result.ok) {
        std::cerr << "[cook_tool] FAILED: " << result.error << "\n";
        return 2;
    }

    std::cout << "[cook_tool] OK files=" << result.fileCount
              << " deps=" << result.dependencyCount << "\n"
              << "  pak: " << result.pakPath << "\n"
              << "  db:  " << result.dbPath << "\n";
    return 0;
}
