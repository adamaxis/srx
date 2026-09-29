#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"

#include <cstdint>
#include <vector>

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

    ControllerEventLatchState latch;
    const auto no_event = latch_controller_events(latch, 0U, 0U);
    if (!expect(no_event.event_bits == 0U &&
                latch.activity_timer == 0 && !latch.start_latched)) {
        return 1;
    }
    const auto event = latch_controller_events(latch, 0x2000U, 0x0100U);
    if (!expect(event.event_bits == 0x2100U &&
                event.activity_latched && !event.start_latched &&
                latch.activity_timer == -1 && !latch.start_latched)) {
        return 1;
    }
    const auto start = latch_controller_events(latch, 0U, 0x8000U);
    if (!expect(start.event_bits == 0x8000U && start.start_latched &&
                latch.start_latched)) {
        return 1;
    }

    std::uint32_t random_state = 0U;
    if (!expect(advance_source_prng(random_state) == 1U &&
                random_state == 1U &&
                advance_source_prng(random_state) == 0x20E6U)) {
        return 1;
    }
    random_state = 0U;
    if (!expect(sample_source_random_bound(random_state, 0xFFFFU) == 0 &&
                random_state == 1U &&
                sample_source_random_bound(random_state, 0xFFFFU) ==
                    static_cast<std::int16_t>(0x20E5U))) {
        return 1;
    }
    random_state = 0xFFFFFFFFU;
    if (!expect(advance_source_prng(random_state) == 0xDF1CU &&
                random_state == 0xDF1CU)) {
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

    TitleLogoRevealState logo;
    const auto first_column = advance_title_logo_column(logo);
    if (!expect(first_column.column_render_requested &&
                !first_column.completed && first_column.column_index == 0U &&
                logo.columns_revealed == 1U &&
                logo.source_descriptor_offset == 2U)) {
        return 1;
    }
    logo.columns_revealed = 0x24U;
    logo.source_descriptor_offset = 0x48U;
    const auto completion_column = advance_title_logo_column(logo);
    if (!expect(completion_column.column_render_requested &&
                completion_column.completed &&
                completion_column.completion_palette_copy_requested &&
                completion_column.column_index == 0x24U &&
                logo.columns_revealed == kTitleCompletionColumn &&
                logo.source_descriptor_offset == 0x4AU)) {
        return 1;
    }
    const auto final_column = advance_title_logo_column(logo);
    if (!expect(final_column.column_render_requested &&
                !final_column.completed && final_column.column_index == 0x25U &&
                logo.columns_revealed == kTitleLogoColumnLimit)) {
        return 1;
    }
    if (!expect(!advance_title_logo_column(logo).column_render_requested)) {
        return 1;
    }

    TitleTextState text{1U, 2};
    if (!expect(advance_title_text_state(text) == TitleTextAction::None &&
                text.countdown == 1)) {
        return 1;
    }
    if (!expect(advance_title_text_state(text) ==
                    TitleTextAction::StepPaletteTowardTarget &&
                text.countdown == 0)) {
        return 1;
    }
    if (!expect(advance_title_text_state(text) ==
                    TitleTextAction::ShowCopyright &&
                text.completion_state == 2U)) {
        return 1;
    }
    if (!expect(advance_title_text_state(text) == TitleTextAction::None)) {
        return 1;
    }

    TitleSpriteState sprite;
    const auto first_sprite = advance_title_sprite_state(sprite);
    if (!expect(first_sprite.phase_changed &&
                first_sprite.projection_rebuild_requested &&
                sprite.cadence == 3 && sprite.phase == 1U &&
                sprite.x == 0x9C)) {
        return 1;
    }
    for (int frame = 0; frame < 4; ++frame) {
        advance_title_sprite_state(sprite);
    }
    if (!expect(sprite.phase == 2U)) {
        return 1;
    }
    sprite.phase = 5U;
    sprite.cadence = 0;
    if (!expect(advance_title_sprite_state(sprite).phase_changed &&
                sprite.phase == 0U)) {
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

    TitleLoopState loop;
    loop.logo_reveal_delay = 0;
    std::size_t loop_vblanks = 0;
    std::size_t loop_samples = 0;
    std::size_t palette_steps = 0;
    std::vector<TitlePromptVariant> prompt_variants;
    TitleLoopCallbacks loop_callbacks{};
    loop_callbacks.wait_for_vblank = [&loop_vblanks] { ++loop_vblanks; };
    loop_callbacks.sample_controller = [&loop_samples] {
        ++loop_samples;
        return loop_samples == 2U ? static_cast<std::uint16_t>(0x4000U)
                                  : static_cast<std::uint16_t>(0U);
    };
    loop_callbacks.draw_prompt_variant =
        [&prompt_variants](const TitlePromptVariant variant) {
            prompt_variants.push_back(variant);
        };
    loop_callbacks.step_palette_transition = [&palette_steps] {
        ++palette_steps;
    };

    if (!expect(advance_title_loop_frame(loop, loop_callbacks) ==
                    TitleLoopFrameResult::start_requested &&
                loop_vblanks == 1U && loop_samples == 3U &&
                loop.logo.columns_revealed == 1U &&
                loop.sprite.phase == 1U && loop.input_history.count == 0U &&
                palette_steps == 1U && prompt_variants.empty())) {
        return 1;
    }

    TitleLoopState completed_loop;
    completed_loop.prompt.frames_remaining = 0U;
    std::size_t completed_samples = 0;
    std::size_t completed_palette_steps = 0;
    TitleLoopCallbacks completed_callbacks{};
    completed_callbacks.sample_controller = [&completed_samples] {
        ++completed_samples;
        return static_cast<std::uint16_t>(0U);
    };
    completed_callbacks.step_palette_transition = [&completed_palette_steps] {
        ++completed_palette_steps;
    };
    if (!expect(advance_title_loop_frame(
                    completed_loop, completed_callbacks) ==
                    TitleLoopFrameResult::source_complete &&
                completed_samples == 3U && completed_palette_steps == 1U &&
                completed_loop.prompt.finished)) {
        return 1;
    }
    return 0;
}
