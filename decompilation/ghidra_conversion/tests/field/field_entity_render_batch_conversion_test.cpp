#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    EntityAnchorSnapshotState snapshot{};
    int published = 0;
    snapshot_entity_anchors(
        snapshot, {{{1, 2}, {3, 4}, {5, 6}}}, EntityAnchor{7, 8},
        {1, 2, 3, 4, 5, 6, 7, 8}, [&](const EntityAnchor& anchor) {
            if (anchor.x == 7 && anchor.y == 8) {
                ++published;
            }
        });
    if (snapshot.anchors[2].y != 6 || published != 1 ||
        snapshot.pending_profile[5] != 6 || !snapshot.pending_profile_active) {
        return 1;
    }

    std::vector<std::uint16_t> words;
    const TileWordStreamCallbacks stream{
        {}, [](const std::uint16_t key, const std::size_t offset) {
            return static_cast<std::uint16_t>(key + offset);
        }, [&](const std::uint16_t word) { words.push_back(word); }};
    if (emit_field_tile_column_words(5, 2, 0, 0x8000, stream) != 5 ||
        emit_field_tile_row_words(3, 4, 0, 0, stream) != 3 || words.size() != 8) {
        return 2;
    }

    const auto wide_column = plan_field_tile_column(0);
    const auto wrap_column = plan_field_tile_column(0x0070U);
    if (wide_column.first_word_count != 0x002CU ||
        wide_column.second_word_count != 0U ||
        wrap_column.first_word_count != 8U ||
        wrap_column.second_word_count != 0x0024U ||
        wrap_column.wrapped_destination != 0U ||
        wrap_column.first_word_count + wrap_column.second_word_count !=
            0x002CU) {
        return 22;
    }
    const auto wide_row = plan_field_tile_row(0);
    const auto wrap_row = plan_field_tile_row(0x0F80U);
    if (wide_row.first_word_count != 0x0020U ||
        wide_row.second_word_count != 0U || wrap_row.first_word_count != 1U ||
        wrap_row.second_word_count != 0x001FU ||
        wrap_row.wrapped_destination != 0U) {
        return 23;
    }

    std::vector<std::uint16_t> uploaded;
    std::vector<std::uint16_t> table_reads;
    FieldTileRowUploadState upload{};
    upload.remaining = 4;
    upload.palette_word = -1;
    upload.row_stride_bytes = 0x0040U;
    const auto uploaded_count = upload_field_tile_row(
        upload,
        {[] {},
         [] {},
         [&table_reads](const std::uint16_t table_offset) {
             table_reads.push_back(table_offset);
             return static_cast<std::uint16_t>(0x10U + table_offset);
         },
         [](const std::uint16_t tile_key, const std::uint16_t word_offset) {
             return static_cast<std::uint16_t>(tile_key + word_offset);
         },
         [&uploaded](const std::uint16_t word) { uploaded.push_back(word); }});
    if (uploaded_count != 5U || uploaded.size() != 5U ||
        table_reads.size() != 2U || table_reads[0] != 0U ||
        table_reads[1] != 0x0040U || uploaded[0] != (0x10U ^ 0x8000U) ||
        uploaded[4] != (0x0050U ^ 0x8000U) || upload.word_offset != 8U ||
        upload.remaining != -1) {
        return 24;
    }

    FieldTilePlaneFillState plane{};
    std::vector<std::uint16_t> plane_dest;
    std::vector<std::uint16_t> plane_y;
    int prepare = 0;
    int compose = 0;
    const auto plane_rows = render_field_tile_plane_rows(
        plane,
        {[&] { ++prepare; },
         [&] {
             ++compose;
             return static_cast<std::uint16_t>(0x0F80U);
         },
         [&](const std::uint16_t destination, const std::uint16_t y) {
             plane_dest.push_back(destination);
             plane_y.push_back(y);
         }});
    if (plane_rows != 31U || prepare != 1 || compose != 1 ||
        plane_dest.size() != 31U || plane_dest.front() != 0x0F80U ||
        plane_dest[1] != 0U || plane.destination != 0x0F00U ||
        plane.y != 0x00F8U || plane_y.front() != 0U ||
        plane_y.back() != 0x00F0U) {
        return 25;
    }
    FieldTilePlaneFillState high_bank{};
    high_bank.destination = 0xFF80U;
    std::vector<std::uint16_t> high_dest;
    render_field_tile_plane_rows(
        high_bank,
        {{},
         {},
         [&](const std::uint16_t destination, const std::uint16_t) {
             high_dest.push_back(destination);
         }});
    if (high_dest.front() != 0xFF80U || high_dest[1] != 0xF000U) {
        return 26;
    }

    FieldTileRowEdgeState edge_state{};
    edge_state.horizontal_direction = -1;
    edge_state.camera_x = 0x0200U;
    edge_state.camera_x_limit = 50;
    edge_state.camera_y = 0x0100U;
    edge_state.field_y = 0x0080U;
    edge_state.base_destination = 0x0200U;
    edge_state.plane_a_offset = 0x0010U;
    edge_state.plane_b_offset = 0x0020U;
    edge_state.row_stride_bytes = 0x0040U;
    std::vector<FieldTileRowEdgeRequest> edge_requests;
    const auto edge_result = render_field_tile_row_edge(
        edge_state,
        {[&edge_requests](const FieldTileRowEdgeRequest& request) {
            edge_requests.push_back(request);
        }});
    if (edge_result.action != FieldTileRowEdgeAction::rows_rendered ||
        edge_result.rows_rendered != 2U || edge_requests.size() != 2U ||
        edge_requests[0].palette_or_flip != 0U ||
        edge_requests[1].palette_or_flip != 0x8000U ||
        edge_requests[0].x_coordinate != 40 ||
        edge_requests[0].row_coordinate != 0 ||
        edge_requests[0].source_layout.row_table_byte_offset != 2U ||
        edge_requests[0].source_layout.tile_word_byte_offset != 2U) {
        return 3;
    }
    edge_state.horizontal_direction = 1;
    edge_state.camera_x = 0x0070U;
    const auto outside_result =
        render_field_tile_row_edge(edge_state, {});
    if (outside_result.action !=
        FieldTileRowEdgeAction::outside_visible_window) {
        return 4;
    }

    FieldTileColumnEdgeState column_state{};
    column_state.vertical_direction = -1;
    column_state.field_y = 0x0100U;
    column_state.field_y_limit = 0x0040;
    column_state.camera_x = 0x0070U;
    column_state.camera_y = 0x0100U;
    column_state.vdp_data_word = 0x0200U;
    column_state.plane_a_offset = 0x0010U;
    column_state.plane_b_offset = 0x0020U;
    column_state.row_stride_bytes = 0x0040U;
    std::vector<FieldTileColumnEdgeRequest> column_requests;
    const auto column_result = render_field_tile_column_edge(
        column_state,
        {[&column_requests](const FieldTileColumnEdgeRequest& request) {
            column_requests.push_back(request);
        }});
    if (column_result.action != FieldTileColumnEdgeAction::columns_rendered ||
        column_result.columns_rendered != 2U || column_requests.size() != 2U ||
        column_requests[0].palette_or_flip != 0U ||
        column_requests[1].palette_or_flip != 0x8000U ||
        column_requests[0].y_coordinate != 8 ||
        column_requests[0].x_coordinate != 7) {
        return 17;
    }
    column_state.vertical_direction = 1;
    column_state.field_y = 0x0070U;
    column_state.field_y_limit = 8;
    const auto column_outside =
        render_field_tile_column_edge(column_state, {});
    if (column_outside.action !=
        FieldTileColumnEdgeAction::outside_visible_window) {
        return 18;
    }

    std::array<RenderEntityState, 16> entities{};
    entities[2].active = true;
    int pass_events = 0;
    update_entity_render_pass(
        entities,
        {[&](RenderEntityState&) { ++pass_events; },
         [&](RenderEntityState&) { ++pass_events; },
         [&](RenderEntityState&) { ++pass_events; },
         [&] { ++pass_events; }});
    if (pass_events != 4) {
        return 5;
    }

    EffectRecordState effect{9, 0xFF, {1, 2, 3, 4, 5}};
    int clears = 0;
    reset_effect_record(effect, [&] { ++clears; });
    if (effect.coordinate != 0 || effect.tile_bytes[4] != 0 || clears != 1) {
        return 6;
    }
    EntitySpawnRecord source{1, 2, 3, 4, 5};
    EntitySpawnRecord destination{};
    clone_entity_spawn_variant_a(destination, source);
    if (destination.mode != 5 || destination.variant != 0x0D || destination.x != 3) {
        return 7;
    }
    clone_entity_spawn_variant_b(destination, source);
    if (destination.variant != 6) {
        return 8;
    }

    TitleSpriteTimerState timer{0, 3};
    int timer_builds = 0;
    advance_title_sprite_timer(timer, [&](const std::uint8_t phase) {
        if (phase == 0) {
            ++timer_builds;
        }
    });
    if (timer.countdown != 3 || timer.phase != 0 || timer_builds != 1) {
        return 9;
    }
    int burst = 0;
    emit_title_sprite_burst(4, 8, [&] { ++burst; });
    if (burst != 16) {
        return 10;
    }

    SpriteExpansionState sprite{0x80, 2, 0, 0, false};
    int decoded = 0;
    int expanded = 0;
    int appended = 0;
    const SpriteExpansionCallbacks sprite_callbacks{
        [&](SpriteExpansionState&) { ++decoded; },
        [&](SpriteExpansionState&) { ++expanded; },
        [&](SpriteExpansionState&) { ++appended; }};
    if (!append_entity_render_sprite(sprite, sprite_callbacks) ||
        decoded != 1 || expanded != 0 || appended != 1 || !sprite.appended) {
        return 11;
    }
    sprite = SpriteExpansionState{0x20, 2, 0, 0, false};
    decoded = expanded = appended = 0;
    if (!append_entity_render_sprite(sprite, sprite_callbacks) ||
        decoded != 0 || expanded != 1 || appended != 1) {
        return 27;
    }
    sprite = SpriteExpansionState{0xA0, 2, 0, 0, false};
    decoded = expanded = appended = 0;
    if (!append_entity_render_sprite(sprite, sprite_callbacks) ||
        decoded != 1 || expanded != 1 || appended != 1) {
        return 28;
    }
    sprite = SpriteExpansionState{0x80, 0, 0, 0, false};
    decoded = expanded = appended = 0;
    if (append_entity_render_sprite(sprite, sprite_callbacks) ||
        decoded != 0 || expanded != 0 || appended != 0 || sprite.appended) {
        return 29;
    }

    EntityRuntimeState runtime{};
    int initialized = 0;
    initialize_entity_runtime(runtime, [&] { ++initialized; });
    if (!runtime.entity_table_ready || runtime.sprite_count != 0 || initialized != 1) {
        return 12;
    }
    int actions = 0;
    if (!dispatch_player_action_if_ready(0, 2, -1, [&] { ++actions; }) ||
        actions != 1 || dispatch_player_action_if_ready(0, 2, 0, {})) {
        return 13;
    }
    std::uint8_t flags = 0x28;
    int world_effects = 0;
    if (!clear_entity_effect_if_forced(flags, -1, [&] { ++world_effects; }) ||
        (flags & 0x20U) != 0 || world_effects != 1) {
        return 14;
    }

    TargetHeadingState heading{0, true, 0, false};
    int heading_events = 0;
    update_entity_target_heading(
        heading,
        {[&] { return static_cast<std::uint16_t>(12); }, [] { return false; },
         [&] { ++heading_events; }, [&] { ++heading_events; }});
    if (heading.target_heading != 12 || heading_events != 1) {
        return 15;
    }

    std::vector<std::uint16_t> glyphs;
    if (render_encoded_title_text({0x20, 0x3D, 0x80}, [&](const std::uint16_t glyph) {
            glyphs.push_back(glyph);
        }) != 2 || glyphs[0] != 0 || glyphs[1] != 0x1D) {
        return 16;
    }
    MenuValueState menu{};
    select_menu_value(menu, {4, 8, 10, 12}, 1, true, 99);
    if (menu.primary != 12 || menu.selected != 12) {
        return 17;
    }

    std::vector<int> events;
    run_menu_text_display_sequence(
        2,
        {[&] { events.push_back(1); }, [&] { events.push_back(2); },
         [&] { events.push_back(3); }, [&] { events.push_back(4); },
         [&](const std::uint32_t offset) { events.push_back(static_cast<int>(offset)); }});
    if (events != std::vector<int>{1, 2, 3, 3, 3, 3, 0x60, 4}) {
        return 18;
    }
    std::vector<std::size_t> refreshed;
    std::vector<std::uint8_t> animation_states{1, 0, 1};
    refresh_ready_entity_animations(animation_states, [&](const std::size_t index) {
        refreshed.push_back(index);
    });
    if (refreshed != std::vector<std::size_t>{0, 2}) {
        return 19;
    }
    int blocks = 0;
    std::uint8_t marker = 3;
    write_repeated_asset_batch(3, 2, [&] { ++blocks; }, marker);
    if (blocks != 6 || marker != 0) {
        return 20;
    }
    int triples = 0;
    if (transfer_menu_glyph_triple(
            [&]() -> std::optional<std::array<std::uint16_t, 3>> {
                if (triples++ == 0) {
                    return std::array<std::uint16_t, 3>{1, 2, 3};
                }
                return std::nullopt;
            },
            0x8000, [&](const std::uint16_t word) { words.push_back(word); }) != 3) {
        return 21;
    }
    return 0;
}
