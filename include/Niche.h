#pragma once

/**
 * @file Niche.h
 * @brief Minimal niche container for the restart.
 */

#include "Population.h"

#include <cstddef>
#include <vector>

class Niche {
public:
    using PopulationSet = std::vector<Population>;

    Niche() = default;

    double getSurface() const;
    double getEcologicalHealth() const;
    double getNutrients() const;
    const PopulationSet& getPopulationSet() const;
    PopulationSet& getPopulationSet();
    const std::vector<double>& getReturnRate() const;
    double getReturnRate(std::size_t index, double out_of_range_default = 0.0) const;
    const std::vector<double>& getConditions() const;
    double getConditions(std::size_t index, double out_of_range_default = 0.0) const;
    const std::vector<double>& getLimitingFactors() const;
    double getLimitingFactors(std::size_t index, double out_of_range_default = 0.0) const;
    double getProspectingScanSharpness() const;

    Niche& setSurface(double value);
    Niche& setEcologicalHealth(double value);
    Niche& setNutrients(double value);
    Niche& setPopulationSet(PopulationSet value);
    Niche& setReturnRate(std::vector<double> value);
    Niche& setConditions(std::vector<double> value);
    Niche& setLimitingFactors(std::vector<double> value);
    Niche& setProspectingScanSharpness(double value);

    double getDeathBiomass() const;
    double getLivingBiomass() const;
    double calculateEnergy() const;
    double getEnergy() const;
    double getAutotrophBiomass() const;
    double getHeterotrophBiomass() const;
    double getBiomassForDietIndex(std::vector<std::tuple<int, int, int, int>> diet_by_population_index) const;

    /**
     * @brief Per-height-stratum sum of living autotroph biomass.
     *        Each autotroph maps life-history stage -> stratum via @c Autotroph::getStratum();
     *        result index @a h is total biomass in stratum @a h.
     */
    std::vector<double> getAutotrophBiomassPerStratum() const;

    /**
     * @brief Per-stratum light transmission fractions after canopy shading.
     *        Shadow density is (sum of biomass×opacity per stratum) / surface.
     *        Top stratum (highest index) receives 1.0 incident light.
     *        For lower strata: percent[h] = exp(-density[h+1]) × percent[h+1], clamped to [0,1].
     */
    std::vector<double> getLithPerStratum() const;

    void update_nutrients();
    void update_ecological_health();
    void update_niche();
    void step();
    void initialize();

private:
    void update_populations();

    double surface_{0.0};
    double ecological_health_{1.0};
    double nutrients_{0.0};
    PopulationSet population_set_;
    std::vector<double> return_rate_{};
    double return_cost_{0.0};
    std::vector<double> conditions_;
    std::vector<double> limiting_factors_;
    double prospecting_scan_sharpness_{1.0};
};
