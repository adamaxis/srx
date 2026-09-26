#pragma once

#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

void write_repeated_menu_grid(const std::function<void()>& write_grid);
void prepare_menu_grid_then_service(
    const std::function<void()>& write_grid,
    const std::function<void()>& service_menu);
void prepare_grid_then_write(const std::function<void()>& prepare,
                             const std::function<void()>& write_grid);
void write_three_grids_then_service(
    const std::function<void()>& write_grid,
    const std::function<void()>& service_once);

struct MenuMetricLookupInput {
    // ROM FFF0D1 selects the six-entry action row; FFF0D0 selects its column.
    std::uint8_t menu_row = 0;
    std::uint8_t menu_column = 0;
    // Host copy of the byte table reached through FFF10E. The source performs
    // MULU.W row*6, ADD.B column, then LSL.W/ADDQ.W to select the byte.
    std::vector<std::uint8_t> source_action_indices;
    std::uint16_t value = 0;
    EntityActionRecord entity{};
    const EntityActionSpecialItemModifierTable* special_item_table = nullptr;
};

struct MenuMetricLookupResult {
    std::uint8_t action_index = 0;
    std::uint8_t matching_item_code = 0;
    EntityActionPhaseModifierResult modifiers{};
    std::size_t source_table_offset = 0;
};

// ROM 0x104C8 -> 0x104F6: select the source action index from the menu table,
// derive its item code, and run the shared phase/special-item continuation.
// A missing host table is unresolved rather than a fabricated action.
std::optional<MenuMetricLookupResult> resolve_menu_metric_action(
    const MenuMetricLookupInput& input);

void upload_resource_then_write_two_blocks(
    const std::function<void()>& upload_resource,
    const std::function<void()>& write_block);
void upload_resource_stream(const std::function<void()>& upload_stream);

struct EntityAuxiliaryState {
    std::uint8_t marker = 0;
};

// ROM 0x125FA: clear the auxiliary entity marker at source offset 0x4E.
void clear_entity_auxiliary_marker(EntityAuxiliaryState& state);

void write_single_grid(const std::function<void()>& write_grid);

struct ResourceVariantState {
    bool alternate = false;
};

// ROM 0x12A54: toggle the selected resource variant only when the input
// threshold crosses thirteen, then submit one source upload.
bool update_resource_variant(ResourceVariantState& state,
                            std::int16_t selection,
                            const std::function<void()>& upload_stream);

// ROM 0x13CD4: retain the source's bypass for phase one and threshold guard
// for all other entity phases.
bool passes_entity_action_threshold(
    std::int8_t entity_phase,
    std::int16_t requested_threshold,
    std::uint8_t current_threshold,
    const std::function<void()>& sample_threshold);

struct PaletteFadeState {
    bool active = false;
    std::int16_t remaining = 0;
    bool dirty = false;
    bool completed = false;
};

struct PaletteFadeCallbacks {
    std::function<void()> advance_effects;
    std::function<void()> step_palette_toward_black;
};

// ROM 0x16396: advance effect records, step the palette on even fade counts,
// and publish completion when the signed counter expires.
void advance_palette_fade(PaletteFadeState& state,
                          const PaletteFadeCallbacks& callbacks);

struct CoordinateProjectionInput {
    std::uint16_t first_word = 0;
    std::uint16_t second_word = 0;
    std::uint16_t orientation = 0;
};

// ROM 0x1669C / 0x166D0: project a source entity coordinate into a tile-grid
// offset without exposing packed Genesis records to the host runtime.
std::int32_t project_entity_x_axis(const CoordinateProjectionInput& input,
                                   std::int32_t row_stride);
std::int32_t project_entity_y_axis(const CoordinateProjectionInput& input,
                                   std::int32_t row_stride);

void draw_hud_panel_forward(const std::function<void()>& draw_panel);
void draw_hud_footer_forward(const std::function<void()>& draw_footer);

} // namespace shadowrun::ghidra_conversion
