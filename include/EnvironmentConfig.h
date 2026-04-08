#pragma once

/**
 * @file EnvironmentConfig.h
 * @brief Restart placeholder for environment loading.
 */

#include "Autotroph.h"
#include "Environment.h"

#include <nlohmann/json_fwd.hpp>

#include <string>
#include <vector>

struct LoadedEnvironment {
    std::vector<Autotroph> species;
    Environment environment;
};

LoadedEnvironment loadEnvironmentFromJson(const nlohmann::json& root);
LoadedEnvironment loadEnvironmentFromJsonFile(const std::string& path);
