#include "ConsumerLivingBeing.h"

#include "Cohort.h"
#include "Constants.h"
#include "DietFoodTypeMatch.h"
#include "SimulationConfig.h"
#include "Utilities.h"

#include <algorithm>
#include <numeric>

const std::vector<double>& ConsumerLivingBeing::getProspectingAbilityRate() const {
    return prospecting_ability_rate_;
}

const std::vector<double>& ConsumerLivingBeing::getHandlingTimePenalty() const {
    return handling_time_penalty_;
}

const std::vector<double>& ConsumerLivingBeing::getAssimilationEfficiency() const {
    return assimilation_efficiency_;
}

const std::vector<std::vector<double>>& ConsumerLivingBeing::getIngestionResidueFractionBySize() const {
    return ingestion_residue_fraction_by_size_;
}

const std::vector<std::tuple<std::string, int, int>>& ConsumerLivingBeing::getDietByFoodType() const {
    return diet_by_food_type_;
}

void ConsumerLivingBeing::setDietByFoodType(std::vector<std::tuple<std::string, int, int>> diet_by_food_type) {
    diet_by_food_type_ = std::move(diet_by_food_type);
}

bool ConsumerLivingBeing::isFoodTypeMyDiet(const std::string& prey_food_type, int prey_stage) const {
    return diet_food_type_match::isFoodTypeMyDiet(diet_by_food_type_, prey_food_type, prey_stage);
}

std::tuple<int, int> ConsumerLivingBeing::getRangeForFoodType(const std::string& prey_food_type) const {
    return diet_food_type_match::rangeForMatchingFoodType(diet_by_food_type_, prey_food_type);
}

std::vector<double> ConsumerLivingBeing::clampUnitInterval(std::vector<double> values) {
    for (double& value : values) {
        value = std::clamp(value, 0.0, 1.0);
    }
    return values;
}

std::vector<double> ConsumerLivingBeing::normalizeResidueRow(std::vector<double> row) {
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

ConsumerLivingBeing& ConsumerLivingBeing::setProspectingAbilityRate(std::vector<double> values) {
    prospecting_ability_rate_ = clampUnitInterval(std::move(values));
    return *this;
}

ConsumerLivingBeing& ConsumerLivingBeing::setHandlingTimePenalty(std::vector<double> values) {
    handling_time_penalty_ = clampUnitInterval(std::move(values));
    return *this;
}

ConsumerLivingBeing& ConsumerLivingBeing::setAssimilationEfficiency(std::vector<double> values) {
    assimilation_efficiency_ = clampUnitInterval(std::move(values));
    return *this;
}

ConsumerLivingBeing& ConsumerLivingBeing::setIngestionResidueFractionBySize(
    std::vector<std::vector<double>> values) {
    for (std::vector<double>& row : values) {
        row = normalizeResidueRow(std::move(row));
    }
    ingestion_residue_fraction_by_size_ = std::move(values);
    return *this;
}

void ConsumerLivingBeing::addWasteToDeathBins(Cohort& target,
                                              const std::vector<std::vector<double>>& residue_matrix,
                                              std::size_t stage_index,
                                              double waste) {
    if (waste <= 0.0) {
        return;
    }
    std::vector<double> death = target.getDeathBiomass();
    const std::vector<double> distribution =
        stage_index < residue_matrix.size() ? residue_matrix[stage_index] : std::vector<double>{};
    if (distribution.empty()) {
        if (death.empty()) {
            death.resize(DEATH_BIOMASS_FINEST_BIN + 1, 0.0);
        }
        death.back() += waste;
        target.setDeathBiomass(std::move(death));
        return;
    }

    if (death.size() < distribution.size()) {
        death.resize(distribution.size(), 0.0);
    }
    for (std::size_t i = 0; i < distribution.size(); ++i) {
        death[i] += waste * distribution[i];
    }
    target.setDeathBiomass(std::move(death));
}

double ConsumerLivingBeing::applyParentalSupplyGross(const bool has_parental_supply_in_diet,
                                                     std::vector<double>& consumer_biomass,
                                                     const std::size_t su,
                                                     const double max_gross_ingestion,
                                                     const double total_from_primary_sources,
                                                     const LivingBeing& specie) {
    constexpr double kEps = 1e-12;
    double parental_supply_gross = 0.0;
    if (!has_parental_supply_in_diet || max_gross_ingestion <= total_from_primary_sources + kEps) {
        return parental_supply_gross;
    }

    const std::vector<double>& max_fertility = specie.getMaxFertility();
    const double recipient_fertility = su < max_fertility.size() ? std::clamp(max_fertility[su], 0.0, 1.0) : 0.0;
    if (recipient_fertility > 0.0) {
        return parental_supply_gross;
    }

    const double remaining_gross_need = std::max(0.0, max_gross_ingestion - total_from_primary_sources);
    const double stochastic_factor =
        std::clamp(1.0 + utilities::randomNormal(0.0, SimulationConfig::global().noise_stdv), 0.0, 1.0);
    const double target_parental_take = remaining_gross_need * stochastic_factor;
    if (target_parental_take <= kEps) {
        return parental_supply_gross;
    }

    double fertility_weight_sum = 0.0;
    for (std::size_t stage = 0; stage < consumer_biomass.size(); ++stage) {
        if (stage == su || stage >= max_fertility.size()) {
            continue;
        }
        const double fertility = std::clamp(max_fertility[stage], 0.0, 1.0);
        if (fertility <= 0.0 || consumer_biomass[stage] <= 0.0) {
            continue;
        }
        fertility_weight_sum += fertility;
    }

    if (fertility_weight_sum <= kEps) {
        return parental_supply_gross;
    }

    double scale = 1.0;
    for (std::size_t stage = 0; stage < consumer_biomass.size(); ++stage) {
        if (stage == su || stage >= max_fertility.size()) {
            continue;
        }
        const double fertility = std::clamp(max_fertility[stage], 0.0, 1.0);
        if (fertility <= 0.0 || consumer_biomass[stage] <= 0.0) {
            continue;
        }
        const double requested = target_parental_take * fertility / fertility_weight_sum;
        if (requested > kEps) {
            scale = std::min(scale, consumer_biomass[stage] / requested);
        }
    }
    scale = std::clamp(scale, 0.0, 1.0);

    for (std::size_t stage = 0; stage < consumer_biomass.size(); ++stage) {
        if (stage == su || stage >= max_fertility.size()) {
            continue;
        }
        const double fertility = std::clamp(max_fertility[stage], 0.0, 1.0);
        if (fertility <= 0.0 || consumer_biomass[stage] <= 0.0) {
            continue;
        }
        const double requested = target_parental_take * fertility / fertility_weight_sum;
        const double take = std::min(consumer_biomass[stage], scale * requested);
        if (take <= 0.0) {
            continue;
        }
        consumer_biomass[stage] = std::max(0.0, consumer_biomass[stage] - take);
        parental_supply_gross += take;
    }
    return parental_supply_gross;
}
