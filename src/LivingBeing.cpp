#include "LivingBeing.h"
#include "Cohort.h"
#include "Niche.h"
#include "SimulationConfig.h"
#include "Utilities.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace {

void clampMatrixToZeroOne(std::vector<std::vector<double>>& matrix) {
    for (auto& row : matrix) {
        for (double& value : row) {
            value = std::clamp(value, 0.0, 1.0);
        }
    }
}

std::vector<double> normalizeFractionRow(std::vector<double> row) {
    for (double& value : row) {
        value = std::max(0.0, value);
    }
    const double sum = std::accumulate(row.begin(), row.end(), 0.0);
    if (sum <= 0.0) {
        return {};
    }
    for (double& value : row) {
        value /= sum;
    }
    return row;
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

const std::vector<std::tuple<int, int, int>>& LivingBeing::getDietByCohortIndex() const {
    return diet_by_cohort_index_;
}

void LivingBeing::setDietByCohortIndex(std::vector<std::tuple<int, int, int>> diet_by_cohort_index) {
    diet_by_cohort_index_ = std::move(diet_by_cohort_index);
}

const std::string& LivingBeing::getFoodType() const {
    return food_type_;
}

void LivingBeing::setFoodType(std::string food_type) {
    food_type_ = std::move(food_type);
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

const std::vector<double>& LivingBeing::getMaxFertility() const {
    return max_fertility_;
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

const std::vector<double>& LivingBeing::getIndividualOccupiedSurface() const {
    return individual_occupied_surface_;
}

const std::vector<std::vector<double>>& LivingBeing::getCharacteristicsDeathBiomass() const {
    return characteristics_death_biomass_;
}

const std::vector<std::vector<double>>& LivingBeing::getDeathBiomassFractionBySize() const {
    return death_biomass_fraction_by_size_;
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

const std::vector<double>& LivingBeing::getMaxIndividualGrowth() const {
    return max_individual_growth_;
}

double LivingBeing::getColonyAbilityRate() const {
    return colony_ability_rate_;
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

void LivingBeing::setMaxFertility(std::vector<double> max_fertility) {
    for (double& value : max_fertility) {
        value = std::clamp(value, 0.0, 1.0);
    }
    max_fertility_ = std::move(max_fertility);
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

void LivingBeing::setIndividualOccupiedSurface(std::vector<double> individual_occupied_surface) {
    for (double& value : individual_occupied_surface) {
        value = std::max(0.0, value);
    }
    individual_occupied_surface_ = std::move(individual_occupied_surface);
}

void LivingBeing::setCharacteristicsDeathBiomass(
    std::vector<std::vector<double>> characteristics_death_biomass) {
    for (std::vector<double>& row : characteristics_death_biomass) {
        for (double& value : row) {
            value = std::max(0.0, value);
        }
    }
    characteristics_death_biomass_ = std::move(characteristics_death_biomass);
}

void LivingBeing::setDeathBiomassFractionBySize(
    std::vector<std::vector<double>> death_biomass_fraction_by_size) {
    for (std::vector<double>& row : death_biomass_fraction_by_size) {
        row = normalizeFractionRow(std::move(row));
    }
    death_biomass_fraction_by_size_ = std::move(death_biomass_fraction_by_size);
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

void LivingBeing::setMaxIndividualGrowth(std::vector<double> max_individual_growth) {
    for (double& value : max_individual_growth) {
        value = std::clamp(value, 0.0, 1.0);
    }
    max_individual_growth_ = std::move(max_individual_growth);
}

void LivingBeing::setColonyAbilityRate(double colony_ability_rate) {
    colony_ability_rate_ = std::clamp(colony_ability_rate, 0.0, 1.0);
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

void LivingBeing::process_individual_growth(Niche& /*niche*/, Cohort& cohort, int stage) const {
    if (cohort.getSpecie() == nullptr || cohort.getBiomass().empty() || stage < 0) {
        throw std::invalid_argument("Invalid cohort specie or stage");
    }
    const std::size_t su = static_cast<std::size_t>(stage);
    if (su >= cohort.getBiomass().size()) {
        throw std::invalid_argument("Invalid stage index");
    }

}

void LivingBeing::process_reproductive_growth(Cohort& cohort,
                                              int stage_index,
                                              double biomass_increment_this_cycle) const {
    if (cohort.getSpecie() == nullptr || cohort.getBiomass().empty() || stage_index < 0) {
        throw std::invalid_argument("Invalid cohort specie or stage");
    }
    const std::size_t su = static_cast<std::size_t>(stage_index);
    std::vector<double> biomass = cohort.getBiomass();
    if (su >= biomass.size()) {
        throw std::invalid_argument("Invalid stage index");
    }
    if (su == 0) {
        return;
    }

    const double B_after = std::max(0.0, biomass[su]);
    const double delta = std::max(0.0, biomass_increment_this_cycle);
    const double B_before_growth = std::max(0.0, B_after - delta);

    const std::vector<double>& fertility = getMaxFertility();
    const double f = su < fertility.size() ? std::clamp(fertility[su], 0.0, 1.0) : 0.0;

    const std::vector<double>& max_growth = getMaxIndividualGrowth();
    const double g_max = su < max_growth.size() ? std::clamp(max_growth[su], 0.0, 1.0) : 0.0;

    constexpr double kEps = 1e-12;
    const double cap = g_max * B_before_growth;
    const double r = cap > kEps ? std::clamp(delta / cap, 0.0, 1.0) : 0.0;

    const double stochastic_factor =
        std::max(0.0, 1.0 + utilities::randomNormal(0.0, SimulationConfig::global().noise_stddev));
    double reproductive_biomass = B_after * f * r * stochastic_factor;
    reproductive_biomass = std::clamp(reproductive_biomass, 0.0, B_after);
    if (reproductive_biomass <= 0.0) {
        return;
    }

    biomass[su] = std::max(0.0, biomass[su] - reproductive_biomass);
    biomass[0] += reproductive_biomass;
    cohort.setBiomass(std::move(biomass));
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


