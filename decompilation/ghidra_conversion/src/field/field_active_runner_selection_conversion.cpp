#include "shadowrun_ghidra/field/field_active_runner_selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::uint8_t kActiveFlag = 0x01U;
constexpr std::uint8_t kSelectionMarker = 0x02U;

void clear_selection_marker(FieldActiveRunnerRecord& record) {
    record.selection_flags = static_cast<std::uint8_t>(
        record.selection_flags & static_cast<std::uint8_t>(~kSelectionMarker));
}

void publish_selection_marker(FieldActiveRunnerRecord& record) {
    record.selection_flags = static_cast<std::uint8_t>(
        record.selection_flags | kSelectionMarker);
}

} // namespace

FieldActiveRunnerSelectionDecision advance_field_active_runner_selection(
    FieldActiveRunnerSelectionState& state,
    const FieldActiveRunnerSelectionCallbacks& callbacks) {
    if (callbacks.refresh_selection_grid) {
        callbacks.refresh_selection_grid(FieldActiveRunnerSelectionGrid{});
    }

    if (state.records.empty() || state.active_index >= state.records.size()) {
        return {FieldActiveRunnerSelectionResult::invalid_record_table,
                state.active_index, state.active_index};
    }

    const auto previous_index = state.active_index;
    auto& current = state.records[previous_index];

    // ROM 0xD4EC..0xD508: type 2 is the terminal field record, and the
    // source immediately resets the active record pointer to FF0100.
    clear_selection_marker(current);
    if (current.selection_type == 2U) {
        state.active_index = 0U;
        publish_selection_marker(state.records.front());
        return {FieldActiveRunnerSelectionResult::wrapped_to_first_runner,
                previous_index, 0U};
    }

    // ROM 0xD50C..0xD534: scan forward one 0x100-byte live-record stride.
    // The host vector is the typed equivalent of that contiguous record table.
    auto candidate = previous_index + 1U;
    while (candidate < state.records.size()) {
        const auto& candidate_record = state.records[candidate];

        // ROM BPL at 0xD51E and BEQ at 0xD524 return through the wrap path
        // before testing the candidate's flags.
        if (candidate_record.selection_type >= 3U ||
            candidate_record.selection_type == 0U) {
            state.active_index = 0U;
            publish_selection_marker(state.records.front());
            return {FieldActiveRunnerSelectionResult::wrapped_to_first_runner,
                    previous_index, 0U};
        }

        if ((candidate_record.selection_flags & kActiveFlag) != 0U) {
            state.active_index = candidate;
            publish_selection_marker(state.records[candidate]);
            return {FieldActiveRunnerSelectionResult::selected_next_runner,
                    previous_index, candidate};
        }

        // ROM 0xD52C branches to 0xD50C for an inactive candidate, clearing
        // its marker before continuing to the next record.
        clear_selection_marker(state.records[candidate]);
        ++candidate;
    }

    // A valid extracted table has a type-0 or type-3+ boundary before the
    // vector ends. Keep malformed host data bounded instead of reproducing
    // the source's unchecked pointer walk.
    return {FieldActiveRunnerSelectionResult::invalid_record_table,
            previous_index, previous_index};
}

} // namespace shadowrun::ghidra_conversion
