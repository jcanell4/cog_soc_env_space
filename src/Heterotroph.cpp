#include "Heterotroph.h"

#include "Cohort.h"
#include "Constants.h"
#include "Niche.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

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

    const std::vector<double>& maintenance = specie->getMaintenanceCost();
    const double m_stage = su < maintenance.size() ? std::clamp(maintenance[su], 0.0, 1.0) : 0.0;
    const auto& hetero = static_cast<const Heterotroph&>(*specie);
    const std::vector<double>& assimilation = hetero.getAssimilationEfficiency();
    const std::vector<std::vector<double>>& residue_by_size = hetero.getIngestionResidueFractionBySize();
    const std::vector<double>& max_growth = specie->getMaxIndividualGrowth();
    const std::vector<double>& prospecting = hetero.getProspectingAbilityRate();
    const auto& diet_by_stage = specie->getDietByCohortIndex();
    if (su >= diet_by_stage.size()) {
        return;
    }
    const std::vector<std::tuple<int, int, int>>& stage_diet = diet_by_stage[su];

    const double old_predator_biomass = std::max(0.0, predator_biomass[su]);
    const double assimilation_stage = su < assimilation.size() ? std::clamp(assimilation[su], 0.0, 1.0) : 0.0;
    const double max_growth_stage = su < max_growth.size() ? std::clamp(max_growth[su], 0.0, 1.0) : 0.0;
    const double movement_rate_stage = su < prospecting.size() ? std::max(0.0, prospecting[su]) : 0.0;

    constexpr double kEps = 1e-12;
    const double niche_surface = std::max(kEps, niche.getSurface());
    const double x_scan = movement_rate_stage / niche_surface;
    const double f_scan = std::clamp(1.0 - std::exp(-PROSPECTING_SCAN_SHARPNESS * x_scan), 0.0, 1.0);

    const double max_net_growth = old_predator_biomass * max_growth_stage;
    const double max_gross_ingestion = assimilation_stage > kEps ? (max_net_growth / assimilation_stage) : 0.0;

    const auto& recruitment_matrix = specie->getRecruitmentStrategies();
    const std::vector<double> recruitment_stage =
        su < recruitment_matrix.size() ? recruitment_matrix[su] : std::vector<double>{};

    struct CaptureItem {
        std::size_t prey_cohort_index{0};
        std::size_t prey_stage{0};
        double available{0.0};
        double theory{0.0};
        double take{0.0};
        double take_effective{0.0};
    };
    std::map<std::pair<std::size_t, std::size_t>, CaptureItem> capture_map;
    bool has_parental_supply_in_diet = false;

    // Pass 1: compute theoretical prey captures from find/capture probabilities.
    Niche::CohortSet& cohorts = niche.getCohortSet();
    for (const auto& rule : stage_diet) {
        const int food_index = std::get<0>(rule);
        if (food_index == DietType::PARENTAL_SUPPLY_TYPE) {
            has_parental_supply_in_diet = true;
            continue;
        }
        if (food_index < 0) {
            continue;
        }
        const std::size_t prey_cohort_index = static_cast<std::size_t>(food_index);
        if (0 > prey_cohort_index || prey_cohort_index >= cohorts.size()) {
            continue;
        }
        Cohort& prey_cohort = cohorts[prey_cohort_index];
        if (&prey_cohort == &cohort) {
            continue;
        }
        const LivingBeing* prey_specie = prey_cohort.getSpecie();
        if (prey_specie == nullptr) {
            continue;
        }

        const std::vector<double>& prey_biomass = prey_cohort.getBiomass();
        if (prey_biomass.empty()) {
            continue;
        }

        const int min_prey_stage = std::get<1>(rule);
        const int max_prey_stage = std::get<2>(rule);
        if (min_prey_stage > max_prey_stage) {
            continue;
        }

        const std::vector<double>& prey_biomass_per_individual = prey_specie->getBiomassPerIndividualAmount();
        const std::vector<double>& prey_individual_surface = prey_specie->getIndividualOccupiedSurface();
        const std::vector<std::vector<double>>& prey_defense = prey_specie->getDefenseStrategies();

        const double colony_rate = std::clamp(prey_specie->getColonyAbilityRate(), 0.0, 1.0);
        const double gamma = std::max(kEps, COLONY_MIX_GAMMA);
        //Effective weight of the colony in calculating the search probability
        const double w_colony = std::clamp(std::pow(colony_rate, gamma), 0.0, 1.0);

        // Whole-cohort colony footprint (all prey stages); diet range below limits predation only.
        double colony_area_base = 0.0;
        for (std::size_t stage = 0; stage < prey_biomass.size(); ++stage) {
            const double stage_biomass = std::max(0.0, prey_biomass[stage]);
            const double biomass_individual =
                stage < prey_biomass_per_individual.size() ? std::max(prey_biomass_per_individual[stage], kEps) : kEps;
            const double surface_individual =
                stage < prey_individual_surface.size() ? std::max(0.0, prey_individual_surface[stage]) : 0.0;
            const double individuals = stage_biomass / biomass_individual;
            colony_area_base += individuals * surface_individual;
        }
        const double colony_area_effective = std::min(
            niche_surface, colony_area_base * (1.0 + COLONY_SURFACE_GAIN_ETA * colony_rate));
        const double c_colony = std::clamp(colony_area_effective / niche_surface, 0.0, 1.0);
        const double p_colony = std::clamp(1.0 - std::pow(1.0 - c_colony, f_scan), 0.0, 1.0);

        for (int stage = min_prey_stage; stage <= max_prey_stage; ++stage) {
            if (stage < 0) {
                continue;
            }
            const std::size_t prey_stage = static_cast<std::size_t>(stage);
            if (prey_stage >= prey_biomass.size()) {
                continue;
            }
            const double prey_stage_biomass = std::max(0.0, prey_biomass[prey_stage]);
            if (prey_stage_biomass <= 0.0) {
                continue;
            }

            const double biomass_individual =
                prey_stage < prey_biomass_per_individual.size() ? std::max(prey_biomass_per_individual[prey_stage], kEps)
                                                                : kEps;
            const double surface_individual =
                prey_stage < prey_individual_surface.size() ? std::max(0.0, prey_individual_surface[prey_stage]) : 0.0;
            const double prey_individuals = prey_stage_biomass / biomass_individual;
            const double stage_area_base = prey_individuals * surface_individual;
            const double stage_area_effective =
                std::min(niche_surface, stage_area_base * (1.0 + COLONY_SURFACE_GAIN_ETA * colony_rate));
            const double c_individual = std::clamp(stage_area_effective / niche_surface, 0.0, 1.0);
            const double p_individual = std::clamp(1.0 - std::pow(1.0 - c_individual, f_scan), 0.0, 1.0);
            const double p_find = std::clamp((1.0 - w_colony) * p_individual + w_colony * p_colony, 0.0, 1.0);

            const std::vector<double> defense_row =
                prey_stage < prey_defense.size() ? prey_defense[prey_stage] : std::vector<double>{};
            const double capture_efficiency = std::clamp(
                LivingBeing::calculate_effective_recruitment_efficiency(recruitment_stage, defense_row), 0.0, 1.0);
            const double w_capture = std::clamp(p_find * capture_efficiency, 0.0, 1.0);
            const double theory_capture = std::min(prey_stage_biomass, w_capture * prey_stage_biomass);
            if (theory_capture <= 0.0) {
                continue;
            }

            const auto key = std::make_pair(prey_cohort_index, prey_stage);
            auto& item = capture_map[key];
            item.prey_cohort_index = prey_cohort_index;
            item.prey_stage = prey_stage;
            item.available = prey_stage_biomass;
            item.theory = std::min(prey_stage_biomass, item.theory + theory_capture);
        }
    }

    double theory_total = 0.0;
    for (auto& [_, item] : capture_map) {
        item.theory = std::clamp(item.theory, 0.0, item.available);
        theory_total += item.theory;
    }

    // Pass 2: normalize theoretical captures to global growth-limited ingestion.
    double alpha = 0.0;
    if (theory_total > kEps && max_gross_ingestion > kEps) {
        alpha = std::min(1.0, max_gross_ingestion / theory_total);
    }

    double gross_intake = 0.0;
    for (auto& [_, item] : capture_map) {
        item.take = alpha * item.theory;
        gross_intake += item.take;
    }

    // Apply realized captures to prey cohorts and route non-assimilated intake to prey dead biomass.
    double total_removed_from_prey = 0.0;
    for (auto& [_, item] : capture_map) {
        if (item.take <= 0.0) {
            continue;
        }
        item.take_effective = item.take;
        if (item.take_effective <= 0.0) {
            continue;
        }

        Cohort& prey_cohort = cohorts[item.prey_cohort_index];
        std::vector<double> prey_biomass = prey_cohort.getBiomass();
        if (item.prey_stage >= prey_biomass.size()) {
            continue;
        }
        const double available_now = std::max(0.0, prey_biomass[item.prey_stage]);
        const double take_now = std::min(available_now, item.take_effective);
        if (take_now <= 0.0) {
            continue;
        }
        prey_biomass[item.prey_stage] = std::max(0.0, available_now - take_now);
        prey_cohort.setBiomass(std::move(prey_biomass));

        const double waste = (1.0 - assimilation_stage) * take_now;
        ConsumerLivingBeing::addWasteToDeathBins(prey_cohort, residue_by_size, su, waste);

        total_removed_from_prey += take_now;
    }

    const double parental_supply_gross = ConsumerLivingBeing::applyParentalSupplyGross(
        has_parental_supply_in_diet, predator_biomass, su, max_gross_ingestion, total_removed_from_prey, *specie);

    const double total_gross_intake = total_removed_from_prey + parental_supply_gross;
    const double assimilated = assimilation_stage * total_gross_intake;
    const double parental_waste = (1.0 - assimilation_stage) * parental_supply_gross;
    ConsumerLivingBeing::addWasteToDeathBins(cohort, residue_by_size, su, parental_waste);

    const double maintenance_cost = old_predator_biomass * m_stage;
    predator_biomass[su] = std::max(0.0, old_predator_biomass + assimilated - maintenance_cost);
    cohort.setBiomass(std::move(predator_biomass));
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
    std::vector<std::vector<std::tuple<int, int, int>>> cohort_diet_by_stage = getDietByCohortIndex();
    cohort_diet_by_stage.resize(stage_count);
    const Niche::CohortSet& cohorts = niche.getCohortSet();
    const auto append_unique_rule =
        [](std::vector<std::tuple<int, int, int>>& stage_diet, const std::tuple<int, int, int>& rule) {
            if (std::find(stage_diet.begin(), stage_diet.end(), rule) == stage_diet.end()) {
                stage_diet.push_back(rule);
            }
        };
    for (std::size_t stage = 0; stage < stage_count; ++stage) {
        std::vector<std::tuple<int, int, int>>& stage_diet = cohort_diet_by_stage[stage];
        for (std::size_t i = 0; i < cohorts.size(); ++i) {
            const LivingBeing* prey_species = cohorts[i].getSpecie();
            if (prey_species == nullptr) {
                continue;
            }
            const std::tuple<int, int> range =
                getRangeForFoodType(prey_species->getFoodType(), static_cast<int>(stage));
            const int min_st = std::get<0>(range);
            const int max_st = std::get<1>(range);
            if (min_st < 0 || max_st < 0) {
                continue;
            }
            append_unique_rule(stage_diet, std::make_tuple(static_cast<int>(i), min_st, max_st));
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

Heterotroph& Heterotroph::setProspectingAbilityRate(std::vector<double> values) {
    ConsumerLivingBeing::setProspectingAbilityRate(std::move(values));
    return *this;
}

Heterotroph& Heterotroph::setAssimilationEfficiency(std::vector<double> values) {
    ConsumerLivingBeing::setAssimilationEfficiency(std::move(values));
    return *this;
}

Heterotroph& Heterotroph::setIngestionResidueFractionBySize(std::vector<std::vector<double>> values) {
    ConsumerLivingBeing::setIngestionResidueFractionBySize(std::move(values));
    return *this;
}
