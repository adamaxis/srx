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

} // namespace

ModeTextPromptResult run_mode_text_prompt(
    ModeTextPromptState& state,
    const ModeTextPromptDescriptor& prompt,
    const ModeTextPromptCallbacks& callbacks) {
    ModeTextPromptResult result{};

    for (;;) {
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
            if (callbacks.wait_frames) {
                callbacks.wait_frames(30U);
            }
            if (callbacks.wait_for_controller_event) {
                callbacks.wait_for_controller_event();
            }
            result.kind = ModeTextPromptResultKind::negative_first_option;
            result.command = kNegativeFirstOptionResult;
            return result;
        }

        render_option(prompt.first_option,
                      ModeTextPromptTextRole::first_option, callbacks);

        state.second_option_available = prompt.second_option.present;
        render_option(prompt.second_option,
                      ModeTextPromptTextRole::second_option, callbacks);

        state.third_option_available = prompt.third_option.present;
        render_option(prompt.third_option,
                      ModeTextPromptTextRole::third_option, callbacks);

        if (callbacks.wait_frames) {
            callbacks.wait_frames(10U);
        }

        for (;;) {
            if (!callbacks.poll_controller_mask) {
                result.kind = ModeTextPromptResultKind::input_boundary_ended;
                return result;
            }
            const auto mask = callbacks.poll_controller_mask();
            if (!mask.has_value()) {
                result.kind = ModeTextPromptResultKind::input_boundary_ended;
                return result;
            }

            std::size_t selected_option = 0;
            if ((*mask & kFirstOptionMask) != 0U) {
                selected_option = 0U;
            } else if ((*mask & kSecondOptionMask) != 0U &&
                       state.second_option_available) {
                selected_option = 1U;
            } else if ((*mask & kThirdOptionMask) != 0U &&
                       state.third_option_available) {
                selected_option = 2U;
            } else {
                continue;
            }

            const auto command = prompt.commands[selected_option];
            if (command >= kFirstTerminalCommand) {
                result.kind = ModeTextPromptResultKind::terminal_command;
                result.command = command;
                return result;
            }

            if (callbacks.dispatch_subcommand) {
                callbacks.dispatch_subcommand(command);
            }
            ++result.dispatched_subcommands;
            break;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
