#include "SimulationConfig.h"

#include <nlohmann/json.hpp>

#include <cmath>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>

namespace {

std::unique_ptr<const SimulationConfig> g_config;

void ensureNotLoaded() {
    if (g_config) {
        throw std::runtime_error("Simulation config: already loaded; load only once at startup");
    }
}

} // namespace

SimulationConfig SimulationConfig::parseObject(const nlohmann::json& obj) {
    if (!obj.is_object()) {
        throw std::runtime_error("Simulation config: expected JSON object");
    }

    SimulationConfig out;
    out.version = obj.value("version", 1);
    if (out.version < 1) {
        throw std::runtime_error("Simulation config: \"version\" must be >= 1");
    }

    if (obj.contains("random_seed")) {
        const auto& v = obj["random_seed"];
        if (v.is_number_unsigned()) {
            out.random_seed = v.get<std::uint64_t>();
        } else if (v.is_number_integer()) {
            const auto i = v.get<std::int64_t>();
            if (i < 0) {
                throw std::runtime_error("Simulation config: \"random_seed\" must be non-negative");
            }
            out.random_seed = static_cast<std::uint64_t>(i);
        } else {
            throw std::runtime_error("Simulation config: \"random_seed\" must be a non-negative integer");
        }
    }

    if (obj.contains("total_cycles")) {
        const auto& v = obj["total_cycles"];
        if (!v.is_number_integer()) {
            throw std::runtime_error("Simulation config: \"total_cycles\" must be an integer");
        }
        const auto i = v.get<std::int64_t>();
        if (i < 0) {
            throw std::runtime_error("Simulation config: \"total_cycles\" must be non-negative");
        }
        if (i > static_cast<std::int64_t>(std::numeric_limits<int>::max())) {
            throw std::runtime_error("Simulation config: \"total_cycles\" is too large");
        }
        out.total_cycles = static_cast<int>(i);
    }

    bool has_noise_stddev = false;
    bool has_noise_stdv = false;
    if (obj.contains("noise_stddev")) {
        const auto& v = obj["noise_stddev"];
        if (!v.is_number()) {
            throw std::runtime_error("Simulation config: \"noise_stddev\" must be a number");
        }
        out.noise_stddev = v.get<double>();
        if (out.noise_stddev < 0.0) {
            throw std::runtime_error("Simulation config: \"noise_stddev\" must be non-negative");
        }
        has_noise_stddev = true;
    }
    if (obj.contains("noise_stdv")) {
        const auto& v = obj["noise_stdv"];
        if (!v.is_number()) {
            throw std::runtime_error("Simulation config: \"noise_stdv\" must be a number");
        }
        out.noise_stdv = v.get<double>();
        if (out.noise_stdv < 0.0) {
            throw std::runtime_error("Simulation config: \"noise_stdv\" must be non-negative");
        }
        has_noise_stdv = true;
    }
    if (has_noise_stddev && !has_noise_stdv) {
        out.noise_stdv = out.noise_stddev;
    } else if (!has_noise_stddev && has_noise_stdv) {
        out.noise_stddev = out.noise_stdv;
    } else if (has_noise_stddev && has_noise_stdv &&
               std::fabs(out.noise_stddev - out.noise_stdv) > 1e-12) {
        throw std::runtime_error(
            "Simulation config: \"noise_stddev\" and \"noise_stdv\" must be equal when both are provided");
    }

    if (obj.contains("environment_path")) {
        const auto& v = obj["environment_path"];
        if (!v.is_string()) {
            throw std::runtime_error("Simulation config: \"environment_path\" must be a string");
        }
        out.environment_path = v.get<std::string>();
    }
    if (out.environment_path.empty()) {
        throw std::runtime_error("Simulation config: \"environment_path\" must be a non-empty string");
    }

    return out;
}

void SimulationConfig::loadFromJson(const nlohmann::json& root) {
    ensureNotLoaded();

    const nlohmann::json* body = &root;
    if (root.is_object() && root.contains("simulation_config")) {
        const auto& sub = root["simulation_config"];
        if (!sub.is_object()) {
            throw std::runtime_error("Simulation config: \"simulation_config\" must be an object");
        }
        body = &sub;
    }

    SimulationConfig parsed = parseObject(*body);
    g_config = std::unique_ptr<const SimulationConfig>(
        new SimulationConfig(std::move(parsed)));
}

void SimulationConfig::loadFromFile(const std::string& path) {
    ensureNotLoaded();

    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Simulation config: cannot open file: " + path);
    }

    nlohmann::json root;
    try {
        in >> root;
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("Simulation config: JSON parse error: ") + e.what());
    }

    loadFromJson(root);
}

const SimulationConfig& SimulationConfig::global() {
    if (!g_config) {
        throw std::runtime_error("Simulation config: not loaded; call loadFromFile first");
    }
    return *g_config;
}

bool SimulationConfig::isLoaded() {
    return static_cast<bool>(g_config);
}
