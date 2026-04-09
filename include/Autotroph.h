#pragma once

/**
 * @file Autotroph.h
 * @brief Minimal autotroph placeholder for the restart.
 */

#include "LivingBeing.h"

/**
 * @class Autotroph
 * @brief Reduced concrete species used as a temporary restart scaffold.
 */
class Autotroph : public LivingBeing {
public:
    Autotroph();

    int getFoodType() const override;

    std::vector<std::vector<std::size_t>> getDietByCohortIndex() const override;

    void initialize(const Niche& niche) override;
};
