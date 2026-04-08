#pragma once

/**
 * @file LivingBeing.h
 * @brief Minimal base class for the ecological domain restart.
 */

#include <string>
#include <vector>

class Niche;

/**
 * @class LivingBeing
 * @brief Small shared state holder for future species implementations.
 *
 * This restart version intentionally keeps only identity, basic numeric attributes,
 * and no-op virtual hooks that can be specialized later.
 */
class LivingBeing {
public:
    LivingBeing() = default;
    explicit LivingBeing(std::string name, float biomass_to_energy_conversion_factor_ = 0.0f);
    virtual ~LivingBeing();

    virtual void initialize(const Niche& niche);

    const std::string& getName() const;
    float getBiomassToEnergyConversionFactor() const;
    const std::vector<double>& getBestConditions() const;
    
    void setName(std::string name);
    void setBiomassToEnergyConversionFactor(float energy_content);
    void setBestConditions(std::vector<double> best_conditions);

protected:
    std::string name_;
    float biomass_to_energy_conversion_factor_{0.0f};
    std::vector<double> best_conditions_;
    bool initialized_{false};
};
