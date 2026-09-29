#pragma once

#include "shadowrun_ghidra/menus/menu_hud_service_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_slot_batch_conversion.hpp"
#include "shadowrun_ghidra/rendering/wrapped_encoded_text_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// Kept as a source-family alias for existing field-menu conversion callers;
// the implementation belongs to the rendering/display-text boundary because
// ROM 0xC168 is a shared encoded-text transfer routine.
using WrappedTextCallbacks = WrappedEncodedTextCallbacks;

bool dispatch_menu_handler(
    std::size_t selected,
    const std::vector<std::function<void()>>& handlers);

bool pass_entity_action_threshold_variant(
    std::uint8_t entity_phase,
    std::int16_t requested_threshold,
    std::uint8_t current_threshold,
    const std::function<void()>& sample_threshold);

struct EffectServiceState {
    std::uint8_t mode = 0;
    bool effect_active = false;
    std::vector<bool> active_slots;
};

std::size_t service_active_effect_slots(
    const EffectServiceState& state,
    const std::function<void(std::size_t)>& service_slot);

struct EntityDescriptorPair {
    std::int32_t id = 0;
    std::int32_t value = 0;
};

struct EntityTableRecord {
    std::int16_t status = 0;
    std::int32_t id = 0;
    std::int32_t value = 0;
    std::uint32_t behavior_token = 0;
    std::uint16_t field_a = 0;
    std::uint16_t field_b = 0;
};

std::size_t populate_entity_table(
    const std::vector<EntityDescriptorPair>& descriptors,
    std::vector<EntityTableRecord>& records,
    const std::function<std::optional<std::size_t>()>& allocate_slot);

bool mark_projected_terrain_cell(
    std::vector<std::uint16_t>& terrain_words,
    const std::function<std::optional<std::size_t>()>& project_cell,
    std::uint8_t& marked_count,
    bool& render_dirty);

struct WorldActivityDescriptor {
    std::uint32_t id = 0;
    std::uint32_t descriptor_offset = 0;
};

std::size_t publish_world_activity_descriptors(
    std::vector<std::uint32_t>& source_ids,
    std::vector<WorldActivityDescriptor>& published,
    bool& active,
    const std::function<void()>& schedule_timeout);

struct TerrainWorkEntry {
    std::int16_t id = 0;
    std::int16_t payload = 0;
    bool alternate_region = false;
};

struct TerrainWorkCallbacks {
    std::function<void(const TerrainWorkEntry&)> mark_boundary;
    std::function<bool()> copy_region_rows;
};

bool process_terrain_work_batch(
    const std::vector<TerrainWorkEntry>& entries,
    const std::vector<bool>& markers,
    const TerrainWorkCallbacks& callbacks,
    bool& render_dirty);

void refresh_field_hud(
    const std::function<void()>& advance_footer,
    const std::function<void()>& write_grid);

// ROM 0x19856: footer delta 0, then five 3x1 grids at ED14+i*C from
// 0xD84D6+FFF0ED[i]*6, mask 0x6000.
void refresh_field_hud(
    HudOffsetState& footer,
    const std::array<std::uint8_t, 5>& status,
    const WordTransferCallbacks& callbacks);

void refresh_mode_glyph(
    std::vector<std::uint8_t>& mode_values,
    std::size_t mode_index,
    const std::vector<std::uint8_t>& remap_table,
    const std::function<void()>& write_grid);

constexpr std::array<std::uint8_t, 12> kModeGlyphRemap{
    0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 0, 0};
constexpr std::uint16_t kModeGlyphGridDestination = 0xC22EU;
constexpr std::uint16_t kModeGlyphGridWord = 0x05ABU;
constexpr std::size_t kModeGlyphGridWidth = 14U;

// ROM 0x19AEE: remap FF1100+FFF118*0x14+0x13 via 0xE31DD, then 14x 0x5AB at C22E.
void refresh_mode_glyph(
    std::vector<std::uint8_t>& records,
    std::uint16_t selected_index,
    const WordTransferCallbacks& callbacks);

std::optional<std::uint8_t> select_hud_mode(
    std::uint8_t display_mode,
    std::uint8_t slot_value,
    const std::vector<std::uint8_t>& mode_table);

std::uint32_t pass_through_value(std::uint32_t value);

struct MenuServiceCallbacks {
    std::function<void()> set_selection;
    std::function<void()> render_text;
    std::function<void()> refresh_grid;
    std::function<void()> prepare_offset;
    std::function<void()> service_auxiliary;
    std::function<void()> render_progression;
    std::function<void()> service_secondary;
};

enum class MenuSlotServicePath {
    cleared_empty_slot,
    pending_slot,
};

struct MenuSlotServiceState {
    // Signed source byte FFFB30[FFF102]. Values below one take the empty-slot
    // path; a positive value takes the pending-slot path.
    std::int8_t pending_marker = 0;
    bool pending_marker_cleared = false;
};

struct MenuSlotServiceExactCallbacks {
    // ROM 0x17C84 receives D0=0x0B and the path-specific D1 value.
    std::function<void(std::uint8_t source_code, std::uint8_t value)>
        set_hud_selection;
    std::function<void()> render_menu_text_followup;
    std::function<void()> refresh_hud_grid_if_ready;
    std::function<void()> prepare_menu_offset;
    std::function<void()> merge_menu_pending_flags;
    std::function<void()> render_menu_text_progression;
    std::function<void()> render_menu_slot_status;
};

// ROM 0x19D66: publish the selected HUD code as 0x0A for an empty/negative
// slot, or 0x06 for a pending slot. Only the empty path prepares the menu
// offset; the pending path merges the runner flags and renders its
// progression/status text.
MenuSlotServicePath run_menu_slot_service(
    MenuSlotServiceState& state,
    const MenuSlotServiceExactCallbacks& callbacks);

void run_menu_slot_service(
    std::uint8_t& pending_flags,
    std::uint8_t pending_mask,
    const MenuServiceCallbacks& callbacks);

struct MenuPendingFlagState {
    // FFFB30 indexed by FFF102.
    std::vector<std::uint8_t> pending_flags;
    std::size_t selected_mode_index = 0;
    // FF1100 compact runner records, each 0x14 bytes; FFF118 selects one.
    std::vector<std::uint8_t> runner_records;
    std::size_t selected_runner_index = 0;
};

struct MenuPendingFlagMergeResult {
    std::uint8_t source_flags = 0;
    std::size_t metadata_offset = 0;
    std::uint8_t merged_flags = 0;
};

// ROM 0x19DAC -> 0x19DD8: read runner-record byte +0x5, merge it into the
// selected pending-mode byte, and select the E2D52 metadata row by flags*0x20.
// Missing host records are unresolved rather than treated as zero.
std::optional<MenuPendingFlagMergeResult> merge_menu_pending_flags(
    MenuPendingFlagState& state,
    const std::function<void(std::size_t metadata_offset)>& refresh_metadata);

void run_menu_metadata_service(const MenuServiceCallbacks& callbacks);

struct MenuMetadataServiceState {
    std::uint16_t eligibility_value = 0;
    std::uint8_t selected_runner_flags = 0;
    std::optional<std::size_t> metadata_offset;
};

struct MenuMetadataServiceCallbacks {
    // ROM 0x17C3C and the compact selected-runner record byte +0x05.
    std::function<std::optional<std::uint16_t>()>
        read_profile_eligibility;
    std::function<std::optional<std::uint8_t>()>
        read_selected_runner_flags;
    // ROM 0x19DD8 consumes D0 as a byte-offset into the mode metadata table.
    std::function<void(std::size_t metadata_offset)> refresh_mode_metadata;
};

// ROM 0x19DF2: read the eligibility nibble, optionally derive the selected
// runner's metadata row as flags*0x20, and always refresh the metadata leaf
// once the required extracted source data is available.
bool run_menu_metadata_service(
    MenuMetadataServiceState& state,
    const MenuMetadataServiceCallbacks& callbacks);

struct MenuSelectionServiceState {
    // Signed source marker FFF0D5.
    std::int8_t marker = 0;
};

struct MenuSelectionServiceExactCallbacks {
    std::function<void(std::uint8_t source_code, std::uint8_t value)>
        set_hud_selection;
    std::function<void()> render_menu_text_followup;
    std::function<void()> prepare_menu_offset;
    std::function<void(std::uint8_t delta)>
        advance_menu_selection_service_timer;
    std::function<void()> refresh_hud_grid_if_ready;
};

// ROM 0x19E32: D0=0, map the signed marker to D1=7/8/9, and run the
// positive-marker timer/grid branch before the unconditional preparation.
void run_menu_selection_service(
    const MenuSelectionServiceState& state,
    const MenuSelectionServiceExactCallbacks& callbacks);

struct MenuSelectionVariantState {
    // Signed source marker FFF0D6 and source byte FFFBBE+0x0F.
    std::int8_t marker = 0;
    std::uint8_t timing_byte = 0;
    std::uint16_t computed_delta = 0;
};

struct MenuSelectionVariantExactCallbacks {
    std::function<void(std::uint8_t source_code, std::uint8_t value)>
        set_hud_selection;
    std::function<void()> render_menu_text_followup;
    std::function<void(std::uint8_t delta)>
        advance_menu_selection_variant_timer;
    std::function<void()> prepare_menu_offset;
};

// ROM 0x19EA6: D0=1, map the signed marker to D1=7/8/9, and when positive
// call the variant timer with ((marker*2)+(timing_byte>>1)) low-byte D1.
void run_menu_selection_variant(
    MenuSelectionVariantState& state,
    const MenuSelectionVariantExactCallbacks& callbacks);

struct MenuProgressionVariantState {
    // Signed source marker FFF0D7.
    std::int8_t marker = 0;
};

struct MenuProgressionVariantExactCallbacks {
    std::function<void(std::uint8_t source_code, std::uint8_t value)>
        set_hud_selection;
    std::function<void()> render_menu_text_followup;
    std::function<void(std::uint16_t delta)>
        reduce_menu_progression_timing;
    std::function<void()> render_menu_text_progression;
    std::function<void()> prepare_menu_offset;
};

// ROM 0x19F18: D0=2, map the signed marker to D1=7/8/9, reduce progression
// timing only for a positive marker, then prepare the menu offset.
void run_menu_progression_variant(
    const MenuProgressionVariantState& state,
    const MenuProgressionVariantExactCallbacks& callbacks);

struct ReadyMenuSlotState {
    // FFFBBE+0x1A, FFF0E8, FFF0F2, and FFF0ED respectively.
    std::array<std::uint8_t, 5> slot_codes{};
    std::array<std::uint8_t, 5> active_values{};
    std::array<std::uint16_t, 5> load_offsets{};
    std::array<std::uint8_t, 5> row_status{};
};

struct ReadyMenuSlotCallbacks {
    std::function<std::optional<std::uint16_t>()>
        calculate_matrix_program_load_steps;
    std::function<void()> refresh_menu_number_or_grid;
    std::function<void(std::size_t)> clear_menu_status;
};

// ROM 0x19F7E: scan the five HUD/menu slots from four down to zero for code
// seven, clear its active value, publish load_steps*10, refresh the number/
// grid branch, and clear that slot's status row.
std::optional<std::size_t> clear_first_ready_menu_slot(
    ReadyMenuSlotState& state,
    const ReadyMenuSlotCallbacks& callbacks);

void run_menu_selection_service(
    std::uint8_t display_ready,
    const MenuServiceCallbacks& callbacks);

void run_menu_selection_variant(
    std::uint8_t display_ready,
    const MenuServiceCallbacks& callbacks);

void run_menu_progression_variant(
    std::uint8_t display_ready,
    const MenuServiceCallbacks& callbacks);

std::optional<std::size_t> clear_first_ready_menu_slot(
    const std::vector<std::uint8_t>& statuses,
    const std::function<void(std::size_t)>& clear_visual,
    const std::function<void(std::size_t)>& refresh_number,
    const std::function<void(std::size_t)>& clear_status);

} // namespace shadowrun::ghidra_conversion
