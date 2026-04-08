#include "SimulationConfig.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <memory>
#include <optional>
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

    if (obj.contains("time_step")) {
        const auto& v = obj["time_step"];
        if (!v.is_number()) {
            throw std::runtime_error("Simulation config: \"time_step\" must be a number");
        }
        out.time_step = v.get<double>();
        if (!(out.time_step > 0.0)) {
            throw std::runtime_error("Simulation config: \"time_step\" must be positive");
        }
    }

    if (obj.contains("max_steps")) {
        const auto& v = obj["max_steps"];
        if (!v.is_number_unsigned() && !v.is_number_integer()) {
            throw std::runtime_error("Simulation config: \"max_steps\" must be a non-negative integer");
        }
        if (v.is_number_integer()) {
            const auto i = v.get<std::int64_t>();
            if (i < 0) {
                throw std::runtime_error("Simulation config: \"max_steps\" must be non-negative");
            }
            out.max_steps = static_cast<std::uint64_t>(i);
        } else {
            out.max_steps = v.get<std::uint64_t>();
        }
    }

    if (obj.contains("verbose")) {
        const auto& v = obj["verbose"];
        if (!v.is_boolean()) {
            throw std::runtime_error("Simulation config: \"verbose\" must be a boolean");
        }
        out.verbose = v.get<bool>();
    }

    std::optional<double> omin;
    std::optional<double> omax;
    if (obj.contains("min_growth_rate_supported")) {
        const auto& v = obj["min_growth_rate_supported"];
        if (!v.is_number()) {
            throw std::runtime_error("Simulation config: \"min_growth_rate_supported\" must be a number");
        }
        omin = v.get<double>();
    }
    if (obj.contains("max_growth_rate_supported")) {
        const auto& v = obj["max_growth_rate_supported"];
        if (!v.is_number()) {
            throw std::runtime_error("Simulation config: \"max_growth_rate_supported\" must be a number");
        }
        omax = v.get<double>();
    }
    if (omin && omax) {
        out.min_growth_rate_supported = *omin;
        out.max_growth_rate_supported = *omax;
    } else if (omin && !omax) {
        out.min_growth_rate_supported = *omin;
        out.max_growth_rate_supported = *omin;
    } else if (!omin && omax) {
        out.min_growth_rate_supported = *omax;
        out.max_growth_rate_supported = *omax;
    }
    if (out.min_growth_rate_supported > out.max_growth_rate_supported) {
        throw std::runtime_error(
            "Simulation config: min_growth_rate_supported must be <= max_growth_rate_supported");
    }

    std::optional<double> hmin;
    std::optional<double> hmax;
    if (obj.contains("min_half_saturation_constant_supported")) {
        const auto& v = obj["min_half_saturation_constant_supported"];
        if (!v.is_number()) {
            throw std::runtime_error(
                "Simulation config: \"min_half_saturation_constant_supported\" must be a number");
        }
        hmin = v.get<double>();
    }
    if (obj.contains("max_half_saturation_constant_supported")) {
        const auto& v = obj["max_half_saturation_constant_supported"];
        if (!v.is_number()) {
            throw std::runtime_error(
                "Simulation config: \"max_half_saturation_constant_supported\" must be a number");
        }
        hmax = v.get<double>();
    }
    if (hmin && hmax) {
        out.min_half_saturation_constant_supported = *hmin;
        out.max_half_saturation_constant_supported = *hmax;
    } else if (hmin && !hmax) {
        out.min_half_saturation_constant_supported = *hmin;
        out.max_half_saturation_constant_supported = *hmin;
    } else if (!hmin && hmax) {
        out.min_half_saturation_constant_supported = *hmax;
        out.max_half_saturation_constant_supported = *hmax;
    }
    if (out.min_half_saturation_constant_supported > out.max_half_saturation_constant_supported) {
        throw std::runtime_error(
            "Simulation config: min_half_saturation_constant_supported must be <= "
            "max_half_saturation_constant_supported");
    }

    return out;
}

void SimulationConfig::loadFromJson(const nlohmann::json& root) {
    ensureNotLoaded();

    const nlohmann::json* body = &root;
    if (root.is_object() && root.contains("simulation")) {
        const auto& sub = root["simulation"];
        if (!sub.is_object()) {
            throw std::runtime_error("Simulation config: \"simulation\" must be an object");
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
