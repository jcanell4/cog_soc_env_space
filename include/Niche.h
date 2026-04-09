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
    double getDecomposerBiomass() const;

    void update_nutrients();
    void update_ecological_health();
    void step();
    void initialize();

private:
    void update_cohorts();

    double surface_{0.0};
    double ecological_health_{1.0};
    double nutrients_{0.0};
    CohortSet cohort_set_;
    std::vector<double> return_rate_{0.0, 0.0};
    double return_cost_{0.0};
    std::vector<double> conditions_;
    std::vector<double> limiting_factors_;
};
