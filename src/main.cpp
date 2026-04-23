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

} // namespace

int main(int argc, char* argv[]) {
    std::cout << "cog_soc_env_space v0.1.0 - Cognitive-social environment space\n";
    std::cout << "Platform: " << platform_name << "\n";

    const std::string config_path = (argc >= 2 && argv[1] != nullptr && argv[1][0] != '\0')
                                        ? std::string(argv[1])
                                        : std::string(kDefaultSimulationConfig);

    try {
        SimulationConfig::loadFromFile(config_path);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load simulation config: " << e.what() << "\n";
        std::cerr << "Usage: " << (argc >= 1 ? argv[0] : "env_soc_cog_space")
                  << " [path/to/simulation.json]\n";
        return 0;
    }

    const SimulationConfig& cfg = SimulationConfig::global();
    utilities::seedRng(cfg.random_seed);

    std::cout << "Simulation config: version=" << cfg.version
              << " seed=" << cfg.random_seed
              << " total_cycles=" << cfg.total_cycles << "\n";

    Niche niche = NicheBuilder().loadEnvironment(cfg.environment_path).build();
    niche.initialize();
    nlohmann::json snapshot = JsonEcosystem::createJson(niche);
    for (int cycle = 0; cycle < cfg.total_cycles; ++cycle) {
        niche.step();
        JsonEcosystem::updateJson(niche, cycle, snapshot);
    }
    JsonEcosystem::saveJsonToFile(snapshot, "output/simulation.json");
    return 0;
}
