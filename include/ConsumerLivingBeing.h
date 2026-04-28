#pragma once

/**
 * @file ConsumerLivingBeing.h
 * @brief Shared ingestion state for heterotrophs and decomposers (assimilation, residue routing, prospecting).
 */

#include "LivingBeing.h"

#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

class Cohort;

/**
 * @class ConsumerLivingBeing
 * @brief Base for species that ingest external biomass (living or dead) with partial assimilation and
 *        residue routing to dead-biomass size bins.
 */
class ConsumerLivingBeing : public LivingBeing {
public:
    ConsumerLivingBeing() = default;

    const std::vector<double>& getProspectingAbilityRate() const;
    const std::vector<double>& getAssimilationEfficiency() const;
    const std::vector<std::vector<double>>& getIngestionResidueFractionBySize() const;

    ConsumerLivingBeing& setProspectingAbilityRate(std::vector<double> values);
    ConsumerLivingBeing& setAssimilationEfficiency(std::vector<double> values);
    ConsumerLivingBeing& setIngestionResidueFractionBySize(std::vector<std::vector<double>> values);

    /**
     * @brief Taxonomic diet rules grouped by consumer stage.
     *        Each tuple is (hierarchy_prefix, min_prey_stage, max_prey_stage inclusive).
     */
    const std::vector<std::vector<std::tuple<std::string, int, int>>>& getDietByFoodType() const;
    void setDietByFoodType(std::vector<std::vector<std::tuple<std::string, int, int>>> diet_by_food_type);
    bool isFoodTypeMyDiet(const std::string& prey_food_type, int consumer_stage, int prey_stage) const;

    /**
     * @brief Stage range [min, max] if @a prey_food_type matches a row in @ref diet_by_food_type_
     *        for @a consumer_stage; else (-1, -1).
     */
    std::tuple<int, int> getRangeForFoodType(const std::string& prey_food_type, int consumer_stage) const;

protected:
    static std::vector<double> clampUnitInterval(std::vector<double> values);
    static std::vector<double> normalizeResidueRow(std::vector<double> row);

    /**
     * @brief Add non-assimilated mass to cohort dead biomass bins using per-stage residue fractions.
     */
    static void addWasteToDeathBins(Cohort& target,
                                    const std::vector<std::vector<double>>& residue_matrix,
                                    std::size_t stage_index,
                                    double waste);

    /**
     * @brief Parental supply gross uptake from fertile donor stages (same rules as heterotroph).
     * @return Total gross biomass taken from donor (living) stages.
     */
    static double applyParentalSupplyGross(bool has_parental_supply_in_diet,
                                           std::vector<double>& consumer_biomass,
                                           std::size_t stage_index,
                                           double max_gross_ingestion,
                                           double total_from_primary_sources,
                                           const LivingBeing& specie);

    std::vector<double> prospecting_ability_rate_;
    std::vector<double> assimilation_efficiency_;
    std::vector<std::vector<double>> ingestion_residue_fraction_by_size_;

    /** @brief Taxonomic diet per consumer stage for heterotrophs and decomposers. */
    std::vector<std::vector<std::tuple<std::string, int, int>>> diet_by_food_type_{};
};
