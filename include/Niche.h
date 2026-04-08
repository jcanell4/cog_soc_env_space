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
    double getBiologicalPotentialPerSurfaceUnit() const;
    double getEcologicalHealth() const;
    double getNutrients() const;
    double getSubstrate() const;
    const CohortSet& getCohortSet() const;
    double getReturnRate() const;
    const std::vector<double>& getConditions() const;

    Niche& setSurface(double value);
    Niche& setBiologicalPotentialPerSurfaceUnit(double value);
    Niche& setEcologicalHealth(double value);
    Niche& setNutrients(double value);
    Niche& setSubstrate(double value);
    Niche& setCohortSet(CohortSet value);
    Niche& setReturnRate(double value);
    Niche& setConditions(std::vector<double> value);

    double getMaxBiologicalPotential() const;
    double getDeathBiomass() const;
    double getLivingBiomass() const;
    double getDecomposerBiomass() const;

    void update_nutrients(double dt);
    void update_ecological_health(double recycling_increment);
    void step(double dt);
    void initialize();

private:
    void update_cohorts();

    double surface_{0.0};
    double biological_potential_per_surface_unit_{0.0};
    double ecological_health_{1.0};
    double nutrients_{0.0};
    double substrate_{0.0};
    CohortSet cohort_set_;
    double return_rate_{0.0};
    std::vector<double> conditions_;
};
