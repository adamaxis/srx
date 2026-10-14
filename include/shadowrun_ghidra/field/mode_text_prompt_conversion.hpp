#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

enum class ModeTextPromptTextRole {
    title,
    first_option,
    second_option,
    third_option,
};

struct ModeTextPromptOption {
    std::uint32_t text_key = 0;
    bool present = false;
};

// The source descriptor is four text entries followed by three command
// entries. The text keys are host-side identifiers; the original ROM pointers
// are provenance only and never enter this native conversion.
struct ModeTextPromptDescriptor {
    std::uint32_t title_text = 0;
    ModeTextPromptOption first_option{0, true};
    ModeTextPromptOption second_option{};
    ModeTextPromptOption third_option{};
    std::array<std::uint32_t, 3> commands{};
    bool first_option_is_exit_sentinel = false;
};

struct ModeTextPromptState {
    // ROM FFF10C: a nonzero marker means the caller already prepared the
    // prompt display. The prompt consumes the marker on entry.
    bool display_already_prepared = false;
    bool second_option_available = false;
    bool third_option_available = false;
    enum class Phase : std::uint8_t {
        not_started,
        awaiting_option,
        awaiting_any_event,
        terminal,
    } phase = Phase::not_started;
    std::uint16_t frames_remaining = 0;
    std::uint32_t terminal_command = 0;
    std::size_t dispatched_subcommands = 0;
};

struct ModeTextPromptCallbacks {
    // Covers ROM 0x1F5A8, which performs the two shared layout writes.
    std::function<void()> prepare_prompt_layout;
    std::function<void(ModeTextPromptTextRole, std::uint32_t)> render_text;
    std::function<void(ModeTextPromptTextRole)> write_option_grid;
    std::function<void(std::uint16_t)> wait_frames;
    // Covers the source vblank/latch/sample sequence and returns the
    // controller mask after the sampled event. A missing value represents a
    // host input boundary ending the conversion run, not a source branch.
    std::function<std::optional<std::uint8_t>()> poll_controller_mask;
    std::function<void()> wait_for_controller_event;
    // Commands below 0xFA are source dispatch-table entries and return to the
    // prompt after the handler. The handler remains an explicit boundary.
    std::function<void(std::uint32_t)> dispatch_subcommand;
};

enum class ModeTextPromptResultKind {
    terminal_command,
    negative_first_option,
    input_boundary_ended,
};

struct ModeTextPromptResult {
    ModeTextPromptResultKind kind =
        ModeTextPromptResultKind::input_boundary_ended;
    std::uint32_t command = 0;
    std::size_t dispatched_subcommands = 0;
};

enum class ModeTextPromptStepKind {
    waiting,
    subcommand_dispatched,
    terminal_command,
    negative_first_option,
};

struct ModeTextPromptStepResult {
    ModeTextPromptStepKind kind = ModeTextPromptStepKind::waiting;
    std::uint32_t command = 0;
};

// Host-friendly form of ROM 0x1F3C4. The first call draws the resolved prompt;
// later calls consume elapsed source frames and at most one sampled controller
// mask. A subcommand returns to the prompt loop after its handler completes.
ModeTextPromptStepResult advance_mode_text_prompt(
    ModeTextPromptState& state,
    const ModeTextPromptDescriptor& prompt,
    std::optional<std::uint8_t> controller_mask,
    std::uint16_t elapsed_frames,
    const ModeTextPromptCallbacks& callbacks);

// ROM 0x1F3C4: present a title and up to three options, wait ten frames,
// prioritize first/second/third controller choices, dispatch commands below
// 0xFA back through the mode table, and return terminal commands unchanged.
ModeTextPromptResult run_mode_text_prompt(
    ModeTextPromptState& state,
    const ModeTextPromptDescriptor& prompt,
    const ModeTextPromptCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
