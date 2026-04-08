#pragma once

/**
 * @file SensitivityLevel.h
 * @brief Ordinal sensitivity (e.g. to environmental conditions) as a scalar weight.
 *
 * C++ enumerators must be integral; use @ref sensitivityLevelScalar to obtain 0.5 / 0.3 / 0.15
 * (or 0.0 for @c UNSET).
 */

/**
 * @brief How tightly an entity is tied to a preferred regime (conditions, resources, etc.).
 */
enum class SensitivityLevel : int {
    UNSET = -1,       ///< Not set; @ref sensitivityLevelScalar returns 0.
    ADAPTABLE = 0,    ///< Broader tolerance (scalar weight 0.5).
    INTERMEDIATE = 1, ///< Moderate (scalar weight 0.3).
    SPECIALIZED = 2,  ///< Narrow tolerance (scalar weight 0.15).
};

/**
 * @brief Numeric weight associated with @p level in model equations.
 * @return 0.5 for ADAPTABLE, 0.3 for INTERMEDIATE, 0.15 for SPECIALIZED, 0.0 for UNSET.
 */
constexpr double sensitivityLevelScalar(SensitivityLevel level) noexcept {
    switch (level) {
        case SensitivityLevel::UNSET:
            return 0.0;
        case SensitivityLevel::ADAPTABLE:
            return 0.5;
        case SensitivityLevel::INTERMEDIATE:
            return 0.3;
        case SensitivityLevel::SPECIALIZED:
            return 0.15;
    }
    return 0.0;
}
