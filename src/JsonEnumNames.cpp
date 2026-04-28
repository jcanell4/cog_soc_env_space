#include "JsonEnumNames.h"

#include "Constants.h"

#include <nlohmann/json.hpp>

#include <array>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using nlohmann::json;

struct NamedCode {
    std::string_view name;
    int code;
};

constexpr std::array<NamedCode, 3> kClassTypeNames{{
    {"AUTOTROPH", LivingBeingClassType::AUTOTROPH},
    {"HETEROTROPH", LivingBeingClassType::HETEROTROPH},
    {"DECOMPOSER", LivingBeingClassType::DECOMPOSER},
}};

constexpr std::array<NamedCode, 4> kDietCohortIndexNames{{
    {"NUTRIENTS_TYPE", DietType::NUTRIENTS_TYPE},
    {"CATABOLIC_TYPE", DietType::CATABOLIC_TYPE},
    {"PARENTAL_SUPPLY_TYPE", DietType::PARENTAL_SUPPLY_TYPE},
    {"HETEROTROPH_TYPE", DietType::HETEROTROPH_TYPE},
}};

template <std::size_t N>
std::optional<int> lookupCodeByName(const std::array<NamedCode, N>& values, std::string_view name) {
    for (const NamedCode& item : values) {
        if (item.name == name) {
            return item.code;
        }
    }
    return std::nullopt;
}

template <std::size_t N>
std::optional<std::string_view> lookupNameByCode(const std::array<NamedCode, N>& values, int code) {
    for (const NamedCode& item : values) {
        if (item.code == code) {
            return item.name;
        }
    }
    return std::nullopt;
}

template <std::size_t N>
int parseNamedOrNumericValue(const json& value,
                             std::string_view field_path,
                             const std::array<NamedCode, N>& names) {
    if (value.is_number_integer()) {
        return value.get<int>();
    }
    if (value.is_string()) {
        const std::string name = value.get<std::string>();
        const std::optional<int> code = lookupCodeByName(names, name);
        if (!code.has_value()) {
            throw std::runtime_error("Unknown constant for " + std::string(field_path) + ": " + name);
        }
        return *code;
    }
    throw std::runtime_error("Expected integer or string constant for " + std::string(field_path));
}

template <std::size_t N>
json toJsonNamedOrNumeric(int code, const std::array<NamedCode, N>& names) {
    const std::optional<std::string_view> name = lookupNameByCode(names, code);
    if (name.has_value()) {
        return json(std::string(*name));
    }
    return json(code);
}

}  // namespace

int json_enum_names::parseClassTypeValue(const nlohmann::json& value, std::string_view field_path) {
    return parseNamedOrNumericValue(value, field_path, kClassTypeNames);
}

int json_enum_names::parseDietCohortIndexValue(const nlohmann::json& value, std::string_view field_path) {
    return parseNamedOrNumericValue(value, field_path, kDietCohortIndexNames);
}

nlohmann::json json_enum_names::classTypeToJson(int class_type) {
    return toJsonNamedOrNumeric(class_type, kClassTypeNames);
}

nlohmann::json json_enum_names::dietCohortIndexToJson(int cohort_index) {
    return toJsonNamedOrNumeric(cohort_index, kDietCohortIndexNames);
}
