#include "Decomposer.h"

#include "Constants.h"
#include "Niche.h"

#include <algorithm>
#include <utility>

Decomposer::Decomposer() = default;

void Decomposer::initialize(const Niche& niche) {
    LivingBeing::initialize(niche);
}

int Decomposer::getFoodType() const {
    return DietType::CATABOLIC_TYPE;
}

int Decomposer::getClassType() const {
    return LivingBeingClassType::DECOMPOSER;
}

std::vector<std::vector<std::size_t>> Decomposer::getDietByCohortIndex() const {
    std::vector<std::size_t> row;
    row.push_back(static_cast<std::size_t>(DietType::CATABOLIC_TYPE));
    for (std::size_t j = 0; j < donor_efficiency_.size(); ++j) {
        if (j == decomposer_cohort_index_) {
            continue;
        }
        if (donor_efficiency_[j] > 0.0) {
            row.push_back(j);
        }
    }
    const std::size_t n_stages = cycles_per_stages_.empty() ? 1 : cycles_per_stages_.size();
    return std::vector<std::vector<std::size_t>>(n_stages, row);
}

std::vector<double> Decomposer::clamp_unit_interval(std::vector<double> v) {
    for (auto& x : v) {
        x = std::clamp(x, 0.0, 1.0);
    }
    return v;
}

const std::vector<double>& Decomposer::getDonorEfficiency() const {
    return donor_efficiency_;
}

std::size_t Decomposer::getDecomposerCohortIndex() const {
    return decomposer_cohort_index_;
}

double Decomposer::getMaxDecompositionRate() const {
    return max_decomposition_rate_;
}

Decomposer& Decomposer::setName(std::string name) {
    LivingBeing::setName(std::move(name));
    return *this;
}

Decomposer& Decomposer::setEnergyContent(float energy_content) {
    setBiomassToEnergyConversionFactor(energy_content);
    return *this;
}

Decomposer& Decomposer::setDonorEfficiency(std::vector<double> values) {
    donor_efficiency_ = clamp_unit_interval(std::move(values));
    return *this;
}

Decomposer& Decomposer::setDecomposerCohortIndex(std::size_t value) {
    decomposer_cohort_index_ = value;
    return *this;
}

Decomposer& Decomposer::setMaxDecompositionRate(double value) {
    max_decomposition_rate_ = std::max(0.0, value);
    return *this;
}
