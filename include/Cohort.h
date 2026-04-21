#pragma once

/**
 * @file Cohort.h
 * @brief Minimal cohort container for the restart.
 */

#include "LivingBeing.h"

#include <cstdint>
#include <vector>

class Niche;

class Cohort {
public:
    Cohort();
    Cohort(const Cohort& other);
    Cohort(Cohort&& other) noexcept;
    Cohort& operator=(const Cohort& other);
    Cohort& operator=(Cohort&& other) noexcept;

    /** @brief Unique stable identifier assigned at construction (read-only). */
    std::uint64_t getId() const;

    const std::string& getSpecieName() const;
    double getEnergy() const;
    double calculateEnergy() const;
    const std::vector<double>& getBiomass() const;
    double getTotalBiomass() const;
    const std::vector<double>& getDeathBiomass() const;
    double getTotalDeathBiomass() const;

    /** @brief Pointer to the species model, or nullptr if not set. */
    const LivingBeing* getSpecie() const;

    Cohort& setSpecie(const LivingBeing& value);
    Cohort& setBiomass(std::vector<double> value);
    Cohort& setDeathBiomass(std::vector<double> value);

    void update_deaths(int stage);
    double decrement_death_biomass(std::vector<double> amounts);
    void update_step(Niche& niche);
    void initialize(const Niche& niche);

    /**
     * @brief Updates living biomass at @a stage using @a specie_->getDietByCohortIndex() prey cohort rules.
     * @param self_cohort_index Index of this cohort in @a niche.getCohortSet().
     */
    void update_individual_growth(Niche& niche, int self_cohort_index, int stage);

private:
    std::uint64_t id_;
    const LivingBeing* specie_{nullptr};
    std::vector<double> biomass_{0.0};
    /**
     * @brief Dead biomass by size class; index 0 is finest/most degraded.
     *        Vector length is dynamic (no fixed number of bins).
     */
    std::vector<double> death_biomass_;
};
