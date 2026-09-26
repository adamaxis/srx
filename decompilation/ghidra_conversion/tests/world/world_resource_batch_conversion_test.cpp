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
    transfer_world_word_block([&] { ++calls; });
    std::int16_t countdown = 1;
    std::int16_t stage = 0;
    advance_world_sequence({{1}, {2}, {static_cast<std::int16_t>(0x8000)}, {3}}, countdown,
                            stage, [&](const WorldSequenceEntry&) { ++calls; });
    assert(calls == 3 && countdown == 0 && stage == 1);

    assert(expand_random_nibble_mask({{0, 0x80}}, 0, 0, {}) == 0xF0000000U);
    std::vector<EntityServiceRecord> service_records(10);
    service_records[0].flags = 0xFF;
    service_records[0].mode_flags = 0xFF;
    std::uint8_t global_state = 7;
    reset_entity_service_records(service_records, global_state);
    assert(global_state == 0 && service_records[0].work_a == 0 &&
           (service_records[0].flags & 0x40U) == 0U &&
           (service_records[0].mode_flags & 0x10U) == 0U);

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
    assert(clear_high_effect_slots(high, [&](std::size_t) { ++cleared; }) == 2);

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

    std::vector<std::uint16_t> rows(4);
    copy_two_word_rows({1, 2, 3, 4}, rows, 2, 2, 2);
    assert(rows == std::vector<std::uint16_t>({1, 2, 3, 4}));
    upload_decoded_source_stream([&] { ++calls; });
    assert(calls > 10);
    return 0;
}
