#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct WrappedTextCallbacks {
    std::function<void()> begin_transfer;
    std::function<std::uint16_t(std::uint8_t)> glyph_for_byte;
    std::function<void(std::uint16_t)> write_glyph;
};

// ROM 0x0C160: stream a bounded line of encoded text, stopping at a high-bit
// terminator and wrapping at the supplied character width.
std::size_t render_wrapped_text(const std::vector<std::uint8_t>& encoded,
                                std::size_t line_width,
                                const WrappedTextCallbacks& callbacks);

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

void refresh_mode_glyph(
    std::vector<std::uint8_t>& mode_values,
    std::size_t mode_index,
    const std::vector<std::uint8_t>& remap_table,
    const std::function<void()>& write_grid);

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

void run_menu_slot_service(
    std::uint8_t& pending_flags,
    std::uint8_t pending_mask,
    const MenuServiceCallbacks& callbacks);

void merge_menu_pending_flags(
    std::uint8_t& pending_flags,
    std::uint8_t source_flags,
    const std::function<void()>& service_display);

void run_menu_metadata_service(const MenuServiceCallbacks& callbacks);

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
