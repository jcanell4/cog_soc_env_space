#include "JsonFrameSource.h"

void JsonFrameSource::load(const std::string& path) {
    reader_.load(path);
}

std::size_t JsonFrameSource::frameCount() const {
    return reader_.frameCount();
}

const SimulationFrameData& JsonFrameSource::frameAt(std::size_t index) const {
    return reader_.frameAt(index);
}

SimulationFrameData JsonFrameSource::interpolate(double frame_position) const {
    return reader_.interpolate(frame_position);
}
