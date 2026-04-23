#pragma once

/**
 * @file Decomposer.h
 * @brief Decomposer species: uptake from other cohorts' dead biomass pools.
 */

#include "ConsumerLivingBeing.h"

#include <string>
#include <vector>

class Niche;

/**
 * @class Decomposer
 * @brief Decomposer species: uptake from other cohorts' dead biomass pools.
 *
 * Decomposition uptake follows the same two-pass structure as @c Heterotroph (theory → global cap α →
 * handling → assimilation with residue routing to donor death bins; optional parental supply).
 *
 * Dead-biomass transfers are modeled via niche/cohort updates (not via species callbacks in this restart).
 */
class Decomposer : public ConsumerLivingBeing {
public:
    Decomposer();

    void initialize(const Niche& niche) override;

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

    /**
     * @brief Fills @ref LivingBeing::diet_by_cohort_index_ with tuples
     *        @c (donor_cohort_index, min_dead_bin, max_dead_bin inclusive). Taxonomic rules from
     *        @ref ConsumerLivingBeing::diet_by_food_type_ supply bin ranges via the same hierarchy match
     *        as prey diets (interpreted as dead-matter size-bin indices for decomposers).
     *        If @ref ConsumerLivingBeing::diet_by_food_type_ is empty, the decomposer is a generalist:
     *        every cohort with a species is linked with donor bins @c 0 … @c max_bin (from death/traits size).
     */
    void rebuild_diet_by_cohort_index_from_food_type(const Niche& niche);

    using ConsumerLivingBeing::getProspectingAbilityRate;
    using ConsumerLivingBeing::getHandlingTimePenalty;
    using ConsumerLivingBeing::getAssimilationEfficiency;
    using ConsumerLivingBeing::getIngestionResidueFractionBySize;

    Decomposer& setName(std::string name);
    Decomposer& setEnergyContent(float energy_content);
    Decomposer& setProspectingAbilityRate(std::vector<double> values);
    Decomposer& setHandlingTimePenalty(std::vector<double> values);
    Decomposer& setAssimilationEfficiency(std::vector<double> values);
    Decomposer& setIngestionResidueFractionBySize(std::vector<std::vector<double>> values);
};
