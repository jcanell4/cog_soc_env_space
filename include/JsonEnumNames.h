#pragma once

#include <nlohmann/json_fwd.hpp>

#include <string_view>

namespace json_enum_names {

/**
 * @brief Parse class_type JSON value supporting integer and strict constant names.
 * @throws std::runtime_error on invalid type or unknown string constant.
 */
int parseClassTypeValue(const nlohmann::json& value, std::string_view field_path);

/**
 * @brief Parse diet cohort_index JSON value supporting integer and strict constant names.
 * @throws std::runtime_error on invalid type or unknown string constant.
 */
int parseDietCohortIndexValue(const nlohmann::json& value, std::string_view field_path);

/**
 * @brief Serialize class type using constant name when known, numeric literal otherwise.
 */
nlohmann::json classTypeToJson(int class_type);

/**
 * @brief Serialize diet cohort index using constant name when known, numeric literal otherwise.
 */
nlohmann::json dietCohortIndexToJson(int cohort_index);

}  // namespace json_enum_names
