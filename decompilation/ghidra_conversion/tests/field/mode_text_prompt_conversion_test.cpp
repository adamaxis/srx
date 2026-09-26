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

    return 0;
}
