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
    /** Nominal integration step length (simulation units). */
    double time_step = 1.0;
    /** Upper bound on step count; 0 means no fixed cap (policy for the main loop). */
    std::uint64_t max_steps = 0;
    /** Global lower bound for supported growth rate (model units; used by growth logic). */
    double min_growth_rate_supported = 0.0;
    /** Global upper bound for supported growth rate (must be >= @ref min_growth_rate_supported when both set from JSON). */
    double max_growth_rate_supported = 1.0;
    /** Global lower bound for supported half-saturation constant (e.g. Michaelis–Menten / Monod K). */
    double min_half_saturation_constant_supported = 0.03;
    /** Global upper bound for supported half-saturation constant (must be >= @ref min_half_saturation_constant_supported). */
    double max_half_saturation_constant_supported = 8.0;
    bool verbose = false;

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
