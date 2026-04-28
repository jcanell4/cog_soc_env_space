/**
 * @file test_app.cpp
 * @brief Local smoke tests for core types (not built by default; excluded from CI "all" target).
 *
 * Build explicitly:
 *   cmake --build build --target cog_soc_env_space_tests
 * Run (from repo root):
 *   ./build/cog_soc_env_space_tests [path/to/simulation.json]
 */

#include "Autotroph.h"
#include "Builders.h"
#include "Constants.h"
#include "JsonEnumNames.h"
#include "JsonEcosystem.h"
#include "SimulationSnapshotReader.h"
#include "LivingBeing.h"
#include "SimulationConfig.h"
#include "Utilities.h"

#include <nlohmann/json.hpp>

#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr const char* kDefaultSimulationConfig = "config/simulation.example.json";

void log_fail(const char* message) {
    std::cerr << "TEST FAIL: " << message << "\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::cout << "cog_soc_env_space_tests (local smoke)\n";

    const std::string config_path = (argc >= 2 && argv[1] != nullptr && argv[1][0] != '\0')
                                        ? std::string(argv[1])
                                        : std::string(kDefaultSimulationConfig);

    try {
        SimulationConfig::loadFromFile(config_path);
    } catch (const std::exception& e) {
        std::cerr << "load simulation: " << e.what() << "\n";
        return 1;
    }

    const SimulationConfig& cfg = SimulationConfig::global();
    utilities::seedRng(cfg.random_seed);

    Niche niche;
    try {
        niche = NicheBuilder().loadEnvironment(cfg.environment_path).build();
    } catch (const std::exception& e) {
        std::cerr << "load niche: " << e.what() << "\n";
        return 1;
    }

    niche.initialize();

    if (niche.getCohortSet().empty()) {
        log_fail("expected at least one cohort");
        return 1;
    }

    const nlohmann::json snapshot = JsonEcosystem::createJson(niche);
    if (!snapshot.contains("initial_data") || !snapshot.contains("step_data")) {
        log_fail("JsonEcosystem::createJson missing keys");
        return 1;
    }
    if (!snapshot["step_data"].is_array()) {
        log_fail("step_data must be array");
        return 1;
    }
    if (!JsonEcosystem::saveJsonToFile(snapshot, "output/test_snapshot.json")) {
        log_fail("failed to write smoke snapshot file");
        return 1;
    }
    try {
        SimulationSnapshotReader reader;
        reader.load("output/test_snapshot.json");
        if (reader.frameCount() == 0U) {
            log_fail("snapshot reader must load at least one frame");
            return 1;
        }
        const SimulationFrameData& first_frame = reader.frameAt(0);
        if (first_frame.cohorts.empty()) {
            log_fail("snapshot reader should preserve cohort entries");
            return 1;
        }
        const SimulationFrameData interpolated = reader.interpolate(0.0);
        if (std::fabs(interpolated.nutrients - first_frame.nutrients) > 1e-9) {
            log_fail("snapshot interpolation at 0.0 must match first frame");
            return 1;
        }

        nlohmann::json named_snapshot = snapshot;
        named_snapshot["initial_data"]["data"]["cohorts"][0]["specie"]["class_type"] = "AUTOTROPH";
        JsonEcosystem::saveJsonToFile(named_snapshot, "output/test_snapshot.json");
        SimulationSnapshotReader named_reader;
        named_reader.load("output/test_snapshot.json");
        if (named_reader.frameAt(0).cohorts.empty() ||
            named_reader.frameAt(0).cohorts[0].class_type != LivingBeingClassType::AUTOTROPH) {
            log_fail("snapshot reader must accept class_type as string constant");
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "snapshot reader: " << e.what() << "\n";
        return 1;
    }

    {
        nlohmann::json mixed_input = {
            {"initial_data",
             {
                 {"data",
                  {{"cohorts",
                    nlohmann::json::array(
                        {{{"biomass", {1.0}},
                          {"death_biomass", {0.0}},
                          {"specie",
                           {{"name", "string_class_type"},
                            {"class_type", "AUTOTROPH"},
                            {"diet_by_cohort_index",
                             nlohmann::json::array(
                                 {nlohmann::json::array(
                                     {{{"cohort_index", "NUTRIENTS_TYPE"}, {"min_stage", 0}, {"max_stage", 0}},
                                      {{"cohort_index", 12345}, {"min_stage", 0}, {"max_stage", 0}}})})}}}},
                         {{"biomass", {1.0}},
                          {"death_biomass", {0.0}},
                          {"specie", {{"name", "int_class_type"}, {"class_type", LivingBeingClassType::HETEROTROPH}}}}})}}}}}};

        Niche mixed_niche;
        try {
            mixed_niche = NicheBuilder().fromJson(mixed_input).build();
        } catch (const std::exception& e) {
            std::cerr << "mixed parse: " << e.what() << "\n";
            return 1;
        }

        if (mixed_niche.getCohortSet().size() < 2U) {
            log_fail("mixed parse should build two cohorts");
            return 1;
        }
        const LivingBeing* first_specie = mixed_niche.getCohortSet()[0].getSpecie();
        const LivingBeing* second_specie = mixed_niche.getCohortSet()[1].getSpecie();
        if (first_specie == nullptr || second_specie == nullptr) {
            log_fail("mixed parse should build species pointers");
            return 1;
        }
        if (first_specie->getClassType() != LivingBeingClassType::AUTOTROPH ||
            second_specie->getClassType() != LivingBeingClassType::HETEROTROPH) {
            log_fail("class_type should parse from both string and integer");
            return 1;
        }

        const auto& diet_rules = first_specie->getDietByCohortIndex();
        if (diet_rules.empty() || diet_rules[0].size() < 2U ||
            std::get<0>(diet_rules[0][0]) != DietType::NUTRIENTS_TYPE ||
            std::get<0>(diet_rules[0][1]) != 12345) {
            log_fail("cohort_index should parse from string and integer");
            return 1;
        }

        const nlohmann::json mixed_snapshot = JsonEcosystem::createJson(mixed_niche);
        const nlohmann::json& mixed_rules =
            mixed_snapshot["initial_data"]["data"]["cohorts"][0]["specie"]["diet_by_cohort_index"][0];
        if (!mixed_snapshot["initial_data"]["data"]["cohorts"][0]["specie"]["class_type"].is_string() ||
            mixed_snapshot["initial_data"]["data"]["cohorts"][0]["specie"]["class_type"].get<std::string>() != "AUTOTROPH") {
            log_fail("known class_type should serialize to string constant");
            return 1;
        }
        if (!mixed_rules[0]["cohort_index"].is_string() ||
            mixed_rules[0]["cohort_index"].get<std::string>() != "NUTRIENTS_TYPE") {
            log_fail("known cohort_index should serialize to string constant");
            return 1;
        }
        if (!mixed_rules[1]["cohort_index"].is_number_integer() ||
            mixed_rules[1]["cohort_index"].get<int>() != 12345) {
            log_fail("unknown cohort_index should serialize as numeric fallback");
            return 1;
        }
    }

    if (!json_enum_names::classTypeToJson(999).is_number_integer() ||
        json_enum_names::classTypeToJson(999).get<int>() != 999) {
        log_fail("unknown class_type should serialize as numeric fallback");
        return 1;
    }

    std::cout << "Niche nutrients: " << niche.getNutrients() << "\n";
    for (const Cohort& cohort : niche.getCohortSet()) {
        std::cout << "Cohort death biomass " << cohort.getId() << ": " << cohort.getTotalDeathBiomass() << "\n";
    }
    niche.update_nutrients();
    std::cout << "Niche nutrients after update: " << niche.getNutrients() << "\n";
    for (const Cohort& cohort : niche.getCohortSet()) {
        std::cout << "Cohort (" << cohort.getId() << ") death biomass: " << cohort.getTotalDeathBiomass() << "\n";
    }
    for (Cohort& cohort : niche.getCohortSet()) {
        std::cout << "Cohort (" << cohort.getId() << ") biomass: " << cohort.getTotalBiomass() << "\n";
        for (int stage = 0; stage < cohort.getBiomass().size(); ++stage) {
            std::cout << "   - Stage " << stage << ": " << cohort.getBiomass()[stage] << "\n";
            cohort.update_deaths(stage);
            std::cout << "   - Stage " << stage << " after update deaths: " << cohort.getBiomass()[stage] << "\n";
        }
        std::cout << "Cohort (" << cohort.getId() << ") biomass after update deaths: " << cohort.getTotalBiomass() << "\n";
    }

    utilities::seedRng(12345ULL);
    Autotroph stage_species;
    stage_species.setName("stage_test");
    stage_species.setCyclesPerStages({10, 10});
    {
        Cohort cohort;
        cohort.setSpecie(stage_species);
        cohort.setBiomass({100.0, 0.0});
        const double total0 = cohort.getTotalBiomass();
        stage_species.updateStages(cohort, 1);
        const double total1 = cohort.getTotalBiomass();
        if (std::fabs(total0 - total1) > 1e-9) {
            log_fail("updateStages: total living biomass should be conserved");
            return 1;
        }
        if (cohort.getBiomass()[0] >= total0 - 1e-9) {
            log_fail("updateStages: stage 0 should lose biomass");
            return 1;
        }
        if (cohort.getBiomass()[1] <= 0.0) {
            log_fail("updateStages: stage 1 should gain biomass");
            return 1;
        }
    }

    std::cout << "All smoke checks passed.\n";
    return 0;
}
