#include "Builders.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace {

using nlohmann::json;

std::vector<double> readDoubleVector(const json& j, const char* key) {
    if (!j.contains(key) || !j.at(key).is_array()) {
        return {};
    }
    return j.at(key).get<std::vector<double>>();
}

std::vector<int> readIntVector(const json& j, const char* key) {
    if (!j.contains(key) || !j.at(key).is_array()) {
        return {};
    }
    return j.at(key).get<std::vector<int>>();
}

std::vector<std::vector<double>> readDoubleMatrix(const json& j, const char* key) {
    if (!j.contains(key) || !j.at(key).is_array()) {
        return {};
    }
    return j.at(key).get<std::vector<std::vector<double>>>();
}

bool readDietRule(const json& rule_json, std::tuple<int, int, int>& out_rule) {
    if (rule_json.is_array()) {
        if (rule_json.size() != 3U) {
            return false;
        }
        if (!rule_json[0].is_number_integer() ||
            !rule_json[1].is_number_integer() ||
            !rule_json[2].is_number_integer()) {
            return false;
        }
        out_rule = std::make_tuple(
            rule_json[0].get<int>(),
            rule_json[1].get<int>(),
            rule_json[2].get<int>());
        return true;
    }
    if (rule_json.is_object()) {
        if (!rule_json.contains("cohort_index") || !rule_json["cohort_index"].is_number_integer()) {
            return false;
        }
        const bool has_stage_names = rule_json.contains("min_stage") && rule_json.contains("max_stage");
        const bool has_legacy_names = rule_json.contains("min_index") && rule_json.contains("max_index");
        if (!has_stage_names && !has_legacy_names) {
            return false;
        }
        const char* min_key = has_stage_names ? "min_stage" : "min_index";
        const char* max_key = has_stage_names ? "max_stage" : "max_index";
        if (!rule_json[min_key].is_number_integer() || !rule_json[max_key].is_number_integer()) {
            return false;
        }
        out_rule = std::make_tuple(
            rule_json["cohort_index"].get<int>(),
            rule_json[min_key].get<int>(),
            rule_json[max_key].get<int>());
        return true;
    }
    return false;
}

std::vector<std::vector<std::tuple<int, int, int>>> readDietByCohortIndex(const json& j) {
    std::vector<std::vector<std::tuple<int, int, int>>> out;
    if (!j.is_array()) {
        return out;
    }
    for (const json& stage_rules_json : j) {
        if (!stage_rules_json.is_array()) {
            continue;
        }
        std::vector<std::tuple<int, int, int>> stage_rules;
        for (const json& rule_json : stage_rules_json) {
            std::tuple<int, int, int> parsed_rule{};
            if (readDietRule(rule_json, parsed_rule)) {
                stage_rules.push_back(parsed_rule);
            }
        }
        out.push_back(std::move(stage_rules));
    }
    return out;
}

std::vector<std::vector<std::tuple<std::string, int, int>>> readDietByFoodType(const json& j) {
    std::vector<std::vector<std::tuple<std::string, int, int>>> out;
    if (!j.is_array()) {
        return out;
    }
    for (const json& stage_rules_json : j) {
        if (!stage_rules_json.is_array()) {
            continue;
        }
        std::vector<std::tuple<std::string, int, int>> stage_rules;
        for (const json& row : stage_rules_json) {
            if (!row.is_object()) {
                continue;
            }
            const bool has_stage_names = row.contains("min_stage") && row.contains("max_stage");
            const bool has_legacy_names = row.contains("min_index") && row.contains("max_index");
            if (!has_stage_names && !has_legacy_names) {
                continue;
            }
            const char* min_key = has_stage_names ? "min_stage" : "min_index";
            const char* max_key = has_stage_names ? "max_stage" : "max_index";
            stage_rules.emplace_back(
                row.value("food_type_prefix", std::string{}),
                row.value(min_key, 0),
                row.value(max_key, 0));
        }
        out.push_back(std::move(stage_rules));
    }
    return out;
}

void applyLivingBeingCommonFields(LivingBeing& target, const json& specie_j) {
    if (specie_j.contains("name") && specie_j["name"].is_string()) {
        target.setName(specie_j["name"].get<std::string>());
    }
    if (specie_j.contains("food_type") && specie_j["food_type"].is_string()) {
        target.setFoodType(specie_j["food_type"].get<std::string>());
    }
    if (specie_j.contains("biomass_to_energy_conversion_factor")) {
        target.setBiomassToEnergyConversionFactor(
            specie_j["biomass_to_energy_conversion_factor"].get<float>());
    }
    if (specie_j.contains("maintenance_cost")) {
        target.setMaintenanceCost(readDoubleVector(specie_j, "maintenance_cost"));
    }
    if (specie_j.contains("max_fertility")) {
        target.setMaxFertility(readDoubleVector(specie_j, "max_fertility"));
    }
    if (specie_j.contains("resilience")) {
        target.setResilience(readDoubleVector(specie_j, "resilience"));
    }
    if (specie_j.contains("biomass_per_individual_amount")) {
        target.setBiomassPerIndividualAmount(readDoubleVector(specie_j, "biomass_per_individual_amount"));
    }
    if (specie_j.contains("individual_occupied_surface")) {
        target.setIndividualOccupiedSurface(readDoubleVector(specie_j, "individual_occupied_surface"));
    }
    if (specie_j.contains("characteristics_death_biomass")) {
        target.setCharacteristicsDeathBiomass(readDoubleMatrix(specie_j, "characteristics_death_biomass"));
    }
    if (specie_j.contains("death_biomass_fraction_by_size")) {
        target.setDeathBiomassFractionBySize(readDoubleMatrix(specie_j, "death_biomass_fraction_by_size"));
    }
    if (specie_j.contains("best_environmental_conditions")) {
        target.setBestEnvironmentalConditions(readDoubleMatrix(specie_j, "best_environmental_conditions"));
    }
    if (specie_j.contains("cycles_per_stages")) {
        target.setCyclesPerStages(readIntVector(specie_j, "cycles_per_stages"));
    }
    if (specie_j.contains("defense_strategies")) {
        target.setDefenseStrategies(readDoubleMatrix(specie_j, "defense_strategies"));
    }
    if (specie_j.contains("recruitment_strategies")) {
        target.setRecruitmentStrategies(readDoubleMatrix(specie_j, "recruitment_strategies"));
    }
    if (specie_j.contains("max_individual_growth")) {
        target.setMaxIndividualGrowth(readDoubleVector(specie_j, "max_individual_growth"));
    }
    if (specie_j.contains("colony_ability_rate")) {
        target.setColonyAbilityRate(specie_j["colony_ability_rate"].get<double>());
    }
    if (specie_j.contains("diet_by_cohort_index")) {
        target.setDietByCohortIndex(readDietByCohortIndex(specie_j["diet_by_cohort_index"]));
    }
}

void applyConsumerCommonFields(ConsumerLivingBeing& target, const json& specie_j) {
    if (specie_j.contains("prospecting_ability_rate")) {
        target.setProspectingAbilityRate(readDoubleVector(specie_j, "prospecting_ability_rate"));
    }
    if (specie_j.contains("handling_time_penalty")) {
        target.setHandlingTimePenalty(readDoubleVector(specie_j, "handling_time_penalty"));
    }
    if (specie_j.contains("assimilation_efficiency")) {
        target.setAssimilationEfficiency(readDoubleVector(specie_j, "assimilation_efficiency"));
    }
    if (specie_j.contains("ingestion_residue_fraction_by_size")) {
        target.setIngestionResidueFractionBySize(readDoubleMatrix(specie_j, "ingestion_residue_fraction_by_size"));
    }
    if (specie_j.contains("diet_by_food_type")) {
        target.setDietByFoodType(readDietByFoodType(specie_j["diet_by_food_type"]));
    }
}

const LivingBeing* buildSpecieFromSnapshotJson(const json& cohort_j) {
    if (!cohort_j.contains("specie") || !cohort_j["specie"].is_object()) {
        return nullptr;
    }
    const json& specie_j = cohort_j["specie"];
    const int class_type = specie_j.value("class_type", LivingBeingClassType::AUTOTROPH);
    static std::vector<std::unique_ptr<LivingBeing>> owned_species;

    if (class_type == LivingBeingClassType::AUTOTROPH) {
        auto specie = std::make_unique<Autotroph>();
        applyLivingBeingCommonFields(*specie, specie_j);
        if (specie_j.contains("opacity")) {
            specie->setOpacity(readDoubleVector(specie_j, "opacity"));
        }
        if (specie_j.contains("stratum")) {
            specie->setStratum(readIntVector(specie_j, "stratum"));
        }
        if (specie_j.contains("max_density")) {
            specie->setMaxDensity(readDoubleVector(specie_j, "max_density"));
        }
        if (specie_j.contains("min_light")) {
            specie->setMinLight(readDoubleVector(specie_j, "min_light"));
        }
        if (specie_j.contains("seed_dispersal_rate")) {
            specie->setSeedDispersalRate(specie_j["seed_dispersal_rate"].get<double>());
        }
        const LivingBeing* ptr = specie.get();
        owned_species.push_back(std::move(specie));
        return ptr;
    }

    if (class_type == LivingBeingClassType::HETEROTROPH) {
        auto specie = std::make_unique<Heterotroph>();
        applyLivingBeingCommonFields(*specie, specie_j);
        applyConsumerCommonFields(*specie, specie_j);
        if (specie_j.contains("search_capture_efficiency")) {
            specie->setSearchCaptureEfficiency(readDoubleVector(specie_j, "search_capture_efficiency"));
        }
        const LivingBeing* ptr = specie.get();
        owned_species.push_back(std::move(specie));
        return ptr;
    }

    if (class_type == LivingBeingClassType::DECOMPOSER) {
        auto specie = std::make_unique<Decomposer>();
        applyLivingBeingCommonFields(*specie, specie_j);
        applyConsumerCommonFields(*specie, specie_j);
        const LivingBeing* ptr = specie.get();
        owned_species.push_back(std::move(specie));
        return ptr;
    }

    return nullptr;
}

const json* resolveNicheSnapshot(const json& j) {
    if (!j.is_object()) {
        return nullptr;
    }

    const json* initial_data = nullptr;
    if (j.contains("initial_data") && j["initial_data"].is_object() &&
        j["initial_data"].contains("data") && j["initial_data"]["data"].is_object()) {
        initial_data = &j["initial_data"]["data"];
    }

    if (!j.contains("step_data") || !j["step_data"].is_array() || j["step_data"].empty()) {
        return initial_data;
    }

    const json& last_step = j["step_data"].back();
    if (last_step.is_object() && last_step.contains("data") && last_step["data"].is_object()) {
        return &last_step["data"];
    }
    return initial_data;
}

}  // namespace

AutotrophBuilder& AutotrophBuilder::withName(std::string value) {
    object_.setName(std::move(value));
    return *this;
}

AutotrophBuilder& AutotrophBuilder::withEnergyContent(float value) {
    object_.setBiomassToEnergyConversionFactor(value);
    return *this;
}

AutotrophBuilder& AutotrophBuilder::withBestEnvironmentalConditions(std::vector<std::vector<double>> value) {
    object_.setBestEnvironmentalConditions(std::move(value));
    return *this;
}

AutotrophBuilder& AutotrophBuilder::fromJson(const nlohmann::json&) {
    return *this;
}

Autotroph AutotrophBuilder::build() const {
    return object_;
}

HeterotrophBuilder& HeterotrophBuilder::withName(std::string value) {
    object_.setName(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withEnergyContent(float value) {
    object_.setBiomassToEnergyConversionFactor(value);
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withSearchCaptureEfficiency(std::vector<double> value) {
    object_.setSearchCaptureEfficiency(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withProspectingAbilityRate(std::vector<double> value) {
    object_.setProspectingAbilityRate(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withAssimilationEfficiency(std::vector<double> value) {
    object_.setAssimilationEfficiency(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withHandlingTimePenalty(std::vector<double> value) {
    object_.setHandlingTimePenalty(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withIngestionResidueFractionBySize(std::vector<std::vector<double>> value) {
    object_.setIngestionResidueFractionBySize(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::fromJson(const nlohmann::json&) {
    return *this;
}

Heterotroph HeterotrophBuilder::build() const {
    return object_;
}

DecomposerBuilder& DecomposerBuilder::withName(std::string value) {
    object_.setName(std::move(value));
    return *this;
}

DecomposerBuilder& DecomposerBuilder::withEnergyContent(float value) {
    object_.setBiomassToEnergyConversionFactor(value);
    return *this;
}

DecomposerBuilder& DecomposerBuilder::withProspectingAbilityRate(std::vector<double> value) {
    object_.setProspectingAbilityRate(std::move(value));
    return *this;
}

DecomposerBuilder& DecomposerBuilder::withAssimilationEfficiency(std::vector<double> value) {
    object_.setAssimilationEfficiency(std::move(value));
    return *this;
}

DecomposerBuilder& DecomposerBuilder::withHandlingTimePenalty(std::vector<double> value) {
    object_.setHandlingTimePenalty(std::move(value));
    return *this;
}

DecomposerBuilder& DecomposerBuilder::withIngestionResidueFractionBySize(std::vector<std::vector<double>> value) {
    object_.setIngestionResidueFractionBySize(std::move(value));
    return *this;
}

DecomposerBuilder& DecomposerBuilder::fromJson(const nlohmann::json&) {
    return *this;
}

Decomposer DecomposerBuilder::build() const {
    return object_;
}

CohortBuilder& CohortBuilder::withSpecie(const LivingBeing& value) {
    object_.setSpecie(value);
    return *this;
}

CohortBuilder& CohortBuilder::withBiomass(std::vector<double> value) {
    object_.setBiomass(std::move(value));
    return *this;
}

CohortBuilder& CohortBuilder::withDeathBiomass(std::vector<double> value) {
    object_.setDeathBiomass(value);
    return *this;
}

CohortBuilder& CohortBuilder::fromJson(const nlohmann::json&, const SpeciesRegistry&) {
    // Kept as scaffolding for legacy config format.
    return *this;
}

CohortBuilder& CohortBuilder::fromJson(const nlohmann::json& j) {
    const SpeciesRegistry empty_registry;
    return fromJson(j, empty_registry);
}

Cohort CohortBuilder::build() const {
    return object_;
}

NicheBuilder& NicheBuilder::withSurface(double value) {
    object_.setSurface(value);
    return *this;
}

NicheBuilder& NicheBuilder::withEcologicalHealth(double value) {
    object_.setEcologicalHealth(value);
    return *this;
}

NicheBuilder& NicheBuilder::withNutrients(double value) {
    object_.setNutrients(value);
    return *this;
}

NicheBuilder& NicheBuilder::withCohortSet(Niche::CohortSet value) {
    object_.setCohortSet(std::move(value));
    return *this;
}

NicheBuilder& NicheBuilder::withReturnRate(std::vector<double> value) {
    object_.setReturnRate(std::move(value));
    return *this;
}

NicheBuilder& NicheBuilder::withConditions(std::vector<double> value) {
    object_.setConditions(std::move(value));
    return *this;
}

NicheBuilder& NicheBuilder::loadEnvironment(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("NicheBuilder::loadEnvironment: cannot open file: " + path);
    }

    json root;
    try {
        in >> root;
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(
            std::string("NicheBuilder::loadEnvironment: JSON parse error: ") + e.what());
    }

    return fromJson(root);
}

NicheBuilder& NicheBuilder::fromJson(const nlohmann::json& j) {
    const json* snapshot = resolveNicheSnapshot(j);
    if (snapshot == nullptr) {
        return *this;
    }

    if (snapshot->contains("surface")) {
        object_.setSurface((*snapshot)["surface"].get<double>());
    }
    if (snapshot->contains("ecological_health")) {
        object_.setEcologicalHealth((*snapshot)["ecological_health"].get<double>());
    }
    if (snapshot->contains("nutrients")) {
        object_.setNutrients((*snapshot)["nutrients"].get<double>());
    }
    if (snapshot->contains("return_rate")) {
        object_.setReturnRate(readDoubleVector(*snapshot, "return_rate"));
    }
    if (snapshot->contains("conditions")) {
        object_.setConditions(readDoubleVector(*snapshot, "conditions"));
    }
    if (snapshot->contains("limiting_factors")) {
        object_.setLimitingFactors(readDoubleVector(*snapshot, "limiting_factors"));
    }

    Niche::CohortSet cohorts;
    if (snapshot->contains("cohorts") && (*snapshot)["cohorts"].is_array()) {
        for (const json& cohort_j : (*snapshot)["cohorts"]) {
            if (!cohort_j.is_object()) {
                continue;
            }
            Cohort cohort;
            if (cohort_j.contains("biomass")) {
                cohort.setBiomass(readDoubleVector(cohort_j, "biomass"));
            }
            if (cohort_j.contains("death_biomass")) {
                cohort.setDeathBiomass(readDoubleVector(cohort_j, "death_biomass"));
            }

            const LivingBeing* specie = buildSpecieFromSnapshotJson(cohort_j);
            if (specie != nullptr) {
                cohort.setSpecie(*specie);
            }

            cohorts.push_back(std::move(cohort));
        }
    }
    object_.setCohortSet(std::move(cohorts));

    return *this;
}

NicheBuilder& NicheBuilder::fromJson(const nlohmann::json& j, const SpeciesRegistry&) {
    return fromJson(j);
}

Niche NicheBuilder::build() const {
    return object_;
}

EnvironmentBuilder& EnvironmentBuilder::withNiches(Environment::NicheContainer value) {
    object_.setNiches(std::move(value));
    return *this;
}

EnvironmentBuilder& EnvironmentBuilder::withAdjacency(Environment::AdjacencyList value) {
    object_.setAdjacency(std::move(value));
    return *this;
}

EnvironmentBuilder& EnvironmentBuilder::fromJson(const nlohmann::json&, const SpeciesRegistry&) {
    return *this;
}

EnvironmentBuilder& EnvironmentBuilder::fromJson(const nlohmann::json& j) {
    const SpeciesRegistry empty_registry;
    return fromJson(j, empty_registry);
}

Environment EnvironmentBuilder::build() const {
    return object_;
}
