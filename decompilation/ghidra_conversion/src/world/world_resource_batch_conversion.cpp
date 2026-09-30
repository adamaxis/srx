#include "shadowrun_ghidra/world/world_resource_batch_conversion.hpp"

#include <algorithm>
#include <limits>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void initialize_world_sequence(WorldSequenceState& state) {
    state.animation_phase = 0U;
    state.frame_countdown = 4U;
    state.active = true;
    state.source_start = 0x70U;
    state.source_end = 0xAFU;
    state.source_cursor = 0xAFU;
    state.sequence_table_token = 0x0001D428U;
    state.source_buffer_token = 0x000DA038U;
    state.destination_base = 0x811BU;
    state.current_row = 0x51U;
    state.substage = 0U;
}

void transfer_world_word_block(const std::function<void()>& transfer) {
    call_once(transfer);
}

void advance_world_sequence(
    const std::vector<WorldSequenceEntry>& entries,
    std::int16_t& countdown,
    std::int16_t& stage,
    const std::function<void(const WorldSequenceEntry&)>& process_entry) {
    for (const auto& entry : entries) {
        if (entry.marker == static_cast<std::int16_t>(0x8000)) {
            break;
        }
        if (process_entry) {
            process_entry(entry);
        }
    }
    countdown = std::max<std::int16_t>(0, static_cast<std::int16_t>(countdown - 1));
    if (countdown == 0) {
        ++stage;
    }
}

const std::vector<std::vector<std::uint8_t>>&
source_random_nibble_mask_banks() {
    // ROM 0x0F1C5E stores pointers to these nine source banks. Their leading
    // count bytes are 1, 8, 28, 56, 70, 56, 28, 8, and 1; the host vectors
    // retain only the selectable pattern bytes.
    static const std::vector<std::vector<std::uint8_t>> banks{
        {0x00U},
        {0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U},
        {0x03U, 0x05U, 0x06U, 0x09U, 0x0AU, 0x0CU, 0x11U, 0x12U,
         0x14U, 0x18U, 0x21U, 0x22U, 0x24U, 0x28U, 0x30U, 0x41U,
         0x42U, 0x44U, 0x48U, 0x50U, 0x60U, 0x81U, 0x82U, 0x84U,
         0x88U, 0x90U, 0xA0U, 0xC0U},
        {0x07U, 0x0BU, 0x0DU, 0x0EU, 0x13U, 0x15U, 0x16U, 0x19U,
         0x1AU, 0x1CU, 0x23U, 0x25U, 0x26U, 0x29U, 0x2AU, 0x2CU,
         0x31U, 0x32U, 0x34U, 0x38U, 0x43U, 0x45U, 0x46U, 0x49U,
         0x4AU, 0x4CU, 0x51U, 0x52U, 0x54U, 0x58U, 0x61U, 0x62U,
         0x64U, 0x68U, 0x70U, 0x83U, 0x85U, 0x86U, 0x89U, 0x8AU,
         0x8CU, 0x91U, 0x92U, 0x94U, 0x98U, 0xA1U, 0xA2U, 0xA4U,
         0xA8U, 0xB0U, 0xC1U, 0xC2U, 0xC4U, 0xC8U, 0xD0U, 0xE0U},
        {0x0FU, 0x17U, 0x1BU, 0x1DU, 0x1EU, 0x27U, 0x2BU, 0x2DU,
         0x2EU, 0x33U, 0x35U, 0x36U, 0x39U, 0x3AU, 0x3CU, 0x47U,
         0x4BU, 0x4DU, 0x4EU, 0x53U, 0x55U, 0x56U, 0x59U, 0x5AU,
         0x5CU, 0x63U, 0x65U, 0x66U, 0x69U, 0x6AU, 0x6CU, 0x71U,
         0x72U, 0x74U, 0x78U, 0x87U, 0x8BU, 0x8DU, 0x8EU, 0x93U,
         0x95U, 0x96U, 0x99U, 0x9AU, 0x9CU, 0xA3U, 0xA5U, 0xA6U,
         0xA9U, 0xAAU, 0xACU, 0xB1U, 0xB2U, 0xB4U, 0xB8U, 0xC3U,
         0xC5U, 0xC6U, 0xC9U, 0xCAU, 0xCCU, 0xD1U, 0xD2U, 0xD4U,
         0xD8U, 0xE1U, 0xE2U, 0xE4U, 0xE8U, 0xF0U},
        {0x1FU, 0x2FU, 0x37U, 0x3BU, 0x3DU, 0x3EU, 0x4FU, 0x57U,
         0x5BU, 0x5DU, 0x5EU, 0x67U, 0x6BU, 0x6DU, 0x6EU, 0x73U,
         0x75U, 0x76U, 0x79U, 0x7AU, 0x7CU, 0x8FU, 0x97U, 0x9BU,
         0x9DU, 0x9EU, 0xA7U, 0xABU, 0xADU, 0xAEU, 0xB3U, 0xB5U,
         0xB6U, 0xB9U, 0xBAU, 0xBCU, 0xC7U, 0xCBU, 0xCDU, 0xCEU,
         0xD3U, 0xD5U, 0xD6U, 0xD9U, 0xDAU, 0xDCU, 0xE3U, 0xE5U,
         0xE6U, 0xE9U, 0xEAU, 0xECU, 0xF1U, 0xF2U, 0xF4U, 0xF8U},
        {0x3FU, 0x5FU, 0x6FU, 0x77U, 0x7BU, 0x7DU, 0x7EU, 0x9FU,
         0xAFU, 0xB7U, 0xBBU, 0xBDU, 0xBEU, 0xCFU, 0xD7U, 0xDBU,
         0xDDU, 0xDEU, 0xE7U, 0xEBU, 0xEDU, 0xEEU, 0xF3U, 0xF5U,
         0xF6U, 0xF9U, 0xFAU, 0xFCU},
        {0x7FU, 0xBFU, 0xDFU, 0xEFU, 0xF7U, 0xFBU, 0xFDU, 0xFEU},
        {0xFFU},
    };
    return banks;
}

std::uint32_t expand_random_nibble_mask(
    const std::vector<std::vector<std::uint8_t>>& pattern_banks,
    const std::size_t bank,
    const std::function<std::uint16_t(std::uint16_t bound)>& sample_random) {
    if (bank >= pattern_banks.size() || pattern_banks[bank].empty()) {
        return 0U;
    }
    const auto& patterns = pattern_banks[bank];
    const auto sampled_index = sample_random
                                   ? sample_random(static_cast<std::uint16_t>(
                                         patterns.size()))
                                   : 0U;
    const auto pattern = patterns[static_cast<std::size_t>(sampled_index) %
                                  patterns.size()];
    std::uint32_t result = 0U;
    auto bits = pattern;
    for (int nibble = 0; nibble < 8; ++nibble) {
        result <<= 4U;
        result |= (bits & 0x80U) != 0U ? 0x0FU : 0U;
        bits = static_cast<std::uint8_t>((bits << 1U) | (bits >> 7U));
    }
    return result;
}

void reset_entity_service_records(std::vector<EntityServiceRecord>& records,
                                  std::int16_t& timeout_marker) {
    timeout_marker = 0;
    const auto limit = std::min<std::size_t>(10U, records.size());
    for (std::size_t index = 0; index < limit; ++index) {
        auto& record = records[index];
        record.pair_a2 = 0U;
        record.pair_ac = 0U;
        record.byte_43 = 0U;
        record.route_flags =
            static_cast<std::uint8_t>(record.route_flags & 0xBFU);
        record.animation_flags =
            static_cast<std::uint8_t>(record.animation_flags & 0xEFU);
    }
}

EntitySelectionResult select_next_entity(
    EntityServiceTargetOwner& owner,
    std::vector<EntityServiceTargetRecord>& records,
    const EntityServiceTargetCallbacks& callbacks) {
    // ROM 0x1DC22 scans ten 0x100-byte records at FF0100 and only enters the
    // shared selector if one already carries +0x11 bit 4. The conversion
    // keeps the table bounded by the supplied typed records.
    const auto scan_limit = std::min<std::size_t>(10U, records.size());
    const auto marked = std::find_if(
        records.begin(), records.begin() + static_cast<std::ptrdiff_t>(scan_limit),
        [](const EntityServiceTargetRecord& record) { return record.selected; });
    if (marked == records.begin() + static_cast<std::ptrdiff_t>(scan_limit)) {
        return EntitySelectionResult::no_marked_record;
    }
    const auto action = select_world_entity_service_target(owner, records, false, callbacks);
    return action == EntityServiceTargetAction::selected
               ? EntitySelectionResult::selected_target
               : EntitySelectionResult::no_available_target;
}

std::size_t clear_inactive_entity_effects(
    const std::vector<EntityServiceTargetRecord>& records,
    const std::function<void(std::size_t)>& clear_slot) {
    std::size_t cleared = 0U;
    const auto limit = std::min<std::size_t>(10U, records.size());
    for (std::size_t index = 0; index < limit; ++index) {
        const auto& record = records[index];
        // ROM 0x1DEA0 clears only +0x10 bit 6 records whose +0x10 bit 3 is
        // clear and whose +0x0C action word is zero.
        if (record.effect_locked && !record.effect_pending && !record.has_action) {
            if (clear_slot) {
                clear_slot(index);
            }
            ++cleared;
        }
    }
    return cleared;
}

std::size_t clear_high_effect_slots(
    std::vector<EffectHighFlagRecord>& records,
    bool& companion_restore,
    const std::function<void(std::size_t)>& clear_slot) {
    companion_restore = false;
    std::size_t cleared = 0U;
    const auto limit = std::min<std::size_t>(7U, records.size());
    for (std::size_t index = 0; index < limit; ++index) {
        if ((records[index].flags & 0x80U) == 0U) {
            continue;
        }
        if (clear_slot) {
            clear_slot(index);
        }
        ++cleared;
    }
    return cleared;
}

bool resolve_nearby_effects(
    std::vector<NearbyEffectRecord>& records,
    const std::uint16_t target_x,
    const std::uint16_t target_y,
    const std::function<std::pair<std::uint16_t, std::uint16_t>(
        const NearbyEffectRecord&)>& sample_position,
    const std::function<void()>& service_audio,
    const std::function<void(std::size_t)>& clear_slot) {
    for (std::size_t index = 0; index < std::min<std::size_t>(10U, records.size()); ++index) {
        auto& record = records[index];
        const auto distance_x = static_cast<std::int32_t>(target_x) - record.x;
        const auto distance_y = static_cast<std::int32_t>(target_y) - record.y;
        if (!record.active || distance_x < -0x100 || distance_x > 0x100 ||
            distance_y < -0x100 || distance_y > 0x100) {
            continue;
        }
        const auto sampled = sample_position ? sample_position(record)
                                             : std::make_pair(target_x, target_y);
        record.x = static_cast<std::int16_t>(sampled.first);
        record.y = static_cast<std::int16_t>(sampled.second);
        if (record.direction >= 0) {
            call_once(service_audio);
            record.flags = static_cast<std::uint8_t>(record.flags & ~0x01U);
            if (record.kind != 0x65U) {
                return true;
            }
            record.active = false;
            if (clear_slot) {
                clear_slot(index);
            }
            return true;
        }
    }
    return false;
}

std::size_t spawn_world_effect_sequence(
    const std::int16_t mode,
    const bool effects_enabled,
    const std::vector<std::uint32_t>& coordinates,
    const std::function<void()>& prepare_single_coordinate,
    const std::function<void()>& build_coordinate_candidates,
    const std::function<bool(std::size_t, std::uint32_t)>& spawn_coordinate) {
    if (mode == 0) {
        call_once(prepare_single_coordinate);
    } else {
        call_once(build_coordinate_candidates);
    }
    if (!effects_enabled) {
        return 0U;
    }
    std::size_t spawned = 0U;
    for (std::size_t index = 0; index < coordinates.size() && coordinates[index] != 0U;
         ++index) {
        if (!spawn_coordinate || !spawn_coordinate(index, coordinates[index])) {
            break;
        }
        ++spawned;
    }
    return spawned;
}

void subtract_resource_clamped(const std::int32_t amount, std::int32_t& resource) {
    if (amount <= resource) {
        resource -= amount;
    }
}

void run_fixed_delay(const std::size_t iterations,
                     const std::function<void(std::size_t)>& tick) {
    for (std::size_t index = 0; index < iterations; ++index) {
        if (tick) {
            tick(index);
        }
    }
}

std::size_t count_active_entities(
    std::vector<std::uint8_t>& field_state,
    const std::function<bool(std::size_t)>& is_active) {
    field_state.clear();
    std::size_t count = 0U;
    for (std::size_t index = 0; index < 3U; ++index) {
        const bool active = is_active && is_active(index);
        field_state.push_back(active ? 1U : 0U);
        count += active ? 1U : 0U;
    }
    return count;
}

void service_random_roll(const std::function<void()>& sample_roll) {
    call_once(sample_roll);
}

bool dispatch_indexed_handler(
    const std::int16_t index,
    const std::vector<std::function<void()>>& handlers) {
    if (index < 0 || static_cast<std::size_t>(index) >= handlers.size() ||
        !handlers[static_cast<std::size_t>(index)]) {
        return false;
    }
    handlers[static_cast<std::size_t>(index)]();
    return true;
}

void clear_grid_segments(GridClearState& state,
                         const std::int16_t decrement,
                         const std::size_t rows,
                         const std::size_t row_bytes,
                         const std::size_t row_stride) {
    state.remaining = static_cast<std::int16_t>(state.remaining - decrement);
    if (state.bytes.size() < rows * row_stride) {
        state.bytes.resize(rows * row_stride, 0);
    }
    for (std::size_t row = 0; row < rows; ++row) {
        const auto begin = state.bytes.begin() + static_cast<std::ptrdiff_t>(row * row_stride);
        std::fill_n(begin, std::min(row_bytes, row_stride), 0U);
    }
}

std::size_t write_sequential_asset_grid(
    const bool wide_variant,
    const std::size_t repetitions,
    const std::function<void()>& make_transfer_command,
    const std::function<void(std::uint16_t)>& write_word,
    const std::uint16_t first_word) {
    const auto words_per_row = wide_variant ? 10U : 8U;
    std::size_t written = 0U;
    auto word = first_word;
    for (std::size_t repetition = 0; repetition < repetitions; ++repetition) {
        call_once(make_transfer_command);
        for (std::size_t index = 0; index < words_per_row; ++index) {
            if (write_word) {
                write_word(word++);
            }
            ++written;
        }
    }
    return written;
}

void copy_tile_blocks(const std::vector<std::uint32_t>& source,
                      std::vector<std::uint32_t>& destination,
                      const std::size_t block_width,
                      const std::size_t block_height,
                      const std::size_t source_stride,
                      const std::size_t destination_stride) {
    for (std::size_t row = 0; row < block_height; ++row) {
        for (std::size_t column = 0; column < block_width; ++column) {
            const auto source_index = row * source_stride + column;
            const auto destination_index = row * destination_stride + column;
            if (source_index >= source.size()) {
                continue;
            }
            if (destination_index >= destination.size()) {
                destination.resize(destination_index + 1U);
            }
            destination[destination_index] = source[source_index];
        }
    }
}

std::size_t append_world_sprite_records(
    const std::vector<WorldSpriteSource>& sources,
    std::vector<WorldSpriteRecord>& records,
    const std::int16_t origin_x,
    const std::int16_t origin_y,
    std::uint8_t& next_attribute) {
    for (const auto& source : sources) {
        records.push_back({static_cast<std::int16_t>(origin_x + source.source_x),
                           static_cast<std::int16_t>(origin_y + source.source_y),
                           source.offset_x, source.offset_y, next_attribute++});
    }
    return sources.size();
}

bool copy_field_mode_selection_state(
    const std::vector<std::uint16_t>& source,
    std::vector<std::uint16_t>& destination) {
    if (source.size() < kFieldModeSelectionStateWordCount) {
        destination.clear();
        return false;
    }
    destination.assign(source.begin(),
                       source.begin() + kFieldModeSelectionStateWordCount);
    return true;
}

void upload_decoded_source_stream(const std::function<void()>& decode_upload) {
    call_once(decode_upload);
}

} // namespace shadowrun::ghidra_conversion
