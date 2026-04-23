#pragma once

#include "IFrameSource.h"
#include "SimulationSnapshotReader.h"

#include <string>

/**
 * @brief IFrameSource implementation backed by snapshot JSON playback.
 */
class JsonFrameSource : public IFrameSource {
public:
    void load(const std::string& path);

    std::size_t frameCount() const override;
    const SimulationFrameData& frameAt(std::size_t index) const override;

    SimulationFrameData interpolate(double frame_position) const;

private:
    SimulationSnapshotReader reader_;
};
