#include "shadowrun_ghidra/field/mode_action_loop_conversion.hpp"

#include "shadowrun_ghidra/core/selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

bool source_subtract_byte_is_nonpositive(const std::uint8_t minuend,
                                         const std::uint8_t subtrahend,
                                         const std::uint8_t result) {
    // 0x54662 performs SUB.B and branches with BLE. Preserve the source
    // N/Z/V decision instead of treating the wrapped byte as a C++ integer.
    const bool negative = (result & 0x80U) != 0U;
    const bool zero = result == 0U;
    const bool overflow =
        (((minuend ^ subtrahend) & 0x80U) != 0U) &&
        (((minuend ^ result) & 0x80U) != 0U);
    return zero || (negative != overflow);
}

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

            const auto action_edges =
                static_cast<std::uint16_t>(ModeActionInput::confirm) |
                static_cast<std::uint16_t>(ModeActionInput::alternate_confirm) |
                static_cast<std::uint16_t>(ModeActionInput::mode_action);
            if ((input & action_edges) != 0U) {
                const auto gate = callbacks.calculate_mode_action_gate_inputs
                                      ? callbacks.calculate_mode_action_gate_inputs()
                                      : ModeActionGateInputs{};
                auto trial_count = gate.calculated_runner_action_delay;
                const auto active_delay = static_cast<std::uint16_t>(
                    gate.active_runner_action_delay);
                // 0x548BE..0x548C6: CMP.W D6,D7 / BPL, then ADD.W D7,D6.
                if (active_delay >= trial_count) {
                    trial_count = active_delay;
                }
                trial_count = static_cast<std::uint16_t>(
                    trial_count + active_delay);
                if (trial_count == 0U) {
                    trial_count = 1U;
                }

                // The compact +0x05 byte is decremented first, then SUB.B
                // changes only D6's low byte. BLE observes the byte flags;
                // the resulting full word is the 0x0DBA trial count.
                const auto compact_gate = static_cast<std::uint8_t>(
                    record.action_gate_value - 1U);
                const auto delay_low = static_cast<std::uint8_t>(trial_count);
                const auto gated_low = static_cast<std::uint8_t>(
                    delay_low - compact_gate);
                trial_count = static_cast<std::uint16_t>(
                    (trial_count & 0xFF00U) | gated_low);
                if (source_subtract_byte_is_nonpositive(
                        delay_low, compact_gate, gated_low)) {
                    call_if_present(callbacks.write_repeated_grid);
                    call_if_present(callbacks.render_encoded_text);
                    call_if_present(callbacks.wait_for_controller_event);
                } else {
                    const auto thresholded_rolls =
                        callbacks.count_mode_action_rolls
                            ? callbacks.count_mode_action_rolls(
                                  trial_count, record.action_difficulty)
                            : std::int8_t{0};
                    if (thresholded_rolls > 0) {
                        call_if_present(callbacks.dispatch_mode_action,
                                        record.kind, state.cursor);
                    } else {
                        call_if_present(callbacks.write_repeated_grid);
                        call_if_present(callbacks.render_encoded_text);
                        call_if_present(callbacks.wait_for_controller_event);
                    }
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
