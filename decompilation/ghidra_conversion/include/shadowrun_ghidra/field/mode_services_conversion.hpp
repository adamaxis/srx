#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct SceneModeInitializationState {
    std::uint8_t current_mode = 0;
    std::uint16_t scene_transition_value = 0;
    std::uint8_t scene_mode_marker = 0;
    std::uint16_t camera_state = 0;
    std::uint8_t special_mode_flag = 0;
    std::uint8_t special_mode_indicator = 0;
};

struct SceneModeInitializationCallbacks {
    std::function<std::uint8_t()> prepare_mode_entry;
};

// ROM 0x136DE: install the mode-entry result and clear the scene-mode
// transient markers. Mode six receives the source's special initialization.
void initialize_scene_mode_state(
    SceneModeInitializationState& state,
    const SceneModeInitializationCallbacks& callbacks);

struct ModeExitState {
    std::uint8_t exit_service_marker = 0;
};

struct ModeExitCallbacks {
    std::function<void()> service_mode_exit;
};

// ROM 0x20714: clear the exit-service marker and enter the existing exit
// service boundary at 0x1F246.
void finalize_mode_exit(ModeExitState& state,
                        const ModeExitCallbacks& callbacks);

// ROM 0x0E6F4: source table families selected by the first two bounded random
// draws. The byte offset is the source's D5 adjustment applied to the chosen
// variant resource; it is provenance, not a host pointer or ROM lookup.
enum class ModeWorldPrimarySource {
    first_zero_second_zero_table,
    first_zero_second_nonzero_table,
    first_nonzero_second_zero_table,
    first_nonzero_second_nonzero_table,
};

enum class ModeWorldVariantSource {
    fixed_three_variant_resources,
    indexed_four_variant_resources,
};

struct ModeWorldGridPlan {
    bool first_selection_nonzero = false;
    bool second_selection_nonzero = false;
    ModeWorldPrimarySource primary_source =
        ModeWorldPrimarySource::first_zero_second_zero_table;
    std::uint8_t primary_entry_index = 0;
    ModeWorldVariantSource variant_source =
        ModeWorldVariantSource::fixed_three_variant_resources;
    std::uint8_t variant_entry_index = 0;
    std::uint8_t variant_byte_offset = 0;
    std::uint8_t grid_row_count = 2;
};

struct ModeWorldRefreshState {
    std::uint8_t random_samples = 0;
    bool grid_write_requested = false;
    ModeWorldGridPlan plan{};
};

struct ModeWorldRefreshCallbacks {
    std::function<std::uint16_t(std::uint16_t bound)> sample_random;
    std::function<void(const ModeWorldGridPlan& plan)> write_world_grid;
};

// ROM 0x0E6F4: consume the four source random samples and request the world
// grid write. The random generator and grid writer remain host boundaries;
// the selected table family and row count remain typed source evidence.
void service_mode_world_state(
    ModeWorldRefreshState& state,
    const ModeWorldRefreshCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
