#include "shadowrun_ghidra/field/mode_text_prompt_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::uint8_t kFirstOptionMask = 0x40U;
constexpr std::uint8_t kSecondOptionMask = 0x10U;
constexpr std::uint8_t kThirdOptionMask = 0x20U;
constexpr std::uint32_t kFirstTerminalCommand = 0x00FAU;
constexpr std::uint32_t kNegativeFirstOptionResult = 0x00FBU;

void render_option(const ModeTextPromptOption& option,
                   const ModeTextPromptTextRole role,
                   const ModeTextPromptCallbacks& callbacks) {
    if (!option.present) {
        return;
    }
    if (callbacks.render_text) {
        callbacks.render_text(role, option.text_key);
    }
    if (callbacks.write_option_grid) {
        callbacks.write_option_grid(role);
    }
}

void enter_prompt(ModeTextPromptState& state,
                  const ModeTextPromptDescriptor& prompt,
                  const ModeTextPromptCallbacks& callbacks) {
    const bool reuse_prepared_display = state.display_already_prepared;
    state.display_already_prepared = false;

    if (!reuse_prepared_display) {
        if (callbacks.prepare_prompt_layout) {
            callbacks.prepare_prompt_layout();
        }
        if (callbacks.render_text) {
            callbacks.render_text(ModeTextPromptTextRole::title,
                                  prompt.title_text);
        }
    }

    if (prompt.first_option_is_exit_sentinel) {
        state.phase = ModeTextPromptState::Phase::awaiting_any_event;
        state.frames_remaining = 30U;
        return;
    }

    render_option(prompt.first_option,
                  ModeTextPromptTextRole::first_option, callbacks);
    state.second_option_available = prompt.second_option.present;
    render_option(prompt.second_option,
                  ModeTextPromptTextRole::second_option, callbacks);
    state.third_option_available = prompt.third_option.present;
    render_option(prompt.third_option,
                  ModeTextPromptTextRole::third_option, callbacks);
    state.phase = ModeTextPromptState::Phase::awaiting_option;
    state.frames_remaining = 10U;
}

} // namespace

ModeTextPromptStepResult advance_mode_text_prompt(
    ModeTextPromptState& state,
    const ModeTextPromptDescriptor& prompt,
    const std::optional<std::uint8_t> controller_mask,
    const std::uint16_t elapsed_frames,
    const ModeTextPromptCallbacks& callbacks) {
    if (state.phase == ModeTextPromptState::Phase::terminal) {
        return {state.terminal_command == kNegativeFirstOptionResult &&
                        prompt.first_option_is_exit_sentinel
                    ? ModeTextPromptStepKind::negative_first_option
                    : ModeTextPromptStepKind::terminal_command,
                state.terminal_command};
    }
    if (state.phase == ModeTextPromptState::Phase::not_started) {
        enter_prompt(state, prompt, callbacks);
        return {};
    }

    state.frames_remaining = elapsed_frames >= state.frames_remaining
                                 ? 0U
                                 : static_cast<std::uint16_t>(
                                       state.frames_remaining - elapsed_frames);
    if (state.frames_remaining != 0U || !controller_mask.has_value() ||
        *controller_mask == 0U) {
        return {};
    }

    if (state.phase == ModeTextPromptState::Phase::awaiting_any_event) {
        state.phase = ModeTextPromptState::Phase::terminal;
        state.terminal_command = kNegativeFirstOptionResult;
        return {ModeTextPromptStepKind::negative_first_option,
                state.terminal_command};
    }

    std::size_t selected_option = 0U;
    if ((*controller_mask & kFirstOptionMask) != 0U) {
        selected_option = 0U;
    } else if ((*controller_mask & kSecondOptionMask) != 0U &&
               state.second_option_available) {
        selected_option = 1U;
    } else if ((*controller_mask & kThirdOptionMask) != 0U &&
               state.third_option_available) {
        selected_option = 2U;
    } else {
        return {};
    }

    const auto command = prompt.commands[selected_option];
    if (command >= kFirstTerminalCommand) {
        state.phase = ModeTextPromptState::Phase::terminal;
        state.terminal_command = command;
        return {ModeTextPromptStepKind::terminal_command, command};
    }

    if (callbacks.dispatch_subcommand) {
        callbacks.dispatch_subcommand(command);
    }
    ++state.dispatched_subcommands;
    state.phase = ModeTextPromptState::Phase::not_started;
    enter_prompt(state, prompt, callbacks);
    return {ModeTextPromptStepKind::subcommand_dispatched, command};
}

ModeTextPromptResult run_mode_text_prompt(
    ModeTextPromptState& state,
    const ModeTextPromptDescriptor& prompt,
    const ModeTextPromptCallbacks& callbacks) {
    ModeTextPromptResult result{};
    const auto prior_dispatch_count = state.dispatched_subcommands;

    // This wrapper preserves the blocking source contract for existing
    // callers. The Qt host uses advance_mode_text_prompt() once per frame.
    for (;;) {
        if (state.phase == ModeTextPromptState::Phase::not_started) {
            (void)advance_mode_text_prompt(state, prompt, std::nullopt, 0U,
                                           callbacks);
        }

        if (state.phase == ModeTextPromptState::Phase::awaiting_any_event) {
            if (callbacks.wait_frames) {
                callbacks.wait_frames(state.frames_remaining);
            }
            if (callbacks.wait_for_controller_event) {
                callbacks.wait_for_controller_event();
            }
            const auto step = advance_mode_text_prompt(
                state, prompt, std::uint8_t{0xFFU}, state.frames_remaining,
                callbacks);
            result.kind = step.kind ==
                                  ModeTextPromptStepKind::negative_first_option
                              ? ModeTextPromptResultKind::negative_first_option
                              : ModeTextPromptResultKind::terminal_command;
            result.command = step.command;
            break;
        }

        if (state.frames_remaining != 0U && callbacks.wait_frames) {
            callbacks.wait_frames(state.frames_remaining);
        }
        bool first_sample = true;
        for (;;) {
            if (!callbacks.poll_controller_mask) {
                result.kind = ModeTextPromptResultKind::input_boundary_ended;
                result.dispatched_subcommands =
                    state.dispatched_subcommands - prior_dispatch_count;
                return result;
            }
            const auto mask = callbacks.poll_controller_mask();
            if (!mask.has_value()) {
                result.kind = ModeTextPromptResultKind::input_boundary_ended;
                result.dispatched_subcommands =
                    state.dispatched_subcommands - prior_dispatch_count;
                return result;
            }
            const auto elapsed =
                first_sample ? state.frames_remaining
                              : static_cast<std::uint16_t>(1U);
            first_sample = false;
            const auto step = advance_mode_text_prompt(
                state, prompt, mask, elapsed, callbacks);
            if (step.kind == ModeTextPromptStepKind::waiting) {
                continue;
            }
            if (step.kind == ModeTextPromptStepKind::subcommand_dispatched) {
                break;
            }
            result.kind = ModeTextPromptResultKind::terminal_command;
            result.command = step.command;
            result.dispatched_subcommands =
                state.dispatched_subcommands - prior_dispatch_count;
            return result;
        }
    }

    result.dispatched_subcommands =
        state.dispatched_subcommands - prior_dispatch_count;
    return result;
}

} // namespace shadowrun::ghidra_conversion
