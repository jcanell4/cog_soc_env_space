#include "Heterotroph.h"

#include "Cohort.h"
#include "Constants.h"
#include "DietFoodTypeMatch.h"
#include "Niche.h"
#include "SimulationConfig.h"
#include "Utilities.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

const std::vector<double> kEmptyDoubleRow{};
constexpr double kMaxDecompositionRate = 1.0;
constexpr double kEps = 1e-12;

/** Key: (matter_type, source_cohort_index, prey_stage_or_death_bin). */
using IngestionMapKey = std::tuple<int, std::size_t, std::size_t>;
/** (matter_type, cohort_index, stage_or_death_bin, capture_potential, theoretical_capture, maximum_capture). */
using CaptureItem = std::tuple<int, std::size_t, std::size_t, double, double, double>;

const int MATTER_TYPE = 0;
const int PREY_COHOR_INDEX = 1;
const int PREY_STAGE = 2;
const int CAPTURE_POTENTIAL = 3;
const int THEORETICAL_CAPTURE = 4;
const int MAXIMUM_CAPTURE = 5;
const int DONOR_COHOR_INDEX = 1;
const int DONOR_DEATH_BIN = 2;

using CaptureMap = std::map<IngestionMapKey, CaptureItem>;
/** Same storage as @ref CaptureMap; enables a safe reference cast at mixed diet call sites. */
using DetritusMap = CaptureMap;

struct PreyWorkingState {
    std::vector<double> biomass;
    std::vector<double> death;
};

using PreyWorkingMap = std::unordered_map<std::size_t, PreyWorkingState>;

PreyWorkingMap buildPreyWorkingMap(Niche::CohortSet& cohorts, const CaptureMap& capture_map) {
    PreyWorkingMap out;
    for (const auto& [key, item] : capture_map) {
        (void)item;
        const std::size_t cidx = std::get<1>(key);
        if (cidx >= cohorts.size()) {
            continue;
        }
        out.try_emplace(cidx, PreyWorkingState{cohorts[cidx].getBiomass(), cohorts[cidx].getDeathBiomass()});
    }
    return out;
}

double initialPoolForKey(const PreyWorkingMap& prey_work, const IngestionMapKey& key) {
    const int matter = std::get<0>(key);
    const std::size_t cidx = std::get<1>(key);
    const std::size_t idx = std::get<2>(key);
    const auto it = prey_work.find(cidx);
    if (it == prey_work.end()) {
        return 0.0;
    }
    if (matter == MatterType::DEAD) {
        return idx < it->second.death.size() ? it->second.death[idx] : 0.0;
    }
    return idx < it->second.biomass.size() ? it->second.biomass[idx] : 0.0;
}

/** Same residue routing as @ref Heterotroph::addWasteToDeathBins, on a mutable death vector. */
void addWasteToDeathVector(std::vector<double>& death,
                           const std::vector<std::vector<double>>& residue_matrix,
                           std::size_t stage_index,
                           double waste) {
    if (waste <= 0.0) {
        return;
    }
    const std::vector<double> distribution =
        stage_index < residue_matrix.size() ? residue_matrix[stage_index] : std::vector<double>{};
    if (distribution.empty()) {
        if (death.empty()) {
            death.resize(DEATH_BIOMASS_FINEST_BIN + 1, 0.0);
        }
        death.back() += waste;
        return;
    }
    if (death.size() < distribution.size()) {
        death.resize(distribution.size(), 0.0);
    }
    for (std::size_t i = 0; i < distribution.size(); ++i) {
        death[i] += waste * distribution[i];
    }
}

/** Per-prey-stage encounter rate and available living biomass for the capture map. */
void calculateAvailabilityForLivingBiomass(int min_prey_stage,
                                            int max_prey_stage,
                                            const Heterotroph& hetero,
                                            std::size_t consumer_stage,
                                            const Niche& niche,
                                            std::size_t prey_cohort_index,
                                            CaptureMap& capture_map,
                                            double& total_capture_potential) {
    const Niche::CohortSet& cohorts = niche.getCohortSet();
    if (prey_cohort_index >= cohorts.size()) {
        return;
    }
    const Cohort& prey_cohort = cohorts[prey_cohort_index];
    const LivingBeing* prey_specie = prey_cohort.getSpecie();
    if (prey_specie == nullptr) {
        return;
    }
    for (int stage = min_prey_stage; stage <= max_prey_stage; ++stage) {
        if (stage < 0) {
            continue;
        }
        const std::size_t prey_stage = static_cast<std::size_t>(stage);
        double prey_stage_biomass = prey_cohort.getBiomass(prey_stage, 0.0);
        double prey_individual_detection_area = prey_specie->getIndividualOccupiedSurface(prey_stage, 0.0);
        double prey_biomass_individual = prey_specie->getBiomassPerIndividualAmount(prey_stage, 0.0);
        double prey_detection_area_rate =
            std::pow((prey_individual_detection_area * prey_stage_biomass / prey_biomass_individual) /
                         niche.getSurface(),
                     prey_specie->getColonyAbilityRate() * 2 + 1);
        double prey_detection_area_rate_eff =
            prey_detection_area_rate *
            (1 + hetero.getPreyLocation(consumer_stage, 0.0) * niche.getProspectingScanSharpness());
        const double capture_efficiency = LivingBeing::calculate_effective_recruitment_efficiency(
            hetero.getRecruitmentStrategies(consumer_stage), prey_specie->getDefenseStrategies(prey_stage));
        double capture_potential =
            hetero.getProspectingAbility(consumer_stage, 0.0) * prey_detection_area_rate_eff * capture_efficiency;
        total_capture_potential += capture_potential;
        const auto key = std::make_tuple(MatterType::LIVING, prey_cohort_index, prey_stage);
        auto& item = capture_map[key];
        item = CaptureItem(MatterType::LIVING, prey_cohort_index, prey_stage, capture_potential, 0.0, 0.0);
    }
}

/** Per death-bin theoretical uptake and available dead biomass (same role as @ref calculateAvailabilityForLivingBiomass). */
void calculateAvailabilityForDeadBiomass(int min_prey_death_bin,
                                          int max_prey_death_bin,
                                          const Heterotroph& hetero,
                                          std::size_t consumer_stage,
                                          const Niche& niche,
                                          std::size_t donor_cohort_index,
                                          DetritusMap& detritus_map,
                                          double& total_capture_potential) {
    const Niche::CohortSet& cohorts = niche.getCohortSet();
    if (donor_cohort_index >= cohorts.size()) {
        return;
    }
    const Cohort& donor_cohort = cohorts[donor_cohort_index];
    const LivingBeing* donor_specie = donor_cohort.getSpecie();
    if (donor_specie == nullptr) {
        return;
    }
    for (int bin = min_prey_death_bin; bin <= max_prey_death_bin; ++bin) {
        if (bin < 0) {
            continue;
        }
        const std::size_t prey_bin = static_cast<std::size_t>(bin);
        double detah_bin_biomass = donor_cohort.getDeathBiomass(prey_bin, 0.0);
        double death_bin_detection_area = donor_specie->getDeathBiomassFractionSurface(prey_bin, 0.0);
        double death_bin_biomass_fraction = donor_specie->getDeathBiomassPerFractionAmount(prey_bin, 0.0);
        if (death_bin_biomass_fraction <= kEps) {
            continue;
        }
        double death_bin_detection_area_rate =
            std::pow((death_bin_detection_area * detah_bin_biomass / death_bin_biomass_fraction) /
                            niche.getSurface(),
                        donor_specie->getColonyAbilityRate() * 2 + 1);
        double death_bin_detection_area_rate_eff =
            death_bin_detection_area_rate *
            (1 + hetero.getPreyLocation(consumer_stage, 0.0) * niche.getProspectingScanSharpness());
        const double capture_efficiency = LivingBeing::calculate_effective_recruitment_efficiency(
            hetero.getRecruitmentStrategies(consumer_stage), donor_specie->getCharacteristicsDeathBiomass(prey_bin));
        double capture_potential =
            hetero.getProspectingAbility(consumer_stage, 0.0) * death_bin_detection_area_rate_eff * capture_efficiency;
        total_capture_potential += capture_potential;
        const auto key = std::make_tuple(MatterType::DEAD, donor_cohort_index, prey_bin);
        auto& item = detritus_map[key];
        item = CaptureItem(MatterType::DEAD, donor_cohort_index, prey_bin, capture_potential, 0.0, 0.0);
    }
}

bool stageDietHasDeadMatter(const std::vector<std::tuple<int, int, int, int>>& stage_diet) {
    for (const auto& rule : stage_diet) {
        if (std::get<3>(rule) == MatterType::DEAD) {
            const int food_index = std::get<0>(rule);
            if (food_index >= 0) {
                return true;
            }
        }
    }
    return false;
}

}  // namespace

Heterotroph::Heterotroph() {
    setFoodType(std::string{FoodType::ANIMAL});
}

void Heterotroph::initialize(const Niche& niche) {
    LivingBeing::initialize(niche);
    rebuild_diet_by_cohort_index_from_food_type(niche);
}

void Heterotroph::process_individual_growth(Niche& niche, Cohort& cohort, int stage_index) const {
    LivingBeing::process_individual_growth(niche, cohort, stage_index);
    const LivingBeing* specie = cohort.getSpecie();
    if (specie == nullptr || specie->getClassType() != LivingBeingClassType::HETEROTROPH) {
        return;
    }
    const std::size_t su = static_cast<std::size_t>(stage_index);
    std::vector<double> predator_biomass = cohort.getBiomass();
    if (stage_index < 0 || su >= predator_biomass.size()) {
        return;
    }
    const auto& hetero = static_cast<const Heterotroph&>(*specie);
    const double m_stage = std::clamp(specie->getMaintenanceCost(su, 0.0), 0.0, 1.0);
    const double assimilation_stage = std::clamp(hetero.getAssimilationEfficiency(su, 0.0), 0.0, 1.0);
    const std::vector<std::vector<double>>& residue_by_size = hetero.getIngestionResidueFractionBySize();
    const double max_growth_stage = std::clamp(specie->getMaxIndividualGrowth(su, 0.0), 0.0, 1.0);

    const auto& diet_by_stage = specie->getDietByCohortIndex();
    if (su >= diet_by_stage.size()) {
        return;
    }
    Niche::CohortSet& cohorts = niche.getCohortSet();
    bool has_parental_supply_in_diet = false;
    const std::vector<std::tuple<int, int, int, int>>& stage_diet = diet_by_stage[su];

    const double total_density = niche.getBiomassForDietIndex(stage_diet) / niche.getSurface();
    const double max_density = specie->getMaxDensity(su, 0.0);
    const double pcpacity = std::clamp(1.0 - (total_density / max_density), 0.0, 1.0);

    double total_capture_potential = 0.0;

    CaptureMap capture_map;

    for (const auto& rule : stage_diet) {
        const int food_index = std::get<0>(rule);
        const int min_prey_stage = std::get<1>(rule);
        const int max_prey_stage = std::get<2>(rule);
        const int matter_type = std::get<3>(rule);
        if (food_index == DietType::PARENTAL_SUPPLY_TYPE) {
            has_parental_supply_in_diet = true;
            continue;
        }
        if (food_index < 0) {
            continue;
        }
        const std::size_t prey_cohort_index = static_cast<std::size_t>(food_index);
        if (prey_cohort_index >= cohorts.size()) {
            continue;
        }

        if (matter_type == MatterType::LIVING) {
            calculateAvailabilityForLivingBiomass(min_prey_stage,
                                                  max_prey_stage,
                                                  hetero,
                                                  su,
                                                  niche,
                                                  prey_cohort_index,
                                                  capture_map,
                                                  total_capture_potential);
        } else if (matter_type == MatterType::DEAD) {
            calculateAvailabilityForDeadBiomass(min_prey_stage,
                                                  max_prey_stage,
                                                  hetero,
                                                  su,
                                                  niche,
                                                  prey_cohort_index,
                                                  capture_map,
                                                  total_capture_potential);
        }
    }

    PreyWorkingMap prey_work = buildPreyWorkingMap(cohorts, capture_map);

    std::size_t consumer_cohort_index = cohorts.size();
    for (std::size_t i = 0; i < cohorts.size(); ++i) {
        if (&cohorts[i] == &cohort) {
            consumer_cohort_index = i;
            break;
        }
    }

    double theoretical_accumulated = 0.0;
    double maximum_accumulated = 0.0;
    double surplus = 0.0;

    double total_growth_rate = 1 - std::exp(-total_capture_potential);

    if (total_capture_potential > kEps) {
        for (auto& [key, item] : capture_map) {
            const double pool = initialPoolForKey(prey_work, key);
            double theoretical_capture = predator_biomass[su] * max_growth_stage * total_growth_rate * pcpacity *
                                         (std::get<CAPTURE_POTENTIAL>(item) / total_capture_potential);
            double old_maximum_capture = std::get<MAXIMUM_CAPTURE>(item);
            std::get<THEORETICAL_CAPTURE>(item) += theoretical_capture;
            std::get<MAXIMUM_CAPTURE>(item) = std::min(std::get<THEORETICAL_CAPTURE>(item), pool);
            theoretical_accumulated += theoretical_capture;
            maximum_accumulated += std::get<MAXIMUM_CAPTURE>(item) - old_maximum_capture;
            surplus += pool - std::get<MAXIMUM_CAPTURE>(item);
        }
    }

    double missing = theoretical_accumulated - maximum_accumulated;

    if (missing > 0.0 && surplus > kEps) {
        for (auto& [key, item] : capture_map) {
            const double pool = initialPoolForKey(prey_work, key);
            if (pool - std::get<MAXIMUM_CAPTURE>(item) > 0.0) {
                std::get<THEORETICAL_CAPTURE>(item) +=
                    (pool - std::get<MAXIMUM_CAPTURE>(item)) * missing / surplus;
                std::get<MAXIMUM_CAPTURE>(item) = std::min(std::get<THEORETICAL_CAPTURE>(item), pool);
            }
        }
    }

    double total_ingested_from_prey = 0.0;
    for (const auto& [key, item] : capture_map) {
        (void)key;
        const std::size_t cidx = std::get<PREY_COHOR_INDEX>(item);
        const double amount = std::get<MAXIMUM_CAPTURE>(item);
        const double waste = (1.0 - assimilation_stage) * amount;
        PreyWorkingState& w = prey_work.at(cidx);
        if (std::get<MATTER_TYPE>(item) == MatterType::DEAD) {
            const std::size_t bin = std::get<DONOR_DEATH_BIN>(item);
            if (bin < w.death.size()) {
                w.death[bin] = std::max(0.0, w.death[bin] - amount);
            }
        } else {
            const std::size_t st = std::get<PREY_STAGE>(item);
            if (st < w.biomass.size()) {
                w.biomass[st] = std::max(0.0, w.biomass[st] - amount);
            }
        }
        addWasteToDeathVector(w.death, residue_by_size, su, waste);
        total_ingested_from_prey += amount;
    }
    const double parental_supply_gross = Heterotroph::applyParentalSupplyGross(
        has_parental_supply_in_diet, predator_biomass, su, theoretical_accumulated, total_ingested_from_prey, *specie);

    const double total_gross_intake = total_ingested_from_prey + parental_supply_gross;
    const double parental_waste = (1.0 - assimilation_stage) * parental_supply_gross;
    if (consumer_cohort_index < cohorts.size() && prey_work.count(consumer_cohort_index) > 0) {
        addWasteToDeathVector(prey_work[consumer_cohort_index].death, residue_by_size, su, parental_waste);
    } else {
        Heterotroph::addWasteToDeathBins(cohort, residue_by_size, su, parental_waste);
    }

    const double assimilated = assimilation_stage * total_gross_intake;
    const double maintenance_cost = predator_biomass[su] * m_stage;
    predator_biomass[su] = std::max(0.0, predator_biomass[su] + assimilated - maintenance_cost);

    if (consumer_cohort_index < cohorts.size() && prey_work.count(consumer_cohort_index) > 0 &&
        su < prey_work[consumer_cohort_index].biomass.size()) {
        prey_work[consumer_cohort_index].biomass[su] = predator_biomass[su];
    }

    for (auto& [idx, w] : prey_work) {
        cohorts[idx].setBiomass(std::move(w.biomass));
        cohorts[idx].setDeathBiomass(std::move(w.death));
    }
    if (consumer_cohort_index >= cohorts.size() || prey_work.find(consumer_cohort_index) == prey_work.end()) {
        cohort.setBiomass(std::move(predator_biomass));
    }
    niche.setNutrients(niche.getNutrients() + maintenance_cost);
}

void Heterotroph::process_reproductive_growth(Cohort& cohort,
                                              int stage_index,
                                              double stage_biomass_before_growth,
                                              double biomass_increment_this_cycle) const {
    LivingBeing::process_reproductive_growth(
        cohort, stage_index, stage_biomass_before_growth, biomass_increment_this_cycle);
}

int Heterotroph::getClassType() const {
    return LivingBeingClassType::HETEROTROPH;
}

void Heterotroph::setCyclesPerStages(std::vector<int> cycles_per_stages) {
    LivingBeing::setCyclesPerStages(std::move(cycles_per_stages));
}

void Heterotroph::rebuild_diet_by_cohort_index_from_food_type(const Niche& niche) {
    const std::size_t stage_count = getCyclesPerStages().size();
    std::vector<std::vector<std::tuple<int, int, int, int>>> cohort_diet_by_stage = getDietByCohortIndex();
    cohort_diet_by_stage.resize(stage_count);
    const Niche::CohortSet& cohorts = niche.getCohortSet();
    const auto append_unique_rule =
        [](std::vector<std::tuple<int, int, int, int>>& stage_diet, const std::tuple<int, int, int, int>& rule) {
            if (std::find(stage_diet.begin(), stage_diet.end(), rule) == stage_diet.end()) {
                stage_diet.push_back(rule);
            }
        };
    for (std::size_t stage = 0; stage < stage_count; ++stage) {
        std::vector<std::tuple<int, int, int, int>>& stage_diet = cohort_diet_by_stage[stage];
        for (std::size_t i = 0; i < cohorts.size(); ++i) {
            const LivingBeing* prey_species = cohorts[i].getSpecie();
            if (prey_species == nullptr) {
                continue;
            }
            const std::tuple<int, int, int> range =
                getRangeForFoodType(prey_species->getFoodType(), static_cast<int>(stage));
            const int min_st = std::get<0>(range);
            const int max_st = std::get<1>(range);
            const int matter_type = std::get<2>(range);
            if (min_st < 0 || max_st < 0) {
                continue;
            }
            append_unique_rule(stage_diet, std::make_tuple(static_cast<int>(i), min_st, max_st, matter_type));
        }
    }
    setDietByCohortIndex(std::move(cohort_diet_by_stage));
}

Heterotroph& Heterotroph::setName(std::string name) {
    LivingBeing::setName(std::move(name));
    return *this;
}

Heterotroph& Heterotroph::setEnergyContent(float energy_content) {
    setBiomassToEnergyConversionFactor(energy_content);
    return *this;
}

Heterotroph& Heterotroph::setProspectingAbility(std::vector<double> values) {
    for (double& value : values) {
        value = std::max(0.0, value);
    }
    prospecting_ability_ = std::move(values);
    return *this;
}

Heterotroph& Heterotroph::setProspectingAbilityRate(std::vector<double> values) {
    return setProspectingAbility(std::move(values));
}

Heterotroph& Heterotroph::setAssimilationEfficiency(std::vector<double> values) {
    assimilation_efficiency_ = clampUnitInterval(std::move(values));
    return *this;
}

Heterotroph& Heterotroph::setIngestionResidueFractionBySize(std::vector<std::vector<double>> values) {
    for (std::vector<double>& row : values) {
        row = normalizeResidueRow(std::move(row));
    }
    ingestion_residue_fraction_by_size_ = std::move(values);
    return *this;
}

const std::vector<double>& Heterotroph::getProspectingAbility() const {
    return prospecting_ability_;
}

double Heterotroph::getProspectingAbility(std::size_t index, double out_of_range_default) const {
    return index < prospecting_ability_.size() ? prospecting_ability_[index] : out_of_range_default;
}

const std::vector<double>& Heterotroph::getAssimilationEfficiency() const {
    return assimilation_efficiency_;
}

double Heterotroph::getAssimilationEfficiency(std::size_t index, double out_of_range_default) const {
    return index < assimilation_efficiency_.size() ? assimilation_efficiency_[index] : out_of_range_default;
}

const std::vector<std::vector<double>>& Heterotroph::getIngestionResidueFractionBySize() const {
    return ingestion_residue_fraction_by_size_;
}

const std::vector<double>& Heterotroph::getIngestionResidueFractionBySize(std::size_t row_index) const {
    return row_index < ingestion_residue_fraction_by_size_.size() ? ingestion_residue_fraction_by_size_[row_index]
                                                                : kEmptyDoubleRow;
}

const std::vector<std::vector<std::tuple<std::string, int, int, int>>>& Heterotroph::getDietByFoodType() const {
    return diet_by_food_type_;
}

void Heterotroph::setDietByFoodType(
    std::vector<std::vector<std::tuple<std::string, int, int, int>>> diet_by_food_type) {
    diet_by_food_type_ = std::move(diet_by_food_type);
}

bool Heterotroph::isFoodTypeMyDiet(const std::string& prey_food_type,
                                   int consumer_stage,
                                   int prey_stage,
                                   int prey_matter_type) const {
    if (consumer_stage < 0) {
        return false;
    }
    const std::size_t stage_index = static_cast<std::size_t>(consumer_stage);
    if (stage_index >= diet_by_food_type_.size()) {
        return false;
    }
    return diet_food_type_match::isFoodTypeMyDiet(
        diet_by_food_type_[stage_index], prey_food_type, prey_stage, prey_matter_type);
}

std::tuple<int, int, int> Heterotroph::getRangeForFoodType(const std::string& prey_food_type,
                                                         int consumer_stage) const {
    if (consumer_stage < 0) {
        return {-1, -1, MatterType::LIVING};
    }
    const std::size_t stage_index = static_cast<std::size_t>(consumer_stage);
    if (stage_index >= diet_by_food_type_.size()) {
        return {-1, -1, MatterType::LIVING};
    }
    return diet_food_type_match::rangeForMatchingFoodType(diet_by_food_type_[stage_index], prey_food_type);
}

std::vector<double> Heterotroph::clampUnitInterval(std::vector<double> values) {
    for (double& value : values) {
        value = std::clamp(value, 0.0, 1.0);
    }
    return values;
}

std::vector<double> Heterotroph::normalizeResidueRow(std::vector<double> row) {
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

void Heterotroph::addWasteToDeathBins(Cohort& target,
                                      const std::vector<std::vector<double>>& residue_matrix,
                                      std::size_t stage_index,
                                      double waste) {
    if (waste <= 0.0) {
        return;
    }
    std::vector<double> death = target.getDeathBiomass();
    addWasteToDeathVector(death, residue_matrix, stage_index, waste);
    target.setDeathBiomass(std::move(death));
}

double Heterotroph::applyParentalSupplyGross(const bool has_parental_supply_in_diet,
                                             std::vector<double>& consumer_biomass,
                                             const std::size_t su,
                                             const double max_gross_ingestion,
                                             const double total_from_primary_sources,
                                             const LivingBeing& specie) {
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

const std::vector<double>& Heterotroph::getPreyLocation() const {
    return prey_location_;
}

double Heterotroph::getPreyLocation(std::size_t index, double out_of_range_default) const {
    return index < prey_location_.size() ? prey_location_[index] : out_of_range_default;
}

Heterotroph& Heterotroph::setPreyLocation(std::vector<double> values) {
    for (double& value : values) {
        value = std::max(1.0, value);
    }
    prey_location_ = std::move(values);
    if (prey_location_.empty()) {
        prey_location_.push_back(1.0);
    }
    return *this;
}
