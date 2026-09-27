#include "shadowrun_ghidra/field/mode_action_loop_conversion.hpp"

#include "shadowrun_ghidra/core/selection_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

} // namespace

void fill_mode_auxiliary_sentinel(std::vector<std::int32_t>& values) {
    values.assign(35U, -1);
}

void write_mode_repeated_grid_service(const std::function<void()>& write_grid) {
    call_if_present(write_grid);
}

void select_mode_display_status(ModeDisplayStatusState& state,
                                const std::int8_t display_mode) {
    if (display_mode == 0) {
        state.status_code = 0xCCU;
    } else if (display_mode < 0) {
        state.status_code = 0xC0U;
    } else {
        state.status_code = 0x0CU;
    }
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x01U);
}

void render_mode_status_summary(
    const std::function<void()>& render_encoded_text,
    const std::function<void()>& build_grouped_decimal_glyphs) {
    // 0x54A28 calls C0DA twice before C216.  These are two distinct source
    // fields followed by the grouped decimal presentation, not one bitmap.
    call_if_present(render_encoded_text);
    call_if_present(render_encoded_text);
    call_if_present(build_grouped_decimal_glyphs);
}

bool advance_random_mode_event(
    ModeActionState& state,
    const RandomModeEventCallbacks& callbacks) {
    const auto random_bound = callbacks.sample_random_bound
                                  ? callbacks.sample_random_bound()
                                  : std::int8_t{0};
    if (state.mode_random_count <= random_bound) {
        return false;
    }

    // 0x54A8E clears the counter before changing the mode/HUD state.
    state.mode_random_count = 0;
    call_if_present(callbacks.transition_hud_mode);
    if (state.hud_abort == 0) {
        call_if_present(callbacks.select_display_status);
        call_if_present(callbacks.render_encoded_text);
        call_if_present(callbacks.wait_for_vblank_frame);
        call_if_present(callbacks.wait_for_controller_event);
    }
    call_if_present(callbacks.write_repeated_grid);
    return true;
}

bool run_mode_action_loop(ModeActionState& state,
                          const ModeActionRecord& record,
                          const ModeActionLoopCallbacks& callbacks) {
    // 0x54662 entry: clear the mode-owned low flags, publish bit 6 and merge
    // the selected compact record's flags into the scene state.
    state.display_result = -1;
    state.mode_work_marker = 0;
    state.mode_flags = static_cast<std::uint8_t>(state.mode_flags & 0x0FU);
    state.mode_flags = static_cast<std::uint8_t>(state.mode_flags | 0x40U);
    state.mode_flags = static_cast<std::uint8_t>(state.mode_flags | record.flags);
    if (record.clears_auxiliary_menu_state) {
        // The old export called this an audio sentinel.  Its only proven
        // behavior here is clearing the selected mode's auxiliary buffer, so
        // expose that behavior through the host callback rather than keeping
        // the misleading subsystem name in this conversion.
        call_if_present(callbacks.clear_mode_auxiliary_state);
    }
    state.auxiliary_marker = -1;
    state.mode_random_count = 0;
    call_if_present(callbacks.set_display_control, 0x8B00U);

    for (;;) {
        call_if_present(callbacks.set_display_control, 0x8134U);
        call_if_present(callbacks.reset_display_buffers);
        call_if_present(callbacks.refresh_resource_state);
        call_if_present(callbacks.write_wrapped_resource_block);
        call_if_present(callbacks.write_wrapped_resource_block);
        // The source executes six grid writes in the first loop and four more
        // immediately after it: ten writes per outer redraw.
        for (std::size_t index = 0; index < 10U; ++index) {
            call_if_present(callbacks.write_grid);
        }
        call_if_present(callbacks.select_display_status);
        call_if_present(callbacks.refresh_mode_metadata);
        call_if_present(callbacks.set_display_control, 0x8174U);
        if (callbacks.advance_random_mode_event) {
            callbacks.advance_random_mode_event(state);
        }
        // 0x547D8 tests FFF0B1 after 0x54A8E returns.  FFF0B7 is the menu
        // result written by the entry and Matrix paths, not this exit gate.
        if (state.hud_abort < 0) {
            return true;
        }

        call_if_present(callbacks.render_encoded_text);
        call_if_present(callbacks.render_encoded_text);
        // The helper is a separate converted source function.  Its callback
        // is invoked once here after the two direct C0DA calls, matching the
        // call site at 0x54828.
        call_if_present(callbacks.render_mode_status_summary);
        state.cursor = 0;

        const auto publish_cursor = [&](const std::int8_t direction) {
            state.cursor = static_cast<std::int8_t>(move_wrapped_menu_cursor(
                static_cast<std::uint8_t>(state.cursor), direction,
                static_cast<std::uint8_t>(record.cursor_limit)));
            if (callbacks.move_menu_cursor) {
                callbacks.move_menu_cursor(direction, state.cursor);
            }
        };

        // 0x5482C clears the cursor and 0x5488C performs the initial C376
        // draw with D2=0 before the first controller wait.
        publish_cursor(0);

        for (;;) {
            call_if_present(callbacks.wait_for_vblank);
            call_if_present(callbacks.latch_controller_events);
            const auto input = callbacks.sample_controller_input
                                   ? callbacks.sample_controller_input()
                                   : std::uint16_t{0};

            std::int8_t direction = 0;
            if ((input & static_cast<std::uint16_t>(ModeActionInput::move_previous)) != 0U) {
                direction = -1;
            } else if ((input & static_cast<std::uint16_t>(ModeActionInput::move_next)) != 0U) {
                direction = 1;
            }
            if (direction != 0) {
                publish_cursor(direction);
                continue;
            }

            if ((input & static_cast<std::uint16_t>(ModeActionInput::action)) != 0U) {
                call_if_present(callbacks.run_action_preamble);
                const auto gate = callbacks.read_action_gate_inputs
                                      ? callbacks.read_action_gate_inputs()
                                      : ModeActionGateInputs{};
                const auto normalized_action_bound =
                    std::max(1, static_cast<int>(gate.action_bound) - 1);
                const auto record_option_limit = static_cast<int>(record.option_count) - 1;
                if (normalized_action_bound <= record_option_limit ||
                    gate.thresholded_random_rolls < 1) {
                    call_if_present(callbacks.write_repeated_grid);
                    call_if_present(callbacks.render_encoded_text);
                    call_if_present(callbacks.wait_for_controller_event);
                } else {
                    call_if_present(callbacks.dispatch_mode_action,
                                    record.kind, state.cursor);
                }
                state.mode_random_count = static_cast<std::uint8_t>(
                    state.mode_random_count + 1U);
                break;
            }

            if ((input & static_cast<std::uint16_t>(ModeActionInput::leave_to_matrix)) != 0U) {
                call_if_present(callbacks.set_display_control, 0x8134U);
                state.display_result = 0;
                call_if_present(callbacks.render_menu_footer);
                call_if_present(callbacks.upload_menu_resource_pair);
                call_if_present(callbacks.set_display_control, 0x8174U);
                const bool matrix_abort = callbacks.enter_matrix_mode
                                               ? callbacks.enter_matrix_mode()
                                               : false;
                for (std::size_t slot = 0; slot < 5U; ++slot) {
                    call_if_present(callbacks.reset_matrix_slot_timer, slot);
                }
                if (matrix_abort) {
                    state.hud_abort = -1;
                }
                break;
            }
        }
    }
}

} // namespace shadowrun::ghidra_conversion
