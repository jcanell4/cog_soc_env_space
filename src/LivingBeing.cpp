#include "LivingBeing.h"

#include <algorithm>
#include <utility>

LivingBeing::LivingBeing(std::string name, float energy_content)
    : name_(std::move(name)), biomass_to_energy_conversion_factor_(energy_content) {}

LivingBeing::~LivingBeing() = default;

void LivingBeing::initialize(const Niche&) {
    initialized_ = true;
}

const std::string& LivingBeing::getName() const {
    return name_;
}

float LivingBeing::getBiomassToEnergyConversionFactor() const {
    return biomass_to_energy_conversion_factor_;
}

const std::vector<double>& LivingBeing::getBestConditions() const {
    return best_conditions_;
}

void LivingBeing::setName(std::string name) {
    name_ = std::move(name);
}

void LivingBeing::setBiomassToEnergyConversionFactor(float energy_content) {
    biomass_to_energy_conversion_factor_ = energy_content;
}

void LivingBeing::setBestConditions(std::vector<double> best_conditions) {
    for (double& value : best_conditions) {
        value = std::clamp(value, 0.0, 1.0);
    }
    best_conditions_ = std::move(best_conditions);
}


