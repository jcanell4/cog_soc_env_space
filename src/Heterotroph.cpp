#include "Heterotroph.h"

#include "Constants.h"

#include <algorithm>
#include <utility>

Heterotroph::Heterotroph() = default;

void Heterotroph::initialize(const Niche& niche) {
    update_factor_resources(niche);
    LivingBeing::initialize(niche);
}

int Heterotroph::getFoodType() const {
    return DietType::HETEROTROPH_TYPE;
}

int Heterotroph::getClassType() const {
    return LivingBeingClassType::HETEROTROPH;
}

std::vector<std::vector<std::size_t>> Heterotroph::getDietByCohortIndex() const {
    const std::size_t n_stages = cycles_per_stages_.empty() ? 1 : cycles_per_stages_.size();
    const std::size_t hetero_code = static_cast<std::size_t>(DietType::HETEROTROPH_TYPE);
    return std::vector<std::vector<std::size_t>>(n_stages, std::vector<std::size_t>{hetero_code});
}

std::vector<double> Heterotroph::clamp_unit_interval(std::vector<double> values) {
    for (double& value : values) {
        value = std::clamp(value, 0.0, 1.0);
    }
    return values;
}

const std::vector<double>& Heterotroph::getSearchCaptureEfficiency() const {
    return search_capture_efficiency_;
}

const std::vector<double>& Heterotroph::getHandlingTimePenalty() const {
    return handling_time_penalty_;
}

std::size_t Heterotroph::getPredatorCohortIndex() const {
    return predator_cohort_index_;
}

double Heterotroph::getMaxIngestionRate() const {
    return max_ingestion_rate_;
}

double Heterotroph::getFactorConditions() const {
    return factor_conditions_;
}

double Heterotroph::getFactorResources() const {
    return factor_resources_;
}

Heterotroph& Heterotroph::setName(std::string name) {
    LivingBeing::setName(std::move(name));
    return *this;
}

Heterotroph& Heterotroph::setEnergyContent(float energy_content) {
    setBiomassToEnergyConversionFactor(energy_content);
    return *this;
}

Heterotroph& Heterotroph::setSearchCaptureEfficiency(std::vector<double> values) {
    search_capture_efficiency_ = clamp_unit_interval(std::move(values));
    return *this;
}

Heterotroph& Heterotroph::setHandlingTimePenalty(std::vector<double> values) {
    handling_time_penalty_ = clamp_unit_interval(std::move(values));
    return *this;
}

Heterotroph& Heterotroph::setPredatorCohortIndex(std::size_t value) {
    predator_cohort_index_ = value;
    return *this;
}

Heterotroph& Heterotroph::setMaxIngestionRate(double value) {
    max_ingestion_rate_ = std::max(0.0, value);
    return *this;
}

void Heterotroph::update_factor_resources(const Niche&) {
    factor_resources_ = 1.0;
}
