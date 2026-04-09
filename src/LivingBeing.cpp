#include "LivingBeing.h"
#include "Niche.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace {

void clampMatrixToZeroOne(std::vector<std::vector<double>>& matrix) {
    for (auto& row : matrix) {
        for (double& value : row) {
            value = std::clamp(value, 0.0, 1.0);
        }
    }
}

}  // namespace

LivingBeing::LivingBeing(std::string name, float energy_content)
    : name_(std::move(name)), biomass_to_energy_conversion_factor_(energy_content) {}

LivingBeing::~LivingBeing() = default;

void LivingBeing::initialize(const Niche& niche) {
    const auto& current_conditions = niche.getConditions();
    const int stage = 0;
    vulnerability_ = calculateVulnerability(
                               current_conditions, best_environmental_conditions_[static_cast<std::size_t>(stage)]);
    initialized_ = true;
}

bool LivingBeing::isDecomposer() const {
    return false;
}

std::vector<std::vector<std::size_t>> LivingBeing::getDietByCohortIndex() const {
    return {};
}

const std::string& LivingBeing::getName() const {
    return name_;
}

float LivingBeing::getBiomassToEnergyConversionFactor() const {
    return biomass_to_energy_conversion_factor_;
}

const std::vector<double>& LivingBeing::getMaintenanceCost() const {
    return maintenance_cost_;
}

const std::vector<double>& LivingBeing::getFertility() const {
    return fertility_;
}

const std::vector<double>& LivingBeing::getResilience() const {
    return resilience_;
}

double LivingBeing::getVulnerability() const {
    return vulnerability_;
}

const std::vector<double>& LivingBeing::getBiomassPerIndividualAmount() const {
    return biomass_per_individual_amount_;
}

const std::vector<std::vector<double>>& LivingBeing::getBestEnvironmentalConditions() const {
    return best_environmental_conditions_;
}

const std::vector<int>& LivingBeing::getCyclesPerStages() const {
    return cycles_per_stages_;
}

const std::vector<std::vector<double>>& LivingBeing::getDefenseStrategies() const {
    return defense_strategies_;
}

const std::vector<std::vector<double>>& LivingBeing::getRecruitmentStrategies() const {
    return recruitment_strategies_;
}

void LivingBeing::setName(std::string name) {
    name_ = std::move(name);
}

void LivingBeing::setBiomassToEnergyConversionFactor(float energy_content) {
    biomass_to_energy_conversion_factor_ = energy_content;
}

void LivingBeing::setMaintenanceCost(std::vector<double> maintenance_cost) {
    for (double& value : maintenance_cost) {
        value = std::clamp(value, 0.0, 1.0);
    }
    maintenance_cost_ = std::move(maintenance_cost);
}

void LivingBeing::setFertility(std::vector<double> fertility) {
    for (double& value : fertility) {
        value = std::clamp(value, 0.0, 1.0);
    }
    fertility_ = std::move(fertility);
}

void LivingBeing::setResilience(std::vector<double> resilience) {
    for (double& value : resilience) {
        value = std::clamp(value, 0.0, 1.0);
    }
    resilience_ = std::move(resilience);
}

void LivingBeing::setBiomassPerIndividualAmount(std::vector<double> biomass_per_individual_amount) {
    biomass_per_individual_amount_ = std::move(biomass_per_individual_amount);
}

void LivingBeing::setBestEnvironmentalConditions(std::vector<std::vector<double>> best_environmental_conditions) {
    clampMatrixToZeroOne(best_environmental_conditions);
    best_environmental_conditions_ = std::move(best_environmental_conditions);
}

void LivingBeing::setCyclesPerStages(std::vector<int> cycles_per_stages) {
    cycles_per_stages_ = std::move(cycles_per_stages);
}

void LivingBeing::setDefenseStrategies(std::vector<std::vector<double>> defense_strategies) {
    clampMatrixToZeroOne(defense_strategies);
    defense_strategies_ = std::move(defense_strategies);
}

void LivingBeing::setRecruitmentStrategies(std::vector<std::vector<double>> recruitment_strategies) {
    clampMatrixToZeroOne(recruitment_strategies);
    recruitment_strategies_ = std::move(recruitment_strategies);
}

int LivingBeing::calculateStage(int cycles_elapsed) const {
    const auto& d = cycles_per_stages_;
    if (d.empty()) {
        return -1;
    }
    int sum = 0;
    for (std::size_t i = 0; i < d.size(); ++i) {
        const int next = sum + d[i];
        if (cycles_elapsed >= sum && cycles_elapsed < next) {
            return static_cast<int>(i);
        }
        sum = next;
    }
    return d.size() - 1;
}

double LivingBeing::calculate_effective_recruitment_efficiency(
    const std::vector<double>& recruitment_strategies,
    const std::vector<double>& defense_strategies) {
    const std::size_t n = std::max(recruitment_strategies.size(), defense_strategies.size());
    double product = 1.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double D_i = i < defense_strategies.size() ? defense_strategies[i] : 0.0;
        const double R_i = i < recruitment_strategies.size() ? recruitment_strategies[i] : 0.0;
        const double e_i = std::clamp(1.0 - (D_i - R_i), 0.0, 1.0);
        product *= e_i;
    }
    return product;
}

double LivingBeing::calculateObtainedBiomassIncrement(const Niche& /*niche*/,
                                                      int /*cohort_index*/,
                                                      int /*stage_index*/) const {
    return 0.0;
}

double LivingBeing::calculateVulnerability(const std::vector<double>& current_conditions,
                                           const std::vector<double>& best_conditions) {
    const std::size_t n = std::max(current_conditions.size(), best_conditions.size());
    if (n == 0) {
        return 0.0;
    }
    double sum_sq = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double c = i < current_conditions.size() ? current_conditions[i] : 0.0;
        const double b = i < best_conditions.size() ? best_conditions[i] : 0.0;
        const double d = c - b;
        sum_sq += d * d;
    }
    const double euclidean = std::sqrt(sum_sq);
    const double max_distance = std::sqrt(static_cast<double>(n));
    return euclidean / max_distance;
}


