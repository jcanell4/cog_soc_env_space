#include "Autotroph.h"
#include "Cohort.h"
#include "Constants.h"
#include "Niche.h"
#include "SimulationConfig.h"
#include "Utilities.h"

#include <algorithm>
#include <utility>

Autotroph::Autotroph()
    : LivingBeing(std::string{}, 17.5f) {
    setFoodType(std::string{FoodType::VEGETABLE});
}

int Autotroph::getClassType() const {
    return LivingBeingClassType::AUTOTROPH;
}

void Autotroph::setCyclesPerStages(std::vector<int> cycles_per_stages) {
    LivingBeing::setCyclesPerStages(std::move(cycles_per_stages));
}

void Autotroph::initialize(const Niche& niche) {
    LivingBeing::initialize(niche);
    std::vector<std::vector<std::tuple<int, int, int>>> diet_by_cohort_index = getDietByCohortIndex();
    const std::size_t stage_count = getCyclesPerStages().size();
    if (diet_by_cohort_index.size() < stage_count) {
        diet_by_cohort_index.resize(stage_count);
    }
    for (std::size_t stage = 0; stage < stage_count; ++stage) {
        std::vector<std::tuple<int, int, int>>& stage_rules = diet_by_cohort_index[stage];
        bool has_nutrients_rule = false;
        for (const auto& rule : stage_rules) {
            if (std::get<0>(rule) == DietType::NUTRIENTS_TYPE) {
                has_nutrients_rule = true;
                break;
            }
        }
        if (!has_nutrients_rule) {
            stage_rules.insert(stage_rules.begin(), std::make_tuple(DietType::NUTRIENTS_TYPE, 0, 0));
        }
    }
    setDietByCohortIndex(std::move(diet_by_cohort_index));
}

/**
 * @brief Processes the individual growth of an autotroph.
 * @param niche The niche of the autotroph.
 * @param cohort The cohort of the autotroph.
 * @param stage_index The stage index of the autotroph.
 *        The autotroph growth is determined by nutrients or catabolic growth (seeds).
 *        The autotroph growth is determined by the light transmission fraction per stratum.
 *        The autotroph growth is determined by the nutrient availability.
 *        The autotroph growth is determined by the maintenance cost.
 *        The autotroph growth is determined by the reproductive cost.
 *        The autotroph growth is determined by the death cost.
 *        The autotroph growth is determined by the competition.
 *        The autotroph growth is determined by the predation.
 *        The equation for delta_biomass is:
 *        delta_biomass = (min(mig_k, gross_k * lf_k) - mc_k) * biomass
 *        where mig_k is the maximum individual growth rate for stage k, gross_k is \prod_{i=0}^{max(|R_k|,|L|)}(max(0,min(1,1-(l_i-r_{ki})))), 
 *        lf_k is the light transmission fraction per stratum (lf_k = (lightstratum{stratumfor_k} - min_light_k) / (1 - min_light_k)), 
 *        and mc_k is the maintenance cost for stage k.
 *        lightstratum{stratumfor_k} is the light transmission fraction per stratum for stratum s and stage k.
 *        min_light_k is the minimum light required at the stage k for photosynthesis.
 */
void Autotroph::process_individual_growth(Niche& niche, Cohort& cohort, int stage_index) const {
    LivingBeing::process_individual_growth(niche, cohort, stage_index);
    const LivingBeing* specie = cohort.getSpecie();
    if (specie == nullptr) {
        return;
    }
    if (stage_index < 0) {
        return;
    }
    const std::size_t su = static_cast<std::size_t>(stage_index);
    const auto& diet_by_stage = specie->getDietByCohortIndex();
    if (su >= diet_by_stage.size()) {
        return;
    }
    const std::vector<std::tuple<int, int, int>>& stage_diet = diet_by_stage[su];

    const std::vector<double>& maintenance = specie->getMaintenanceCost();
    const double m_stage = su < maintenance.size() ? std::clamp(maintenance[su], 0.0, 1.0) : 0.0;

    for (const auto& rule : stage_diet) {
        const int food_index = std::get<0>(rule);
        const double gross_noise = utilities::randomNormal(0.0, SimulationConfig::global().noise_stdv);
        const double cost_noise = utilities::randomNormal(0.0, SimulationConfig::global().noise_stdv);
        if (food_index == DietType::NUTRIENTS_TYPE) {
            // Nutrient-limited autotroph growth (not expressed via cohort-index diet tuples).
            const auto& rec_matrix = specie->getRecruitmentStrategies();
            std::vector<double> rec_row;
            if (su < rec_matrix.size()) {
                rec_row = rec_matrix[su];
            }
            const std::vector<double> lith = niche.getLithPerStratum();
            const int stratum = su < getStratum().size() ? getStratum()[su] : getStratum().back();
            const double lith_s = stratum < lith.size() ? lith[stratum] : 1;
            const double min_l = su < this->getMinLight().size() ? this->getMinLight()[su] : 0.0;
            const double light_denominator = 1.0 - min_l;
            const double lf = light_denominator == 0.0
                                  ? lith_s
                                  : std::clamp((lith_s - min_l) / light_denominator, 0.0, 1.0);
            const double gross = LivingBeing::calculate_effective_recruitment_efficiency(
                rec_row, niche.getLimitingFactors());
            double effective = gross * lf + gross_noise;
            std::vector<double> biomass = cohort.getBiomass();
            if (su < biomass.size()) {
                const std::vector<double>& mig = cohort.getSpecie()->getMaxIndividualGrowth();
                effective = std::min(effective, mig[su]);
                const double new_biomass = std::max(0.0, biomass[su] * (1.0 + effective));
                niche.setNutrients(niche.getNutrients() - (new_biomass - biomass[su]));
                biomass[su] = new_biomass - biomass[su] * std::max(0.0, m_stage + cost_noise);
                cohort.setBiomass(std::move(biomass));
            }
            continue;
        }
        if (food_index == DietType::CATABOLIC_TYPE) {
            // Catabolic autotroph growth (not expressed via cohort-index diet tuples).
            std::vector<double> biomass = cohort.getBiomass();
            if (su >= biomass.size()) {
                continue;
            }
            biomass[su] = std::max(0.0, biomass[su] * (1.0 - m_stage - cost_noise));
            cohort.setBiomass(std::move(biomass));
            continue;
        }
    }
}

void Autotroph::process_reproductive_growth(Cohort& cohort,
                                            int stage_index,
                                            double stage_biomass_before_growth,
                                            double biomass_increment_this_cycle) const {
    LivingBeing::process_reproductive_growth(
        cohort, stage_index, stage_biomass_before_growth, biomass_increment_this_cycle);
}

const std::vector<double>& Autotroph::getOpacity() const {
    return opacity_;
}

void Autotroph::setOpacity(std::vector<double> value) {
    for (double& v : value) {
        v = std::clamp(v, 0.0, 1.0);
    }
    opacity_ = std::move(value);
}

const std::vector<int>& Autotroph::getStratum() const {
    return stratum_;
}

void Autotroph::setStratum(std::vector<int> value) {
    stratum_ = std::move(value);
}

const std::vector<double>& Autotroph::getMaxDensity() const {
    return max_density_;
}

void Autotroph::setMaxDensity(std::vector<double> value) {
    max_density_ = std::move(value);
}

const std::vector<double>& Autotroph::getMinLight() const {
    return min_light_;
}

void Autotroph::setMinLight(std::vector<double> value) {
    min_light_ = std::move(value);
}

double Autotroph::getSeedDispersalRate() const {
    return seed_dispersal_rate_;
}

void Autotroph::setSeedDispersalRate(double value) {
    seed_dispersal_rate_ = std::clamp(value, 0.0, 1.0);
}
