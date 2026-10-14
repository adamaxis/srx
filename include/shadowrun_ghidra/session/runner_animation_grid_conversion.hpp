#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kRunnerAnimationGridWidth = 14U;
constexpr std::size_t kRunnerAnimationGridHeight = 10U;
constexpr std::size_t kRunnerAnimationGridCellCount =
    kRunnerAnimationGridWidth * kRunnerAnimationGridHeight;

enum class RunnerAnimationGridDirection {
    north,
    east,
    south,
    west,
};

enum class RunnerAnimationGridStep {
    blocked,
    mark_only,
    recurse,
};

struct RunnerAnimationGridPosition {
    std::int8_t x = 0; // compact record +3
    std::int8_t y = 0; // compact record +4
};

struct RunnerAnimationGridState {
    std::array<std::uint8_t, kRunnerAnimationGridCellCount> marked{};
    std::int16_t start_cell = -1;
    std::size_t marked_count = 0;
    std::uint16_t recursion_guard = 0;
};

struct RunnerAnimationGridCallbacks {
    std::function<void()> select_primary_runner_record_table;
    std::function<RunnerAnimationGridPosition()> resolve_selected_position;
    std::function<RunnerAnimationGridStep(
        std::int16_t from,
        std::int16_t to,
        RunnerAnimationGridDirection direction)>
        classify_step;
    std::function<void(std::int16_t)> mark_cell;
};

// ROM 0x53434: recursively expand one marked grid cell. The source's four
// guard bits are explicit here; the tile/record transition predicate remains a
// host callback until the four ROM type tables are fully assigned names.
void expand_runner_animation_grid(
    RunnerAnimationGridState& state,
    std::int16_t current_cell,
    const RunnerAnimationGridCallbacks& callbacks,
    std::uint16_t recursion_guard = 0);

// ROM 0x533EE: resolve the selected compact record's +3/+4 position, mark its
// 14-wide grid cell, and begin the guarded expansion.
bool initialize_runner_animation_grid(
    RunnerAnimationGridState& state,
    const RunnerAnimationGridCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
