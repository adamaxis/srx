#include "shadowrun_ghidra/cyberspace/matrix_loop_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

std::uint8_t wrap_matrix_index(const std::uint8_t current,
                               const int delta) {
    const auto candidate = static_cast<int>(current) + delta;
    if (candidate < 0) {
        return 3;
    }
    if (candidate >= 4) {
        return 0;
    }
    return static_cast<std::uint8_t>(candidate);
}

void refresh_cursor_markers(MatrixCursorState& cursor) {
    cursor.row_marker = static_cast<std::uint16_t>(0x94U + cursor.row * 0x28U);
    cursor.column_marker =
        static_cast<std::uint16_t>(0x8CU + cursor.column * 0x20U);
}

} // namespace

std::uint8_t matrix_program_index(const std::uint8_t row,
                                  const std::uint8_t column) {
    return static_cast<std::uint8_t>(row * 4U + column);
}

bool matrix_program_is_available(
    const std::uint8_t program_index,
    const std::array<std::uint8_t, 12>& availability_flags) {
    return program_index >= availability_flags.size() ||
           availability_flags[program_index] != 0U;
}

bool matrix_program_uses_runner_details(const std::uint8_t program_index) {
    return program_index < 12U;
}

bool render_matrix_panel(
    const std::uint8_t program_index,
    const std::array<std::uint8_t, 12>& availability_flags,
    const MatrixPanelRenderCallbacks& callbacks) {
    if (callbacks.write_panel_header) {
        callbacks.write_panel_header();
    }

    if (!matrix_program_is_available(program_index, availability_flags)) {
        if (callbacks.render_unavailable_message) {
            callbacks.render_unavailable_message();
        }
        return false;
    }

    if (callbacks.render_program_icon) {
        callbacks.render_program_icon(program_index);
    }
    if (callbacks.render_program_detail) {
        callbacks.render_program_detail(program_index);
    }

    if (matrix_program_uses_runner_details(program_index) &&
        callbacks.render_runner_detail_panel) {
        callbacks.render_runner_detail_panel(program_index);
    }
    if (callbacks.render_description_line) {
        callbacks.render_description_line(program_index);
        callbacks.render_description_line(program_index);
    }
    return true;
}

namespace {

std::uint8_t detail_repeat_count(const std::uint16_t value) {
    const auto clamped = static_cast<std::uint16_t>(
        value >= 0x0EU ? 0x0DU : value);
    return static_cast<std::uint8_t>(clamped == 0U ? 1U : clamped);
}

std::int16_t wrapped_secondary_counter(const std::uint8_t source_value) {
    const auto wrapped = static_cast<std::uint8_t>(source_value - 2U);
    return static_cast<std::int16_t>(static_cast<std::int8_t>(wrapped));
}

} // namespace

MatrixRunnerDetailPlan render_matrix_runner_details(
    const MatrixRunnerDetailInput& input,
    const MatrixRunnerDetailCallbacks& callbacks) {
    MatrixRunnerDetailPlan plan{};
    plan.program_index = input.program_index;

    if (callbacks.write_runner_header_grid) {
        callbacks.write_runner_header_grid();
        callbacks.write_runner_header_grid();
    }

    if (!input.matrix_run_active) {
        if (callbacks.render_no_active_run_text) {
            callbacks.render_no_active_run_text();
        }
        return plan;
    }

    if (input.program_index == 4U) {
        plan.branch = MatrixRunnerDetailBranch::direct_status_program;
        plan.primary_value = input.program_status;
        plan.primary_repeat_count = detail_repeat_count(plan.primary_value);
        if (callbacks.write_primary_detail_rows) {
            callbacks.write_primary_detail_rows(plan.primary_repeat_count);
        }
    } else if (input.program_index >= 5U && input.program_index < 8U) {
        plan.branch = MatrixRunnerDetailBranch::special_program_group;
        if (callbacks.render_special_program_text) {
            callbacks.render_special_program_text();
        }
    } else {
        plan.branch = MatrixRunnerDetailBranch::delay_scaled_program;
        auto primary_delay_input = input.delay_input;
        primary_delay_input.request = 0;
        const auto primary_delay =
            calculate_runner_action_delay(primary_delay_input).delay;
        plan.primary_value = static_cast<std::uint16_t>(
            std::min<std::uint16_t>(primary_delay, input.program_status) +
            input.program_status);
        plan.primary_repeat_count = detail_repeat_count(plan.primary_value);
        if (callbacks.write_primary_detail_rows) {
            callbacks.write_primary_detail_rows(plan.primary_repeat_count);
        }

        const auto secondary_counter =
            wrapped_secondary_counter(input.current_record_value);
        if (secondary_counter >= 0 && callbacks.read_eligibility) {
            const auto eligibility = callbacks.read_eligibility();
            plan.eligibility_available = eligibility.has_value();
            plan.eligibility_positive =
                eligibility.has_value() && eligibility.value() > 0U;
            if (plan.eligibility_positive) {
                plan.secondary_repeat_count = static_cast<std::uint8_t>(
                    secondary_counter + 1);
                plan.secondary_destination_delta = static_cast<std::int16_t>(
                    plan.primary_value - secondary_counter);
                if (callbacks.write_secondary_detail_rows) {
                    callbacks.write_secondary_detail_rows(
                        plan.secondary_repeat_count,
                        plan.secondary_destination_delta);
                }
            }
        }
    }

    auto final_delay_input = input.delay_input;
    final_delay_input.request = -1;
    const auto final_delay =
        calculate_runner_action_delay(final_delay_input).delay;
    plan.final_value = static_cast<std::uint8_t>(
        final_delay + input.delay_input.profile_floor);
    plan.final_repeat_count = detail_repeat_count(plan.final_value);
    if (callbacks.write_final_detail_rows) {
        callbacks.write_final_detail_rows(plan.final_repeat_count);
    }
    return plan;
}

MatrixProgramSlotSelection select_matrix_program_slot(
    const std::uint8_t program_index,
    const std::array<std::int8_t, 5>& slot_programs) {
    for (std::uint8_t slot = 0; slot < slot_programs.size(); ++slot) {
        if (slot_programs[slot] ==
            static_cast<std::int8_t>(program_index)) {
            return MatrixProgramSlotSelection{slot, true, true};
        }
    }
    for (std::uint8_t slot = 0; slot < slot_programs.size(); ++slot) {
        if (slot_programs[slot] < 0) {
            return MatrixProgramSlotSelection{slot, true, false};
        }
    }
    return MatrixProgramSlotSelection{};
}

void apply_matrix_action_input(MatrixActionState& state,
                               const std::uint16_t input_events) {
    // The source tests bit 14 first, then bits 12 and 13, and only checks the
    // bit-15 exit branch when none of those action modes was selected.
    if ((input_events & set_overlay_marker_special) != 0U) {
        state.overlay_marker = 0xFFU;
        state.action_requested = true;
    } else if ((input_events & set_overlay_marker_zero) != 0U) {
        state.overlay_marker = 0;
        state.action_requested = true;
    } else if ((input_events & set_overlay_marker_one) != 0U) {
        state.overlay_marker = 1U;
        state.action_requested = true;
    } else if ((input_events & leave_matrix) != 0U) {
        state.exit_requested = true;
    }
}

MatrixActionRoute choose_matrix_action_route(
    const std::uint8_t row, const std::uint8_t column,
    const bool matrix_run_active) {
    if (row != 3U) {
        return MatrixActionRoute::program_slot_menu;
    }
    if (column == 3U) {
        return matrix_run_active ? MatrixActionRoute::disconnect_confirmation
                                 : MatrixActionRoute::no_action;
    }
    if (column == 2U && !matrix_run_active) {
        return MatrixActionRoute::no_action;
    }
    return MatrixActionRoute::column_action_dispatch;
}

void update_matrix_cursor(MatrixCursorState& state,
                          const std::uint16_t input_events) {
    // 0x1833C gives vertical movement priority over horizontal movement.
    // The source uses -1/+1 and wraps each byte between 0 and 3.
    if ((input_events & move_up) != 0U) {
        state.row = wrap_matrix_index(state.row, -1);
    } else if ((input_events & move_down) != 0U) {
        state.row = wrap_matrix_index(state.row, 1);
    } else if ((input_events & move_left) != 0U) {
        state.column = wrap_matrix_index(state.column, -1);
    } else if ((input_events & move_right) != 0U) {
        state.column = wrap_matrix_index(state.column, 1);
    } else {
        return;
    }

    refresh_cursor_markers(state);
    state.redraw_requested = true;
}

void initialize_matrix_loop(MatrixLoopState& state,
                            const MatrixLoopCallbacks& callbacks) {
    // 0x1822C saves these values before clearing the mode-owned working
    // fields.  The entry shell at 0x1817E has already selected Matrix mode.
    state.saved_overlay_marker = state.overlay_marker;
    state.saved_field_state = state.field_state;
    state.overlay_marker = 0;
    state.field_state = 0;

    // 0x18246..0x182A6: source upload, mode metadata, HUD sprites, and the
    // field HUD.  The addresses are extraction/presentation concerns and are
    // intentionally expressed only at the callback boundary.
    call_if_present(callbacks.write_transfer_header);
    call_if_present(callbacks.write_fixed_transfer_pair);
    call_if_present(callbacks.set_display_command, 0x8134U);
    call_if_present(callbacks.clear_plane_a);
    call_if_present(callbacks.reset_upload_state);
    call_if_present(callbacks.decode_and_upload_source_stream, 0xD9376U,
                    0x8160U);
    call_if_present(callbacks.refresh_mode_metadata);
    call_if_present(callbacks.render_hud_sprite_stream);
    call_if_present(callbacks.render_field_hud);

    // 0x182B0..0x1831A establishes the Matrix panel and its initial cursor.
    call_if_present(callbacks.build_title_sprite_records);
    state.display_dirty = true;
    call_if_present(callbacks.set_display_command, 0x8174U);
    call_if_present(callbacks.write_hud_panel_grid);
    call_if_present(callbacks.render_hud_indicator_grids);
    state.cursor.column = 0;
    state.cursor.row = 3;
    state.cursor.redraw_requested = true;
    refresh_cursor_markers(state.cursor);
    call_if_present(callbacks.write_two_hud_grids);
    call_if_present(callbacks.set_display_command, 0x8174U);
    // 0x18314 clears FFF0D1 before the first 0x1882E redraw. The temporary
    // row-3 write above is only part of the panel's setup transfer.
    state.cursor.row = 0;
    refresh_cursor_markers(state.cursor);
    state.cursor.redraw_requested = false;
    call_if_present(callbacks.redraw_matrix_panel);
}

} // namespace shadowrun::ghidra_conversion
