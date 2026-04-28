/**
 * env_soc_cog_space - Environmental Social Cognitive Space
 * Cross-platform C++ entry point.
 */

#include "Builders.h"
#include "JsonEcosystem.h"
#include "SimulationConfig.h"
#include "Utilities.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>

#if defined(COG_SOC_ENV_SPACE_PLATFORM_LINUX)
const char* platform_name = "Linux";
#elif defined(COG_SOC_ENV_SPACE_PLATFORM_MACOS)
const char* platform_name = "macOS";
#elif defined(COG_SOC_ENV_SPACE_PLATFORM_WINDOWS)
const char* platform_name = "Windows";
#else
const char* platform_name = "Unknown";
#endif

namespace {

/** Default path when run from the project root (see config/simulation.example.json). */
constexpr const char* kDefaultSimulationConfig = "config/simulation.example.json";

void printUsage(const char* exe_name) {
    const char* exe = (exe_name != nullptr && exe_name[0] != '\0') ? exe_name : "env_soc_cog_space";
    std::cerr << "Usage: " << exe
              << " [-c|--config path/to/simulation.json] [-e|--environment path/to/niche.json]\n";
}

} // namespace

int main(int argc, char* argv[]) {
    std::cout << "cog_soc_env_space v0.1.0 - Cognitive-social environment space\n";
    std::cout << "Platform: " << platform_name << "\n";

    std::string config_path = kDefaultSimulationConfig;
    std::string environment_override_path;
    bool has_environment_override = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = (argv[i] != nullptr) ? std::string(argv[i]) : std::string{};
        if (arg == "-c" || arg == "--config") {
            if (i + 1 >= argc || argv[i + 1] == nullptr || argv[i + 1][0] == '\0') {
                std::cerr << "Missing value for " << arg << "\n";
                printUsage(argc >= 1 ? argv[0] : nullptr);
                return 1;
            }
            config_path = argv[++i];
            continue;
        }
        if (arg == "-e" || arg == "--environment") {
            if (i + 1 >= argc || argv[i + 1] == nullptr || argv[i + 1][0] == '\0') {
                std::cerr << "Missing value for " << arg << "\n";
                printUsage(argc >= 1 ? argv[0] : nullptr);
                return 1;
            }
            environment_override_path = argv[++i];
            has_environment_override = true;
            continue;
        }
        std::cerr << "Unknown argument: " << arg << "\n";
        printUsage(argc >= 1 ? argv[0] : nullptr);
        return 1;
    }

    try {
        SimulationConfig::loadFromFile(config_path);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load simulation config: " << e.what() << "\n";
        printUsage(argc >= 1 ? argv[0] : nullptr);
        return 1;
    }

    const SimulationConfig& cfg = SimulationConfig::global();
    utilities::seedRng(cfg.random_seed);
    const std::string environment_path = has_environment_override ? environment_override_path : cfg.environment_path;

    std::cout << "Simulation config: version=" << cfg.version
              << " seed=" << cfg.random_seed
              << " total_cycles=" << cfg.total_cycles << "\n";
    std::cout << "Environment config: " << environment_path << "\n";

    Niche niche = NicheBuilder().loadEnvironment(environment_path).build();
    niche.initialize();
    nlohmann::json snapshot = JsonEcosystem::createJson(niche);
    for (int cycle = 0; cycle < cfg.total_cycles; ++cycle) {
        niche.step();
        JsonEcosystem::updateJson(niche, cycle, snapshot);
    }
    JsonEcosystem::saveJsonToFile(snapshot, "output/simulation.json");
    return 0;
}
