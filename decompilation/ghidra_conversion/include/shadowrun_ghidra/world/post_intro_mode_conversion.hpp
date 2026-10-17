#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string_view>

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

// ROM 0x13726: FFF09C indexes 0xD445D, then 0xD4464 record * 0x1E. Five
// outline phases consume 0xD4536 (D1+3 by D2+3 tiles) at D4=0xC208, then
// 0xC0DA caption at 0xEA10 on the 0xE000 plane.
constexpr std::uint8_t kCabModeSelectionPhaseCount = 5U;
constexpr std::uint16_t kCabCursorNametableBase = 0xC208U;
constexpr std::uint16_t kCabCaptionVram = 0xEA10U;
constexpr std::uint16_t kCabCaptionPlaneBase = 0xE000U;
constexpr std::uint16_t kCabModeSelectionShrinkFrames = 10U;

struct CabModeSelectionPlan {
    std::uint8_t mode = 0;
    std::uint8_t record_index = 0;
    std::uint8_t phase = 0;
    std::uint16_t nametable_position = 0;
    int cursor_x = 0;
    int cursor_y = 0;
    int cursor_width = 0;
    int cursor_height = 0;
    std::string_view caption;
    int caption_x = 0;
    int caption_y = 0;
};

struct PostIntroCabTimeline {
    std::uint8_t mode = 4;
    std::uint8_t phase = 0;
    bool caption_visible = false;
};

// ROM 0x134D2 writes the 26x17 cab-map name table at VRAM $2208 of the
// $2000 plane (64-cell rows): row 4, column 4, i.e. plane pixel (32,32), with
// zero scroll. The longplay capture shows the 208x136 plate at screen
// (31..240, 32..168), so the host blits the full plate at (32,32).
inline constexpr int kCabMapPlateScreenX = 32;
inline constexpr int kCabMapPlateScreenY = 32;

CabModeSelectionPlan cab_mode_selection_plan(std::uint8_t mode,
                                             std::uint8_t phase);
PostIntroCabTimeline post_intro_cab_timeline(std::uint16_t frame);

// ROM 0x134D2: enter the post-intro mode selector and preserve its two input
// branches. Presentation, world updates, and mode transitions are callbacks;
// this module owns the source order and cursor/event semantics.
PostIntroModeResult run_post_intro_mode(
    PostIntroModeState& state,
    const PostIntroModeCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
