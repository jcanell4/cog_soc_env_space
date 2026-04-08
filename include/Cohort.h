#pragma once

/**
 * @file Cohort.h
 * @brief Minimal cohort container for the restart.
 */

#include "LivingBeing.h"

#include <tuple>
#include <vector>

class Niche;

class Cohort {
public:
    Cohort() = default;

    const std::string& getSpecieName() const;
    double getEnergy() const;
    double calculateEnergy() const;
    double getBiomass() const;
    double getDeathBiomass() const;
    bool isDecomposerCohort() const;

    Cohort& setSpecie(const LivingBeing& value);
    Cohort& setBiomass(double value);
    Cohort& setDeathBiomass(double value);

    void update_biomass(double accepted_growth);
    void update_deaths(double deaths);
    double decrement_death_biomass(double amount);
    std::vector<std::tuple<int, double>> calculate_growth_demand(const Niche& niche) const;
    void initialize(const Niche& niche);

private:
    const LivingBeing* specie_{nullptr};
    double biomass_{0.0};
    double death_biomass_{0.0};
};
