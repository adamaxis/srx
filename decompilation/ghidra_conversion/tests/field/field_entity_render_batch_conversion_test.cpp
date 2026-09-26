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
        return 3;
    }

    EffectRecordState effect{9, 0xFF, {1, 2, 3, 4, 5}};
    int clears = 0;
    reset_effect_record(effect, [&] { ++clears; });
    if (effect.coordinate != 0 || effect.tile_bytes[4] != 0 || clears != 1) {
        return 4;
    }
    EntitySpawnRecord source{1, 2, 3, 4, 5};
    EntitySpawnRecord destination{};
    clone_entity_spawn_variant_a(destination, source);
    if (destination.mode != 5 || destination.variant != 0x0D || destination.x != 3) {
        return 5;
    }
    clone_entity_spawn_variant_b(destination, source);
    if (destination.variant != 6) {
        return 6;
    }

    TitleSpriteTimerState timer{0, 3};
    int timer_builds = 0;
    advance_title_sprite_timer(timer, [&](const std::uint8_t phase) {
        if (phase == 0) {
            ++timer_builds;
        }
    });
    if (timer.countdown != 3 || timer.phase != 0 || timer_builds != 1) {
        return 7;
    }
    int burst = 0;
    emit_title_sprite_burst(4, 8, [&] { ++burst; });
    if (burst != 16) {
        return 8;
    }

    SpriteExpansionState sprite{0x80, 2, 0, 0, false};
    int expanded = 0;
    int appended = 0;
    if (!append_entity_render_sprite(
            sprite,
            {[&](SpriteExpansionState&) { ++expanded; },
             [&](SpriteExpansionState&) { ++appended; }}) ||
        expanded != 1 || appended != 1 || !sprite.appended) {
        return 9;
    }

    EntityRuntimeState runtime{};
    int initialized = 0;
    initialize_entity_runtime(runtime, [&] { ++initialized; });
    if (!runtime.entity_table_ready || runtime.sprite_count != 0 || initialized != 1) {
        return 10;
    }
    int actions = 0;
    if (!dispatch_player_action_if_ready(0, 2, -1, [&] { ++actions; }) ||
        actions != 1 || dispatch_player_action_if_ready(0, 2, 0, {})) {
        return 11;
    }
    std::uint8_t flags = 0x28;
    int world_effects = 0;
    if (!clear_entity_effect_if_forced(flags, -1, [&] { ++world_effects; }) ||
        (flags & 0x20U) != 0 || world_effects != 1) {
        return 12;
    }

    TargetHeadingState heading{0, true, 0, false};
    int heading_events = 0;
    update_entity_target_heading(
        heading,
        {[&] { return static_cast<std::uint16_t>(12); }, [] { return false; },
         [&] { ++heading_events; }, [&] { ++heading_events; }});
    if (heading.target_heading != 12 || heading_events != 1) {
        return 13;
    }

    std::vector<std::uint16_t> glyphs;
    if (render_encoded_title_text({0x20, 0x3D, 0x80}, [&](const std::uint16_t glyph) {
            glyphs.push_back(glyph);
        }) != 2 || glyphs[0] != 0 || glyphs[1] != 0x1D) {
        return 14;
    }
    MenuValueState menu{};
    select_menu_value(menu, {4, 8, 10, 12}, 1, true, 99);
    if (menu.primary != 12 || menu.selected != 12) {
        return 15;
    }

    std::vector<int> events;
    run_menu_text_display_sequence(
        2,
        {[&] { events.push_back(1); }, [&] { events.push_back(2); },
         [&] { events.push_back(3); }, [&] { events.push_back(4); },
         [&](const std::uint32_t offset) { events.push_back(static_cast<int>(offset)); }});
    if (events != std::vector<int>{1, 2, 3, 3, 3, 3, 0x60, 4}) {
        return 16;
    }
    std::vector<std::size_t> refreshed;
    std::vector<std::uint8_t> animation_states{1, 0, 1};
    refresh_ready_entity_animations(animation_states, [&](const std::size_t index) {
        refreshed.push_back(index);
    });
    if (refreshed != std::vector<std::size_t>{0, 2}) {
        return 17;
    }
    int blocks = 0;
    std::uint8_t marker = 3;
    write_repeated_asset_batch(3, 2, [&] { ++blocks; }, marker);
    if (blocks != 6 || marker != 0) {
        return 18;
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
        return 19;
    }
    return 0;
}
