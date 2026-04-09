#include "Builders.h"

#include <utility>

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

HeterotrophBuilder& HeterotrophBuilder::withHandlingTimePenalty(std::vector<double> value) {
    object_.setHandlingTimePenalty(std::move(value));
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withPredatorCohortIndex(std::size_t value) {
    object_.setPredatorCohortIndex(value);
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::withMaxIngestionRate(double value) {
    object_.setMaxIngestionRate(value);
    return *this;
}

HeterotrophBuilder& HeterotrophBuilder::fromJson(const nlohmann::json&) {
    return *this;
}

Heterotroph HeterotrophBuilder::build() const {
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
    return *this;
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

NicheBuilder& NicheBuilder::fromJson(const nlohmann::json&, const SpeciesRegistry&) {
    return *this;
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

Environment EnvironmentBuilder::build() const {
    return object_;
}
