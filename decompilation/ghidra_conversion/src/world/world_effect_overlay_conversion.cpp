#include "shadowrun_ghidra/world/world_effect_overlay_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool decrement_until_negative(std::uint16_t& counter,
                              const std::uint16_t reload) {
    counter = static_cast<std::uint16_t>(counter - 1U);
    if ((counter & 0x8000U) == 0U) {
        return false;
    }
    counter = reload;
    return true;
}

WorldEffectOverlayAction render_overlay_sprite(
    WorldEffectOverlayState& state,
    const WorldEffectOverlayCallbacks& callbacks,
    const WorldEffectOverlayKind kind,
    const std::uint16_t position,
    const std::uint16_t position_shift,
    const std::uint16_t attribute,
    const std::uint16_t phase_limit) {
    if (state.phase > phase_limit) {
        state.phase = 0U;
    }
    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(kind, state.phase)
                           : std::nullopt;
    if (!token.has_value()) {
        return WorldEffectOverlayAction::sprite_data_unresolved;
    }

    if (callbacks.append_sprite_record) {
        callbacks.append_sprite_record(0x07A8U);
    }
    WorldEffectSpriteBuild build{};
    build.kind = kind;
    build.attribute = attribute;
    build.projected_position = static_cast<std::uint16_t>(
        position >> position_shift);
    build.sprite_data_token = *token;
    if (callbacks.build_sprite) {
        callbacks.build_sprite(build);
    }
    return WorldEffectOverlayAction::sprite_rendered;
}

void publish_threshold_result(WorldEffectOverlayState& state,
                              const WorldEffectOverlayCallbacks& callbacks,
                              const std::uint16_t position) {
    if (callbacks.arm_after_threshold &&
        callbacks.arm_after_threshold(position)) {
        state.scene_complete = true;
        state.scene_completion_aux = 0U;
    }
}

} // namespace

WorldEffectOverlayAction advance_primary_world_effect_overlay(
    WorldEffectOverlayState& state,
    const WorldEffectOverlayCallbacks& callbacks) {
    if (state.scene_complete || !state.primary_started) {
        return WorldEffectOverlayAction::no_action;
    }

    if (state.threshold_control) {
        state.primary_position = static_cast<std::uint16_t>(state.source_x << 2U);
        if (state.primary_position >= 0x6B00U) {
            state.threshold_control = false;
            state.primary_started = false;
            state.descriptor_stream_ready = true;
            if (callbacks.begin_descriptor_transition) {
                callbacks.begin_descriptor_transition();
            }
            return WorldEffectOverlayAction::descriptor_transition_started;
        }
    }

    if (state.primary_started != 1U) {
        state.primary_position = static_cast<std::uint16_t>(
            state.primary_position + state.motion_step);
        const auto threshold_reached =
            callbacks.arm_after_threshold
                ? callbacks.arm_after_threshold(state.primary_position)
                : false;
        if (threshold_reached) {
            state.scene_complete = true;
            state.scene_completion_aux = 0U;
        } else {
            if (callbacks.advance_pending_roll) {
                callbacks.advance_pending_roll();
            }
            return WorldEffectOverlayAction::intro_transition_requested;
        }
    }

    if (decrement_until_negative(state.frame_delay, 8U)) {
        ++state.phase;
        if (state.phase > 0x1AU) {
            state.phase = 0U;
        }
    }
    return render_overlay_sprite(state, callbacks,
                                 WorldEffectOverlayKind::primary,
                                 state.primary_position, 6U, 0x00B9U, 0x1AU);
}

WorldEffectOverlayAction advance_secondary_world_effect_overlay(
    WorldEffectOverlayState& state,
    const WorldEffectOverlayCallbacks& callbacks) {
    if (state.scene_complete || !state.secondary_started) {
        return WorldEffectOverlayAction::no_action;
    }

    if (state.threshold_control) {
        state.secondary_position =
            static_cast<std::uint16_t>(state.source_x << 2U);
        if (state.secondary_position >= 0x6B00U) {
            state.secondary_started = false;
            state.threshold_control = false;
            state.descriptor_stream_ready = true;
            if (callbacks.begin_descriptor_transition) {
                callbacks.begin_descriptor_transition();
            }
            return WorldEffectOverlayAction::descriptor_transition_started;
        }
    }

    if (state.secondary_started != 1U) {
        state.secondary_position = static_cast<std::uint16_t>(
            state.secondary_position + state.motion_step);
        publish_threshold_result(state, callbacks, state.secondary_position);
    }

    if (decrement_until_negative(state.frame_delay, 6U)) {
        ++state.phase;
        if (state.phase > 3U) {
            state.phase = 0U;
        }
    }
    return render_overlay_sprite(state, callbacks,
                                 WorldEffectOverlayKind::secondary,
                                 state.secondary_position, 6U, 0x00B9U, 3U);
}

WorldEffectOverlayAction advance_background_world_effect_overlay(
    WorldEffectOverlayState& state,
    const WorldEffectOverlayCallbacks& callbacks) {
    if (state.scene_complete || !state.background_started) {
        return WorldEffectOverlayAction::no_action;
    }

    if (!state.intro_marker) {
        state.background_started = false;
    }
    state.background_position = static_cast<std::uint16_t>(
        state.background_position + state.background_step);

    if (decrement_until_negative(state.frame_delay, 3U)) {
        ++state.phase;
        if (state.phase > 3U) {
            state.phase = 0U;
        }
    }
    return render_overlay_sprite(state, callbacks,
                                 WorldEffectOverlayKind::background,
                                 state.background_position, 4U, 0x00D0U, 3U);
}

} // namespace shadowrun::ghidra_conversion
