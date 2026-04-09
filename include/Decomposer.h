#pragma once

/**
 * @file Decomposer.h
 * @brief Decomposer species: uptake from other cohorts' dead biomass pools.
 */

#include "LivingBeing.h"

#include <cstddef>
#include <vector>

class Niche;

/**
 * @class Decomposer
 * @brief Decomposer with per-donor efficiency on death biomass, fixed niche cohort index.
 *
 * Dead-biomass uptake is modeled via niche/cohort updates (not via species callbacks in this restart).
 */
class Decomposer : public LivingBeing {
public:
    Decomposer();

    void initialize(const Niche& niche) override;

    int getFoodType() const override;

    int getClassType() const override;

    std::vector<std::vector<std::size_t>> getDietByCohortIndex() const override;

    const std::vector<double>& getDonorEfficiency() const;
    std::size_t getDecomposerCohortIndex() const;
    double getMaxDecompositionRate() const;

    Decomposer& setName(std::string name);
    Decomposer& setEnergyContent(float energy_content);
    Decomposer& setDonorEfficiency(std::vector<double> values);
    Decomposer& setDecomposerCohortIndex(std::size_t value);
    Decomposer& setMaxDecompositionRate(double value);

private:
    static std::vector<double> clamp_unit_interval(std::vector<double> v);

    std::vector<double> donor_efficiency_;
    std::size_t decomposer_cohort_index_{0};
    double max_decomposition_rate_{0.15};
};
