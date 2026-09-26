#pragma once

#include "shadowrun_ghidra/entities/entity_service_target_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

void transfer_world_word_block(const std::function<void()>& transfer);

struct WorldSequenceEntry {
    std::int16_t marker = 0;
};

struct WorldSequenceState {
    std::uint16_t animation_phase = 0; // FFE0AE
    std::uint16_t frame_countdown = 0; // FFE0B0
    bool active = false;               // FFDF8C
    std::uint16_t source_start = 0;   // FFDF76
    std::uint16_t source_end = 0;     // FFDF78
    std::uint16_t source_cursor = 0;  // FFDF72
    std::uint32_t sequence_table_token = 0; // FFDF7A
    std::uint32_t source_buffer_token = 0;  // FFDF7E
    std::uint16_t destination_base = 0; // FFDF74
    std::uint16_t current_row = 0;    // FFDF70
    std::uint16_t substage = 0;       // FFDF82
};

// ROM 0x1D280: install the fixed world-sequence table, source buffer,
// destination, row range, and initial counters used by 0x1D2E6.
void initialize_world_sequence(WorldSequenceState& state);

void advance_world_sequence(
    const std::vector<WorldSequenceEntry>& entries,
    std::int16_t& countdown,
    std::int16_t& stage,
    const std::function<void(const WorldSequenceEntry&)>& process_entry);

std::uint32_t expand_random_nibble_mask(
    const std::vector<std::vector<std::uint8_t>>& pattern_banks,
    std::size_t bank,
    std::uint16_t random_index,
    const std::function<std::uint16_t()>& sample_random);

struct EntityServiceRecord {
    std::uint32_t work_a = 0;
    std::uint32_t work_b = 0;
    std::uint8_t transient = 0;
    std::uint8_t flags = 0;
    std::uint8_t mode_flags = 0;
};

void reset_entity_service_records(std::vector<EntityServiceRecord>& records,
                                  std::uint8_t& global_state);

enum class EntitySelectionResult {
    no_marked_record,
    selected_target,
    no_available_target,
};

// ROM 0x1DC22: find a selected bit in the ten-record entity table, then enter
// 0x1DC4C with D1 set to the 0x678CC sentinel. This is the wrapper/gate around
// the typed service-target selector, not a generic circular entity iterator.
EntitySelectionResult select_next_entity(
    EntityServiceTargetOwner& owner,
    std::vector<EntityServiceTargetRecord>& records,
    const EntityServiceTargetCallbacks& callbacks = {});

std::size_t clear_inactive_entity_effects(
    const std::vector<EntityServiceTargetRecord>& records,
    const std::function<void(std::size_t)>& clear_slot);

struct EffectHighFlagRecord {
    std::uint8_t flags = 0;
};

std::size_t clear_high_effect_slots(
    std::vector<EffectHighFlagRecord>& records,
    const std::function<void(std::size_t)>& clear_slot);

struct NearbyEffectRecord {
    bool active = false;
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t direction = 0;
    std::uint16_t kind = 0;
    std::uint8_t flags = 0;
};

bool resolve_nearby_effects(
    std::vector<NearbyEffectRecord>& records,
    std::uint16_t target_x,
    std::uint16_t target_y,
    const std::function<std::pair<std::uint16_t, std::uint16_t>(
        const NearbyEffectRecord&)>& sample_position,
    const std::function<void()>& service_audio,
    const std::function<void(std::size_t)>& clear_slot);

std::size_t spawn_world_effect_sequence(
    std::int16_t mode,
    bool effects_enabled,
    const std::vector<std::uint32_t>& coordinates,
    const std::function<void()>& prepare_single_coordinate,
    const std::function<void()>& build_coordinate_candidates,
    const std::function<bool(std::size_t, std::uint32_t)>& spawn_coordinate);

void subtract_resource_clamped(std::int32_t amount, std::int32_t& resource);

void run_fixed_delay(std::size_t iterations,
                     const std::function<void(std::size_t)>& tick);

std::size_t count_active_entities(
    std::vector<std::uint8_t>& field_state,
    const std::function<bool(std::size_t)>& is_active);

void service_random_roll(const std::function<void()>& sample_roll);

bool dispatch_indexed_handler(
    std::int16_t index,
    const std::vector<std::function<void()>>& handlers);

struct GridClearState {
    std::int16_t remaining = 0;
    std::vector<std::uint8_t> bytes;
};

void clear_grid_segments(GridClearState& state,
                         std::int16_t decrement,
                         std::size_t rows,
                         std::size_t row_bytes,
                         std::size_t row_stride);

std::size_t write_sequential_asset_grid(
    bool wide_variant,
    std::size_t repetitions,
    const std::function<void()>& make_transfer_command,
    const std::function<void(std::uint16_t)>& write_word,
    std::uint16_t first_word);

void copy_tile_blocks(const std::vector<std::uint32_t>& source,
                      std::vector<std::uint32_t>& destination,
                      std::size_t block_width,
                      std::size_t block_height,
                      std::size_t source_stride,
                      std::size_t destination_stride);

struct WorldSpriteSource {
    std::int16_t source_x = 0;
    std::int16_t source_y = 0;
    std::int16_t offset_x = 0;
    std::int16_t offset_y = 0;
    std::uint8_t attribute = 0;
};

struct WorldSpriteRecord {
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t tile = 0;
    std::int16_t depth = 0;
    std::uint8_t attribute = 0;
};

std::size_t append_world_sprite_records(
    const std::vector<WorldSpriteSource>& sources,
    std::vector<WorldSpriteRecord>& records,
    std::int16_t origin_x,
    std::int16_t origin_y,
    std::uint8_t& next_attribute);

void copy_two_word_rows(const std::vector<std::uint16_t>& source,
                        std::vector<std::uint16_t>& destination,
                        std::size_t row_width,
                        std::size_t source_stride,
                        std::size_t destination_stride);

void upload_decoded_source_stream(const std::function<void()>& decode_upload);

} // namespace shadowrun::ghidra_conversion
