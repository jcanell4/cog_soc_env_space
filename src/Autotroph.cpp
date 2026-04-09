#include "Autotroph.h"
#include "Constants.h"

#include <algorithm>
#include <utility>

Autotroph::Autotroph()
    : LivingBeing(std::string{}, 17.5f) {}

int Autotroph::getFoodType() const {
    return DietType::NUTRIENTS_TYPE;
}

int Autotroph::getClassType() const {
    return LivingBeingClassType::AUTOTROPH;
}

std::vector<std::vector<std::size_t>> Autotroph::getDietByCohortIndex() const {
    const std::size_t n_stages = cycles_per_stages_.empty() ? 1 : cycles_per_stages_.size();
    const std::size_t nutrient_code = static_cast<std::size_t>(DietType::NUTRIENTS_TYPE);
    return std::vector<std::vector<std::size_t>>(n_stages, std::vector<std::size_t>{nutrient_code});
}

void Autotroph::initialize(const Niche& niche) {
    LivingBeing::initialize(niche);
}

const std::vector<double>& Autotroph::getOpacity() const {
    return opacity_;
}

void Autotroph::setOpacity(std::vector<double> value) {
    for (double& v : value) {
        v = std::clamp(v, 0.0, 1.0);
    }
    opacity_ = std::move(value);
}

const std::vector<int>& Autotroph::getStratum() const {
    return stratum_;
}

void Autotroph::setStratum(std::vector<int> value) {
    stratum_ = std::move(value);
}

const std::vector<double>& Autotroph::getMaxDensity() const {
    return max_density_;
}

void Autotroph::setMaxDensity(std::vector<double> value) {
    max_density_ = std::move(value);
}

const std::vector<double>& Autotroph::getMinLight() const {
    return min_light_;
}

void Autotroph::setMinLight(std::vector<double> value) {
    min_light_ = std::move(value);
}
