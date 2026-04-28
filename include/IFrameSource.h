#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct CohortFrameData {
    int id = -1;
    std::string specie_name;
    std::string class_name;
    int class_type = -1;
    double energy = 0.0;
    double total_biomass = 0.0;
    double total_death_biomass = 0.0;
    std::vector<double> stage_biomass;
};

struct SimulationFrameData {
    int elapsed_cycles = 0;
    double nutrients = 0.0;
    double total_energy = 0.0;
    double ecological_health = 0.0;
    double living_biomass = 0.0;
    double death_biomass = 0.0;
    double decomposer_biomass = 0.0;
    double autotroph_biomass = 0.0;
    double heterotroph_biomass = 0.0;
    double other_living_biomass = 0.0;
    std::vector<CohortFrameData> cohorts;
};

/**
 * @brief Data provider abstraction consumed by visualization renderers.
 *
 * Implementations may read data from files, memory, or a live simulation stream.
 */
class IFrameSource {
public:
    virtual ~IFrameSource() = default;

    /** @return Number of available frames in the source. */
    virtual std::size_t frameCount() const = 0;
    /** @return Frame by index. Implementations should throw on invalid index. */
    virtual const SimulationFrameData& frameAt(std::size_t index) const = 0;
};
