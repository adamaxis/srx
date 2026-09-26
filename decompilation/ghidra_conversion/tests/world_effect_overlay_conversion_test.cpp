#include "shadowrun_ghidra/world_effect_overlay_conversion.hpp"

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
        [](const WorldEffectOverlayKind kind, const std::uint16_t phase)
        -> std::optional<std::uint32_t> {
        return 0xF1BCEU + static_cast<std::uint32_t>(kind) * 0x100U + phase;
    };
    callbacks.append_sprite_record = [&](const std::uint16_t key) {
        appends += key == 0x07A8U ? 1 : 100;
    };
    callbacks.build_sprite = [&](const WorldEffectSpriteBuild& build) {
        builds += build.attribute == 0x00B9U &&
                          build.projected_position == (0x2100U >> 6U)
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
        [](const WorldEffectOverlayKind, const std::uint16_t)
        -> std::optional<std::uint32_t> { return 0xF1BBEU; };
    WorldEffectSpriteBuild background_build{};
    callbacks.build_sprite =
        [&](const WorldEffectSpriteBuild& build) { background_build = build; };
    if (advance_background_world_effect_overlay(background, callbacks) !=
            WorldEffectOverlayAction::sprite_rendered ||
        background.background_started || background.background_position != 0x0110U ||
        background.phase != 0U || background_build.attribute != 0x00D0U ||
        background_build.projected_position != (0x0110U >> 4U)) {
        return 4;
    }

    return 0;
}
