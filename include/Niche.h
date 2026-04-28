#pragma once

/**
 * @file Niche.h
 * @brief Minimal niche container for the restart.
 */

#include "Cohort.h"

#include <vector>

class Niche {
public:
    using CohortSet = std::vector<Cohort>;

    Niche() = default;

    double getSurface() const;
    double getEcologicalHealth() const;
    double getNutrients() const;
    const CohortSet& getCohortSet() const;
    CohortSet& getCohortSet();
    const std::vector<double>& getReturnRate() const;
    const std::vector<double>& getConditions() const;
    const std::vector<double>& getLimitingFactors() const;

    Niche& setSurface(double value);
    Niche& setEcologicalHealth(double value);
    Niche& setNutrients(double value);
    Niche& setCohortSet(CohortSet value);
    Niche& setReturnRate(std::vector<double> value);
    Niche& setConditions(std::vector<double> value);
    Niche& setLimitingFactors(std::vector<double> value);

    double getDeathBiomass() const;
    double getLivingBiomass() const;
    double calculateEnergy() const;
    double getEnergy() const;
    double getAutotrophBiomass() const;
    double getHeterotrophBiomass() const;
    double getDecomposerBiomass() const;

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
    void update_cohorts();

    double surface_{0.0};
    double ecological_health_{1.0};
    double nutrients_{0.0};
    CohortSet cohort_set_;
    std::vector<double> return_rate_{};
    double return_cost_{0.0};
    std::vector<double> conditions_;
    std::vector<double> limiting_factors_;
};
