#include "shadowrun_ghidra/world/world_resource_batch_conversion.hpp"

#include <cassert>

using namespace shadowrun::ghidra_conversion;

int main() {
    WorldSequenceState sequence{};
    initialize_world_sequence(sequence);
    if (sequence.animation_phase != 0U || sequence.frame_countdown != 4U ||
        !sequence.active || sequence.source_start != 0x70U ||
        sequence.source_end != 0xAFU || sequence.source_cursor != 0xAFU ||
        sequence.sequence_table_token != 0x1D428U ||
        sequence.source_buffer_token != 0xDA038U ||
        sequence.destination_base != 0x811BU ||
        sequence.current_row != 0x51U || sequence.substage != 0U) {
        return 1;
    }

    int calls = 0;
    WorldSourceBlockTransferRequest transfer_request{
        0xDA038U, 0x0070U, 0x0100U};
    WorldSourceBlockTransferRequest observed_transfer{};
    transfer_world_word_block(
        transfer_request,
        [&](const WorldSourceBlockTransferRequest& request) {
            ++calls;
            observed_transfer = request;
        });
    assert(observed_transfer.source_address == 0xDA038U &&
           observed_transfer.destination_address == 0x0070U &&
           observed_transfer.byte_count == 0x0100U);
    sequence.current_row = 0x51U;
    sequence.source_end = 0xAFU;
    sequence.substage = 0;
    int rendered_entries = 0;
    assert(!advance_world_sequence(
        sequence,
        {{1}, {2}, {static_cast<std::int16_t>(0x8000)}, {3}},
        [&](const WorldSequenceEntry& entry) {
            ++calls;
            rendered_entries += entry.marker;
        }));
    assert(calls == 3 && rendered_entries == 3 && sequence.current_row == 0x52U &&
           sequence.substage == 0);
    sequence.substage = 1;
    sequence.current_row = 0xAFU;
    assert(!advance_world_sequence(sequence, {{1}, {static_cast<std::int16_t>(0x8000)}},
                                    {}));
    assert(sequence.substage == 0 && sequence.current_row == 0xAFU);
    sequence.substage = 0;
    assert(advance_world_sequence(sequence, {{1}, {static_cast<std::int16_t>(0x8000)}},
                                             {}));
    assert(sequence.substage == 0 && sequence.current_row == 0xB0U);

    const auto& mask_banks = source_random_nibble_mask_banks();
    assert(mask_banks.size() == 9U && mask_banks[0].size() == 1U &&
           mask_banks[1].size() == 8U && mask_banks[2].size() == 28U &&
           mask_banks[3].size() == 56U && mask_banks[4].size() == 70U &&
           mask_banks[5].size() == 56U && mask_banks[6].size() == 28U &&
           mask_banks[7].size() == 8U && mask_banks[8].size() == 1U);
    bool saw_bank_one_bound = false;
    assert(expand_random_nibble_mask(
               mask_banks, 1U, [&](const std::uint16_t bound) {
                   saw_bank_one_bound = bound == 8U;
                   return static_cast<std::uint16_t>(7U);
               }) == 0xF0000000U &&
           saw_bank_one_bound);
    assert(expand_random_nibble_mask(
               mask_banks, 0U, [&](const std::uint16_t bound) {
                   return static_cast<std::uint16_t>(bound == 1U ? 0U : 1U);
               }) == 0U);
    std::vector<EntityServiceRecord> service_records(10);
    service_records[0].route_flags = 0xFF;
    service_records[0].animation_flags = 0xFF;
    service_records[0].pair_a2 = 1U;
    service_records[9].route_flags = 0x40U;
    service_records[9].animation_flags = 0x10U;
    std::int16_t timeout_marker = -1;
    reset_entity_service_records(service_records, timeout_marker);
    assert(timeout_marker == 0 && service_records[0].pair_a2 == 0 &&
           service_records[0].byte_43 == 0 &&
           service_records[0].route_flags == 0xBFU &&
           service_records[0].animation_flags == 0xEFU &&
           service_records[9].route_flags == 0U &&
           service_records[9].animation_flags == 0U);

    std::vector<EntityServiceTargetRecord> cycle(10U);
    cycle[0].selected = true;
    cycle[3].active = true;
    cycle[3].has_action = true;
    EntityServiceTargetOwner cycle_owner{};
    assert(select_next_entity(cycle_owner, cycle) ==
           EntitySelectionResult::selected_target);
    assert(cycle_owner.selected_target == 3U && cycle[3].selected);
    std::vector<EntityServiceTargetRecord> unmarked(10U);
    unmarked[3].active = true;
    unmarked[3].has_action = true;
    assert(select_next_entity(cycle_owner, unmarked) ==
           EntitySelectionResult::no_marked_record);
    int cleared = 0;
    cycle[0].selected = false;
    cycle[0].effect_locked = true;
    cycle[0].has_action = false;
    cycle[1].effect_locked = true;
    cycle[1].effect_pending = true;
    cycle[2].effect_locked = true;
    cycle[2].has_action = true;
    assert(clear_inactive_entity_effects(cycle, [&](std::size_t) { ++cleared; }) == 1);
    assert(cleared == 1);

    std::vector<EffectHighFlagRecord> high{{0x80}, {0}, {0x80}};
    bool companion_restore = true;
    assert(clear_high_effect_slots(high, companion_restore,
                                   [&](std::size_t) { ++cleared; }) == 2 &&
           !companion_restore);

    std::vector<NearbyEffectRecord> nearby{{true, 100, 100, 1, 0x65, 1}};
    assert(resolve_nearby_effects(nearby, 100, 100,
                                  [](const NearbyEffectRecord& record) {
                                      return std::make_pair(static_cast<std::uint16_t>(record.x),
                                                            static_cast<std::uint16_t>(record.y));
                                  },
                                  [&] { ++calls; }, [&](std::size_t) { ++cleared; }));
    assert(!nearby[0].active);

    assert(spawn_world_effect_sequence(0, true, {4, 5, 0}, [&] { ++calls; }, {},
                                       [&](std::size_t, std::uint32_t) { return true; }) == 2);
    std::int32_t resource = 10;
    subtract_resource_clamped(4, resource);
    subtract_resource_clamped(20, resource);
    assert(resource == 6);
    run_fixed_delay(8, [&](std::size_t) { ++calls; });

    std::vector<std::uint8_t> state;
    assert(count_active_entities(state, [](std::size_t index) { return index != 1; }) == 2);
    assert((state == std::vector<std::uint8_t>{1, 0, 1}));
    service_random_roll([&] { ++calls; });
    assert(dispatch_indexed_handler(0, {[&] { ++calls; }}));
    assert(!dispatch_indexed_handler(-1, {[&] { ++calls; }}));

    GridClearState grid{5, {1, 2, 3, 4, 5, 6}};
    clear_grid_segments(grid, 2, 2, 2, 3);
    assert(grid.remaining == 3 && grid.bytes[0] == 0 && grid.bytes[1] == 0 && grid.bytes[3] == 0);

    std::vector<std::uint16_t> words;
    assert(write_sequential_asset_grid(false, 2, [&] { ++calls; },
                                       [&](std::uint16_t word) { words.push_back(word); }, 0x393) == 16);
    assert(words[0] == 0x393 && words.back() == 0x3A2);

    std::vector<std::uint32_t> destination(8, 0);
    copy_tile_blocks({1, 2, 3, 4}, destination, 2, 2, 2, 4);
    assert(destination[0] == 1 && destination[1] == 2 && destination[4] == 3);

    std::vector<WorldSpriteRecord> sprites;
    std::uint8_t attribute = 0;
    assert(append_world_sprite_records({{1, 2, 3, 4, 0}, {5, 6, 7, 8, 0}}, sprites, 10, 20,
                                       attribute) == 2);
    assert(sprites[1].x == 15 && sprites[1].attribute == 1);

    std::vector<std::uint16_t> mode_state;
    std::vector<std::uint16_t> mode_source(
        kFieldModeSelectionStateWordCount + 2U, 0U);
    mode_source.front() = 1U;
    mode_source[kFieldModeSelectionStateWordCount - 1U] = 23U;
    assert(copy_field_mode_selection_state(mode_source, mode_state));
    assert(mode_state.size() == kFieldModeSelectionStateWordCount &&
           mode_state.front() == 1U && mode_state.back() == 23U);
    assert(!copy_field_mode_selection_state({1, 2}, mode_state) &&
           mode_state.empty());
    upload_decoded_source_stream([&] { ++calls; });
    assert(calls > 10);
    return 0;
}
