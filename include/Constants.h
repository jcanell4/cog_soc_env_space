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
