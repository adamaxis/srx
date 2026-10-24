#include "shadowrun_ghidra/title_intro/title_loop_conversion.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"

#include <algorithm>
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

    // docs/24 R2: the title shares FFF9F8; `random_state` mirrors it.
    std::uint32_t random_state = 0U;
    source_random_seed() = 0U;
    if (!expect(advance_source_prng(random_state) == 1U &&
                random_state == 1U &&
                advance_source_prng(random_state) == 0x20E6U)) {
        return 1;
    }
    random_state = 0U;
    source_random_seed() = 0U;
    if (!expect(sample_source_random_bound(random_state, 0xFFFFU) == 0 &&
                random_state == 1U &&
                sample_source_random_bound(random_state, 0xFFFFU) ==
                    static_cast<std::int16_t>(0x20E5U))) {
        return 1;
    }
    random_state = 0xFFFFFFFFU;
    source_random_seed() = 0xFFFFU;
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

    // The first visible stretch is truncated by the source countdown
    // boundary. Every later 64-VBlank period is 54 visible frames followed
    // by the ten-frame blank record.
    for (int frame = 0; frame < 33; ++frame) {
        if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantA})) {
            return 1;
        }
    }
    if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantB})) {
        return 1;
    }
    for (int frame = 0; frame < 9; ++frame) {
        if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantB})) {
            return 1;
        }
    }
    if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantA})) {
        return 1;
    }
    for (int frame = 0; frame < 53; ++frame) {
        if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantA})) {
            return 1;
        }
    }
    if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantB})) {
        return 1;
    }
    for (int frame = 0; frame < 9; ++frame) {
        if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantB})) {
            return 1;
        }
    }
    if (!expect(advance_title_prompt_schedule(prompt) ==
                    std::optional<TitlePromptVariant>{
                        TitlePromptVariant::VariantA})) {
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
                text.countdown == 0 && text.palette_steps == 1U)) {
        return 1;
    }
    if (!expect(advance_title_text_state(text) ==
                    TitleTextAction::ShowCopyright &&
                text.completion_state == 2U &&
                text.palette_steps == 1U)) {
        return 1;
    }
    if (!expect(advance_title_text_state(text) == TitleTextAction::None)) {
        return 1;
    }

    TitleTextState circuit_reveal{1U, 0x5AU};
    std::uint8_t observed_palette_steps = 0U;
    for (int frame = 0; frame < 0x5A; ++frame) {
        if (advance_title_text_state(circuit_reveal) ==
            TitleTextAction::StepPaletteTowardTarget) {
            ++observed_palette_steps;
        }
    }
    if (!expect(observed_palette_steps == 3U &&
                circuit_reveal.palette_steps == 3U &&
                circuit_reveal.completion_state == 1U &&
                circuit_reveal.countdown == 0)) {
        return 1;
    }
    if (!expect(advance_title_text_state(circuit_reveal) ==
                    TitleTextAction::ShowCopyright &&
                circuit_reveal.palette_steps == 3U &&
                circuit_reveal.completion_state == 2U)) {
        return 1;
    }

    // ROM 0x1E4E requests 0x5D on even reveal-column indices (19 requests
    // across 38 columns). ROM 0x1C1E requests title music 0x00 91 loop updates
    // after the 0x25 completion count is set, immediately before copyright.
    TitleLoopState title_audio;
    title_audio.logo_reveal_delay = 0;
    std::vector<TitleAudioCommand> title_audio_commands;
    std::vector<std::uint8_t> title_presentation_order;
    TitleLoopCallbacks title_audio_callbacks{};
    title_audio_callbacks.request_audio_command =
        [&title_audio_commands, &title_presentation_order](
            const TitleAudioCommand command) {
            title_audio_commands.push_back(command);
            title_presentation_order.push_back(
                static_cast<std::uint8_t>(command));
        };
    title_audio_callbacks.apply_text_action =
        [&title_presentation_order](const TitleTextAction action) {
            if (action == TitleTextAction::ShowCopyright) {
                title_presentation_order.push_back(0xFEU);
            }
        };
    for (int update = 0; update < 73; ++update) {
        advance_title_loop_frame(title_audio, title_audio_callbacks);
    }
    if (!expect(title_audio.logo.columns_revealed == 37U &&
                title_audio.text.completion_state == 1U &&
                title_audio.text.countdown == 0x5AU &&
                title_audio_commands.size() == 19U &&
                std::all_of(title_audio_commands.begin(),
                            title_audio_commands.end(), [](const auto command) {
                                return command == TitleAudioCommand::LogoReveal;
                            }))) {
        return 1;
    }
    for (int update = 0; update < 90; ++update) {
        advance_title_loop_frame(title_audio, title_audio_callbacks);
    }
    if (!expect(title_audio.text.countdown == 0 &&
                title_audio_commands.size() == 19U)) {
        return 1;
    }
    advance_title_loop_frame(title_audio, title_audio_callbacks);
    if (!expect(title_audio.text.completion_state == 2U &&
                title_audio_commands.size() == 20U &&
                title_audio_commands.back() == TitleAudioCommand::ThemeStart &&
                title_presentation_order.size() == 21U &&
                title_presentation_order[19] == static_cast<std::uint8_t>(
                    TitleAudioCommand::ThemeStart) &&
                title_presentation_order[20] == 0xFEU)) {
        return 1;
    }

    TitleSpriteState sprite;
    const auto first_sprite = advance_title_sprite_state(sprite);
    if (!expect(first_sprite.phase_changed &&
                first_sprite.projection_rebuild_requested &&
                sprite.cadence == 3 && sprite.phase == 1U &&
                sprite.x == 0x9C && sprite.projection_visible)) {
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
    for (int frame = 0; frame < 4; ++frame) {
        if (!expect(!advance_title_animation_timer(timer))) {
            return 1;
        }
    }
    if (!expect(timer.countdown == 0 && timer.animation_offset == 1U &&
                advance_title_animation_timer(timer) &&
                timer.countdown == 4 && timer.animation_offset == 2U)) {
        return 1;
    }
    for (int frame = 0; frame < 4; ++frame) {
        if (!expect(!advance_title_animation_timer(timer))) {
            return 1;
        }
    }
    if (!expect(advance_title_animation_timer(timer) &&
                timer.countdown == 4 && timer.animation_offset == 3U)) {
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
    std::size_t palette_event_latches = 0;
    std::vector<TitlePromptVariant> prompt_variants;
    TitleLoopCallbacks loop_callbacks{};
    loop_callbacks.wait_for_vblank = [&loop_vblanks] { ++loop_vblanks; };
    loop_callbacks.sample_controller = [&loop_samples] {
        ++loop_samples;
        if (loop_samples == 2U) {
            return static_cast<std::uint16_t>(0x4000U); // A is not Start.
        }
        if (loop_samples == 3U) {
            return static_cast<std::uint16_t>(0x8000U);
        }
        return static_cast<std::uint16_t>(0U);
    };
    loop_callbacks.draw_prompt_variant =
        [&prompt_variants](const TitlePromptVariant variant) {
            prompt_variants.push_back(variant);
        };
    loop_callbacks.step_palette_transition = [&palette_steps] {
        ++palette_steps;
    };
    loop_callbacks.latch_palette_transition_events =
        [&palette_event_latches] { ++palette_event_latches; };

    if (!expect(advance_title_loop_frame(loop, loop_callbacks) ==
                    TitleLoopFrameResult::running &&
                loop_vblanks == 1U && loop_samples == 2U &&
                loop.logo.columns_revealed == 1U &&
                loop.sprite.phase == 1U && loop.sprite.projection_visible &&
                loop.input_history.count == 1U &&
                !loop.exit_palette_fade.active && palette_steps == 0U &&
                prompt_variants.empty())) {
        return 1;
    }
    // ROM 0x1A9E tests FFDDC2 only in its D7 < 0x0C6C prompt branch: an
    // early Start stays latched through the logo reveal and is taken on the
    // first prompt-phase update (1a919311/69724662).
    if (!expect(advance_title_loop_frame(loop, loop_callbacks) ==
                    TitleLoopFrameResult::running &&
                loop_vblanks == 2U && loop_samples == 3U &&
                loop.controller_events.start_latched &&
                !loop.exit_palette_fade.active)) {
        return 1;
    }
    auto start_result = TitleLoopFrameResult::running;
    for (int frame = 0; frame < 0x2000 &&
                        start_result == TitleLoopFrameResult::running;
         ++frame) {
        start_result = advance_title_loop_frame(loop, loop_callbacks);
    }
    const auto start_vblanks = loop_vblanks;
    const auto start_samples = loop_samples;
    if (!expect(start_result == TitleLoopFrameResult::start_requested &&
                loop.input_history.count == 1U &&
                loop.exit_palette_fade.active &&
                loop.exit_palette_fade.passes_completed == 0U &&
                loop.exit_palette_fade.vblanks_in_pass == 0U &&
                palette_steps == 0U && prompt_variants.size() == 1U)) {
        return 1;
    }

    const auto paused_logo_columns = loop.logo.columns_revealed;
    const auto paused_sprite_x = loop.sprite.x;
    const auto paused_sprite_phase = loop.sprite.phase;
    const auto paused_animation_offset = loop.animation_timer.animation_offset;
    const auto paused_prompt_frames = loop.prompt.frames_remaining;
    constexpr auto kTitleExitVblankCount = static_cast<std::uint8_t>(
        kTitleExitPalettePasses * kTitleExitVblanksPerPalettePass);
    for (std::uint8_t vblank = 1U; vblank <= kTitleExitVblankCount; ++vblank) {
        const auto result = advance_title_loop_frame(loop, loop_callbacks);
        const auto expected_passes = static_cast<std::uint8_t>(
            vblank / kTitleExitVblanksPerPalettePass);
        const bool final_vblank = vblank == kTitleExitVblankCount;
        const auto expected_vblanks_in_pass = static_cast<std::uint8_t>(
            vblank % kTitleExitVblanksPerPalettePass);
        if (!expect(result == (final_vblank
                                  ? TitleLoopFrameResult::
                                        palette_transition_complete
                                  : TitleLoopFrameResult::
                                        palette_transitioning) &&
                    loop_vblanks == start_vblanks + vblank &&
                    loop_samples == start_samples &&
                    palette_event_latches == vblank &&
                    palette_steps == expected_passes &&
                    loop.exit_palette_fade.passes_completed ==
                        expected_passes &&
                    loop.exit_palette_fade.vblanks_in_pass ==
                        expected_vblanks_in_pass &&
                    loop.logo.columns_revealed == paused_logo_columns &&
                    loop.sprite.x == paused_sprite_x &&
                    loop.sprite.phase == paused_sprite_phase &&
                    loop.animation_timer.animation_offset ==
                        paused_animation_offset &&
                    loop.prompt.frames_remaining == paused_prompt_frames)) {
            return 1;
        }
    }
    if (!expect(loop.exit_palette_fade.complete &&
                !loop.exit_palette_fade.active &&
                loop.input_history.count == 0U && palette_steps == 17U)) {
        return 1;
    }

    TitleLoopState sprite_lifetime;
    sprite_lifetime.logo_reveal_delay = 0;
    std::size_t upload_resets = 0;
    TitleLoopCallbacks sprite_callbacks{};
    sprite_callbacks.reset_vdp_upload_state = [&upload_resets] {
        ++upload_resets;
    };
    for (int frame = 0; frame < 75; ++frame) {
        advance_title_loop_frame(sprite_lifetime, sprite_callbacks);
    }
    if (!expect(sprite_lifetime.logo.columns_revealed ==
                    kTitleLogoColumnLimit &&
                sprite_lifetime.sprite.x == 0x1C4 &&
                sprite_lifetime.sprite.projection_visible)) {
        return 1;
    }
    advance_title_loop_frame(sprite_lifetime, sprite_callbacks);
    if (!expect(!sprite_lifetime.sprite.projection_visible &&
                upload_resets == 1U)) {
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
                completed_samples == 3U && completed_palette_steps == 0U &&
                completed_loop.prompt.finished &&
                completed_loop.exit_palette_fade.active)) {
        return 1;
    }
    for (std::uint8_t vblank = 0U; vblank < kTitleExitVblankCount; ++vblank) {
        advance_title_loop_frame(completed_loop, completed_callbacks);
    }
    if (!expect(completed_loop.exit_palette_fade.complete &&
                completed_loop.exit_palette_fade.passes_completed == 17U &&
                completed_samples == 3U && completed_palette_steps == 17U)) {
        return 1;
    }

    TitleModeEntryServiceState mode_entry;
    std::size_t mode_entry_vblanks = 0;
    std::size_t mode_entry_flushes = 0;
    std::size_t mode_entry_audio_ticks = 0;
    std::vector<std::uint8_t> mode_entry_commands;
    TitleModeEntryServiceCallbacks mode_entry_callbacks{};
    mode_entry_callbacks.wait_for_vblank = [&mode_entry_vblanks] {
        ++mode_entry_vblanks;
    };
    mode_entry_callbacks.run_audio_transaction =
        [&mode_entry_commands](const std::uint8_t command) {
            mode_entry_commands.push_back(command);
        };
    mode_entry_callbacks.flush_audio_bus = [&mode_entry_flushes] {
        ++mode_entry_flushes;
    };
    mode_entry_callbacks.service_audio_tick = [&mode_entry_audio_ticks] {
        ++mode_entry_audio_ticks;
    };
    begin_title_mode_entry_service(mode_entry, mode_entry_callbacks);
    if (!expect(mode_entry.active && !mode_entry.complete &&
                mode_entry.transactions_started == 1U &&
                mode_entry.vblanks_in_transaction == 0U &&
                mode_entry_commands.size() == 1U &&
                mode_entry_commands.front() == 0x14U &&
                mode_entry_flushes == 0U && mode_entry_audio_ticks == 0U)) {
        return 1;
    }
    for (std::uint8_t vblank = 1U; vblank <= 18U; ++vblank) {
        const auto result = advance_title_mode_entry_service_vblank(
            mode_entry, mode_entry_callbacks);
        const auto expected_transactions = static_cast<std::uint8_t>(
            std::min<unsigned>(
                1U + vblank / kTitleModeEntryVblanksPerTransaction,
                kTitleModeEntryAudioTransactions));
        const auto expected_vblanks_in_transaction = static_cast<std::uint8_t>(
            vblank % kTitleModeEntryVblanksPerTransaction);
        const bool final_vblank = vblank == 18U;
        if (!expect(result == (final_vblank
                                  ? TitleModeEntryServiceResult::complete
                                  : TitleModeEntryServiceResult::waiting) &&
                    mode_entry_vblanks == vblank &&
                    mode_entry.transactions_started == expected_transactions &&
                    mode_entry.vblanks_in_transaction ==
                        expected_vblanks_in_transaction &&
                    mode_entry_commands.size() == expected_transactions &&
                    mode_entry.active == !final_vblank &&
                    mode_entry.complete == final_vblank &&
                    mode_entry_flushes == (final_vblank ? 1U : 0U) &&
                    mode_entry_audio_ticks == (final_vblank ? 1U : 0U))) {
            return 1;
        }
    }
    for (const auto command : mode_entry_commands) {
        if (!expect(command == kTitleModeEntryAudioCommand)) {
            return 1;
        }
    }

    TitleIdleRestartFadeState idle_restart_fade;
    std::size_t idle_restart_vblanks = 0U;
    std::size_t idle_restart_latches = 0U;
    std::size_t idle_restart_palette_steps = 0U;
    TitleLoopCallbacks idle_restart_callbacks{};
    idle_restart_callbacks.wait_for_vblank = [&idle_restart_vblanks] {
        ++idle_restart_vblanks;
    };
    idle_restart_callbacks.latch_palette_transition_events =
        [&idle_restart_latches] { ++idle_restart_latches; };
    idle_restart_callbacks.step_palette_transition =
        [&idle_restart_palette_steps] { ++idle_restart_palette_steps; };
    begin_title_idle_restart_fade(idle_restart_fade);
    if (!expect(idle_restart_fade.active && !idle_restart_fade.complete &&
                idle_restart_fade.passes_completed == 0U &&
                idle_restart_fade.vblanks_in_pass == 0U)) {
        return 1;
    }
    constexpr auto kIdleRestartVblankCount = static_cast<std::uint8_t>(
        kTitleExitPalettePasses * kTitleExitVblanksPerPalettePass);
    for (std::uint8_t vblank = 1U; vblank <= kIdleRestartVblankCount;
         ++vblank) {
        const auto complete = advance_title_idle_restart_fade(
            idle_restart_fade, idle_restart_callbacks);
        const auto expected_passes = static_cast<std::uint8_t>(
            vblank / kTitleExitVblanksPerPalettePass);
        const bool final_vblank = vblank == kIdleRestartVblankCount;
        const auto expected_vblanks_in_pass = static_cast<std::uint8_t>(
            vblank % kTitleExitVblanksPerPalettePass);
        if (!expect(complete == final_vblank &&
                    idle_restart_vblanks == vblank &&
                    idle_restart_latches == vblank &&
                    idle_restart_palette_steps == expected_passes &&
                    idle_restart_fade.passes_completed == expected_passes &&
                    idle_restart_fade.vblanks_in_pass ==
                        expected_vblanks_in_pass &&
                    idle_restart_fade.active == !final_vblank &&
                    idle_restart_fade.complete == final_vblank)) {
            return 1;
        }
    }
    if (!expect(advance_title_idle_restart_fade(
                    idle_restart_fade, idle_restart_callbacks) &&
                idle_restart_vblanks == kIdleRestartVblankCount &&
                idle_restart_palette_steps == kTitleExitPalettePasses)) {
        return 1;
    }
    return 0;
}
