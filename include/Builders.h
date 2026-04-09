#pragma once

/**
 * @file Builders.h
 * @brief Minimal builders kept only as restart scaffolding.
 */

#include "Autotroph.h"
#include "Cohort.h"
#include "Environment.h"
#include "Heterotroph.h"
#include "Niche.h"

#include <nlohmann/json_fwd.hpp>

#include <string>
#include <unordered_map>
#include <vector>

using SpeciesRegistry = std::unordered_map<std::string, const LivingBeing*>;

class AutotrophBuilder {
public:
    AutotrophBuilder& withName(std::string value);
    AutotrophBuilder& withEnergyContent(float value);
    AutotrophBuilder& withBestEnvironmentalConditions(std::vector<std::vector<double>> value);
    AutotrophBuilder& fromJson(const nlohmann::json& j);
    Autotroph build() const;

private:
    Autotroph object_;
};

class HeterotrophBuilder {
public:
    HeterotrophBuilder& withName(std::string value);
    HeterotrophBuilder& withEnergyContent(float value);
    HeterotrophBuilder& withSearchCaptureEfficiency(std::vector<double> value);
    HeterotrophBuilder& withHandlingTimePenalty(std::vector<double> value);
    HeterotrophBuilder& withPredatorCohortIndex(std::size_t value);
    HeterotrophBuilder& withMaxIngestionRate(double value);
    HeterotrophBuilder& fromJson(const nlohmann::json& j);
    Heterotroph build() const;

private:
    Heterotroph object_;
};

class CohortBuilder {
public:
    CohortBuilder& withSpecie(const LivingBeing& value);
    CohortBuilder& withBiomass(std::vector<double> value);
    CohortBuilder& withDeathBiomass(std::vector<double> value);
    CohortBuilder& fromJson(const nlohmann::json& j, const SpeciesRegistry& registry);
    Cohort build() const;

private:
    Cohort object_;
};

class NicheBuilder {
public:
    NicheBuilder& withSurface(double value);
    NicheBuilder& withEcologicalHealth(double value);
    NicheBuilder& withNutrients(double value);
    NicheBuilder& withCohortSet(Niche::CohortSet value);
    NicheBuilder& withReturnRate(std::vector<double> value);
    NicheBuilder& withConditions(std::vector<double> value);
    NicheBuilder& fromJson(const nlohmann::json& j, const SpeciesRegistry& registry);
    Niche build() const;

private:
    Niche object_;
};

class EnvironmentBuilder {
public:
    EnvironmentBuilder& withNiches(Environment::NicheContainer value);
    EnvironmentBuilder& withAdjacency(Environment::AdjacencyList value);
    EnvironmentBuilder& fromJson(const nlohmann::json& j, const SpeciesRegistry& registry);
    Environment build() const;

private:
    Environment object_;
};
