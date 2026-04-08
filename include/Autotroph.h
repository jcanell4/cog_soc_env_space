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

    Autotroph& setName(std::string name);
    Autotroph& setEnergyContent(float energy_content);

    double getSubstrate() const;
    Autotroph& setSubstrate(double substrate);

    double getFactorConditions() const;
    double getFactorSubtrate() const;
    double getFactorNutrients() const;
    double getGrowthEffectiveRate() const;
    double getMaxGrowthRate() const;

    void initialize(const Niche& niche) override;
    double calculate_death_biomass(double total_biomass, double accepted_growth) const override;
    std::vector<std::tuple<int, double>> calculate_growth_biomass(
        const Niche& niche,
        double cohort_biomass) const override;

private:
    double substrate_{0.0};
    double factor_conditions_{1.0};
    double factor_subtrate_{1.0};
    double factor_nutrients_{1.0};
};
