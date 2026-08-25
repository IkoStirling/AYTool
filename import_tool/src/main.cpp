// import_tool — P5 import orchestration CLI (thin argv wrapper over AYImportJob)
//
// Core logic lives in AYResource (AYResource/ImportJob.h). This EXE only parses args
// and prints progress / exit codes.

#include "AYResource/ImportJob.h"

#include <AYIO/Env.h>
#include <AYLog.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using ayt::resource::ImportBatchOptions;
using ayt::resource::ImportBatchResult;
using ayt::resource::ImportOptions;
using ayt::resource::ImportProgress;
using ayt::resource::ImportResult;
using ayt::resource::ImportStage;
using ayt::resource::IConverter;
using ayt::resource::importAsset;
using ayt::resource::importAssetBatch;

static bool parseAxis(const std::string& text, ayt::resource::ImportAxis& out)
{
    if (text == "+X" || text == "X") out = ayt::resource::ImportAxis::PositiveX;
    else if (text == "-X") out = ayt::resource::ImportAxis::NegativeX;
    else if (text == "+Y" || text == "Y") out = ayt::resource::ImportAxis::PositiveY;
    else if (text == "-Y") out = ayt::resource::ImportAxis::NegativeY;
    else if (text == "+Z" || text == "Z") out = ayt::resource::ImportAxis::PositiveZ;
    else if (text == "-Z") out = ayt::resource::ImportAxis::NegativeZ;
    else return false;
    return true;
}

// Env fallback for --cook-textures (non-empty and not '0' → true).
static bool envTrue(const char* name)
{
    const auto v = ayt::io::env::get(name);
    return v.has_value() && !v->empty() && (*v)[0] != '0';
}

static void printUsage(const char* exe)
{
    std::cerr
        << "Usage: " << exe << " --in <file> --out <assetsDir> [options]\n"
        << "       " << exe << " --in <file1> --in <file2> ... --out <assetsDir>\n"
        << "\n"
        << "Import source assets (FBX/glTF/textures) into a cooked .ay* cache tree.\n"
        << "Matches EditorShellDemo layout: typically --out ayeditor_cache/assets\n"
        << "\n"
        << "Required:\n"
        << "  --in <path>        Source file (repeatable for batch)\n"
        << "  --out <dir>        Output assets root (meshes/, materials/, …)\n"
        << "\n"
        << "Options:\n"
        << "  --force            Ignore .aydep.json cache reuse\n"
        << "  --mesh-only        IConverter::LoadOption::MeshOnly\n"
        << "  --animation-only   Extract Animation resources only; do not cook\n"
        << "                     meshes, materials, textures, or helper geometry\n"
        << "  --no-character     Do not require Mesh+Skeleton for FBX cache hits\n"
        << "  --stop-on-error    Abort batch after first failure\n"
        << "  --cook-textures    Release cook: BC7+mips to .aytex (default: dev\n"
        << "                     mode — textures referenced raw as .png/.jpg)\n"
        << "                     Also honors env AY_IMPORT_COOK_TEXTURES=1\n"
        << "  --source-coordinates auto|manual\n"
        << "  --source-up <+X|-X|+Y|-Y|+Z|-Z>       Manual source Up axis\n"
        << "  --source-forward <axis>                Manual source Forward axis\n"
        << "  --source-handedness left|right         Manual source handedness\n"
        << "  --source-uv-origin top-left|bottom-left Source texture UV origin\n"
        << "  --source-meters-per-unit <float>       0 uses file unit metadata\n"
        << "  --source-coordinate-tag <text>         Cache/preset discriminator\n"
        << "  -h, --help         Show this help\n"
        << "\n"
        << "Example:\n"
        << "  " << exe << " --in model.fbx --out ayeditor_cache/assets\n"
        << "  " << exe << " --in a.fbx --in b.fbx --out cache/assets --force\n";
}

static const char* stageName(ImportStage s)
{
    switch (s) {
    case ImportStage::Validate: return "validate";
    case ImportStage::CheckCache: return "cache";
    case ImportStage::CacheHit: return "cache-hit";
    case ImportStage::CreateConverter: return "create";
    case ImportStage::Convert: return "convert";
    case ImportStage::Done: return "done";
    case ImportStage::Cancelled: return "cancelled";
    case ImportStage::Failed: return "failed";
    }
    return "?";
}

int main(int argc, char* argv[])
{
    ayt::log::LogConfig logConfig;
    logConfig.fileEnabled = false;
    logConfig.crashHandlerEnabled = false;
    logConfig.rateLimit.enabled = false;
    ayt::log::initialize(logConfig);
    struct LogLifetime {
        ~LogLifetime() {
            ayt::log::flush();
            ayt::log::shutdown();
        }
    } logLifetime;

    std::vector<std::string> inputs;
    std::string outDir;
    bool force = false;
    bool meshOnly = false;
    bool animationOnly = false;
    bool requireCharacter = true;
    bool stopOnError = false;
    bool cookTextures = false;
    ayt::resource::SourceCoordinatePolicy sourceCoordinates;

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
        if (std::strcmp(a, "--in") == 0) {
            inputs.emplace_back(need("--in"));
            continue;
        }
        if (std::strcmp(a, "--out") == 0) {
            outDir = need("--out");
            continue;
        }
        if (std::strcmp(a, "--force") == 0) {
            force = true;
            continue;
        }
        if (std::strcmp(a, "--mesh-only") == 0) {
            meshOnly = true;
            continue;
        }
        if (std::strcmp(a, "--animation-only") == 0) {
            animationOnly = true;
            continue;
        }
        if (std::strcmp(a, "--no-character") == 0) {
            requireCharacter = false;
            continue;
        }
        if (std::strcmp(a, "--stop-on-error") == 0) {
            stopOnError = true;
            continue;
        }
        if (std::strcmp(a, "--cook-textures") == 0) {
            cookTextures = true;
            continue;
        }
        if (std::strcmp(a, "--source-coordinates") == 0) {
            const std::string value = need("--source-coordinates");
            if (value == "manual") sourceCoordinates.mode = ayt::resource::SourceCoordinateMode::Manual;
            else if (value == "auto") sourceCoordinates.mode = ayt::resource::SourceCoordinateMode::Auto;
            else { std::cerr << "Error: source coordinates must be auto or manual\n"; return 1; }
            continue;
        }
        if (std::strcmp(a, "--source-up") == 0) {
            if (!parseAxis(need("--source-up"), sourceCoordinates.up)) {
                std::cerr << "Error: invalid --source-up axis\n"; return 1;
            }
            continue;
        }
        if (std::strcmp(a, "--source-forward") == 0) {
            if (!parseAxis(need("--source-forward"), sourceCoordinates.forward)) {
                std::cerr << "Error: invalid --source-forward axis\n"; return 1;
            }
            continue;
        }
        if (std::strcmp(a, "--source-handedness") == 0) {
            const std::string value = need("--source-handedness");
            if (value == "left") sourceCoordinates.handedness = ayt::resource::ImportHandedness::Left;
            else if (value == "right") sourceCoordinates.handedness = ayt::resource::ImportHandedness::Right;
            else { std::cerr << "Error: handedness must be left or right\n"; return 1; }
            continue;
        }
        if (std::strcmp(a, "--source-uv-origin") == 0) {
            const std::string value = need("--source-uv-origin");
            if (value == "top-left") {
                sourceCoordinates.uvOrigin =
                    ayt::resource::ImportUvOrigin::TopLeft;
            } else if (value == "bottom-left") {
                sourceCoordinates.uvOrigin =
                    ayt::resource::ImportUvOrigin::BottomLeft;
            } else {
                std::cerr << "Error: UV origin must be top-left or bottom-left\n";
                return 1;
            }
            continue;
        }
        if (std::strcmp(a, "--source-meters-per-unit") == 0) {
            try { sourceCoordinates.metersPerUnit = std::stof(need("--source-meters-per-unit")); }
            catch (...) { std::cerr << "Error: invalid meters-per-unit\n"; return 1; }
            continue;
        }
        if (std::strcmp(a, "--source-coordinate-tag") == 0) {
            sourceCoordinates.tag = need("--source-coordinate-tag");
            continue;
        }

        std::cerr << "Error: unknown argument '" << a << "'\n";
        printUsage(argv[0]);
        return 1;
    }

    if (inputs.empty() || outDir.empty()) {
        printUsage(argv[0]);
        return 1;
    }
    if (meshOnly && animationOnly) {
        std::cerr << "Error: --mesh-only and --animation-only are mutually exclusive\n";
        return 1;
    }

    auto onProgress = [](const ImportProgress& p) {
        std::cout << "[import_tool] " << stageName(p.stage)
                  << " " << static_cast<int>(p.fraction * 100.0f) << "% "
                  << p.message << "\n";
    };

    if (inputs.size() == 1) {
        ImportOptions opts;
        opts.sourcePath = inputs[0];
        opts.outputDir = outDir;
        opts.force = force;
        opts.requireCharacterAssets = requireCharacter && !animationOnly;
        opts.requireAnimationAssets = animationOnly;
        opts.loadOption = animationOnly ? IConverter::LoadOption::AnimationOnly
                        : meshOnly ? IConverter::LoadOption::MeshOnly
                                   : IConverter::LoadOption::Full;
        opts.cookTextures = cookTextures
            || envTrue("AY_IMPORT_COOK_TEXTURES");
        opts.sourceCoordinates = sourceCoordinates;

        std::cout << "[import_tool] in=" << opts.sourcePath
                  << " out=" << opts.outputDir << "\n";

        const ImportResult result = importAsset(opts, onProgress, nullptr);
        if (result.cancelled) {
            std::cerr << "[import_tool] CANCELLED\n";
            return 3;
        }
        if (!result.ok) {
            std::cerr << "[import_tool] FAILED: " << result.error << "\n";
            return 2;
        }
        std::cout << "[import_tool] OK cache=" << (result.usedCache ? "hit" : "miss")
                  << " resources=" << result.conversion.resources.size() << "\n";
        return 0;
    }

    ImportBatchOptions batch;
    batch.sourcePaths = std::move(inputs);
    batch.outputDir = outDir;
    batch.force = force;
    batch.requireCharacterAssets = requireCharacter && !animationOnly;
    batch.requireAnimationAssets = animationOnly;
    batch.stopOnError = stopOnError;
    batch.loadOption = animationOnly ? IConverter::LoadOption::AnimationOnly
                     : meshOnly ? IConverter::LoadOption::MeshOnly
                                : IConverter::LoadOption::Full;
    batch.cookTextures = cookTextures
        || envTrue("AY_IMPORT_COOK_TEXTURES");
    batch.sourceCoordinates = sourceCoordinates;

    std::cout << "[import_tool] batch count=" << batch.sourcePaths.size()
              << " out=" << batch.outputDir << "\n";

    const ImportBatchResult result = importAssetBatch(batch, onProgress, nullptr);
    std::cout << "[import_tool] batch done ok=" << result.okCount
              << " fail=" << result.failCount
              << " cacheHits=" << result.cacheHitCount
              << " cancelled=" << result.cancelledCount << "\n";

    if (result.cancelledCount > 0) {
        return 3;
    }
    if (result.failCount > 0) {
        for (size_t i = 0; i < result.results.size(); ++i) {
            if (!result.results[i].ok && !result.results[i].cancelled) {
                std::cerr << "[import_tool] FAIL[" << i << "]: "
                          << result.results[i].error << "\n";
            }
        }
        return 2;
    }
    return 0;
}
