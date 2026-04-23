#include "Niche.h"
#include "Autotroph.h"
#include "Constants.h"
#include "SimulationConfig.h"
#include "Utilities.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

namespace {

std::vector<double> clampUnitInterval(std::vector<double> values) {
    for (double& value : values) {
        value = std::clamp(value, 0.0, 1.0);
    }
    return values;
}

std::vector<double> normalizeNonNegative(std::vector<double> value) {
    for (double& v : value) {
        v = std::max(0.0, v);
    }
    return value;
}

} // namespace

double Niche::getSurface() const {
    return surface_;
}

double Niche::getEcologicalHealth() const {
    return ecological_health_;
}

double Niche::getNutrients() const {
    return nutrients_;
}

const Niche::CohortSet& Niche::getCohortSet() const {
    return cohort_set_;
}

Niche::CohortSet& Niche::getCohortSet() {
    return cohort_set_;
}

const std::vector<double>& Niche::getReturnRate() const {
    return return_rate_;
}

const std::vector<double>& Niche::getConditions() const {
    return conditions_;
}

const std::vector<double>& Niche::getLimitingFactors() const {
    return limiting_factors_;
}

Niche& Niche::setSurface(double value) {
    surface_ = std::max(0.0, value);
    return *this;
}

Niche& Niche::setEcologicalHealth(double value) {
    ecological_health_ = std::clamp(value, 0.0, 1.0);
    return *this;
}

Niche& Niche::setNutrients(double value) {
    nutrients_ = std::max(0.0, value);
    return *this;
}

Niche& Niche::setCohortSet(CohortSet value) {
    cohort_set_ = std::move(value);
    return *this;
}

Niche& Niche::setReturnRate(std::vector<double> value) {
    return_rate_ = normalizeNonNegative(std::move(value));
    return *this;
}

Niche& Niche::setConditions(std::vector<double> value) {
    conditions_ = clampUnitInterval(std::move(value));
    return *this;
}

Niche& Niche::setLimitingFactors(std::vector<double> value) {
    limiting_factors_ = std::move(value);
    return *this;
}

double Niche::getDeathBiomass() const {
    double total = 0.0;
    for (const Cohort& cohort : cohort_set_) {
        total += cohort.getTotalDeathBiomass();
    }
    return total;
}

double Niche::getLivingBiomass() const {
    double total = 0.0;
    for (const Cohort& cohort : cohort_set_) {
        total += cohort.getTotalBiomass();
    }
    return total;
}

double Niche::getDecomposerBiomass() const {
    double total = 0.0;
    for (const Cohort& cohort : cohort_set_) {
        const LivingBeing* sp = cohort.getSpecie();
        if (sp != nullptr && sp->getClassType() == LivingBeingClassType::DECOMPOSER) {
            total += cohort.getTotalBiomass();
        }
    }
    return total;
}

std::vector<double> Niche::getAutotrophBiomassPerStratum() const {
    std::vector<double> per_stratum;
    for (const Cohort& cohort : cohort_set_) {
        const LivingBeing* sp = cohort.getSpecie();
        if (sp == nullptr || sp->getClassType() != LivingBeingClassType::AUTOTROPH) {
            continue;
        }
        const Autotroph* autotroph = static_cast<const Autotroph*>(sp);
        const std::vector<int>& stratum = autotroph->getStratum();
        const std::vector<double>& biomass = cohort.getBiomass();
        for (std::size_t stage = 0; stage < biomass.size(); ++stage) {
            if (stage >= stratum.size()) {
                continue;
            }
            const int h = stratum[stage];
            if (h < 0) {
                continue;
            }
            const std::size_t hi = static_cast<std::size_t>(h);
            if (hi >= per_stratum.size()) {
                per_stratum.resize(hi + 1, 0.0);
            }
            per_stratum[hi] += biomass[stage];
        }
    }
    return per_stratum;
}

std::vector<double> Niche::getLithPerStratum() const {
    std::vector<double> shadow_accumulated;
    for (const Cohort& cohort : cohort_set_) {
        const LivingBeing* sp = cohort.getSpecie();
        if (sp == nullptr || sp->getClassType() != LivingBeingClassType::AUTOTROPH) {
            continue;
        }
        const Autotroph* autotroph = static_cast<const Autotroph*>(sp);
        const std::vector<int>& stratum = autotroph->getStratum();
        const std::vector<double>& opacity = autotroph->getOpacity();
        const std::vector<double>& biomass = cohort.getBiomass();
        for (std::size_t stage = 0; stage < biomass.size(); ++stage) {
            if (stage >= stratum.size() || stage >= opacity.size()) {
                continue;
            }
            const int h = stratum[stage];
            if (h < 0) {
                continue;
            }
            const std::size_t hi = static_cast<std::size_t>(h);
            if (hi >= shadow_accumulated.size()) {
                shadow_accumulated.resize(hi + 1, 0.0);
            }
            shadow_accumulated[hi] += biomass[stage] * opacity[stage];
        }
    }
    if (shadow_accumulated.empty()) {
        return {};
    }
    const double surf = getSurface();
    if (surf <= 0.0) {
        return {};
    }

    const std::size_t n = shadow_accumulated.size();
    std::vector<double> shadow_density(n);
    for (std::size_t i = 0; i < n; ++i) {
        shadow_density[i] = shadow_accumulated[i] / surf;
    }

    const int H = static_cast<int>(n);
    std::vector<double> light_fraction_per_stratum(n, 0.0);
    const std::size_t top = n - 1U;
    double incoming = 1.0;
    light_fraction_per_stratum[top] = std::exp(-0.3 * shadow_density[top]) * incoming;
    for (int h = H - 2; h >= 0; --h) {
        const std::size_t u = static_cast<std::size_t>(h);
        const std::size_t upper = u + 1U;
        const double new_incoming = std::exp(-shadow_density[upper]) * incoming;
        const double transmitted = std::exp(-(0.3*shadow_density[u]+shadow_density[upper])) * incoming;
        light_fraction_per_stratum[u] = std::clamp(transmitted, 0.0, 1.0);
        incoming = new_incoming;
    }
    return light_fraction_per_stratum;
}

void Niche::update_nutrients() {
    const double noise_stddev = SimulationConfig::global().noise_stddev;
    const double effective_return_cost = std::clamp(return_cost_, 0.0, 1.0);
    const double nutrient_factor = 1.0 - effective_return_cost;

    for (Cohort& cohort : cohort_set_) {
        const std::vector<double>& death = cohort.getDeathBiomass();
        if (death.empty() || return_rate_.empty()) {
            continue;
        }

        const double noise = utilities::randomNormal(0.0, noise_stddev);
        const double multiplicative = std::max(0.0, 1.0 + noise);

        const std::size_t n = std::min(death.size(), return_rate_.size());
        std::vector<double> processed(n, 0.0);
        for (std::size_t i = 0; i < n; ++i) {
            const double death_i = i < death.size() ? death[i] : 0.0;
            const double return_i = i < return_rate_.size() ? return_rate_[i] : 0.0;
            processed[i] = death_i * return_i * multiplicative;
        }

        const double extracted = cohort.decrement_death_biomass(std::move(processed));
        nutrients_ += extracted * nutrient_factor;
    }
}

void Niche::update_ecological_health() {
}

void Niche::update_niche() {
    update_ecological_health();
}

void Niche::step() {
    update_nutrients();
    update_cohorts();
    update_niche();
}

void Niche::initialize() {
    for (Cohort& cohort : cohort_set_) {
        cohort.initialize(*this);
    }
}

void Niche::update_cohorts() {
    for (Cohort& cohort : cohort_set_) {
        cohort.update_step(*this);
    }
}

