#include "Autotroph.h"

#include <algorithm>
#include <utility>

Autotroph::Autotroph()
    : LivingBeing(std::string{}, 17.5f) {}

Autotroph& Autotroph::setName(std::string name) {
    setNameValue(std::move(name));
    return *this;
}

Autotroph& Autotroph::setEnergyContent(float energy_content) {
    setEnergyContentValue(energy_content);
    return *this;
}

double Autotroph::getSubstrate() const {
    return substrate_;
}

Autotroph& Autotroph::setSubstrate(double substrate) {
    substrate_ = std::clamp(substrate, 0.0, 1.0);
    return *this;
}

double Autotroph::getFactorConditions() const {
    return factor_conditions_;
}

double Autotroph::getFactorSubtrate() const {
    return factor_subtrate_;
}

double Autotroph::getFactorNutrients() const {
    return factor_nutrients_;
}

double Autotroph::getGrowthEffectiveRate() const {
    return getMaxGrowthRate() * factor_conditions_ * factor_subtrate_ * factor_nutrients_;
}

double Autotroph::getMaxGrowthRate() const {
    return LivingBeing::getMaxGrowthRate();
}

void Autotroph::initialize(const Niche& niche) {
    LivingBeing::initialize(niche);
}

double Autotroph::calculate_death_biomass(double, double) const {
    return 0.0;
}

std::vector<std::tuple<int, double>> Autotroph::calculate_growth_biomass(const Niche&, double) const {
    return {};
}
