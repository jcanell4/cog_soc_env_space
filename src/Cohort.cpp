#include "Cohort.h"
#include "Constants.h"
#include "Niche.h"
#include "Utilities.h"

#include <algorithm>
#include <cmath>

namespace {

std::vector<double> normalizeTwoNonNegative(std::vector<double> value) {
    value.resize(2, 0.0);
    for (double& v : value) {
        v = std::max(0.0, v);
    }
    return value;
}

std::vector<double> normalizeNonNegative(std::vector<double> value) {
    if (value.empty()) {
        value.push_back(0.0);
    }
    for (double& v : value) {
        v = std::max(0.0, v);
    }
    return value;
}

}  // namespace

const std::string& Cohort::getSpecieName() const {
    static const std::string empty_name;
    return specie_ ? specie_->getName() : empty_name;
}

double Cohort::getEnergy() const {
    return calculateEnergy();
}

double Cohort::calculateEnergy() const {
    return specie_ ? getTotalBiomass() * static_cast<double>(specie_->getBiomassToEnergyConversionFactor()) : 0.0;
}

const std::vector<double>& Cohort::getBiomass() const {
    return biomass_;
}

double Cohort::getTotalBiomass() const {
    double total = 0.0;
    for (double value : biomass_) {
        total += value;
    }
    return total;
}

const std::vector<double>& Cohort::getDeathBiomass() const {
    return death_biomass_;
}

double Cohort::getTotalDeathBiomass() const {
    return death_biomass_[0] + death_biomass_[1];
}

const LivingBeing* Cohort::getSpecie() const {
    return specie_;
}

Cohort& Cohort::setSpecie(const LivingBeing& value) {
    specie_ = &value;
    return *this;
}

Cohort& Cohort::setBiomass(std::vector<double> value) {
    biomass_ = normalizeNonNegative(std::move(value));
    return *this;
}

Cohort& Cohort::setDeathBiomass(std::vector<double> value) {
    death_biomass_ = normalizeTwoNonNegative(std::move(value));
    return *this;
}

void Cohort::update_deaths(int stage) {
    if (specie_ == nullptr || biomass_.empty() || stage < 0) {
        return;
    }
    const std::size_t i = static_cast<std::size_t>(stage);
    if (i >= biomass_.size()) {
        return;
    }

    const auto& resilience = specie_->getResilience();
    const double vulnerability = specie_->getVulnerability();
    const double noise = utilities::randomNormal(0.0, NOISE_STDDEV);
    const double noise_factor = std::max(0.0, 1.0 + noise);

    const double resilience_i = i < resilience.size() ? resilience[i] : 0.0;
    const double death_factor = (vulnerability + (1.0 - resilience_i)) * noise_factor;
    const double dead_i = biomass_[i] * death_factor;
    biomass_[i] = std::max(0.0, biomass_[i] - dead_i);
    death_biomass_[0] += dead_i;
}

double Cohort::decrement_death_biomass(std::vector<double> amounts) {
    amounts = normalizeTwoNonNegative(std::move(amounts));
    const double before = getTotalDeathBiomass();
    const double take_unfragmented = std::min(death_biomass_[0], amounts[0]);
    const double take_fragmented = std::min(death_biomass_[1], amounts[1]);
    death_biomass_[0] -= take_unfragmented;
    death_biomass_[1] -= take_fragmented;

    return before - getTotalDeathBiomass();
}

void Cohort::update_step(const Niche& niche) {
    for (std::size_t stage = 0; stage < biomass_.size(); ++stage) {
        update_deaths(static_cast<int>(stage));
//        update_individual_growth(niche, static_cast<int>(self_cohort_index), static_cast<int>(stage));
    }
}

void Cohort::initialize(const Niche&) {
}

void Cohort::update_individual_growth(Niche& niche, int self_cohort_index, int stage) {
    if (specie_ == nullptr || biomass_.empty() || stage < 0 || self_cohort_index < 0) {
        return;
    }
    const std::size_t su = static_cast<std::size_t>(stage);
    if (su >= biomass_.size()) {
        return;
    }

    const std::vector<std::vector<std::size_t>> diet_matrix = specie_->getDietByCohortIndex();
    if (su >= diet_matrix.size()) {
        return;
    }
    const std::vector<std::size_t>& diet_row = diet_matrix[su];

    const std::vector<double>& maintenance = specie_->getMaintenanceCost();
    const double m_stage = su < maintenance.size() ? std::clamp(maintenance[su], 0.0, 1.0) : 0.0;

    for (std::size_t code : diet_row) {
        const int channel = static_cast<int>(code);

        if (channel == DietType::NUTRIENTS_TYPE) {
            const auto& rec_matrix = specie_->getRecruitmentStrategies();
            std::vector<double> rec_row;
            if (su < rec_matrix.size()) {
                rec_row = rec_matrix[su];
            }
            const double gross = LivingBeing::calculate_effective_recruitment_efficiency(
                rec_row, niche.getLimitingFactors());
            const double noise = utilities::randomNormal(0.0, NOISE_STDV);
            const double effective = gross - m_stage + noise;
            biomass_[su] = std::max(0.0, biomass_[su] * (1.0 + effective));
            continue;
        }

        if (channel == DietType::CATABOLIC_TYPE) {
            biomass_[su] = std::max(0.0, biomass_[su] * (1.0 - m_stage));
            continue;
        }

        if (channel == DietType::PARENTAL_SUPPLY_TYPE) {
            const auto& fert = specie_->getFertility();
            double sum_f = 0.0;
            for (std::size_t i = 0; i < fert.size(); ++i) {
                if (i == su) {
                    continue;
                }
                if (fert[i] > 0.0) {
                    sum_f += fert[i];
                }
            }
            if (sum_f <= 0.0) {
                continue;
            }
            const double gross_gain = 0.5 * biomass_[su];
            const double net_gain = std::max(0.0, gross_gain - m_stage * biomass_[su]);
            for (std::size_t i = 0; i < fert.size(); ++i) {
                if (i == su || fert[i] <= 0.0) {
                    continue;
                }
                if (i >= biomass_.size()) {
                    continue;
                }
                const double take = net_gain * (fert[i] / sum_f);
                biomass_[i] = std::max(0.0, biomass_[i] - take);
            }
            biomass_[su] += net_gain;
            continue;
        }

        if (channel == DietType::HETEROTROPH_TYPE) {
            Niche::CohortSet& cohorts = niche.getCohortSet();
            for (std::size_t c = 0; c < cohorts.size(); ++c) {
                if (static_cast<int>(c) == self_cohort_index) {
                    continue;
                }
                Cohort& prey = cohorts[c];
                for (std::size_t st = 0; st < prey.biomass_.size(); ++st) {
                    const double obtained =
                        specie_->calculateObtainedBiomassIncrement(niche, static_cast<int>(c), static_cast<int>(st));
                    const double take = std::min(std::max(0.0, obtained), prey.biomass_[st]);
                    prey.biomass_[st] -= take;
                    biomass_[su] += take;
                }
            }
            biomass_[su] = std::max(0.0, biomass_[su] - biomass_[su] * m_stage);
            continue;
        }
    }
}

