#pragma once

/**
 * @file Autotroph.h
 * @brief Minimal autotroph placeholder for the restart.
 */

#include "LivingBeing.h"

#include <vector>

/**
 * @class Autotroph
 * @brief Reduced concrete species used as a temporary restart scaffold.
 */
class Autotroph : public LivingBeing {
public:
    Autotroph();

    int getFoodType() const override;

    int getClassType() const override;

    std::vector<std::vector<std::size_t>> getDietByCohortIndex() const override;

    void initialize(const Niche& niche) override;

    const std::vector<double>& getOpacity() const;
    void setOpacity(std::vector<double> value);

    const std::vector<int>& getStratum() const;
    void setStratum(std::vector<int> value);

    const std::vector<double>& getMaxDensity() const;
    void setMaxDensity(std::vector<double> value);

    /** Minimum light required at each life-history stage for photosynthesis (same indexing as cohort biomass). */
    const std::vector<double>& getMinLight() const;
    void setMinLight(std::vector<double> value);

private:
    std::vector<double> opacity_;
    std::vector<int> stratum_;
    std::vector<double> max_density_;
    std::vector<double> min_light_;
};
