#pragma once

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

    std::uint64_t getId() const;

    const std::string& getSpecieName() const;
    double getEnergy() const;
    double calculateEnergy() const;
    const std::vector<double>& getBiomass() const;
    double getTotalBiomass() const;
    const std::vector<double>& getDeathBiomass() const;
    double getTotalDeathBiomass() const;

    const LivingBeing* getSpecie() const;

    Cohort& setSpecie(const LivingBeing& value);
    Cohort& setBiomass(std::vector<double> value);
    Cohort& setDeathBiomass(std::vector<double> value);

    void update_deaths(int stage);
    double decrement_death_biomass(std::vector<double> amounts);
    void update_step(Niche& niche);
    void initialize(const Niche& niche);

    std::uint64_t getCohortElapsedCycles() const;

    void transferStageBiomass(int from_stage, int to_stage, double amount);
    void death_by_age(double dead_biomass_by_age);

private:
    std::uint64_t id_;
    const LivingBeing* specie_{nullptr};
    std::vector<double> biomass_{0.0};
    std::vector<double> death_biomass_;
    std::uint64_t cohort_elapsed_cycles_{0};
};
