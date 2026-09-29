#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"

#include "shadowrun_ghidra/core/random_conversion.hpp"

namespace shadowrun::ghidra_conversion {

ControllerSample derive_controller_sample(std::uint8_t current,
                                           std::uint8_t previous) {
    return ControllerSample{
        current,
        static_cast<std::uint8_t>((current ^ previous) & current)};
}

ControllerEventLatchResult latch_controller_events(
    ControllerEventLatchState& state, std::uint16_t pending_events,
    std::uint16_t sampled_word) {
    const auto event_bits = static_cast<std::uint16_t>(
        (pending_events | sampled_word) & 0xFF00U);
    if (event_bits == 0U) {
        return ControllerEventLatchResult{};
    }

    state.activity_timer = -1;
    const bool start_latched = (sampled_word & 0x8000U) != 0U;
    if (start_latched) {
        state.start_latched = true;
    }
    return ControllerEventLatchResult{
        event_bits,
        true,
        start_latched,
    };
}

std::uint32_t advance_source_prng(std::uint32_t& state) {
    auto seed = static_cast<std::uint16_t>(state);
    state = advance_source_random_seed(seed);
    return state;
}

std::int16_t sample_source_random_bound(std::uint32_t& state,
                                        const std::uint16_t bound) {
    auto seed = static_cast<std::uint16_t>(state);
    const auto rolled = sample_source_random_bound(seed, bound);
    state = seed;
    return rolled;
}

std::uint16_t palette_toward_black(std::uint16_t color) {
    auto component = static_cast<std::uint16_t>(color & 0x000EU);
    if (component != 0) {
        component = static_cast<std::uint16_t>(component - 0x0002U);
    }
    auto stepped = static_cast<std::uint16_t>((color & 0xEEE0U) | component);

    component = static_cast<std::uint16_t>(stepped & 0x00E0U);
    if (component != 0) {
        component = static_cast<std::uint16_t>(component - 0x0020U);
    }
    stepped = static_cast<std::uint16_t>((stepped & 0xEE0EU) | component);

    component = static_cast<std::uint16_t>(stepped & 0x0E00U);
    if (component != 0) {
        component = static_cast<std::uint16_t>(component - 0x0200U);
    }
    return static_cast<std::uint16_t>((stepped & 0xE0EEU) | component);
}

std::uint16_t palette_toward_target(std::uint16_t color,
                                    std::uint16_t target) {
    auto component = static_cast<std::uint16_t>(color & 0x000EU);
    if (component != (target & 0x000EU)) {
        component = static_cast<std::uint16_t>(component + 0x0002U);
    }
    auto stepped = static_cast<std::uint16_t>((color & 0xEEE0U) | component);

    component = static_cast<std::uint16_t>(stepped & 0x00E0U);
    if (component != (target & 0x00E0U)) {
        component = static_cast<std::uint16_t>(component + 0x0020U);
    }
    stepped = static_cast<std::uint16_t>((stepped & 0xEE0EU) | component);

    component = static_cast<std::uint16_t>(stepped & 0x0E00U);
    if (component != (target & 0x0E00U)) {
        component = static_cast<std::uint16_t>(component + 0x0200U);
    }
    return static_cast<std::uint16_t>((stepped & 0xE0EEU) | component);
}

void run_palette_transition(Palette& current, const Palette& target,
                            PaletteTransitionMode mode) {
    for (std::size_t pass = 0; pass < kPaletteTransitionPasses; ++pass) {
        for (std::size_t index = 0; index < kPaletteEntryCount; ++index) {
            current[index] = mode == PaletteTransitionMode::TowardBlack
                ? palette_toward_black(current[index])
                : palette_toward_target(current[index], target[index]);
        }
    }
}

void copy_target_palette(Palette& target, const Palette& source) {
    target = source;
}

bool advance_title_reveal(TitleRevealState& state) {
    if (state.completion_state == 1U) {
        --state.text_timer;
        if (state.text_timer < 0) {
            state.completion_state = 2U;
        }
    }

    --state.reveal_delay;
    if (state.reveal_delay >= 0 || state.revealed_columns > 0x25U) {
        return false;
    }

    bool revealed = false;
    --state.column_cadence;
    if (state.column_cadence < 0) {
        state.column_cadence = 1;
        if (state.revealed_columns < 38U) {
            ++state.revealed_columns;
            revealed = true;
            if (state.revealed_columns == 0x25U) {
                state.completion_state = 1U;
                state.text_timer = 0x5A;
            }
        }
    }

    --state.sprite_cadence;
    if (state.sprite_cadence < 0) {
        state.sprite_cadence = 3;
        ++state.sprite_phase;
        if (state.sprite_phase >= 6U) {
            state.sprite_phase = 0;
        }
    }
    state.sprite_x = static_cast<std::int16_t>(state.sprite_x + 4);
    return revealed;
}

bool record_title_input_edge(TitleInputHistory& history,
                             std::uint16_t sampled_word) {
    const auto edge = static_cast<std::uint8_t>(
        (sampled_word >> 8U) & 0x007FU);
    if (edge == 0U || history.count >= kTitleInputHistoryCapacity) {
        return false;
    }
    history.events[history.count] = edge;
    ++history.count;
    return true;
}

TitleInputSequenceResult finalize_title_input_sequence(
    TitleInputHistory& history) {
    constexpr std::array<std::uint8_t, 7> kSourceSequence{
        0x40U, 0x10U, 0x10U, 0x40U, 0x20U, 0x40U, 0x10U};
    if (history.count == kSourceSequence.size()) {
        bool matches = true;
        for (std::size_t index = 0; index < kSourceSequence.size(); ++index) {
            if (history.events[index] != kSourceSequence[index]) {
                matches = false;
                break;
            }
        }
        if (matches) {
            history.sequence_complete = true;
            return TitleInputSequenceResult::Matched;
        }
    }
    history.count = 0;
    history.sequence_complete = false;
    return TitleInputSequenceResult::NotMatched;
}

std::optional<TitlePromptVariant> advance_title_prompt_schedule(
    TitlePromptSchedule& schedule) {
    if (schedule.finished) {
        return std::nullopt;
    }

    const auto frame = schedule.frames_remaining;
    std::optional<TitlePromptVariant> variant;
    if (frame < TitlePromptSchedule::kPromptStart) {
        const auto blink_frame = static_cast<std::uint16_t>(
            frame & (TitlePromptSchedule::kBlinkPeriod - 1U));
        variant = blink_frame < TitlePromptSchedule::kVariantBFrames
            ? TitlePromptVariant::VariantB
            : TitlePromptVariant::VariantA;
    }

    if (schedule.frames_remaining == 0U) {
        schedule.finished = true;
    } else {
        --schedule.frames_remaining;
    }
    return variant;
}

TitleLogoRevealResult advance_title_logo_column(TitleLogoRevealState& state) {
    if (state.columns_revealed >= kTitleLogoColumnLimit) {
        return TitleLogoRevealResult{};
    }

    const auto column_index = state.columns_revealed;
    ++state.columns_revealed;
    state.source_descriptor_offset = static_cast<std::uint16_t>(
        state.source_descriptor_offset + 2U);

    const bool completed =
        state.columns_revealed == kTitleCompletionColumn;
    return TitleLogoRevealResult{
        true,
        completed,
        completed,
        column_index,
    };
}

TitleTextAction advance_title_text_state(TitleTextState& state) {
    if (state.completion_state != 1U) {
        return TitleTextAction::None;
    }

    --state.countdown;
    if (state.countdown < 0) {
        state.completion_state = 2U;
        return TitleTextAction::ShowCopyright;
    }
    if ((state.countdown & 0x001FU) == 0) {
        return TitleTextAction::StepPaletteTowardTarget;
    }
    return TitleTextAction::None;
}

TitleSpriteAdvanceResult advance_title_sprite_state(TitleSpriteState& state) {
    --state.cadence;
    bool phase_changed = false;
    if (state.cadence < 0) {
        state.cadence = 3;
        ++state.phase;
        if (state.phase > 5U) {
            state.phase = 0U;
        }
        phase_changed = true;
    }
    state.x = static_cast<std::int16_t>(state.x + 4);
    return TitleSpriteAdvanceResult{phase_changed, true};
}

bool advance_title_animation_timer(TitleAnimationTimer& timer) {
    --timer.countdown;
    if (timer.countdown >= 0) {
        return false;
    }
    timer.countdown = 4;
    ++timer.animation_offset;
    return true;
}

std::optional<std::uint8_t> advance_title_circuit_lights(
    TitleCircuitLightState& state, std::uint16_t completion_state) {
    if (completion_state != 2U) {
        return std::nullopt;
    }
    --state.countdown;
    if (state.countdown >= 0) {
        return std::nullopt;
    }
    state.countdown = 4;
    state.phase = static_cast<std::uint8_t>((state.phase + 1U) % 7U);
    return state.phase;
}

TitleLoopFrameResult advance_title_loop_frame(
    TitleLoopState& state, const TitleLoopCallbacks& callbacks) {
    // 0x1ABA samples once after setup and before the first vblank loop pass.
    // The source uses it to seed the controller edge state; it is not a title
    // input event and therefore is not recorded or latched here.
    if (!state.initial_controller_sampled) {
        state.initial_controller_sampled = true;
        if (callbacks.sample_controller) {
            callbacks.sample_controller();
        }
    }

    // 0x1A9E starts each iteration at the vblank boundary, then services the
    // already-converted title state before sampling the controller.
    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }

    const auto text_action = advance_title_text_state(state.text);
    if (text_action != TitleTextAction::None && callbacks.apply_text_action) {
        callbacks.apply_text_action(text_action);
    }

    if (const auto phase = advance_title_circuit_lights(
            state.circuit_lights, state.text.completion_state);
        phase.has_value() && callbacks.publish_circuit_light_phase) {
        callbacks.publish_circuit_light_phase(*phase);
    }

    if (advance_title_animation_timer(state.animation_timer) &&
        callbacks.publish_animation_offset) {
        callbacks.publish_animation_offset(state.animation_timer.animation_offset);
    }

    // 0x195C -> 0x1BEA -> 0x0BF2. The host supplies the already sampled
    // controller word; the conversion retains the source edge recording and
    // event latch behavior without exposing a controller port.
    const auto sampled_word = callbacks.sample_controller
        ? callbacks.sample_controller()
        : static_cast<std::uint16_t>(0U);
    record_title_input_edge(state.input_history, sampled_word);
    state.last_controller_events = latch_controller_events(
        state.controller_events, 0U, sampled_word);
    if (callbacks.publish_controller_events) {
        callbacks.publish_controller_events(state.last_controller_events);
    }

    --state.logo_reveal_delay;
    if (state.logo_reveal_delay < 0 &&
        state.logo.columns_revealed < kTitleLogoColumnLimit) {
        --state.logo_column_cadence;
        if (state.logo_column_cadence < 0) {
            state.logo_column_cadence = 1;
            const auto logo_result = advance_title_logo_column(state.logo);
            if (logo_result.completed) {
                state.text.completion_state = 1U;
                state.text.countdown = 0x5AU;
            }
            if (logo_result.completion_palette_copy_requested &&
                callbacks.copy_completion_palette) {
                callbacks.copy_completion_palette();
            }
            if (callbacks.render_logo_column) {
                callbacks.render_logo_column(logo_result);
            }
        }

        const auto sprite_result = advance_title_sprite_state(state.sprite);
        if (callbacks.rebuild_sprite_projection) {
            callbacks.rebuild_sprite_projection(sprite_result);
        }
        state.display_dirty_flags = static_cast<std::uint8_t>(
            state.display_dirty_flags | 0x02U);
    } else if (callbacks.reset_vdp_upload_state) {
        callbacks.reset_vdp_upload_state();
    }

    const auto prompt = advance_title_prompt_schedule(state.prompt);
    if (prompt.has_value() && callbacks.draw_prompt_variant) {
        callbacks.draw_prompt_variant(*prompt);
    }

    const auto finish_title_loop = [&] {
        // The ROM takes one final controller sample after leaving the loop,
        // then performs its palette transition and validates the seven-edge
        // sequence. Do not feed this final sample into the edge buffer.
        if (callbacks.sample_controller) {
            callbacks.sample_controller();
        }
        if (callbacks.step_palette_transition) {
            callbacks.step_palette_transition();
        }
        finalize_title_input_sequence(state.input_history);
    };

    if (state.last_controller_events.event_bits != 0U) {
        finish_title_loop();
        return TitleLoopFrameResult::start_requested;
    }
    if (state.prompt.finished) {
        finish_title_loop();
        return TitleLoopFrameResult::source_complete;
    }
    return TitleLoopFrameResult::running;
}

} // namespace shadowrun::ghidra_conversion
