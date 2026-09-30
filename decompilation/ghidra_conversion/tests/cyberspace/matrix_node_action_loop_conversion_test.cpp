#include "shadowrun_ghidra/cyberspace/matrix_node_action_loop_conversion.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::int32_t> auxiliary{1};
    fill_matrix_node_auxiliary_sentinel(auxiliary);
    if (!expect(auxiliary.size() == 35U && auxiliary.front() == -1)) {
        return 1;
    }

    MatrixAlertDisplayState display_status{};
    select_matrix_alert_display(display_status, 0);
    if (!expect(display_status.status_code == 0xCCU &&
                display_status.dirty_flags == 1U)) {
        return 2;
    }
    select_matrix_alert_display(display_status, -1);
    if (!expect(display_status.status_code == 0xC0U &&
                display_status.dirty_flags == 1U)) {
        return 3;
    }
    select_matrix_alert_display(display_status, 1);
    if (!expect(display_status.status_code == 0x0CU &&
                display_status.dirty_flags == 1U)) {
        return 4;
    }

    int repeated_grid_calls = 0;
    redraw_matrix_node_action_grid_service([&] { ++repeated_grid_calls; });
    if (!expect(repeated_grid_calls == 1)) {
        return 5;
    }

    std::vector<MatrixNodeTextRequest> summary_text;
    std::vector<MatrixNodeNumberRequest> summary_numbers;
    render_matrix_node_status_summary(
        MatrixNodeStatusSummary{MatrixAlertBand::orange,
                                MatrixFileCategory::project_files, 6U},
        [&](const MatrixNodeTextRequest& request) {
            summary_text.push_back(request);
        },
        [&](const MatrixNodeNumberRequest& request) {
            summary_numbers.push_back(request);
        });
    if (!expect(summary_text.size() == 2U &&
                summary_text[0].role == MatrixNodeTextRole::alert_band_label &&
                summary_text[0].alert_band == MatrixAlertBand::orange &&
                summary_text[1].role ==
                    MatrixNodeTextRole::file_category_label &&
                summary_text[1].file_category ==
                    MatrixFileCategory::project_files &&
                summary_numbers.size() == 1U &&
                summary_numbers.front().action_difficulty == 6U)) {
        return 6;
    }

    std::vector<std::string> order;
    std::vector<std::uint16_t> controls;
    std::vector<std::pair<MatrixNodeFunction, std::int8_t>> dispatched;
    std::vector<std::pair<std::int8_t, std::int8_t>> cursor_moves;
    std::vector<MatrixNodeTextRequest> text_requests;
    std::vector<MatrixNodeNumberRequest> number_requests;
    std::vector<std::size_t> reset_slots;
    std::vector<std::uint16_t> inputs{0x0100U, 0x4000U, 0x8000U};
    std::size_t input_index = 0;
    int outer_redraws = 0;
    bool gate_values_verified = false;
    MatrixNodeActionState state{};

    MatrixNodeActionLoopCallbacks callbacks{};
    callbacks.set_display_control =
        [&](const std::uint16_t value) { controls.push_back(value); };
    callbacks.reset_display_buffers = [&] { order.emplace_back("reset"); };
    callbacks.refresh_matrix_node_resources =
        [&] { order.emplace_back("resource"); };
    callbacks.clear_matrix_node_auxiliary_state =
        [&] { order.emplace_back("clear"); };
    callbacks.upload_matrix_node_resource_block =
        [&] { order.emplace_back("block"); };
    callbacks.write_matrix_node_grid = [&] { order.emplace_back("grid"); };
    callbacks.select_matrix_alert_display =
        [&] { order.emplace_back("status"); };
    callbacks.refresh_matrix_node_metadata =
        [&] { order.emplace_back("metadata"); };
    callbacks.advance_random_matrix_alert =
        [&](MatrixNodeActionState& current) {
            ++outer_redraws;
            current.matrix_node_result = 0;
            return false;
        };
    callbacks.render_matrix_text =
        [&](const MatrixNodeTextRequest& request) {
            text_requests.push_back(request);
            order.emplace_back("text");
        };
    callbacks.render_matrix_number =
        [&](const MatrixNodeNumberRequest& request) {
            number_requests.push_back(request);
            order.emplace_back("number");
        };
    callbacks.move_matrix_action_cursor =
        [&](const std::int8_t direction, const std::int8_t cursor) {
            order.emplace_back("cursor");
            cursor_moves.emplace_back(direction, cursor);
        };
    callbacks.wait_for_vblank = [&] { order.emplace_back("vblank"); };
    callbacks.latch_controller_events = [&] { order.emplace_back("latch"); };
    callbacks.sample_controller_input = [&] { return inputs[input_index++]; };
    callbacks.calculate_matrix_node_action_gate_inputs = [&] {
        order.emplace_back("action");
        return MatrixNodeActionGateInputs{4, 2};
    };
    callbacks.count_matrix_node_action_rolls =
        [&](const std::uint16_t trial_count, const std::uint8_t difficulty) {
            gate_values_verified = trial_count == 5U && difficulty == 6U;
            return std::int8_t{2};
        };
    callbacks.redraw_matrix_node_action_grid =
        [&] { order.emplace_back("repeat"); };
    callbacks.wait_for_controller_event = [&] { order.emplace_back("wait"); };
    callbacks.dispatch_matrix_node_action =
        [&](const MatrixNodeFunction node_function, const std::int8_t cursor) {
            dispatched.emplace_back(node_function, cursor);
        };
    callbacks.render_matrix_footer = [&] { order.emplace_back("footer"); };
    callbacks.upload_matrix_resource_pair = [&] { order.emplace_back("upload"); };
    callbacks.enter_matrix_navigation = [&] {
        order.emplace_back("matrix");
        return true;
    };
    callbacks.reset_matrix_program_slot_timer =
        [&](const std::size_t slot) { reset_slots.push_back(slot); };

    const auto result = run_matrix_node_action_loop(
        state,
        MatrixNodeActionRecord{MatrixNodeFunction::input_output_port,
                               MatrixAlertBand::green,
                               MatrixFileCategory::project_files,
                               6U,
                               0x08U,
                               false,
                               2},
        callbacks);

    if (!expect(result && state.matrix_node_result == 0 &&
                state.matrix_hud_abort < 0 &&
                state.matrix_node_flags == 0x48U &&
                state.matrix_alert_event_counter == 1U &&
                outer_redraws == 3 && gate_values_verified &&
                controls == std::vector<std::uint16_t>({
                    0x8B00U, 0x8134U, 0x8174U, 0x8134U, 0x8174U,
                    0x8134U, 0x8174U, 0x8134U, 0x8174U}) &&
                dispatched == std::vector<std::pair<MatrixNodeFunction,
                                                    std::int8_t>>{{
                    MatrixNodeFunction::input_output_port, 2}} &&
                cursor_moves ==
                    std::vector<std::pair<std::int8_t, std::int8_t>>{{
                        0, 0}, {-1, 2}, {0, 0}} &&
                text_requests.size() == 8U && number_requests.size() == 2U &&
                order.size() == 71U &&
                reset_slots == std::vector<std::size_t>({0, 1, 2, 3, 4}))) {
        return 7;
    }
    if (!expect(text_requests[0].role == MatrixNodeTextRole::node_function_title &&
                text_requests[0].node_function ==
                    MatrixNodeFunction::input_output_port &&
                text_requests[2].role == MatrixNodeTextRole::alert_band_label &&
                text_requests[3].role == MatrixNodeTextRole::file_category_label &&
                text_requests[3].file_category ==
                    MatrixFileCategory::project_files)) {
        return 8;
    }

    MatrixNodeActionState action_state{};
    reset_slots.clear();
    std::vector<std::string> action_order;
    std::vector<std::uint16_t> action_inputs{0x2000U, 0x1000U, 0x8000U};
    std::size_t action_input_index = 0;
    MatrixNodeActionLoopCallbacks action_callbacks{};
    action_callbacks.advance_random_matrix_alert =
        [&](MatrixNodeActionState& current) {
            current.matrix_node_result = 0;
            return false;
        };
    action_callbacks.sample_controller_input =
        [&] { return action_inputs[action_input_index++]; };
    action_callbacks.calculate_matrix_node_action_gate_inputs = [&] {
        action_order.emplace_back("action");
        return MatrixNodeActionGateInputs{9, 3};
    };
    action_callbacks.count_matrix_node_action_rolls =
        [&](const std::uint16_t, const std::uint8_t) { return std::int8_t{3}; };
    action_callbacks.dispatch_matrix_node_action =
        [&](const MatrixNodeFunction node_function, const std::int8_t cursor) {
            action_order.emplace_back("dispatch");
            if (node_function != MatrixNodeFunction::slave_module ||
                cursor != 0) {
                action_order.emplace_back("bad-dispatch");
            }
        };
    action_callbacks.render_matrix_footer =
        [&] { action_order.emplace_back("footer"); };
    action_callbacks.upload_matrix_resource_pair =
        [&] { action_order.emplace_back("upload"); };
    action_callbacks.enter_matrix_navigation = [&] {
        action_order.emplace_back("matrix");
        return true;
    };
    action_callbacks.reset_matrix_program_slot_timer =
        [&](const std::size_t slot) { reset_slots.push_back(slot); };

    const auto action_result = run_matrix_node_action_loop(
        action_state,
        MatrixNodeActionRecord{MatrixNodeFunction::slave_module,
                               MatrixAlertBand::red,
                               MatrixFileCategory::security_control,
                               8U,
                               0x02U,
                               false,
                               4},
        action_callbacks);
    if (!expect(action_result &&
                action_order ==
                    std::vector<std::string>{"action", "dispatch", "action",
                                             "dispatch", "footer", "upload",
                                             "matrix"} &&
                reset_slots == std::vector<std::size_t>({0, 1, 2, 3, 4}))) {
        return 9;
    }

    if (!expect(static_cast<std::uint16_t>(MatrixNodeActionInput::confirm) ==
                    0x4000U &&
                static_cast<std::uint16_t>(
                    MatrixNodeActionInput::alternate_confirm) == 0x1000U &&
                static_cast<std::uint16_t>(
                    MatrixNodeActionInput::matrix_node_action) == 0x2000U)) {
        return 10;
    }

    MatrixNodeActionState random_state{};
    random_state.matrix_alert_event_counter = 3;
    random_state.matrix_hud_abort = 0;
    std::vector<std::string> random_order;
    MatrixNodeTextRequest random_request{};
    const bool random_fired = advance_random_matrix_alert(
        random_state,
        {
            [] { return std::int8_t{1}; },
            [&] { random_order.emplace_back("transition"); },
            [&] { random_order.emplace_back("status"); },
            [&](const MatrixNodeTextRequest& request) {
                random_request = request;
                random_order.emplace_back("text");
            },
            [&] { random_order.emplace_back("vblank"); },
            [&] { random_order.emplace_back("wait"); },
            [&] { random_order.emplace_back("grid"); },
        });
    if (!expect(random_fired && random_state.matrix_alert_event_counter == 0 &&
                random_request.role == MatrixNodeTextRole::system_alert_notice &&
                random_order == std::vector<std::string>{
                                    "transition", "status", "text", "vblank",
                                    "wait", "grid"})) {
        return 11;
    }

    return 0;
}
