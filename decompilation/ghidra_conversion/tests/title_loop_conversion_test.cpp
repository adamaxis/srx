#include "shadowrun_ghidra/title_loop_conversion.hpp"

#include <cstdint>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto first = derive_controller_sample(0x41U, 0U);
    if (!expect(first.held == 0x41U && first.newly_pressed == 0x41U)) {
        return 1;
    }
    const auto held = derive_controller_sample(0x41U, first.held);
    if (!expect(held.newly_pressed == 0U)) {
        return 1;
    }

    Palette current{};
    Palette target{};
    current.fill(0x0EEE);
    run_palette_transition(current, target,
                            PaletteTransitionMode::TowardBlack);
    if (!expect(current[0] == 0U)) {
        return 1;
    }
    current.fill(0U);
    target.fill(0x0EEE);
    run_palette_transition(current, target,
                            PaletteTransitionMode::TowardTarget);
    if (!expect(current[0] == 0x0EEE)) {
        return 1;
    }

    TitleRevealState title;
    for (int frame = 0; frame < 90; ++frame) {
        if (!expect(!advance_title_reveal(title))) {
            return 1;
        }
    }
    if (!expect(title.revealed_columns == 0U)) {
        return 1;
    }
    if (!expect(advance_title_reveal(title) &&
                title.revealed_columns == 1U &&
                title.sprite_phase == 1U && title.sprite_x == 0x9C)) {
        return 1;
    }

    Palette source{};
    Palette copied{};
    source[7] = 0x0E60;
    copy_target_palette(copied, source);
    if (!expect(copied[7] == 0x0E60)) {
        return 1;
    }

    TitleInputHistory history;
    if (!expect(record_title_input_edge(history, 0x1200U) &&
                history.count == 1U && history.events[0] == 0x12U)) {
        return 1;
    }
    if (!expect(!record_title_input_edge(history, 0x8000U))) {
        return 1;
    }
    for (std::size_t index = history.count;
         index < kTitleInputHistoryCapacity; ++index) {
        if (!expect(record_title_input_edge(
                history, static_cast<std::uint16_t>(0x0100U + index)))) {
            return 1;
        }
    }
    if (!expect(history.count == kTitleInputHistoryCapacity &&
                !record_title_input_edge(history, 0x0100U))) {
        return 1;
    }

    TitleInputHistory sequence;
    for (const auto value : {0x40U, 0x10U, 0x10U, 0x40U,
                             0x20U, 0x40U, 0x10U}) {
        if (!expect(record_title_input_edge(
                sequence, static_cast<std::uint16_t>(value << 8U)))) {
            return 1;
        }
    }
    if (!expect(finalize_title_input_sequence(sequence) ==
                    TitleInputSequenceResult::Matched &&
                sequence.sequence_complete)) {
        return 1;
    }
    sequence.events[0] = 0x00U;
    if (!expect(finalize_title_input_sequence(sequence) ==
                    TitleInputSequenceResult::NotMatched &&
                sequence.count == 0U && !sequence.sequence_complete)) {
        return 1;
    }

    TitlePromptSchedule prompt;
    for (int frame = 0; frame < 0x1A5; ++frame) {
        if (!expect(!advance_title_prompt_schedule(prompt).has_value())) {
            return 1;
        }
    }
    if (!expect(prompt.frames_remaining == TitlePromptSchedule::kPromptStart - 1U)) {
        return 1;
    }
    if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantA})) {
        return 1;
    }
    prompt.frames_remaining = 9U;
    if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantB})) {
        return 1;
    }
    prompt.frames_remaining = 0U;
    prompt.finished = false;
    if (!expect(advance_title_prompt_schedule(prompt).has_value() &&
                prompt.finished)) {
        return 1;
    }
    if (!expect(!advance_title_prompt_schedule(prompt).has_value())) {
        return 1;
    }

    TitleAnimationTimer timer;
    if (!expect(advance_title_animation_timer(timer) &&
                timer.countdown == 4 && timer.animation_offset == 1U)) {
        return 1;
    }
    if (!expect(!advance_title_animation_timer(timer))) {
        return 1;
    }

    TitleCircuitLightState lights{0, 0};
    if (!expect(!advance_title_circuit_lights(lights, 1U).has_value())) {
        return 1;
    }
    const auto phase = advance_title_circuit_lights(lights, 2U);
    if (!expect(phase.has_value() && *phase == 1U && lights.countdown == 4)) {
        return 1;
    }
    return 0;
}
