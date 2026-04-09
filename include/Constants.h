#pragma once

/**
 * @file Constants.h
 * @brief Shared numerical constants for growth-channel encoding.
 */

#include <limits>

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
inline constexpr int DECOMPOSER_TYPE = NUTRIENTS_POS - 4;
}  // namespace DietType

/**
 * @brief Concrete @c LivingBeing subclass identifiers (see @ref LivingBeing::getClassType).
 */
namespace LivingBeingClassType {
inline constexpr int AUTOTROPH = 0;
inline constexpr int HETEROTROPH = 1;
inline constexpr int DECOMPOSER = 2;
}  // namespace LivingBeingClassType

/** @brief Default stochastic noise standard deviation for nutrient return processing. */
inline constexpr double NOISE_STDDEV = 0.2;
/** @brief Alias for growth / maintenance noise (same as @ref NOISE_STDDEV). */
inline constexpr double NOISE_STDV = NOISE_STDDEV;
