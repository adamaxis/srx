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
    apply_selected_field_action,
    purchase_field_action_detail,
    show_runner_action_description,
    remove_runner_action,
    runner_action_unavailable,
    install_matrix_program_resource,
    purchase_field_resource_record,
    purchase_field_quantity,
    purchase_field_resource,
    apply_field_equipment_upgrade,
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
        FieldActionSelectionRouteKind::runner_action_unavailable;
    std::optional<FieldActionRecordSource> record_source;
};

struct FieldActionSelectionCallbacks {
    // Each callback names the actual 0x56DC4 leaf operation. The leaf
    // conversion owns its own typed records and presentation callbacks; this
    // dispatcher only preserves the source branch and hands it off.
    std::function<void(FieldActionRecordSource,
                       FieldActionSelectionState&)>
        apply_selected_field_action;
    std::function<void(FieldActionSelectionState&)>
        purchase_field_action_detail;
    std::function<void(FieldActionSelectionState&)>
        show_runner_action_description;
    std::function<void(FieldActionSelectionState&)> remove_runner_action;
    std::function<void(FieldActionSelectionState&)> runner_action_unavailable;
    std::function<void(FieldActionSelectionState&)>
        install_matrix_program_resource;
    std::function<void(FieldActionSelectionState&)>
        purchase_field_resource_record;
    std::function<void(FieldActionSelectionState&)>
        purchase_field_quantity;
    std::function<void(FieldActionSelectionState&)>
        purchase_field_resource;
    std::function<void(FieldActionSelectionState&)>
        apply_field_equipment_upgrade;
};

// ROM 0x56DC4's branch matrix, expressed in terms of the menu result, saved
// panel selection, and the active runner's signed archetype/status gate. The
// result identifies the exact field/matrix action family that the caller must
// execute next.
FieldActionSelectionRoute classify_field_action_selection(
    const FieldActionSelectionState& state);

// ROM 0x56DC4 dispatcher boundary. It clears FFF112, classifies the selected
// field action, and hands that route to the corresponding typed leaf. No ROM
// pointer, Genesis address, or bytecode interpreter is involved.
FieldActionSelectionRoute run_field_action_selection(
    FieldActionSelectionState& state,
    const FieldActionSelectionCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
