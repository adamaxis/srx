#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::uint8_t kPostIntroLeftEvents = 0x09U;
constexpr std::uint8_t kPostIntroRightEvents = 0x06U;
constexpr std::uint8_t kPostIntroExitEvents = 0x50U;
constexpr std::uint8_t kPostIntroActionEvent = 0x20U;
constexpr std::uint8_t kPostIntroTransferEvent = 0x80U;
constexpr std::uint8_t kPostIntroModeCount = 7U;

struct PostIntroModeState {
    std::uint8_t mode_family = 0;
    std::uint8_t selected_mode = 0;
    std::uint8_t current_mode = 0;
    std::uint8_t title_state = 0;
    bool alternate_branch = false;
    bool menu_transition_pending = false;
    std::uint16_t stop_marker = 0;
    std::uint16_t alternate_first_frame = 0;
    std::uint16_t alternate_second_frame = 0;
    bool exit_marker_written = false;
};

struct PostIntroModeCallbacks {
    std::function<std::uint8_t(std::uint8_t)> resolve_initial_mode;
    std::function<void()> decode_and_upload_source_stream;
    std::function<void()> render_hud_sprite_stream;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> sample_controller_input;
    std::function<std::uint8_t()> read_controller_events;
    std::function<void()> service_world_state;
    std::function<void(std::uint8_t)> render_mode_selection;
    std::function<void()> handle_action_event;
    std::function<void()> transfer_menu_asset_batch;
    std::function<void()> handle_exit_event;
    std::function<void()> finalize_mode_exit;
    std::function<void()> initialize_scene_mode_state;
    std::function<void()> prepare_mode_entry;
};

enum class PostIntroModeResult {
    alternate_branch_complete,
    returned_after_exit,
    returned_after_transfer,
    external_stop,
};

// ROM 0x134D2: enter the post-intro mode selector and preserve its two input
// branches. Presentation, world updates, and mode transitions are callbacks;
// this module owns the source order and cursor/event semantics.
PostIntroModeResult run_post_intro_mode(
    PostIntroModeState& state,
    const PostIntroModeCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
