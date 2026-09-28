#pragma once

#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"

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
    // Item/action record +0x26, read by ROM 0x13000 when the selected action
    // is nonzero. Keeping this separate from selected_action_code preserves
    // the source's two table lookups.
    std::optional<std::uint8_t> action_detail_code;
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

// Source-data-bound 0x130C0 path. The action byte is the zero-based D4540
// row index; the typed row supplies the source +0x24 detail value.
ActionDetailRenderPlan build_field_action_detail_plan(
    const FieldActionDetailInput& input,
    const UsItemCatalog& items,
    ResourceVariantState& resource_variant,
    const std::function<void()>& upload_resource = {});

struct InventoryActionDetailInput {
    std::uint8_t item_code = 0; // active record +0x66 + selected slot
    std::uint8_t phase = 0;     // active record +0x6E + selected slot
    // 0x13228 calls 0x12148 for action-dependent item codes. The source
    // lookup is retained as typed evidence, but 0x13228 restores its original
    // item code before returning; callers must not use this value as the
    // detail-text, cost-record, or item-name key.
    std::optional<std::uint8_t> action_lookup_code;
};

// ROM 0x13228. Builds the detail plan for the selected inventory item used by
// the field-service menu. Items 0x14 and 0x18..0x28 request the source
// 0x12148 action-code lookup; the returned value is a source call result, not
// the key used by the enclosing 0x58540 renderer.
std::optional<ActionDetailRenderPlan> build_inventory_action_detail_plan(
    const InventoryActionDetailInput& input,
    const ActionDetailResourceTable& resources,
    ResourceVariantState& resource_variant,
    const std::function<void()>& upload_resource = {});

// Source-data-bound 0x13228 path. Inventory item codes are one-based and
// select the corresponding typed D4540 row before the source +0x24 read.
std::optional<ActionDetailRenderPlan> build_inventory_action_detail_plan(
    const InventoryActionDetailInput& input,
    const UsItemCatalog& items,
    ResourceVariantState& resource_variant,
    const std::function<void()>& upload_resource = {});

} // namespace shadowrun::ghidra_conversion
