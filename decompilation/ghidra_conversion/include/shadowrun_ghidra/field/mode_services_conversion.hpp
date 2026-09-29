#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct SceneModeInitializationState {
    std::uint8_t current_mode = 0;            // FFDDC4
    std::uint16_t scene_transition_value = 0;  // FFDF40
    std::uint8_t scene_mode_marker = 0;        // FFF0A0
    std::uint16_t camera_state = 0;            // FFDF8E
    std::uint8_t special_mode_flag = 0;        // FFFC0E
    std::uint8_t special_mode_indicator = 0;   // FFE10E
    std::uint16_t table_select = 0;            // FFFC10
};

struct SceneModeInitializationCallbacks {
    std::function<std::uint8_t()> prepare_mode_entry;
};

// ROM 0x136DE: install the mode-entry result and clear the scene-mode
// transient markers. Mode six receives the source's special initialization.
void initialize_scene_mode_state(
    SceneModeInitializationState& state,
    const SceneModeInitializationCallbacks& callbacks);

enum class ModeExitScriptFamily {
    world_flags_bit0_clear,
    scene_table_match,
    scene_table_mismatch,
};

struct ModeExitState {
    std::uint8_t exit_service_marker = 0; // FFF10C
    std::uint32_t world_mode_flags = 0;   // FFFC1E
    std::uint8_t scene_compare_index = 0; // FFF09C
    std::uint8_t scene_id_byte = 0;       // FFFBF8
    ModeExitScriptFamily selected_script =
        ModeExitScriptFamily::world_flags_bit0_clear;
};

struct ModeExitCallbacks {
    std::function<void()> service_mode_exit;
    std::function<std::uint8_t(std::uint8_t)> scene_compare_byte;
};

// ROM 0x20714: choose 0xFE620 / 0xFE628 / 0xFE5D4 from FFFC1E bit 0 and
// 0xD445D[FFF09C] vs FFFBF8, CLR FFF10C, then BRA 0x1F246 (VDP callback).
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
