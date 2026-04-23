#pragma once

/**
 * @file Heterotroph.h
 * @brief Minimal heterotroph placeholder for the restart.
 */

#include "ConsumerLivingBeing.h"

#include <string>
#include <vector>

class Niche;

class Heterotroph : public ConsumerLivingBeing {
public:
    Heterotroph();

    void initialize(const Niche& niche) override;

    /**
     * @brief Stage-level predation and growth update for heterotroph cohorts.
     *
     * The routine models prey finding, capture, and biomass transfers in three phases:
     * 1) Build theoretical captures across all prey cohorts/stages from encounter probability and
     *    recruitment-vs-defense compatibility, then enforce a global growth-limited ingestion cap.
     * 2) Apply handling-time reduction, subtract consumed biomass from prey cohorts, and route
     *    non-assimilated fractions to prey dead-biomass size bins.
     * 3) Optionally complement missing ingestion via parental supply (if diet contains
     *    @c DietType::PARENTAL_SUPPLY_TYPE), taking biomass proportionally from fertile donor stages,
     *    with stochastic correction to avoid deterministic maximum attainment every step.
     *
     * Final predator-stage biomass is updated as:
     * assimilated intake minus maintenance cost.
     */
    void process_individual_growth(Niche& niche, Cohort& cohort, int stage_index) const override;
    void process_reproductive_growth(Cohort& cohort,
                                     int stage_index,
                                     double stage_biomass_before_growth,
                                     double biomass_increment_this_cycle) const override;

    int getClassType() const override;

    void setCyclesPerStages(std::vector<int> cycles_per_stages) override;

    using ConsumerLivingBeing::getDietByFoodType;
    using ConsumerLivingBeing::setDietByFoodType;
    using ConsumerLivingBeing::isFoodTypeMyDiet;
    using ConsumerLivingBeing::getRangeForFoodType;

    const std::vector<double>& getSearchCaptureEfficiency() const;
    using ConsumerLivingBeing::getProspectingAbilityRate;
    using ConsumerLivingBeing::getHandlingTimePenalty;
    using ConsumerLivingBeing::getAssimilationEfficiency;
    using ConsumerLivingBeing::getIngestionResidueFractionBySize;

    Heterotroph& setName(std::string name);
    Heterotroph& setEnergyContent(float energy_content);
    Heterotroph& setSearchCaptureEfficiency(std::vector<double> values);
    Heterotroph& setProspectingAbilityRate(std::vector<double> values);
    Heterotroph& setHandlingTimePenalty(std::vector<double> values);
    Heterotroph& setAssimilationEfficiency(std::vector<double> values);
    Heterotroph& setIngestionResidueFractionBySize(std::vector<std::vector<double>> values);

    /**
     * @brief Fills @ref LivingBeing::diet_by_cohort_index_ from taxonomic diet rules and cohort species
     *        food types in @a niche (one tuple per cohort whose prey taxonomy matches a diet rule).
     */
    void rebuild_diet_by_cohort_index_from_food_type(const Niche& niche);

private:
    std::vector<double> search_capture_efficiency_;
};
