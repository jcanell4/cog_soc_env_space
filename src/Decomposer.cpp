#include "Decomposer.h"

#include "Niche.h"

#include <algorithm>
#include <utility>

Decomposer::Decomposer() {
    setBaseDeathRate(0.02);
}

void Decomposer::initialize(const Niche& niche) {
    LivingBeing::initialize(niche);
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

bool Decomposer::isDecomposer() const {
    return true;
}

Decomposer& Decomposer::setName(std::string name) {
    setNameValue(std::move(name));
    return *this;
}

Decomposer& Decomposer::setEnergyContent(float energy_content) {
    setEnergyContentValue(energy_content);
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

double Decomposer::calculate_death_biomass(double total_biomass, double /*accepted_growth*/) const {
    return total_biomass * getBaseDeathRate();
}

std::vector<std::tuple<int, double>> Decomposer::calculate_growth_biomass(const Niche& niche,
                                                                          double cohort_biomass) const {
    const auto& cohorts = niche.getCohortSet();
    const std::size_t n = cohorts.size();

    if (n != donor_efficiency_.size()) {
        return {};
    }
    if (cohort_biomass <= 0.0) {
        return {};
    }

    double best_take = 0.0;
    std::size_t best_donor = 0;

    for (std::size_t j = 0; j < n; ++j) {
        if (j == decomposer_cohort_index_) {
            continue;
        }
        const double death_pool = cohorts[j].getDeathBiomass();
        if (death_pool <= 0.0) {
            continue;
        }

        const double eff = donor_efficiency_[j];
        if (eff <= 0.0) {
            continue;
        }

        const double desired = cohort_biomass * max_decomposition_rate_ * eff;
        const double take = std::min(death_pool, desired);
        if (take > best_take) {
            best_take = take;
            best_donor = j;
        }
    }

    if (best_take <= 0.0) {
        return {};
    }
    const int code = -static_cast<int>(best_donor) - 1;
    return {{code, best_take}};
}
