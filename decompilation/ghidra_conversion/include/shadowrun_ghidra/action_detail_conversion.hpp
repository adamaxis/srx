#pragma once

#include "shadowrun_ghidra/menu_transfer_batch_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kActionDetailLabelCount = 13U;
constexpr std::size_t kActionDetailResourceRecordCount = 0x40U;

enum class ActionDetailLabelVariant {
    none,
    ordinary,
    extended,
    action_dependent,
};

// This plan is the host-facing result of the three source detail routines.
// The Qt/native renderer can map label variants and overlay positions to named
// assets; it does not need the source's VDP destinations or ROM pointers.
struct ActionDetailRenderPlan {
    bool valid = false;
    std::uint8_t source_code = 0;
    std::size_t resource_row = 0;
    std::uint8_t detail_value = 0;
    ActionDetailLabelVariant label = ActionDetailLabelVariant::none;
    std::uint8_t label_index = 0;
    std::optional<std::uint8_t> action_code;
    std::array<bool, 6U> phase_overlays{};
    bool alternate_resource_variant = false;
    bool resource_upload_requested = false;
};

// ROM 0x13000's entity-action menu state. `selected_action_code` is the
// result of the 0x12148 record lookup when source mode is below two.
struct EntityActionDetailInput {
    std::uint8_t source_mode = 0;          // FFF0D0
    std::optional<std::uint8_t> selected_action_code;
    std::int8_t display_state = 0;         // active record +0x56
    std::uint8_t active_action_code = 0;   // active record +0x57
    std::uint8_t auxiliary_value = 0;     // active record +0x9D
    std::uint8_t cyberware_base = 0;       // active record +0x79
    std::uint16_t cyberware_flags = 0;     // active record +0x90
};

// ROM 0x13000. Returns the selected entry in the thirteen-label detail table.
// A missing 0x12148 result is an unresolved source boundary, not an invented
// action code.
std::optional<ActionDetailRenderPlan> select_entity_action_detail(
    const EntityActionDetailInput& input);

struct FieldActionDetailInput {
    std::uint8_t action_code = 0; // selected field-action record byte 0
    std::uint8_t flags = 0;       // selected field-action record byte 1
};

struct ActionDetailResourceTable {
    std::array<std::uint8_t, kActionDetailResourceRecordCount>
        detail_values{}; // source 0xD4540 row +0x24
};

// ROM 0x130C0. Builds the detail plan for a field-action record and reports
// the source's 0x12A54 resource-variant upload through ResourceVariantState.
ActionDetailRenderPlan build_field_action_detail_plan(
    const FieldActionDetailInput& input,
    const ActionDetailResourceTable& resources,
    ResourceVariantState& resource_variant,
    const std::function<void()>& upload_resource = {});

struct InventoryActionDetailInput {
    std::uint8_t item_code = 0; // active record +0x66 + selected slot
    std::uint8_t phase = 0;     // active record +0x6E + selected slot
    std::optional<std::uint8_t> selected_action_code;
};

// ROM 0x13228. Builds the detail plan for the selected inventory item used by
// the field-service menu. Items 0x14 and 0x18..0x28 request the source
// 0x12148 action-code lookup; callers may leave that boundary unresolved.
std::optional<ActionDetailRenderPlan> build_inventory_action_detail_plan(
    const InventoryActionDetailInput& input,
    const ActionDetailResourceTable& resources,
    ResourceVariantState& resource_variant,
    const std::function<void()>& upload_resource = {});

} // namespace shadowrun::ghidra_conversion
