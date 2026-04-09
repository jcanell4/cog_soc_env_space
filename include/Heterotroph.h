#pragma once

/**
 * @file Heterotroph.h
 * @brief Minimal heterotroph placeholder for the restart.
 */

#include "LivingBeing.h"

#include <cstddef>
#include <vector>

class Niche;

class Heterotroph : public LivingBeing {
public:
    Heterotroph();

    void initialize(const Niche& niche) override;

    int getFoodType() const override;

    std::vector<std::vector<std::size_t>> getDietByCohortIndex() const override;

    const std::vector<double>& getSearchCaptureEfficiency() const;
    const std::vector<double>& getHandlingTimePenalty() const;
    std::size_t getPredatorCohortIndex() const;
    double getMaxIngestionRate() const;
    double getFactorConditions() const;
    double getFactorResources() const;

    Heterotroph& setName(std::string name);
    Heterotroph& setEnergyContent(float energy_content);
    Heterotroph& setSearchCaptureEfficiency(std::vector<double> values);
    Heterotroph& setHandlingTimePenalty(std::vector<double> values);
    Heterotroph& setPredatorCohortIndex(std::size_t value);
    Heterotroph& setMaxIngestionRate(double value);

    void update_factor_resources(const Niche& niche);

private:
    static std::vector<double> clamp_unit_interval(std::vector<double> values);

    double factor_conditions_{1.0};
    double factor_resources_{1.0};
    std::vector<double> search_capture_efficiency_;
    std::vector<double> handling_time_penalty_;
    std::size_t predator_cohort_index_{0};
    double max_ingestion_rate_{0.0};
};
