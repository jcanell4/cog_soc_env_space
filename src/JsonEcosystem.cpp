#include "JsonEcosystem.h"

#include "Autotroph.h"
#include "Cohort.h"
#include "Constants.h"
#include "ConsumerLivingBeing.h"
#include "Decomposer.h"
#include "Heterotroph.h"
#include "LivingBeing.h"
#include "Niche.h"

#include <algorithm>
#include <fstream>
#include <tuple>
#include <utility>

#include <nlohmann/json.hpp>

namespace {

using nlohmann::json;

const char* classTypeToName(int class_type) {
    switch (class_type) {
    case LivingBeingClassType::AUTOTROPH:
        return "Autotroph";
    case LivingBeingClassType::HETEROTROPH:
        return "Heterotroph";
    case LivingBeingClassType::DECOMPOSER:
        return "Decomposer";
    default:
        return "LivingBeing";
    }
}

json toJsonDietByCohortIndex(const std::vector<std::tuple<int, int, int>>& diet) {
    json out = json::array();
    for (const auto& entry : diet) {
        out.push_back({
            {"cohort_index", std::get<0>(entry)},
            {"min_index", std::get<1>(entry)},
            {"max_index", std::get<2>(entry)},
        });
    }
    return out;
}

json toJsonDietByFoodType(const std::vector<std::tuple<std::string, int, int>>& diet) {
    json out = json::array();
    for (const auto& entry : diet) {
        out.push_back({
            {"food_type_prefix", std::get<0>(entry)},
            {"min_index", std::get<1>(entry)},
            {"max_index", std::get<2>(entry)},
        });
    }
    return out;
}

void writeLivingBeingCommon(const LivingBeing& living_being, json& out) {
    out["class_type"] = living_being.getClassType();
    out["class_name"] = classTypeToName(living_being.getClassType());
    out["name"] = living_being.getName();
    out["food_type"] = living_being.getFoodType();
    out["biomass_to_energy_conversion_factor"] = living_being.getBiomassToEnergyConversionFactor();
    out["maintenance_cost"] = living_being.getMaintenanceCost();
    out["max_fertility"] = living_being.getMaxFertility();
    out["resilience"] = living_being.getResilience();
    out["vulnerability"] = living_being.getVulnerability();
    out["biomass_per_individual_amount"] = living_being.getBiomassPerIndividualAmount();
    out["individual_occupied_surface"] = living_being.getIndividualOccupiedSurface();
    out["characteristics_death_biomass"] = living_being.getCharacteristicsDeathBiomass();
    out["death_biomass_fraction_by_size"] = living_being.getDeathBiomassFractionBySize();
    out["best_environmental_conditions"] = living_being.getBestEnvironmentalConditions();
    out["cycles_per_stages"] = living_being.getCyclesPerStages();
    out["defense_strategies"] = living_being.getDefenseStrategies();
    out["recruitment_strategies"] = living_being.getRecruitmentStrategies();
    out["max_individual_growth"] = living_being.getMaxIndividualGrowth();
    out["colony_ability_rate"] = living_being.getColonyAbilityRate();
    out["diet_by_cohort_index"] = toJsonDietByCohortIndex(living_being.getDietByCohortIndex());
}

void writeConsumerLivingBeingCommon(const ConsumerLivingBeing& consumer, json& out) {
    out["prospecting_ability_rate"] = consumer.getProspectingAbilityRate();
    out["handling_time_penalty"] = consumer.getHandlingTimePenalty();
    out["assimilation_efficiency"] = consumer.getAssimilationEfficiency();
    out["ingestion_residue_fraction_by_size"] = consumer.getIngestionResidueFractionBySize();
    out["diet_by_food_type"] = toJsonDietByFoodType(consumer.getDietByFoodType());
}

}  // namespace

nlohmann::json JsonEcosystem::createJson(const Niche& niche) {
    json root = json::object();
    root["initial_data"] = json::object();
    root["initial_data"]["type"] = "Niche";
    updateJson(niche, root["initial_data"]["data"]);
    root["step_data"] = json::array();
    return root;
}

void JsonEcosystem::updateJson(const Niche& niche, int elapsed_cycles, nlohmann::json& root) {
    if (!root.is_object()) {
        root = json::object();
    }
    if (!root.contains("initial_data")) {
        root["initial_data"] = json::object();
        root["initial_data"]["type"] = "Niche";
        updateJson(niche, root["initial_data"]["data"]);
    }
    if (!root.contains("step_data") || !root["step_data"].is_array()) {
        root["step_data"] = json::array();
    }

    json step_entry = json::object();
    step_entry["type"] = "Niche";
    step_entry["elapsed_cycles"] = std::max(0, elapsed_cycles);
    updateJson(niche, step_entry["data"]);
    root["step_data"].push_back(std::move(step_entry));
}

bool JsonEcosystem::saveJsonToFile(const nlohmann::json& root, const std::string& output_path, int indent) {
    std::ofstream output(output_path);
    if (!output.is_open()) {
        return false;
    }
    output << root.dump(std::max(indent, 0));
    if (!output.good()) {
        return false;
    }
    output << '\n';
    return output.good();
}

void JsonEcosystem::updateJson(const Niche& niche, nlohmann::json& out) {
    out = json::object();
    out["surface"] = niche.getSurface();
    out["ecological_health"] = niche.getEcologicalHealth();
    out["nutrients"] = niche.getNutrients();
    out["return_rate"] = niche.getReturnRate();
    out["conditions"] = niche.getConditions();
    out["limiting_factors"] = niche.getLimitingFactors();
    out["living_biomass"] = niche.getLivingBiomass();
    out["death_biomass"] = niche.getDeathBiomass();
    out["decomposer_biomass"] = niche.getDecomposerBiomass();
    out["autotroph_biomass_per_stratum"] = niche.getAutotrophBiomassPerStratum();
    out["light_per_stratum"] = niche.getLithPerStratum();

    out["cohorts"] = json::array();
    const Niche::CohortSet& cohorts = niche.getCohortSet();
    for (const Cohort& cohort : cohorts) {
        json cohort_json = json::object();
        updateJson(cohort, cohort_json);
        out["cohorts"].push_back(std::move(cohort_json));
    }
}

void JsonEcosystem::updateJson(const Cohort& cohort, nlohmann::json& out) {
    out = json::object();
    out["id"] = cohort.getId();
    out["specie_name"] = cohort.getSpecieName();
    out["energy"] = cohort.getEnergy();
    out["total_biomass"] = cohort.getTotalBiomass();
    out["total_death_biomass"] = cohort.getTotalDeathBiomass();
    out["biomass"] = cohort.getBiomass();
    out["death_biomass"] = cohort.getDeathBiomass();

    const LivingBeing* specie = cohort.getSpecie();
    if (specie != nullptr) {
        json specie_json = json::object();
        updateJson(*specie, specie_json);
        out["specie"] = std::move(specie_json);
    } else {
        out["specie"] = nullptr;
    }
}

void JsonEcosystem::updateJson(const LivingBeing& living_being, nlohmann::json& out) {
    if (!out.is_object()) {
        out = json::object();
    }
    writeLivingBeingCommon(living_being, out);

    switch (living_being.getClassType()) {
    case LivingBeingClassType::AUTOTROPH: {
        const auto* autotroph = dynamic_cast<const Autotroph*>(&living_being);
        if (autotroph != nullptr) {
            updateJson(*autotroph, out);
        }
        break;
    }
    case LivingBeingClassType::HETEROTROPH: {
        const auto* heterotroph = dynamic_cast<const Heterotroph*>(&living_being);
        if (heterotroph != nullptr) {
            updateJson(*heterotroph, out);
        }
        break;
    }
    case LivingBeingClassType::DECOMPOSER: {
        const auto* decomposer = dynamic_cast<const Decomposer*>(&living_being);
        if (decomposer != nullptr) {
            updateJson(*decomposer, out);
        }
        break;
    }
    default:
        break;
    }
}

void JsonEcosystem::updateJson(const Autotroph& autotroph, nlohmann::json& out) {
    if (!out.is_object()) {
        out = json::object();
    }
    out["opacity"] = autotroph.getOpacity();
    out["stratum"] = autotroph.getStratum();
    out["max_density"] = autotroph.getMaxDensity();
    out["min_light"] = autotroph.getMinLight();
    out["seed_dispersal_rate"] = autotroph.getSeedDispersalRate();
}

void JsonEcosystem::updateJson(const Heterotroph& heterotroph, nlohmann::json& out) {
    if (!out.is_object()) {
        out = json::object();
    }
    writeConsumerLivingBeingCommon(heterotroph, out);
    out["search_capture_efficiency"] = heterotroph.getSearchCaptureEfficiency();
}

void JsonEcosystem::updateJson(const Decomposer& decomposer, nlohmann::json& out) {
    if (!out.is_object()) {
        out = json::object();
    }
    writeConsumerLivingBeingCommon(decomposer, out);
}
