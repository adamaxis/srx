#pragma once

#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

// The field action offer tables use four-byte records.  The first byte is the
// action code tested for availability, byte one is retained source metadata
// not consumed by this purchase path, and the final word is the base cost used
// by 0x57D7E when a new progression record is created.
struct FieldActionProgressionOffer {
    std::uint8_t action_code = 0;
    std::uint8_t source_metadata = 0;
    std::uint16_t base_cost = 0;
};

// 0x1AAEB0 and 0x1AAEF8 each provide the fixed-selection families.  The
// 0x1AAF40 family is three 0x20-byte blocks selected by menu mode 3..5; each
// block contains eight four-byte offer records.
struct FieldActionProgressionTables {
    std::array<FieldActionProgressionOffer, 18> saved_selection_two_or_more{};
    std::array<FieldActionProgressionOffer, 18> saved_selection_one{};
    std::array<std::array<FieldActionProgressionOffer, 8>, 3>
        saved_selection_zero_by_menu_mode{};
};

struct FieldActionProgressionRecord {
    std::uint8_t action_code = 0;
    std::uint8_t progression_level = 0;
};

// FFF9FA's leading word is kept separately because the ROM increments it
// when a new two-byte action record is published.  The vector is the typed
// host view of the records that follow that word; it is not a ROM pointer.
struct FieldActionProgressionLedger {
    std::uint16_t source_record_count_word = 0;
    std::vector<FieldActionProgressionRecord> records;
};

struct FieldActionProgressionPurchaseState {
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t saved_selection = 0; // FFF0B8
    std::int8_t primary_cursor = 0; // FFF0D0
    std::int8_t runner_archetype_status = 0; // active runner +0x9B
    std::int8_t runner_progression_limit = 0; // active runner +0x7F
    std::uint32_t funds = 0; // FFFBFE
    std::uint8_t selected_action_code = 0; // FFF0AE
    std::uint32_t pending_cost = 0; // FFF0BE
    std::int8_t service_result = 0; // FFF112
    FieldActionCostContext cost_context{};
    FieldActionProgressionLedger ledger{};
};

enum class FieldActionProgressionPurchaseResult {
    applied,
    not_neutral_selection,
    invalid_offer_selection,
    runner_unavailable,
    offer_unavailable,
    progression_maxed,
    progression_limit,
    insufficient_funds,
    cancelled,
};

struct FieldActionProgressionPurchaseCallbacks {
    // 0x56E76/0x56E7A establish the action panel before the offer checks.
    std::function<void()> render_action_panel;
    // The source's status gate draws the unavailable message and waits for a
    // controller event as one presentation path.
    std::function<void()> show_runner_unavailable;
    std::function<void()> show_progression_limit;
    std::function<void()> show_progression_maxed;
    std::function<void()> show_insufficient_funds;
    std::function<bool()> confirm_purchase;
};

// ROM 0x56E5A's neutral-result action acquisition branch, together with the
// 0x57D7E quote preparation that publishes FFF0AE/FFF0BE/FFF0C2.  It selects
// the source offer by saved panel selection and menu mode, finds the first
// matching typed progression record, and applies the exact funds/count/level
// mutation without retaining a source pointer.
FieldActionProgressionPurchaseResult purchase_field_action_progression(
    const FieldActionProgressionTables& tables,
    FieldActionProgressionPurchaseState& state,
    const FieldActionProgressionPurchaseCallbacks& callbacks = {});

// Select the four-byte offer consumed by the field action quantity path.
// Missing table data is reported explicitly instead of being replaced by a
// guessed action or cost.
std::optional<FieldActionProgressionOffer>
select_field_action_progression_offer(
    const FieldActionProgressionTables& tables,
    std::uint8_t menu_mode,
    std::int8_t saved_selection,
    std::int8_t primary_cursor);

} // namespace shadowrun::ghidra_conversion
