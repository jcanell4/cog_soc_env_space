#include "Niche.h"
#include "Constants.h"
#include "Utilities.h"

#include <algorithm>
#include <utility>

namespace {

std::vector<double> clampUnitInterval(std::vector<double> values) {
    for (double& value : values) {
        value = std::clamp(value, 0.0, 1.0);
    }
    return values;
}

std::vector<double> normalizeTwoNonNegative(std::vector<double> value) {
    value.resize(2, 0.0);
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
    return_rate_ = normalizeTwoNonNegative(std::move(value));
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
        if (cohort.isDecomposerCohort()) {
            total += cohort.getTotalBiomass();
        }
    }
    return total;
}

void Niche::update_nutrients() {
    const double noise_stddev = NOISE_STDDEV;
    const double effective_return_cost = std::clamp(return_cost_, 0.0, 1.0);
    const double nutrient_factor = 1.0 - effective_return_cost;

    for (Cohort& cohort : cohort_set_) {
        const std::vector<double>& death = cohort.getDeathBiomass();
        if (death.size() < 2) {
            continue;
        }

        const double noise = utilities::randomNormal(0.0, noise_stddev);
        const double multiplicative = std::max(0.0, 1.0 + noise);

        std::vector<double> processed(2, 0.0);
        processed[0] = death[0] * return_rate_[0] * multiplicative;
        processed[1] = death[1] * return_rate_[1] * multiplicative;

        const double extracted = cohort.decrement_death_biomass(std::move(processed));
        nutrients_ += extracted * nutrient_factor;
    }
}

void Niche::update_ecological_health() {
}

void Niche::step() {
    update_nutrients();
    update_cohorts();
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

