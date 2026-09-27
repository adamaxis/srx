#include "shadowrun_ghidra/field/field_active_runner_selection_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::string> events;
    FieldActiveRunnerSelectionCallbacks callbacks{};
    callbacks.refresh_selection_grid = [&](const FieldActiveRunnerSelectionGrid& grid) {
        events.push_back(std::to_string(grid.rows) + "x" +
                         std::to_string(grid.words_per_row));
    };

    FieldActiveRunnerSelectionState next{
        {{1U, 0x03U}, {1U, 0x01U}, {1U, 0x00U}, {0U, 0x03U}}, 0U};
    const auto selected =
        advance_field_active_runner_selection(next, callbacks);
    if (!expect(selected.result ==
                    FieldActiveRunnerSelectionResult::selected_next_runner &&
                selected.previous_index == 0U && selected.selected_index == 1U &&
                next.active_index == 1U && next.records[0].selection_flags == 0x01U &&
                next.records[1].selection_flags == 0x03U &&
                events == std::vector<std::string>{"3x4"})) {
        return 1;
    }

    FieldActiveRunnerSelectionState skips_inactive{
        {{1U, 0x01U}, {1U, 0x02U}, {1U, 0x00U}, {1U, 0x01U}, {0U, 0x00U}},
        0U};
    const auto skipped = advance_field_active_runner_selection(skips_inactive);
    if (!expect(skipped.result ==
                    FieldActiveRunnerSelectionResult::selected_next_runner &&
                skipped.selected_index == 3U && skips_inactive.active_index == 3U &&
                skips_inactive.records[1].selection_flags == 0U &&
                skips_inactive.records[2].selection_flags == 0U &&
                skips_inactive.records[3].selection_flags == 0x03U)) {
        return 2;
    }

    FieldActiveRunnerSelectionState wraps_on_boundary{
        {{1U, 0x01U}, {1U, 0x00U}, {0U, 0x00U}, {1U, 0x01U}}, 0U};
    const auto boundary = advance_field_active_runner_selection(wraps_on_boundary);
    if (!expect(boundary.result ==
                    FieldActiveRunnerSelectionResult::wrapped_to_first_runner &&
                boundary.selected_index == 0U &&
                wraps_on_boundary.active_index == 0U &&
                wraps_on_boundary.records[0].selection_flags == 0x03U)) {
        return 3;
    }

    FieldActiveRunnerSelectionState wraps_on_terminal{{{2U, 0x03U}, {1U, 0x01U}},
                                                       0U};
    const auto terminal = advance_field_active_runner_selection(wraps_on_terminal);
    if (!expect(terminal.result ==
                    FieldActiveRunnerSelectionResult::wrapped_to_first_runner &&
                wraps_on_terminal.records[0].selection_flags == 0x03U)) {
        return 4;
    }

    FieldActiveRunnerSelectionState malformed{{{1U, 0x01U}}, 0U};
    const auto invalid = advance_field_active_runner_selection(malformed);
    if (!expect(invalid.result ==
                    FieldActiveRunnerSelectionResult::invalid_record_table &&
                malformed.records[0].selection_flags == 0x01U)) {
        return 5;
    }

    return 0;
}
