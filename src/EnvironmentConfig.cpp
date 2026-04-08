#include "EnvironmentConfig.h"

#include <fstream>

LoadedEnvironment loadEnvironmentFromJson(const nlohmann::json&) {
    LoadedEnvironment out;
    return out;
}

LoadedEnvironment loadEnvironmentFromJsonFile(const std::string& path) {
    std::ifstream in(path);
    (void)in;
    (void)path;
    return {};
}
