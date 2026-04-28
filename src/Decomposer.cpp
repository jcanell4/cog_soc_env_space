#include "Decomposer.h"

#include "Cohort.h"
#include "Constants.h"
#include "Niche.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

namespace {
/** Per-step decomposition intensity factor in the theoretical uptake term (former default member value). */
constexpr double kMaxDecompositionRate = 1.0;
}  // namespace

Decomposer::Decomposer() {
    // No default taxonomic food_type: decomposers span fungi, animals, unicellulars, etc. — set from config.
    setFoodType(std::string{});
}

void Decomposer::initialize(const Niche& niche) {
    LivingBeing::initialize(niche);
    rebuild_diet_by_cohort_index_from_food_type(niche);
}

void Decomposer::process_individual_growth(Niche& niche, Cohort& cohort, int stage_index) const {
    LivingBeing::process_individual_growth(niche, cohort, stage_index);
    const LivingBeing* specie = cohort.getSpecie();
    if (specie == nullptr || specie->getClassType() != LivingBeingClassType::DECOMPOSER) {
        return;
    }
    if (stage_index < 0) {
        return;
    }
    const std::size_t su = static_cast<std::size_t>(stage_index);
    std::vector<double> decomposer_biomass = cohort.getBiomass();
    if (su >= decomposer_biomass.size()) {
        return;
    }

    const auto& decomp = static_cast<const Decomposer&>(*specie);
    const std::vector<double>& maintenance = specie->getMaintenanceCost();
    const double m_stage = su < maintenance.size() ? std::clamp(maintenance[su], 0.0, 1.0) : 0.0;
    const std::vector<double>& assimilation = decomp.getAssimilationEfficiency();
    const std::vector<std::vector<double>>& residue_by_size = decomp.getIngestionResidueFractionBySize();
    const std::vector<double>& max_growth = specie->getMaxIndividualGrowth();
    const std::vector<double>& prospecting = decomp.getProspectingAbilityRate();
    const auto& diet_by_stage = specie->getDietByCohortIndex();
    if (su >= diet_by_stage.size()) {
        return;
    }
    const std::vector<std::tuple<int, int, int>>& stage_diet = diet_by_stage[su];

    const double old_biomass = std::max(0.0, decomposer_biomass[su]);
    const double assimilation_stage = su < assimilation.size() ? std::clamp(assimilation[su], 0.0, 1.0) : 0.0;
    const double max_growth_stage = su < max_growth.size() ? std::clamp(max_growth[su], 0.0, 1.0) : 0.0;
    const double movement_rate_stage = su < prospecting.size() ? std::max(0.0, prospecting[su]) : 0.0;

    constexpr double kEps = 1e-12;
    const double niche_surface = std::max(kEps, niche.getSurface());
    const double x_scan = movement_rate_stage / niche_surface;
    const double f_scan = std::clamp(1.0 - std::exp(-PROSPECTING_SCAN_SHARPNESS * x_scan), 0.0, 1.0);

    const double max_net_growth = old_biomass * max_growth_stage;
    const double max_gross_ingestion = assimilation_stage > kEps ? (max_net_growth / assimilation_stage) : 0.0;

    const auto& recruitment_matrix = specie->getRecruitmentStrategies();
    const std::vector<double> recruitment_stage =
        su < recruitment_matrix.size() ? recruitment_matrix[su] : std::vector<double>{};

    struct DetritusItem {
        std::size_t donor_cohort_index{0};
        std::size_t bin_index{0};
        double available{0.0};
        double theory{0.0};
        double take{0.0};
        double take_effective{0.0};
        /** Realized removal from donor bin after pool clamping. */
        double realized_take{0.0};
    };
    std::map<std::pair<std::size_t, std::size_t>, DetritusItem> detritus_map;
    bool has_parental_supply_in_diet = false;

    Niche::CohortSet& cohorts = niche.getCohortSet();
    const double max_rate = kMaxDecompositionRate;

    // Pass 1: theoretical uptake per donor cohort and dead-biomass bin.
    for (const auto& rule : stage_diet) {
        const int food_index = std::get<0>(rule);
        if (food_index == DietType::PARENTAL_SUPPLY_TYPE) {
            has_parental_supply_in_diet = true;
            continue;
        }
        if (food_index < 0) {
            continue;
        }
        const std::size_t donor_cohort_index = static_cast<std::size_t>(food_index);
        if (donor_cohort_index >= cohorts.size()) {
            continue;
        }
        Cohort& donor_cohort = cohorts[donor_cohort_index];
        const LivingBeing* donor_specie = donor_cohort.getSpecie();
        if (donor_specie == nullptr) {
            continue;
        }

        const std::vector<double>& donor_death = donor_cohort.getDeathBiomass();
        if (donor_death.empty()) {
            continue;
        }
        const std::vector<std::vector<double>>& donor_traits = donor_specie->getCharacteristicsDeathBiomass();

        const int min_food_bin = std::get<1>(rule);
        const int max_food_bin = std::get<2>(rule);
        if (min_food_bin > max_food_bin) {
            continue;
        }
        const std::size_t n_bins = donor_death.size();
        const int max_bin_index = static_cast<int>(n_bins > 0 ? n_bins - 1 : 0);
        if (max_food_bin < 0 || min_food_bin > max_bin_index) {
            continue;
        }
        const int i0 = std::max(0, min_food_bin);
        const int i1 = std::min(max_food_bin, max_bin_index);
        if (i0 > i1) {
            continue;
        }

        for (int bi = i0; bi <= i1; ++bi) {
            const std::size_t s = static_cast<std::size_t>(bi);
            const double available = std::max(0.0, donor_death[s]);
            if (available <= kEps) {
                continue;
            }
            const std::vector<double> donor_row = s < donor_traits.size() ? donor_traits[s] : std::vector<double>{};
            const double compatibility = std::clamp(
                LivingBeing::calculate_effective_recruitment_efficiency(recruitment_stage, donor_row), 0.0, 1.0);
            const double w_det =
                std::clamp(f_scan * max_rate * compatibility, 0.0, 1.0);
            const double theory_capture = available * w_det;
            if (theory_capture <= 0.0) {
                continue;
            }

            const auto key = std::make_pair(donor_cohort_index, s);
            auto& item = detritus_map[key];
            item.donor_cohort_index = donor_cohort_index;
            item.bin_index = s;
            item.available = available;
            item.theory = std::min(available, item.theory + theory_capture);
        }
    }

    double theory_total = 0.0;
    for (auto& [_, item] : detritus_map) {
        item.theory = std::clamp(item.theory, 0.0, item.available);
        theory_total += item.theory;
    }

    double alpha = 0.0;
    if (theory_total > kEps && max_gross_ingestion > kEps) {
        alpha = std::min(1.0, max_gross_ingestion / theory_total);
    }

    double gross_intake = 0.0;
    for (auto& [_, item] : detritus_map) {
        item.take = alpha * item.theory;
        gross_intake += item.take;
    }

    std::map<std::size_t, std::vector<double>> decrement_by_donor;
    double total_from_donors = 0.0;

    for (auto& [_, item] : detritus_map) {
        if (item.take <= 0.0) {
            continue;
        }
        item.take_effective = item.take;
        if (item.take_effective <= kEps) {
            continue;
        }

        Cohort& donor_cohort = cohorts[item.donor_cohort_index];
        const std::vector<double>& death_now = donor_cohort.getDeathBiomass();
        const std::size_t n_bins = death_now.size();
        if (item.bin_index >= n_bins) {
            continue;
        }
        const double pool_now = std::max(0.0, death_now[item.bin_index]);
        const double take_now = std::min(pool_now, item.take_effective);
        if (take_now <= kEps) {
            continue;
        }

        std::vector<double>& dec = decrement_by_donor[item.donor_cohort_index];
        if (dec.size() < n_bins) {
            dec.resize(n_bins, 0.0);
        }
        dec[item.bin_index] += take_now;
        item.realized_take = take_now;
        total_from_donors += take_now;
    }

    for (auto& entry : decrement_by_donor) {
        cohorts[entry.first].decrement_death_biomass(std::move(entry.second));
    }

    for (auto& [_, item] : detritus_map) {
        if (item.realized_take <= kEps) {
            continue;
        }
        Cohort& donor_cohort = cohorts[item.donor_cohort_index];
        const double waste = (1.0 - assimilation_stage) * item.realized_take;
        ConsumerLivingBeing::addWasteToDeathBins(donor_cohort, residue_by_size, su, waste);
    }

    const double parental_supply_gross = ConsumerLivingBeing::applyParentalSupplyGross(
        has_parental_supply_in_diet, decomposer_biomass, su, max_gross_ingestion, total_from_donors, *specie);

    const double total_gross_intake = total_from_donors + parental_supply_gross;
    const double assimilated = assimilation_stage * total_gross_intake;
    const double parental_waste = (1.0 - assimilation_stage) * parental_supply_gross;
    ConsumerLivingBeing::addWasteToDeathBins(cohort, residue_by_size, su, parental_waste);

    const double maintenance_cost = old_biomass * m_stage;
    decomposer_biomass[su] = std::max(0.0, old_biomass + assimilated - maintenance_cost);
    cohort.setBiomass(std::move(decomposer_biomass));
}

void Decomposer::process_reproductive_growth(Cohort& cohort,
                                             int stage_index,
                                             double stage_biomass_before_growth,
                                             double biomass_increment_this_cycle) const {
    LivingBeing::process_reproductive_growth(
        cohort, stage_index, stage_biomass_before_growth, biomass_increment_this_cycle);
}

int Decomposer::getClassType() const {
    return LivingBeingClassType::DECOMPOSER;
}

void Decomposer::setCyclesPerStages(std::vector<int> cycles_per_stages) {
    LivingBeing::setCyclesPerStages(std::move(cycles_per_stages));
}

void Decomposer::rebuild_diet_by_cohort_index_from_food_type(const Niche& niche) {
    const std::size_t stage_count = getCyclesPerStages().size();
    std::vector<std::vector<std::tuple<int, int, int>>> cohort_diet_by_stage = getDietByCohortIndex();
    cohort_diet_by_stage.resize(stage_count);
    const Niche::CohortSet& cohorts = niche.getCohortSet();
    const auto& food_type_by_stage = getDietByFoodType();
    const auto append_unique_rule =
        [](std::vector<std::tuple<int, int, int>>& stage_diet, const std::tuple<int, int, int>& rule) {
            if (std::find(stage_diet.begin(), stage_diet.end(), rule) == stage_diet.end()) {
                stage_diet.push_back(rule);
            }
        };
    bool has_any_food_rules = false;
    for (const auto& stage_rules : food_type_by_stage) {
        if (!stage_rules.empty()) {
            has_any_food_rules = true;
            break;
        }
    }

    if (!has_any_food_rules) {
        // Generalist detritivore: all dead-matter bins available per donor cohort (bin indices, not life stages).
        for (std::size_t stage = 0; stage < stage_count; ++stage) {
            std::vector<std::tuple<int, int, int>>& stage_diet = cohort_diet_by_stage[stage];
            for (std::size_t i = 0; i < cohorts.size(); ++i) {
                const LivingBeing* sp = cohorts[i].getSpecie();
                if (sp == nullptr) {
                    continue;
                }
                const std::vector<double>& death_bins = cohorts[i].getDeathBiomass();
                const std::size_t n_traits = sp->getCharacteristicsDeathBiomass().size();
                const std::size_t n_bin = std::max(death_bins.size(), n_traits);
                if (n_bin == 0) {
                    continue;
                }
                const int max_bin = static_cast<int>(n_bin) - 1;
                append_unique_rule(stage_diet, std::make_tuple(static_cast<int>(i), 0, max_bin));
            }
        }
    } else {
        for (std::size_t stage = 0; stage < stage_count; ++stage) {
            std::vector<std::tuple<int, int, int>>& stage_diet = cohort_diet_by_stage[stage];
            for (std::size_t i = 0; i < cohorts.size(); ++i) {
                const LivingBeing* donor_species = cohorts[i].getSpecie();
                if (donor_species == nullptr) {
                    continue;
                }
                const std::tuple<int, int> range =
                    getRangeForFoodType(donor_species->getFoodType(), static_cast<int>(stage));
                const int min_bin = std::get<0>(range);
                const int max_bin = std::get<1>(range);
                if (min_bin < 0 || max_bin < 0) {
                    continue;
                }
                append_unique_rule(stage_diet, std::make_tuple(static_cast<int>(i), min_bin, max_bin));
            }
        }
    }
    setDietByCohortIndex(std::move(cohort_diet_by_stage));
}

Decomposer& Decomposer::setName(std::string name) {
    LivingBeing::setName(std::move(name));
    return *this;
}

Decomposer& Decomposer::setEnergyContent(float energy_content) {
    setBiomassToEnergyConversionFactor(energy_content);
    return *this;
}

Decomposer& Decomposer::setProspectingAbilityRate(std::vector<double> values) {
    ConsumerLivingBeing::setProspectingAbilityRate(std::move(values));
    return *this;
}

Decomposer& Decomposer::setAssimilationEfficiency(std::vector<double> values) {
    ConsumerLivingBeing::setAssimilationEfficiency(std::move(values));
    return *this;
}

Decomposer& Decomposer::setIngestionResidueFractionBySize(std::vector<std::vector<double>> values) {
    ConsumerLivingBeing::setIngestionResidueFractionBySize(std::move(values));
    return *this;
}
