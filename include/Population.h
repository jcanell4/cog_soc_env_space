#pragma once

#include "LivingBeing.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class Niche;

class Population {
public:
    Population();
    Population(const Population& other);
    Population(Population&& other) noexcept;
    Population& operator=(const Population& other);
    Population& operator=(Population&& other) noexcept;

    std::uint64_t getId() const;

    const std::string& getSpecieName() const;
    double getEnergy() const;
    double calculateEnergy() const;
    const std::vector<double>& getBiomass() const;
    double getBiomass(std::size_t index, double out_of_range_default = 0.0) const;
    double getTotalBiomass() const;
    const std::vector<double>& getDeathBiomass() const;
    double getDeathBiomass(std::size_t index, double out_of_range_default = 0.0) const;
    double getTotalDeathBiomass() const;

    const LivingBeing* getSpecie() const;

    Population& setSpecie(const LivingBeing& value);
    Population& setBiomass(std::vector<double> value);
    Population& setDeathBiomass(std::vector<double> value);

    void update_deaths(int stage);
    double decrement_death_biomass(std::vector<double> amounts);
    void update_step(Niche& niche);
    void initialize(const Niche& niche);

    std::uint64_t getPopulationElapsedCycles() const;

    void transferStageBiomass(int from_stage, int to_stage, double amount);
    void death_by_age(double dead_biomass_by_age);

private:
    std::uint64_t id_;
    const LivingBeing* specie_{nullptr};
    std::vector<double> biomass_{0.0};
    std::vector<double> death_biomass_;
    std::uint64_t population_elapsed_cycles_{0};
};
