#pragma once

/**
 * @file LivingBeing.h
 * @brief Minimal base class for the ecological domain restart.
 */

#include <cstddef>
#include <string>
#include <vector>

class Niche;

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
    explicit LivingBeing(std::string name, float biomass_to_energy_conversion_factor_ = 0.0f);
    virtual ~LivingBeing();

    virtual void initialize(const Niche& niche);
    virtual int getFoodType() const = 0;

    /** @brief Runtime subclass tag; compare to @c LivingBeingClassType::* in Constants.h. */
    virtual int getClassType() const = 0;

    /**
     * @brief Per-stage cohort diet: row @a s lists niche cohort indices used as food at stage @a s.
     *        Empty outer vector when diet is not cohort-based (e.g. autotrophs on nutrients).
     */
    virtual std::vector<std::vector<std::size_t>> getDietByCohortIndex() const;

    const std::string& getName() const;
    float getBiomassToEnergyConversionFactor() const;
    const std::vector<double>& getMaintenanceCost() const;
    const std::vector<double>& getFertility() const;
    const std::vector<double>& getResilience() const;
    double getVulnerability() const;
    const std::vector<double>& getBiomassPerIndividualAmount() const;
    const std::vector<std::vector<double>>& getBestEnvironmentalConditions() const;
    const std::vector<int>& getCyclesPerStages() const;
    const std::vector<std::vector<double>>& getDefenseStrategies() const;
    const std::vector<std::vector<double>>& getRecruitmentStrategies() const;

    void setName(std::string name);
    void setBiomassToEnergyConversionFactor(float energy_content);
    void setMaintenanceCost(std::vector<double> maintenance_cost);
    void setFertility(std::vector<double> fertility);
    void setResilience(std::vector<double> resilience);
    void setBiomassPerIndividualAmount(std::vector<double> biomass_per_individual_amount);
    void setBestEnvironmentalConditions(std::vector<std::vector<double>> best_environmental_conditions);
    void setCyclesPerStages(std::vector<int> cycles_per_stages);
    void setDefenseStrategies(std::vector<std::vector<double>> defense_strategies);
    void setRecruitmentStrategies(std::vector<std::vector<double>> recruitment_strategies);

    /**
     * @brief Stage index i using relative durations in cycles_per_stages_: stage i covers
     *        [sum(cycles_per_stages_[0..i-1]), sum(cycles_per_stages_[0..i])) (half-open).
     * @return Index i, or -1 if empty or cycles_elapsed is outside the total span.
     */
    int calculateStage(int cycles_elapsed) const;

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

protected:
    std::string name_;
    float biomass_to_energy_conversion_factor_{19.5f};
    std::vector<double> maintenance_cost_;
    std::vector<double> fertility_;
    std::vector<double> resilience_;
    std::vector<double> biomass_per_individual_amount_;
    std::vector<std::vector<double>> best_environmental_conditions_;
    /** Relative number of cycles spent in each stage; e.g. {3,10,15} => stage 0 on [0,3), 1 on [3,13), 2 on [13,28). */
    std::vector<int> cycles_per_stages_;
    std::vector<std::vector<double>> defense_strategies_;
    std::vector<std::vector<double>> recruitment_strategies_;
    double vulnerability_{0.0};
    bool initialized_{false};
};
