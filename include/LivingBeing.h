#pragma once

/**
 * @file LivingBeing.h
 * @brief Minimal base class for the ecological domain restart.
 */

#include "Constants.h"

#include <string>
#include <tuple>
#include <vector>

class Niche;
class Cohort;

/**
 * @class LivingBeing
 * @brief Small shared state holder for future species implementations.
 *
 * This restart version intentionally keeps only identity, basic numeric attributes,
 * and no-op virtual hooks that can be specialized later.
 */
class LivingBeing {
public:
    LivingBeing() = default;
    explicit LivingBeing(std::string name, float biomass_to_energy_conversion_factor_ = 50.0f);
    virtual ~LivingBeing();

    virtual void initialize(const Niche& niche);
    const std::string& getFoodType() const;
    void setFoodType(std::string food_type);

    /** @brief Runtime subclass tag; compare to @c LivingBeingClassType::* in Constants.h. */
    virtual int getClassType() const = 0;

    /**
     * @brief Cohort diet links grouped by consumer stage.
     *        Outer index is consumer stage; each inner tuple is (source_cohort_index, min_stage, max_stage inclusive).
     *        For heterotrophs, @a min_stage/@a max_stage are prey life-history stages; for decomposers, donor dead-biomass
     *        size-bin indices. Empty per-stage vector means no cohort-indexed source for that stage.
     */
    const std::vector<std::vector<std::tuple<int, int, int>>>& getDietByCohortIndex() const;
    void setDietByCohortIndex(std::vector<std::vector<std::tuple<int, int, int>>> diet_by_cohort_index);

    const std::string& getName() const;
    float getBiomassToEnergyConversionFactor() const;
    float getDeathBiomassToEnergyConversionFactor() const;
    const std::vector<double>& getMaintenanceCost() const;
    const std::vector<double>& getMaxFertility() const;
    const std::vector<double>& getResilience() const;
    double getVulnerability() const;
    const std::vector<double>& getBiomassPerIndividualAmount() const;
    const std::vector<double>& getIndividualOccupiedSurface() const;
    /**
     * @brief Per-species descriptors of dead matter by size class.
     *        Outer index is dead-biomass bin (same order as Cohort::death_biomass_);
     *        each row stores chemical/physical/structural traits for that bin.
     */
    const std::vector<std::vector<double>>& getCharacteristicsDeathBiomass() const;
    /**
     * @brief Per-stage dead-matter size distribution.
     *        Row i corresponds to life stage i and contains bin proportions that sum to 1.
     */
    const std::vector<std::vector<double>>& getDeathBiomassFractionBySize() const;
    const std::vector<std::vector<double>>& getBestEnvironmentalConditions() const;
    const std::vector<int>& getCyclesPerStages() const;
    const std::vector<std::vector<double>>& getDefenseStrategies() const;
    const std::vector<std::vector<double>>& getRecruitmentStrategies() const;
    /** @brief Per-stage cap on individual growth; each component in [0,1]. */
    const std::vector<double>& getMaxIndividualGrowth() const;
    /** @brief Capacity to form colonies and colony size tendency; clamped to [0,1]. */
    double getColonyAbilityRate() const;

    void setName(std::string name);
    void setBiomassToEnergyConversionFactor(float energy_content);
    void setDeathBiomassToEnergyConversionFactor(float energy_content);
    void setMaintenanceCost(std::vector<double> maintenance_cost);
    void setMaxFertility(std::vector<double> max_fertility);
    void setResilience(std::vector<double> resilience);
    void setBiomassPerIndividualAmount(std::vector<double> biomass_per_individual_amount);
    void setIndividualOccupiedSurface(std::vector<double> individual_occupied_surface);
    void setCharacteristicsDeathBiomass(std::vector<std::vector<double>> characteristics_death_biomass);
    void setDeathBiomassFractionBySize(std::vector<std::vector<double>> death_biomass_fraction_by_size);
    void setBestEnvironmentalConditions(std::vector<std::vector<double>> best_environmental_conditions);
    virtual void setCyclesPerStages(std::vector<int> cycles_per_stages);
    void setDefenseStrategies(std::vector<std::vector<double>> defense_strategies);
    void setRecruitmentStrategies(std::vector<std::vector<double>> recruitment_strategies);
    void setMaxIndividualGrowth(std::vector<double> max_individual_growth);
    void setColonyAbilityRate(double colony_ability_rate);

    /**
     * @brief Stage index i using relative durations in cycles_per_stages_: stage i covers
     *        [sum(cycles_per_stages_[0..i-1]), sum(cycles_per_stages_[0..i])) (half-open).
     * @return Index i, or -1 if empty or cycles_elapsed is outside the total span.
     */
    int calculateStage(int cycles_elapsed) const;

    /**
     * @brief Redistributes cohort living biomass across life-history stages (mass moved only between stage bins).
     * @param cohort Cohort bound to this species; must have matching @c getSpecie() == this.
     * @param elapsed_cycles Monotonic cohort age in simulation cycles (typically incremented once per @c update_step).
     *        Uses proportional transfer each cycle: stage i moves roughly @c 1/cycles_per_stages_[i]
     *        to stage i+1 with a small stochastic perturbation.
     */
    virtual void updateStages(Cohort& cohort, int elapsed_cycles) const;

    /**
     * @brief For each index i up to max(size), e_i = max(0, min(1, 1 - (D_i - R_i))) with D_i, R_i from the
     *        vectors; missing entries in the shorter vector are treated as 0. E_eff = product of e_i (1.0 if both empty).
     */
    static double calculate_effective_recruitment_efficiency(
        const std::vector<double>& recruitment_strategies,
        const std::vector<double>& defense_strategies);

    /**
     * @brief Normalized L2 distance between two [0,1] vectors: ||current_conditions-best_conditions||_2 / sqrt(n),
     *        with n = max(size). Shorter vector is padded with 0; returns 0 if both empty.
     */
    static double calculateVulnerability(const std::vector<double>& current_conditions,
                                         const std::vector<double>& best_conditions);

    /**
     * @brief Biomass gained from prey cohort @a cohort_index at stage @a stage_index (heterotroph path).
     *        Default: no uptake; specialized later.
     */
    virtual double calculateObtainedBiomassIncrement(const Niche& niche,
                                                     int cohort_index,
                                                     int stage_index) const;

    /**
     * @brief Per-stage individual growth for this species on the given cohort.
     * @param stage_index Life-history stage to process.
     */
    virtual void process_individual_growth(Niche& niche, Cohort& cohort, int stage_index) const;

    /**
     * @brief Reproductive transfer from the current stage to stage 0 after individual growth.
     * @param stage_biomass_before_growth Stage biomass right before @ref process_individual_growth is applied.
     * @param biomass_increment_this_cycle Net biomass increment at @p stage_index caused by the growth step.
     */
    virtual void process_reproductive_growth(Cohort& cohort,
                                             int stage_index,
                                             double stage_biomass_before_growth,
                                             double biomass_increment_this_cycle) const;

protected:
    std::string name_;
    std::string food_type_{std::string{FoodType::LIVING_BEING}};
    float biomass_to_energy_conversion_factor_{50.0f};
    float death_biomass_to_energy_conversion_factor_{19.5f};
    std::vector<double> maintenance_cost_;
    std::vector<double> max_fertility_;
    std::vector<double> resilience_;
    std::vector<double> biomass_per_individual_amount_;
    std::vector<double> individual_occupied_surface_;
    std::vector<std::vector<double>> characteristics_death_biomass_;
    std::vector<std::vector<double>> death_biomass_fraction_by_size_;
    std::vector<std::vector<double>> best_environmental_conditions_;
    /** Relative number of cycles spent in each stage; e.g. {3,10,15} => stage 0 on [0,3), 1 on [3,13), 2 on [13,28). */
    std::vector<int> cycles_per_stages_;
    std::vector<std::vector<double>> defense_strategies_;
    std::vector<std::vector<double>> recruitment_strategies_;
    std::vector<double> max_individual_growth_;
    double colony_ability_rate_{0.0};
    std::vector<std::vector<std::tuple<int, int, int>>> diet_by_cohort_index_{};
    double vulnerability_{0.0};
    bool initialized_{false};
};
