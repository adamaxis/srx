#include "shadowrun_ghidra/world_resource_batch_conversion.hpp"

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

std::uint32_t expand_random_nibble_mask(
    const std::vector<std::vector<std::uint8_t>>& pattern_banks,
    const std::size_t bank,
    const std::uint16_t random_index,
    const std::function<std::uint16_t()>& sample_random) {
    if (bank >= pattern_banks.size() || pattern_banks[bank].empty()) {
        return 0U;
    }
    const auto random = sample_random ? sample_random() : random_index;
    const auto& patterns = pattern_banks[bank];
    const auto pattern = patterns[(static_cast<std::size_t>(random) + 1U) % patterns.size()];
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
                                  std::uint8_t& global_state) {
    global_state = 0;
    const auto limit = std::min<std::size_t>(10U, records.size());
    for (std::size_t index = 0; index < limit; ++index) {
        auto& record = records[index];
        record.work_a = 0;
        record.work_b = 0;
        record.transient = 0;
        record.flags = static_cast<std::uint8_t>(record.flags & ~0x40U);
        record.mode_flags = static_cast<std::uint8_t>(record.mode_flags & ~0x10U);
    }
}

std::optional<std::size_t> select_next_entity(
    std::vector<EntityCycleRecord>& records,
    const std::size_t current,
    const bool special_input,
    const std::function<bool(const EntityCycleRecord&)>& eligible) {
    if (records.empty()) {
        return std::nullopt;
    }
    const auto start = current < records.size() ? current + 1U : 0U;
    for (std::size_t offset = 0; offset < records.size(); ++offset) {
        const auto index = (start + offset) % records.size();
        auto& record = records[index];
        if (special_input && record.busy) {
            record.busy = false;
        }
        if (!record.input_enabled || record.reserved || record.linked ||
            (eligible && !eligible(record))) {
            continue;
        }
        record.busy = true;
        return index;
    }
    return std::nullopt;
}

std::size_t clear_inactive_entity_effects(
    std::vector<EntityCycleRecord>& records,
    const std::function<void(std::size_t)>& clear_slot) {
    std::size_t cleared = 0U;
    const auto limit = std::min<std::size_t>(10U, records.size());
    for (std::size_t index = 0; index < limit; ++index) {
        auto& record = records[index];
        if (!record.reserved && record.busy && record.priority == 0) {
            record.busy = false;
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
    const std::function<void(std::size_t)>& clear_slot) {
    std::size_t cleared = 0U;
    const auto limit = std::min<std::size_t>(7U, records.size());
    for (std::size_t index = 0; index < limit; ++index) {
        if ((records[index].flags & 0x80U) == 0U) {
            continue;
        }
        records[index].flags = static_cast<std::uint8_t>(records[index].flags & ~0x80U);
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

void copy_two_word_rows(const std::vector<std::uint16_t>& source,
                        std::vector<std::uint16_t>& destination,
                        const std::size_t row_width,
                        const std::size_t source_stride,
                        const std::size_t destination_stride) {
    for (std::size_t row = 0; row < 2U; ++row) {
        for (std::size_t column = 0; column < row_width; ++column) {
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

void upload_decoded_source_stream(const std::function<void()>& decode_upload) {
    call_once(decode_upload);
}

} // namespace shadowrun::ghidra_conversion
