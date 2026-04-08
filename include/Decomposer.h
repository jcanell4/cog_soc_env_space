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
 * Growth demand uses negative tuple codes: donor cohort index @c d is encoded as @c -(d + 1)
 * (see @ref Niche::update_cohorts). The decomposer skips its own cohort index
 * @ref decomposer_cohort_index_.
 */
class Decomposer : public LivingBeing {
public:
    Decomposer();

    void initialize(const Niche& niche) override;

    const std::vector<double>& getDonorEfficiency() const;
    std::size_t getDecomposerCohortIndex() const;
    double getMaxDecompositionRate() const;

    bool isDecomposer() const override;

    Decomposer& setName(std::string name);
    Decomposer& setEnergyContent(float energy_content);
    Decomposer& setDonorEfficiency(std::vector<double> values);
    Decomposer& setDecomposerCohortIndex(std::size_t value);
    Decomposer& setMaxDecompositionRate(double value);

    double calculate_death_biomass(double total_biomass, double accepted_growth) const override;

    /**
     * @brief Chooses the donor cohort with the largest feasible uptake from @c death_biomass.
     * @return One tuple @c (-(donor_index + 1), amount), or empty if no valid donor.
     */
    std::vector<std::tuple<int, double>> calculate_growth_biomass(const Niche& niche,
                                                                  double cohort_biomass) const override;

private:
    static std::vector<double> clamp_unit_interval(std::vector<double> v);

    std::vector<double> donor_efficiency_;
    std::size_t decomposer_cohort_index_{0};
    double max_decomposition_rate_{0.15};
};
