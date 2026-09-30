#include "shadowrun_ghidra/world/world_effect_overlay_conversion.hpp"

#include <cassert>
#include <cstdint>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectOverlayState primary{};
    primary.primary_started = true;
    primary.threshold_control = true;
    primary.source_x = 0x1AC0U;
    int descriptor_transitions = 0;
    WorldEffectOverlayCallbacks callbacks{};
    callbacks.begin_descriptor_transition = [&] { ++descriptor_transitions; };
    if (advance_primary_world_effect_overlay(primary, callbacks) !=
            WorldEffectOverlayAction::descriptor_transition_started ||
        primary.primary_started || primary.threshold_control ||
        !primary.descriptor_stream_ready || descriptor_transitions != 1) {
        return 1;
    }

    WorldEffectOverlayState primary_motion{};
    primary_motion.primary_started = 2U;
    primary_motion.primary_position = 0x1000U;
    primary_motion.motion_step = 0x0100U;
    primary_motion.frame_delay = 0U;
    int intro_requests = 0;
    int appends = 0;
    int builds = 0;
    callbacks = {};
    callbacks.arm_after_threshold = [](const std::uint16_t) { return false; };
    callbacks.advance_pending_roll = [&] { ++intro_requests; };
    if (advance_primary_world_effect_overlay(primary_motion, callbacks) !=
            WorldEffectOverlayAction::intro_transition_requested ||
        primary_motion.primary_position != 0x1100U || intro_requests != 1) {
        return 2;
    }

    WorldEffectOverlayState secondary{};
    secondary.secondary_started = 2U;
    secondary.secondary_position = 0x2000U;
    secondary.motion_step = 0x0100U;
    secondary.frame_delay = 1U;
    callbacks = {};
    callbacks.arm_after_threshold = [](const std::uint16_t) { return true; };
    callbacks.resolve_sprite_data =
        [](const WorldEffectOverlayKind kind,
           const std::uint32_t source_table_token,
           const std::uint16_t phase) -> std::optional<std::uint32_t> {
        assert(kind == WorldEffectOverlayKind::secondary &&
               source_table_token == 0x000F1BCEU);
        return source_table_token + phase;
    };
    callbacks.append_sprite_record = [&](const std::uint16_t key) {
        appends += key == 0x07A8U ? 1 : 100;
    };
    callbacks.build_sprite = [&](const WorldEffectSpriteBuild& build) {
        builds += build.source_table_token == 0x000F1BCEU &&
                          build.phase == 0U && build.base_y == 0x00B9U &&
                          build.base_x == (0x2100U >> 6U)
                      ? 1
                      : 100;
    };
    if (advance_secondary_world_effect_overlay(secondary, callbacks) !=
            WorldEffectOverlayAction::sprite_rendered ||
        !secondary.scene_complete || secondary.scene_completion_aux != 0U ||
        secondary.secondary_position != 0x2100U || appends != 1 || builds != 1 ||
        secondary.phase != 0U) {
        return 3;
    }

    WorldEffectOverlayState background{};
    background.background_started = true;
    background.intro_marker = false;
    background.background_position = 0x0100U;
    background.background_step = 0x0010U;
    background.frame_delay = 0U;
    background.phase = 3U;
    callbacks = {};
    callbacks.resolve_sprite_data =
        [](const WorldEffectOverlayKind kind,
           const std::uint32_t source_table_token, const std::uint16_t)
        -> std::optional<std::uint32_t> {
        assert(kind == WorldEffectOverlayKind::background &&
               source_table_token == 0x000F1BBEU);
        return source_table_token;
    };
    WorldEffectSpriteBuild background_build{};
    callbacks.build_sprite =
        [&](const WorldEffectSpriteBuild& build) { background_build = build; };
    if (advance_background_world_effect_overlay(background, callbacks) !=
            WorldEffectOverlayAction::sprite_rendered ||
        background.background_started || background.background_position != 0x0110U ||
        background.phase != 0U ||
        background_build.source_table_token != 0x000F1BBEU ||
        background_build.base_y != 0x00D0U ||
        background_build.base_x != (0x0110U >> 4U)) {
        return 4;
    }

    WorldEffectOverlayState primary_phase{};
    primary_phase.primary_started = 1U;
    primary_phase.frame_delay = 0U;
    primary_phase.phase = 0x19U;
    callbacks = {};
    callbacks.resolve_sprite_data =
        [](const WorldEffectOverlayKind kind,
           const std::uint32_t source_table_token, const std::uint16_t phase)
        -> std::optional<std::uint32_t> {
        assert(kind == WorldEffectOverlayKind::primary &&
               source_table_token == 0x000F1BDEU && phase == 0U);
        return source_table_token + phase;
    };
    if (advance_primary_world_effect_overlay(primary_phase, callbacks) !=
            WorldEffectOverlayAction::sprite_rendered ||
        primary_phase.phase != 0U || primary_phase.frame_delay != 8U) {
        return 5;
    }

    return 0;
}
