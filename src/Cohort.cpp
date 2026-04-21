#include "Cohort.h"
#include "Constants.h"
#include "Niche.h"
#include "SimulationConfig.h"
#include "Utilities.h"

#include <algorithm>
#include <utility>

namespace {

std::uint64_t allocate_cohort_id() {
    static std::uint64_t next = 1;
    return next++;
}

std::vector<double> normalizeDeathBins(std::vector<double> value) {
    for (double& v : value) {
        v = std::max(0.0, v);
    }
    return value;
}

std::vector<double> normalizeNonNegative(std::vector<double> value) {
    if (value.empty()) {
        value.push_back(0.0);
    }
    for (double& v : value) {
        v = std::max(0.0, v);
    }
    return value;
}

}  // namespace

Cohort::Cohort() : id_(allocate_cohort_id()) {}

Cohort::Cohort(const Cohort& other)
    : id_(allocate_cohort_id()),
      specie_(other.specie_),
      biomass_(other.biomass_),
      death_biomass_(other.death_biomass_) {}

Cohort::Cohort(Cohort&& other) noexcept
    : id_(other.id_),
      specie_(other.specie_),
      biomass_(std::move(other.biomass_)),
      death_biomass_(std::move(other.death_biomass_)) {
    other.specie_ = nullptr;
    other.id_ = 0;
}

Cohort& Cohort::operator=(const Cohort& other) {
    if (this != &other) {
        specie_ = other.specie_;
        biomass_ = other.biomass_;
        death_biomass_ = other.death_biomass_;
    }
    return *this;
}

Cohort& Cohort::operator=(Cohort&& other) noexcept {
    if (this != &other) {
        specie_ = other.specie_;
        biomass_ = std::move(other.biomass_);
        death_biomass_ = std::move(other.death_biomass_);
        other.specie_ = nullptr;
        other.id_ = 0;
    }
    return *this;
}

std::uint64_t Cohort::getId() const {
    return id_;
}

const std::string& Cohort::getSpecieName() const {
    static const std::string empty_name;
    return specie_ ? specie_->getName() : empty_name;
}

double Cohort::getEnergy() const {
    return calculateEnergy();
}

double Cohort::calculateEnergy() const {
    return specie_ ? getTotalBiomass() * static_cast<double>(specie_->getBiomassToEnergyConversionFactor()) : 0.0;
}

const std::vector<double>& Cohort::getBiomass() const {
    return biomass_;
}

double Cohort::getTotalBiomass() const {
    double total = 0.0;
    for (double value : biomass_) {
        total += value;
    }
    return total;
}

const std::vector<double>& Cohort::getDeathBiomass() const {
    return death_biomass_;
}

double Cohort::getTotalDeathBiomass() const {
    double total = 0.0;
    for (double value : death_biomass_) {
        total += value;
    }
    return total;
}

const LivingBeing* Cohort::getSpecie() const {
    return specie_;
}

Cohort& Cohort::setSpecie(const LivingBeing& value) {
    specie_ = &value;
    return *this;
}

Cohort& Cohort::setBiomass(std::vector<double> value) {
    biomass_ = normalizeNonNegative(std::move(value));
    return *this;
}

Cohort& Cohort::setDeathBiomass(std::vector<double> value) {
    death_biomass_ = normalizeDeathBins(std::move(value));
    return *this;
}

void Cohort::update_deaths(int stage) {
    if (specie_ == nullptr || biomass_.empty() || stage < 0) {
        return;
    }
    const std::size_t i = static_cast<std::size_t>(stage);
    if (i >= biomass_.size()) {
        return;
    }

    const auto& resilience = specie_->getResilience();
    const double vulnerability = specie_->getVulnerability();
    const double noise = utilities::randomNormal(0.0, SimulationConfig::global().noise_stddev);
    const double noise_factor = std::max(0.0, 1.0 + noise);

    const double resilience_i = i < resilience.size() ? resilience[i] : 0.0;
    const double death_factor = (vulnerability + (1.0 - resilience_i)) * noise_factor;
    const double dead_i = biomass_[i] * death_factor;
    biomass_[i] = std::max(0.0, biomass_[i] - dead_i);
    const std::vector<std::vector<double>>& death_fraction_by_size = specie_->getDeathBiomassFractionBySize();
    const std::vector<double> distribution =
        i < death_fraction_by_size.size() ? death_fraction_by_size[i] : std::vector<double>{};
    if (distribution.empty()) {
        if (death_biomass_.empty()) {
            death_biomass_.resize(DEATH_BIOMASS_FINEST_BIN + 1, 0.0);
        }
        death_biomass_[DEATH_BIOMASS_FINEST_BIN] += dead_i;
        return;
    }

    if (death_biomass_.size() < distribution.size()) {
        death_biomass_.resize(distribution.size(), 0.0);
    }
    for (std::size_t s = 0; s < distribution.size(); ++s) {
        death_biomass_[s] += dead_i * distribution[s];
    }
}

double Cohort::decrement_death_biomass(std::vector<double> amounts) {
    amounts = normalizeDeathBins(std::move(amounts));
    const double before = getTotalDeathBiomass();
    const std::size_t n = std::min(death_biomass_.size(), amounts.size());
    for (std::size_t i = 0; i < n; ++i) {
        const double take = std::min(death_biomass_[i], amounts[i]);
        death_biomass_[i] = std::max(0.0, death_biomass_[i] - take);
    }
    return before - getTotalDeathBiomass();
}

void Cohort::update_step(Niche& niche) {
    for (std::size_t stage = 0; stage < biomass_.size(); ++stage) {
        update_deaths(static_cast<int>(stage));
        const std::vector<double>& biomass_after_death = getBiomass();
        const double stage_biomass_after_death = stage < biomass_after_death.size() ? biomass_after_death[stage] : 0.0;
        getSpecie()->process_individual_growth(niche, *this, static_cast<int>(stage));
        const std::vector<double>& biomass_after_growth = getBiomass();
        const double stage_biomass_after_growth =
            stage < biomass_after_growth.size() ? biomass_after_growth[stage] : 0.0;
        const double biomass_increment_this_cycle = stage_biomass_after_growth - stage_biomass_after_death;
        getSpecie()->process_reproductive_growth(*this, static_cast<int>(stage), biomass_increment_this_cycle);
        //        update_individual_growth(niche, static_cast<int>(self_cohort_index), static_cast<int>(stage));
    }
}

void Cohort::initialize(const Niche&) {
}

void Cohort::update_individual_growth(Niche& niche, int self_cohort_index, int stage) {
    // if (specie_ == nullptr || biomass_.empty() || stage < 0 || self_cohort_index < 0) {
    //     return;
    // }
    // const std::size_t su = static_cast<std::size_t>(stage);
    // if (su >= biomass_.size()) {
    //     return;
    // }

    // const std::vector<std::tuple<int, int, int>>& diet_rules = specie_->getDietByCohortIndex();

    // const std::vector<double>& maintenance = specie_->getMaintenanceCost();
    // const double m_stage = su < maintenance.size() ? std::clamp(maintenance[su], 0.0, 1.0) : 0.0;

    // Niche::CohortSet& cohorts = niche.getCohortSet();

    // for (const auto& rule : diet_rules) {
    //     const int prey_cohort_idx = std::get<0>(rule);
    //     const int min_prey_st = std::get<1>(rule);
    //     const int max_prey_st = std::get<2>(rule);
    //     if (min_prey_st > max_prey_st) {
    //         continue;
    //     }
    //     if (prey_cohort_idx < 0 || static_cast<std::size_t>(prey_cohort_idx) >= cohorts.size()) {
    //         continue;
    //     }
    //     if (prey_cohort_idx == self_cohort_index) {
    //         continue;
    //     }
    //     Cohort& prey = cohorts[static_cast<std::size_t>(prey_cohort_idx)];
    //     for (std::size_t st = 0; st < prey.biomass_.size(); ++st) {
    //         const int st_i = static_cast<int>(st);
    //         if (st_i < min_prey_st || st_i > max_prey_st) {
    //             continue;
    //         }
    //         const double obtained =
    //             specie_->calculateObtainedBiomassIncrement(niche, prey_cohort_idx, st_i);
    //         const double take = std::min(std::max(0.0, obtained), prey.biomass_[st]);
    //         prey.biomass_[st] -= take;
    //         biomass_[su] += take;
    //     }
    // }
    // biomass_[su] = std::max(0.0, biomass_[su] - biomass_[su] * m_stage);
}

