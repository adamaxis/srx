#include "shadowrun_ghidra/field/mode_text_prompt_conversion.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeTextPromptState state{};
    ModeTextPromptDescriptor prompt{
        10U,
        {11U, true},
        {12U, true},
        {13U, false},
        {0x00FAU, 0x0020U, 0x0000U},
        false};
    std::vector<std::uint32_t> rendered;
    std::vector<ModeTextPromptTextRole> grids;
    std::vector<std::uint16_t> waits;
    std::vector<std::uint32_t> dispatched;
    std::vector<std::optional<std::uint8_t>> inputs{
        std::optional<std::uint8_t>{0x10U},
        std::optional<std::uint8_t>{0x40U}};
    int preparations = 0;

    const auto result = run_mode_text_prompt(
        state, prompt,
        { [&] { ++preparations; },
          [&](const ModeTextPromptTextRole, const std::uint32_t text_key) {
              rendered.push_back(text_key);
          },
          [&](const ModeTextPromptTextRole role) { grids.push_back(role); },
          [&](const std::uint16_t frame_count) { waits.push_back(frame_count); },
          [&]() -> std::optional<std::uint8_t> {
              if (inputs.empty()) {
                  return std::nullopt;
              }
              const auto input = inputs.front();
              inputs.erase(inputs.begin());
              return input;
          },
          {},
          [&](const std::uint32_t command) {
              dispatched.push_back(command);
              // A source command handler may leave the display prepared; the
              // next prompt entry then consumes FFF10C and skips the title.
              state.display_already_prepared = true;
          } });
    if (expect(result.kind == ModeTextPromptResultKind::terminal_command &&
               result.command == 0x00FAU &&
               result.dispatched_subcommands == 1U &&
               dispatched == std::vector<std::uint32_t>{0x0020U} &&
               preparations == 1 &&
               rendered == std::vector<std::uint32_t>{10U, 11U, 12U, 11U, 12U} &&
               grids.size() == 4U && waits == std::vector<std::uint16_t>{10U, 10U}) !=
            0) {
        return 1;
    }

    ModeTextPromptState reused_state{};
    reused_state.display_already_prepared = true;
    ModeTextPromptDescriptor reused_prompt{
        20U,
        {21U, true},
        {0U, false},
        {0U, false},
        {0x00FAU, 0U, 0U},
        false};
    int reused_preparations = 0;
    int reused_title_renders = 0;
    auto reused_inputs = std::vector<std::optional<std::uint8_t>>{
        std::optional<std::uint8_t>{0x40U}};
    const auto reused_result = run_mode_text_prompt(
        reused_state, reused_prompt,
        { [&] { ++reused_preparations; },
          [&](const ModeTextPromptTextRole role, const std::uint32_t) {
              if (role == ModeTextPromptTextRole::title) {
                  ++reused_title_renders;
              }
          },
          {},
          {},
          [&]() -> std::optional<std::uint8_t> {
              if (reused_inputs.empty()) {
                  return std::nullopt;
              }
              const auto input = reused_inputs.front();
              reused_inputs.erase(reused_inputs.begin());
              return input;
          },
          {},
          {} });
    if (expect(reused_result.kind == ModeTextPromptResultKind::terminal_command &&
               reused_result.command == 0x00FAU && reused_preparations == 0 &&
               reused_title_renders == 0 && !reused_state.display_already_prepared) !=
            0) {
        return 2;
    }

    ModeTextPromptState sentinel_state{};
    ModeTextPromptDescriptor sentinel_prompt{
        30U, {31U, true}, {}, {}, {0U, 0U, 0U}, true};
    std::vector<std::uint16_t> sentinel_waits;
    bool waited_for_event = false;
    const auto sentinel_result = run_mode_text_prompt(
        sentinel_state, sentinel_prompt,
        { {},
          {},
          {},
          [&](const std::uint16_t frame_count) { sentinel_waits.push_back(frame_count); },
          {},
          [&] { waited_for_event = true; },
          {} });
    if (expect(sentinel_result.kind ==
                   ModeTextPromptResultKind::negative_first_option &&
               sentinel_result.command == 0x00FBU && waited_for_event &&
               sentinel_waits == std::vector<std::uint16_t>{30U}) != 0) {
        return 3;
    }

    ModeTextPromptState stepped_state{};
    const ModeTextPromptDescriptor stepped_prompt{
        40U,
        {41U, true},
        {42U, true},
        {0U, false},
        {0x00FAU, 0x0020U, 0U},
        false};
    std::vector<std::uint32_t> stepped_rendered;
    std::vector<std::uint32_t> stepped_dispatches;
    int stepped_preparations = 0;
    const ModeTextPromptCallbacks stepped_callbacks{
        [&] { ++stepped_preparations; },
        [&](const ModeTextPromptTextRole, const std::uint32_t text_key) {
            stepped_rendered.push_back(text_key);
        },
        {},
        {},
        {},
        {},
        [&](const std::uint32_t command) {
            stepped_dispatches.push_back(command);
            stepped_state.display_already_prepared = true;
        }};

    const auto entered = advance_mode_text_prompt(
        stepped_state, stepped_prompt, std::nullopt, 0U,
        stepped_callbacks);
    const auto entered_frames_remaining = stepped_state.frames_remaining;
    const auto early_choice = advance_mode_text_prompt(
        stepped_state, stepped_prompt, std::uint8_t{0x10U}, 9U,
        stepped_callbacks);
    const auto early_frames_remaining = stepped_state.frames_remaining;
    const auto dispatched_step = advance_mode_text_prompt(
        stepped_state, stepped_prompt, std::uint8_t{0x10U}, 1U,
        stepped_callbacks);
    const auto dispatched_phase = stepped_state.phase;
    const auto dispatched_frames_remaining = stepped_state.frames_remaining;
    const auto dispatched_count = stepped_state.dispatched_subcommands;
    const auto selected_first = advance_mode_text_prompt(
        stepped_state, stepped_prompt, std::uint8_t{0x50U}, 10U,
        stepped_callbacks);
    if (expect(entered.kind == ModeTextPromptStepKind::waiting &&
               entered_frames_remaining == 10U &&
               early_choice.kind == ModeTextPromptStepKind::waiting &&
               early_frames_remaining == 1U &&
               dispatched_step.kind ==
                   ModeTextPromptStepKind::subcommand_dispatched &&
               dispatched_step.command == 0x0020U &&
               dispatched_phase ==
                   ModeTextPromptState::Phase::awaiting_option &&
               dispatched_frames_remaining == 10U &&
               dispatched_count == 1U &&
               selected_first.kind ==
                   ModeTextPromptStepKind::terminal_command &&
               selected_first.command == 0x00FAU &&
               stepped_preparations == 1 &&
               stepped_dispatches == std::vector<std::uint32_t>{0x0020U} &&
               stepped_rendered ==
                   std::vector<std::uint32_t>{40U, 41U, 42U, 41U, 42U}) != 0) {
        return 4;
    }

    ModeTextPromptState stepped_sentinel_state{};
    const auto sentinel_entered = advance_mode_text_prompt(
        stepped_sentinel_state, sentinel_prompt, std::nullopt, 0U, {});
    const auto sentinel_initial_delay = stepped_sentinel_state.frames_remaining;
    const auto early_sentinel_event = advance_mode_text_prompt(
        stepped_sentinel_state, sentinel_prompt, std::uint8_t{0x01U}, 29U,
        {});
    const auto sentinel_final_delay = stepped_sentinel_state.frames_remaining;
    const auto sentinel_completed = advance_mode_text_prompt(
        stepped_sentinel_state, sentinel_prompt, std::uint8_t{0x01U}, 1U,
        {});
    if (expect(sentinel_entered.kind == ModeTextPromptStepKind::waiting &&
               sentinel_initial_delay == 30U &&
               early_sentinel_event.kind == ModeTextPromptStepKind::waiting &&
               sentinel_final_delay == 1U &&
               sentinel_completed.kind ==
                   ModeTextPromptStepKind::negative_first_option &&
               sentinel_completed.command == 0x00FBU) != 0) {
        return 5;
    }

    return 0;
}
