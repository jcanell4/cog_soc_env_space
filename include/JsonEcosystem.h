#pragma once

/**
 * @file JsonEcosystem.h
 * @brief Helpers to serialize ecosystem runtime state to JSON snapshots.
 */

#include <nlohmann/json_fwd.hpp>

#include <string>

class Niche;
class Cohort;
class LivingBeing;
class Autotroph;
class Heterotroph;
class Decomposer;

class JsonEcosystem {
public:
    /**
     * @brief Creates the root JSON object with initial niche snapshot and empty step_data array.
     */
    static nlohmann::json createJson(const Niche& niche);

    /**
     * @brief Appends one niche snapshot entry to step_data using elapsed cycle index.
     */
    static void updateJson(const Niche& niche, int elapsed_cycles, nlohmann::json& root);

    /**
     * @brief Stores a JSON value to disk.
     * @return true on success, false when file cannot be opened or written.
     */
    static bool saveJsonToFile(const nlohmann::json& root, const std::string& output_path, int indent = 2);

private:
    static void updateJson(const Niche& niche, nlohmann::json& out);
    static void updateJson(const Cohort& cohort, nlohmann::json& out);
    static void updateJson(const LivingBeing& living_being, nlohmann::json& out);
    static void updateJson(const Autotroph& autotroph, nlohmann::json& out);
    static void updateJson(const Heterotroph& heterotroph, nlohmann::json& out);
    static void updateJson(const Decomposer& decomposer, nlohmann::json& out);
};
