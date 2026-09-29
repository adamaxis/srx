#include "shadowrun_ghidra/field/field_menu_state_batch_conversion.hpp"

#include <array>
#include <cassert>
#include <tuple>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<std::uint16_t> glyphs;
    std::vector<std::tuple<std::size_t, std::size_t, std::uint16_t>> placements;
    int begins = 0;
    assert(render_wrapped_encoded_text(
               {0x21, 0x22, 0x20, 0x23, 0xFF}, 4,
               {[&] { ++begins; },
                [](const std::uint16_t index) {
                    return static_cast<std::uint16_t>(index + 0x100U);
                },
                [&](const std::uint16_t glyph) { glyphs.push_back(glyph); },
                [&](const std::size_t line, const std::size_t column,
                    const std::uint16_t glyph) {
                    placements.emplace_back(line, column, glyph);
                }}) == 4U);
    const std::vector<std::tuple<std::size_t, std::size_t, std::uint16_t>>
        expected_source_index_placements{
            {0, 0, 0x101}, {0, 1, 0x102}, {0, 2, 0x100}, {1, 0, 0x103}};
    assert(begins == 2 && glyphs.empty() &&
           placements == expected_source_index_placements);

    assert(wrapped_encoded_text_glyph_index(0x20) ==
           std::optional<std::uint16_t>(0));
    assert(wrapped_encoded_text_glyph_index(0x5C) ==
           std::optional<std::uint16_t>(0x3C));
    assert(wrapped_encoded_text_glyph_index(0x5D) ==
           std::optional<std::uint16_t>(0x39));
    assert(!wrapped_encoded_text_glyph_index(0x80).has_value());

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
        expected_break_placements{{0, 0, 0x1}, {1, 0, 0x2}};
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
    HudOffsetState footer{-3};
    std::vector<std::uint16_t> destinations;
    std::vector<std::uint16_t> hud_words;
    WordTransferCallbacks hud_callbacks{};
    hud_callbacks.begin_transfer = [&](const VdpTransferCommand& command) {
        destinations.push_back(command.low_word);
    };
    hud_callbacks.write_word = [&](const std::uint16_t word) {
        hud_words.push_back(word);
    };
    refresh_field_hud(footer, {0, 1, 2, 3, 6}, hud_callbacks);
    assert(footer.offset == 0);
    assert(destinations.size() == 5U);
    assert(destinations[0] == make_vdp_transfer_command(0xED14U).low_word);
    assert(destinations[4] == make_vdp_transfer_command(0xED44U).low_word);
    assert(hud_words.size() == 15U);
    assert((hud_words == std::vector<std::uint16_t>{
               0x60B4U, 0x60B4U, 0x60B4U, 0x60B5U, 0x60B4U, 0x60B4U, 0x60B2U,
               0x60B4U, 0x60B4U, 0x60B2U, 0x60B5U, 0x60B4U, 0x60B2U, 0x60B2U,
               0x60B2U}));

    std::vector<std::uint8_t> mode_values{0x12};
    refresh_mode_glyph(mode_values, 0, {5, 6}, [&] { ++grids; });
    assert(mode_values[0] == 6);
    std::vector<std::uint8_t> glyph_records(0x14, 0);
    glyph_records[0x13] = 0x13;
    std::uint16_t glyph_destination = 0;
    std::vector<std::uint16_t> glyph_words;
    WordTransferCallbacks glyph_callbacks{};
    glyph_callbacks.begin_transfer = [&](const VdpTransferCommand& command) {
        glyph_destination = command.low_word;
    };
    glyph_callbacks.write_word = [&](const std::uint16_t word) {
        glyph_words.push_back(word);
    };
    refresh_mode_glyph(glyph_records, 0, glyph_callbacks);
    assert(glyph_records[0x13] == 1);
    assert(glyph_destination == make_vdp_transfer_command(0xC22EU).low_word);
    assert(glyph_words.size() == 14U);
    assert(glyph_words.front() == 0x85ABU);
    assert(glyph_words.back() == 0x85ABU);
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

    std::vector<int> exact_events;
    std::uint8_t exact_code = 0;
    std::uint8_t exact_value = 0;
    MenuSlotServiceExactCallbacks exact_slot_callbacks{};
    exact_slot_callbacks.set_hud_selection = [&](const std::uint8_t code,
                                                  const std::uint8_t value) {
        exact_code = code;
        exact_value = value;
        exact_events.push_back(1);
    };
    exact_slot_callbacks.render_menu_text_followup = [&] { exact_events.push_back(2); };
    exact_slot_callbacks.refresh_hud_grid_if_ready = [&] { exact_events.push_back(3); };
    exact_slot_callbacks.prepare_menu_offset = [&] { exact_events.push_back(4); };
    exact_slot_callbacks.merge_menu_pending_flags = [&] { exact_events.push_back(5); };
    exact_slot_callbacks.render_menu_text_progression = [&] { exact_events.push_back(6); };
    exact_slot_callbacks.render_menu_slot_status = [&] { exact_events.push_back(7); };
    MenuSlotServiceState empty_slot{-1, false};
    assert(run_menu_slot_service(empty_slot, exact_slot_callbacks) ==
           MenuSlotServicePath::cleared_empty_slot);
    assert(empty_slot.pending_marker == 0 && empty_slot.pending_marker_cleared &&
           exact_code == 0x0B && exact_value == 0x0A &&
           (exact_events == std::vector<int>{1, 2, 3, 4}));
    exact_events.clear();
    MenuSlotServiceState pending_slot{2, false};
    assert(run_menu_slot_service(pending_slot, exact_slot_callbacks) ==
           MenuSlotServicePath::pending_slot);
    assert(!pending_slot.pending_marker_cleared && exact_code == 0x0B &&
           exact_value == 6 &&
           (exact_events == std::vector<int>{1, 2, 5, 6, 7}));

    MenuMetadataServiceState metadata_state{};
    bool metadata_flags_read = false;
    std::size_t exact_metadata_offset = 99U;
    assert(run_menu_metadata_service(
        metadata_state,
        {[&] { return std::optional<std::uint16_t>(0); },
         [&] {
             metadata_flags_read = true;
             return std::optional<std::uint8_t>(3);
         },
         [&](const std::size_t offset) { exact_metadata_offset = offset; }}) &&
           !metadata_flags_read && metadata_state.metadata_offset ==
               std::optional<std::size_t>(0) && exact_metadata_offset == 0U);
    assert(run_menu_metadata_service(
        metadata_state,
        {[] { return std::optional<std::uint16_t>(1); },
         [] { return std::optional<std::uint8_t>(3); },
         [&](const std::size_t offset) { exact_metadata_offset = offset; }}) &&
           metadata_state.eligibility_value == 1U &&
           metadata_state.selected_runner_flags == 3U &&
           metadata_state.metadata_offset == std::optional<std::size_t>(0x60U) &&
           exact_metadata_offset == 0x60U);

    exact_events.clear();
    MenuSelectionServiceExactCallbacks selection_callbacks{};
    selection_callbacks.set_hud_selection = [&](const std::uint8_t code,
                                                const std::uint8_t value) {
        exact_code = code;
        exact_value = value;
        exact_events.push_back(1);
    };
    selection_callbacks.render_menu_text_followup = [&] { exact_events.push_back(2); };
    selection_callbacks.prepare_menu_offset = [&] { exact_events.push_back(3); };
    selection_callbacks.advance_menu_selection_service_timer =
        [&](const std::uint8_t delta) {
            assert(delta == 2U);
            exact_events.push_back(4);
        };
    selection_callbacks.refresh_hud_grid_if_ready = [&] { exact_events.push_back(5); };
    run_menu_selection_service(MenuSelectionServiceState{-1}, selection_callbacks);
    assert(exact_code == 0U && exact_value == 7U &&
           (exact_events == std::vector<int>{1, 2, 3}));
    exact_events.clear();
    run_menu_selection_service(MenuSelectionServiceState{2}, selection_callbacks);
    assert(exact_code == 0U && exact_value == 9U &&
           (exact_events == std::vector<int>{1, 2, 3, 4, 5, 3}));

    exact_events.clear();
    std::uint8_t variant_delta = 0;
    MenuSelectionVariantExactCallbacks variant_callbacks{};
    variant_callbacks.set_hud_selection = [&](const std::uint8_t code,
                                               const std::uint8_t value) {
        exact_code = code;
        exact_value = value;
        exact_events.push_back(1);
    };
    variant_callbacks.render_menu_text_followup = [&] { exact_events.push_back(2); };
    variant_callbacks.advance_menu_selection_variant_timer =
        [&](const std::uint8_t delta) {
            variant_delta = delta;
            exact_events.push_back(3);
        };
    variant_callbacks.prepare_menu_offset = [&] { exact_events.push_back(4); };
    MenuSelectionVariantState variant_state{1, 0x0F, 0};
    run_menu_selection_variant(variant_state, variant_callbacks);
    assert(exact_code == 1U && exact_value == 9U &&
           variant_state.computed_delta == 9U && variant_delta == 9U &&
           (exact_events == std::vector<int>{1, 2, 3, 4}));

    exact_events.clear();
    MenuProgressionVariantExactCallbacks progression_callbacks{};
    progression_callbacks.set_hud_selection = [&](const std::uint8_t code,
                                                   const std::uint8_t value) {
        exact_code = code;
        exact_value = value;
        exact_events.push_back(1);
    };
    progression_callbacks.render_menu_text_followup = [&] { exact_events.push_back(2); };
    progression_callbacks.reduce_menu_progression_timing =
        [&](const std::uint16_t delta) {
            assert(delta == 3U);
            exact_events.push_back(3);
        };
    progression_callbacks.render_menu_text_progression = [&] { exact_events.push_back(4); };
    progression_callbacks.prepare_menu_offset = [&] { exact_events.push_back(5); };
    run_menu_progression_variant(MenuProgressionVariantState{3},
                                 progression_callbacks);
    assert(exact_code == 2U && exact_value == 9U &&
           (exact_events == std::vector<int>{1, 2, 3, 4, 5}));

    ReadyMenuSlotState ready_slots{};
    ready_slots.slot_codes = {1, 7, 2, 7, 3};
    ready_slots.active_values = {4, 4, 4, 4, 4};
    ready_slots.row_status = {1, 2, 3, 4, 5};
    exact_events.clear();
    const auto cleared_ready_slot = clear_first_ready_menu_slot(
        ready_slots,
        {[] { return std::optional<std::uint16_t>(6U); },
         [&] { exact_events.push_back(1); },
         [&](const std::size_t slot) {
             assert(slot == 3U);
             exact_events.push_back(2);
         }});
    assert(cleared_ready_slot == std::optional<std::size_t>(3U) &&
           ready_slots.active_values[3] == 0U &&
           ready_slots.load_offsets[3] == 60U && ready_slots.row_status[3] == 0U &&
           (exact_events == std::vector<int>{1, 2}));

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
