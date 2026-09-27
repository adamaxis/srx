#pragma once

#include "shadowrun_ghidra/field/field_action_cost_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_resource_selection_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion {

// The source pointer table at 0xE31F2 is represented by host aliases in the
// conversion layer.  An alias is an extracted resource name, not a ROM
// address or a runtime decoder input.
struct FieldActionDescriptionResourceTable {
    std::vector<std::string> aliases;
};

struct FieldActionDescriptionInput {
    std::uint8_t description_index = 0; // D7, selected menu-resource byte
    std::uint8_t quantity = 0;           // FFF0D6
    std::optional<std::uint32_t> displayed_cost; // FFF0BE
    std::optional<FieldActionResourceSelectionResult> resource_selection;
};

// The three fixed labels emitted by 0x580FE before the quantity summary.  The
// aliases are source text resources at 0x1AB4B4, 0x1AB4BA, and 0x1AB4D8;
// naming the roles keeps the native presentation boundary independent of ROM
// addresses while retaining the source call order.
enum class FieldActionQuantityText {
    level_heading,
    size_deck_storage_heading,
    price_label,
};

enum class FieldActionDescriptionRenderKind {
    invalid_description,
    description_only,
    quantity_summary,
    missing_resource_selection,
    missing_cost,
};

struct FieldActionDescriptionRenderResult {
    FieldActionDescriptionRenderKind kind =
        FieldActionDescriptionRenderKind::invalid_description;
    std::uint8_t description_index = 0;
    std::uint8_t quantity = 0;
    std::string description_alias;
    std::uint32_t selected_storage_weight = 0;
    std::uint16_t displayed_storage_total = 0;
    std::uint32_t displayed_cost = 0;
};

struct FieldActionDescriptionRenderCallbacks {
    // 0xE31F2 text resource after 0xC134 normalization and 0xC168 wrapping.
    std::function<void(const std::string&)> render_description_text;

    // In the quantity branch, 0x580FE emits the three fixed labels, quantity,
    // selected-resource weight, restored storage total, and cost in that
    // order.  The enum retains the identity of each source text resource.
    std::function<void(FieldActionQuantityText)> render_quantity_detail_text;
    std::function<void(std::uint8_t)> render_quantity;
    std::function<void(std::uint32_t)> render_selected_storage_weight;
    std::function<void(std::uint16_t)> render_storage_total;
    std::function<void(std::uint32_t)> render_cost;

    // The quantity >= 9 path renders the source's highest-version notice
    // (0x1AB4E7) instead of the quantity summary.
    std::function<void()> render_highest_version_notice;

    // 0x1AC90C + description_index * 0x0C selects the source grid record.
    std::function<void(std::uint8_t)> write_description_grid;
};

// ROM 0x580FE.  Render a menu-resource description.  Quantities whose source
// byte compares below nine take the Matrix-capacity summary branch; larger
// quantities render the highest-version notice instead.  The description and
// final grid write happen on both paths.  The source's 0x575F8 result is
// supplied as typed data because its own conversion is a separate reusable
// field-resource boundary.
FieldActionDescriptionRenderResult render_field_action_description(
    const FieldActionDescriptionResourceTable& resources,
    const FieldActionDescriptionInput& input,
    const FieldActionDescriptionRenderCallbacks& callbacks = {});

enum class FieldActionCostSummaryNumericField {
    byte_at_6,
    byte_at_7,
    byte_at_c,
    byte_at_d,
    word_at_8,
    word_at_a,
};

// The source cost-summary rows are 0x18 bytes at 0x1AB118.  Only the fields
// used by 0x5825C are named here; the unused bytes do not become fake runtime
// state.
struct FieldActionCostSummaryRecord {
    std::int8_t availability_marker = 0; // row +0, compared with FFFBCB
    std::uint32_t base_cost = 0;         // row +2
    std::uint8_t byte_at_6 = 0;
    std::uint8_t byte_at_7 = 0;
    std::uint16_t word_at_8 = 0;
    std::uint16_t word_at_a = 0;
    std::uint8_t byte_at_c = 0;
    std::uint8_t byte_at_d = 0;
};

enum class FieldActionCostSummaryRenderKind {
    blocked,
    discounted,
    large_discounted,
};

struct FieldActionCostSummaryRenderResult {
    FieldActionCostSummaryRenderKind kind =
        FieldActionCostSummaryRenderKind::blocked;
    FieldActionCostSummaryRecord record{};
    std::uint32_t displayed_cost = 0;
};

struct FieldActionCostSummaryRenderCallbacks {
    std::function<void()> render_title;
    std::function<void(FieldActionCostSummaryNumericField, std::uint32_t)>
        render_numeric_field;
    std::function<void()> write_blocked_grid;
    std::function<void(std::uint32_t)> render_discounted_cost;
};

// ROM 0x5825C.  Emit the six row fields, then either select grid variant A
// when the current cursor is below the row requirement or apply the normal /
// large cost discount at the source's 0x10000 boundary.
FieldActionCostSummaryRenderResult render_field_action_cost_summary(
    const FieldActionCostSummaryRecord& record,
    std::int8_t current_cursor,
    const FieldActionCostContext& cost_context,
    const FieldActionCostSummaryRenderCallbacks& callbacks = {});

enum class FieldActionFeedbackRenderKind {
    unavailable,
    mode_feedback,
};

struct FieldActionFeedbackRenderResult {
    FieldActionFeedbackRenderKind kind =
        FieldActionFeedbackRenderKind::unavailable;
    std::uint8_t mode = 0;
    std::uint8_t quantity_text_offset = 0;
    std::uint8_t grid_group = 0;
};

struct FieldActionFeedbackRenderCallbacks {
    std::function<void()> render_unavailable;
    std::function<void(std::uint8_t)> render_mode_text;
    std::function<void(std::uint8_t)> render_quantity_text;
    std::function<void(std::uint8_t)> write_feedback_grid;
    std::function<void()> render_tail_text;
};

// ROM 0x58316.  Modes 0..3 select a mode message, a five-byte-spaced result
// message, and the source grid group.  Modes 4 and above only show the
// unavailable message before the common tail text.
FieldActionFeedbackRenderResult render_field_action_feedback(
    std::uint8_t mode,
    const FieldActionFeedbackRenderCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
