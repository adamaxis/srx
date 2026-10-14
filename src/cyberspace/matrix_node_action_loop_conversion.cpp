#include "shadowrun_ghidra/cyberspace/matrix_node_action_loop_conversion.hpp"

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

void fill_matrix_node_auxiliary_sentinel(std::vector<std::int32_t>& values) {
    values.assign(35U, -1);
}

void redraw_matrix_node_action_grid_service(
    const std::function<void()>& write_matrix_node_grid) {
    call_if_present(write_matrix_node_grid);
}

void select_matrix_alert_display(MatrixAlertDisplayState& state,
                                 const std::int8_t alert_state) {
    if (alert_state == 0) {
        state.status_code = 0xCCU;
    } else if (alert_state < 0) {
        state.status_code = 0xC0U;
    } else {
        state.status_code = 0x0CU;
    }
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x01U);
}

void render_matrix_node_status_summary(
    const MatrixNodeStatusSummary& summary,
    const std::function<void(const MatrixNodeTextRequest&)>& render_matrix_text,
    const std::function<void(const MatrixNodeNumberRequest&)>& render_matrix_number) {
    // 0x54A28 calls C0DA for the alert-band row, then for the file/node
    // classification, and finally C216 for the action difficulty. These are
    // three distinct Matrix status fields, not one generic text/bitmap pass.
    call_if_present(render_matrix_text,
                    MatrixNodeTextRequest{MatrixNodeTextRole::alert_band_label,
                                          MatrixNodeFunction::invalid,
                                          summary.alert_band,
                                          MatrixFileCategory::invalid});
    call_if_present(render_matrix_text,
                    MatrixNodeTextRequest{MatrixNodeTextRole::file_category_label,
                                          MatrixNodeFunction::invalid,
                                          MatrixAlertBand::invalid,
                                          summary.file_category});
    call_if_present(render_matrix_number,
                    MatrixNodeNumberRequest{summary.action_difficulty});
}

bool advance_random_matrix_alert(
    MatrixNodeActionState& state,
    const MatrixRandomAlertCallbacks& callbacks) {
    const auto random_bound = callbacks.sample_random_bound
                                  ? callbacks.sample_random_bound()
                                  : std::int8_t{0};
    if (state.matrix_alert_event_counter <= random_bound) {
        return false;
    }

    // 0x54A8E clears the counter before changing the Matrix HUD state.
    state.matrix_alert_event_counter = 0;
    call_if_present(callbacks.transition_hud_mode);
    if (state.matrix_hud_abort == 0) {
        call_if_present(callbacks.select_matrix_alert_display);
        call_if_present(
            callbacks.render_matrix_text,
            MatrixNodeTextRequest{MatrixNodeTextRole::system_alert_notice,
                                  MatrixNodeFunction::invalid,
                                  MatrixAlertBand::invalid,
                                  MatrixFileCategory::invalid});
        call_if_present(callbacks.wait_for_vblank_frame);
        call_if_present(callbacks.wait_for_controller_event);
    }
    call_if_present(callbacks.redraw_matrix_node_action_grid);
    return true;
}

bool run_matrix_node_action_loop(
    MatrixNodeActionState& state,
    const MatrixNodeActionRecord& record,
    const MatrixNodeActionLoopCallbacks& callbacks) {
    // 0x54662 entry: clear the Matrix-node low flags, publish bit 6 and merge
    // the selected compact record's flags into the scene state.
    state.matrix_node_result = -1;
    state.matrix_node_work_marker = 0;
    state.matrix_node_flags =
        static_cast<std::uint8_t>(state.matrix_node_flags & 0x0FU);
    state.matrix_node_flags =
        static_cast<std::uint8_t>(state.matrix_node_flags | 0x40U);
    state.matrix_node_flags = static_cast<std::uint8_t>(
        state.matrix_node_flags | record.flags);
    if (record.clears_matrix_node_auxiliary_state) {
        // The old export called this an audio sentinel. Its only proven
        // behavior here is clearing the Matrix node's auxiliary buffer, so
        // expose that behavior through the host callback rather than keeping
        // the misleading subsystem name in this conversion.
        call_if_present(callbacks.clear_matrix_node_auxiliary_state);
    }
    state.matrix_auxiliary_marker = -1;
    state.matrix_alert_event_counter = 0;
    call_if_present(callbacks.set_display_control, 0x8B00U);

    for (;;) {
        call_if_present(callbacks.set_display_control, 0x8134U);
        call_if_present(callbacks.reset_display_buffers);
        call_if_present(callbacks.refresh_matrix_node_resources);
        call_if_present(callbacks.upload_matrix_node_resource_block);
        call_if_present(callbacks.upload_matrix_node_resource_block);
        // The source executes six grid writes in the first loop and four more
        // immediately after it: ten writes per outer redraw.
        for (std::size_t index = 0; index < 10U; ++index) {
            call_if_present(callbacks.write_matrix_node_grid);
        }
        call_if_present(callbacks.select_matrix_alert_display);
        call_if_present(callbacks.refresh_matrix_node_metadata);
        call_if_present(callbacks.set_display_control, 0x8174U);
        if (callbacks.advance_random_matrix_alert) {
            callbacks.advance_random_matrix_alert(state);
        }
        // 0x547D8 tests FFF0B1 after 0x54A8E returns.  FFF0B7 is the menu
        // result written by the entry and Matrix paths, not this exit gate.
        if (state.matrix_hud_abort < 0) {
            return true;
        }

        call_if_present(
            callbacks.render_matrix_text,
            MatrixNodeTextRequest{MatrixNodeTextRole::node_function_title,
                                  record.node_function,
                                  MatrixAlertBand::invalid,
                                  MatrixFileCategory::invalid});
        call_if_present(
            callbacks.render_matrix_text,
            MatrixNodeTextRequest{MatrixNodeTextRole::action_panel_header,
                                  record.node_function,
                                  MatrixAlertBand::invalid,
                                  MatrixFileCategory::invalid});
        render_matrix_node_status_summary(
            MatrixNodeStatusSummary{record.alert_band, record.file_category,
                                    record.action_difficulty},
            callbacks.render_matrix_text, callbacks.render_matrix_number);
        state.action_cursor = 0;

        const auto publish_cursor = [&](const std::int8_t direction) {
            state.action_cursor = static_cast<std::int8_t>(move_wrapped_menu_cursor(
                static_cast<std::uint8_t>(state.action_cursor), direction,
                static_cast<std::uint8_t>(record.action_cursor_limit)));
            if (callbacks.move_matrix_action_cursor) {
                callbacks.move_matrix_action_cursor(direction, state.action_cursor);
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
            if ((input & static_cast<std::uint16_t>(MatrixNodeActionInput::move_previous)) != 0U) {
                direction = -1;
            } else if ((input & static_cast<std::uint16_t>(MatrixNodeActionInput::move_next)) != 0U) {
                direction = 1;
            }
            if (direction != 0) {
                publish_cursor(direction);
                continue;
            }

            const auto action_edges =
                static_cast<std::uint16_t>(MatrixNodeActionInput::confirm) |
                static_cast<std::uint16_t>(MatrixNodeActionInput::alternate_confirm) |
                static_cast<std::uint16_t>(MatrixNodeActionInput::matrix_node_action);
            if ((input & action_edges) != 0U) {
                const auto gate = callbacks.calculate_matrix_node_action_gate_inputs
                                      ? callbacks.calculate_matrix_node_action_gate_inputs()
                                      : MatrixNodeActionGateInputs{};
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
                    static_cast<std::uint8_t>(record.alert_band) - 1U);
                const auto delay_low = static_cast<std::uint8_t>(trial_count);
                const auto gated_low = static_cast<std::uint8_t>(
                    delay_low - compact_gate);
                trial_count = static_cast<std::uint16_t>(
                    (trial_count & 0xFF00U) | gated_low);
                if (source_subtract_byte_is_nonpositive(
                        delay_low, compact_gate, gated_low)) {
                    call_if_present(callbacks.redraw_matrix_node_action_grid);
                    call_if_present(
                        callbacks.render_matrix_text,
                        MatrixNodeTextRequest{
                            MatrixNodeTextRole::action_failure_message,
                            MatrixNodeFunction::invalid,
                            MatrixAlertBand::invalid,
                            MatrixFileCategory::invalid});
                    call_if_present(callbacks.wait_for_controller_event);
                } else {
                    const auto thresholded_rolls =
                        callbacks.count_matrix_node_action_rolls
                            ? callbacks.count_matrix_node_action_rolls(
                                  trial_count, record.action_difficulty)
                            : std::int8_t{0};
                    if (thresholded_rolls > 0) {
                        call_if_present(callbacks.dispatch_matrix_node_action,
                                        record.node_function, state.action_cursor);
                    } else {
                        call_if_present(callbacks.redraw_matrix_node_action_grid);
                        call_if_present(
                            callbacks.render_matrix_text,
                            MatrixNodeTextRequest{
                                MatrixNodeTextRole::action_failure_message,
                                MatrixNodeFunction::invalid,
                                MatrixAlertBand::invalid,
                                MatrixFileCategory::invalid});
                        call_if_present(callbacks.wait_for_controller_event);
                    }
                }
                state.matrix_alert_event_counter = static_cast<std::uint8_t>(
                    state.matrix_alert_event_counter + 1U);
                break;
            }

            if ((input & static_cast<std::uint16_t>(MatrixNodeActionInput::leave_to_matrix)) != 0U) {
                call_if_present(callbacks.set_display_control, 0x8134U);
                state.matrix_node_result = 0;
                call_if_present(callbacks.render_matrix_footer);
                call_if_present(callbacks.upload_matrix_resource_pair);
                call_if_present(callbacks.set_display_control, 0x8174U);
                const bool matrix_abort = callbacks.enter_matrix_navigation
                                               ? callbacks.enter_matrix_navigation()
                                               : false;
                for (std::size_t slot = 0; slot < 5U; ++slot) {
                    call_if_present(callbacks.reset_matrix_program_slot_timer, slot);
                }
                if (matrix_abort) {
                    state.matrix_hud_abort = -1;
                }
                break;
            }
        }
    }
}

} // namespace shadowrun::ghidra_conversion
