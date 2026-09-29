#include "shadowrun_ghidra/world/world_entity_conversion.hpp"

#include <array>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    WorldFrameState world{true, true, true, true, false, true, true, 2};
    std::vector<int> events;
    const WorldFrameCallbacks callbacks{
        [&] { events.push_back(1); }, [&] { events.push_back(2); },
        [&] { events.push_back(3); }, [&] { events.push_back(4); },
        [&] { events.push_back(5); }, [&] { events.push_back(6); },
        [&] { events.push_back(7); }, [&] { events.push_back(8); },
        [&] { events.push_back(9); }, [&] { events.push_back(10); },
        [&] { events.push_back(11); }, [&] { events.push_back(12); },
        [&] { events.push_back(13); }, [&] { events.push_back(14); },
        [&] { events.push_back(15); }, [&] { events.push_back(16); },
        [&] { events.push_back(17); }};
    run_world_frame_services(world, callbacks);
    if (world.audio_pending || world.world_dirty || world.effect_transition_pending ||
        world.compressed_entity_data_pending || events.size() != 17 ||
        events[0] != 1 || events[1] != 2 || events.back() != 17) {
        return 1;
    }

    std::array<EntityFrameRecord, 16> entities{};
    entities[0] = {true, 1, 0xF0, 1, 2, 0, false};
    int expanded = 0;
    update_entity_frame(entities, {
        {}, {}, {}, [&](EntityFrameRecord&) {}, {}, {}, {}, {}, {},
        [&](EntityFrameRecord&) { ++expanded; }, {}, {}, {}});
    if (expanded != 1) {
        return 2;
    }

    EffectAnimationState animation{0, 0, 1, 0, true};
    int builds = 0;
    for (int i = 0; i < 8; ++i) {
        advance_effect_animation(animation, {}, {}, [&] { ++builds; });
    }
    if (animation.active || builds == 0) {
        return 3;
    }

    JitterEffectState jitter{};
    jitter.animation = {0, 1, 3, 0, true};
    jitter.remaining = 2;
    int jitter_builds = 0;
    advance_jitter_effect(jitter, [](std::uint32_t& state) {
        state = 0x12345678U;
        return state;
    }, {}, [&] { ++jitter_builds; });
    if (jitter_builds != 1) {
        return 4;
    }
    if (!set_sprite_descriptor_carry()) {
        return 5;
    }

    if (select_projected_entity_sprite_source(0) != 0x00005F78U ||
        select_projected_entity_sprite_source(12) != 0x000095A4U ||
        select_projected_entity_sprite_source(0x64) != 0x00005FAAU ||
        select_projected_entity_sprite_source(0x65) != 0x0001D47CU ||
        select_projected_entity_sprite_source(-0x4000) != 0x00005F82U ||
        select_projected_entity_sprite_source(-1) != 0x00005FA0U) {
        return 6;
    }

    std::array<ProjectedEntitySpriteSlot, 10> slots{};
    slots[0] = {0x0100U, 0, 0x0100U, 0x0010U, 0, 0, 1};
    slots[1] = {0xFF00U, 0, 0x0100U, 0x0010U, 0, 0, 1};
    slots[2] = {0x0100U, 0, 0x0100U, 0x0010U, 0, 0, 0};
    slots[3] = {0x0100U, 12, 0x0100U, 0x0010U, 0, 0, 1};
    const auto sprites =
        refresh_projected_entity_sprites(slots, 0x0080U, 0x0080U, 0U);
    if (slots[0].screen_x != 0x0088U || slots[0].screen_y != 0x0098U ||
        slots[1].screen_y != 0U || sprites.size() != 2U ||
        sprites[0].y != 0x80 || sprites[0].x != 0x93 ||
        sprites[0].size != 4U || sprites[0].sequence != 0U ||
        sprites[0].attributes != 0x47F2U || sprites[1].y != 0x84 ||
        sprites[1].x != 0x94 || sprites[1].size != 0U ||
        sprites[1].sequence != 1U || sprites[1].attributes != 0x45B2U) {
        return 7;
    }

    EntityEffectTimerState timer{};
    timer.countdown = 0;
    timer.slot = 2;
    timer.collision_base = 10;
    timer.saved_tiles = {1, 2, 3, 4, 5};
    std::array<std::uint8_t, 18> effect_slots{};
    effect_slots[2] = 1;
    std::vector<std::uint8_t> collision(20, 0xAA);
    auto effect_sprites = advance_entity_effect_timer(
        timer, effect_slots, collision, 0x0080U, 0x0080U, 0U);
    if (timer.countdown != -1 || effect_slots[2] != 0 ||
        !effect_sprites.empty() || collision[10] != 1) {
        return 8;
    }

    timer = {};
    timer.countdown = 5;
    timer.world_x = 0x0100U;
    timer.world_y = 0x0100U;
    timer.sprite_offset_y = 0x0010U;
    effect_sprites = advance_entity_effect_timer(
        timer, effect_slots, collision, 0x0080U, 0x0080U, 0U);
    if (timer.countdown != 4 || timer.screen_x != 0x0088U ||
        timer.screen_y != 0x0098U || effect_sprites.size() != 2U ||
        effect_sprites[0].y != 0x78 || effect_sprites[0].x != 0x88 ||
        effect_sprites[0].size != 15U ||
        effect_sprites[0].attributes != 0x2000U ||
        effect_sprites[1].y != 0x38 || effect_sprites[1].x != 0x88 ||
        effect_sprites[1].sequence != 1U) {
        return 9;
    }
    return 0;
}
