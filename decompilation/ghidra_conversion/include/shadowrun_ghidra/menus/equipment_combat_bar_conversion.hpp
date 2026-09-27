#pragma once

#include "shadowrun_ghidra/entities/action_detail_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EquipmentCombatInput {
    // Record +0x77..+0x7E attribute bytes used by the command overview.
    std::array<std::uint8_t, 8> attributes{};
    // Record +0x81..+0x8C skill bytes. The source selects these by item class.
    std::array<std::uint8_t, 12> skills{};
    // Eight inventory item codes and their adjacent +0x6E phase bytes.
    std::array<std::uint8_t, 8> inventory_item_codes{};
    std::array<std::uint8_t, 8> inventory_phase_bytes{};
    // Record +0x56/+0x57: equipped display state and active item code.
    std::int8_t display_state = 0;
    std::uint8_t active_item_code = 0;
    // Record +0x4D and the selected inventory slot used by 0x12148.
    std::uint8_t posture = 0;
    std::uint8_t selected_inventory_slot = 0;
    // Record +0x90: installed cyberware bits.
    std::uint16_t cyberware_flags = 0;
    // Record +0x11 bit 5, consumed by ROM 0x55E62 before its item-0x27
    // phase scan.
    std::uint8_t action_flags = 0;
    // Global +0xFFF0D0. Values below two use the selector-action gate;
    // values two and above enable the special equipped-state path.
    std::uint8_t source_mode = 0;
    // ROM 0x12148's selected action code. When absent, the conversion uses
    // the item code at selected_inventory_slot as the host-side equivalent.
    std::optional<std::uint8_t> source_action_code;
    // Record +0x80/+0x7F/+0x9D used by 0x12FA4 -> 0x10660 and the negative
    // +0x56 attack calculation.
    std::uint8_t special_item18_base = 0;
    std::uint8_t roll_count_cap = 0;
    std::uint8_t auxiliary_roll_bonus = 0;
    // Decoded D4EA0/166AD4 row consumed by the special action effect path.
    // Keeping it optional makes the unresolved data-table boundary explicit.
    std::optional<EntityActionCatalogRecord> special_action_record;
    std::optional<EntityActionSpecialItemModifierTable>
        special_item_modifier_table;
};

struct EquipmentCombatSpecialActionResult {
    // ROM 0x12D7C calls 0x12FA4 before it calculates the attack strip.
    bool menu_service_called = false;
    bool residual_calculated = false;
    bool effect_written = false;
    std::uint16_t adjusted_roll_count = 0;
    std::uint16_t effective_difficulty = 0;
    std::int16_t residual_effect = 0;
};

struct EquipmentCombatBarResult {
    std::uint8_t attack_bar = 0;
    std::uint8_t defense_bar = 0;
    bool attack_written = true;
    bool defense_written = true;
    std::uint16_t command_average = 0;
    std::uint16_t attack_value = 0;
    std::uint16_t defense_value = 0;
    EquipmentCombatSpecialActionResult special_action{};
};

struct EquipmentActionMetricInput {
    // ROM 0x105E6 reads the selected action code from the menu action table
    // before passing it to the 0x10660 residual calculation.
    std::uint8_t action_code = 0;
    std::uint16_t current_force = 0;
    EntityActionRecord entity{};
    EntityActionCatalogRecord action{};
};

struct EquipmentActionMetricCallbacks {
    std::function<void()> render_no_effect_text;
    std::function<void(std::uint16_t)> write_effect_grid;
};

struct EquipmentActionMetricResult {
    bool residual_valid = false;
    bool no_effect_text_rendered = false;
    bool effect_grid_written = false;
    std::int16_t residual_effect = 0;
    std::uint16_t effect_grid_row = 0;
};

enum class EquipmentCombatGrid {
    primary,
    secondary,
};

struct EquipmentCombatValuesInput {
    // D0=0 selects the action-metric path; any nonzero source mode selects
    // the ready-party/attribute path.
    bool alternate_path = false;
    std::uint16_t current_force = 0;
    std::uint8_t action_cap = 0;  // active record +0x7F
    EntityActionRecord active_entity{};
    std::array<EntityActionRecord, 3> ready_entities{};
    std::uint8_t attribute_77 = 0;
    std::uint8_t attribute_7c = 0;
};

struct EquipmentCombatValuesCallbacks {
    // The source 0x1047E service has a presentation side effect in addition
    // to the typed three-record bonus calculation.
    std::function<void()> refresh_ready_entity_animations;
    // This is the already-converted 0x104C8/0x104F6 menu metric boundary.
    std::function<std::optional<std::uint16_t>(std::uint16_t)>
        resolve_menu_metric;
    std::function<void(EquipmentCombatGrid, std::uint16_t)> write_grid;
};

struct EquipmentCombatValuesResult {
    bool menu_metric_available = false;
    std::uint16_t primary_value = 0;
    std::uint16_t secondary_value = 0;
    std::vector<std::pair<EquipmentCombatGrid, std::uint16_t>> grid_requests;
};

struct EquipmentHighlightInput {
    // Source record +0x56 is signed; 0x7F is the equipped-item sentinel.
    std::int8_t equipped_display_state = -1;
    // Source record +0x76 selects the extra item highlight when nonzero.
    std::uint8_t highlight_item = 0;
    std::array<std::uint8_t, 8> inventory_item_codes{};
};

struct EquipmentHighlightCallbacks {
    std::function<void()> write_menu_grid_quad;
    std::function<void()> write_menu_grid;
    std::function<void()> render_menu_tile_rows;
};

struct EquipmentHighlightResult {
    bool equipped_grid_written = false;
    bool inventory_grid_written = false;
    std::int8_t matched_inventory_slot = -1;
};

struct ItemActionDetailInput {
    // D0 at ROM 0x12A8C entry. Item records are one-based in the source;
    // the item-name row is item_code - 1.
    std::uint8_t item_code = 0;
    // Item row +0x1E is the optional art/resource pointer. The source skips
    // the art grid when its longword is negative.
    bool item_art_available = false;
    // Item row +0x24, used by ordinary action labels for the final detail
    // value grid.
    std::uint8_t item_detail_value = 0;
    // Active record +0x6E..+0x76. The source normally selects slot 8 after
    // an item-art pointer is present; the explicit slot keeps the unresolved
    // no-art entry boundary visible to the host conversion.
    std::array<std::uint8_t, 9U> phase_bytes{};
    std::uint8_t phase_slot = 8U;
};

enum class ItemActionDetailGridKind {
    item_art,
    phase_overlay,
    detail_value,
};

struct ItemActionDetailGridPlan {
    ItemActionDetailGridKind kind = ItemActionDetailGridKind::item_art;
    // Set for phase_overlay; source tests the active-record byte one bit at a
    // time in the order 1, 2, 0, 3, 4, 5.
    std::uint8_t phase_bit = 0;
    // The source VDP destinations are retained as layout provenance only;
    // callbacks consume named host resources rather than these addresses.
    std::uint16_t destination_tile_base = 0;
    // Source D1 and D7 values passed to the converted grid writer.
    std::uint16_t outer_loop_counter = 0;
    std::uint16_t inner_loop_counter = 0;
    std::uint16_t palette_flags = 0;
    // Only detail_value uses this; it selects the decoded item-detail asset.
    std::uint8_t detail_value = 0;
};

enum class ItemActionDetailTextKind {
    item_name,
    action_label,
};

struct ItemActionDetailTextPlan {
    ItemActionDetailTextKind kind = ItemActionDetailTextKind::item_name;
    std::size_t item_record_index = 0;
    ActionDetailLabelVariant label = ActionDetailLabelVariant::none;
    std::optional<std::uint8_t> action_code;
    std::uint16_t destination_tile_base = 0;
    std::uint16_t palette_flags = 0;
};

struct ItemActionDetailCallbacks {
    std::function<void(const ItemActionDetailGridPlan&)> write_grid;
    std::function<void(const ItemActionDetailTextPlan&)> render_text;
    // ROM 0x12148: read the selected entity action code. This is an explicit
    // source boundary because the record/table interpretation is converted
    // separately from this renderer.
    std::function<std::uint8_t()> read_entity_action_code;
    // ROM 0x11352: action codes >= 0x2C transfer to armor selection.
    std::function<void(bool preview_new_selection)> apply_armor_selection;
    // ROM 0x12A54: upload the alternate item resource stream when the code
    // crosses the source threshold at thirteen.
    std::function<void()> upload_resource_stream;
};

struct ItemActionDetailResult {
    // 0x12A8C restores D0 before returning.
    std::uint8_t returned_item_code = 0;
    std::optional<std::uint8_t> action_code;
    std::optional<std::uint8_t> label_action_code;
    ActionDetailLabelVariant label = ActionDetailLabelVariant::none;
    std::vector<ItemActionDetailGridPlan> grid_requests;
    std::vector<ItemActionDetailTextPlan> text_requests;
    bool resource_variant_changed = false;
    bool item_art_written = false;
    bool item_name_rendered = false;
    bool action_label_rendered = false;
    bool detail_value_written = false;
    bool early_return = false;
    bool armor_selection_requested = false;
    bool action_lookup_missing = false;
    std::uint8_t phase_slot_used = 0;
};

// ROM 0x55ED6: byte-wrapped command overview base from attributes +0x78,
// +0x7B, +0x7C and the first item-code-0x28 phase.
std::uint16_t calculate_equipment_command_average(
    const EquipmentCombatInput& input);

// ROM 0x55E62: apply the entity flag or first item-code-0x27 phase reduction
// used by the defense strip.
std::uint16_t apply_equipment_defense_action_modifier(
    const EquipmentCombatInput& input,
    std::uint16_t value);

struct EquipmentActionSkillMergeInput {
    // D6 at entry to ROM 0x12C74.
    std::uint16_t current_value = 0;
    // Active record +0x57. The source selects the comparison skill from
    // +0x81..+0x84 using these action-code ranges.
    std::uint8_t action_code = 0;
    std::uint8_t skill_81 = 0;
    std::uint8_t skill_82 = 0;
    std::uint8_t skill_83 = 0;
    std::uint8_t skill_84 = 0;
};

struct EquipmentActionSkillMergeResult {
    std::uint16_t value = 0;
    std::uint8_t selected_skill = 0;
};

// ROM 0x12C74: select +0x82, +0x83, or +0x84 according to the temporary
// action-code CMP.B/BPL bands, compare the candidate with +0x81 using the
// source wrapped byte subtraction, then perform the wrapped word merge with
// D6 and replace a zero result with one. The source selection is part of this
// function; callers must not pre-collapse it into a generic skill argument.
EquipmentActionSkillMergeResult merge_equipment_action_skill_value(
    const EquipmentActionSkillMergeInput& input);

// ROM 0x12CEE and its helpers 0x55ED6/0x55F20/0x55E62/0x12C74:
// calculate the command-screen attack/defense strips from typed runner
// fields. For source mode >= 2, a negative +0x56 uses 0x12FA4/0x10660 for
// the special action effect and then computes its attack strip from +0x80,
// +0x7F, +0x9D, and the matching inventory phase.
EquipmentCombatBarResult compute_equipment_combat_bars(
    const EquipmentCombatInput& input);

// ROM 0x105E6: calculate the selected action's residual through 0x10660;
// nonpositive residuals use the no-effect text path, while positive residuals
// select the source effect row (residual - 1) for one grid write.
EquipmentActionMetricResult refresh_equipment_action_metric(
    const EquipmentActionMetricInput& input,
    const EquipmentActionMetricCallbacks& callbacks = {});

// ROM 0x1055E: choose the normal action-metric branch or the alternate
// ready-party branch, then publish the source primary/secondary grid rows.
EquipmentCombatValuesResult refresh_equipment_combat_values(
    const EquipmentCombatValuesInput& input,
    const EquipmentCombatValuesCallbacks& callbacks = {});

// ROM 0x11624: write the quad base, conditionally write the equipped-item
// highlight, scan inventory slots 7..0 for (+0x76 + 0x2B), and render rows.
EquipmentHighlightResult render_equipment_highlights(
    const EquipmentHighlightInput& input,
    const EquipmentHighlightCallbacks& callbacks = {});

// ROM 0x12A8C: render one selected item in the equipment/action detail
// panel. This preserves the source action-code bands and callback order while
// exposing typed host plans instead of ROM pointers or VDP memory accesses.
ItemActionDetailResult render_item_action_details(
    const ItemActionDetailInput& input,
    ResourceVariantState& resource_variant,
    const ItemActionDetailCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
