#include "Niche.h"

#include <algorithm>
#include <utility>

namespace {

std::vector<double> clampUnitInterval(std::vector<double> values) {
    for (double& value : values) {
        value = std::clamp(value, 0.0, 1.0);
    }
    return values;
}

} // namespace

double Niche::getSurface() const {
    return surface_;
}

double Niche::getBiologicalPotentialPerSurfaceUnit() const {
    return biological_potential_per_surface_unit_;
}

double Niche::getEcologicalHealth() const {
    return ecological_health_;
}

double Niche::getNutrients() const {
    return nutrients_;
}

double Niche::getSubstrate() const {
    return substrate_;
}

const Niche::CohortSet& Niche::getCohortSet() const {
    return cohort_set_;
}

double Niche::getReturnRate() const {
    return return_rate_;
}

const std::vector<double>& Niche::getConditions() const {
    return conditions_;
}

Niche& Niche::setSurface(double value) {
    surface_ = std::max(0.0, value);
    return *this;
}

Niche& Niche::setBiologicalPotentialPerSurfaceUnit(double value) {
    biological_potential_per_surface_unit_ = std::max(0.0, value);
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

Niche& Niche::setSubstrate(double value) {
    substrate_ = std::clamp(value, 0.0, 1.0);
    return *this;
}

Niche& Niche::setCohortSet(CohortSet value) {
    cohort_set_ = std::move(value);
    return *this;
}

Niche& Niche::setReturnRate(double value) {
    return_rate_ = std::max(0.0, value);
    return *this;
}

Niche& Niche::setConditions(std::vector<double> value) {
    conditions_ = clampUnitInterval(std::move(value));
    return *this;
}

double Niche::getMaxBiologicalPotential() const {
    return surface_ * biological_potential_per_surface_unit_ * ecological_health_;
}

double Niche::getDeathBiomass() const {
    double total = 0.0;
    for (const Cohort& cohort : cohort_set_) {
        total += cohort.getDeathBiomass();
    }
    return total;
}

double Niche::getLivingBiomass() const {
    double total = 0.0;
    for (const Cohort& cohort : cohort_set_) {
        total += cohort.getBiomass();
    }
    return total;
}

double Niche::getDecomposerBiomass() const {
    double total = 0.0;
    for (const Cohort& cohort : cohort_set_) {
        if (cohort.isDecomposerCohort()) {
            total += cohort.getBiomass();
        }
    }
    return total;
}

void Niche::update_nutrients(double) {
}

void Niche::update_ecological_health(double) {
}

void Niche::step(double dt) {
    update_nutrients(dt);
    update_cohorts();
}

void Niche::initialize() {
    for (Cohort& cohort : cohort_set_) {
        cohort.initialize(*this);
    }
}

void Niche::update_cohorts() {
}

