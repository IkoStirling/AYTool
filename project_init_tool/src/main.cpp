#include <AYProject/ProjectScaffold.h>

#include <cstdlib>
#include <iostream>
#include <string>

#ifndef AY_PROJECT_INIT_DEFAULT_ENGINE_SOURCE
#define AY_PROJECT_INIT_DEFAULT_ENGINE_SOURCE ""
#endif

namespace
{

void usage(const char* executable)
{
    std::cerr
        << "Usage: " << executable
        << " --output <directory> --name <display-name> [options]\n\n"
        << "Options:\n"
        << "  --id <stable-id>       Lowercase project id; derived from name when omitted\n"
        << "  --profile <profile>    client-2d or client-3d (default: client-3d)\n"
        << "  --engine <directory>   AliyatEngine source checkout\n"
        << "  --dry-run              Print the deterministic file plan only\n"
        << "  -h, --help             Show this help\n";
}

} // namespace

int main(int argc, char* argv[])
{
    ayt::project::GameProjectScaffoldOptions options;
    options.engineSource = AY_PROJECT_INIT_DEFAULT_ENGINE_SOURCE;
    bool dryRun = false;

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
        if (argument == "--output") options.destination = need("--output");
        else if (argument == "--name") options.displayName = need("--name");
        else if (argument == "--id") options.projectId = need("--id");
        else if (argument == "--engine") {
            options.engineSource = need("--engine");
        } else if (argument == "--profile") {
            const std::string profile = need("--profile");
            if (profile == "client-2d") {
                options.profile =
                    ayt::project::GameProjectTemplateProfile::Client2D;
            } else if (profile == "client-3d") {
                options.profile =
                    ayt::project::GameProjectTemplateProfile::Client3D;
            } else {
                std::cerr << "Unknown profile: " << profile << "\n";
                usage(argv[0]);
                return 1;
            }
        } else if (argument == "--dry-run") {
            dryRun = true;
        } else {
            std::cerr << "Unknown argument: " << argument << "\n";
            usage(argv[0]);
            return 1;
        }
    }

    if (options.destination.empty() || options.displayName.empty()) {
        usage(argv[0]);
        return 1;
    }

    const ayt::project::GameProjectScaffoldPlan plan =
        ayt::project::planGameProjectScaffold(options);
    if (!plan) {
        for (const std::string& diagnostic : plan.diagnostics) {
            std::cerr << "[project_init] " << diagnostic << "\n";
        }
        return 2;
    }

    std::cout << "[project_init] " << plan.displayName << " ("
              << plan.projectId << ")\n"
              << "  root: " << plan.projectRoot << "\n"
              << "  profile: " << plan.engineProfile << "\n"
              << "  application: " << plan.applicationTarget << "\n";
    for (const auto& file : plan.files) {
        std::cout << "  " << file.relativePath << "\n";
    }
    if (dryRun) return 0;

    std::string error;
    if (!ayt::project::writeGameProjectScaffold(plan, &error)) {
        std::cerr << "[project_init] " << error << "\n";
        return 3;
    }
    std::cout << "[project_init] Project created successfully.\n";
    return 0;
}
