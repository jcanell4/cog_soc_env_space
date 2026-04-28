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

}

Cohort::Cohort() : id_(allocate_cohort_id()) {}

Cohort::Cohort(const Cohort& other)
    : id_(allocate_cohort_id()),
      specie_(other.specie_),
      biomass_(other.biomass_),
      death_biomass_(other.death_biomass_),
      cohort_elapsed_cycles_(0) {}

Cohort::Cohort(Cohort&& other) noexcept
    : id_(other.id_),
      specie_(other.specie_),
      biomass_(std::move(other.biomass_)),
      death_biomass_(std::move(other.death_biomass_)),
      cohort_elapsed_cycles_(other.cohort_elapsed_cycles_) {
    other.specie_ = nullptr;
    other.id_ = 0;
    other.cohort_elapsed_cycles_ = 0;
}

Cohort& Cohort::operator=(const Cohort& other) {
    if (this != &other) {
        specie_ = other.specie_;
        biomass_ = other.biomass_;
        death_biomass_ = other.death_biomass_;
        cohort_elapsed_cycles_ = other.cohort_elapsed_cycles_;
    }
    return *this;
}

Cohort& Cohort::operator=(Cohort&& other) noexcept {
    if (this != &other) {
        specie_ = other.specie_;
        biomass_ = std::move(other.biomass_);
        death_biomass_ = std::move(other.death_biomass_);
        cohort_elapsed_cycles_ = other.cohort_elapsed_cycles_;
        other.specie_ = nullptr;
        other.id_ = 0;
        other.cohort_elapsed_cycles_ = 0;
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
    if (specie_ == nullptr) {
        return 0.0;
    }
    const double living_energy =
        getTotalBiomass() * static_cast<double>(specie_->getBiomassToEnergyConversionFactor());
    const double death_energy =
        getTotalDeathBiomass() * static_cast<double>(specie_->getDeathBiomassToEnergyConversionFactor());
    return living_energy + death_energy;
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

/**
 * @brief Updates the deaths of a cohort.
 * @param stage The stage of the cohort.
 *        The deaths are determined by the resilience of the specie (resistance to the adverse conditions).
 *        The deaths are determined by the vulnerability of the specie as euclidean distance between the environment conditions and the best 
 *        conditions for this stage of the specie.
 *        The equation for death_factor is:
 *        death_factor = vulnerability * (1-resilience)
 *        The equation for dead_i is:
 *        dead_i = biomass_[i] * death_factor
 *        where biomass_[i] is the biomass of the stage i of the cohort.
 */
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
    const double death_factor = vulnerability * (1.0 - resilience_i) * noise_factor;
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

std::uint64_t Cohort::getCohortElapsedCycles() const {
    return cohort_elapsed_cycles_;
}

void Cohort::transferStageBiomass(int from_stage, int to_stage, double amount) {
    if (from_stage < 0 || to_stage < 0 || amount <= 0.0) {
        return;
    }
    const std::size_t from = static_cast<std::size_t>(from_stage);
    const std::size_t to = static_cast<std::size_t>(to_stage);
    const std::size_t required_size = std::max(from, to) + 1U;
    if (required_size > biomass_.size()) {
        biomass_.resize(required_size, 0.0);
    }
    const double take = std::min(amount, std::max(0.0, biomass_[from]));
    if (take <= 0.0) {
        return;
    }
    biomass_[from] -= take;
    biomass_[to] += take;
}

void Cohort::death_by_age(double dead_biomass_by_age) {
    if (dead_biomass_by_age <= 0.0 || biomass_.empty()) {
        return;
    }

    const std::size_t stage = biomass_.size() - 1U;
    const double available = std::max(0.0, biomass_[stage]);
    const double dead_amount = std::min(dead_biomass_by_age, available);
    if (dead_amount <= 0.0) {
        return;
    }
    biomass_[stage] = std::max(0.0, biomass_[stage] - dead_amount);

    const std::vector<std::vector<double>> empty_distribution;
    const std::vector<std::vector<double>>& death_fraction_by_size =
        specie_ != nullptr ? specie_->getDeathBiomassFractionBySize() : empty_distribution;
    const std::vector<double> distribution =
        stage < death_fraction_by_size.size() ? death_fraction_by_size[stage] : std::vector<double>{};

    if (distribution.empty()) {
        if (death_biomass_.empty()) {
            death_biomass_.resize(DEATH_BIOMASS_FINEST_BIN + 1, 0.0);
        }
        death_biomass_[DEATH_BIOMASS_FINEST_BIN] += dead_amount;
        return;
    }

    if (death_biomass_.size() < distribution.size()) {
        death_biomass_.resize(distribution.size(), 0.0);
    }
    for (std::size_t s = 0; s < distribution.size(); ++s) {
        death_biomass_[s] += dead_amount * distribution[s];
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
    if (specie_ == nullptr || biomass_.empty()) {
        return;
    }
    ++cohort_elapsed_cycles_;
    for (std::size_t stage = 0; stage < biomass_.size(); ++stage) {
        const std::vector<double>& biomass_before_growth = getBiomass();
        const double stage_biomass_before_growth = stage < biomass_before_growth.size() ? biomass_before_growth[stage] : 0.0;
        getSpecie()->process_individual_growth(niche, *this, static_cast<int>(stage));
        const std::vector<double>& biomass_after_growth = getBiomass();
        const double stage_biomass_after_growth =
            stage < biomass_after_growth.size() ? biomass_after_growth[stage] : 0.0;
        const double biomass_increment_this_cycle = stage_biomass_after_growth - stage_biomass_before_growth;
        getSpecie()->process_reproductive_growth(
            *this,
            static_cast<int>(stage),
            stage_biomass_before_growth,
            biomass_increment_this_cycle);
        update_deaths(static_cast<int>(stage));
    }
    specie_->updateStages(*this, static_cast<int>(cohort_elapsed_cycles_));
}

void Cohort::initialize(const Niche& niche) {
    if (specie_ == nullptr) {
        return;
    }
    // Species initialize is non-const, but cohorts store a const species pointer by design.
    LivingBeing* specie = const_cast<LivingBeing*>(specie_);
    specie->initialize(niche);
}

