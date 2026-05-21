#pragma once

/**
 * @file Autotroph.h
 * @brief Minimal autotroph placeholder for the restart.
 */

#include "LivingBeing.h"

#include <cstddef>
#include <vector>

/**
 * @class Autotroph
 * @brief Reduced concrete species used as a temporary restart scaffold.
 */
class Autotroph : public LivingBeing {
public:
    Autotroph();

    int getClassType() const override;

    void setCyclesPerStages(std::vector<int> cycles_per_stages) override;

    void initialize(const Niche& niche) override;

    void process_individual_growth(Niche& niche, Population& population, int stage_index) const override;
    void process_reproductive_growth(Population& population,
                                     int stage_index,
                                     double stage_biomass_before_growth,
                                     double biomass_increment_this_cycle) const override;

    const std::vector<double>& getOpacity() const;
    double getOpacity(std::size_t index, double out_of_range_default = 0.0) const;
    void setOpacity(std::vector<double> value);

    const std::vector<int>& getStratum() const;
    int getStratum(std::size_t index, int out_of_range_default = 0) const;
    void setStratum(std::vector<int> value);

    /** Minimum light required at each life-history stage for photosynthesis (same indexing as population biomass). */
    const std::vector<double>& getMinLight() const;
    double getMinLight(std::size_t index, double out_of_range_default = 0.0) const;
    void setMinLight(std::vector<double> value);
    /** @brief Seed dispersal capacity through the niche; clamped to [0,1]. */
    double getSeedDispersalRate() const;
    void setSeedDispersalRate(double value);

private:
    std::vector<double> opacity_;
    std::vector<int> stratum_;
    std::vector<double> min_light_;
    double seed_dispersal_rate_{0.0};
};
