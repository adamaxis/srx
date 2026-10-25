#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <QColor>
#include <QPainter>

#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"
#include "numeric_visual_renderer.hpp"

#include "field_scene_runtime.hpp"

namespace shadowrun::ghidra_conversion::qt_runtime::field_scene_detail {

// The game loop seeds FFDDC4 with scene 2 before 0x03084. Loaded saves replace
// that scene reference with the saved row; both paths use the same typed scene
// resource selection below.
inline constexpr std::uint16_t kInitialFieldSceneReference = 2U;
inline constexpr std::uint8_t kFieldModalReturnTransitionFrames = 60U;

inline constexpr std::string_view kFieldHudAlias = "field.hud.background";
// ROM 0x39F2: the active runner's 6x8 frame block (0x6AADE), decoded at
// slot 0's window address 0xF144 (x 272, y 16) [C].
inline constexpr std::string_view kFieldHudActiveFrameAlias = "field.hud.active.frame";
inline constexpr std::string_view kFieldHudFaceAliasPrefix = "field.hud.face.";
inline constexpr std::string_view kFieldHudItemAliasPrefix = "field.hud.item.";
inline constexpr std::string_view kFieldHudUnarmedItemAliasPrefix =
    "field.hud.unarmed.item.";
inline constexpr std::string_view kFieldHudPartyFaceAliasPrefix =
    "field.hud.party.face.";
inline constexpr std::string_view kFieldHudPartyItemAliasPrefix =
    "field.hud.party.item.";
inline constexpr std::string_view kFieldHudPartyUnarmedItemAliasPrefix =
    "field.hud.party.unarmed.item.";
inline constexpr std::string_view kFieldHudMagicItemAliasPrefix =
    "field.hud.magic.item.";
inline constexpr std::string_view kFieldHudDelayMeterAliasPrefix =
    "field.hud.delay.meter.";
inline constexpr std::string_view kFieldHudBehaviorMeterPrimaryAliasPrefix =
    "field.hud.behavior.meter.primary.";
inline constexpr std::string_view kFieldHudBehaviorMeterCompanionAliasPrefix =
    "field.hud.behavior.meter.companion.";
inline constexpr std::string_view kFieldHudAmmoCounterAliasPrefix =
    "field.hud.ammo.counter.";
inline constexpr std::string_view kFieldEntityIdleAliasPrefix = "field.entities.idle.";
// Source +0x10 bit4 (E.RunnerStatus.DOWN 0x10) in the high byte of flags.
inline constexpr std::uint16_t kRunnerFlagsDown = 0x1000U;
inline constexpr std::uint8_t kFieldPlayerPoseFirearm = 0U;
inline constexpr std::uint8_t kFieldPlayerPoseStrike = 1U;
inline constexpr std::uint8_t kFieldPlayerPoseCast = 2U;
inline constexpr std::string_view kFieldPlayerBarrierEffectOwner =
    "field_player_barrier";
struct FieldSpellEffectProfile {
    std::uint8_t action_code;
    std::string_view asset_id;
    std::uint8_t phase_count;
};
inline constexpr std::array<FieldSpellEffectProfile, 5> kFieldSpellEffectProfiles{{
    {1U, "flame-dart", 4U},
    {2U, "flame-bolt", 6U},
    {5U, "sleep", 9U},
    {9U, "mana-blast", 6U},
    {12U, "heal-wounds", 16U},
}};
inline constexpr std::size_t kFieldSpellEffectPhaseCapacity = 16U;
inline constexpr std::string_view kFieldWeaponProjectileAliasPrefix =
    "field.weapons.projectile.";
// ROM 0xB1DC selects kind 1/selector 4 for the player hit child; assets are
// imported from its 0x7330 sprite lists and the shared 0xE44D8 object bank [C].
inline constexpr std::string_view kFieldHitSparkAliasPrefix =
    "field.entities.effects.hit.spark.frame.";
inline constexpr std::uint16_t kFieldHitSparkKind = 1U;
inline constexpr std::uint16_t kFieldHitSparkSelector = 4U;
inline constexpr std::uint16_t kFieldHitSparkPalette = 0x2000U;
inline constexpr std::uint16_t kFieldHitSparkFrameCount = 5U;
inline constexpr std::int16_t kFieldHitSparkFrameIncrement = 0x0400;
inline constexpr std::int16_t kFieldHitSparkFramePeriod = 0x1000;
inline constexpr std::uint16_t kFieldHitSparkTargetYOffset = 0x0010U;
inline constexpr std::string_view kElevatorAnimationAliasPrefix =
    "menu.field.elevator.selector.background.frame.";
inline constexpr std::string_view kElevatorCurrentFloorAliasPrefix =
    "menu.field.elevator.selector.current.floor.";
inline constexpr std::string_view kElevatorSelectedFloorAliasPrefix =
    "menu.field.elevator.selector.selected.floor.";
inline constexpr std::string_view kElevatorFeedbackAliasPrefix =
    "menu.field.elevator.selector.feedback.";
inline constexpr std::array<std::string_view, 4> kElevatorFeedbackAssetNames{{
    "previous.active", "previous.rest", "next.active", "next.rest"}};
inline constexpr int kGenesisSpriteOrigin = 0x80;


// FFDE08 is profile word 33. The 48-word block starts at FFDDC6.
inline constexpr std::size_t kSceneProfileEntityPositionWord = 33U;

inline EntityActionRecord magic_action_record_from_runner(
    const RunnerRecordFields& runner) {
    // 0x1047E / 0x106DA use this subset of the source record for Magical
    // Defense: active byte +0x10 (the high byte of the exposed big-endian
    // flags word), class +0x9B, item-0x18 skill +0x80, +0x77/+0x7C defense
    // attributes, and possession/phase rows +0x66..75.
    EntityActionRecord result{};
    result.flags_10 = static_cast<std::uint8_t>(runner.flags >> 8U);
    result.class_code = runner.archetype_index;
    result.item18_base = runner.skills[0];
    result.posture = runner.posture;
    result.attribute_77 = runner.attributes[0];
    result.attribute_7c = runner.attributes[5];
    result.item_codes = runner.inventory_item_codes;
    result.item_phases = runner.inventory_phase_bytes;
    return result;
}


namespace {
constexpr std::size_t kFieldSpellProgressionRecordCapacity = 8U;

FieldActionCostContext field_shop_cost_context(
    const std::uint8_t menu_mode, const RunnerRecordFields& runner,
    const std::uint8_t runner_feature_flags) {
    // Preserve the source mode, active runner's +0x8C statistic, feature
    // flags, and ROM action-cost table selector for each field shop.
    return FieldActionCostContext{
        menu_mode, runner_feature_flags, runner.skills[11], 2U};
}

// Command-overview text is CRAM line 1 index 10 = 0x02C2 (emulator run5
// FFD8F4 while the Start menu is open) [C]. The glyph atlas coverage is
// 0xDB, so the tint is pre-scaled to land on levels (1,6,1).
QColor cram_text_tint(const std::uint16_t cram) {
    constexpr int kGlyphCoverage = 0xDB;
    const auto level = [](const int cram_nibble) {
        return ((cram_nibble >> 1) * 255 + 3) / 7;
    };
    const auto scale = [&](const int value) {
        return std::min(255, (value * 255 + kGlyphCoverage / 2) / kGlyphCoverage);
    };
    return QColor(scale(level(cram & 0xF)),
                  scale(level((cram >> 4) & 0xF)),
                  scale(level((cram >> 8) & 0xF)));
}

QColor command_overview_text_tint() {
    return cram_text_tint(0x02C2U);
}

// ROM 0xD03C/0xD046 (Attributes/Skills page, shared by the Command and
// Hotel Karma entries) rewrites the text CRAM: FFD8D4 (the 0x4000 text
// line) = 0x02C2 bright green for the focused row and description, and
// FFD8F4 (the 0x6000 base-grid line) = 0x0060 dim green [C ROM; emulator
// e2 8844: focus 20ce20, others 006500].
QColor attributes_focus_text_tint() {
    return cram_text_tint(0x02C2U);
}

QColor attributes_base_text_tint() {
    return cram_text_tint(0x0060U);
}

// ROM 0xC1AD8.. stores " Pistols", " SMGs", " Shotguns" with a leading space
// (Firearms sub-skills); the text catalog trims it [C ROM].
int attributes_label_indent(const std::size_t label_index) {
    return label_index >= 10U && label_index <= 12U ? 8 : 0;
}

// The ROM menu strings carry their layout as padding: runner names are
// centred in 13 cells ("   Joshua    ", ROM 0x72972), posture and item
// labels in 12 ("  Neutral   " 0xD1A21, " Model 101T " 0xD4571), and
// "Physical   0%" (0xD198C) right-aligns the value to cell 13 [C]. The
// text catalog stores trimmed strings, so the presenter re-applies it.
constexpr int kRunnerNameCells = 13;
constexpr int kMenuLabelCells = 12;
constexpr int kMetricValueRightEdge = 128 + 13 * 8;

int centered_text_x(const int x, const int field_cells,
                    const std::string_view text) {
    const auto length = static_cast<int>(text.size());
    return length >= field_cells ? x : x + ((field_cells - length) / 2) * 8;
}

int right_aligned_text_x(const int right_edge,
                         const std::string_view text) {
    return right_edge - static_cast<int>(text.size()) * 8;
}

void draw_command_overview_number(
    QPainter& painter, const QImage& font, const int x, const int y,
    const std::uint32_t value,
    const NumericVisualAlignment alignment = NumericVisualAlignment::left,
    const bool group_thousands = false) {
    draw_numeric_visual(painter, font, x, y, value, alignment,
                        group_thousands, command_overview_text_tint());
}

struct FieldShopProgressionInput {
    FieldActionProgressionTables tables{};
    FieldActionProgressionPurchaseState state{};
};

std::optional<FieldShopProgressionInput> make_field_shop_progression_input(
    const ShopMenuController& menu, const RunnerRecordFields& runner,
    const std::uint32_t funds, const MagicSpellTable& spell_table,
    const std::uint8_t runner_feature_flags) {
    const auto menu_mode = menu.menu_mode();
    if (menu.menu_result() != 0 ||
        menu.selected_group() != ShopActionGroup::magic_spells ||
        menu_mode < 3U || menu_mode > 5U ||
        spell_table.entry_count > kFieldSpellProgressionRecordCapacity) {
        return std::nullopt;
    }

    FieldShopProgressionInput result;
    const auto mode_index = static_cast<std::size_t>(menu_mode - 3U);
    const auto offer_count = menu.progression_offer_count(
        ShopActionGroup::magic_spells);
    if (offer_count >
        result.tables.saved_selection_zero_by_menu_mode[mode_index].size()) {
        return std::nullopt;
    }
    for (std::size_t index = 0U; index < offer_count; ++index) {
        const auto offer = menu.progression_offer(
            ShopActionGroup::magic_spells, index);
        if (!offer.has_value()) {
            return std::nullopt;
        }
        result.tables.saved_selection_zero_by_menu_mode[mode_index][index] = {
            offer->action_code, offer->source_metadata, offer->base_cost};
    }

    result.state.menu_mode = menu_mode;
    result.state.saved_selection = 0;
    result.state.primary_cursor = static_cast<std::int8_t>(
        menu.selected_cursor());
    result.state.runner_archetype_status = static_cast<std::int8_t>(
        runner.archetype_index);
    // Source +0x7F is the active runner's spell progression ceiling.
    result.state.runner_progression_limit =
        static_cast<std::int8_t>(runner.auxiliary[0]);
    result.state.funds = funds;
    result.state.cost_context =
        field_shop_cost_context(menu_mode, runner, runner_feature_flags);
    result.state.ledger.source_record_count_word =
        spell_table.entry_count == 0U
            ? 0U
            : static_cast<std::uint16_t>(spell_table.entry_count - 1U);
    result.state.ledger.records.resize(
        kFieldSpellProgressionRecordCapacity);
    for (std::size_t index = 0U; index < spell_table.entry_count; ++index) {
        result.state.ledger.records[index] = {
            spell_table.entries[index].spell_code,
            spell_table.entries[index].force_limit};
    }
    return result;
}
} // namespace

} // namespace shadowrun::ghidra_conversion::qt_runtime::field_scene_detail
