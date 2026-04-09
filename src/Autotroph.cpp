#include "Autotroph.h"
#include "Constants.h"

Autotroph::Autotroph()
    : LivingBeing(std::string{}, 17.5f) {}

int Autotroph::getFoodType() const {
    return DietType::NUTRIENTS_TYPE;
}

std::vector<std::vector<std::size_t>> Autotroph::getDietByCohortIndex() const {
    const std::size_t n_stages = cycles_per_stages_.empty() ? 1 : cycles_per_stages_.size();
    const std::size_t nutrient_code = static_cast<std::size_t>(DietType::NUTRIENTS_TYPE);
    return std::vector<std::vector<std::size_t>>(n_stages, std::vector<std::size_t>{nutrient_code});
}

void Autotroph::initialize(const Niche& niche) {
    LivingBeing::initialize(niche);
}
