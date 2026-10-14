#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// The live field record fields consumed by ROM 0xD4C8.  The host model keeps
// the two source roles explicit instead of exposing the original record
// pointer and byte offsets.
struct FieldActiveRunnerRecord {
    // Source live-record +0x3E. Values 0/3+ terminate the forward scan; the
    // source's current-record value 2 also causes an immediate wrap.
    std::uint16_t selection_type = 0;
    // Source live-record +0x10. Bit 0 means selectable/active and bit 1 is
    // the transient field-selection marker published by this routine.
    std::uint8_t selection_flags = 0;
};

struct FieldActiveRunnerSelectionState {
    std::vector<FieldActiveRunnerRecord> records;
    std::size_t active_index = 0;
};

struct FieldActiveRunnerSelectionGrid {
    // ROM 0xD4C8 loads D1=2 and D7=3 before 0x12C5A/0x11C0. DBF makes that
    // three rows by four words per row in the host presentation request.
    std::uint8_t rows = 3;
    std::uint8_t words_per_row = 4;
};

struct FieldActiveRunnerSelectionCallbacks {
    // Rebuild the small field runner-selection/HUD grid before publishing the
    // next active record. The renderer owns the actual host asset/layout.
    std::function<void(const FieldActiveRunnerSelectionGrid&)>
        refresh_selection_grid;
};

enum class FieldActiveRunnerSelectionResult {
    selected_next_runner,
    wrapped_to_first_runner,
    invalid_record_table,
};

struct FieldActiveRunnerSelectionDecision {
    FieldActiveRunnerSelectionResult result =
        FieldActiveRunnerSelectionResult::invalid_record_table;
    std::size_t previous_index = 0;
    std::size_t selected_index = 0;
};

// ROM 0xD4C8: refresh the field runner-selection grid, clear the current
// record's transient marker, and advance to the next active field runner.
// Record type 2 and the source's 0/3+ boundaries wrap to the first live
// record; an active candidate publishes bit 1 and becomes the field record.
// This is a typed conversion/reference routine, not a ROM-address runtime.
FieldActiveRunnerSelectionDecision advance_field_active_runner_selection(
    FieldActiveRunnerSelectionState& state,
    const FieldActiveRunnerSelectionCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
