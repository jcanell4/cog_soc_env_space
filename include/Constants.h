#pragma once

/**
 * @file Constants.h
 * @brief Shared numerical constants for growth-channel encoding.
 */

#include <limits>
#include <cstddef>
#include <string_view>

/**
 * @var NUTRIENTS_POS
 * @brief Sentinel int used as the first element of growth-demand tuples for nutrient-limited
 *        autotroph growth (distinct from cohort indices and negative decomposer codes).
 */
inline constexpr int NUTRIENTS_POS = std::numeric_limits<int>::max();

namespace DietType {
inline constexpr int NUTRIENTS_TYPE = NUTRIENTS_POS;
inline constexpr int CATABOLIC_TYPE = NUTRIENTS_POS - 1;
inline constexpr int PARENTAL_SUPPLY_TYPE = NUTRIENTS_POS - 2;
inline constexpr int HETEROTROPH_TYPE = NUTRIENTS_POS - 3;
}  // namespace DietType

/**
 * @brief Concrete @c LivingBeing subclass identifiers (see @ref LivingBeing::getClassType).
 */
namespace LivingBeingClassType {
inline constexpr int AUTOTROPH = 0;
inline constexpr int HETEROTROPH = 1;
inline constexpr int DECOMPOSER = 2;
}  // namespace LivingBeingClassType

namespace FoodType {
inline constexpr std::string_view LIVING_BEING = "0";
inline constexpr std::string_view VEGETABLE = "0.0";
inline constexpr std::string_view ANIMAL = "0.1";
}  // namespace FoodType
/** @brief Colony-induced effective occupied-surface gain when colony ability is 1.0. */
inline constexpr double COLONY_SURFACE_GAIN_ETA = 0.2;
/** @brief Colony-vs-individual mixing exponent in prey-find probability interpolation. */
inline constexpr double COLONY_MIX_GAMMA = 1.0;
/** @brief Sharpness of the movement-rate to scanned-surface exponential transform. */
inline constexpr double PROSPECTING_SCAN_SHARPNESS = 3.0;

/**
 * @brief Dead-biomass size-bin convention (dynamic-size vectors):
 *        index 0 is the finest/most degraded detritus class.
 */
inline constexpr std::size_t DEATH_BIOMASS_FINEST_BIN = 0;
