#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// One 0x0E-byte record from the two 0x70-byte tables used by the positive
// result/saved-selection-3 branch at 0x573FC.  Only the fields that the ROM
// copies or compares are named here; the other source bytes remain data-owned.
struct FieldResourceRecord {
    std::uint8_t availability_requirement = 0; // record +0x00
    std::array<std::uint8_t, 5> opaque_bytes_01_05{};
    std::uint8_t installed_item_code = 0; // record +0x06
    std::uint8_t installed_item_flags = 0; // record +0x07
    std::uint16_t primary_capacity = 0; // record +0x08
    std::uint16_t secondary_capacity = 0; // record +0x0A
    std::uint8_t resource_limit = 0; // record +0x0C
    std::uint8_t resource_progress = 0; // record +0x0D
};

struct FieldResourceRecordTables {
    std::array<FieldResourceRecord, 8> general_records{};
    std::array<FieldResourceRecord, 8> menu_mode_six_records{};
};

struct FieldResourceInventoryState {
    std::uint8_t installed_item_code = 0; // FFFBBE
    std::uint8_t installed_item_flags = 0; // FFFBBF
    std::uint16_t primary_capacity = 0; // FFFBC0
    std::uint16_t secondary_capacity = 0; // FFFBC2
    std::uint8_t resource_limit = 0; // FFFBC4
    std::uint8_t resource_progress = 0; // FFFBC5
    std::uint8_t availability_marker = 0; // FFFBCB
};

struct FieldResourceRecordPurchaseState {
    std::uint8_t menu_mode = 0; // FFF0B6
    std::uint8_t category = 0; // FFF0D0
    std::uint32_t cost = 0; // FFF0BE
    std::uint32_t funds = 0; // FFFBFE
    FieldResourceInventoryState inventory{};
};

enum class FieldResourceRecordPurchaseResult {
    applied,
    invalid_record,
    insufficient_requirement,
    insufficient_funds,
    cancelled,
};

struct FieldResourceRecordPurchaseCallbacks {
    std::function<void()> render_detail_grid;
    std::function<void()> show_insufficient_requirement;
    std::function<void()> show_insufficient_funds;
    std::function<bool()> confirm_purchase;
    std::function<void()> wait_for_controller_event;
};

// Positive-result saved-selection-3 branch at ROM 0x573FC. Select the table
// by menu mode, select the category's 0x0E-byte record, check its source byte
// requirement and the current funds, then copy the confirmed record fields.
FieldResourceRecordPurchaseResult apply_field_resource_record_purchase(
    const FieldResourceRecordTables& tables,
    FieldResourceRecordPurchaseState& state,
    const FieldResourceRecordPurchaseCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
