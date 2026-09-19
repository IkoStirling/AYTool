#include "AYResource/ProjectBuild.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

namespace fs = std::filesystem;
using namespace ayt::resource;

void usage(const char* executable)
{
    std::cerr
        << "Usage: " << executable
        << " --project <root> --profile <file> [options]\n\n"
        << "Options:\n"
        << "  --plan          Print the deterministic asset plan only\n"
        << "  --dry-run       Validate and count without writing output\n"
        << "  --skip-code     Do not invoke the configured CMake backend\n"
        << "  --skip-package  Stage Pak-selected files as loose files\n"
        << "  --force-cook    Ignore existing .cookCache objects\n"
        << "  --cache-only    Never cook; fail on a cache miss\n"
        << "  --no-cook       Alias for a strict cache-only execution\n"
        << "  -h, --help      Show this help\n";
}

const char* severity(ProjectBuildDiagnostic::Severity value)
{
    switch (value) {
    case ProjectBuildDiagnostic::Severity::Info: return "info";
    case ProjectBuildDiagnostic::Severity::Warning: return "warning";
    case ProjectBuildDiagnostic::Severity::Error: return "error";
    }
    return "unknown";
}

} // namespace

int main(int argc, char* argv[])
{
    std::string project;
    std::string profilePath;
    bool planOnly = false;
    ProjectBuildExecutionOptions options;

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        const auto need = [&](const char* flag) -> const char* {
            if (index + 1 >= argc) {
                std::cerr << flag << " requires an argument\n";
                std::exit(1);
            }
            return argv[++index];
        };
        if (argument == "-h" || argument == "--help") {
            usage(argv[0]);
            return 0;
        }
        if (argument == "--project") project = need("--project");
        else if (argument == "--profile") profilePath = need("--profile");
        else if (argument == "--plan") planOnly = true;
        else if (argument == "--dry-run") options.dryRun = true;
        else if (argument == "--skip-code") options.skipCode = true;
        else if (argument == "--skip-package") options.skipPackage = true;
        else if (argument == "--force-cook") options.forceCook = true;
        else if (argument == "--cache-only") options.cacheOnly = true;
        else if (argument == "--no-cook") options.neverCook = true;
        else {
            std::cerr << "Unknown argument: " << argument << "\n";
            usage(argv[0]);
            return 1;
        }
    }
    if (project.empty() || profilePath.empty()) {
        usage(argv[0]);
        return 1;
    }

    const fs::path projectRoot = fs::absolute(project).lexically_normal();
    fs::path profileFile = profilePath;
    if (profileFile.is_relative()) profileFile = projectRoot / profileFile;
    std::string error;
    const ProjectBuildProfile profile = ProjectBuildProfile::load(
        profileFile.string(), &error);
    if (!profile) {
        std::cerr << "[project_build] " << error << "\n";
        return 2;
    }
    const ProjectBuildPlan plan = ProjectBuildPlanner::create(
        profile, projectRoot.string());
    if (planOnly) {
        std::cout << plan.serialize(true) << "\n";
        return plan.valid() ? 0 : 3;
    }
    for (const ProjectBuildDiagnostic& item : plan.diagnostics) {
        std::cerr << "[" << severity(item.severity) << "]";
        if (!item.asset.empty()) std::cerr << " " << item.asset << ":";
        std::cerr << " " << item.message << "\n";
    }
    if (!plan.valid()) return 3;

    const ProjectBuildResult result = ProjectBuildExecutor::execute(
        plan, options, [](const ProjectBuildProgress& progress) {
            std::cout << "[project_build] "
                      << static_cast<int>(progress.fraction * 100.0f)
                      << "% " << progress.message << "\n";
        });
    for (const ProjectBuildDiagnostic& item : result.diagnostics) {
        if (item.severity == ProjectBuildDiagnostic::Severity::Info) continue;
        std::cerr << "[" << severity(item.severity) << "]";
        if (!item.asset.empty()) std::cerr << " " << item.asset << ":";
        std::cerr << " " << item.message << "\n";
    }
    if (!result.ok) {
        std::cerr << "[project_build] FAILED: " << result.error << "\n";
        return 4;
    }
    std::cout << "[project_build] OK raw=" << result.rawCount
              << " cooked=" << result.cookedCount
              << " cacheHits=" << result.cacheHitCount
              << " pakFiles=" << result.pakFileCount << "\n"
              << "  output: " << result.outputDirectory << "\n";
    if (!result.executable.empty()) {
        std::cout << "  executable: " << result.executable << "\n";
    }
    if (!result.manifestPath.empty()) {
        std::cout << "  manifest: " << result.manifestPath << "\n";
    }
    return 0;
}
