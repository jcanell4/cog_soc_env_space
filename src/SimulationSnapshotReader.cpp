#include "SimulationSnapshotReader.h"
#include "JsonEnumNames.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

using nlohmann::json;

double readNumberOrDefault(const json& object, const char* key, double default_value = 0.0) {
    if (!object.contains(key) || object[key].is_null()) {
        return default_value;
    }
    if (!object[key].is_number()) {
        throw std::runtime_error(std::string("Expected numeric field: ") + key);
    }
    return object[key].get<double>();
}

int readIntOrDefault(const json& object, const char* key, int default_value = 0) {
    if (!object.contains(key) || object[key].is_null()) {
        return default_value;
    }
    if (!object[key].is_number_integer()) {
        throw std::runtime_error(std::string("Expected integer field: ") + key);
    }
    return object[key].get<int>();
}

int readClassTypeOrDefault(const json& object, const char* key, int default_value = -1) {
    if (!object.contains(key) || object[key].is_null()) {
        return default_value;
    }
    return json_enum_names::parseClassTypeValue(object[key], key);
}

std::string readStringOrDefault(const json& object, const char* key, const std::string& default_value = "") {
    if (!object.contains(key) || object[key].is_null()) {
        return default_value;
    }
    if (!object[key].is_string()) {
        throw std::runtime_error(std::string("Expected string field: ") + key);
    }
    return object[key].get<std::string>();
}

std::vector<double> readNumberArrayOrDefault(const json& object, const char* key) {
    std::vector<double> values;
    if (!object.contains(key) || object[key].is_null()) {
        return values;
    }
    if (!object[key].is_array()) {
        throw std::runtime_error(std::string("Expected numeric array field: ") + key);
    }
    for (const json& value : object[key]) {
        if (!value.is_number()) {
            throw std::runtime_error(std::string("Expected numeric element in array field: ") + key);
        }
        values.push_back(value.get<double>());
    }
    return values;
}

SimulationFrameData parseFrameData(const json& data_object, int elapsed_cycles) {
    if (!data_object.is_object()) {
        throw std::runtime_error("Frame data entry must be an object");
    }

    SimulationFrameData frame;
    frame.elapsed_cycles = std::max(0, elapsed_cycles);
    frame.nutrients = readNumberOrDefault(data_object, "nutrients");
    frame.total_energy = readNumberOrDefault(data_object, "total_energy");
    frame.ecological_health = readNumberOrDefault(data_object, "ecological_health");
    frame.living_biomass = readNumberOrDefault(data_object, "living_biomass");
    frame.death_biomass = readNumberOrDefault(data_object, "death_biomass");
    frame.decomposer_biomass = readNumberOrDefault(data_object, "decomposer_biomass");
    if (data_object.contains("biomass_by_class") && data_object["biomass_by_class"].is_object()) {
        const json& by_class = data_object["biomass_by_class"];
        frame.autotroph_biomass = readNumberOrDefault(by_class, "autotroph");
        frame.heterotroph_biomass = readNumberOrDefault(by_class, "heterotroph");
        frame.decomposer_biomass = readNumberOrDefault(by_class, "decomposer", frame.decomposer_biomass);
        frame.other_living_biomass = readNumberOrDefault(by_class, "other");
    }

    if (data_object.contains("cohorts") && data_object["cohorts"].is_array()) {
        for (const json& cohort_json : data_object["cohorts"]) {
            if (!cohort_json.is_object()) {
                continue;
            }
            CohortFrameData cohort;
            cohort.id = readIntOrDefault(cohort_json, "id", -1);
            cohort.specie_name = readStringOrDefault(cohort_json, "specie_name");
            cohort.energy = readNumberOrDefault(cohort_json, "energy");
            cohort.total_biomass = readNumberOrDefault(cohort_json, "total_biomass");
            cohort.total_death_biomass = readNumberOrDefault(cohort_json, "total_death_biomass");
            cohort.stage_biomass = readNumberArrayOrDefault(cohort_json, "biomass");

            if (cohort_json.contains("specie") && cohort_json["specie"].is_object()) {
                const json& specie_json = cohort_json["specie"];
                cohort.class_type = readClassTypeOrDefault(specie_json, "class_type", -1);
                cohort.class_name = readStringOrDefault(specie_json, "class_name");
            }
            frame.cohorts.push_back(std::move(cohort));
        }
    }

    return frame;
}

double lerp(double a, double b, double t) {
    return a + (b - a) * t;
}

}  // namespace

void SimulationSnapshotReader::load(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Unable to open snapshot JSON: " + path);
    }

    json root;
    input >> root;
    if (!root.is_object()) {
        throw std::runtime_error("Snapshot root must be a JSON object");
    }

    if (!root.contains("initial_data") || !root["initial_data"].is_object()) {
        throw std::runtime_error("Missing initial_data object in snapshot");
    }

    const json& initial_data = root["initial_data"];
    if (!initial_data.contains("data")) {
        throw std::runtime_error("Missing initial_data.data in snapshot");
    }

    std::vector<SimulationFrameData> loaded_frames;
    loaded_frames.push_back(parseFrameData(initial_data["data"], 0));

    if (root.contains("step_data")) {
        if (!root["step_data"].is_array()) {
            throw std::runtime_error("step_data must be an array");
        }
        for (const json& step_entry : root["step_data"]) {
            if (!step_entry.is_object() || !step_entry.contains("data")) {
                continue;
            }
            const int elapsed_cycles = readIntOrDefault(step_entry, "elapsed_cycles", 0);
            loaded_frames.push_back(parseFrameData(step_entry["data"], elapsed_cycles));
        }
    }

    if (loaded_frames.empty()) {
        throw std::runtime_error("Snapshot contains no frames");
    }
    frames_ = std::move(loaded_frames);
}

std::size_t SimulationSnapshotReader::frameCount() const {
    return frames_.size();
}

const SimulationFrameData& SimulationSnapshotReader::frameAt(std::size_t index) const {
    if (index >= frames_.size()) {
        throw std::out_of_range("Frame index out of range");
    }
    return frames_[index];
}

SimulationFrameData SimulationSnapshotReader::interpolate(double frame_position) const {
    if (frames_.empty()) {
        throw std::runtime_error("No frames loaded");
    }
    if (frames_.size() == 1U) {
        return frames_.front();
    }

    const double clamped = std::clamp(frame_position, 0.0, static_cast<double>(frames_.size() - 1U));
    const std::size_t left_index = static_cast<std::size_t>(std::floor(clamped));
    const std::size_t right_index = std::min(left_index + 1U, frames_.size() - 1U);
    const double t = clamped - static_cast<double>(left_index);

    const SimulationFrameData& left = frames_[left_index];
    const SimulationFrameData& right = frames_[right_index];

    SimulationFrameData out = (t < 0.5) ? left : right;
    out.elapsed_cycles = static_cast<int>(std::lround(lerp(static_cast<double>(left.elapsed_cycles), static_cast<double>(right.elapsed_cycles), t)));
    out.nutrients = lerp(left.nutrients, right.nutrients, t);
    out.total_energy = lerp(left.total_energy, right.total_energy, t);
    out.ecological_health = lerp(left.ecological_health, right.ecological_health, t);
    out.living_biomass = lerp(left.living_biomass, right.living_biomass, t);
    out.death_biomass = lerp(left.death_biomass, right.death_biomass, t);
    out.decomposer_biomass = lerp(left.decomposer_biomass, right.decomposer_biomass, t);
    out.autotroph_biomass = lerp(left.autotroph_biomass, right.autotroph_biomass, t);
    out.heterotroph_biomass = lerp(left.heterotroph_biomass, right.heterotroph_biomass, t);
    out.other_living_biomass = lerp(left.other_living_biomass, right.other_living_biomass, t);
    return out;
}
