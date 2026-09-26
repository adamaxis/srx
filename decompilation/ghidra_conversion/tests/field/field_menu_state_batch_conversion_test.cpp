#include "shadowrun_ghidra/field/field_menu_state_batch_conversion.hpp"

#include <cassert>
#include <array>
#include <tuple>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<std::uint16_t> glyphs;
    std::vector<std::tuple<std::size_t, std::size_t, std::uint16_t>> placements;
    int begins = 0;
    assert(render_wrapped_text(
               {0x21, 0x22, 0x20, 0x23, 0xFF}, 4,
               {[&] { ++begins; },
                [](const std::uint8_t value) {
                    return static_cast<std::uint16_t>(value + 0x100U);
                },
                [&](const std::uint16_t glyph) { glyphs.push_back(glyph); },
                [&](const std::size_t line, const std::size_t column,
                    const std::uint16_t glyph) {
                    placements.emplace_back(line, column, glyph);
                }}) == 4U);
    const std::vector<std::tuple<std::size_t, std::size_t, std::uint16_t>>
        expected_wrapped_placements{
            {0, 0, 0x121}, {0, 1, 0x122}, {0, 2, 0x120}, {1, 0, 0x123}};
    assert(begins == 2 && glyphs.empty() &&
           placements == expected_wrapped_placements);

    begins = 0;
    placements.clear();
    assert(render_wrapped_text(
               {0x21, 0x80, 0x22, 0xFF}, 4,
               {[&] { ++begins; }, {}, {},
                [&](const std::size_t line, const std::size_t column,
                    const std::uint16_t glyph) {
                    placements.emplace_back(line, column, glyph);
                }}) == 2U);
    const std::vector<std::tuple<std::size_t, std::size_t, std::uint16_t>>
        expected_break_placements{{0, 0, 0x21}, {1, 0, 0x22}};
    assert(begins == 2 && placements == expected_break_placements);

    int dispatched = 0;
    assert(dispatch_menu_handler(1, {{}, [&] { ++dispatched; }}) && dispatched == 1);
    assert(!dispatch_menu_handler(2, {{}, [&] { ++dispatched; }}));

    int sampled = 0;
    assert(pass_entity_action_threshold_variant(0, 4, 3, [&] { ++sampled; }));
    assert(!pass_entity_action_threshold_variant(0, -1, 3, [&] { ++sampled; }));
    assert(sampled == 2);

    EffectServiceState effects{0, false, {true, false, true, true}};
    std::vector<std::size_t> serviced;
    assert(service_active_effect_slots(effects, [&](const std::size_t slot) {
               serviced.push_back(slot);
           }) == 3U);
    assert((serviced == std::vector<std::size_t>{0, 2, 3}));

    std::vector<EntityTableRecord> records(2);
    int next_slot = 0;
    assert(populate_entity_table({{4, 8}, {7, 9}, {0, 10}}, records, [&]() {
               if (next_slot == 2) {
                   return std::optional<std::size_t>{};
               }
               return std::optional<std::size_t>(next_slot++);
           }) == 2U);
    assert(records[0].status == -1 && records[0].behavior_token == 0x1641A &&
           records[1].value == 9);

    std::vector<std::uint16_t> terrain{4, 11};
    std::uint8_t marked = 0;
    bool dirty = false;
    assert(mark_projected_terrain_cell(terrain, [] { return std::optional<std::size_t>(0); },
                                       marked, dirty));
    assert(terrain[0] == 11 && marked == 1 && dirty);

    std::vector<std::uint32_t> source{7, 8, 0};
    std::vector<WorldActivityDescriptor> published;
    int scheduled = 0;
    bool active = false;
    assert(publish_world_activity_descriptors(source, published, active,
                                              [&] { ++scheduled; }) == 2U);
    assert(active && scheduled == 1 && source.empty() && published[1].descriptor_offset == 0x16A70);

    int boundaries = 0;
    bool copied = false;
    assert(process_terrain_work_batch(
               {{1, 2, false}, {2, 3, true}, {0, 0, false}}, {true, true},
               {[&](const TerrainWorkEntry&) { ++boundaries; },
                [&] {
                    copied = true;
                    return true;
                }},
               dirty));
    assert(boundaries == 2 && copied && dirty);

    std::vector<int> events;
    int grids = 0;
    refresh_field_hud([&] { events.push_back(4); }, [&] { ++grids; });
    assert(grids == 5);

    std::vector<std::uint8_t> mode_values{0x12};
    refresh_mode_glyph(mode_values, 0, {5, 6}, [&] { ++grids; });
    assert(mode_values[0] == 6);
    assert(select_hud_mode(1, 7, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}) ==
           std::optional<std::uint8_t>(15));
    assert(pass_through_value(9) == 9);

    MenuServiceCallbacks callbacks{
        [&] { events.push_back(5); }, [&] { events.push_back(6); },
        [&] { events.push_back(7); }, [&] { events.push_back(8); },
        [&] { events.push_back(9); }, [&] { events.push_back(10); },
        [&] { events.push_back(11); }};
    MenuPendingFlagState pending_state{};
    pending_state.pending_flags = {1, 0, 0, 0};
    pending_state.selected_mode_index = 2U;
    pending_state.runner_records.resize(0x14U * 3U, 0U);
    pending_state.selected_runner_index = 1U;
    pending_state.runner_records[0x14U + 0x05U] = 3U;
    std::uint8_t pending = 1U;
    run_menu_slot_service(pending, 1, callbacks);
    std::size_t metadata_offset = 0U;
    const auto pending_merge = merge_menu_pending_flags(
        pending_state, [&](const std::size_t offset) {
            metadata_offset = offset;
            events.push_back(12);
        });
    run_menu_metadata_service(callbacks);
    run_menu_selection_service(1, callbacks);
    run_menu_selection_variant(1, callbacks);
    run_menu_progression_variant(1, callbacks);
    assert(pending_merge.has_value() && pending_merge->source_flags == 3U &&
           pending_merge->metadata_offset == 0x60U && metadata_offset == 0x60U &&
           pending_state.pending_flags[2] == 3U);

    std::vector<std::size_t> cleared;
    assert(clear_first_ready_menu_slot({1, 7, 2, 7}, [&](const std::size_t slot) {
               cleared.push_back(slot);
           },
           [&](const std::size_t slot) { cleared.push_back(slot + 10); },
           [&](const std::size_t slot) { cleared.push_back(slot + 20); }) ==
           std::optional<std::size_t>(3));
    assert((cleared == std::vector<std::size_t>{3, 13, 23}));
    return 0;
}
