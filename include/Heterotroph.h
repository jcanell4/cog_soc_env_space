#pragma once

/**
 * @file Heterotroph.h
 * @brief Consumer species: living prey and/or dead matter (MatterType::DEAD) via diet tuples.
 */

#include "LivingBeing.h"

#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

class Population;
class Niche;

class Heterotroph : public LivingBeing {
public:
    Heterotroph();

    void initialize(const Niche& niche) override;

    /**
     * @brief Stage-level ingestion: living prey (unchanged pipeline) then dead-matter uptake when diet
     *        includes @c MatterType::DEAD tuples.
     */
    void process_individual_growth(Niche& niche, Population& population, int stage_index) const override;
    void process_reproductive_growth(Population& population,
                                   int stage_index,
                                   double stage_biomass_before_growth,
                                   double biomass_increment_this_cycle) const override;

    int getClassType() const override;

    void setCyclesPerStages(std::vector<int> cycles_per_stages) override;

    const std::vector<double>& getProspectingAbility() const;
    double getProspectingAbility(std::size_t index, double out_of_range_default = 0.0) const;
    const std::vector<double>& getAssimilationEfficiency() const;
    double getAssimilationEfficiency(std::size_t index, double out_of_range_default = 0.0) const;
    const std::vector<std::vector<double>>& getIngestionResidueFractionBySize() const;
    const std::vector<double>& getIngestionResidueFractionBySize(std::size_t row_index) const;

    const std::vector<std::vector<std::tuple<std::string, int, int, int>>>& getDietByFoodType() const;
    void setDietByFoodType(std::vector<std::vector<std::tuple<std::string, int, int, int>>> diet_by_food_type);
    bool isFoodTypeMyDiet(const std::string& prey_food_type,
                          int consumer_stage,
                          int prey_stage,
                          int prey_matter_type) const;
    std::tuple<int, int, int> getRangeForFoodType(const std::string& prey_food_type, int consumer_stage) const;

    Heterotroph& setProspectingAbility(std::vector<double> values);
    Heterotroph& setProspectingAbilityRate(std::vector<double> values);
    Heterotroph& setAssimilationEfficiency(std::vector<double> values);
    Heterotroph& setIngestionResidueFractionBySize(std::vector<std::vector<double>> values);

    const std::vector<double>& getPreyLocation() const;
    double getPreyLocation(std::size_t index, double out_of_range_default = 1.0) const;

    Heterotroph& setName(std::string name);
    Heterotroph& setEnergyContent(float energy_content);
    Heterotroph& setPreyLocation(std::vector<double> values);

    /**
     * @brief Fills @ref LivingBeing::diet_by_population_index_ from taxonomic diet rules and population species
     *        food types in @a niche (one tuple per population whose prey taxonomy matches a diet rule,
     *        including matter type 0/1 as the fourth component).
     */
    void rebuild_diet_by_population_index_from_food_type(const Niche& niche);

private:
    static std::vector<double> clampUnitInterval(std::vector<double> values);
    static std::vector<double> normalizeResidueRow(std::vector<double> row);
    static void addWasteToDeathBins(Population& target,
                                    const std::vector<std::vector<double>>& residue_matrix,
                                    std::size_t stage_index,
                                    double waste);
    static double applyParentalSupplyGross(bool has_parental_supply_in_diet,
                                           std::vector<double>& consumer_biomass,
                                           std::size_t stage_index,
                                           double max_gross_ingestion,
                                           double total_from_primary_sources,
                                           const LivingBeing& specie);

    std::vector<double> prospecting_ability_;
    std::vector<double> assimilation_efficiency_;
    std::vector<std::vector<double>> ingestion_residue_fraction_by_size_;
    std::vector<std::vector<std::tuple<std::string, int, int, int>>> diet_by_food_type_{};
    std::vector<double> prey_location_{1.0};
};
