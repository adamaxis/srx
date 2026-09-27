#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// These are the source table families selected by the 0x56DC4 dispatcher
// before it enters 0x5794E.  The values describe extracted data provenance;
// they are not runtime ROM addresses.
enum class FieldActionRecordSource {
    negative_selection_zero,
    negative_selection_one,
    negative_selection_two,
    neutral_selection_one,
    neutral_low_menu_mode,
    neutral_menu_mode_three,
    neutral_high_menu_mode,
};

enum class FieldActionSelectionRouteKind {
    selected_action_record,
    field_action_detail_selection,
    runner_action_description,
    remove_runner_action,
    neutral_action_unavailable,
    resource_quantity_selection,
    resource_record_purchase,
    quantity_purchase,
    resource_purchase,
    equipment_action,
};

struct FieldActionSelectionState {
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t menu_result = 0; // FFF0B7
    std::uint8_t saved_selection = 0; // FFF0B8
    std::int8_t runner_archetype_status = 0; // active record +0x9B
    std::int8_t service_result = 0; // FFF112
};

struct FieldActionSelectionRoute {
    FieldActionSelectionRouteKind kind =
        FieldActionSelectionRouteKind::neutral_action_unavailable;
    std::optional<FieldActionRecordSource> record_source;
};

struct FieldActionSelectionCallbacks {
    // The dispatcher chooses a concrete leaf path; the leaf conversion owns
    // its own typed state and callbacks.  This boundary is deliberately not
    // a generic command interpreter.
    std::function<void(const FieldActionSelectionRoute&,
                       FieldActionSelectionState&)> execute_route;
};

// ROM 0x56DC4's branch matrix, expressed in terms of the menu result and
// saved panel selection.  The result identifies the exact field/matrix action
// family that the caller must execute next.
FieldActionSelectionRoute classify_field_action_selection(
    const FieldActionSelectionState& state);

// ROM 0x56DC4 dispatcher boundary. It clears FFF112, classifies the selected
// field action, and hands that route to the corresponding typed leaf. No ROM
// pointer, Genesis address, or bytecode interpreter is involved.
FieldActionSelectionRoute run_field_action_selection(
    FieldActionSelectionState& state,
    const FieldActionSelectionCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
