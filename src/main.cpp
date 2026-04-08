/**
 * env_soc_cog_space - Environmental Social Cognitive Space
 * Cross-platform C++ entry point.
 */

#include "SimulationConfig.h"
#include "Utilities.h"

#include <iostream>
#include <string>

#if defined(ENV_SOC_COG_SPACE_PLATFORM_LINUX)
const char* platform_name = "Linux";
#elif defined(ENV_SOC_COG_SPACE_PLATFORM_MACOS)
const char* platform_name = "macOS";
#elif defined(ENV_SOC_COG_SPACE_PLATFORM_WINDOWS)
const char* platform_name = "Windows";
#else
const char* platform_name = "Unknown";
#endif

namespace {

/** Default path when run from the project root (see config/simulation.example.json). */
constexpr const char* kDefaultSimulationConfig = "config/simulation.example.json";

} // namespace

int main(int argc, char* argv[]) {
    std::cout << "env_soc_cog_space v0.1.0 - Environmental Social Cognitive Space\n";
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
        return 1;
    }

    const SimulationConfig& cfg = SimulationConfig::global();
    utilities::seedRng(cfg.random_seed);

    if (cfg.verbose) {
        std::cout << "Simulation config: version=" << cfg.version << " seed=" << cfg.random_seed
                  << " time_step=" << cfg.time_step << " max_steps=" << cfg.max_steps
                  << " growth_rate=[" << cfg.min_growth_rate_supported << ","
                  << cfg.max_growth_rate_supported << "]"
                  << " half_saturation=[" << cfg.min_half_saturation_constant_supported << ","
                  << cfg.max_half_saturation_constant_supported << "]\n";
    }

    return 0;
}
