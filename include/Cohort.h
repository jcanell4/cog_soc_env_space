#pragma once

/**
 * @file Cohort.h
 * @brief Minimal cohort container for the restart.
 */

#include "LivingBeing.h"

#include <vector>

class Niche;

class Cohort {
public:
    Cohort() = default;

    const std::string& getSpecieName() const;
    double getEnergy() const;
    double calculateEnergy() const;
    const std::vector<double>& getBiomass() const;
    double getTotalBiomass() const;
    const std::vector<double>& getDeathBiomass() const;
    double getTotalDeathBiomass() const;
    bool isDecomposerCohort() const;

    Cohort& setSpecie(const LivingBeing& value);
    Cohort& setBiomass(std::vector<double> value);
    Cohort& setDeathBiomass(std::vector<double> value);

    void update_deaths(int stage);
    double decrement_death_biomass(std::vector<double> amounts);
    void update_step(const Niche& niche);
    void initialize(const Niche& niche);

    /**
     * @brief Updates living biomass at @a stage using @a specie_->getDietByCohortIndex()[stage] channels.
     * @param self_cohort_index Index of this cohort in @a niche.getCohortSet().
     */
    void update_individual_growth(Niche& niche, int self_cohort_index, int stage);

private:
    const LivingBeing* specie_{nullptr};
    std::vector<double> biomass_{0.0};
    std::vector<double> death_biomass_{0.0, 0.0};
};
