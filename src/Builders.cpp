#include "Builders.h"
#include "JsonEnumNames.h"

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

bool readDietRule(const json& rule_json, std::tuple<int, int, int, int>& out_rule) {
    if (rule_json.is_array()) {
        if (rule_json.size() != 3U && rule_json.size() != 4U) {
            return false;
        }
        if (!rule_json[1].is_number_integer() || !rule_json[2].is_number_integer()) {
            return false;
        }
        const int matter_type =
            rule_json.size() >= 4U && rule_json[3].is_number_integer() ? rule_json[3].get<int>() : MatterType::LIVING;
        out_rule = std::make_tuple(
            json_enum_names::parseDietPopulationIndexValue(rule_json[0], "diet_by_population_index[][0]"),
            rule_json[1].get<int>(),
            rule_json[2].get<int>(),
            matter_type);
        return true;
    }
    if (rule_json.is_object()) {
        if (!rule_json.contains("population_index")) {
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
        const int matter_type = rule_json.value("matter_type", MatterType::LIVING);
        out_rule = std::make_tuple(
            json_enum_names::parseDietPopulationIndexValue(rule_json["population_index"], "diet_by_population_index[].population_index"),
            rule_json[min_key].get<int>(),
            rule_json[max_key].get<int>(),
            matter_type);
        return true;
    }
    return false;
}

std::vector<std::vector<std::tuple<int, int, int, int>>> readDietByPopulationIndex(const json& j) {
    std::vector<std::vector<std::tuple<int, int, int, int>>> out;
    if (!j.is_array()) {
        return out;
    }
    for (const json& stage_rules_json : j) {
        if (!stage_rules_json.is_array()) {
            continue;
        }
        std::vector<std::tuple<int, int, int, int>> stage_rules;
        for (const json& rule_json : stage_rules_json) {
            std::tuple<int, int, int, int> parsed_rule{};
            if (readDietRule(rule_json, parsed_rule)) {
                stage_rules.push_back(parsed_rule);
            }
        }
        out.push_back(std::move(stage_rules));
    }
    return out;
}

std::vector<std::vector<std::tuple<std::string, int, int, int>>> readDietByFoodType(const json& j) {
    std::vector<std::vector<std::tuple<std::string, int, int, int>>> out;
    if (!j.is_array()) {
        return out;
    }
    for (const json& stage_rules_json : j) {
        if (!stage_rules_json.is_array()) {
            continue;
        }
        std::vector<std::tuple<std::string, int, int, int>> stage_rules;
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
            const int matter_type = row.value("matter_type", MatterType::LIVING);
            stage_rules.emplace_back(
                row.value("food_type_prefix", std::string{}),
                row.value(min_key, 0),
                row.value(max_key, 0),
                matter_type);
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
    if (specie_j.contains("death_biomass_to_energy_conversion_factor")) {
        target.setDeathBiomassToEnergyConversionFactor(
            specie_j["death_biomass_to_energy_conversion_factor"].get<float>());
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
    if (specie_j.contains("death_biomass_fraction_surface")) {
        target.setDeathBiomassFractionSurface(readDoubleVector(specie_j, "death_biomass_fraction_surface"));
    }
    if (specie_j.contains("death_biomass_per_fraction_amount")) {
        target.setDeathBiomassPerFractionAmount(readDoubleVector(specie_j, "death_biomass_per_fraction_amount"));
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
    if (specie_j.contains("max_density")) {
        target.setMaxDensity(readDoubleVector(specie_j, "max_density"));
    }
    if (specie_j.contains("colony_ability_rate")) {
        target.setColonyAbilityRate(specie_j["colony_ability_rate"].get<double>());
    }
    if (specie_j.contains("diet_by_population_index")) {
        target.setDietByPopulationIndex(readDietByPopulationIndex(specie_j["diet_by_population_index"]));
    }
}

void applyHeterotrophConsumerFields(Heterotroph& target, const json& specie_j) {
    if (specie_j.contains("prospecting_ability")) {
        target.setProspectingAbility(readDoubleVector(specie_j, "prospecting_ability"));
    } else if (specie_j.contains("prospecting_ability_rate")) {
        target.setProspectingAbility(readDoubleVector(specie_j, "prospecting_ability_rate"));
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

const LivingBeing* buildSpecieFromSnapshotJson(const json& population_j) {
    if (!population_j.contains("specie") || !population_j["specie"].is_object()) {
        return nullptr;
    }
    const json& specie_j = population_j["specie"];
    int class_type = LivingBeingClassType::AUTOTROPH;
    if (specie_j.contains("class_type")) {
        class_type = json_enum_names::parseClassTypeValue(specie_j["class_type"], "specie.class_type");
    }
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
        applyHeterotrophConsumerFields(*specie, specie_j);
        if (specie_j.contains("prospecting_ability")) {
            specie->setProspectingAbility(readDoubleVector(specie_j, "prospecting_ability"));
        } else if (specie_j.contains("prospecting_ability_rate")) {
            specie->setProspectingAbility(readDoubleVector(specie_j, "prospecting_ability_rate"));
        }
        if (specie_j.contains("prey_location")) {
            specie->setPreyLocation(readDoubleVector(specie_j, "prey_location"));
        }
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

AutotrophBuilder& AutotrophBuilder::withDeathEnergyContent(float value) {
    object_.setDeathBiomassToEnergyConversionFactor(value);
    return *this;
}

AutotrophBuilder& AutotrophBuilder::withBestEnvironmentalConditions(std::vector<std::vector<double>> value) {
    object_.setBestEnvironmentalConditions(std::move(value));
    return *this;
}

AutotrophBuilder& AutotrophBuilder::withDeathBiomassFractionSurface(std::vector<double> value) {
    object_.setDeathBiomassFractionSurface(std::move(value));
    return *this;
}

AutotrophBuilder& AutotrophBuilder::withDeathBiomassPerFractionAmount(std::vector<double> value) {
    object_.setDeathBiomassPerFractionAmount(std::move(value));
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

HeterotrophBuilder& HeterotrophBuilder::withDeathEnergyContent(float value) {
    object_.setDeathBiomassToEnergyConversionFactor(value);
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withProspectingAbility(std::vector<double> value) {
    object_.setProspectingAbility(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withProspectingAbilityRate(std::vector<double> value) {
    return withProspectingAbility(std::move(value));
}

HeterotrophBuilder& HeterotrophBuilder::withAssimilationEfficiency(std::vector<double> value) {
    object_.setAssimilationEfficiency(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withIngestionResidueFractionBySize(std::vector<std::vector<double>> value) {
    object_.setIngestionResidueFractionBySize(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withPreyLocation(std::vector<double> value) {
    object_.setPreyLocation(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withDeathBiomassFractionSurface(std::vector<double> value) {
    object_.setDeathBiomassFractionSurface(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withDeathBiomassPerFractionAmount(std::vector<double> value) {
    object_.setDeathBiomassPerFractionAmount(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::fromJson(const nlohmann::json& j) {
    if (j.contains("prospecting_ability")) {
        object_.setProspectingAbility(readDoubleVector(j, "prospecting_ability"));
    } else if (j.contains("prospecting_ability_rate")) {
        object_.setProspectingAbility(readDoubleVector(j, "prospecting_ability_rate"));
    }
    if (j.contains("prey_location")) {
        object_.setPreyLocation(readDoubleVector(j, "prey_location"));
    }
    return *this;
}

Heterotroph HeterotrophBuilder::build() const {
    return object_;
}

PopulationBuilder& PopulationBuilder::withSpecie(const LivingBeing& value) {
    object_.setSpecie(value);
    return *this;
}

PopulationBuilder& PopulationBuilder::withBiomass(std::vector<double> value) {
    object_.setBiomass(std::move(value));
    return *this;
}

PopulationBuilder& PopulationBuilder::withDeathBiomass(std::vector<double> value) {
    object_.setDeathBiomass(value);
    return *this;
}

PopulationBuilder& PopulationBuilder::fromJson(const nlohmann::json&, const SpeciesRegistry&) {
    // Kept as scaffolding for legacy config format.
    return *this;
}

PopulationBuilder& PopulationBuilder::fromJson(const nlohmann::json& j) {
    const SpeciesRegistry empty_registry;
    return fromJson(j, empty_registry);
}

Population PopulationBuilder::build() const {
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

NicheBuilder& NicheBuilder::withPopulationSet(Niche::PopulationSet value) {
    object_.setPopulationSet(std::move(value));
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

NicheBuilder& NicheBuilder::withProspectingScanSharpness(double value) {
    object_.setProspectingScanSharpness(value);
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
    if (snapshot->contains("prospecting_scan_sharpness")) {
        object_.setProspectingScanSharpness((*snapshot)["prospecting_scan_sharpness"].get<double>());
    }

    Niche::PopulationSet populations;
    if (snapshot->contains("populations") && (*snapshot)["populations"].is_array()) {
        for (const json& population_j : (*snapshot)["populations"]) {
            if (!population_j.is_object()) {
                continue;
            }
            Population population;
            if (population_j.contains("biomass")) {
                population.setBiomass(readDoubleVector(population_j, "biomass"));
            }
            if (population_j.contains("death_biomass")) {
                population.setDeathBiomass(readDoubleVector(population_j, "death_biomass"));
            }

            const LivingBeing* specie = buildSpecieFromSnapshotJson(population_j);
            if (specie != nullptr) {
                population.setSpecie(*specie);
            }

            populations.push_back(std::move(population));
        }
    }
    object_.setPopulationSet(std::move(populations));

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
