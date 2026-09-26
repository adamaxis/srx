#include "shadowrun_ghidra/world/world_effect_record_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectRecordState record{};
    record.position_x = 0x0200;
    record.position_y = 0x1000;
    record.frame_delay = 0;
    record.source_variant = 1;
    record.active = 0xFF;
    record.animation_phase = 2;
    record.phase_delay = 0;
    record.lifetime = 0x3C;
    record.velocity_x = 2;
    record.velocity_y = -4;
    record.target_x = 0x1000;
    record.direction = 1;

    std::uint8_t easing_index_seen = 0xFF;
    WorldEffectRecordCallbacks callbacks{};
    callbacks.resolve_easing = [&](const std::uint8_t index) {
        easing_index_seen = index;
        return static_cast<std::int16_t>(0);
    };
    if (advance_world_effect_record(record, callbacks) !=
            WorldEffectRecordAction::advanced ||
        easing_index_seen != 1U || record.lifetime != 0x3BU ||
        record.frame_delay != 0x14U || record.source_variant != 2U ||
        record.target_x != 0x1002 || record.position_x != 0x1002 ||
        record.position_y != 0x0FFCU || record.phase_delay != 2U ||
        record.animation_phase != 0U || record.active != 0xFFU) {
        return 1;
    }

    record = {};
    record.lifetime = 0U;
    record.active = 0xFFU;
    record.direction = 0;
    record.target_x = 0x2000;
    record.velocity_x = 4;
    record.frame_delay = 1U;
    record.phase_delay = 0U;
    record.animation_phase = 1U;
    callbacks.resolve_easing = [](const std::uint8_t) {
        return static_cast<std::int16_t>(0x4000);
    };
    if (advance_world_effect_record(record, callbacks) !=
            WorldEffectRecordAction::advanced) {
        return 21;
    }
    if (record.active != 0U) {
        return 22;
    }
    if (record.position_x >= 0x2000) {
        return 23;
    }
    if (record.animation_phase != 2U) {
        return 24;
    }
    if (record.phase_delay != 2U) {
        return 25;
    }

    WorldEffectRecordState source{};
    source.source_variant = 2U;
    source.animation_phase = 1U;
    source.position_x = static_cast<std::int16_t>(0x1230);
    source.position_y = static_cast<std::int16_t>(0xFFE0);
    std::uint8_t variant_seen = 0xFF;
    std::uint8_t phase_seen = 0xFF;
    WorldEffectSpriteCallbacks source_callbacks{};
    source_callbacks.resolve_sprite_source =
        [&](const std::uint8_t variant, const std::uint8_t phase)
        -> std::optional<std::uint32_t> {
        variant_seen = variant;
        phase_seen = phase;
        return 0x1D128U;
    };
    const auto resolved =
        resolve_world_effect_sprite(source, source_callbacks);
    if (!resolved.has_value() || resolved->source_table_token != 0x1D128U ||
        resolved->position_x != 0x0123U ||
        resolved->position_y != 0x0FFEU || variant_seen != 2U ||
        phase_seen != 1U) {
        return 3;
    }

    WorldEffectSpawnState spawn{};
    spawn.work_record_selected = true;
    spawn.source_x = 100;
    spawn.source_y = 200;
    spawn.destination_x = 300;
    spawn.destination_y = 400;
    int clears = 0;
    int audio = 0;
    WorldEffectSpawnCallbacks spawn_callbacks{};
    spawn_callbacks.clear_work_record = [&] { ++clears; };
    spawn_callbacks.play_effect_audio = [&] {
        ++audio;
        return static_cast<std::int16_t>(7);
    };
    if (spawn_world_effect_record(spawn, spawn_callbacks) !=
            WorldEffectSpawnAction::spawned ||
        spawn.record.active != 0xFFU || spawn.record.direction != 7 ||
        spawn.record.target_x != 0x0840 ||
        spawn.record.position_y != 0x0A00 ||
        spawn.record.velocity_x != 44 || spawn.record.velocity_y != 64 ||
        spawn.record.lifetime != 0x3CU || clears != 1 || audio != 1) {
        return 4;
    }

    spawn.record.active = 0xFFU;
    if (spawn_world_effect_record(spawn, spawn_callbacks) !=
            WorldEffectSpawnAction::record_already_active || clears != 1 ||
        audio != 1) {
        return 5;
    }
    return 0;
}
