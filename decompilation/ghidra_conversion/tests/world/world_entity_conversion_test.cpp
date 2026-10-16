#include "shadowrun_ghidra/world/world_entity_conversion.hpp"

#include <array>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    WorldFrameState world{true, true, true, true, false, true, true, 2};
    world.world_effect_event_lock = true;
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
    WorldFrameCallbacks source_callbacks = callbacks;
    source_callbacks.reset_decoded_entity_stream_cursors =
        [&] { events.push_back(18); };
    run_world_frame_services(world, source_callbacks);
    if (world.audio_command_frame_pending || !world.field_refresh_pending ||
        world.world_effect_event_lock || world.effect_transition_pending ||
        world.entity_source_stream_decode_pending || events.size() != 18 ||
        events[0] != 1 || events[1] != 2 || events[16] != 18 ||
        events[17] != 17) {
        return 1;
    }

    // The alternate source branch consumes FFE171 and then clears FFDF8A.
    world = {};
    world.field_refresh_pending = true;
    world.terrain_copy_suppression_pending = true;
    run_world_frame_services(world, {});
    if (world.field_refresh_pending ||
        world.terrain_copy_suppression_pending) {
        return 2;
    }

    std::array<EntityFrameRecord, 16> entities{};
    entities[0] = {true, 1, 0xF0, 1, 2, 0, false};
    int expanded = 0;
    update_entity_frame(entities, {
        {}, {}, {}, [&](EntityFrameRecord&) {}, {}, {}, {}, {}, {},
        [&](EntityFrameRecord&) { ++expanded; }, {}, {}, {}});
    if (expanded != 1) {
        return 3;
    }

    EffectAnimationState animation{0, 0, 1, 0, true};
    int builds = 0;
    for (int i = 0; i < 8; ++i) {
        advance_effect_animation(animation, {}, {}, [&] { ++builds; });
    }
    if (animation.active || builds == 0) {
        return 4;
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
        return 5;
    }
    if (!set_sprite_descriptor_carry()) {
        return 6;
    }

    if (select_projected_entity_sprite_source(0) != 0x00005F78U ||
        select_projected_entity_sprite_source(12) != 0x000095A4U ||
        select_projected_entity_sprite_source(0x64) != 0x00005FAAU ||
        select_projected_entity_sprite_source(0x65) != 0x0001D47CU ||
        select_projected_entity_sprite_source(-0x4000) != 0x00005F82U ||
        select_projected_entity_sprite_source(-1) != 0x00005FA0U) {
        return 7;
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
        return 8;
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
        return 9;
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
        return 10;
    }

    EffectAnimationSpriteState anim{};
    anim.animation = {0, 3, 3, 0, true};
    anim.slot = 4;
    anim.entity_flags = 0x40;
    std::array<std::uint8_t, 18> anim_slots{};
    anim_slots[4] = 1;
    auto anim_sprites = advance_effect_animation(anim, anim_slots, 0U);
    if (anim.animation.active || anim_slots[4] != 0 ||
        (anim.entity_flags & 0x40U) != 0 || !anim_sprites.empty()) {
        return 11;
    }
    anim = {};
    anim.animation = {1, 0, 3, 0, true};
    anim.screen_x = 0x0088U;
    anim.screen_y = 0x0098U;
    anim_sprites = advance_effect_animation(anim, anim_slots, 0U);
    if (anim.animation.countdown != 0 || anim.animation.phase != 0 ||
        anim_sprites.size() != 1U || anim_sprites[0].y != 0x81 ||
        anim_sprites[0].x != 0x71 || anim_sprites[0].size != 5U ||
        anim_sprites[0].attributes != 0x2000U) {
        return 12;
    }

    EffectAnimationVariantState variant{};
    variant.animation = {0, 4, 4, 0, true};
    auto variant_sprites = advance_effect_animation_variant(variant, 0U);
    if (variant.animation.active || variant.auxiliary != 0 ||
        !variant_sprites.empty()) {
        return 13;
    }
    variant = {};
    variant.animation = {1, 0, 4, 0, true};
    variant.screen_x = 0x0088U;
    variant.screen_y = 0x0098U;
    variant_sprites = advance_effect_animation_variant(variant, 0U);
    if (variant.animation.countdown != 0 || variant_sprites.size() != 1U ||
        variant_sprites[0].y != 0x7A || variant_sprites[0].x != 0x7D ||
        variant_sprites[0].size != 15U ||
        variant_sprites[0].attributes != 0x6000U) {
        return 14;
    }
    variant.animation = {1, 4, 4, 0, true};
    variant.jitter_bit = 1;
    variant_sprites = advance_effect_animation_variant(variant, 0U);
    if (variant_sprites[0].y != 0x7C || variant_sprites[0].x != 0x7D) {
        return 15;
    }

    JitterEffectSpriteState jitter_sprite{};
    jitter_sprite.animation = {0, 3, 3, 0, true};
    jitter_sprite.remaining = 0;
    auto jitter_sprites = advance_jitter_effect(jitter_sprite, 0U);
    if (jitter_sprite.animation.active || jitter_sprite.auxiliary != 0 ||
        !jitter_sprites.empty()) {
        return 16;
    }
    jitter_sprite = {};
    jitter_sprite.animation = {1, 0, 3, 0, true};
    jitter_sprite.screen_x = 0x0088U;
    jitter_sprite.screen_y = 0x0098U;
    jitter_sprites = advance_jitter_effect(jitter_sprite, 0U);
    if (jitter_sprite.animation.countdown != 0 || jitter_sprites.size() != 1U ||
        jitter_sprites[0].y != 0x85 || jitter_sprites[0].x != 0x82 ||
        jitter_sprites[0].size != 1U ||
        jitter_sprites[0].attributes != 0x6000U) {
        return 17;
    }
    jitter_sprite.animation = {1, 1, 3, 0, true};
    jitter_sprite.random_word = 0x1000U;
    jitter_sprites = advance_jitter_effect(jitter_sprite, 0U);
    if (jitter_sprites[0].y != 0x82 || jitter_sprites[0].x != 0x7D ||
        jitter_sprites[0].size != 5U) {
        return 18;
    }
    jitter_sprite = {};
    jitter_sprite.animation = {0, 2, 3, 0, true};
    jitter_sprite.remaining = 1;
    jitter_sprite.seed = 1;
    jitter_sprite.screen_x = 0x0088U;
    jitter_sprite.screen_y = 0x0098U;
    jitter_sprites = advance_jitter_effect(jitter_sprite, 0U);
    if (jitter_sprite.animation.phase != 0 ||
        jitter_sprite.animation.countdown != 5 ||
        jitter_sprite.offset_x != -6 || jitter_sprite.offset_y != -6 ||
        jitter_sprites[0].y != 0x7F || jitter_sprites[0].x != 0x7C) {
        return 19;
    }

    EffectVariantRecord self{};
    std::array<EffectVariantRecord, 9> neighbors{};
    EffectVariantRecord auxiliary{};
    EffectVariantSharedState shared{};
    shared.gate = 2;
    auto scan = run_effect_variant_a(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::gated || shared.gate != 1 ||
        !scan.sprites.empty()) {
        return 20;
    }
    self.spawn.auxiliary = 0x0D;
    self.spawn.countdown = 1;
    self.screen_x = 0x0088U;
    self.screen_y = 0x0098U;
    shared.gate = 0;
    shared.delay = 1;
    scan = run_effect_variant_a(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::jitter_live ||
        shared.gate != -1 || shared.delay != 0 || scan.sprites.size() != 1U ||
        scan.sprites[0].y != 0x85 || scan.sprites[0].x != 0x82) {
        return 21;
    }
    self.spawn.auxiliary = 0;
    self.spawn.countdown = 1;
    self.spawn.slot = 3;
    self.spawn.attribute = 0x2A;
    self.spawn.script = 0xD800U;
    neighbors[1].kind = 1;
    neighbors[1].discriminator = 0x0D;
    neighbors[1].flags = 1;
    shared.gate = -1;
    scan = run_effect_variant_a(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::child_cloned ||
        scan.cloned_index != 1U || neighbors[1].spawn.auxiliary != 0x0D ||
        neighbors[1].spawn.countdown != 5 || neighbors[1].spawn.slot != 3 ||
        shared.phase_gate != 4) {
        return 22;
    }
    neighbors = {};
    neighbors[0].kind = 9;
    shared.delay = -1;
    shared.slots[3] = 1;
    shared.cleanup.transient_flags.fill(0xFFU);
    shared.cleanup.masked_flags.fill(0xFFU);
    scan = run_effect_variant_a(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::expired ||
        shared.slots[3] != 0 || shared.cleanup.transient_flags[0] != 0 ||
        shared.cleanup.masked_flags[0] != 0xFDU) {
        return 23;
    }
    shared.delay = 0;
    scan = run_effect_variant_a(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::auxiliary_cloned ||
        shared.gate != 0x1E || auxiliary.spawn.auxiliary != 0x0D ||
        auxiliary.spawn.script != 0xD800U) {
        return 24;
    }

    self = {};
    neighbors = {};
    auxiliary = {};
    shared = {};
    self.spawn.auxiliary = 6;
    self.spawn.countdown = 1;
    self.screen_x = 0x0088U;
    self.screen_y = 0x0098U;
    shared.gate = -1;
    scan = run_effect_variant_b(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::jitter_live ||
        scan.sprites.size() != 1U || scan.sprites[0].y != 0x7A ||
        scan.sprites[0].x != 0x7D || scan.sprites[0].size != 15U ||
        scan.sprites[0].attributes != 0x6000U) {
        return 25;
    }
    self.spawn.auxiliary = 0;
    self.spawn.countdown = 1;
    self.spawn.slot = 4;
    self.spawn.script = 0xABU;
    neighbors[2].kind = 1;
    neighbors[2].discriminator = 6;
    neighbors[2].flags = 1;
    scan = run_effect_variant_b(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::child_cloned ||
        scan.cloned_index != 2U || neighbors[2].spawn.auxiliary != 6 ||
        neighbors[2].spawn.script != 0xABU) {
        return 26;
    }
    neighbors = {};
    neighbors[0].kind = 9;
    shared.delay = -1;
    shared.slots[4] = 1;
    shared.cleanup.transient_flags.fill(0xFFU);
    shared.cleanup.masked_flags.fill(0xFFU);
    scan = run_effect_variant_b(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::expired ||
        shared.slots[4] != 0 || shared.cleanup.masked_flags[0] != 0xFEU) {
        return 27;
    }
    shared.delay = 0;
    scan = run_effect_variant_b(self, neighbors, auxiliary, shared, 0U);
    if (scan.result != EffectVariantScanResult::auxiliary_cloned ||
        shared.gate != 0x1E || auxiliary.spawn.auxiliary != 6) {
        return 28;
    }

    EntityAnimationEffectState anim_effect{};
    anim_effect.countdown = 1;
    anim_effect.screen_x = 0x0088U;
    anim_effect.screen_y = 0x0098U;
    auto anim_effect_sprites = advance_entity_animation_effect(anim_effect, 0U);
    if (anim_effect.countdown != 0 || anim_effect_sprites.size() != 1U ||
        anim_effect_sprites[0].y != 0x7A || anim_effect_sprites[0].x != 0x78 ||
        anim_effect_sprites[0].size != 14U ||
        anim_effect_sprites[0].attributes != 0x4580U) {
        return 29;
    }
    anim_effect.countdown = 0;
    anim_effect.phase = 3;
    anim_effect.marker = 0x000FU;
    anim_effect_sprites = advance_entity_animation_effect(anim_effect, 0U);
    if (anim_effect.countdown != 3 || anim_effect.phase != 0 ||
        anim_effect_sprites.size() != 2U ||
        anim_effect_sprites[0].y != 0x4C || anim_effect_sprites[0].x != 0x58 ||
        anim_effect_sprites[1].y != 0xA3 || anim_effect_sprites[1].x != 0x58 ||
        anim_effect_sprites[1].sequence != 1U) {
        return 30;
    }
    EntityAnimationEffectState delayed{};
    delayed.countdown = 1;
    delayed.screen_x = 0x0088U;
    delayed.screen_y = 0x0098U;
    auto delayed_sprites = refresh_delayed_effect_sprites(delayed, 0U);
    if (delayed.countdown != 0 || delayed_sprites.size() != 1U ||
        delayed_sprites[0].y != 0x7C || delayed_sprites[0].x != 0x84 ||
        delayed_sprites[0].size != 13U ||
        delayed_sprites[0].attributes != 0x245FU) {
        return 31;
    }

    EntityEffectSpawnState spawned{};
    std::array<std::uint8_t, 18> spawn_slots{};
    std::vector<std::uint8_t> spawn_map(0x400, 0xAA);
    if (!spawn_entity_effect(spawned, {0x1000U, 0x2000U, 0, 10}, 1U,
                             spawn_slots, spawn_map) ||
        spawned.active != 1 || spawned.slot != 0 ||
        spawned.attribute != 0x49 || spawned.heading != 0 ||
        spawned.countdown != 0x190U || spawned.world_x != 0x1100U ||
        spawned.world_y != 0x1B00U || spawned.collision_base != 0x348U ||
        spawn_map[0x348] != 0x7E || spawned.saved_tiles[0] != 0xAA) {
        return 32;
    }
    spawn_slots.fill(1);
    spawned.active = 0;
    if (spawn_entity_effect(spawned, {0x1000U, 0x2000U, 0, 5}, 1U, spawn_slots,
                            spawn_map)) {
        return 32;
    }
    return 0;
}
