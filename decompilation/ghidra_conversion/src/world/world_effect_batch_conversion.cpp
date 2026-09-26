#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

void copy_words(const std::vector<std::uint16_t>& source,
                std::vector<std::uint16_t>& destination,
                const std::size_t count) {
    destination.assign(count, 0);
    const auto copy_count = std::min(count, source.size());
    std::copy_n(source.begin(), copy_count, destination.begin());
}

} // namespace

void prepare_menu_offset_once(
    MenuOffsetPreparationState& state,
    const std::function<std::int32_t()>& calculate_offset,
    const std::function<void()>& initialize_followup) {
    if (state.prepared) {
        return;
    }
    state.prepared = true;
    state.offset = calculate_offset ? calculate_offset() : 0;
    call_once(initialize_followup);
}

void copy_seven_words(const std::vector<std::uint16_t>& source,
                      std::vector<std::uint16_t>& destination) {
    copy_words(source, destination, 7);
}

void copy_eleven_words(const std::vector<std::uint16_t>& source,
                       std::vector<std::uint16_t>& destination) {
    copy_words(source, destination, 11);
}

void initialize_world_effect_motion(
    WorldEffectMotionState& state,
    const bool alternate_motion,
    const std::function<void()>& service_audio) {
    state.horizontal = alternate_motion ? -18 : -10;
    state.origin_x = 0x1340;
    state.origin_y = 0x1020;
    state.destination_x = 0;
    state.destination_y = 0;
    state.lifetime = -1;
    state.palette = 0x1E;
    state.audio_started = true;
    call_once(service_audio);
}

void clear_world_effect_work(
    const bool source_active,
    std::array<std::uint32_t, 7>& work_values) {
    if (!source_active) {
        work_values.fill(0);
    }
}

std::uint16_t advance_effect_vertical_position(const std::uint16_t value) {
    return value < 0x00F4U ? static_cast<std::uint16_t>(value + 2U) : value;
}

std::uint16_t retreat_effect_vertical_position(const std::uint16_t value) {
    return value > 0x00AFU ? static_cast<std::uint16_t>(value - 2U) : value;
}

std::uint16_t advance_effect_horizontal_position(const std::uint16_t value) {
    return value < 0x0198U ? static_cast<std::uint16_t>(value + 2U) : value;
}

std::uint16_t retreat_effect_horizontal_position(const std::uint16_t value) {
    return value > 0x00A7U ? static_cast<std::uint16_t>(value - 2U) : value;
}

void forward_effect_sequence(const std::function<void()>& sequence) {
    call_once(sequence);
}

bool begin_world_activity_if_idle(WorldActivityState& state,
                                  const std::uint16_t source_value) {
    if (state.timeout != 0U) {
        return false;
    }
    state.timeout = 0x3CU;
    state.source_value = source_value;
    return true;
}

void begin_world_activity(WorldActivityState& state,
                          const std::uint16_t source_value) {
    state.timeout = 0x3CU;
    state.source_value = source_value;
}

WorldActivityRequestAction handle_world_activity_request(
    WorldActivityRequestState& state,
    const std::vector<WorldActivityRequestRecord>& records,
    const std::function<void()>& begin_activity) {
    state.reset_a = 0U;
    state.reset_b = 0U;
    state.reset_c = 0U;
    state.reset_d = 0U;
    if (!state.request_pending) {
        return WorldActivityRequestAction::no_request;
    }

    state.request_pending = false;
    state.availability = -1;
    const auto count = std::min<std::size_t>(13U, records.size());
    for (std::size_t index = 0; index < count; ++index) {
        if (records[index].active && records[index].high_effect) {
            state.availability = 0;
            break;
        }
    }
    state.activity_handler_token = 0x0001F16AU;
    if (begin_activity) {
        begin_activity();
    }
    return WorldActivityRequestAction::activity_started;
}

void build_world_effect_candidates(
    WorldEffectCandidateState& state,
    const std::int16_t first_x,
    const std::int16_t first_y,
    const std::int16_t second_x,
    const std::int16_t second_y,
    const WorldEffectCandidateCallbacks& callbacks) {
    state.candidates.clear();
    constexpr std::array<std::int16_t, 5> offsets{0x100, 0x300, 0x500,
                                                   -0x100, -0x300};
    const auto add_ring = [&](const std::int16_t x, const std::int16_t y) {
        for (const auto offset : offsets) {
            const auto candidate = std::make_pair(x, static_cast<std::int16_t>(y + offset));
            if (!callbacks.is_valid || callbacks.is_valid(candidate.first,
                                                          candidate.second)) {
                state.candidates.push_back(candidate);
            }
        }
    };
    add_ring(first_x, first_y);
    add_ring(second_x, second_y);
}

std::optional<std::uint8_t> lookup_world_effect_tile(
    const std::vector<std::uint8_t>& tile_table,
    const std::uint16_t first_coordinate,
    const std::uint16_t second_coordinate) {
    const auto index = static_cast<std::size_t>(first_coordinate >> 9U) +
                       static_cast<std::size_t>(second_coordinate >> 4U &
                                                0x7FE0U) *
                           2U;
    if (index >= tile_table.size()) {
        return std::nullopt;
    }
    return tile_table[index];
}

bool clear_effect_slot_state(EffectSlotState& state,
                             const std::function<void()>& clear_slot) {
    const bool was_high = (state.effect_flags & 0x80U) != 0U;
    state.effect_flags = static_cast<std::uint8_t>(state.effect_flags &
                                                   ~0x80U);
    if (was_high) {
        state.global_dirty = false;
    }
    state.entity_flags = static_cast<std::uint8_t>(state.entity_flags &
                                                   ~0x01U);
    const bool cleared = (state.entity_flags & 0x08U) == 0U;
    if (cleared) {
        call_once(clear_slot);
    }
    return cleared;
}

void initialize_effect_fade_marker(EffectFadeMarkerState& state) {
    state.marker = -1;
    state.countdown = 3;
}

void reset_world_effect_globals(WorldEffectGlobals& state) {
    state.selected_effect = -1;
    state.collision_flags = 0;
}

bool dispatch_world_effect(
    WorldEffectDispatchState& state,
    const std::array<std::function<void()>, 6>& handlers) {
    state.selected_effect = 0;
    if (state.input_effect == 0xFFU || (state.flags & 0x80U) != 0U ||
        state.input_effect >= handlers.size()) {
        return false;
    }
    call_once(handlers[state.input_effect]);
    return true;
}

std::uint8_t advance_eight_step_counter(const std::uint8_t value) {
    return value < 8U ? static_cast<std::uint8_t>(value + 1U) : 8U;
}

void mark_effect_bit(std::uint32_t& mask, const std::int32_t source_index) {
    const auto bit = static_cast<std::uint32_t>(source_index) & 0x1FU;
    mask |= static_cast<std::uint32_t>(1U << bit);
}

} // namespace shadowrun::ghidra_conversion
