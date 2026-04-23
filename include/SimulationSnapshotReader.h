#pragma once

#include "IFrameSource.h"

#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief Reads simulation snapshots written by JsonEcosystem.
 *
 * The reader is renderer-agnostic and can be reused by file playback
 * and future live visualization adapters.
 */
class SimulationSnapshotReader {
public:
    /** Load snapshot JSON from disk and populate frame data. */
    void load(const std::string& path);

    /** @return Number of loaded frames (initial + step_data entries). */
    std::size_t frameCount() const;

    /** @return Frame by index. Throws std::out_of_range on invalid index. */
    const SimulationFrameData& frameAt(std::size_t index) const;

    /**
     * @brief Returns an interpolated frame for fractional indices.
     *
     * Interpolation is linear for niche aggregate metrics. Cohort data is taken
     * from the closest source frame.
     */
    SimulationFrameData interpolate(double frame_position) const;

private:
    std::vector<SimulationFrameData> frames_;
};
