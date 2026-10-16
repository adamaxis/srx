#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::array<std::uint8_t, kPostIntroModeCount> kCabModeRecords{
    {0x06U, 0x04U, 0x02U, 0x05U, 0x00U, 0x03U, 0x01U}};

constexpr std::array<std::array<std::uint16_t, kCabModeSelectionPhaseCount>,
                     kPostIntroModeCount>
    kCabRecordPositions{{{{0x0000U, 0x0184U, 0x030AU, 0x0410U, 0x0514U}},
                         {{0x0000U, 0x0080U, 0x0180U, 0x0280U, 0x0380U}},
                         {{0x0000U, 0x010CU, 0x0216U, 0x0322U, 0x03AEU}},
                         {{0x0000U, 0x0182U, 0x0306U, 0x048AU, 0x058CU}},
                         {{0x0000U, 0x0108U, 0x0190U, 0x0216U, 0x031CU}},
                         {{0x0000U, 0x0188U, 0x0310U, 0x0416U, 0x051EU}},
                         {{0x0000U, 0x0086U, 0x010CU, 0x0190U, 0x0214U}}}};

constexpr std::array<std::uint8_t, kCabModeSelectionPhaseCount> kCabPhaseWidth{
    {0x17U, 0x11U, 0x0CU, 0x06U, 0x00U}};
constexpr std::array<std::uint8_t, kCabModeSelectionPhaseCount> kCabPhaseHeight{
    {0x0EU, 0x0BU, 0x08U, 0x05U, 0x00U}};

constexpr std::array<std::string_view, kPostIntroModeCount> kCabCaptions{{
    " Downtown Seattle ",
    " Puyallup Barrens ",
    "  Redmond Barrens ",
    " Penumbra District",
    "  Council Island  ",
    " Renraku Arcology ",
    "  Salish-Shidhe   ",
}};

} // namespace

CabModeSelectionPlan cab_mode_selection_plan(const std::uint8_t mode,
                                             const std::uint8_t phase) {
    CabModeSelectionPlan plan{};
    plan.mode = static_cast<std::uint8_t>(mode % kPostIntroModeCount);
    plan.phase = static_cast<std::uint8_t>(
        phase < kCabModeSelectionPhaseCount ? phase
                                            : kCabModeSelectionPhaseCount - 1U);
    plan.record_index = kCabModeRecords[plan.mode];
    plan.nametable_position =
        kCabRecordPositions[plan.record_index][plan.phase];
    const auto relative = static_cast<std::uint32_t>(kCabCursorNametableBase) +
                          plan.nametable_position - 0xC000U;
    plan.cursor_x = static_cast<int>(((relative % 0x80U) / 2U) * 8U);
    plan.cursor_y = static_cast<int>((relative / 0x80U) * 8U);
    plan.cursor_width =
        static_cast<int>(kCabPhaseWidth[plan.phase] + 3U) * 8;
    plan.cursor_height =
        static_cast<int>(kCabPhaseHeight[plan.phase] + 3U) * 8;
    plan.caption = kCabCaptions[plan.record_index];
    const auto caption_words =
        static_cast<int>((kCabCaptionVram - kCabCaptionPlaneBase) / 2U);
    plan.caption_x = (caption_words % 64) * 8;
    plan.caption_y = (caption_words / 64) * 8;
    return plan;
}

PostIntroCabTimeline post_intro_cab_timeline(const std::uint16_t frame) {
    // 0x13726: five outline phases, two vblanks each, then 0xC0DA. 0x134D2
    // holds mode 4 for DBF 0x40 and mode 2 for DBF 0x60 after each call.
    constexpr std::uint16_t kHoldFour = 0x41U;
    constexpr std::uint16_t kHoldTwo = 0x61U;
    const auto shrink = kCabModeSelectionShrinkFrames;
    PostIntroCabTimeline timeline{};
    if (frame < shrink) {
        timeline.mode = 4U;
        timeline.phase = static_cast<std::uint8_t>(frame / 2U);
        return timeline;
    }
    if (frame < shrink + kHoldFour) {
        timeline.mode = 4U;
        timeline.phase = 4U;
        timeline.caption_visible = true;
        return timeline;
    }
    const auto mode_two = static_cast<std::uint16_t>(frame - shrink - kHoldFour);
    if (mode_two < shrink) {
        timeline.mode = 2U;
        timeline.phase = static_cast<std::uint8_t>(mode_two / 2U);
        return timeline;
    }
    timeline.mode = 2U;
    timeline.phase = 4U;
    timeline.caption_visible = true;
    return timeline;
}

namespace {

void service_frame(PostIntroModeState& state,
                   const PostIntroModeCallbacks& callbacks) {
    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.latch_controller_events) {
        callbacks.latch_controller_events();
    }
    if (callbacks.sample_controller_input) {
        callbacks.sample_controller_input();
    }
    if (state.title_state == 0U && callbacks.service_world_state) {
        callbacks.service_world_state();
    }
}

void finalize_mode_transition(PostIntroModeState& state,
                              const PostIntroModeCallbacks& callbacks) {
    if (!state.menu_transition_pending &&
        state.selected_mode != state.current_mode) {
        if (callbacks.initialize_scene_mode_state) {
            callbacks.initialize_scene_mode_state();
        }
        if (callbacks.prepare_mode_entry) {
            callbacks.prepare_mode_entry();
        }
    }
}

} // namespace

PostIntroModeResult run_post_intro_mode(
    PostIntroModeState& state,
    const PostIntroModeCallbacks& callbacks) {
    if (callbacks.resolve_initial_mode) {
        state.selected_mode = callbacks.resolve_initial_mode(state.mode_family);
    }
    if (callbacks.decode_and_upload_source_stream) {
        callbacks.decode_and_upload_source_stream();
    }
    if (callbacks.render_hud_sprite_stream) {
        callbacks.render_hud_sprite_stream();
    }
    service_frame(state, callbacks);

    if (state.alternate_branch) {
        state.selected_mode = 4U;
        if (callbacks.render_mode_selection) {
            callbacks.render_mode_selection(state.selected_mode);
        }
        for (state.alternate_first_frame = 0;
             state.alternate_first_frame <= 0x0040U;
             ++state.alternate_first_frame) {
            service_frame(state, callbacks);
            if (state.stop_marker != 0U) {
                state.title_state = 0;
                return PostIntroModeResult::external_stop;
            }
        }

        state.selected_mode = 2U;
        if (callbacks.render_mode_selection) {
            callbacks.render_mode_selection(state.selected_mode);
        }
        for (state.alternate_second_frame = 0;
             state.alternate_second_frame <= 0x0060U;
             ++state.alternate_second_frame) {
            service_frame(state, callbacks);
            if (state.stop_marker != 0U) {
                state.title_state = 0;
                return PostIntroModeResult::external_stop;
            }
        }
        state.title_state = 0;
        return PostIntroModeResult::alternate_branch_complete;
    }

    if (callbacks.render_mode_selection) {
        callbacks.render_mode_selection(state.selected_mode);
    }
    if (!callbacks.read_controller_events) {
        return PostIntroModeResult::external_stop;
    }

    for (;;) {
        service_frame(state, callbacks);
        const auto events = callbacks.read_controller_events();
        if ((events & kPostIntroLeftEvents) != 0U) {
            if (state.selected_mode == 0U) {
                state.selected_mode = static_cast<std::uint8_t>(
                    kPostIntroModeCount - 1U);
            } else {
                --state.selected_mode;
            }
            if (callbacks.render_mode_selection) {
                callbacks.render_mode_selection(state.selected_mode);
            }
        }
        if ((events & kPostIntroRightEvents) != 0U) {
            ++state.selected_mode;
            if (state.selected_mode >= kPostIntroModeCount) {
                state.selected_mode = 0U;
            }
            if (callbacks.render_mode_selection) {
                callbacks.render_mode_selection(state.selected_mode);
            }
        }
        if ((events & kPostIntroExitEvents) != 0U) {
            if (callbacks.handle_exit_event) {
                callbacks.handle_exit_event();
            }
            state.exit_marker_written = true;
            if (callbacks.finalize_mode_exit) {
                callbacks.finalize_mode_exit();
            }
            finalize_mode_transition(state, callbacks);
            return PostIntroModeResult::returned_after_exit;
        }
        if ((events & kPostIntroActionEvent) != 0U &&
            callbacks.handle_action_event) {
            callbacks.handle_action_event();
        }
        if ((events & kPostIntroTransferEvent) != 0U) {
            if (callbacks.transfer_menu_asset_batch) {
                callbacks.transfer_menu_asset_batch();
            }
            finalize_mode_transition(state, callbacks);
            return PostIntroModeResult::returned_after_transfer;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
