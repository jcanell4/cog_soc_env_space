#pragma once

/**
 * @file SimulationConfig.h
 * @brief Global simulation parameters loaded once from JSON at startup.
 */

#include <nlohmann/json_fwd.hpp>

#include <cstdint>
#include <string>

/**
 * @brief Immutable-after-load simulation settings (seed, timestep, growth / half-saturation bounds, verbosity).
 *
 * Load via @ref loadFromFile or @ref loadFromJson exactly once before @ref global.
 */
struct SimulationConfig {
    std::uint32_t version = 1;
    /** RNG seed; 0 means "not set" (callers may substitute a runtime default). */
    std::uint64_t random_seed = 0;
    /** Default stochastic noise standard deviation (legacy replacement for NOISE_STDDEV). */
    double noise_stddev = 0.2;
    /** Alias noise for growth / maintenance paths (legacy replacement for NOISE_STDV). */
    double noise_stdv = 0.2;
    /** Path to environment JSON snapshot/config file. */
    std::string environment_path;
    
    /**
     * @brief Load from UTF-8 JSON file.
     *
     * If the root object contains a @c "simulation" object, that sub-object is parsed;
     * otherwise the root is parsed as the config body (dedicated file).
     * @throws std::runtime_error if the file cannot be read, JSON is invalid, or values are invalid.
     * @throws std::runtime_error if called when config is already loaded.
     */
    static void loadFromFile(const std::string& path);

    /**
     * @brief Parse from a JSON value (object).
     * @throws std::runtime_error on invalid types or duplicate load.
     */
    static void loadFromJson(const nlohmann::json& root);

    /** @return Reference to the loaded config. @throws std::runtime_error if not loaded. */
    static const SimulationConfig& global();

    /** @return Whether @ref loadFromFile or @ref loadFromJson has completed successfully. */
    static bool isLoaded();

private:
    static SimulationConfig parseObject(const nlohmann::json& obj);
};
