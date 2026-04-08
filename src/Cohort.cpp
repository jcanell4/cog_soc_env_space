#include "Cohort.h"

#include <algorithm>

const std::string& Cohort::getSpecieName() const {
    static const std::string empty_name;
    return specie_ ? specie_->getName() : empty_name;
}

double Cohort::getEnergy() const {
    return calculateEnergy();
}

double Cohort::calculateEnergy() const {
    return specie_ ? biomass_ * static_cast<double>(specie_->getBiomassToEnergyConversionFactor()) : 0.0;
}

double Cohort::getBiomass() const {
    return biomass_;
}

double Cohort::getDeathBiomass() const {
    return death_biomass_;
}

bool Cohort::isDecomposerCohort() const {
    return specie_ != nullptr && specie_->isDecomposer();
}

Cohort& Cohort::setSpecie(const LivingBeing& value) {
    specie_ = &value;
    return *this;
}

Cohort& Cohort::setBiomass(double value) {
    biomass_ = std::max(0.0, value);
    return *this;
}

Cohort& Cohort::setDeathBiomass(double value) {
    death_biomass_ = std::max(0.0, value);
    return *this;
}

void Cohort::update_biomass(double accepted_growth) {
    biomass_ = std::max(0.0, biomass_ + accepted_growth);
}

void Cohort::update_deaths(double deaths) {
    if (deaths > biomass_) {
        deaths = biomass_;
    }
    biomass_ -= deaths;
    death_biomass_ += deaths;
}

double Cohort::decrement_death_biomass(double amount) {
    const double before = death_biomass_;
    death_biomass_ = std::max(0.0, death_biomass_ - amount);
    return before - death_biomass_;
}

std::vector<std::tuple<int, double>> Cohort::calculate_growth_demand(const Niche&) const {
    return {};
}

void Cohort::initialize(const Niche&) {
}

