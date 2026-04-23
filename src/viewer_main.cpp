#include <cstdio>

#if defined(COG_SOC_ENV_SPACE_HAS_RAYLIB)

#include "JsonFrameSource.h"

#include <raylib.h>

#include <algorithm>
#include <array>
#include <exception>
#include <string>
#include <vector>

namespace {

constexpr int kWindowWidth = 1440;
constexpr int kWindowHeight = 900;
constexpr float kFramesPerSecond = 60.0F;
constexpr const char* kDefaultSnapshotPath = "output/simulation.json";
constexpr int kTopNCohorts = 8;

struct ChartMetric {
    const char* label;
    Color color;
    double SimulationFrameData::*member;
};

constexpr std::array<ChartMetric, 3> kChartMetrics = {{
    {"Nutrients", SKYBLUE, &SimulationFrameData::nutrients},
    {"Living biomass", GREEN, &SimulationFrameData::living_biomass},
    {"Death biomass", ORANGE, &SimulationFrameData::death_biomass},
}};

void drawHelpText() {
    DrawText("Controls: SPACE Play/Pause | LEFT/RIGHT Step | 1/2/3 Speed (0.5x/1x/2x) | R Restart | ESC Exit",
             20,
             16,
             18,
             LIGHTGRAY);
}

void drawHud(const SimulationFrameData& frame, std::size_t frame_index, std::size_t frame_count, float speed, bool playing) {
    char line[256];
    std::snprintf(line, sizeof(line), "Frame: %zu/%zu  |  Elapsed cycles: %d  |  Playback: %s  |  Speed: %.1fx",
                  frame_index + 1U,
                  frame_count,
                  frame.elapsed_cycles,
                  playing ? "Playing" : "Paused",
                  speed);
    DrawText(line, 20, 46, 20, WHITE);

    std::snprintf(line, sizeof(line), "Nutrients: %.3f  |  Ecological health: %.3f", frame.nutrients, frame.ecological_health);
    DrawText(line, 20, 74, 20, WHITE);

    std::snprintf(line, sizeof(line), "Living biomass: %.3f  |  Death biomass: %.3f  |  Decomposer biomass: %.3f",
                  frame.living_biomass,
                  frame.death_biomass,
                  frame.decomposer_biomass);
    DrawText(line, 20, 102, 20, WHITE);
}

void drawTimeSeries(const JsonFrameSource& source, std::size_t current_index, Rectangle area) {
    DrawRectangleLinesEx(area, 1.0F, GRAY);
    DrawText("Niche time-series", static_cast<int>(area.x), static_cast<int>(area.y) - 24, 20, LIGHTGRAY);

    if (source.frameCount() < 2U) {
        DrawText("Not enough frames", static_cast<int>(area.x + 10), static_cast<int>(area.y + 10), 16, GRAY);
        return;
    }

    for (std::size_t metric_index = 0; metric_index < kChartMetrics.size(); ++metric_index) {
        const ChartMetric& metric = kChartMetrics[metric_index];
        double min_value = source.frameAt(0).*metric.member;
        double max_value = min_value;
        for (std::size_t i = 1U; i < source.frameCount(); ++i) {
            const double value = source.frameAt(i).*metric.member;
            min_value = std::min(min_value, value);
            max_value = std::max(max_value, value);
        }
        const double range = std::max(1e-9, max_value - min_value);

        for (std::size_t i = 1U; i < source.frameCount(); ++i) {
            const float x0 = area.x + (area.width * static_cast<float>(i - 1U)) / static_cast<float>(source.frameCount() - 1U);
            const float x1 = area.x + (area.width * static_cast<float>(i)) / static_cast<float>(source.frameCount() - 1U);

            const double v0 = source.frameAt(i - 1U).*metric.member;
            const double v1 = source.frameAt(i).*metric.member;

            const float y0 = area.y + area.height - static_cast<float>((v0 - min_value) / range) * area.height;
            const float y1 = area.y + area.height - static_cast<float>((v1 - min_value) / range) * area.height;

            DrawLineEx(Vector2{x0, y0}, Vector2{x1, y1}, 2.0F, metric.color);
        }

        const int legend_x = static_cast<int>(area.x + 14.0F + static_cast<float>(metric_index) * 190.0F);
        const int legend_y = static_cast<int>(area.y + 12.0F);
        DrawRectangle(legend_x, legend_y + 6, 18, 8, metric.color);
        DrawText(metric.label, legend_x + 28, legend_y, 16, LIGHTGRAY);
    }

    const float marker_x = area.x + (area.width * static_cast<float>(current_index)) / static_cast<float>(source.frameCount() - 1U);
    DrawLineV(Vector2{marker_x, area.y}, Vector2{marker_x, area.y + area.height}, YELLOW);
}

void drawTopCohorts(const SimulationFrameData& frame, Rectangle area) {
    DrawRectangleLinesEx(area, 1.0F, GRAY);
    DrawText("Top cohorts by biomass", static_cast<int>(area.x), static_cast<int>(area.y) - 24, 20, LIGHTGRAY);

    if (frame.cohorts.empty()) {
        DrawText("No cohort data", static_cast<int>(area.x + 10), static_cast<int>(area.y + 10), 16, GRAY);
        return;
    }

    std::vector<CohortFrameData> sorted = frame.cohorts;
    std::sort(sorted.begin(),
              sorted.end(),
              [](const CohortFrameData& a, const CohortFrameData& b) { return a.total_biomass > b.total_biomass; });
    if (sorted.size() > static_cast<std::size_t>(kTopNCohorts)) {
        sorted.resize(static_cast<std::size_t>(kTopNCohorts));
    }

    double max_biomass = 0.0;
    for (const CohortFrameData& cohort : sorted) {
        max_biomass = std::max(max_biomass, cohort.total_biomass);
    }
    max_biomass = std::max(max_biomass, 1e-9);

    const float row_height = area.height / static_cast<float>(kTopNCohorts);
    for (std::size_t i = 0; i < sorted.size(); ++i) {
        const CohortFrameData& cohort = sorted[i];
        const float y = area.y + static_cast<float>(i) * row_height;
        const float normalized = static_cast<float>(cohort.total_biomass / max_biomass);
        const float bar_width = (area.width - 260.0F) * normalized;

        DrawRectangle(static_cast<int>(area.x + 240.0F), static_cast<int>(y + 6.0F), static_cast<int>(bar_width), static_cast<int>(row_height - 10.0F), DARKGREEN);

        char label[128];
        std::snprintf(label, sizeof(label), "#%d %s (%s)", cohort.id, cohort.specie_name.c_str(), cohort.class_name.c_str());
        DrawText(label, static_cast<int>(area.x + 8.0F), static_cast<int>(y + 8.0F), 16, LIGHTGRAY);

        std::snprintf(label, sizeof(label), "%.3f", cohort.total_biomass);
        DrawText(label, static_cast<int>(area.x + area.width - 90.0F), static_cast<int>(y + 8.0F), 16, WHITE);
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string snapshot_path = (argc >= 2 && argv[1] != nullptr && argv[1][0] != '\0')
                                          ? std::string(argv[1])
                                          : std::string(kDefaultSnapshotPath);

    JsonFrameSource frame_source;
    try {
        frame_source.load(snapshot_path);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Failed to load snapshot JSON '%s': %s\n", snapshot_path.c_str(), e.what());
        return 1;
    }

    if (frame_source.frameCount() == 0U) {
        std::fprintf(stderr, "Snapshot file has zero frames\n");
        return 1;
    }

    InitWindow(kWindowWidth, kWindowHeight, "cog_soc_env_space viewer (raylib)");
    SetTargetFPS(static_cast<int>(kFramesPerSecond));

    bool playing = true;
    float speed = 1.0F;
    double playback_index = 0.0;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            playing = !playing;
        }
        if (IsKeyPressed(KEY_ONE)) {
            speed = 0.5F;
        }
        if (IsKeyPressed(KEY_TWO)) {
            speed = 1.0F;
        }
        if (IsKeyPressed(KEY_THREE)) {
            speed = 2.0F;
        }
        if (IsKeyPressed(KEY_R)) {
            playback_index = 0.0;
        }

        if (!playing) {
            if (IsKeyPressed(KEY_RIGHT)) {
                playback_index = std::min(playback_index + 1.0, static_cast<double>(frame_source.frameCount() - 1U));
            }
            if (IsKeyPressed(KEY_LEFT)) {
                playback_index = std::max(0.0, playback_index - 1.0);
            }
        }

        if (playing) {
            playback_index += static_cast<double>(GetFrameTime() * speed * 12.0F);
            if (playback_index >= static_cast<double>(frame_source.frameCount())) {
                playback_index = 0.0;
            }
        }

        const std::size_t current_index = std::min(static_cast<std::size_t>(playback_index), frame_source.frameCount() - 1U);
        const SimulationFrameData& current_frame = frame_source.frameAt(current_index);

        BeginDrawing();
        ClearBackground(Color{18, 18, 22, 255});

        drawHelpText();
        drawHud(current_frame, current_index, frame_source.frameCount(), speed, playing);

        drawTimeSeries(frame_source, current_index, Rectangle{20.0F, 150.0F, 1400.0F, 320.0F});
        drawTopCohorts(current_frame, Rectangle{20.0F, 530.0F, 1400.0F, 340.0F});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#else

int main() {
    std::fprintf(stderr,
                 "Viewer built without raylib support. Reconfigure with -DENABLE_RAYLIB_VIEWER=ON "
                 "and ensure raylib dependencies are available.\n");
    return 1;
}

#endif
