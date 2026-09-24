#include "shadowrun_ghidra/title_loop_conversion.hpp"

namespace shadowrun::ghidra_conversion {

ControllerSample derive_controller_sample(std::uint8_t current,
                                           std::uint8_t previous) {
    return ControllerSample{
        current,
        static_cast<std::uint8_t>((current ^ previous) & current)};
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

} // namespace shadowrun::ghidra_conversion
