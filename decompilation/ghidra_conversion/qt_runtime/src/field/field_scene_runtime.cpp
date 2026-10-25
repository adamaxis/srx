#include "field_scene_runtime.hpp"
#include "field_scene_runtime_internal.hpp"

#include "enums.hpp"
#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_proximity_collision_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_route_primitives_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"
#include "shadowrun_ghidra/field/field_equipment_upgrade_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/session/runner_status_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/vid_phone_service_conversion.hpp"
#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/heading_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_state_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_spawn_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/menus/cyberware_detail_conversion.hpp"
#include "genesis_image_compositor.hpp"
#include "numeric_visual_renderer.hpp"
#include "field_map_composition.hpp"
#include "matrix_map_presenter.hpp"
#include "matrix_datastore_facts.hpp"

#include <QPainter>
#include <QColor>
#include <QRegion>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

using field_scene_detail::kInitialFieldSceneReference;
using field_scene_detail::kSceneProfileEntityPositionWord;
using field_scene_detail::kFieldModalReturnTransitionFrames;
using field_scene_detail::magic_action_record_from_runner;
namespace {

// Qt 6.9 added QImage::flipped(Qt::Orientations); 6.8 (Debian/Ubuntu packages)
// only has mirrored(bool, bool). Same pixels either way.
// game.state facts for the FFFC1E long (story_flag_bit_<n>) and the FFDD64
// trigger index (story_trigger_active_<n>); see gameplay/scripts/story_flags.lua.
constexpr std::string_view kStoryFlagBitPrefix = "story_flag_bit_";
constexpr std::string_view kStoryTriggerActivePrefix = "story_trigger_active_";
// Source FFFC30: field-mode story flags. This is distinct from the FFFC1E
// world story flags and the FFFC34 mode-effect flags below.
constexpr std::string_view kFieldModeFlagBitPrefix = "field_mode_flag_bit_";
constexpr std::string_view kFieldModeEffectFlagBitPrefix =
    "field_mode_effect_flag_bit_";
// ROM FFFC26: dialogue and indexed-title-service branch state. Keep this
// byte separate from the FFFC30 mode word and FFFC34 effect word.
constexpr std::string_view kFieldDialogueFlagBitPrefix =
    "field_dialogue_flag_bit_";
// ROM FFFC40 is a separate field-activity fact word used by the Council
// Island Spirit Eyes sequence. Keep it distinct from FFFC34 mode effects.
constexpr std::string_view kFieldAuxiliaryFlagBitPrefix =
    "field_auxiliary_flag_bit_";
constexpr std::string_view kGroupItemFlagBitPrefix =
    "group_item_flag_bit_";
constexpr std::string_view kNotebookTipFlagBitPrefix =
    "notebook_tip_flag_bit_";
constexpr std::string_view kMrJohnsonsFlagBitPrefix =
    "mr_johnsons_flag_bit_";
constexpr std::string_view kRunnerInformationFlagBitPrefix =
    "runner_information_bit_";

std::optional<std::uint8_t> story_fact_bit_index(
    const std::string_view name, const std::string_view prefix,
    const unsigned bit_count = 32U) {
    if (name.size() <= prefix.size() ||
        name.substr(0U, prefix.size()) != prefix) {
        return std::nullopt;
    }
    unsigned index = 0U;
    const auto suffix = name.substr(prefix.size());
    const auto parsed = std::from_chars(
        suffix.data(), suffix.data() + suffix.size(), index);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != suffix.data() + suffix.size() || index >= bit_count ||
        index > std::numeric_limits<std::uint8_t>::max()) {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(index);
}

QImage flipped_image(const QImage& image, const Qt::Orientations orientations) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    return image.flipped(orientations);
#else
    return image.mirrored(orientations.testFlag(Qt::Horizontal),
                          orientations.testFlag(Qt::Vertical));
#endif
}
// Keep the ROM's FFF101 debug-menu branch exposed during development. Set to
// false to return to the title-code-only unlock rule.
constexpr bool kDebugMenuAlwaysUnlockedForTesting = true;

// ROM 0x5A94/0x5ABE: +0x0C is the projected Y, cleared outside the window.
bool caster_projection_active(const std::uint16_t world_x,
                              const std::uint16_t world_y,
                              const std::uint16_t camera_x,
                              const std::uint16_t camera_y) {
    EntityProjectionState projection{};
    projection.world_x = world_x;
    projection.world_y = world_y;
    project_entity_screen_position_standard(projection, camera_x, camera_y);
    return projection.screen_y != 0U;
}
constexpr std::array<FieldWorldActorSlot, 7> kFieldWorldActorSlots{{
    FieldWorldActorSlot::slot_3,
    FieldWorldActorSlot::slot_4,
    FieldWorldActorSlot::slot_5,
    FieldWorldActorSlot::slot_6,
    FieldWorldActorSlot::slot_7,
    FieldWorldActorSlot::slot_8,
    FieldWorldActorSlot::slot_9,
}};

enum class ActorServiceFlag : std::uint8_t {
    active = 1U << 0U,
    unresolved_service_bit_2 = 1U << 2U,
    targets_world_actor_group = 1U << 3U,
    action_candidate_reserved = 1U << 4U,
    activity_probe_enabled = 1U << 5U,
    effect_locked = 1U << 6U,
};

enum class ActorSpawnStateFlag : std::uint8_t {
    effect_route = 1U << 3U,
    transition = 1U << 5U,
    world_activity_member = 1U << 6U,
};

enum class ActorInteractionFlag : std::uint8_t {
    service_selection_latch = 1U << 4U,
    primary_animation_timer_enabled = 1U << 7U,
};

enum class ActorBehaviorFlag : std::uint8_t {
    action_window_open = 1U << 4U,
    resettable_effect_state = 1U << 6U,
    high_effect_route = 1U << 7U,
};

constexpr std::uint8_t actor_flag_mask(const ActorServiceFlag flag) {
    return static_cast<std::uint8_t>(flag);
}

constexpr std::uint8_t actor_flag_mask(const ActorSpawnStateFlag flag) {
    return static_cast<std::uint8_t>(flag);
}

constexpr std::uint8_t actor_flag_mask(const ActorInteractionFlag flag) {
    return static_cast<std::uint8_t>(flag);
}

constexpr std::uint8_t actor_flag_mask(const ActorBehaviorFlag flag) {
    return static_cast<std::uint8_t>(flag);
}

constexpr bool actor_flag_is_set(const std::uint8_t flags,
                                 const ActorServiceFlag flag) {
    return (flags & actor_flag_mask(flag)) != 0U;
}

constexpr bool actor_flag_is_set(const std::uint8_t flags,
                                 const ActorInteractionFlag flag) {
    return (flags & actor_flag_mask(flag)) != 0U;
}

constexpr bool actor_flag_is_set(const std::uint8_t flags,
                                 const ActorBehaviorFlag flag) {
    return (flags & actor_flag_mask(flag)) != 0U;
}

constexpr bool actor_flag_is_set(const std::uint8_t flags,
                                 const ActorSpawnStateFlag flag) {
    return (flags & actor_flag_mask(flag)) != 0U;
}

scripting::ScriptCombatantFacts script_combatant_facts(
    const RunnerRecordFields& record) {
    scripting::ScriptCombatantFacts facts{};
    facts.record_type = record.record_type;
    facts.source_flags = static_cast<std::uint8_t>(record.flags >> 8U);
    facts.interaction_flags = static_cast<std::uint8_t>(record.flags);
    facts.cyberware_flags = record.cyberware_flags;
    facts.physical_meter = record.delay_meter;
    facts.mental_meter = record.behavior_meter;
    facts.posture = record.posture;
    facts.command_attribute_77 = record.attributes[0];
    facts.command_attribute_78 = record.attributes[1];
    facts.command_attribute_79 = record.attributes[2];
    facts.command_attribute_7b = record.attributes[4];
    facts.command_attribute_7c = record.attributes[5];
    facts.command_attribute_85 = record.skills[5];
    facts.item18_base = record.skills[0];
    facts.auxiliary_roll_bonus = record.magic_force;
    facts.action_skills = {record.skills[1], record.skills[2],
                           record.skills[3], record.skills[4]};
    facts.armor_index = record.equipped_armor_index;
    facts.race_or_state = static_cast<std::int8_t>(record.species_index);
    facts.class_code = record.archetype_index;
    facts.equipped_action_code = record.equipped_item_code;
    facts.action_profile_modifier_flags =
        record.action_profile_modifier_flags;
    // 0x55786 follows the actor's +0xD8 record link, then reads that linked
    // record's +0xA0 profile selector. The runner's magic menu table index is
    // unrelated, so leave this explicitly unresolved until the live link is
    // available from the typed actor record.
    facts.linked_action_profile_index = 0xFFFFU;
    facts.item_codes = record.inventory_item_codes;
    facts.item_phases = record.inventory_phase_bytes;
    facts.action_phase_extension = {
        record.equipped_armor_index,
        record.attributes[0], record.attributes[1], record.attributes[2],
        record.attributes[3], record.attributes[4], record.attributes[5],
        record.attributes[6], record.attributes[7], record.auxiliary[0]};
    return facts;
}

std::array<scripting::ScriptActionProfileModifierPair, 5>
script_action_profile_modifiers(const EntityActionProfileTable& profiles) {
    std::array<scripting::ScriptActionProfileModifierPair, 5> result{};
    for (std::size_t index = 0U; index < result.size(); ++index) {
        const auto primary = profiles.rows[index].primary_modifier();
        const auto secondary = profiles.rows[index].secondary_modifier();
        result[index] = scripting::ScriptActionProfileModifierPair{
            primary.roll_count, primary.difficulty,
            secondary.roll_count, secondary.difficulty};
    }
    return result;
}

std::array<std::int8_t, 2> script_action_attribute_selectors() {
    const auto& source_rows = us_magic_spell_catalog();
    // ROM 0x55786 selects the signed +0x27 byte from D4EA0 rows 5 and 12
    // when live +0xDF bits 0 and 1 select the primary/secondary branch.
    return {source_rows[5U].attribute_selector,
            source_rows[12U].attribute_selector};
}

constexpr std::string_view kFieldHudAlias = "field.hud.background";
constexpr std::string_view kFieldHudFaceAliasPrefix = "field.hud.face.";
constexpr std::string_view kFieldHudItemAliasPrefix = "field.hud.item.";
constexpr std::string_view kFieldHudUnarmedItemAliasPrefix =
    "field.hud.unarmed.item.";
constexpr std::string_view kFieldHudPartyFaceAliasPrefix =
    "field.hud.party.face.";
constexpr std::string_view kFieldHudPartyItemAliasPrefix =
    "field.hud.party.item.";
constexpr std::string_view kFieldHudPartyUnarmedItemAliasPrefix =
    "field.hud.party.unarmed.item.";
constexpr std::string_view kFieldHudMagicItemAliasPrefix =
    "field.hud.magic.item.";
constexpr std::string_view kFieldHudDelayMeterAliasPrefix =
    "field.hud.delay.meter.";
constexpr std::string_view kFieldHudBehaviorMeterPrimaryAliasPrefix =
    "field.hud.behavior.meter.primary.";
constexpr std::string_view kFieldHudBehaviorMeterCompanionAliasPrefix =
    "field.hud.behavior.meter.companion.";
constexpr std::string_view kFieldHudAmmoCounterAliasPrefix =
    "field.hud.ammo.counter.";
constexpr std::string_view kFieldEntityIdleAliasPrefix = "field.entities.idle.";
constexpr std::string_view kFieldEntityWalkAliasPrefix =
    "field.entities.movement.";
constexpr std::string_view kFieldEntityAttackAliasPrefix =
    "field.entities.attack.";
constexpr std::string_view kFieldEntityStrikeAliasPrefix =
    "field.entities.strike.";
constexpr std::string_view kFieldEntityCastAliasPrefix =
    "field.entities.spell.";
constexpr std::string_view kFieldEntityDownAliasPrefix =
    "field.entities.down.";
// Source +0x10 bit4 (E.RunnerStatus.DOWN 0x10) in the high byte of flags.
constexpr std::uint16_t kRunnerFlagsDown = 0x1000U;
constexpr std::uint8_t kFieldPlayerPoseFirearm = 0U;
constexpr std::uint8_t kFieldPlayerPoseStrike = 1U;
constexpr std::uint8_t kFieldPlayerPoseCast = 2U;
struct FieldSpellEffectProfile {
    std::uint8_t action_code;
    std::string_view asset_id;
    std::uint8_t phase_count;
};
constexpr std::array<FieldSpellEffectProfile, 5> kFieldSpellEffectProfiles{{
    {1U, "flame-dart", 4U},
    {2U, "flame-bolt", 6U},
    {5U, "sleep", 9U},
    {9U, "mana-blast", 6U},
    {12U, "heal-wounds", 16U},
}};
constexpr std::size_t kFieldSpellEffectPhaseCapacity = 16U;
constexpr std::string_view kFieldWeaponProjectileAliasPrefix =
    "field.weapons.projectile.";
constexpr std::string_view kElevatorAnimationAliasPrefix =
    "menu.field.elevator.selector.background.frame.";
constexpr std::string_view kElevatorCurrentFloorAliasPrefix =
    "menu.field.elevator.selector.current.floor.";
constexpr std::string_view kElevatorSelectedFloorAliasPrefix =
    "menu.field.elevator.selector.selected.floor.";
constexpr std::string_view kElevatorFeedbackAliasPrefix =
    "menu.field.elevator.selector.feedback.";
constexpr std::array<std::string_view, 4> kElevatorFeedbackAssetNames{{
    "previous.active", "previous.rest", "next.active", "next.rest"}};
constexpr int kGenesisSpriteOrigin = 0x80;
constexpr std::string_view kCommandOverviewBackgroundAlias =
    "menu.field.background";
constexpr std::string_view kCommandOverviewInventoryBoxesAlias =
    "menu.field.inventory.boxes";
constexpr std::string_view kCommandOverviewCursorAlias = "menu.field.cursor";
constexpr std::string_view kRandomGridFixedAliasPrefix =
    "menu.field.random.grid.fixed.";
constexpr std::string_view kRandomGridIndexedAliasPrefix =
    "menu.field.random.grid.indexed.";
constexpr std::string_view kCommandOverviewFocusAliasPrefix =
    "menu.field.command.focus.";
constexpr std::string_view kCommandOverviewStatusAliasPrefix =
    "menu.field.status.bar.";
constexpr std::string_view kCommandOverviewSelectedCellAliasPrefix =
    "menu.field.selected.cell.";
constexpr std::string_view kCommandOverviewActionBackgroundAlias =
    "menu.field.action.background";
constexpr std::string_view kCommandOverviewActionConfirmationBackgroundAlias =
    "menu.field.action.confirm.background";
constexpr std::string_view kCommandOverviewClipsBackgroundAlias =
    "menu.field.clips.background";
constexpr std::string_view kCommandOverviewClipsTradeBackgroundAlias =
    "menu.field.clips.trade.background";
constexpr std::string_view kCommandOverviewActionFocusAliasPrefix =
    "menu.field.action.focus.";
constexpr std::string_view kCommandOverviewActionConfirmationFocusAliasPrefix =
    "menu.field.action.confirm.focus.";
constexpr std::string_view kCommandOverviewClipsFocusAliasPrefix =
    "menu.field.clips.focus.";
constexpr std::string_view kCommandOverviewClipsTradeFocusAliasPrefix =
    "menu.field.clips.trade.focus.";
constexpr std::string_view kCommandOverviewDamageBarAliasPrefix =
    "menu.field.damage.bar.";
constexpr std::string_view kCommandOverviewDrainBarAliasPrefix =
    "menu.field.drain.bar.";
constexpr std::string_view kCommandOverviewArmorSummaryBarAliasPrefix =
    "menu.field.armor.summary.bar.";
constexpr std::string_view kCommandOverviewDetailValueAliasPrefix =
    "menu.field.detail.value.";
constexpr std::string_view kCommandOverviewAttackBarAliasPrefix =
    "menu.field.attack.bar.";
constexpr std::string_view kCommandOverviewDefenseBarAliasPrefix =
    "menu.field.defense.bar.";
// ROM 0x117A0 writes the 15x2 action row at the same lower command-panel
// origin used by the five command tabs.  0x117A4 overlays one 2x2 focus
// block at each of the four action-button centers.
constexpr int kCommandOverviewActionBackgroundX = 120;
constexpr int kCommandOverviewActionBackgroundY = 184;
// ROM destination C928 is row 18, column 20 relative to C000 when each
// 0x80-byte name-table row holds 64 two-byte cells: host pixel (160,144).
constexpr int kRunnerPreviewPanelX = 160;
constexpr int kRunnerPreviewPanelY = 144;
// CD33E/CD37A button art begins 16 px into the 120 px strip. The prior
// positions reused the five-tab command row's 8 px inset and shifted every
// focus block left, leaving the unselected button art visible beside it.
constexpr std::array<int, 4> kCommandOverviewActionFocusX{{136, 160, 184,
                                                             208}};
constexpr int kCommandOverviewActionFocusY = 184;
// CD3B6 places its Yes and No controls at x=160 and x=184 in the centered
// 120 px row. Keep the selected 16x16 art on those same controls; offsetting
// Yes to x=136 draws a duplicate button beside the two source choices.
constexpr std::array<int, 2> kCommandOverviewActionConfirmationFocusX{{160,
                                                                        184}};
constexpr int kCommandOverviewActionConfirmationFocusY = 184;
// 0x11B58 and 0x11E7E use the same centered 15x2 child row as the inventory
// action loop, but select from their own four-entry focus tables. The caller
// clears the complete 20x2 button panel before placing this child row.
constexpr int kCommandOverviewClipsBackgroundX = 120;
constexpr int kCommandOverviewClipsBackgroundY = 184;
constexpr std::array<int, 4> kCommandOverviewClipsFocusX{{136, 160, 184,
                                                            208}};
constexpr int kCommandOverviewClipsFocusY = 184;
// The ROM writes the selected command caption into a 17-character name-table
// field beginning at x=112. Pad Lua's semantic label into that same field so
// labels remain centered while the button selector moves independently.
std::string centered_command_text(const std::string_view label,
                                  const std::size_t columns) {
    if (columns == 0U || label.size() >= columns) {
        return std::string(label);
    }
    const auto padding = columns - label.size();
    const auto left_padding = padding / 2U;
    const auto right_padding = padding - left_padding;
    return std::string(left_padding, ' ') + std::string(label) +
           std::string(right_padding, ' ');
}

std::string centered_command_footer(const std::string_view label) {
    return centered_command_text(label, 17U);
}

FieldActionCostContext field_shop_cost_context(
    const std::uint8_t menu_mode, const RunnerRecordFields& runner,
    const std::uint8_t runner_feature_flags) {
    // Preserve the source mode, active runner's +0x8C statistic, feature
    // flags, and ROM action-cost table selector for each field shop.
    return FieldActionCostContext{
        menu_mode, runner_feature_flags, runner.skills[11], 2U};
}

std::optional<std::uint16_t> source_delay_mode(
    const std::string_view scale) {
    if (scale == "normal") {
        return 0U;
    }
    if (scale == "half") {
        return 1U;
    }
    if (scale == "double") {
        return 2U;
    }
    return std::nullopt;
}

void draw_command_overview_number(
    QPainter& painter, const QImage& font, const int x, const int y,
    const std::uint32_t value,
    const NumericVisualAlignment alignment = NumericVisualAlignment::left,
    const bool group_thousands = false) {
    draw_numeric_visual(painter, font, x, y, value, alignment,
                        group_thousands, QColor(0, 255, 0));
}

FieldActionPurchaseState make_purchase_state(
    const RunnerRecordFields& runner, const std::uint32_t funds) {
    FieldActionPurchaseState state{};
    state.funds = funds;
    state.runner.action_codes = runner.inventory_item_codes;
    state.runner.companion_values = runner.inventory_phase_bytes;
    state.runner.special_action_count = runner.ammo_clips;
    return state;
}

RunnerActionRemovalState make_sale_state(
    const RunnerRecordFields& runner, const std::uint8_t slot,
    const std::uint32_t funds) {
    RunnerActionRemovalState state{};
    state.runner.action_codes = runner.inventory_item_codes;
    state.runner.companion_values = runner.inventory_phase_bytes;
    state.runner_slot = slot;
    state.selected_runner_slot = runner.equipped_inventory_slot < 0
        ? 0xFFU
        : static_cast<std::uint8_t>(runner.equipped_inventory_slot);
    state.action_suffix_marker = runner.equipped_armor_index;
    state.funds = funds;
    return state;
}

void apply_purchase_state(const FieldActionPurchaseState& state,
                          RunnerRecordFields& runner) {
    runner.inventory_item_codes = state.runner.action_codes;
    runner.inventory_phase_bytes = state.runner.companion_values;
    runner.ammo_clips = state.runner.special_action_count;
}

constexpr std::size_t kFieldSpellProgressionRecordCapacity = 8U;

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

std::optional<MagicSpellTable> spell_table_from_field_progression(
    const FieldActionProgressionPurchaseState& state) {
    if (state.ledger.source_record_count_word >=
        kFieldSpellProgressionRecordCapacity) {
        return std::nullopt;
    }
    const auto entry_count = static_cast<std::size_t>(
        state.ledger.source_record_count_word) + 1U;
    if (entry_count > state.ledger.records.size() ||
        entry_count > kMagicSpellTableEntryCapacity) {
        return std::nullopt;
    }
    MagicSpellTable result{};
    result.entry_count = static_cast<std::uint8_t>(entry_count);
    for (std::size_t index = 0U; index < entry_count; ++index) {
        result.entries[index] = {
            state.ledger.records[index].action_code,
            state.ledger.records[index].progression_level};
    }
    return result;
}

struct FieldShopEquipmentUpgradeInput {
    FieldEquipmentUpgradeTable table{};
    FieldEquipmentUpgradeState state{};
};

std::optional<FieldShopEquipmentUpgradeInput>
make_field_shop_equipment_upgrade_input(
    const scripting::ScriptShopDefinition& definition,
    const ShopMenuController& menu,
    const scripting::ScriptShopEquipmentUpgradeOffer& offer,
    const RunnerRecordFields& runner, const std::uint32_t funds,
    const MagicSpellTable* spell_table,
    const std::uint8_t runner_feature_flags) {
    if (definition.menu_result != 1 || definition.menu_mode != 7U ||
        definition.equipment_upgrade_meter_thresholds.size() != 16U ||
        offer.category >= 8U) {
        return std::nullopt;
    }
    FieldShopEquipmentUpgradeInput result{};
    std::copy(definition.equipment_upgrade_meter_thresholds.begin(),
              definition.equipment_upgrade_meter_thresholds.end(),
              result.table.meter_thresholds.begin());

    auto& state = result.state;
    state.upgrade_flags = runner.cyberware_flags;
    state.selection_category = offer.category;
    state.cost = apply_field_action_cost_discounts(
        offer.base_cost,
        field_shop_cost_context(menu.menu_mode(), runner,
                               runner_feature_flags));
    state.funds = funds;
    state.meter_tens = runner.cyberware_essence_tens();
    state.meter_ones = runner.cyberware_essence_ones();
    state.saved_meter_tens = runner.auxiliary[0U];
    state.archetype_status = static_cast<std::int8_t>(runner.archetype_index);
    if (state.archetype_status < 2) {
        if (spell_table == nullptr ||
            spell_table->entry_count > kMagicSpellTableEntryCapacity) {
            return std::nullopt;
        }
        state.tracked_meter_values.reserve(spell_table->entry_count);
        for (std::size_t index = 0U; index < spell_table->entry_count; ++index) {
            state.tracked_meter_values.push_back(
                spell_table->entries[index].force_limit);
        }
    }
    return result;
}

std::optional<MagicSpellTable> spell_table_after_equipment_upgrade(
    const MagicSpellTable& source,
    const FieldEquipmentUpgradeState& upgrade) {
    if (upgrade.archetype_status >= 2) {
        return source;
    }
    if (upgrade.tracked_meter_values.size() != source.entry_count) {
        return std::nullopt;
    }
    auto result = source;
    for (std::size_t index = 0U; index < result.entry_count; ++index) {
        result.entries[index].force_limit =
            upgrade.tracked_meter_values[index];
    }
    return result;
}

void apply_sale_state(const RunnerActionRemovalState& state,
                      RunnerRecordFields& runner) {
    runner.inventory_item_codes = state.runner.action_codes;
    runner.inventory_phase_bytes = state.runner.companion_values;
    runner.equipped_armor_index = state.action_suffix_marker;
    if (state.menu_selection_cleared) {
        runner.equipped_inventory_slot = -1;
        runner.equipped_item_code = 0U;
        // ROM 0x56DC4 sets FFF098 when the sold row is +0x56; 0x1275C then
        // clears the active record's +0x4E loaded rounds with +0x56/+0x57.
        runner.ammo_rounds = 0U;
    }
}

std::string command_overview_item_alias(
    const scripting::ScriptItemDefinition& item,
    const std::uint8_t palette_variant) {
    if (item.code == 0U || item.presentation.icon_resource_key.empty()) {
        return {};
    }
    return item.presentation.icon_resource_key + "." +
           std::to_string(palette_variant);
}

std::string command_overview_item_detail_alias(
    const scripting::ScriptItemDefinition& item) {
    return item.presentation.detail_resource_key;
}

bool command_overview_uses_alternate_icon_palette(
    const RunnerRecordFields& record) {
    return use_command_overview_alternate_icon_palette(
        HudGlyphAttributeInput{
            static_cast<std::int8_t>(record.equipped_item_code),
            static_cast<std::int8_t>(record.archetype_index),
            record.equipped_inventory_slot});
}

std::array<std::uint8_t, 4> field_sample_state_from_result(
    const std::uint32_t packed_state) {
    return {
        static_cast<std::uint8_t>(packed_state >> 24U),
        static_cast<std::uint8_t>(packed_state >> 16U),
        static_cast<std::uint8_t>(packed_state >> 8U),
        static_cast<std::uint8_t>(packed_state)};
}



} // namespace

FieldSceneRuntime::FieldSceneRuntime(
    std::function<void(std::string_view)> source_boundary,
    std::function<void(std::uint16_t)> source_audio_event,
    std::function<void(std::uint16_t)> source_audio_stop)
    : source_boundary_callback_(std::move(source_boundary)),
      source_audio_event_callback_(std::move(source_audio_event)),
      source_audio_stop_callback_(std::move(source_audio_stop)),
      field_dialogue_([this](const std::string_view boundary) {
          this->source_boundary(boundary);
      }, StokerMotelArrivalCommandCallbacks{
          [this](const std::uint8_t bound)
              -> std::optional<std::uint8_t> {
              if (bound == 0U) {
                  return std::nullopt;
              }
              const auto sample = sample_source_random_bound(
                  field_random_seed_, static_cast<std::uint16_t>(bound));
              if (sample < 0 || sample >= static_cast<std::int16_t>(bound)) {
                  return std::nullopt;
              }
              return static_cast<std::uint8_t>(sample);
          },
          {}}),
      field_interaction_(
          [this](const std::string_view boundary) {
              this->source_boundary(boundary);
          },
          source_audio_event_callback_, source_audio_stop_callback_) {
    register_dialogue_outcome_handlers();
    hospital_random_grid_cells_.fill(-1);
    field_dialogue_.set_script_runtime(&game_scripts_);
    field_interaction_.set_script_runtime(&game_scripts_);
    field_dialogue_.set_room_payment_callback([this]()
        -> std::optional<StokerMotelRoomPaymentOutcome> {
        if (!field_funds_.has_value()) {
            return std::nullopt;
        }
        StokerMotelRoomPaymentState payment_state{};
        payment_state.funds = *field_funds_;
        const auto outcome = resolve_stoker_motel_room_payment(payment_state);
        field_funds_ = outcome.remaining_funds;
        return outcome;
    });
    field_dialogue_.set_debt_payment_callback([this]()
        -> std::optional<StokerMotelDebtPaymentOutcome> {
        constexpr std::uint32_t kDebt = 250U;
        if (!field_funds_.has_value() || !notebook_tip_flags_resolved_ ||
            !notebook_referral_flags_.has_value()) {
            // Story and payment state is not restored by the current save
            // adapter, so keep loaded-session writes explicit.
            return std::nullopt;
        }
        const bool paid_branch = *field_funds_ >= kDebt;
        if (paid_branch &&
            (!field_runner_feature_flags_.has_value() ||
             !field_entity_action_phase_roll_counters_.has_value())) {
            // The successful source branch also updates FF018F and the three
            // reaction counters. Those stay unresolved for restored saves.
            return std::nullopt;
        }

        StokerMotelDebtPaymentState payment_state{};
        payment_state.funds = *field_funds_;
        payment_state.world_flags = field_exit_world_flags_;
        // The live Stoker branch is title-service table entry zero at ROM
        // 0x53AB6; 0x53A78 publishes that index in FFDD64 before dispatch,
        // and the paid branch's 0x2196C marks whatever FFDD64 holds. The
        // short-funds branch (0x213E2) never reads FFDD64.
        if (paid_branch && !field_active_story_trigger_.has_value()) {
            this->source_boundary("field.dialogue.stoker_debt_without_trigger");
            return std::nullopt;
        }
        payment_state.effect_marker =
            field_active_story_trigger_.value_or(0U);
        payment_state.runner_feature_flags =
            field_runner_feature_flags_.value_or(0U);
        payment_state.followup_state.story_flags = notebook_tip_flags_[0];
        payment_state.followup_state.referral_flags =
            *notebook_referral_flags_;
        payment_state.runner_source_bytes =
            field_entity_action_phase_roll_counters_.value_or(
                std::array<std::uint8_t, 3>{});
        const auto outcome =
            resolve_stoker_motel_debt_payment(payment_state);
        field_funds_ = payment_state.funds;
        if (outcome.result == StokerMotelDebtPaymentResult::debt_paid) {
            field_exit_world_flags_ = payment_state.world_flags;
            field_runner_feature_flags_ =
                payment_state.runner_feature_flags;
            field_entity_action_phase_roll_counters_ =
                payment_state.runner_source_bytes;
            cyberdeck_available_ =
                (payment_state.runner_feature_flags & 0x01U) != 0U;
        }
        notebook_tip_flags_[0] = payment_state.followup_state.story_flags;
        notebook_referral_flags_ =
            payment_state.followup_state.referral_flags;
        return outcome;
    });
    field_dialogue_.set_followup_command_callback(
        [this](const StokerMotelFollowupCommand command)
            -> std::optional<StokerMotelFollowupOutcome> {
            StokerMotelFollowupState followup_state{};
            if (command == StokerMotelFollowupCommand::debt_collection) {
                if (!notebook_tip_flags_resolved_) {
                    return std::nullopt;
                }
                followup_state.story_flags = notebook_tip_flags_[0];
            } else {
                if (!notebook_referral_flags_.has_value()) {
                    return std::nullopt;
                }
                followup_state.referral_flags =
                    *notebook_referral_flags_;
            }

            const auto outcome = resolve_stoker_motel_followup_command(
                command, followup_state);
            if (command == StokerMotelFollowupCommand::debt_collection) {
                notebook_tip_flags_[0] = followup_state.story_flags;
            } else {
                notebook_referral_flags_ = followup_state.referral_flags;
            }
            return outcome;
        });
    field_dialogue_.set_native_script_service_callback(
        [this](const std::string_view service_id)
            -> std::optional<std::string> {
            constexpr std::string_view kBoundedRandomPrefix =
                "random.bounded_";
            if (service_id.rfind(kBoundedRandomPrefix, 0U) == 0U) {
                const auto bound_text = service_id.substr(
                    kBoundedRandomPrefix.size());
                std::uint32_t bound = 0U;
                const auto parsed = std::from_chars(
                    bound_text.data(), bound_text.data() + bound_text.size(),
                    bound);
                if (bound_text.empty() ||
                    parsed.ec != std::errc{} ||
                    parsed.ptr != bound_text.data() + bound_text.size() ||
                    bound == 0U || bound > 0x7FFFU) {
                    return std::nullopt;
                }
                const auto roll = sample_source_random_bound(
                    field_random_seed_, static_cast<std::uint16_t>(bound));
                return "roll_" +
                       std::to_string(static_cast<std::uint16_t>(roll));
            }
            if (service_id == "runner.negotiation_skill") {
                if (!field_party_roster_.available()) {
                    return std::nullopt;
                }
                const auto skill_index = static_cast<std::size_t>(
                    RunnerSkillIndex::negotiation);
                const auto& first_runner =
                    field_party_roster_.records().front();
                if (skill_index >= first_runner.skills.size()) {
                    return std::nullopt;
                }
                return "skill_" + std::to_string(
                    first_runner.skills[skill_index]);
            }
            if (service_id == "field.mode_action") {
                // ROM 0xC9EA routes Hospital Options B through the same
                // 0x391A active-runner action used by the field controller.
                if (source_audio_event_callback_) {
                    source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
                }
                handle_field_mode_action_event();
                return "applied";
            }
            constexpr std::string_view kAwardKarmaPrefix =
                "party.award_karma_";
            if (service_id.rfind(kAwardKarmaPrefix, 0U) == 0U) {
                const auto amount_text =
                    service_id.substr(kAwardKarmaPrefix.size());
                std::uint32_t amount = 0U;
                const auto parsed = std::from_chars(
                    amount_text.data(),
                    amount_text.data() + amount_text.size(), amount);
                if (amount_text.empty() || parsed.ec != std::errc{} ||
                    parsed.ptr != amount_text.data() + amount_text.size() ||
                    amount > 0xFFU || !field_party_roster_.available()) {
                    return std::nullopt;
                }
                for (auto& runner : field_party_roster_.records()) {
                    runner.auxiliary[2U] = static_cast<std::uint8_t>(
                        runner.auxiliary[2U] + amount);
                }
                return "applied";
            }
            if (service_id == "field.scene_reference") {
                return "scene_" + std::to_string(
                    state_.field_scene_buffers.scene_reference);
            }
            // Generic story bytes. Lua names them by ROM address
            // (E.StoryByte); unknown addresses or unrestored values are nil.
            const auto story_byte = [this](const std::string_view hex)
                -> std::optional<std::uint8_t>* {
                std::uint32_t address = 0U;
                const auto parsed = std::from_chars(
                    hex.data(), hex.data() + hex.size(), address, 16);
                if (hex.empty() || parsed.ec != std::errc{} ||
                    parsed.ptr != hex.data() + hex.size()) {
                    return nullptr;
                }
                switch (address) {
                case 0xFFFBFAU: return &field_heat_level_;
                case 0xFFFBFBU: return &field_strike_team_counter_;
                default: return nullptr;
                }
            };
            constexpr std::string_view kStoryByteGetPrefix =
                "story.byte_get_";
            if (service_id.rfind(kStoryByteGetPrefix, 0U) == 0U) {
                auto* value =
                    story_byte(service_id.substr(kStoryByteGetPrefix.size()));
                if (value == nullptr || !value->has_value()) {
                    return std::nullopt;
                }
                return "value_" + std::to_string(**value);
            }
            constexpr std::string_view kStoryByteSetPrefix =
                "story.byte_set_";
            if (service_id.rfind(kStoryByteSetPrefix, 0U) == 0U) {
                const auto args = service_id.substr(kStoryByteSetPrefix.size());
                const auto split = args.find('_');
                if (split == std::string_view::npos) {
                    return std::nullopt;
                }
                auto* target = story_byte(args.substr(0U, split));
                const auto value_text = args.substr(split + 1U);
                unsigned value = 0U;
                const auto parsed = std::from_chars(
                    value_text.data(), value_text.data() + value_text.size(),
                    value);
                if (target == nullptr || value_text.empty() ||
                    parsed.ec != std::errc{} ||
                    parsed.ptr != value_text.data() + value_text.size() ||
                    value > 0xFFU) {
                    return std::nullopt;
                }
                *target = static_cast<std::uint8_t>(value);
                return "applied";
            }
            if (service_id == "story.world_effect_idle") {
                return field_pending_story_world_effect_.has_value() ||
                               pending_field_encounter_outcome_.has_value()
                           ? "busy"
                           : "idle";
            }
            // story.queue_world_effect_<scene>_<list hex>_<actors>_<name>
            constexpr std::string_view kQueueWorldEffectPrefix =
                "story.queue_world_effect_";
            if (service_id.rfind(kQueueWorldEffectPrefix, 0U) == 0U) {
                auto rest = service_id.substr(kQueueWorldEffectPrefix.size());
                std::array<std::uint32_t, 3U> numbers{};
                for (std::size_t index = 0U; index < numbers.size(); ++index) {
                    const auto split = rest.find('_');
                    if (split == std::string_view::npos) {
                        return std::nullopt;
                    }
                    const auto text = rest.substr(0U, split);
                    const auto parsed = std::from_chars(
                        text.data(), text.data() + text.size(), numbers[index],
                        index == 1U ? 16 : 10);
                    if (text.empty() || parsed.ec != std::errc{} ||
                        parsed.ptr != text.data() + text.size()) {
                        return std::nullopt;
                    }
                    rest = rest.substr(split + 1U);
                }
                if (rest.empty() || numbers[2] == 0U || numbers[2] > 0xFFU ||
                    numbers[0] != state_.field_scene_buffers.scene_reference ||
                    field_pending_story_world_effect_.has_value() ||
                    pending_field_encounter_outcome_.has_value()) {
                    return std::nullopt;
                }
                field_pending_story_world_effect_ = PendingStoryWorldEffect{
                    static_cast<std::uint16_t>(numbers[0]), numbers[1],
                    static_cast<std::uint8_t>(numbers[2]), std::string(rest)};
                this->source_boundary(
                    "field.world_activity." + std::string(rest) +
                    "_list_queued");
                return "queued";
            }
            // field.queue_route_actor_list_<consume scene>_<source scene>_
            //     <list hex>_<transitions>_<name>
            constexpr std::string_view kQueueRouteActorListPrefix =
                "field.queue_route_actor_list_";
            if (service_id.rfind(kQueueRouteActorListPrefix, 0U) == 0U) {
                if (field_pending_route_actor_list_.has_value() ||
                    field_pending_actor_list_transition_group_.has_value() ||
                    field_route_actor_transitions_remaining_.has_value() ||
                    field_pending_actor_list_callback_.has_value() ||
                    !pending_world_activity_script_actors_.empty()) {
                    return std::nullopt;
                }
                auto rest =
                    service_id.substr(kQueueRouteActorListPrefix.size());
                std::array<std::uint32_t, 4U> numbers{};
                for (std::size_t index = 0U; index < numbers.size(); ++index) {
                    const auto split = rest.find('_');
                    if (split == std::string_view::npos) {
                        return std::nullopt;
                    }
                    const auto text = rest.substr(0U, split);
                    const auto parsed = std::from_chars(
                        text.data(), text.data() + text.size(), numbers[index],
                        index == 2U ? 16 : 10);
                    if (text.empty() || parsed.ec != std::errc{} ||
                        parsed.ptr != text.data() + text.size()) {
                        return std::nullopt;
                    }
                    rest = rest.substr(split + 1U);
                }
                if (rest.empty() || numbers[0] > 0xFFFFU ||
                    numbers[1] > 0xFFFFU || numbers[3] == 0U ||
                    numbers[3] > 0xFFU) {
                    return std::nullopt;
                }
                field_pending_route_actor_list_ = FieldRoutePendingActorList{
                    static_cast<std::uint16_t>(numbers[0]),
                    static_cast<std::uint16_t>(numbers[1]), numbers[2],
                    static_cast<std::uint8_t>(numbers[3]), std::string(rest)};
                this->source_boundary("field.world_activity." +
                                      std::string(rest) + "_list_queued");
                return "queued";
            }
            constexpr std::string_view kRunnerTripletPrefix =
                "runner.adjust_source_byte_triplet_";
            if (service_id.rfind(kRunnerTripletPrefix, 0U) == 0U) {
                const auto delta_text = service_id.substr(
                    kRunnerTripletPrefix.size());
                unsigned delta = 0U;
                const auto parsed = std::from_chars(
                    delta_text.data(), delta_text.data() + delta_text.size(),
                    delta);
                if (delta_text.empty() || parsed.ec != std::errc{} ||
                    parsed.ptr != delta_text.data() + delta_text.size() ||
                    delta > 0xFFU ||
                    !field_entity_action_phase_roll_counters_.has_value()) {
                    return std::nullopt;
                }
                auto source_bytes =
                    *field_entity_action_phase_roll_counters_;
                adjust_runner_source_byte_triplet(
                    source_bytes, static_cast<std::uint8_t>(delta));
                field_entity_action_phase_roll_counters_ = source_bytes;
                return "applied";
            }
            constexpr std::string_view kBuildingChargePrefix =
                "building.charge_";
            if (service_id.rfind(kBuildingChargePrefix, 0U) == 0U) {
                const auto amount_text = service_id.substr(
                    kBuildingChargePrefix.size());
                std::uint32_t amount = 0U;
                const auto parsed = std::from_chars(
                    amount_text.data(),
                    amount_text.data() + amount_text.size(), amount);
                if (amount_text.empty() || parsed.ec != std::errc{} ||
                    parsed.ptr != amount_text.data() + amount_text.size() ||
                    !field_funds_.has_value()) {
                    return std::nullopt;
                }
                if (*field_funds_ < amount) {
                    return "insufficient";
                }
                *field_funds_ -= amount;
                return "paid";
            }
            constexpr std::string_view kRunnerItemPurchasePrefix =
                "building.purchase_runner_item_";
            if (service_id.rfind(kRunnerItemPurchasePrefix, 0U) == 0U) {
                auto remaining = service_id.substr(
                    kRunnerItemPurchasePrefix.size());
                std::array<std::uint32_t, 3> values{};
                for (std::size_t index = 0U; index < values.size(); ++index) {
                    const auto separator = remaining.find('_');
                    if ((index + 1U < values.size() &&
                         separator == std::string_view::npos) ||
                        (index + 1U == values.size() &&
                         separator != std::string_view::npos)) {
                        return std::nullopt;
                    }
                    const auto value_text = separator == std::string_view::npos
                        ? remaining
                        : remaining.substr(0U, separator);
                    const auto parsed = std::from_chars(
                        value_text.data(),
                        value_text.data() + value_text.size(), values[index]);
                    if (value_text.empty() || parsed.ec != std::errc{} ||
                        parsed.ptr != value_text.data() + value_text.size()) {
                        return std::nullopt;
                    }
                    if (separator != std::string_view::npos) {
                        remaining.remove_prefix(separator + 1U);
                    }
                }
                if (values[0] == 0U || values[0] > 0xFFU ||
                    values[1] > 0xFFU || values[2] > 999999U ||
                    !field_party_roster_.available()) {
                    return std::nullopt;
                }
                const auto item_code = static_cast<std::uint8_t>(values[0]);
                const auto* item = find_us_item_definition(item_code);
                if (item == nullptr || item->item_code != item_code) {
                    return std::nullopt;
                }

                auto& runner = field_party_roster_.active_record();
                std::size_t free_slot = 0U;
                while (free_slot < runner.inventory_item_codes.size() &&
                       runner.inventory_item_codes[free_slot] != 0U) {
                    ++free_slot;
                }
                // ROM 0x22874 runs before the 0x21A4E wallet helper, so a
                // full inventory wins over an insufficient-funds result.
                if (free_slot == runner.inventory_item_codes.size()) {
                    return "inventory_full";
                }
                if (!field_funds_.has_value()) {
                    return std::nullopt;
                }
                if (*field_funds_ < values[2]) {
                    return "insufficient";
                }

                *field_funds_ -= values[2];
                runner.inventory_item_codes[free_slot] = item_code;
                runner.inventory_phase_bytes[free_slot] =
                    static_cast<std::uint8_t>(values[1]);
                refresh_field_hud_runner_slot();
                refresh_command_overview_combat_bars();
                refresh_command_overview_item_detail();
                return "purchased";
            }
            // Passage destination and fare are authored in the Lua contact
            // data. The host validates the requested scene, applies the
            // wallet transaction, and schedules the shared scene transition.
            const auto passage_request =
                parse_vid_phone_passage_request(service_id);
            if (passage_request.has_value()) {
                const auto scene_value = passage_request->destination_scene;
                if (!field_session_source_assets_ ||
                    scene_value >= field_session_source_assets_
                                       ->field_scene_resources.size() ||
                    field_phone_pending_destination_.has_value()) {
                    return std::nullopt;
                }
                const auto result = apply_vid_phone_passage_fare(
                    passage_request->price, field_funds_);
                if (result == VidPhonePassageResult::passage_requested) {
                    field_phone_pending_destination_ = scene_value;
                    this->source_boundary(
                        "field.vid_phone.passage.payment_accepted");
                }
                const auto script_result =
                    vid_phone_passage_script_result(result);
                return script_result.has_value()
                           ? std::optional<std::string>{
                                 std::string(*script_result)}
                           : std::nullopt;
            }
            const auto deck_action = vid_phone_deck_action_for_id(service_id);
            if (deck_action.has_value()) {
                const auto result = apply_vid_phone_deck_service(
                    *deck_action, field_funds_, field_matrix_deck_record_);
                const auto script_result = vid_phone_deck_script_result(result);
                return script_result.has_value()
                           ? std::optional<std::string>{
                                 std::string(*script_result)}
                           : std::nullopt;
            }
            const auto vid_phone_action =
                vid_phone_service_action_for_id(service_id);
            if (vid_phone_action.has_value()) {
                if (!field_party_roster_.available()) {
                    return std::nullopt;
                }
                auto& runner = field_party_roster_.active_record();
                const auto result = apply_vid_phone_service(
                    *vid_phone_action, field_funds_,
                    runner.inventory_item_codes,
                    runner.inventory_phase_bytes,
                    notebook_contacts_flags_, &field_runner_feature_flags_,
                    static_cast<std::int8_t>(runner.archetype_index),
                    &group_item_flags_);
                if (result == VidPhoneServiceResult::purchased) {
                    refresh_field_hud_runner_slot();
                    refresh_command_overview_combat_bars();
                    refresh_command_overview_item_detail();
                }
                const auto script_result =
                    vid_phone_service_script_result(result);
                return script_result.has_value()
                           ? std::optional<std::string>{
                                 std::string(*script_result)}
                           : std::nullopt;
            }
            return std::nullopt;
        });
    configure_callbacks();
}

void FieldSceneRuntime::source_boundary(
    const std::string_view boundary) const {
    if (source_boundary_callback_) {
        source_boundary_callback_(boundary);
    }
}

void FieldSceneRuntime::set_actor_heartbeat_observer(
    scripting::ScriptActorHeartbeatObserver observer) {
    game_scripts_.set_actor_heartbeat_observer(std::move(observer));
}

QJsonObject FieldSceneRuntime::debug_inspection_fields() const {
    QJsonObject fields;
    const auto scene = state_.field_scene_buffers.scene_reference;
    fields.insert(QStringLiteral("field.scene"), scene);
    fields.insert(QStringLiteral("field.mode"),
                  field_movement_transition_state_.world_mode);
    fields.insert(QStringLiteral("field.player.x"),
                  field_entity_.world_position.x);
    fields.insert(QStringLiteral("field.player.y"),
                  field_entity_.world_position.y);
    fields.insert(QStringLiteral("field.player.heading"),
                  field_movement_state_.heading);
    fields.insert(QStringLiteral("field.player.direction"),
                  field_entity_.direction);
    fields.insert(QStringLiteral("field.player.target_heading"),
                  field_target_heading_);
    fields.insert(QStringLiteral("field.route.active"),
                  active_field_script_route_.has_value());
    if (active_field_script_route_.has_value()) {
        const auto& route = *active_field_script_route_;
        const auto route_kind_name = [](const FieldScriptRouteKind kind) {
            switch (kind) {
            case FieldScriptRouteKind::dialogue:
                return QStringLiteral("dialogue");
            case FieldScriptRouteKind::field_service:
                return QStringLiteral("field_service");
            case FieldScriptRouteKind::area_event:
                return QStringLiteral("area_event");
            case FieldScriptRouteKind::source_activity_return:
                return QStringLiteral("source_activity_return");
            case FieldScriptRouteKind::source_boundary:
                return QStringLiteral("source_boundary");
            case FieldScriptRouteKind::no_route:
                return QStringLiteral("no_route");
            }
            return QStringLiteral("unknown");
        };
        const auto script_name = [](const FieldScriptId script) {
            switch (script) {
            case FieldScriptId::lua_dialogue:
                return QStringLiteral("lua_dialogue");
            case FieldScriptId::lua_field_service:
                return QStringLiteral("lua_field_service");
            case FieldScriptId::host_field_service:
                return QStringLiteral("host_field_service");
            case FieldScriptId::area_event_table:
                return QStringLiteral("area_event_table");
            case FieldScriptId::source_activity_return:
                return QStringLiteral("source_activity_return");
            case FieldScriptId::source_boundary:
                return QStringLiteral("source_boundary");
            case FieldScriptId::none:
                return QStringLiteral("none");
            }
            return QStringLiteral("unknown");
        };
        const auto destination_mode_name = [](
            const std::optional<WorldActivityMode> mode) {
            if (!mode.has_value()) {
                return QStringLiteral("none");
            }
            switch (*mode) {
            case WorldActivityMode::field:
                return QStringLiteral("field");
            case WorldActivityMode::resource_activity:
                return QStringLiteral("resource_activity");
            case WorldActivityMode::scripted_activity:
                return QStringLiteral("scripted_activity");
            case WorldActivityMode::halted:
                return QStringLiteral("halted");
            }
            return QStringLiteral("unknown");
        };
        fields.insert(QStringLiteral("field.route.kind"),
                      route_kind_name(route.kind));
        fields.insert(QStringLiteral("field.route.script"),
                      script_name(route.script));
        fields.insert(QStringLiteral("field.route.event_code"),
                      route.event_code);
        fields.insert(QStringLiteral("field.route.target_scene"),
                      route.area_event_target.has_value()
                          ? QJsonValue(*route.area_event_target)
                          : QJsonValue(QJsonValue::Null));
        fields.insert(QStringLiteral("field.route.destination_mode"),
                      destination_mode_name(
                          route.area_event_destination_mode));
        fields.insert(QStringLiteral("field.route.return_anchor"),
                      route.return_anchor ==
                              FieldLocationReturnAnchor::collision_probe_lead
                          ? QStringLiteral("collision_probe_lead")
                          : QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.return_facing"),
                      route.return_facing ==
                              FieldLocationReturnFacing::face_away_from_entry
                          ? QStringLiteral("face_away_from_entry")
                          : QStringLiteral("preserve_entry_heading"));
        fields.insert(QStringLiteral("field.route.entry_dialogue"),
                      !route.entry_dialogue_module.empty());
        fields.insert(QStringLiteral("field.route.entry_module"),
                      QString::fromStdString(route.entry_dialogue_module));
        fields.insert(QStringLiteral("field.route.entry_function"),
                      QString::fromStdString(route.entry_dialogue_function));
        fields.insert(QStringLiteral("field.route.preparation"),
                      route.route_preparation.has_value());
        fields.insert(QStringLiteral("field.route.preparation_module"),
                      route.route_preparation.has_value()
                          ? QString::fromStdString(
                                route.route_preparation->module)
                          : QString());
        fields.insert(QStringLiteral("field.route.preparation_function"),
                      route.route_preparation.has_value()
                          ? QString::fromStdString(
                                route.route_preparation->function)
                          : QString());
    } else {
        fields.insert(QStringLiteral("field.route.kind"),
                      QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.script"),
                      QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.event_code"), 255);
        fields.insert(QStringLiteral("field.route.target_scene"),
                      QJsonValue(QJsonValue::Null));
        fields.insert(QStringLiteral("field.route.destination_mode"),
                      QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.return_anchor"),
                      QStringLiteral("none"));
        fields.insert(QStringLiteral("field.route.return_facing"),
                      QStringLiteral("preserve_entry_heading"));
        fields.insert(QStringLiteral("field.route.entry_dialogue"), false);
        fields.insert(QStringLiteral("field.route.entry_module"),
                      QString());
        fields.insert(QStringLiteral("field.route.entry_function"),
                      QString());
    }
    fields.insert(QStringLiteral("field.camera.x"),
                  state_.field_scene_buffers.camera_x);
    fields.insert(QStringLiteral("field.camera.y"),
                  state_.field_scene_buffers.camera_y);
    fields.insert(QStringLiteral("field.world.frame"),
                  field_world_frame_counter_);
    fields.insert(QStringLiteral("field.world.pending_effect"),
                  field_pending_world_effect_);
    fields.insert(QStringLiteral("field.world.active_effect"),
                  field_world_effect_active_);
    fields.insert(QStringLiteral("field.world.encounter_sequence"),
                  static_cast<double>(field_world_encounter_sequence_));
    fields.insert(QStringLiteral("field.world.random_seed"),
                  field_random_seed_);
    // The frame debugger exposes these typed host gates so a replay can show
    // why a field frame is waiting. They are diagnostics, not a second game
    // state representation and not ROM RAM reads.
    fields.insert(QStringLiteral("field.debug.runtime_ready"), ready_);
    fields.insert(QStringLiteral("field.debug.field_activity_lock"),
                  field_runtime_state_.field_activity_lock);
    fields.insert(QStringLiteral("field.debug.world_activity_countdown"),
                  field_runtime_state_.world_activity_countdown);
    fields.insert(
        QStringLiteral("field.debug.mako_actor_transitions_remaining"),
        field_route_actor_transitions_remaining_.has_value()
            ? QJsonValue(static_cast<int>(
                  *field_route_actor_transitions_remaining_))
            : QJsonValue(-1));
    fields.insert(QStringLiteral("field.debug.actor_list_callback_pending"),
                  field_pending_actor_list_callback_.has_value());
    fields.insert(QStringLiteral("field.debug.heat_level"),
                  field_heat_level_.has_value()
                      ? QJsonValue(*field_heat_level_)
                      : QJsonValue(-1));
    fields.insert(QStringLiteral("field.debug.notebook_tip_flags_resolved"),
                  notebook_tip_flags_resolved_);
    fields.insert(QStringLiteral("field.debug.mode_flags_resolved"),
                  field_mode_flags_resolved_);
    fields.insert(QStringLiteral("field.debug.party_roster_available"),
                  field_party_roster_.available());
    fields.insert(QStringLiteral("field.debug.mako_result_state_available"),
                  field_heat_level_.has_value() &&
                      field_strike_team_counter_.has_value() &&
                      notebook_tip_flags_resolved_ &&
                      field_mode_flags_resolved_ &&
                      field_party_roster_.available());
    fields.insert(QStringLiteral("field.debug.strike_team_counter"),
                  field_strike_team_counter_.has_value()
                      ? QJsonValue(*field_strike_team_counter_)
                      : QJsonValue(-1));
    fields.insert(QStringLiteral("field.debug.display_service_word"),
                  field_runtime_state_.display_service_word);
    fields.insert(QStringLiteral("field.debug.pending_scene_handler"),
                  field_runtime_state_.pending_field_scene_handler);
    fields.insert(QStringLiteral("field.debug.scene_transition_requested"),
                  field_runtime_state_.field_transition_requested);
    fields.insert(QStringLiteral("field.debug.opening_auto_walk"),
                  field_opening_auto_walk_.active);
    fields.insert(QStringLiteral("field.debug.interaction_transition"),
                  field_movement_transition_state_.interaction_transition_pending);
    fields.insert(QStringLiteral("field.debug.interaction_transition_timer"),
                  field_movement_transition_state_.interaction_transition_timer);
    fields.insert(QStringLiteral("field.debug.controller_port"),
                  field_controller_port_byte_);
    fields.insert(QStringLiteral("field.debug.controller_held"),
                  field_controller_sample_state_.held);
    fields.insert(QStringLiteral("field.debug.controller_events"),
                  field_controller_sample_state_.events);
    fields.insert(QStringLiteral("field.ui.command_overview"),
                  command_overview_.open());
    fields.insert(QStringLiteral("field.ui.dialogue"),
                  field_dialogue_.open());
    fields.insert(QStringLiteral("field.ui.interaction"),
                  field_interaction_.open());
    fields.insert(QStringLiteral("field.ui.hotel"), hotel_menu_.open());
    fields.insert(QStringLiteral("field.ui.hotel_karma"),
                  hotel_karma_skills_.is_open());
    fields.insert(QStringLiteral("field.ui.debug_menu_unlocked"),
                  debug_menu_unlocked_);

    std::size_t actor_count = 0U;
    for (const auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != scene) {
            continue;
        }
        ++actor_count;
        const auto actor_id = actor.actor_instance_id.empty()
                                  ? std::to_string(actor.source_order)
                                  : actor.actor_instance_id;
        const auto prefix =
            QStringLiteral("actor.%1.").arg(QString::fromStdString(actor_id));
        fields.insert(prefix + QStringLiteral("profile"),
                      QString::fromStdString(actor.script_profile_id));
        fields.insert(prefix + QStringLiteral("kind"), actor.entity_kind);
        fields.insert(prefix + QStringLiteral("source_order"),
                      actor.source_order);
        fields.insert(prefix + QStringLiteral("record_slot"),
                      actor.source_record_slot.has_value()
                          ? QJsonValue(static_cast<int>(
                                *actor.source_record_slot))
                          : QJsonValue(QJsonValue::Null));
        fields.insert(prefix + QStringLiteral("entity_flags"),
                      actor.source_record_flags); // source +0x10
        fields.insert(prefix + QStringLiteral("collision_flags"),
                      actor.source_interaction_flags); // source +0x11
        fields.insert(prefix + QStringLiteral("special_flags"),
                      actor.source_spawn_state_flags); // source +0x9E
        fields.insert(prefix + QStringLiteral("activity_flags"),
                      actor.source_behavior_flags); // source +0xB0
        fields.insert(prefix + QStringLiteral("world_activity_script_actor"),
                      actor.world_activity_script_actor);
        fields.insert(prefix + QStringLiteral("x"), actor.world_position.x);
        fields.insert(prefix + QStringLiteral("y"), actor.world_position.y);
        fields.insert(prefix + QStringLiteral("action"),
                      static_cast<int>(actor.action_kind));
        fields.insert(prefix + QStringLiteral("animation"),
                      QString::fromStdString(actor.animation_id));
        fields.insert(prefix + QStringLiteral("animation_direction"),
                      actor.animation_direction);
        fields.insert(prefix + QStringLiteral("animation_frame"),
                      actor.animation_frame);
        fields.insert(prefix + QStringLiteral("physical_meter"),
                      actor.physical_meter);
        fields.insert(prefix + QStringLiteral("target_valid"),
                      actor.ai_target_valid);
        fields.insert(prefix + QStringLiteral("target_id"),
                      QString::fromStdString(actor.ai_target_id));
        fields.insert(prefix + QStringLiteral("target_heading"),
                      actor.ai_target_heading);
        fields.insert(prefix + QStringLiteral("target_distance"),
                      actor.ai_target_distance);
        fields.insert(prefix + QStringLiteral("ai_package"),
                      QString::fromStdString(actor.ai_package_id));
        fields.insert(prefix + QStringLiteral("transition_complete"),
                      actor.transition_complete);
        if (actor.ai_weapon_item_id.has_value()) {
            fields.insert(prefix + QStringLiteral("weapon_item"),
                          *actor.ai_weapon_item_id);
        }
    }
    fields.insert(QStringLiteral("field.actor_count"),
                  static_cast<double>(actor_count));

    fields.insert(QStringLiteral("party.active_slot"),
                  static_cast<int>(field_party_roster_.active_slot()));
    for (std::size_t slot = 0; slot < FieldPartyRosterRuntime::kSlotCount;
         ++slot) {
        const auto prefix =
            QStringLiteral("party.runner.%1.").arg(static_cast<qulonglong>(slot));
        fields.insert(prefix + QStringLiteral("active"),
                      field_party_roster_.slot_active(slot));
        const auto& name = field_party_roster_.name(slot);
        fields.insert(prefix + QStringLiteral("name"),
                      QString::fromUtf8(name.data(),
                                        static_cast<qsizetype>(name.size())));
    }

    const auto route = game_scripts_.menu_route_snapshot();
    fields.insert(QStringLiteral("menu.route.open"), route.open);
    fields.insert(QStringLiteral("menu.route.screen"), route.top_screen);
    fields.insert(QStringLiteral("menu.route.title"),
                  QString::fromStdString(route.top_title));
    fields.insert(QStringLiteral("menu.route.text"),
                  QString::fromStdString(route.top_text));
    fields.insert(QStringLiteral("menu.route.matrix.phase"),
                  QString::fromStdString(route.top_matrix_phase));
    fields.insert(QStringLiteral("menu.route.matrix.pending_action"),
                  QString::fromStdString(route.top_matrix_pending_action));
    if (route.top_matrix_world_effect_gate_negative.has_value()) {
        fields.insert(
            QStringLiteral("menu.route.matrix.world_effect_gate_negative"),
            *route.top_matrix_world_effect_gate_negative);
    }
    fields.insert(QStringLiteral("menu.route.matrix.traversal.present"),
                  route.top_matrix_traversal_view.has_value());
    if (route.top_matrix_traversal_view.has_value()) {
        const auto& traversal = *route.top_matrix_traversal_view;
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.world_effect_mode"),
                      traversal.world_effect_mode);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.world_effect_phase"),
                      traversal.world_effect_phase);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.entry_phase"),
                      traversal.world_entry_phase);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.setup_vblanks"),
                      traversal.world_entry_setup_vblanks);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.sequence_vblanks"),
                      traversal.world_entry_sequence_vblanks);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.sequence_duration"),
                      traversal.world_entry_sequence_duration);
        fields.insert(
            QStringLiteral("menu.route.matrix.traversal.published_frame"),
                      traversal.world_entry_published_frame);
    }
    const auto insert_route_optional = [&fields](
        const QString& key, const std::optional<int> value) {
        if (value.has_value()) {
            fields.insert(key, *value);
        }
    };
    insert_route_optional(QStringLiteral("menu.debug.route.program_index"),
                          route.top_program_index);
    insert_route_optional(QStringLiteral("menu.debug.route.program_slot"),
                          route.top_program_slot);
    insert_route_optional(QStringLiteral("menu.debug.route.program_choice"),
                          route.top_program_choice);
    if (route.top_enter_callback_present.has_value()) {
        fields.insert(QStringLiteral("menu.debug.route.enter_callback_present"),
                      *route.top_enter_callback_present);
    }
    if (route.top_enter_called.has_value()) {
        fields.insert(QStringLiteral("menu.debug.route.enter_called"),
                      *route.top_enter_called);
    }
    insert_route_optional(QStringLiteral("menu.debug.route.enter_transition"),
                          route.top_enter_transition);
    fields.insert(QStringLiteral("menu.debug.route.lua_failure"),
                  QString::fromStdString(game_scripts_.failure()));
    const auto insert_optional = [&fields](const QString& key,
                                           const std::optional<int> value) {
        if (value.has_value()) {
            fields.insert(key, *value);
        }
    };
    insert_optional(QStringLiteral("menu.route.page"), route.page);
    insert_optional(QStringLiteral("menu.route.row"), route.row);
    insert_optional(QStringLiteral("menu.route.selection"),
                    route.top_selection);
    for (std::size_t index = 0; index < route.top_option_labels.size();
         ++index) {
        fields.insert(
            QStringLiteral("menu.route.option.%1")
                .arg(static_cast<qulonglong>(index)),
            QString::fromStdString(route.top_option_labels[index]));
    }
    for (const auto& [name, value] : game_scripts_.story_state()) {
        fields.insert(QStringLiteral("flag.%1").arg(QString::fromStdString(name)),
                      value);
    }
    return fields;
}

void FieldSceneRuntime::initialize_field_entity_selection_state() {
    // ROM 0x42B0 publishes the three field-record types in slot order. Seed
    // their selection flags from the same typed records used by the field HUD
    // so the C-button selection action and combat gate share authoritative
    // runner state.
    field_entity_selection_state_ = RunnerSwitchState{};
    const auto& records = field_party_roster_.records();
    field_entity_selection_state_.records.reserve(records.size());
    for (std::size_t slot = 0; slot < records.size(); ++slot) {
        auto selection_flags = static_cast<std::uint8_t>(
            records[slot].flags >> 8U);
        if (!field_party_roster_.slot_active(slot)) {
            selection_flags = static_cast<std::uint8_t>(
                selection_flags & ~0x01U);
        }
        field_entity_selection_state_.records.push_back(
            RunnerSwitchRecord{static_cast<std::uint16_t>(slot),
                               selection_flags});
    }
    field_entity_selection_state_.active_index =
        field_party_roster_.active_slot();
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
    field_runner_refresh_state_ = FieldRunnerRefreshState{};
    field_mode_exit_transition_state_ = FieldModeExitTransitionState{};
    field_mode_exit_transition_state_.action_counter = field_action_counter_;
}

void FieldSceneRuntime::refresh_field_runner_state_and_assets() {
    const auto result =
        ::shadowrun::ghidra_conversion::refresh_field_runner_state_and_assets(
            field_runner_refresh_state_, 0U,
            {[this] {
                 source_boundary("field.runners.serialize_population");
             },
             [this](const FieldRunnerSlot slot) {
                 source_boundary(slot == FieldRunnerSlot::primary
                                     ? "field.runners.clear_primary_effect"
                                     : "field.runners.clear_secondary_effect");
             },
             [this](const FieldRunnerSlot) {
                 handle_field_mode_action_event();
             },
             [this] { source_boundary("field.resources.upload_asset_setup"); },
             [this] {
                 source_boundary("field.resources.run_animation_triplet");
             }});
    if (result.processed[0] || result.processed[1]) {
        source_boundary("field.runners.refresh_processed");
    }
}

void FieldSceneRuntime::handle_field_mode_action_event() {
    field_entity_selection_state_.active_index =
        field_party_roster_.active_slot();
    const auto& runner_records = field_party_roster_.records();
    for (std::size_t slot = 0;
         slot < field_entity_selection_state_.records.size() &&
         slot < runner_records.size();
         ++slot) {
        auto selection_flags = static_cast<std::uint8_t>(
            runner_records[slot].flags >> 8U);
        if (!field_party_roster_.slot_active(slot)) {
            selection_flags = static_cast<std::uint8_t>(
                selection_flags & ~0x01U);
        }
        field_entity_selection_state_.records[slot].flags = selection_flags;
    }
    const auto decision = advance_runner_switch(field_entity_selection_state_);
    const auto selected_slot = field_entity_selection_state_.active_index;
    if (selected_slot < runner_records.size() &&
        field_party_roster_.slot_active(selected_slot)) {
        field_party_roster_.set_active_slot(selected_slot);
    }
    auto& mutable_runner_records = field_party_roster_.records();
    for (std::size_t slot = 0;
         slot < field_entity_selection_state_.records.size() &&
         slot < mutable_runner_records.size();
         ++slot) {
        if (!field_party_roster_.slot_active(slot)) {
            continue;
        }
        auto& runner = mutable_runner_records[slot];
        runner.flags = static_cast<std::uint16_t>(
            (runner.flags & 0x00FFU) |
            (static_cast<std::uint16_t>(field_entity_selection_state_
                                            .records[slot]
                                            .flags)
             << 8U));
    }
    if (field_entity_selection_state_.active_index <
        field_entity_selection_state_.records.size()) {
        field_entity_behavior_flags_ = static_cast<std::uint8_t>(
            field_entity_selection_state_
                .records[field_entity_selection_state_.active_index]
                .flags);
    }
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
    field_entity_.projection.sprite_offset_y =
        active_runner_record().projection_word_08;
    refresh_field_hud_runner_slot();
    source_boundary("field.entities.handle_mode_action_event");
    if (decision.result == RunnerSwitchResult::switched) {
        source_boundary("field.entities.active_runner_changed");
    }
}

void FieldSceneRuntime::run_field_mode_exit_transition() {
    ::shadowrun::ghidra_conversion::finalize_field_mode_exit_transition(
        field_mode_exit_transition_state_,
        {[this](const ExitEffectSlotGroup) {
             source_boundary("field.effects.clear_high_slots");
         },
         [this] { source_boundary("field.world.reset_effect_globals"); },
         [this](const FieldModeExitServicePhase phase) {
             source_boundary(phase == FieldModeExitServicePhase::
                                         mode_specific_cleanup
                                     ? "field.mode_exit.service_cleanup"
                                     : "field.mode_exit.service_refresh");
         },
         [this] { refresh_field_runner_state_and_assets(); },
         [this] { source_boundary("field.resources.run_animation_triplet"); },
         [this] { handle_field_mode_action_event(); },
         [this] { source_boundary("field.mode_exit.initialize_profile_a"); },
         [this] { source_boundary("field.mode_exit.initialize_profile_b"); }});
    field_action_counter_ = field_mode_exit_transition_state_.action_counter;
}

bool FieldSceneRuntime::run_field_exit_orchestrator(
    integration::FieldRuntimeCoordinatorState& runtime_state) {
    FieldExitOrchestratorState orchestrator_state{};
    // FF0110 is a graphics-owned gate in the ROM. The field controller has no
    // equivalent host flag yet, so leave it clear rather than treating a
    // controller button as a guessed graphics route.
    orchestrator_state.controller_flags = 0U;
    orchestrator_state.prompt_display_prepared = false;

    const auto& event_metadata = state_.field_scene_buffers.source_metadata;
    const auto resolved_event_route = game_scripts_.resolve_field_location_route(
        FieldScriptRouteContext{
            static_cast<std::uint8_t>(runtime_state.scene_id),
            static_cast<std::uint8_t>(runtime_state.field_mode),
            field_collision_event_code_from_id(
                field_movement_transition_state_.mode_event_code),
            event_metadata.event_table_available,
            event_metadata.event_mode_table,
            field_exit_alternate_result_});
    FieldScriptRoute event_route{};
    if (resolved_event_route.has_value()) {
        event_route = *resolved_event_route;
    } else {
        source_boundary("field.exit.building_route_script_failed");
    }
    active_field_script_route_ = event_route;
    pending_field_return_anchor_ = event_route.return_anchor;
    pending_field_return_facing_ = event_route.return_facing;
    if (event_route.kind == FieldScriptRouteKind::area_event &&
        event_route.area_event_target.has_value()) {
        const auto selected_route_boundary =
            std::string("field.exit.area_event_route_selected:event=") +
            std::to_string(event_route.event_code) + ":scene=" +
            std::to_string(*event_route.area_event_target);
        source_boundary(selected_route_boundary);
    } else if (event_route.kind ==
               FieldScriptRouteKind::source_activity_return) {
        source_boundary("field.exit.resource_activity_return_selected");
    } else if (event_route.kind == FieldScriptRouteKind::source_boundary) {
        source_boundary(event_route.boundary_reason);
    }

    FieldExitOrchestratorCallbacks callbacks{};
    callbacks.select_mode_entry_resource = [this] {
        source_boundary("field.exit.select_mode_entry_resource");
    };
    callbacks.dispatch_world_effect =
        [this, &runtime_state, &event_route](
            FieldExitOrchestratorState& state) {
            // ROM 0x1EEDC dispatches FFFBED through F3E56. The shared
            // bodyguard/package handler at 0x1EF2C compares the current map
            // and event with the selected run's origin/destination tuple.
            if (!field_funds_.has_value() ||
                !field_party_roster_.available()) {
                return false;
            }
            const auto run_event =
                game_scripts_.resolve_active_run_field_event(
                    static_cast<std::uint8_t>(runtime_state.scene_id),
                    static_cast<std::uint8_t>(
                        field_movement_transition_state_.mode_event_code));
            if (!run_event.has_value()) {
                source_boundary("field.run.event_script_failed");
                state.caller_exit_status = -1;
                return true;
            }
            if (run_event->action ==
                scripting::ScriptRunFieldEventAction::objective_advanced) {
                field_exit_alternate_result_ = -1;
                field_exit_world_flags_ |= field_flag_mask(
                    FieldWorldFactFlag::run_objective_active);
                source_boundary("field.run.package_collected");
            } else if (run_event->action ==
                       scripting::ScriptRunFieldEventAction::entry_dialogue) {
                if (event_route.kind != FieldScriptRouteKind::area_event ||
                    !event_route.area_event_target.has_value()) {
                    source_boundary(
                        "field.run.entry_dialogue_destination_missing");
                    state.caller_exit_status = -1;
                    return true;
                }
                game_scripts_.set_location_context(
                    "field_map", "johnson_run_target", "field.world_event",
                    "field_narrator");
                if (!field_dialogue_.open_gameplay_task(
                        run_event->dialogue_module,
                        run_event->dialogue_function)) {
                    source_boundary("field.run.entry_dialogue_open_failed");
                    state.caller_exit_status = -1;
                    return true;
                }
                // ROM 0x1F046/0x1F106 runs the target passage before the
                // selected event-table destination is prepared. The host
                // dialogue yields, so carry the already-resolved destination
                // through the modal and resume it when the passage closes.
                field_mode_entry_destination_scene_ =
                    *event_route.area_event_target;
                if (run_event->activity_scene_id == 15U &&
                    run_event->actor_group_area != 0U &&
                    run_event->actor_group_event != 0U &&
                    run_event->actor_list_offset != 0U &&
                    run_event->actor_resource_profile_offset != 0U) {
                    field_mode_entry_run_target_actor_group_ = *run_event;
                } else {
                    field_mode_entry_run_target_actor_group_.reset();
                }
                field_mode_entry_transition_pending_ = true;
                field_palette_fade_steps_ = 0U;
                source_boundary("field.run.entry_dialogue_opened");
                state.caller_exit_status = -1;
                return true;
            } else if (run_event->action ==
                       scripting::ScriptRunFieldEventAction::completed) {
                // ROM 0x1EF2C selects result 1 and clears FFFC22 bit 9;
                // 0x1F00A adds the negotiated amount and +0x8D progress to
                // each of the three runner records.
                field_exit_alternate_result_ = 1;
                field_exit_world_flags_ &= ~field_flag_mask(
                    FieldWorldFactFlag::run_objective_active);
                *field_funds_ += run_event->reward_nuyen;
                for (auto& runner : field_party_roster_.records()) {
                    runner.auxiliary[2] = static_cast<std::uint8_t>(
                        runner.auxiliary[2] +
                        run_event->runner_progress_delta);
                }
                field_entity_animation_.redraw_requested = true;
                source_boundary("field.run.package_delivered");
            }
            return false;
        };
    callbacks.run_indexed_title_services =
        [this, &runtime_state, &event_route](
            FieldExitOrchestratorState& state) {
            if (event_route.kind == FieldScriptRouteKind::source_boundary) {
                state.caller_exit_status = -1;
                return true;
            }
            if (event_route.route_preparation.has_value()) {
                const auto& preparation = *event_route.route_preparation;
                game_scripts_.set_field_mode_context(
                    scripting::ScriptFieldModeContext{
                        static_cast<std::uint8_t>(runtime_state.field_mode),
                        event_route.event_code,
                        field_exit_alternate_result_,
                        state_.field_scene_buffers.scene_mode,
                        field_movement_transition_state_.field_transition_context});
                game_scripts_.set_location_context(
                    preparation.location_type,
                    preparation.location_id,
                    preparation.text_profile,
                    preparation.speaker_id);
                const auto outcome = game_scripts_.call_string(
                    preparation.module, preparation.function);
                if (!outcome.has_value() || *outcome != "continue") {
                    failure_ = game_scripts_.failure().empty()
                        ? "Lua route preparation did not continue"
                        : game_scripts_.failure();
                    source_boundary(
                        "field.location.route_preparation_unresolved");
                    state.caller_exit_status = -1;
                    return true;
                }
                source_boundary(
                    "field.location.route_preparation_completed");
            }

            if (event_route.indexed_title_service.has_value()) {
                const auto& task = *event_route.indexed_title_service;
                game_scripts_.set_field_mode_context(
                    scripting::ScriptFieldModeContext{
                        static_cast<std::uint8_t>(runtime_state.field_mode),
                        event_route.event_code,
                        field_exit_alternate_result_,
                        state_.field_scene_buffers.scene_mode,
                        field_movement_transition_state_.field_transition_context});
                game_scripts_.set_location_context(
                    task.location_type, task.location_id,
                    task.text_profile, task.speaker_id);
                if (event_route.return_anchor !=
                    FieldLocationReturnAnchor::none) {
                    // ROM 0xBC36 records the collision-probe lead before a
                    // field-exit service takes over. Mode/event routes use
                    // this callback instead of open_field_location_dialogue.
                    save_field_door_return_point();
                }
                if (!field_dialogue_.open_gameplay_task(
                        task.module, task.function)) {
                    source_boundary(
                        "field.location.indexed_title_service_open_failed");
                    state.caller_exit_status = -1;
                    return true;
                }
                field_palette_fade_steps_ = 0U;
                source_boundary(
                    "field.location.indexed_title_service_opened");
                state.caller_exit_status = -1;
                return true;
            }

            // ROM title services can display a location entry passage before
            // the event-table destination. The Lua entity link supplies the
            // task, location context, and state gate; retain the typed route
            // while the host dialogue panel owns input.
            if (event_route.kind == FieldScriptRouteKind::area_event &&
                !event_route.entry_dialogue_module.empty()) {
                if (!event_route.area_event_target.has_value()) {
                    source_boundary(
                        "field.entry.area_event_destination_missing");
                    state.caller_exit_status = -1;
                    return true;
                }
                game_scripts_.set_field_mode_context(
                    scripting::ScriptFieldModeContext{
                        static_cast<std::uint8_t>(runtime_state.field_mode),
                        event_route.event_code,
                        field_exit_alternate_result_,
                        state_.field_scene_buffers.scene_mode,
                        field_movement_transition_state_.field_transition_context});
                game_scripts_.set_location_context(
                    event_route.entry_location_type,
                    event_route.entry_location_id,
                    event_route.entry_text_profile,
                    event_route.entry_speaker_id,
                    event_route.entry_event_id);
                if (!field_dialogue_.open_gameplay_task(
                        event_route.entry_dialogue_module,
                        event_route.entry_dialogue_function)) {
                    source_boundary(
                        "field.entry.location_dialogue_open_failed");
                    state.caller_exit_status = -1;
                    return true;
                }
                field_indexed_title_service_result_ = -1;
                field_mode_entry_destination_scene_ =
                    *event_route.area_event_target;
                field_mode_entry_transition_pending_ = true;
                field_palette_fade_steps_ = 0U;
                source_boundary("field.entry.location_dialogue_opened");
                state.caller_exit_status = -1;
                return true;
            }

            // ROM 0x53A78 invokes service 22 before generic route selection.
            // Its scene-2/event-7 branch runs the Redmond Vigore search only
            // when Lua sees the pending service fact and the source one-shot
            // bit is clear. Both event-7 records reach this same service; the
            // ROM handler does not distinguish their collision-record IDs.
            if (runtime_state.scene_id == 2U &&
                event_route.kind == FieldScriptRouteKind::area_event &&
                event_route.event_code == static_cast<std::uint8_t>(
                    FieldCollisionEventCode::event_7) &&
                event_route.area_event_target ==
                    std::optional<std::uint8_t>{26U} &&
                field_exit_alternate_result_ == 0) {
                game_scripts_.set_location_context(
                    "field_map", "redmond_barrens", "field.world_event",
                    "field_narrator");
                const auto decision = game_scripts_.call_string(
                    "redmond_barrens", "should_run_vigore_search");
                if (!decision.has_value()) {
                    source_boundary(
                        "field.event.redmond_vigore_search_decision_failed");
                    state.caller_exit_status = -1;
                    return true;
                }
                if (*decision == "run") {
                    // redmond_barrens.run_vigore_search publishes FFDD64 = 22
                    // (story_flags.begin_trigger) and marks the 0x5463C tail.
                    if (!field_dialogue_.open_gameplay_task(
                            "redmond_barrens", "run_vigore_search")) {
                        source_boundary(
                            "field.event.redmond_vigore_search_open_failed");
                    } else {
                        field_palette_fade_steps_ = 0U;
                        source_boundary(
                            "field.event.redmond_vigore_search_opened");
                    }
                    state.caller_exit_status = -1;
                    return true;
                }
                if (*decision != "skip") {
                    source_boundary(
                        "field.event.redmond_vigore_search_invalid_decision");
                    state.caller_exit_status = -1;
                    return true;
                }
            }

            // Same-map dialogue interactions are data-owned route tasks.
            // The selected link supplies the story handler, location context,
            // state gate, and optional door-return policy; the host owns only
            // the modal lifecycle and the saved field position.
            if (event_route.kind != FieldScriptRouteKind::dialogue ||
                event_route.script != FieldScriptId::lua_dialogue ||
                event_route.entry_dialogue_module.empty() ||
                event_route.entry_dialogue_function.empty()) {
                return false;
            }

            game_scripts_.set_field_mode_context(
                scripting::ScriptFieldModeContext{
                    static_cast<std::uint8_t>(runtime_state.field_mode),
                    event_route.event_code,
                    field_exit_alternate_result_,
                    state_.field_scene_buffers.scene_mode,
                    field_movement_transition_state_.field_transition_context});
            game_scripts_.set_location_context(
                event_route.entry_location_type,
                event_route.entry_location_id,
                event_route.entry_text_profile,
                event_route.entry_speaker_id,
                event_route.entry_event_id);
            if (event_route.return_anchor !=
                FieldLocationReturnAnchor::none) {
                // ROM B834 -> BC36 records the active collision probe before
                // the same-map dialogue takes ownership of input.
                save_field_door_return_point();
            }
            if (!field_dialogue_.open_gameplay_task(
                    event_route.entry_dialogue_module,
                    event_route.entry_dialogue_function)) {
                source_boundary("field.location.dialogue_task_open_failed");
                state.caller_exit_status = -1;
                return true;
            }
            field_palette_fade_steps_ = 0U;
            source_boundary("field.location.dialogue_task_opened");
            state.caller_exit_status = -1;
            return true;
        };
    callbacks.select_field_exit_prompt =
        [this, &runtime_state, &event_route](
            FieldExitOrchestratorState& state) {
            if (event_route.field_exit_prompt.has_value()) {
                const auto& task = *event_route.field_exit_prompt;
                game_scripts_.set_field_mode_context(
                    scripting::ScriptFieldModeContext{
                        static_cast<std::uint8_t>(runtime_state.field_mode),
                        event_route.event_code,
                        field_exit_alternate_result_,
                        state_.field_scene_buffers.scene_mode,
                        field_movement_transition_state_.field_transition_context});
                game_scripts_.set_location_context(
                    task.location_type, task.location_id,
                    task.text_profile, task.speaker_id);
                if (event_route.return_anchor !=
                    FieldLocationReturnAnchor::none) {
                    // ROM 0xBC36 records the collision-probe lead before a
                    // linked field-exit prompt takes over input.
                    save_field_door_return_point();
                }
                if (!field_dialogue_.open_gameplay_task(
                        task.module, task.function)) {
                    source_boundary(
                        "field.location.exit_prompt_open_failed");
                    state.caller_exit_status = -1;
                    return true;
                }
                // The ROM's 0x1F65A selector sets D6=-1 after this modal
                // prompt. The linked scene-table byte is retained as source
                // evidence; the Inn prompt itself never loads scene 53, and
                // Little Chiba's never loads scene 26 (0x1F830). Closing the
                // prompt is the same-map service return, not a route resume.
                field_mode_entry_transition_pending_ = false;
                field_mode_entry_destination_scene_.reset();
                field_mode_entry_run_target_actor_group_.reset();
                field_palette_fade_steps_ = 0U;
                source_boundary("field.location.exit_prompt_opened");
                state.caller_exit_status = -1;
                return true;
            }

            FieldExitPromptState prompt_state{};
            const auto dialogue_event_code =
                event_route.kind == FieldScriptRouteKind::dialogue
                    ? event_route.event_code
                    : field_movement_transition_state_.mode_event_code;
            const FieldDialogueEventContext event_context{
                runtime_state.field_mode,
                dialogue_event_code,
                field_exit_alternate_result_,
                field_exit_world_flags_};
            game_scripts_.set_field_mode_context(
                scripting::ScriptFieldModeContext{
                    static_cast<std::uint8_t>(runtime_state.field_mode),
                    dialogue_event_code,
                    field_exit_alternate_result_,
                    state_.field_scene_buffers.scene_mode,
                    field_movement_transition_state_.field_transition_context});
            const auto action = field_dialogue_event_resolver_.resolve(
                event_context, prompt_state,
                {[this](FieldExitPromptSelection& selection,
                       FieldExitPromptState&) {
                     // 0x1F246/0x1F3C4 is blocking on the Genesis but is a
                     // normal host state transition here. The panel owns the
                     // subsequent controller edges without re-entering field
                     // movement or inventing a second dialogue machine.
                     if (field_dialogue_.open_field_exit_prompt(selection)) {
                         // 0x1F246 rebuilds the mode display and refreshes
                         // its palette after preparing the prompt. The Qt
                         // fade is only a presentation overlay, so it must
                         // not cover the newly prepared dialogue screen.
                         field_palette_fade_steps_ = 0U;
                         source_boundary(
                             "field.exit.prompt.display_palette_refreshed");
                     }
                 },
                 [this] {
                     source_boundary("field.exit.hotel_options_unresolved");
                 }});
            state.caller_exit_status = prompt_state.caller_exit_status;
            if (action == FieldExitPromptAction::no_matching_scene) {
                source_boundary("field.exit.prompt_scene_unmatched");
            }
            return false;
        };

    // Direct dialogue and service scripts take priority. Every other event
    // uses the active map's authored event table; the coordinator's ordinary
    // world-activity branch then prepares the selected scene. This keeps the
    // scene-2 C0 doorway (event 0 -> scene 0x14) separate from the Stoker C2
    // interaction and the Ares CB service.
    const auto continues_through_area_event_table = [&event_route] {
        return event_route.kind == FieldScriptRouteKind::area_event;
    };

    // The entity route selects a named host service. ROM 0x1FA78 still
    // supplies that service's typed mode/result profile below.
    const auto is_host_field_service_route = [&event_route] {
        return event_route.kind == FieldScriptRouteKind::field_service &&
               event_route.script == FieldScriptId::host_field_service &&
               !event_route.field_service_id.empty();
    };
    callbacks.prepare_field_mode_exit =
        [this, &runtime_state, &event_route,
         &is_host_field_service_route,
         &continues_through_area_event_table](
            FieldExitOrchestratorState& state) {
        if (is_host_field_service_route()) {
            return false;
        }
        if (continues_through_area_event_table()) {
            const auto content = field_mode_text_.open_if_matched(
                scripting::ScriptFieldModeContext{
                    runtime_state.field_mode,
                    field_movement_transition_state_.mode_event_code,
                    field_exit_alternate_result_});
            if (content == FieldModeTextOpenResult::opened) {
                if (!event_route.area_event_target.has_value()) {
                    field_mode_text_.close();
                    source_boundary(
                        "field.mode_text.entry_destination_missing");
                    state.caller_exit_status = -1;
                    return true;
                }
                field_mode_entry_transition_pending_ = true;
                field_mode_entry_destination_scene_ =
                    *event_route.area_event_target;
                field_palette_fade_steps_ = 0U;
                source_boundary(
                    "field.mode_text.area_event_entry_opened");
                state.caller_exit_status = -1;
                return true;
            }
            if (content == FieldModeTextOpenResult::failed) {
                source_boundary(
                    "field.mode_text.area_event_entry_failed");
                state.caller_exit_status = -1;
                return true;
            }
            source_boundary("field.exit.area_event_table_continuation");
            return false;
        }
        source_boundary("field.exit.prepare_mode_exit_unresolved");
        state.caller_exit_status = -1;
        return true;
    };
    callbacks.select_field_mode_service =
        [this, &runtime_state, &event_route,
         &is_host_field_service_route,
         &continues_through_area_event_table](
            FieldExitOrchestratorState& state) {
            if (continues_through_area_event_table()) {
                return false;
            }
            if (!is_host_field_service_route()) {
                source_boundary("field.exit.mode_service_unresolved");
                state.caller_exit_status = -1;
                return true;
            }

            FieldModeServiceRouteState route_state{};
            route_state.scene_id = runtime_state.field_mode;
            route_state.submode =
                field_movement_transition_state_.mode_event_code;
            route_state.alternate_result = field_exit_alternate_result_;
            bool prompt_opened = false;
            FieldModeServiceRouteCallbacks service_callbacks{};
            service_callbacks.run_mode_exit_services =
                [this, &event_route, &prompt_opened](
                    FieldModeServiceSelection&,
                    FieldModeServiceRouteState& source_state) {
                    scripting::ScriptFieldModeContext context{
                        source_state.scene_id, source_state.submode,
                        source_state.alternate_result};
                    context.service_id = event_route.field_service_id;
                    prompt_opened = field_mode_text_.open(
                        context);
                    if (prompt_opened) {
                        // The source mode-text service rebuilds the display
                        // after the field fade. The Qt fade is a presentation
                        // overlay, so retire it when this full-screen panel
                        // becomes the active screen owner.
                        field_palette_fade_steps_ = 0U;
                        source_boundary(
                            "field.mode_text_prompt.display_palette_refreshed");
                    } else {
                        source_boundary(
                            "field.ares_mode_text_prompt.open_failed");
                    }
                };
            service_callbacks.run_field_mode_service =
                [this, &event_route, &prompt_opened](
                    FieldModeServiceRouteState&) {
                    if (!prompt_opened) {
                        return;
                    }
                    field_shop_route_pending_service_id_ =
                        event_route.field_service_id;
                    source_boundary("field.shop.service_dispatched");
                };

            const auto action = select_field_mode_service(
                route_state, service_callbacks);
            if (action == FieldModeServiceAction::no_matching_route) {
                source_boundary("field.exit.mode_service_unmatched");
            } else if (action ==
                       FieldModeServiceAction::field_mode_service_unresolved) {
                source_boundary("field.exit.mode_service_callback_unresolved");
            }
            state.caller_exit_status = route_state.caller_exit_status;
            // The generic prompt owns the next input samples. For default
            // result it will open the shop after the prompt returns; alternate
            // result records return to the field without dispatching 0x56AC0.
            return true;
        };
    callbacks.finalize_field_mode_exit =
        [this, &continues_through_area_event_table](
            FieldExitOrchestratorState& state) {
        if (continues_through_area_event_table()) {
            return false;
        }
        source_boundary("field.exit.finalize_mode_exit_unresolved");
        state.caller_exit_status = -1;
        return true;
    };

    const auto action = ::shadowrun::ghidra_conversion::
        run_field_exit_orchestrator(orchestrator_state, callbacks);
    if (action == FieldExitOrchestratorAction::graphics_service_routed) {
        source_boundary("field.exit.graphics_service_unresolved");
    }
    return orchestrator_state.caller_exit_status != 0;
}

bool FieldSceneRuntime::project_field_world_interaction() {
    const auto& profile = state_.field_scene_buffers.profile;
    WorldMapEffectInput input{};
    input.world_mode = field_movement_transition_state_.world_mode;
    input.scene_id = field_movement_transition_state_.scene_id;
    input.entity_flags = field_movement_behavior_record_.projected_field_code;
    input.action_requested =
        field_movement_transition_state_.mode_event_code != 0U;
    input.map_marker_index = field_collision_marker_index_;
    input.row_stride = profile.size() > 18U ? profile[18U] : 0;
    input.map_value_a = kSceneFourteenMapWordA;
    input.map_value_b = kSceneFourteenMapWordB;
    // FFDD16 is profile word 40. The 48-word block starts at FFDDC6.
    input.auxiliary_world_y = profile.size() > 40U ? profile[40U] : 0U;
    const auto rotated_slot =
        field_movement_behavior_record_.action_heading_source;
    input.auxiliary_world_y_delta = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(rotated_slot << 2U) |
        static_cast<std::uint16_t>(rotated_slot >> 14U));
    input.coordinate = CoordinateProjectionInput{
        field_entity_.world_position.x, field_entity_.world_position.y,
        field_movement_state_.heading};
    input.auxiliary_entity_slot_available =
        entity_table_has_free_slot(field_entity_table_occupancy_);

    const auto effect = apply_world_map_effect(input);
    if (effect.map_marker_cleared) {
        auto& collision_map =
            state_.field_scene_buffers.source_metadata.terrain_collision_map;
        if (field_collision_marker_index_ < collision_map.size()) {
            collision_map[field_collision_marker_index_] = 0U;
        } else {
            source_boundary("field.world.interaction_marker_out_of_range");
        }
    }
    if (effect.special_resource_selected) {
        field_pending_scene_profile_ = kSceneFourInteractionProfile;
    }
    if (effect.world_effect_x_dirty) {
        field_camera_follow_.layer_x_dirty = true;
        viewport_state_.normal_render_pending = 1U;
    }
    if (effect.world_effect_y_dirty) {
        field_camera_follow_.layer_y_dirty = true;
        viewport_state_.alternate_render_pending = 1U;
    }
    if (effect.world_effect_x_dirty || effect.world_effect_y_dirty) {
        apply_world_effect_layer_writes(effect.world_effect_y_dirty,
                                        effect.map_writes);
    }
    if (effect.auxiliary_entity.has_value()) {
        for (std::size_t index = 0; index < field_entity_table_occupancy_.size();
             ++index) {
            if (field_entity_table_occupancy_[index] != 0U) {
                continue;
            }
            // ROM writes -1 into the free slot's leading word.
            field_entity_table_occupancy_[index] = 0xFFFFU;
            field_entity_table_[index] = *effect.auxiliary_entity;
            break;
        }
    }
    return world_map_effect_sets_carry(input);
}

bool FieldSceneRuntime::load_image(
    const ConversionAssetCatalog& catalog,
    const std::string_view alias,
    QImage& destination) {
    const auto* record = catalog.find(std::string(alias));
    if (record == nullptr) {
        failure_ = "missing direct field asset alias: " + std::string(alias);
        return false;
    }
    destination = QImage(QString::fromStdWString(
        record->absolute_path.wstring()));
    if (destination.isNull()) {
        failure_ = "cannot load direct field asset alias: " +
                   std::string(alias);
        return false;
    }
    destination = destination.convertToFormat(
        QImage::Format_ARGB32_Premultiplied);
    return true;
}

bool FieldSceneRuntime::load_field_elevator_selector_assets(
    const ConversionAssetCatalog& catalog) {
    for (std::size_t frame = 0U;
         frame < field_elevator_selector_backgrounds_.size(); ++frame) {
        const auto alias = std::string(kElevatorAnimationAliasPrefix) +
                           std::to_string(frame);
        if (!load_image(catalog, alias,
                        field_elevator_selector_backgrounds_[frame]) ||
            field_elevator_selector_backgrounds_[frame].size() !=
                QSize(320, 224)) {
            source_boundary("field.elevator.animation_asset_missing");
            return false;
        }
    }
    for (std::size_t floor = 0U;
         floor < kElevatorSelectorMaximumFloors; ++floor) {
        const auto current_alias =
            std::string(kElevatorCurrentFloorAliasPrefix) +
            std::to_string(floor);
        const auto selected_alias =
            std::string(kElevatorSelectedFloorAliasPrefix) +
            std::to_string(floor);
        if (!load_image(
                catalog, current_alias,
                field_elevator_selector_current_floor_layers_[floor]) ||
            !load_image(
                catalog, selected_alias,
                field_elevator_selector_selected_floor_layers_[floor]) ||
            field_elevator_selector_current_floor_layers_[floor].size() !=
                QSize(320, 224) ||
            field_elevator_selector_selected_floor_layers_[floor].size() !=
                QSize(320, 224)) {
            source_boundary("field.elevator.floor_label_asset_missing");
            return false;
        }
    }
    for (std::size_t index = 0U;
         index < field_elevator_selector_feedback_layers_.size(); ++index) {
        const auto alias = std::string(kElevatorFeedbackAliasPrefix) +
                           std::string(kElevatorFeedbackAssetNames[index]);
        if (!load_image(catalog, alias,
                        field_elevator_selector_feedback_layers_[index]) ||
            field_elevator_selector_feedback_layers_[index].size() !=
                QSize(320, 224)) {
            source_boundary("field.elevator.navigation_feedback_asset_missing");
            return false;
        }
    }
    return true;
}

bool FieldSceneRuntime::load_field_taxi_selector_assets(
    const ConversionAssetCatalog& catalog) {
    if (!load_image(catalog, "intro.cab.map", field_taxi_map_) ||
        field_taxi_map_.size() != QSize(208, 136)) {
        source_boundary("field.taxi.map_asset_missing");
        return false;
    }

    constexpr std::array<std::string_view, 8> kCursorTileNames{{
        "top.left", "top.middle", "top.right", "left", "right",
        "bottom.left", "bottom.middle", "bottom.right"}};
    for (std::size_t index = 0U; index < kCursorTileNames.size(); ++index) {
        const auto alias = std::string("intro.cab.cursor.") +
                           std::string(kCursorTileNames[index]);
        if (!load_image(catalog, alias, field_taxi_cursor_tiles_[index]) ||
            field_taxi_cursor_tiles_[index].size() != QSize(8, 8)) {
            source_boundary("field.taxi.cursor_asset_missing");
            return false;
        }
    }
    return true;
}

bool FieldSceneRuntime::open_field_elevator_selector(
    const FieldCollisionEventCode event_code) {
    if (!field_session_source_assets_ || field_elevator_selector_.is_open()) {
        return false;
    }
    const auto scene_reference =
        state_.field_scene_buffers.scene_reference;
    std::vector<std::vector<std::uint16_t>> source_elevator_groups;
    source_elevator_groups.reserve(
        field_session_source_assets_->field_elevator_scene_groups.size());
    for (const auto& source_group :
         field_session_source_assets_->field_elevator_scene_groups) {
        if (source_group.count > source_group.scene_ids.size()) {
            source_boundary("field.elevator.source_group_invalid");
            return false;
        }
        auto& group = source_elevator_groups.emplace_back();
        group.insert(group.end(), source_group.scene_ids.begin(),
                     source_group.scene_ids.begin() +
                         static_cast<std::ptrdiff_t>(source_group.count));
    }

    const auto entry_decision = game_scripts_.resolve_field_location_entry(
        scene_reference, event_code, source_elevator_groups);
    if (!entry_decision.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.elevator.entry_script_failed");
        return false;
    }
    if (entry_decision->action !=
            scripting::ScriptLocationEntryAction::open_elevator_selector ||
        entry_decision->elevator_group.scene_ids.size() < 2U ||
        entry_decision->elevator_group.scene_ids.size() >
            kElevatorSelectorMaximumFloors) {
        failure_ = "Lua declined elevator selector entry for scene " +
                   std::to_string(scene_reference) + " and event code " +
                   std::to_string(field_collision_event_value(event_code));
        source_boundary("field.elevator.entry_script_no_selector");
        return false;
    }

    field_elevator_floor_group_ = entry_decision->elevator_group;
    const auto floor_count = static_cast<std::uint8_t>(
        field_elevator_floor_group_->scene_ids.size());
    if (!field_elevator_selector_.open(
            floor_count, field_elevator_floor_group_->entry_floor)) {
        field_elevator_floor_group_.reset();
        return false;
    }
    field_elevator_script_destination_ =
        game_scripts_.resolve_elevator_destination(
            *field_elevator_floor_group_,
            field_elevator_selector_.selected_floor());
    if (!field_elevator_script_destination_.has_value()) {
        failure_ = game_scripts_.failure();
        field_elevator_selector_.close();
        field_elevator_floor_group_.reset();
        source_boundary("field.elevator.destination_script_failed");
        return false;
    }
    source_boundary("field.elevator.selector.opened");
    return true;
}

bool FieldSceneRuntime::transition_to_field_destination(
    const std::uint16_t scene_reference,
    const FieldDestinationTransitionRoute route) {
    const auto source_prefix =
        route == FieldDestinationTransitionRoute::dialogue_scene_entry
            ? "field.dialogue.scene_entry."
            : route == FieldDestinationTransitionRoute::taxi
                  ? "field.taxi."
                  : route == FieldDestinationTransitionRoute::vid_phone
                        ? "field.vid_phone."
                        : "field.elevator.";
    if (!field_session_source_assets_ ||
        scene_reference >=
            field_session_source_assets_->field_scene_resources.size() ||
        !prepare_runtime_field_scene(scene_reference)) {
        source_boundary(std::string(source_prefix) +
                        "destination_prepare_unresolved");
        return false;
    }

    // ROM 0x16A8E reloads the destination field profile through 0x03074/0x03084.
    // The first profile record is the arrival point for both cab travel and
    // elevator floor changes; coordinates from another map are map-local.
    const auto arrival =
        state_.field_scene_buffers.source_metadata.initial_field_records[0U];
    field_entity_.world_position = arrival;
    field_entity_.projection = EntityProjectionState{};
    field_entity_.visible = false;
    field_entity_.direction = 0U;
    field_movement_state_.position_x = arrival.x;
    field_movement_state_.position_y = arrival.y;
    field_movement_state_.heading = 0U;
    field_movement_state_.movement_substate = 0U;
    field_target_heading_ = 0U;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = false;
    field_entity_animation_ = EntityAnimationState{};
    field_entity_animation_.increment = kFieldWalkAnimationIncrement;
    field_entity_animation_.period = kFieldEntityAnimationPeriod;
    field_entity_animation_id_ = "idle";
    field_player_actor_script_initialized_ = false;
    field_player_animation_reset_pending_ = false;
    field_entity_animation_selector_ = 0U;
    field_movement_behavior_record_.current_heading = 0U;
    field_movement_behavior_record_.target_heading = 0U;
    field_movement_transition_state_.terrain_collision_code = 0U;
    field_movement_transition_state_.mode_event_code = 0U;
    field_movement_transition_state_.activity_sequence_active = false;
    field_movement_transition_state_.terrain_event_suppressed = false;
    field_movement_transition_state_.scene_id = scene_reference;
    // ROM 0x136DE / 0x2A94 seed the FFFC10 cave room for scenes entered from
    // outside the 0x2CE0 graph; Lua world_links owns which scenes do.
    if (const auto entry_room = game_scripts_.call_integers(
            "world_links", "entry_room_flat",
            {static_cast<std::int64_t>(scene_reference)});
        entry_room.has_value() && entry_room->size() == 1U &&
        (*entry_room)[0] >= 0) {
        field_movement_transition_state_.field_transition_context =
            static_cast<std::uint16_t>((*entry_room)[0]);
        source_boundary(std::string(source_prefix) + "entry_room_seeded");
    }
    field_movement_transition_state_.field_origin_x =
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x);
    field_movement_transition_state_.activity_source_word = 0U;
    field_runtime_state_.scene_id = scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.field_scene_gate = 0U;
    field_runtime_state_.field_transition_requested = false;
    field_runtime_state_.pending_field_scene_handler = false;
    field_runtime_state_.field_activity_lock = false;
    field_runtime_state_.world_activity_countdown = 0U;
    world_frame_logic_state_.world_mode = scene_reference;
    field_world_activity_scene_entry_pending_ = false;
    field_world_activity_state_ = WorldActivityState{};
    field_world_activity_timeout_state_ = WorldActivityTimeoutState{};
    field_world_activity_timeout_state_.scene_id = scene_reference;
    world_activity_routine_state_ = WorldActivityRoutineState{};
    world_activity_request_state_ = WorldActivityRoutineRequestState{};
    world_activity_request_start_state_ = WorldActivityStartState{};
    world_activity_return_snapshot_.reset();
    field_weapon_projectiles_.clear();
    field_weapon_slots_occupied_.fill(false);
    last_field_weapon_spawn_result_ = EntitySpawnCycleResult{};
    field_camera_alignment_ready_ = false;
    field_palette_fade_steps_ = 0U;
    field_runtime_state_.display_service_word = 0U;
    state_.display_service_word = 0U;
    // ROM 0x08C0, before 0x12F6: a new field mode runs 31 camera and entity
    // projection passes with FFDFE8 clear, then fades in. The fade at 0x12F6
    // only waits on 0x1342. Scene 9's profile spawn (0x0D80, 0x1100) projects
    // to pixel y 232 under camera (0x0A00, 0x0280), so those passes have to
    // finish while the destination is still black.
    const auto restore_interaction_lock =
        field_movement_transition_state_.world_interaction_lock;
    const bool mode_entry_route =
        route == FieldDestinationTransitionRoute::vid_phone ||
        route == FieldDestinationTransitionRoute::dialogue_scene_entry;
    if (mode_entry_route) {
        field_movement_transition_state_.world_interaction_lock = false;
        field_camera_follow_.layer_x_dirty = false;
        field_camera_follow_.layer_y_dirty = false;
    }
    constexpr int kModeEntryCameraPasses = 31;
    const int camera_passes =
        mode_entry_route ? kModeEntryCameraPasses : 1;
    for (int pass = 0; pass < camera_passes; ++pass) {
        update_source_field_camera();
        update_entity_render_pass(state_.render_entities,
                                  callbacks_.entity_render_pass);
    }
    if (mode_entry_route) {
        field_movement_transition_state_.world_interaction_lock =
            restore_interaction_lock;
    }
    dispatch_field_map_scene_entry(scene_reference);
    source_boundary(std::string(source_prefix) + "destination_loaded");
    return true;
}

void FieldSceneRuntime::advance_field_elevator_selector() {
    field_controller_latch_state_.events =
        field_controller_sample_state_.events;
    field_controller_latch_state_.extra_events = 0U;
    (void)::shadowrun::ghidra_conversion::latch_controller_events(
        field_controller_latch_state_);
    ::shadowrun::ghidra_conversion::sample_controller_input(
        field_controller_sample_state_, field_controller_port_byte_);
    const auto update = field_elevator_selector_.advance(
        field_controller_sample_state_.events);
    if (update.selection_moved) {
        if (!field_elevator_floor_group_.has_value()) {
            source_boundary("field.elevator.floor_group_missing");
            return;
        }
        field_elevator_script_destination_ =
            game_scripts_.resolve_elevator_destination(
                *field_elevator_floor_group_,
                field_elevator_selector_.selected_floor());
        if (!field_elevator_script_destination_.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.elevator.destination_script_failed");
            return;
        }
        source_boundary("field.elevator.selector.selection_moved");
    }
    if (update.navigation_feedback_started) {
        source_boundary("field.elevator.selector.navigation_feedback_started");
    }
    if (update.navigation_feedback_finished) {
        source_boundary("field.elevator.selector.navigation_feedback_finished");
    }
    if (update.sound_effect_requested) {
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
        }
        source_boundary("field.elevator.selector.source_sound_25");
    }
    if (update.animation_started) {
        source_boundary("field.elevator.animation.started");
    } else if (update.animation_frame_changed) {
        source_boundary("field.elevator.animation.frame_changed");
    } else if (update.animation_finished) {
        source_boundary("field.elevator.animation.finished");
    }
    if (!update.confirmed) {
        return;
    }

    const auto current_scene = state_.field_scene_buffers.scene_reference;
    source_boundary("field.elevator.selector.confirmed");
    if (field_elevator_selector_.destination_changed()) {
        if (!field_elevator_floor_group_.has_value()) {
            source_boundary("field.elevator.floor_group_missing");
            return;
        }
        const auto destination = game_scripts_.resolve_elevator_destination(
            *field_elevator_floor_group_,
            field_elevator_selector_.current_floor());
        if (!destination.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.elevator.destination_script_failed");
            return;
        }
        field_elevator_script_destination_ = destination;
        if (*destination == current_scene) {
            field_elevator_selector_.close();
            field_elevator_floor_group_.reset();
            field_runtime_state_.display_service_word = 0U;
            state_.display_service_word = 0U;
            source_boundary("field.elevator.selector.returned_to_current_scene");
            return;
        }
        field_elevator_pending_destination_ = destination;
        if (field_elevator_scene_transition_.begin_fade_out()) {
            source_boundary("field.elevator.destination_fade_out_started");
        } else {
            field_elevator_pending_destination_.reset();
            source_boundary("field.elevator.destination_transition_busy");
        }
        return;
    }
    field_elevator_selector_.close();
    field_elevator_floor_group_.reset();
    field_runtime_state_.display_service_word = 0U;
    state_.display_service_word = 0U;
    source_boundary("field.elevator.selector.returned_to_current_scene");
}

void FieldSceneRuntime::advance_field_elevator_scene_transition() {
    const auto update = field_elevator_scene_transition_.advance();
    const bool dialogue_scene_route =
        field_dialogue_scene_transition_active_;
    const bool phone_route = field_phone_transition_active_;
    const bool taxi_route = field_taxi_transition_active_;
    const auto route = dialogue_scene_route
                           ? FieldDestinationTransitionRoute::
                                 dialogue_scene_entry
                           : phone_route
                                 ? FieldDestinationTransitionRoute::vid_phone
                                 : taxi_route
                                       ? FieldDestinationTransitionRoute::taxi
                                       : FieldDestinationTransitionRoute::
                                             elevator;
    const auto source_prefix =
        dialogue_scene_route
            ? "field.dialogue.scene_entry."
            : phone_route ? "field.vid_phone."
                          : taxi_route ? "field.taxi." : "field.elevator.";
    if (update.palette_step_changed) {
        source_boundary(std::string(source_prefix) +
                        "destination_fade.palette_step");
    }
    if (update.destination_load_requested) {
        const auto destination =
            dialogue_scene_route
                ? field_dialogue_scene_pending_destination_
                : phone_route ? field_phone_pending_destination_
                              : taxi_route
                                    ? field_taxi_pending_destination_
                                    : field_elevator_pending_destination_;
        if (!destination.has_value() ||
            !transition_to_field_destination(*destination, route)) {
            field_elevator_scene_transition_.cancel();
            field_elevator_pending_destination_.reset();
            field_taxi_pending_destination_.reset();
            field_phone_pending_destination_.reset();
            field_dialogue_scene_pending_destination_.reset();
            field_taxi_transition_active_ = false;
            field_phone_transition_active_ = false;
            field_dialogue_scene_transition_active_ = false;
            source_boundary(std::string(source_prefix) +
                            "destination_transition_reopened");
            return;
        }
        if (phone_route) {
            field_phone_pending_destination_.reset();
        } else if (taxi_route) {
            field_taxi_pending_destination_.reset();
        } else if (dialogue_scene_route) {
            field_dialogue_scene_pending_destination_.reset();
        } else {
            field_elevator_selector_.close();
            field_elevator_floor_group_.reset();
            field_elevator_pending_destination_.reset();
        }
        if (!field_elevator_scene_transition_.begin_fade_in()) {
            field_elevator_scene_transition_.cancel();
            field_taxi_transition_active_ = false;
            field_phone_transition_active_ = false;
            field_dialogue_scene_transition_active_ = false;
            source_boundary(std::string(source_prefix) +
                            "destination_fade_in_unresolved");
            return;
        }
        source_boundary(std::string(source_prefix) +
                        "destination_fade_in_started");
    } else if (update.completed) {
        source_boundary(std::string(source_prefix) +
                        "destination_fade_finished");
        field_taxi_transition_active_ = false;
        field_phone_transition_active_ = false;
        field_dialogue_scene_transition_active_ = false;
    }
}

bool FieldSceneRuntime::load_sprite_image(
    const ConversionAssetCatalog& catalog,
    const std::string_view alias,
    QImage& destination,
    int& origin_x,
    int& origin_y) {
    const auto* record = catalog.find(std::string(alias));
    if (record == nullptr) {
        failure_ = "missing direct field sprite alias: " + std::string(alias);
        return false;
    }
    if (!load_image(catalog, alias, destination)) {
        return false;
    }
    origin_x = record->origin_x;
    origin_y = record->origin_y;
    // The extraction bakes color-zero as alpha 0 and loads it through the
    // V4 BMP alpha mask. Re-deriving transparency from black pixels here
    // would punch holes in the sprites' black outlines.
    return true;
}

bool FieldSceneRuntime::load_field_default_actor_assets(
    const ConversionAssetCatalog& catalog) {
    constexpr std::string_view kActorTableAlias =
        "field.entities.default.actors";
    constexpr std::string_view kActorTableHeader =
        "scene\tsource_order\tentity_kind\tprofile_id\tactor_name"
        "\tsprite_stem\taction_kind\tinitial_action_code"
        "\tworld_x\tworld_y\tinitial_heading\tpalette_line"
        "\ttemplate_offset\tposition_offset\tlist_offset"
        "\tordered_overrides\tphysical_meter";
    const auto* table = catalog.find(kActorTableAlias);
    if (table == nullptr || table->kind != "state") {
        failure_ = "missing extracted default field actor table";
        return false;
    }
    std::ifstream input(table->absolute_path, std::ios::binary);
    if (!input) {
        failure_ = "cannot read extracted default field actor table";
        return false;
    }

    field_default_actors_.clear();
    field_default_actor_artwork_ = {};

    std::string line;
    if (!std::getline(input, line) || line != kActorTableHeader) {
        failure_ = "default field actor table header is invalid";
        return false;
    }
    const auto parse_number = [](const std::string& value,
                                 const int base) -> std::uint32_t {
        std::size_t consumed = 0U;
        const auto parsed = std::stoul(value, &consumed, base);
        if (consumed != value.size() || parsed > 0xFFFFFFFFUL) {
            throw std::invalid_argument("invalid default actor number");
        }
        return static_cast<std::uint32_t>(parsed);
    };
    const auto parse_word = [&parse_number](const std::string& value) {
        const auto parsed = parse_number(value, 10);
        if (parsed > 0xFFFFU) {
            throw std::out_of_range("default actor word exceeds 16 bits");
        }
        return static_cast<std::uint16_t>(parsed);
    };
    std::array<std::uint16_t, 2> scene_record_counts{};
    try {
        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::istringstream row(line);
            std::string field;
            while (std::getline(row, field, '\t')) {
                fields.push_back(std::move(field));
            }
            if (fields.size() != 17U) {
                failure_ = "default field actor table row is malformed";
                return false;
            }
            DefaultFieldActorPresentation actor;
            actor.scene_reference = parse_word(fields[0]);
            actor.source_order = parse_word(fields[1]);
            actor.entity_kind = parse_word(fields[2]);
            // The field runtime currently has Lua behavior and directional
            // entry/idle art for pedestrians and corp guards. Other source
            // populations remain visible in the map viewer, but are not
            // instantiated with a substitute actor or sprite here.
            if (actor.entity_kind != 5U && actor.entity_kind != 11U) {
                continue;
            }
            actor.actor_instance_id =
                "field_scene_" + std::to_string(actor.scene_reference) +
                "_actor_" + std::to_string(actor.source_order);
            actor.script_profile_id = fields[3];
            actor.sprite_stem = fields[5];
            if (actor.script_profile_id.empty()) {
                failure_ = "default actor Lua profile identity is empty";
                return false;
            }
            if (fields[6] == "none") {
                actor.action_kind =
                    scripting::ScriptActorActionKind::none;
            } else if (fields[6] == "item") {
                actor.action_kind =
                    scripting::ScriptActorActionKind::item;
            } else if (fields[6] == "spell") {
                actor.action_kind =
                    scripting::ScriptActorActionKind::spell;
            } else {
                failure_ = "default actor action kind is not recognized";
                return false;
            }
            const auto initial_action_code = parse_word(fields[7]);
            if (initial_action_code > 0xFFU) {
                failure_ = "default actor action code exceeds one byte";
                return false;
            }
            actor.initial_action_code =
                static_cast<std::uint8_t>(initial_action_code);
            if ((actor.action_kind ==
                     scripting::ScriptActorActionKind::none) !=
                (actor.initial_action_code == 0U)) {
                failure_ = "default actor action kind/code disagree";
                return false;
            }
            actor.world_position.x = parse_word(fields[8]);
            actor.world_position.y = parse_word(fields[9]);
            actor.initial_heading = parse_word(fields[10]);
            const auto palette_line = parse_word(fields[11]);
            if (palette_line > 0xFFU) {
                failure_ = "default actor palette line exceeds one byte";
                return false;
            }
            actor.palette_line = static_cast<std::uint8_t>(palette_line);
            actor.template_offset = parse_number(fields[12], 0);
            actor.position_offset = parse_number(fields[13], 0);
            actor.list_offset = parse_number(fields[14], 0);
            actor.ordered_overrides = fields[15];
            const auto physical_meter = parse_number(fields[16], 10);
            if (physical_meter > 0xFFU) {
                failure_ = "default actor physical meter exceeds one byte";
                return false;
            }
            actor.physical_meter =
                static_cast<std::uint8_t>(physical_meter);
            if ((actor.entity_kind == 5U && actor.palette_line != 2U) ||
                (actor.entity_kind == 11U && actor.palette_line != 3U) ||
                (actor.entity_kind != 5U && actor.entity_kind != 11U)) {
                failure_ = "default actor has an unresolved source art kind";
                return false;
            }

            if (actor.scene_reference == 2U) {
                if (actor.source_order != scene_record_counts[0]) {
                    failure_ = "scene 2 default actor source order changed";
                    return false;
                }
                ++scene_record_counts[0];
            } else if (actor.scene_reference == 0x14U) {
                if (actor.source_order != scene_record_counts[1]) {
                    failure_ = "scene 0x14 default actor source order changed";
                    return false;
                }
                ++scene_record_counts[1];
            }

            field_default_actors_.push_back(std::move(actor));
        }
    } catch (const std::exception& error) {
        failure_ = std::string("cannot parse default field actor table: ") +
                   error.what();
        return false;
    }
    if (scene_record_counts[0] != 3U || scene_record_counts[1] != 8U) {
        failure_ = "default field actor table is incomplete";
        return false;
    }

    constexpr std::array<std::uint16_t, 2> kActorKinds{5U, 11U};
    std::array<std::string, 2> actor_asset_prefixes{};
    for (const auto& actor : field_default_actors_) {
        const auto kind_index = actor.entity_kind == 5U ? 0U : 1U;
        std::string prefix = "field.entities.actors.";
        prefix.reserve(prefix.size() + actor.sprite_stem.size());
        for (const char character : actor.sprite_stem) {
            prefix.push_back(character == '-' ? '.' : character);
        }
        if (actor_asset_prefixes[kind_index].empty()) {
            actor_asset_prefixes[kind_index] = std::move(prefix);
        } else if (actor_asset_prefixes[kind_index] != prefix) {
            failure_ = "default actor kind maps to multiple extracted sprite stems";
            return false;
        }
    }
    std::array<std::array<bool, 8>, 2> loaded_directions{};
    for (const auto& actor : field_default_actors_) {
        const auto kind_index = actor.entity_kind == 5U ? 0U : 1U;
        const auto direction_decision = game_scripts_.actor_heartbeat(
            scripting::ScriptActorBehaviorContext{
                actor.script_profile_id,
                actor.entity_kind,
                actor.action_kind,
                actor.initial_action_code,
                0U,
                false,
                false,
                actor.actor_instance_id,
                actor.initial_heading,
                true,
            });
        if (!direction_decision.has_value()) {
            failure_ = game_scripts_.failure();
            return false;
        }
        const auto direction = direction_decision->direction;
        if (loaded_directions[kind_index][direction]) {
            continue;
        }
        auto& artwork = field_default_actor_artwork_[kind_index];
        const auto direction_suffix = direction == 0U
            ? std::string{}
            : ".dir." + std::to_string(direction);
        const auto idle_alias = actor_asset_prefixes[kind_index] + ".idle" +
                                direction_suffix;
        if (!load_sprite_image(catalog, idle_alias,
                               artwork.idle_images[direction],
                               artwork.idle_origin_x[direction],
                               artwork.idle_origin_y[direction])) {
            return false;
        }
        for (std::size_t frame = 0; frame < 2U; ++frame) {
            const auto entry_alias =
                actor_asset_prefixes[kind_index] + ".entry.dir." +
                std::to_string(direction) + ".frame." +
                std::to_string(frame);
            if (!load_sprite_image(
                    catalog, entry_alias,
                    artwork.entry_images[direction][frame],
                    artwork.entry_origin_x[direction][frame],
                    artwork.entry_origin_y[direction][frame])) {
                return false;
            }
        }
        if (actor.entity_kind != kActorKinds[kind_index]) {
            failure_ = "default actor art kind mapping changed";
            return false;
        }
        loaded_directions[kind_index][direction] = true;
    }
    return true;
}

bool FieldSceneRuntime::load_dynamic_world_actor_artwork(
    const ConversionAssetCatalog& catalog) {
    constexpr std::string_view kActorTableAlias =
        "field.entities.world.activity.encounter.actors";
    constexpr std::array<std::string_view, 4> kTargetMarkerGroups{{
        "action.0.2", "action.3.5", "action.6.8", "action.9.plus",
    }};
    field_target_marker_images_by_scene_.clear();
    field_target_marker_images_by_scene_.resize(56U);
    for (std::size_t scene = 0U;
         scene < field_target_marker_images_by_scene_.size(); ++scene) {
        for (std::size_t style = 0U;
             style < kTargetMarkerGroups.size(); ++style) {
            const auto alias =
                "field.entities.target.marker.scene." +
                std::to_string(scene) + "." +
                std::string(kTargetMarkerGroups[style]);
            auto& image = field_target_marker_images_by_scene_[scene][style];
            if (!load_image(catalog, alias, image) || image.width() != 8 ||
                image.height() != 8) {
                if (failure_.empty()) {
                    failure_ = "extracted field target-marker tile is not 8x8: " +
                               alias;
                }
                return false;
            }
        }
    }

    field_hound_close_attack_images_by_scene_.clear();
    field_hound_close_attack_images_by_scene_.resize(56U);
    for (std::size_t scene = 0U;
         scene < field_hound_close_attack_images_by_scene_.size(); ++scene) {
        for (std::uint8_t direction = 0U; direction < 8U; ++direction) {
            for (std::uint8_t phase = 1U; phase <= 3U; ++phase) {
                const auto alias =
                    "field.entities.effects.hell.hound.close.attack.scene." +
                    std::to_string(scene) + ".heading." +
                    std::to_string(direction) + ".phase." +
                    std::to_string(phase);
                auto& artwork =
                    field_hound_close_attack_images_by_scene_[scene]
                        [direction];
                if (!load_sprite_image(
                        catalog, alias, artwork.images[phase],
                        artwork.origin_x[phase], artwork.origin_y[phase])) {
                    return false;
                }
            }
        }
    }

    field_spell_effect_images_by_scene_.clear();
    field_spell_effect_images_by_scene_.resize(56U);
    for (std::size_t scene = 0U;
         scene < field_spell_effect_images_by_scene_.size(); ++scene) {
        for (std::size_t profile_index = 0U;
             profile_index < kFieldSpellEffectProfiles.size();
             ++profile_index) {
            const auto& profile = kFieldSpellEffectProfiles[profile_index];
            std::string effect_alias(profile.asset_id);
            std::replace(effect_alias.begin(), effect_alias.end(), '-', '.');
            for (std::uint8_t phase = 0U;
                 phase < profile.phase_count; ++phase) {
                const auto alias =
                    "field.entities.effects.spell." + effect_alias +
                    ".scene." + std::to_string(scene) + ".phase." +
                    std::to_string(phase);
                auto& artwork =
                    field_spell_effect_images_by_scene_[scene][profile_index];
                if (!load_sprite_image(
                        catalog, alias, artwork.images[phase],
                        artwork.origin_x[phase], artwork.origin_y[phase])) {
                    return false;
                }
            }
        }
    }

    const auto* table = catalog.find(std::string(kActorTableAlias));
    if (table == nullptr || table->kind != "state") {
        failure_ = "missing extracted dynamic world-activity actor table";
        return false;
    }

    std::ifstream input(table->absolute_path, std::ios::binary);
    if (!input) {
        failure_ = "cannot read extracted dynamic world-activity actor table";
        return false;
    }

    field_dynamic_actor_artwork_.clear();
    std::string line;
    if (!std::getline(input, line) ||
        line != "scene\tcontext_group\trandom_sum\tselection_cell_offset"
                "\tsequence_list_offset\tsource_order\tactor_name\tprofile_id"
                "\taction_kind\taction_code\tsource_interaction_flags"
                "\tentity_kind\tpalette_line"
                "\tsprite_stem\ttagged_template\ttemplate_offset"
                "\tordered_overrides\tplacement_rule\tphysical_meter"
                "\tdelay_primary_attribute\tdelay_secondary_attribute"
                "\tdelay_capability_flags\tcombat_mental_meter"
                "\tcombat_cyberware_flags\tcombat_posture"
                "\tcombat_attribute_77\tcombat_attribute_78"
                "\tcombat_attribute_79\tcombat_attribute_7b"
                "\tcombat_attribute_7c\tcombat_attribute_85"
                "\tcombat_item18_base\tcombat_armor_index"
                "\tcombat_race_or_state\tcombat_class_code"
                "\tcombat_modifier_flags_df\tcombat_profile_index_a0"
                "\tcombat_item_code_0\tcombat_item_code_1"
                "\tcombat_item_code_2\tcombat_item_code_3"
                "\tcombat_item_code_4\tcombat_item_code_5"
                "\tcombat_item_code_6\tcombat_item_code_7"
                "\tcombat_item_phase_0\tcombat_item_phase_1"
                "\tcombat_item_phase_2\tcombat_item_phase_3"
                "\tcombat_item_phase_4\tcombat_item_phase_5"
                "\tcombat_item_phase_6\tcombat_item_phase_7"
                "\tcombat_action_phase_76\tcombat_action_phase_77"
                "\tcombat_action_phase_78\tcombat_action_phase_79"
                "\tcombat_action_phase_7a\tcombat_action_phase_7b"
                "\tcombat_action_phase_7c\tcombat_action_phase_7d"
                "\tcombat_action_phase_7e\tcombat_action_phase_7f"
                "\tcombat_action_skill_81\tcombat_action_skill_82"
                "\tcombat_action_skill_83\tcombat_action_skill_84"
                "\tcombat_auxiliary_roll_bonus"
                "\tsource_action_slot\tsource_action_ammo"
                "\tsource_action_counter"
                "\trun_target_area\trun_target_event"
                "\trun_target_x\trun_target_y") {
        failure_ = "dynamic world-activity actor table header is invalid";
        return false;
    }

    std::unordered_map<std::string, std::uint16_t> actor_kinds;
    std::vector<std::pair<std::string, std::uint16_t>> script_actor_kinds;
    try {
        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::istringstream row(line);
            std::string field;
            while (std::getline(row, field, '\t')) {
                fields.push_back(std::move(field));
            }
            if (fields.size() != 75U) {
                failure_ = "dynamic world-activity actor row is malformed";
                return false;
            }
            const auto kind = static_cast<std::uint16_t>(std::stoul(fields[11]));
            const auto physical_meter = std::stoul(fields[18]);
            const auto delay_primary_attribute = std::stoul(fields[19]);
            const auto delay_secondary_attribute = std::stoul(fields[20]);
            const auto delay_capability_flags = std::stoul(fields[21]);
            const auto source_action_slot = std::stoul(fields[68]);
            const auto source_action_ammo = std::stoul(fields[69]);
            const auto source_action_counter = std::stoul(fields[70]);
            const auto run_target_area = std::stoul(fields[71]);
            const auto run_target_event = std::stoul(fields[72]);
            const auto run_target_x = std::stoul(fields[73]);
            const auto run_target_y = std::stoul(fields[74]);
            if (kind < 4U || kind > 14U || fields[13].empty() ||
                physical_meter > 0xFFU ||
                delay_primary_attribute > 0xFFU ||
                delay_secondary_attribute > 0xFFU ||
                delay_capability_flags > 0xFFFFU ||
                source_action_slot > 0xFFU || source_action_ammo > 0xFFU ||
                source_action_counter > 0xFFU || run_target_area > 55U ||
                run_target_event > 0xFFU ||
                run_target_x > 0xFFFFU || run_target_y > 0xFFFFU ||
                ((run_target_area == 0U) != (run_target_event == 0U)) ||
                ((run_target_area == 0U) !=
                 (run_target_x == 0U && run_target_y == 0U))) {
                failure_ = "dynamic world-activity actor identity is invalid";
                return false;
            }
            const auto [position, inserted] =
                actor_kinds.emplace(fields[13], kind);
            if (!inserted && position->second != kind) {
                failure_ = "dynamic actor sprite stem maps to multiple kinds";
                return false;
            }
        }
    } catch (const std::exception& error) {
        failure_ = std::string("cannot parse dynamic actor table: ") +
                   error.what();
        return false;
    }

    const auto* script_actor_table =
        catalog.find("field.entities.world.activity.actors");
    if (script_actor_table == nullptr || script_actor_table->kind != "state") {
        failure_ = "missing extracted conditional world-activity actor table";
        return false;
    }
    std::ifstream script_actor_input(script_actor_table->absolute_path,
                                     std::ios::binary);
    if (!script_actor_input) {
        failure_ = "cannot read extracted conditional world-activity actor table";
        return false;
    }
    if (!std::getline(script_actor_input, line)) {
        failure_ = "conditional world-activity actor table header is invalid";
        return false;
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    std::vector<std::string> script_actor_columns;
    {
        std::istringstream header(line);
        std::string column;
        while (std::getline(header, column, '\t')) {
            script_actor_columns.push_back(std::move(column));
        }
    }
    const auto column_index = [&script_actor_columns](
                                 const std::string_view name)
        -> std::optional<std::size_t> {
        const auto found = std::find(script_actor_columns.begin(),
                                     script_actor_columns.end(), name);
        if (found == script_actor_columns.end()) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(
            std::distance(script_actor_columns.begin(), found));
    };
    const auto script_index_column = column_index("script_index");
    const auto entity_kind_column = column_index("entity_kind");
    const auto sprite_stem_column = column_index("sprite_stem");
    if (script_actor_columns.size() < 21U || !script_index_column ||
        !entity_kind_column || !sprite_stem_column) {
        failure_ = "conditional world-activity actor table header is invalid";
        return false;
    }
    try {
        while (std::getline(script_actor_input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::istringstream row(line);
            std::string field;
            while (std::getline(row, field, '\t')) {
                fields.push_back(std::move(field));
            }
            if (fields.size() != script_actor_columns.size() ||
                fields[*sprite_stem_column].empty()) {
                failure_ = "conditional world-activity actor row is malformed";
                return false;
            }
            const auto script_index =
                std::stoi(fields[*script_index_column]);
            if (script_index < 0) {
                continue;
            }
            const auto entity_kind = static_cast<std::uint16_t>(
                std::stoul(fields[*entity_kind_column]));
            if (script_index > 0xFF || entity_kind < 3U ||
                entity_kind > 14U) {
                failure_ = "conditional world-activity actor identity is invalid";
                return false;
            }
            script_actor_kinds.emplace_back(fields[*sprite_stem_column],
                                            entity_kind);
        }
    } catch (const std::exception& error) {
        failure_ = std::string("cannot parse conditional actor table: ") +
                   error.what();
        return false;
    }

    struct ActorActionProfile {
        std::uint16_t entity_kind = 0U;
        std::uint8_t frame_count = 0U;
        std::uint16_t frame_increment = 0U;
        std::uint16_t frame_threshold = 0U;
        std::uint16_t audio_event = 0U;
    };
    std::unordered_map<std::string, ActorActionProfile> action_profiles;
    const auto* action_profile_table =
        catalog.find("field.entities.actor.action.profiles");
    if (action_profile_table == nullptr ||
        action_profile_table->kind != "state") {
        failure_ = "missing extracted actor C0A2 action profile table";
        return false;
    }
    std::ifstream action_profile_input(
        action_profile_table->absolute_path, std::ios::binary);
    if (!action_profile_input) {
        failure_ = "cannot read extracted actor action profile table";
        return false;
    }
    if (!std::getline(action_profile_input, line) ||
        line != "sprite_stem\tentity_kind\tframe_count\tframe_increment"
                "\tframe_threshold\taudio_event") {
        failure_ = "actor action profile table header is invalid";
        return false;
    }
    const auto parse_action_profile_number =
        [](const std::string& value) -> std::uint32_t {
        std::size_t consumed = 0U;
        const auto parsed = std::stoul(value, &consumed, 10);
        if (consumed != value.size() || parsed > 0xFFFFFFFFUL) {
            throw std::invalid_argument("invalid actor action profile number");
        }
        return static_cast<std::uint32_t>(parsed);
    };
    try {
        while (std::getline(action_profile_input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::istringstream row(line);
            std::string field;
            while (std::getline(row, field, '\t')) {
                fields.push_back(std::move(field));
            }
            if (fields.size() != 6U || fields[0].empty()) {
                failure_ = "actor action profile row is malformed";
                return false;
            }
            const auto kind = parse_action_profile_number(fields[1]);
            const auto frame_count = parse_action_profile_number(fields[2]);
            const auto frame_increment =
                parse_action_profile_number(fields[3]);
            const auto frame_threshold =
                parse_action_profile_number(fields[4]);
            const auto audio_event = parse_action_profile_number(fields[5]);
            const auto expected_audio_event = kind == 12U ? 0x55U : 0x4CU;
            // Conditional field-script actors include kind 3 (scene 15's
            // ROM record at 0x170A2). Keep its extracted animation profile
            // available even though the dynamic encounter census remains
            // limited to kinds 4-14.
            if (kind < 3U || kind > 14U || frame_count == 0U ||
                frame_count > 8U || frame_increment == 0U ||
                frame_increment > 0xFFFFU || frame_threshold == 0U ||
                frame_threshold != 0x1000U ||
                audio_event != expected_audio_event) {
                failure_ = "actor action profile values are invalid";
                return false;
            }
            ActorActionProfile profile;
            profile.entity_kind = static_cast<std::uint16_t>(kind);
            profile.frame_count = static_cast<std::uint8_t>(frame_count);
            profile.frame_increment =
                static_cast<std::uint16_t>(frame_increment);
            profile.frame_threshold =
                static_cast<std::uint16_t>(frame_threshold);
            profile.audio_event = static_cast<std::uint16_t>(audio_event);
            if (!action_profiles.emplace(fields[0], profile).second) {
                failure_ = "actor action profile duplicates a sprite stem";
                return false;
            }
        }
    } catch (const std::exception& error) {
        failure_ = std::string("cannot parse actor action profiles: ") +
                   error.what();
        return false;
    }

    struct ActorItemActionProfile {
        std::uint16_t entity_kind = 0U;
        std::uint8_t frame_count = 0U;
        std::uint16_t frame_increment = 0U;
        std::uint16_t frame_threshold = 0U;
    };
    std::unordered_map<std::string, ActorItemActionProfile>
        item_action_profiles;
    const auto* item_action_profile_table =
        catalog.find("field.entities.actor.item.action.profiles");
    if (item_action_profile_table == nullptr ||
        item_action_profile_table->kind != "state") {
        failure_ = "missing extracted actor C042 item-action profile table";
        return false;
    }
    std::ifstream item_action_profile_input(
        item_action_profile_table->absolute_path, std::ios::binary);
    if (!item_action_profile_input) {
        failure_ = "cannot read extracted actor item-action profile table";
        return false;
    }
    if (!std::getline(item_action_profile_input, line) ||
        line != "sprite_stem\tentity_kind\tframe_count\tframe_increment"
                "\tframe_threshold") {
        failure_ = "actor item-action profile table header is invalid";
        return false;
    }
    try {
        while (std::getline(item_action_profile_input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::istringstream row(line);
            std::string field;
            while (std::getline(row, field, '\t')) {
                fields.push_back(std::move(field));
            }
            if (fields.size() != 5U || fields[0].empty()) {
                failure_ = "actor item-action profile row is malformed";
                return false;
            }
            const auto kind = parse_action_profile_number(fields[1]);
            const auto frame_count = parse_action_profile_number(fields[2]);
            const auto frame_increment =
                parse_action_profile_number(fields[3]);
            const auto frame_threshold =
                parse_action_profile_number(fields[4]);
            // Scene 15's conditional story actor is ROM kind 3. Its C042
            // profile is valid source data even though dynamic encounter
            // records remain constrained to kinds 4-14.
            if (kind < 3U || kind > 14U || frame_count == 0U ||
                frame_count > 8U || frame_increment == 0U ||
                frame_increment > 0xFFFFU || frame_threshold != 0x1000U) {
                failure_ = "actor item-action profile values are invalid";
                return false;
            }
            ActorItemActionProfile profile;
            profile.entity_kind = static_cast<std::uint16_t>(kind);
            profile.frame_count = static_cast<std::uint8_t>(frame_count);
            profile.frame_increment =
                static_cast<std::uint16_t>(frame_increment);
            profile.frame_threshold =
                static_cast<std::uint16_t>(frame_threshold);
            if (!item_action_profiles.emplace(fields[0], profile).second) {
                failure_ =
                    "actor item-action profiles duplicate a sprite stem";
                return false;
            }
        }
    } catch (const std::exception& error) {
        failure_ = std::string("cannot parse actor item-action profiles: ") +
                   error.what();
        return false;
    }

    struct ActorReactionProfile {
        std::uint16_t entity_kind = 0U;
        std::uint8_t hurt_frame_count = 0U;
        std::uint16_t hurt_increment = 0U;
        std::uint16_t frame_threshold = 0U;
        std::uint16_t hurt_audio_event = 0U;
        std::uint16_t down_audio_event = 0U;
    };
    std::unordered_map<std::string, ActorReactionProfile> reaction_profiles;
    const auto* reaction_profile_table =
        catalog.find("field.entities.actor.reaction.profiles");
    if (reaction_profile_table == nullptr ||
        reaction_profile_table->kind != "state") {
        failure_ = "missing extracted actor C072 reaction profile table";
        return false;
    }
    std::ifstream reaction_profile_input(
        reaction_profile_table->absolute_path, std::ios::binary);
    if (!reaction_profile_input) {
        failure_ = "cannot read extracted actor reaction profile table";
        return false;
    }
    if (!std::getline(reaction_profile_input, line) ||
        line != "sprite_stem\tentity_kind\thurt_frame_count\thurt_increment"
                "\tframe_threshold\thurt_audio_event\tdown_audio_event") {
        failure_ = "actor reaction profile table header is invalid";
        return false;
    }
    try {
        while (std::getline(reaction_profile_input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (line.empty()) {
                continue;
            }
            std::vector<std::string> fields;
            std::istringstream row(line);
            std::string field;
            while (std::getline(row, field, '\t')) {
                fields.push_back(std::move(field));
            }
            if (fields.size() != 7U || fields[0].empty()) {
                failure_ = "actor reaction profile row is malformed";
                return false;
            }
            const auto kind = parse_action_profile_number(fields[1]);
            const auto frame_count = parse_action_profile_number(fields[2]);
            const auto frame_increment =
                parse_action_profile_number(fields[3]);
            const auto frame_threshold =
                parse_action_profile_number(fields[4]);
            const auto hurt_audio_event =
                parse_action_profile_number(fields[5]);
            const auto down_audio_event =
                parse_action_profile_number(fields[6]);
            // Keep the conditional scene-15 kind-3 source profile alongside
            // the dynamic encounter profiles; it is not an encounter row.
            if (kind < 3U || kind > 14U || frame_count == 0U ||
                frame_count > 8U || frame_increment == 0U ||
                frame_increment > 0xFFFFU || frame_threshold != 0x1000U ||
                hurt_audio_event > 0xFFFFU || down_audio_event > 0xFFFFU) {
                failure_ = "actor reaction profile values are invalid";
                return false;
            }
            ActorReactionProfile profile;
            profile.entity_kind = static_cast<std::uint16_t>(kind);
            profile.hurt_frame_count =
                static_cast<std::uint8_t>(frame_count);
            profile.hurt_increment =
                static_cast<std::uint16_t>(frame_increment);
            profile.frame_threshold =
                static_cast<std::uint16_t>(frame_threshold);
            profile.hurt_audio_event =
                static_cast<std::uint16_t>(hurt_audio_event);
            profile.down_audio_event =
                static_cast<std::uint16_t>(down_audio_event);
            if (!reaction_profiles.emplace(fields[0], profile).second) {
                failure_ = "actor reaction profiles duplicate a sprite stem";
                return false;
            }
        }
    } catch (const std::exception& error) {
        failure_ = std::string("cannot parse actor reaction profiles: ") +
                   error.what();
        return false;
    }

    for (const auto& [sprite_stem, entity_kind] : script_actor_kinds) {
        const auto action_profile = action_profiles.find(sprite_stem);
        const auto item_action_profile = item_action_profiles.find(sprite_stem);
        const auto reaction_profile = reaction_profiles.find(sprite_stem);
        std::string alias_prefix = "field.entities.actors.";
        for (const char character : sprite_stem) {
            alias_prefix.push_back(character == '-' ? '.' : character);
        }
        // Some ROM-selected script rows intentionally remain visible as
        // source data until their profile resources have been extracted. Do
        // not fail unrelated field scenes for those rows; selection of an
        // unsupported row is reported at the actor publication boundary.
        if (action_profile == action_profiles.end() ||
            action_profile->second.entity_kind != entity_kind ||
            item_action_profile == item_action_profiles.end() ||
            item_action_profile->second.entity_kind != entity_kind ||
            reaction_profile == reaction_profiles.end() ||
            reaction_profile->second.entity_kind != entity_kind ||
            catalog.find(alias_prefix + ".idle") == nullptr) {
            continue;
        }
        const auto [existing, inserted] =
            actor_kinds.emplace(sprite_stem, entity_kind);
        if (!inserted && existing->second != entity_kind) {
            failure_ = "conditional actor sprite stem maps to multiple kinds";
            return false;
        }
    }

    for (const auto& [sprite_stem, entity_kind] : actor_kinds) {
        std::string alias_prefix = "field.entities.actors.";
        alias_prefix.reserve(alias_prefix.size() + sprite_stem.size());
        for (const char character : sprite_stem) {
            alias_prefix.push_back(character == '-' ? '.' : character);
        }

        DefaultActorArtwork artwork{};
        const auto action_profile = action_profiles.find(sprite_stem);
        if (action_profile == action_profiles.end() ||
            action_profile->second.entity_kind != entity_kind) {
            failure_ = "dynamic actor has no matching C0A2 action profile";
            return false;
        }
        artwork.action_frame_count = action_profile->second.frame_count;
        artwork.action_frame_increment =
            action_profile->second.frame_increment;
        artwork.action_frame_threshold =
            action_profile->second.frame_threshold;
        artwork.action_audio_event = action_profile->second.audio_event;
        const auto item_action_profile =
            item_action_profiles.find(sprite_stem);
        if (item_action_profile == item_action_profiles.end() ||
            item_action_profile->second.entity_kind != entity_kind) {
            failure_ = "dynamic actor has no matching C042 item-action profile";
            return false;
        }
        artwork.item_action_frame_count =
            item_action_profile->second.frame_count;
        artwork.item_action_increment =
            item_action_profile->second.frame_increment;
        artwork.item_action_threshold =
            item_action_profile->second.frame_threshold;
        const auto reaction_profile = reaction_profiles.find(sprite_stem);
        if (reaction_profile == reaction_profiles.end() ||
            reaction_profile->second.entity_kind != entity_kind) {
            failure_ = "dynamic actor has no matching C072 reaction profile";
            return false;
        }
        artwork.hurt_frame_count =
            reaction_profile->second.hurt_frame_count;
        artwork.hurt_frame_increment =
            reaction_profile->second.hurt_increment;
        artwork.hurt_frame_threshold =
            reaction_profile->second.frame_threshold;
        artwork.hurt_audio_event =
            reaction_profile->second.hurt_audio_event;
        artwork.down_audio_event =
            reaction_profile->second.down_audio_event;
        for (std::uint8_t direction = 0U; direction < 8U; ++direction) {
            const auto suffix = direction == 0U
                ? std::string{}
                : ".dir." + std::to_string(direction);
            const auto alias = alias_prefix + ".idle" + suffix;
            if (!load_sprite_image(
                    catalog, alias,
                    artwork.idle_images[direction],
                    artwork.idle_origin_x[direction],
                    artwork.idle_origin_y[direction])) {
                return false;
            }
            const auto spell_alias = alias_prefix + ".spell" + suffix;
            if (!load_sprite_image(
                    catalog, spell_alias,
                    artwork.spell_images[direction],
                    artwork.spell_origin_x[direction],
                    artwork.spell_origin_y[direction])) {
                return false;
            }
            std::uint8_t entry_frames = 0U;
            for (std::uint8_t frame = 0U; frame < 8U; ++frame) {
                const auto entry_alias =
                    alias_prefix + ".entry.dir." +
                    std::to_string(direction) + ".frame." +
                    std::to_string(frame);
                if (catalog.find(entry_alias) == nullptr) {
                    break;
                }
                if (!load_sprite_image(
                        catalog, entry_alias,
                        artwork.entry_images[direction][frame],
                        artwork.entry_origin_x[direction][frame],
                        artwork.entry_origin_y[direction][frame])) {
                    return false;
                }
                ++entry_frames;
            }
            if (entry_frames == 0U) {
                failure_ =
                    "dynamic actor entry animation is missing a direction";
                return false;
            }
            if (direction == 0U) {
                artwork.entry_frame_count = entry_frames;
            } else if (entry_frames != artwork.entry_frame_count) {
                failure_ =
                    "dynamic actor entry animation frame counts disagree";
                return false;
            }

            std::uint8_t walk_frames = 0U;
            for (std::uint8_t frame = 0U; frame < 8U; ++frame) {
                const auto walk_alias =
                    alias_prefix + ".walk.dir." +
                    std::to_string(direction) + ".frame." +
                    std::to_string(frame);
                if (catalog.find(walk_alias) == nullptr) {
                    break;
                }
                artwork.walk_images[direction].emplace_back();
                artwork.walk_origin_x[direction].push_back(0);
                artwork.walk_origin_y[direction].push_back(0);
                const auto image_index =
                    artwork.walk_images[direction].size() - 1U;
                if (!load_sprite_image(
                        catalog, walk_alias,
                        artwork.walk_images[direction][image_index],
                        artwork.walk_origin_x[direction][image_index],
                        artwork.walk_origin_y[direction][image_index])) {
                    return false;
                }
                ++walk_frames;
            }
            if (walk_frames == 0U) {
                failure_ = "dynamic actor C062 walk art is missing a direction";
                return false;
            }
            if (direction == 0U) {
                artwork.walk_frame_count = walk_frames;
            } else if (walk_frames != artwork.walk_frame_count) {
                failure_ =
                    "dynamic actor C062 walk-list lengths disagree by direction";
                return false;
            }

            for (std::uint8_t frame = 0U;
                 frame < artwork.action_frame_count; ++frame) {
                const auto action_alias =
                    alias_prefix + ".action.dir." +
                    std::to_string(direction) + ".frame." +
                    std::to_string(frame);
                artwork.action_images[direction].emplace_back();
                artwork.action_origin_x[direction].push_back(0);
                artwork.action_origin_y[direction].push_back(0);
                const auto image_index =
                    artwork.action_images[direction].size() - 1U;
                if (!load_sprite_image(
                        catalog, action_alias,
                        artwork.action_images[direction][image_index],
                        artwork.action_origin_x[direction][image_index],
                        artwork.action_origin_y[direction][image_index])) {
                    return false;
                }
            }

            for (std::uint8_t frame = 0U;
                 frame < artwork.item_action_frame_count; ++frame) {
                const auto item_action_alias =
                    alias_prefix + ".item.action.dir." +
                    std::to_string(direction) + ".frame." +
                    std::to_string(frame);
                artwork.item_action_images[direction].emplace_back();
                artwork.item_action_origin_x[direction].push_back(0);
                artwork.item_action_origin_y[direction].push_back(0);
                const auto image_index =
                    artwork.item_action_images[direction].size() - 1U;
                if (!load_sprite_image(
                        catalog, item_action_alias,
                        artwork.item_action_images[direction][image_index],
                        artwork.item_action_origin_x[direction][image_index],
                        artwork.item_action_origin_y[direction][image_index])) {
                    return false;
                }
            }

            for (std::uint8_t frame = 0U;
                 frame < artwork.hurt_frame_count; ++frame) {
                const auto hurt_alias =
                    alias_prefix + ".hurt.dir." +
                    std::to_string(direction) + ".frame." +
                    std::to_string(frame);
                artwork.hurt_images[direction].emplace_back();
                artwork.hurt_origin_x[direction].push_back(0);
                artwork.hurt_origin_y[direction].push_back(0);
                const auto image_index =
                    artwork.hurt_images[direction].size() - 1U;
                if (!load_sprite_image(
                        catalog, hurt_alias,
                        artwork.hurt_images[direction][image_index],
                        artwork.hurt_origin_x[direction][image_index],
                        artwork.hurt_origin_y[direction][image_index])) {
                    return false;
                }
            }
        }
        if (entity_kind < 3U || entity_kind > 14U) {
            failure_ = "dynamic actor source kind is outside the supported range";
            return false;
        }
        field_dynamic_actor_artwork_.emplace(sprite_stem,
                                              std::move(artwork));
    }
    return true;
}

void FieldSceneRuntime::initialize_default_field_actors(
    const std::uint16_t scene_reference) {
    field_default_actors_.erase(
        std::remove_if(
            field_default_actors_.begin(), field_default_actors_.end(),
            [scene_reference](const DefaultFieldActorPresentation& actor) {
                const bool transient_actor =
                    actor.temporary_world_encounter ||
                    actor.world_activity_script_actor;
                return transient_actor &&
                       actor.scene_reference != scene_reference;
            }),
        field_default_actors_.end());
    field_world_effect_slots_occupied_.fill(false);
    for (const auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != scene_reference ||
            !actor.effect_slot.has_value() ||
            *actor.effect_slot >= field_world_effect_slots_occupied_.size()) {
            continue;
        }
        field_world_effect_slots_occupied_[*actor.effect_slot] = true;
    }
    field_spell_effect_slot_target_ids_.fill(std::string{});
    field_entity_target_owner_ = EntityServiceTargetOwner{};
    field_entity_target_records_.assign(16U, EntityServiceTargetRecord{});
    pending_field_encounter_outcome_.reset();
    if (!pending_world_activity_script_actor_scene_.has_value() ||
        *pending_world_activity_script_actor_scene_ != scene_reference) {
        pending_world_activity_script_actors_.clear();
        pending_world_activity_script_actor_scene_.reset();
    }

    for (auto& actor : field_default_actors_) {
        if (actor.active && actor.scene_reference == scene_reference &&
            (actor.temporary_world_encounter ||
             actor.world_activity_script_actor)) {
            continue;
        }
        actor.active = false;
        actor.source_record_slot.reset();
        actor.target_selected = false;
        actor.source_interaction_flags = 0U;
        actor.ai_target_valid = false;
        actor.ai_target_id.clear();
        actor.ai_target_heading = 0U;
        actor.ai_target_distance = 0U;
        actor.ai_package_id.clear();
        actor.ai_weapon_item_id.reset();
    }

    // ROM 0x43B6 drains the FFE182-selected list once per scene entry. The
    // repeated destination-entry pass (world-activity entry marker) keeps the
    // records that drain already published, and FFE182 stays armed until the
    // actor-pair gate consumes it, so that pass must not select the list a
    // second time and publish a duplicate actor.
    const bool preserved_scene_script_actors = std::any_of(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.active && actor.scene_reference == scene_reference &&
                   actor.world_activity_script_actor;
        });
    const auto queue_pending_actor_list =
        [this, scene_reference](
            std::optional<std::vector<scripting::ScriptWorldActivityActor>>
                actors,
            const std::string_view unresolved_boundary) {
        if (!actors.has_value() || actors->empty()) {
            failure_ = game_scripts_.failure().empty()
                ? "selected pending world-activity list has no extracted actors"
                : game_scripts_.failure();
            source_boundary(std::string(unresolved_boundary));
        } else if (actors->size() > kFieldWorldActorSlots.size()) {
            failure_ =
                "selected pending world-activity list exceeds live actor slots";
            source_boundary("field.world_activity.pending_list_slot_overflow");
        } else {
            pending_world_activity_script_actor_scene_ = scene_reference;
            pending_world_activity_script_actors_ = std::move(*actors);
            source_boundary("field.world_activity.pending_actor_list_queued");
        }
    };
    if (field_pending_route_actor_list_.has_value() &&
        scene_reference == field_pending_route_actor_list_->consume_scene) {
        const auto pending_route_list = *field_pending_route_actor_list_;
        field_pending_route_actor_list_.reset();
        char list_id[32]{};
        std::snprintf(list_id, sizeof(list_id), "pending_list_%05x",
                      static_cast<unsigned>(pending_route_list.list));
        // The extractor records the reusable source list in the source
        // scene's activity table (Mako: Scene 47). The ROM's 0x43B6 consumer
        // drains FFE120 without a scene check; the Club finalizer enters
        // Scene 17 before that pass.
        auto actors = game_scripts_.world_activity_actors_for_list_id(
            pending_route_list.source_scene, list_id);
        const bool has_source_actor_list =
            actors.has_value() &&
            actors->size() == pending_route_list.transitions;
        queue_pending_actor_list(
            std::move(actors),
            "field.world_activity." + pending_route_list.name +
                "_list_unresolved");
        if (has_source_actor_list &&
            pending_world_activity_script_actor_scene_ == scene_reference &&
            pending_world_activity_script_actors_.size() ==
                pending_route_list.transitions) {
            field_pending_actor_list_transition_group_ = pending_route_list;
            field_active_route_actor_list_ = pending_route_list.name;
            // ROM 0x43B6 seeds FFE177 with the actor behavior transitions
            // still outstanding for the list (four for 0x1E628).
            field_route_actor_transitions_remaining_ =
                pending_route_list.transitions;
        }
    } else if (field_pending_mode_transition_actor_list_.has_value()) {
        const auto list_id = field_mode_pending_actor_list_id(
            *field_pending_mode_transition_actor_list_);
        auto actors = list_id.empty()
            ? std::optional<
                  std::vector<scripting::ScriptWorldActivityActor>>{}
            : game_scripts_.world_activity_actors_for_list_id(
                  scene_reference, list_id);
        field_pending_mode_transition_actor_list_.reset();
        queue_pending_actor_list(std::move(actors),
                                 "field.mode_transition.actor_list_unresolved");
    } else if (!preserved_scene_script_actors) {
        const auto selected_pending_list =
            game_scripts_.pending_world_activity_actor_list_for_scene(
                scene_reference, field_world_effect_active_,
                field_pending_world_effect_);
        if (!selected_pending_list.has_value() &&
            !game_scripts_.failure().empty()) {
            failure_ = game_scripts_.failure();
            source_boundary(
                "field.world_activity.pending_list_selection_failed");
        } else if (selected_pending_list.has_value()) {
            auto actors = game_scripts_.world_activity_actors_for_source_list(
                scene_reference, *selected_pending_list);
            queue_pending_actor_list(
                std::move(actors),
                "field.world_activity.pending_list_unresolved");
        }
    }
    // Records kept from this scene's earlier destination-entry drain still
    // occupy their live slots ahead of the default list.
    const auto preserved_transient_actor_count = static_cast<std::size_t>(
        std::count_if(
            field_default_actors_.begin(), field_default_actors_.end(),
            [scene_reference](const DefaultFieldActorPresentation& actor) {
                return actor.active &&
                       actor.scene_reference == scene_reference &&
                       (actor.temporary_world_encounter ||
                        actor.world_activity_script_actor);
            }));
    const auto pending_actor_slot_count =
        pending_world_activity_script_actors_.size() +
        preserved_transient_actor_count;
    // 0x43B6 drains the selected list before the default list. Publish now so
    // its actor takes the lowest live record and advances the shared PRNG
    // before defaults initialize their delay bytes.
    publish_pending_world_activity_script_actors(scene_reference);

    // ROM 0x43B6 drains pending lists before the default list, so their
    // descriptors reserve the first ordinary records and shorten the later
    // default population when the seven-record pool fills.
    std::size_t next_source_slot =
        pending_actor_slot_count;
    for (auto& actor : field_default_actors_) {
        if (actor.temporary_world_encounter ||
            actor.world_activity_script_actor) {
            continue;
        }
        if (actor.scene_reference != scene_reference) {
            continue;
        }
        // For this default-list-only projection, 0x1A342's seven ordinary
        // actor slots are available as 3..9. 0x4466 stops at the first row
        // that cannot allocate; later rows remain definitions, not actors.
        // Earlier 0xA800 pending-list occupancy is reserved above.
        if (next_source_slot >= kFieldWorldActorSlots.size()) {
            continue;
        }
        actor.source_record_slot = kFieldWorldActorSlots[next_source_slot++];
        actor.active = true;
        // ROM 0x44BC copies the common +0x10 service byte 0x05 before it
        // publishes a template actor into the seven world-actor records.
        actor.source_record_flags = static_cast<std::uint8_t>(
            actor_flag_mask(ActorServiceFlag::active) |
            actor_flag_mask(ActorServiceFlag::unresolved_service_bit_2));
        actor.source_behavior_flags = 0U;
        actor.initial_behavior_delay = static_cast<std::uint8_t>(
            advance_source_random_seed(field_random_seed_) & 0x000FU);
        const auto decision = game_scripts_.actor_heartbeat(
            scripting::ScriptActorBehaviorContext{
                actor.script_profile_id,
                actor.entity_kind,
                actor.action_kind,
                actor.initial_action_code,
                actor.initial_behavior_delay,
                false,
                false,
                actor.actor_instance_id,
                actor.initial_heading,
                true,
            });
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.actor.lua_package_failed");
            actor.active = false;
            continue;
        }
        actor.ai_package_id = decision->package_id;
        actor.ai_weapon_item_id = decision->weapon_item_id;
        actor.ai_target_id = decision->ai_target_id.value_or("");
        actor.ai_target_heading = decision->ai_target_heading.value_or(0U);
        actor.ai_target_distance = decision->ai_target_distance.value_or(0U);
        actor.action_delay = decision->action_delay;
        if (decision->target_motion_effect_slot_request.has_value()) {
            const auto slot = *decision->target_motion_effect_slot_request;
            if (slot >= field_world_effect_slots_occupied_.size() ||
                field_world_effect_slots_occupied_[slot]) {
                source_boundary(
                    "field.actor.hound_effect_slot_request_invalid");
            } else {
                field_world_effect_slots_occupied_[slot] = true;
                actor.target_motion_effect_slot = slot;
                source_boundary("field.actor.hound_effect_started");
            }
        }
        actor.target_motion_effect_visible =
            decision->target_motion_effect_visible;
        actor.target_motion_effect_direction =
            decision->target_motion_effect_direction;
        actor.target_motion_effect_phase =
            decision->target_motion_effect_phase;
        if (decision->target_motion_effect_slot_released) {
            if (!actor.target_motion_effect_slot.has_value() ||
                !field_world_effect_slots_occupied_[
                    *actor.target_motion_effect_slot]) {
                source_boundary(
                    "field.actor.hound_effect_slot_release_invalid");
            } else {
                field_world_effect_slots_occupied_[
                    *actor.target_motion_effect_slot] = false;
                actor.target_motion_effect_slot.reset();
                source_boundary("field.actor.hound_effect_finished");
            }
        }
        actor.animation_direction = decision->direction;
        actor.animation_id = decision->animation_id;
        actor.animation_frame = decision->animation_frame;
        actor.transition_started = decision->transition_started;
        actor.transition_complete = decision->transition_complete;
    }
}

void FieldSceneRuntime::publish_pending_world_activity_script_actors(
    const std::uint16_t scene_reference) {
    if (!pending_world_activity_script_actor_scene_.has_value() ||
        *pending_world_activity_script_actor_scene_ != scene_reference ||
        pending_world_activity_script_actors_.empty()) {
        return;
    }

    const auto expected_actor_count =
        pending_world_activity_script_actors_.size();
    std::size_t published = 0U;
    for (const auto& source_actor : pending_world_activity_script_actors_) {
        const auto artwork =
            field_dynamic_actor_artwork_.find(source_actor.sprite_stem);
        if (source_actor.entity_kind < 3U || source_actor.entity_kind > 14U ||
            artwork == field_dynamic_actor_artwork_.end()) {
            failure_ = "selected ROM world-script actor has no extracted Lua profile and art";
            source_boundary(
                "field.world_activity.script_actor_art_unresolved");
            break;
        }

        std::array<bool, 7> occupied_actor_slots{};
        for (const auto& actor : field_default_actors_) {
            if (!actor.active || actor.scene_reference != scene_reference ||
                !actor.source_record_slot.has_value()) {
                continue;
            }
            const auto slot = static_cast<std::size_t>(
                *actor.source_record_slot);
            if (slot >= 3U && slot <= 9U) {
                occupied_actor_slots[slot - 3U] = true;
            }
        }
        const auto actor_slot = std::find(occupied_actor_slots.begin(),
                                          occupied_actor_slots.end(), false);
        if (actor_slot == occupied_actor_slots.end()) {
            source_boundary(
                "field.world_activity.script_actor_slots_exhausted");
            break;
        }
        const auto effect_slot = std::find(
            field_world_effect_slots_occupied_.begin(),
            field_world_effect_slots_occupied_.end(), false);
        if (effect_slot == field_world_effect_slots_occupied_.end()) {
            source_boundary(
                "field.world_activity.script_actor_effect_slots_exhausted");
            break;
        }

        DefaultFieldActorPresentation actor{};
        actor.active = true;
        actor.world_activity_script_actor = true;
        // The new slot is live even though its ROM spawn-state byte is kept
        // separately for Lua behavior and presentation.
        actor.source_record_flags = 0x01U;
        actor.scene_reference = scene_reference;
        actor.source_order = source_actor.source_order;
        actor.source_record_slot = static_cast<FieldWorldActorSlot>(
            std::distance(occupied_actor_slots.begin(), actor_slot) + 3);
        actor.entity_kind = source_actor.entity_kind;
        const auto actor_source_key = source_actor.script_index >= 0
            ? "world_script_" +
                  std::to_string(source_actor.script_index)
            : "world_list_" + std::to_string(source_actor.list_offset);
        actor.actor_instance_id =
            "field_scene_" + std::to_string(scene_reference) +
            "_" + actor_source_key +
            "_actor_" + std::to_string(source_actor.source_order);
        actor.script_profile_id = source_actor.profile_id;
        actor.action_kind = source_actor.action_kind;
        actor.initial_action_code = source_actor.action_code;
        actor.world_position = SceneRunnerPosition{
            source_actor.world_x, source_actor.world_y};
        actor.initial_heading = source_actor.heading;
        actor.palette_line = source_actor.palette_line;
        actor.physical_meter = source_actor.physical_meter;
        actor.delay_primary_attribute = source_actor.delay_primary_attribute;
        actor.delay_secondary_attribute = source_actor.delay_secondary_attribute;
        actor.delay_capability_flags = source_actor.delay_capability_flags;
        actor.sprite_stem = source_actor.sprite_stem;
        actor.template_offset = source_actor.template_offset;
        actor.position_offset = source_actor.position_offset;
        actor.list_offset = source_actor.list_offset;
        actor.ordered_overrides = source_actor.ordered_overrides;
        actor.source_record_flags = static_cast<std::uint8_t>(
            actor_flag_mask(ActorServiceFlag::active) |
            actor_flag_mask(ActorServiceFlag::unresolved_service_bit_2) |
            ((source_actor.tagged_template & 0x80000000U) != 0U
                 ? actor_flag_mask(ActorServiceFlag::activity_probe_enabled)
                 : 0U));
        actor.source_interaction_flags =
            source_actor.source_interaction_flags;
        actor.source_spawn_state_flags =
            source_actor.source_spawn_state_flags;
        if (field_pending_actor_list_transition_group_.has_value()) {
            actor.actor_list_completion_group =
                field_pending_actor_list_transition_group_->name;
        } else {
            actor.actor_list_completion_group.reset();
        }
        actor.source_action_slot = source_actor.source_action_slot;
        actor.source_action_ammo = source_actor.source_action_ammo;
        actor.source_action_counter = source_actor.source_action_counter;
        actor.combatant = source_actor.combatant;
        actor.combatant.record_type = static_cast<std::uint16_t>(
            *actor.source_record_slot);
        actor.combatant.source_flags = actor.source_record_flags;
        actor.combatant.interaction_flags = actor.source_interaction_flags;
        actor.combatant.physical_meter = actor.physical_meter;
        actor.combatant.equipped_action_code = actor.initial_action_code;
        actor.effect_slot = static_cast<std::uint8_t>(std::distance(
            field_world_effect_slots_occupied_.begin(), effect_slot));
        actor.initial_behavior_delay = static_cast<std::uint8_t>(
            advance_source_random_seed(field_random_seed_) & 0x000FU);
        actor.animation_direction = static_cast<std::uint8_t>(
            field_movement_direction_from_heading(actor.initial_heading));
        actor.source_animation_selector = static_cast<std::uint16_t>(
            actor.animation_direction * 4U);
        actor.movement_state.position_x = actor.world_position.x;
        actor.movement_state.position_y = actor.world_position.y;
        actor.movement_state.heading = actor.initial_heading;
        actor.movement_state.animation_heading = actor.initial_heading;
        actor.movement_state.collision_extent = 0U;
        actor.movement_state_initialized = true;

        const auto& actor_artwork = artwork->second;
        actor.entry_animation_frame_count = actor_artwork.entry_frame_count;
        actor.walk_animation_frame_count = actor_artwork.walk_frame_count;
        actor.action_animation_frame_count =
            actor_artwork.action_frame_count;
        actor.action_animation_increment =
            actor_artwork.action_frame_increment;
        actor.action_animation_threshold =
            actor_artwork.action_frame_threshold;
        actor.action_animation_audio_event =
            actor_artwork.action_audio_event;
        actor.item_action_frame_count =
            actor_artwork.item_action_frame_count;
        actor.item_action_increment = actor_artwork.item_action_increment;
        actor.item_action_threshold = actor_artwork.item_action_threshold;
        actor.hurt_animation_frame_count = actor_artwork.hurt_frame_count;
        actor.hurt_animation_increment = actor_artwork.hurt_frame_increment;
        actor.hurt_animation_threshold = actor_artwork.hurt_frame_threshold;
        actor.hurt_audio_event = actor_artwork.hurt_audio_event;
        actor.down_audio_event = actor_artwork.down_audio_event;

        scripting::ScriptActorBehaviorContext context{
            actor.script_profile_id,
            actor.entity_kind,
            actor.action_kind,
            actor.initial_action_code,
            actor.initial_behavior_delay,
            false,
            false,
            actor.actor_instance_id,
            actor.initial_heading,
            true,
        };
        context.dynamic_world_encounter_actor = true;
        context.world_activity_script_actor =
            actor.world_activity_script_actor;
        context.combatant = actor.combatant;
        context.source_actor_posture = actor.combatant.posture;
        context.source_record_flags = actor.source_record_flags;
        context.source_interaction_flags = actor.source_interaction_flags;
        context.source_behavior_flags = actor.source_behavior_flags;
        context.source_action_slot = actor.source_action_slot;
        context.source_action_ammo = actor.source_action_ammo;
        context.source_action_counter = actor.source_action_counter;
        context.source_animation_selector = actor.source_animation_selector;
        context.source_spawn_state_flags = actor.source_spawn_state_flags;
        context.source_record_slot = static_cast<std::uint8_t>(
            *actor.source_record_slot);
        context.world_activity_active =
            world_frame_logic_state_.world_active != 0U;
        context.world_scene_state = world_frame_logic_state_.scene_state;
        context.movement_scene_id = scene_reference;
        context.entry_animation_frame_count =
            actor.entry_animation_frame_count;
        context.walk_animation_frame_count = actor.walk_animation_frame_count;
        context.action_animation_frame_count =
            actor.action_animation_frame_count;
        context.action_animation_increment =
            actor.action_animation_increment;
        context.action_animation_threshold =
            actor.action_animation_threshold;
        context.action_animation_audio_event =
            actor.action_animation_audio_event;
        context.item_action_frame_count = actor.item_action_frame_count;
        context.item_action_increment = actor.item_action_increment;
        context.item_action_threshold = actor.item_action_threshold;
        context.hurt_animation_frame_count =
            actor.hurt_animation_frame_count;
        context.hurt_animation_increment = actor.hurt_animation_increment;
        context.hurt_animation_threshold = actor.hurt_animation_threshold;
        context.hurt_audio_event = actor.hurt_audio_event;
        context.down_audio_event = actor.down_audio_event;
        context.delay_primary_attribute = actor.delay_primary_attribute;
        context.delay_secondary_attribute = actor.delay_secondary_attribute;
        context.delay_capability_flags = actor.delay_capability_flags;
        context.action_profile_modifiers =
            script_action_profile_modifiers(state_.action_selection_profiles);
        context.action_attribute_selectors =
            script_action_attribute_selectors();
        context.combat_meter_service_blocked =
            field_movement_transition_state_.world_mode == 3U;
        context.world_effect_slot_occupancy =
            field_world_effect_slots_occupied_;
        context.world_effect_slot_occupancy[actor.effect_slot.value()] = true;
        context.source_projection_active = caster_projection_active(
            actor.world_position.x, actor.world_position.y,
            static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x),
            static_cast<std::uint16_t>(state_.field_scene_buffers.camera_y));
        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary(
                "field.world_activity.script_actor_lua_package_failed");
            break;
        }

        actor.ai_package_id = decision->package_id;
        actor.ai_weapon_item_id = decision->weapon_item_id;
        actor.ai_target_id = decision->ai_target_id.value_or("");
        actor.ai_target_heading = decision->ai_target_heading.value_or(0U);
        actor.ai_target_distance = decision->ai_target_distance.value_or(0U);
        actor.action_delay = decision->action_delay;
        actor.source_record_flags = decision->source_record_flags;
        actor.source_behavior_flags = decision->source_behavior_flags;
        actor.source_animation_selector =
            decision->source_animation_selector;
        actor.animation_direction = decision->direction;
        actor.animation_id = decision->animation_id;
        actor.animation_frame = decision->animation_frame;
        actor.transition_started = decision->transition_started;
        actor.transition_complete = decision->transition_complete;

        field_world_effect_slots_occupied_[actor.effect_slot.value()] = true;
        field_default_actors_.push_back(std::move(actor));
        ++published;
        source_boundary(
            "field.world_activity.script_actor_published_from_rom_list");
    }

    pending_world_activity_script_actors_.clear();
    pending_world_activity_script_actor_scene_.reset();
    if (field_pending_actor_list_transition_group_.has_value()) {
        const auto& group = *field_pending_actor_list_transition_group_;
        if (expected_actor_count != group.transitions ||
            published != expected_actor_count) {
            for (auto& actor : field_default_actors_) {
                if (actor.actor_list_completion_group == group.name) {
                    actor.actor_list_completion_group.reset();
                }
            }
            field_route_actor_transitions_remaining_.reset();
            source_boundary("field.world_activity." + group.name +
                            "_actor_list_incomplete");
        }
        field_pending_actor_list_transition_group_.reset();
    }
    if (published != 0U) {
        refresh_world_entity_service_targets();
        state_.display_dirty_flags = static_cast<std::uint8_t>(
            state_.display_dirty_flags | 0x0EU);
        field_runtime_state_.display_dirty_flags = static_cast<std::uint8_t>(
            field_runtime_state_.display_dirty_flags | 0x0EU);
    }
}

void FieldSceneRuntime::advance_default_field_actors() {
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    publish_pending_world_activity_script_actors(current_scene);
    for (auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene) {
            continue;
        }
        const bool dynamic_world_actor = actor.temporary_world_encounter ||
                                         actor.world_activity_script_actor;
        scripting::ScriptActorBehaviorContext context{
            actor.script_profile_id,
            actor.entity_kind,
            actor.action_kind,
            actor.initial_action_code,
            actor.initial_behavior_delay,
            actor.transition_started,
            actor.transition_complete,
            actor.actor_instance_id,
            dynamic_world_actor
                ? actor.movement_state.heading
                : actor.initial_heading,
            false,
        };
        context.walk_fast_enabled = field_walk_fast_enabled_;
        context.target_selection_latched = actor.target_selected;
        context.action_profile_modifiers =
            script_action_profile_modifiers(state_.action_selection_profiles);
        context.action_attribute_selectors =
            script_action_attribute_selectors();
        context.combat_meter_service_blocked =
            field_movement_transition_state_.world_mode == 3U;
        if (actor.source_record_slot.has_value()) {
            context.source_record_slot = static_cast<std::uint8_t>(
                *actor.source_record_slot);
            context.source_record_flags = actor.source_record_flags;
            context.source_physical_meter = actor.physical_meter;
        }
        if (dynamic_world_actor) {
            context.combatant = actor.combatant;
            context.source_actor_posture = actor.combatant.posture;
        }
        if (dynamic_world_actor) {
            context.dynamic_world_encounter_actor = true;
            context.world_activity_script_actor =
                actor.world_activity_script_actor;
            context.source_record_flags = actor.source_record_flags;
            context.source_interaction_flags =
                actor.source_interaction_flags;
            context.source_behavior_flags = actor.source_behavior_flags;
            context.source_action_slot = actor.source_action_slot;
            context.source_action_ammo = actor.source_action_ammo;
            context.source_action_counter = actor.source_action_counter;
            context.source_animation_selector =
                actor.source_animation_selector;
            context.source_item_spawn_result =
                actor.source_item_spawn_result;
            context.source_spawn_state_flags =
                actor.source_spawn_state_flags;
            context.world_activity_active =
                world_frame_logic_state_.world_active != 0U;
            context.world_scene_state =
                world_frame_logic_state_.scene_state;
            context.movement_scene_id = current_scene;
            context.entry_animation_frame_count =
                actor.entry_animation_frame_count;
            context.ai_animation_frame = actor.animation_frame;
            context.walk_animation_frame_count =
                actor.walk_animation_frame_count;
            context.action_animation_frame_count =
                actor.action_animation_frame_count;
            context.action_animation_increment =
                actor.action_animation_increment;
            context.action_animation_threshold =
                actor.action_animation_threshold;
            context.item_action_frame_count =
                actor.item_action_frame_count;
            context.item_action_increment = actor.item_action_increment;
            context.item_action_threshold = actor.item_action_threshold;
            context.action_animation_audio_event =
                actor.action_animation_audio_event;
            context.hurt_animation_frame_count =
                actor.hurt_animation_frame_count;
            context.hurt_animation_increment =
                actor.hurt_animation_increment;
            context.hurt_animation_threshold =
                actor.hurt_animation_threshold;
            context.hurt_audio_event = actor.hurt_audio_event;
            context.down_audio_event = actor.down_audio_event;
            context.target_heading_audio_alternate =
                (field_random_seed_ & 0x8000U) != 0U;
            context.initial_action_delay = actor.action_delay;
            context.delay_primary_attribute =
                actor.delay_primary_attribute;
            context.delay_secondary_attribute =
                actor.delay_secondary_attribute;
            context.delay_capability_flags =
                actor.delay_capability_flags;
            context.world_effect_slot_occupancy =
                field_world_effect_slots_occupied_;
            context.source_projection_active = caster_projection_active(
                actor.world_position.x, actor.world_position.y,
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_x),
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_y));
            context.ai_target_candidates.clear();
            const auto append_target_candidate =
                [this, &actor, &context](
                    std::string id,
                    const SceneRunnerPosition& position,
                    const std::uint8_t service_flags,
                    const std::uint8_t interaction_flags,
                    const std::uint8_t behavior_flags,
                    const std::uint8_t spawn_state_flags,
                    const bool world_activity_actor,
                    const std::uint8_t source_record_slot,
                    std::optional<scripting::ScriptCombatantFacts> combatant,
                    const std::optional<std::uint16_t>
                        source_record_heading) {
                    EntityProjectionState projection{};
                    projection.world_x = position.x;
                    projection.world_y = position.y;
                    project_entity_screen_position_standard(
                        projection,
                        static_cast<std::uint16_t>(
                            state_.field_scene_buffers.camera_x),
                        static_cast<std::uint16_t>(
                            state_.field_scene_buffers.camera_y));
                    const auto relative =
                        measure_entity_target_heading_and_distance(
                            actor.world_position.x, actor.world_position.y,
                            position.x, position.y);
                    const auto side_heading = static_cast<std::uint16_t>(
                        relative.heading + 0x4000U);
                    const auto side_magnitude = static_cast<std::uint16_t>(
                        relative.distance * 2U);
                    const auto side_vector = transform_entity_heading_vector(
                        side_heading,
                        static_cast<std::int16_t>(side_magnitude));
                    const auto side_x = static_cast<std::uint16_t>(
                        actor.world_position.x + side_vector.horizontal);
                    const auto side_y = static_cast<std::uint16_t>(
                        actor.world_position.y + side_vector.vertical);
                    const auto perpendicular_route =
                        measure_entity_target_heading_and_distance(
                            actor.world_position.x, actor.world_position.y,
                            side_x, side_y);
                    const auto direct_route_step_cost = static_cast<std::uint16_t>(
                        compute_record_diagonal_step_cost(
                            actor.world_position.x,
                            actor.world_position.y,
                            position.x, position.y));
                    const auto perpendicular_route_step_cost =
                        static_cast<std::uint16_t>(
                            compute_record_diagonal_step_cost(
                                actor.world_position.x,
                                actor.world_position.y,
                                side_x, side_y));
                    scripting::ScriptActorTargetCandidate candidate{
                            std::move(id),
                            position.x,
                            position.y,
                            relative.heading,
                            static_cast<std::uint16_t>(
                                std::min<std::uint32_t>(relative.distance,
                                                        0xFFFFU)),
                            service_flags,
                            interaction_flags,
                            behavior_flags,
                            spawn_state_flags,
                            projection.screen_y != 0U,
                            world_activity_actor,
                            direct_route_step_cost,
                            perpendicular_route_step_cost,
                            static_cast<std::uint16_t>(
                                std::min<std::uint32_t>(relative.distance,
                                                        0xFFFFU)),
                            static_cast<std::uint16_t>(
                                std::min<std::uint32_t>(
                                    perpendicular_route.distance,
                                    0xFFFFU))};
                    candidate.combatant = std::move(combatant);
                    candidate.source_record_slot = source_record_slot;
                    candidate.source_record_heading = source_record_heading;
                    context.ai_target_candidates.push_back(
                        std::move(candidate));
                };

            // ROM 0x14492 chooses the three runner records unless the acting
            // record has +0x10 bit 3 set. Runner coordinates come from the
            // scene/save's typed field positions; the active runner follows
            // the live field entity as it moves.
            for (std::size_t slot = 0U;
                 slot < FieldPartyRosterRuntime::kSlotCount; ++slot) {
                if (!field_party_roster_.slot_active(slot)) {
                    continue;
                }
                auto position = state_.field_scene_buffers.source_metadata
                                    .initial_field_records[slot];
                if (slot == field_party_roster_.active_slot() &&
                    field_entity_.active) {
                    position = field_entity_.world_position;
                }
                const auto flags = field_party_roster_.records()[slot].flags;
                const auto source_record_heading =
                    slot == field_party_roster_.active_slot() &&
                            field_entity_.active
                        ? std::optional<std::uint16_t>{
                              field_movement_state_.heading}
                        : std::nullopt;
                append_target_candidate(
                    "party.runner." + std::to_string(slot), position,
                    static_cast<std::uint8_t>(flags >> 8U),
                    static_cast<std::uint8_t>(flags & 0x00FFU), 0U, 0U,
                    false, static_cast<std::uint8_t>(slot),
                    script_combatant_facts(
                        field_party_roster_.records()[slot]),
                    source_record_heading);
            }

            // The other source target pool is the seven world-actor records
            // beginning at FF0400. These are live encounter/field actors, not
            // scenery rows; Lua applies the source record eligibility gates.
            for (const auto& target : field_default_actors_) {
                const auto source_slot = target.source_record_slot.has_value()
                    ? static_cast<std::uint8_t>(
                          *target.source_record_slot)
                    : 0U;
                if (!target.active || target.scene_reference != current_scene ||
                    !target.source_record_slot.has_value() ||
                    source_slot < 3U || source_slot > 9U) {
                    continue;
                }
                append_target_candidate(
                    target.actor_instance_id, target.world_position,
                    target.source_record_flags,
                    target.source_interaction_flags,
                    target.source_behavior_flags,
                    target.source_spawn_state_flags, true, source_slot,
                    target.combatant, target.movement_state.heading);
            }

            context.ai_random_bound = [this](const std::uint16_t bound)
                -> std::optional<std::uint16_t> {
                if (bound == 0U) {
                    return std::nullopt;
                }
                const auto choice =
                    sample_source_random_bound(field_random_seed_, bound);
                if (choice < 0 || choice >= bound) {
                    return std::nullopt;
                }
                return static_cast<std::uint16_t>(choice);
            };

            context.action_line_clear = [this, &actor](
                const std::uint16_t target_x,
                const std::uint16_t target_y) {
                // ROM 0x13FFE calls 0x14D8E before it enters the spell or
                // item branch. Keep the script's target policy in Lua and
                // provide the source terrain and linked-record queries here.
                const SpawnEntityParentState parent{
                    actor.world_position.x,
                    actor.world_position.y,
                    actor.movement_state.heading,
                    actor.source_record_flags,
                    actor.source_interaction_flags,
                    actor.source_behavior_flags,
                    actor.source_spawn_state_flags};
                return probe_action_line_clear(
                    parent,
                    actor.source_record_slot.has_value()
                        ? std::optional<std::size_t>{
                              static_cast<std::size_t>(
                                  *actor.source_record_slot)}
                        : std::nullopt,
                    target_x, target_y);
            };

            const bool has_candidate_in_source_pool = std::any_of(
                context.ai_target_candidates.begin(),
                context.ai_target_candidates.end(),
                [&actor](const auto& candidate) {
                    return candidate.world_activity_actor ==
                           actor_flag_is_set(
                               actor.source_record_flags,
                               ActorServiceFlag::targets_world_actor_group);
                });
            actor.ai_target_valid = has_candidate_in_source_pool;
            context.movement_allowed = has_candidate_in_source_pool;

            context.movement_services_available =
                context.movement_allowed;
            if (context.movement_services_available) {
                context.movement_state = scripting::ScriptMovementStepState{
                    actor.movement_state.position_x,
                    actor.movement_state.position_y,
                    actor.movement_state.heading,
                    actor.movement_state.animation_heading,
                    actor.movement_state.collision_context,
                    actor.movement_state.movement_substate,
                    actor.movement_state.route_status,
                    actor.source_behavior_flags,
                    actor.movement_state.movement_flags,
                    actor.movement_state.collision_flags,
                    actor.movement_state.event_flags,
                    actor.movement_state.collision_extent,
                    actor.movement_terrain_collision_code};
                for (std::size_t index = 0U;
                     index < field_movement_direction_offsets_.size();
                     ++index) {
                    context.movement_offsets[index] =
                        scripting::ScriptMovementOffset{
                            field_movement_direction_offsets_[index].x,
                            field_movement_direction_offsets_[index].y};
                }
                context.movement_idle_selectors =
                    kFieldIdleAnimationSelectors;
                context.movement_entity_flags = actor.source_record_flags;
                context.movement_collision_mode =
                    field_movement_transition_state_.world_mode;
                context.movement_collision =
                    [this, &actor](const std::uint16_t x,
                                   const std::uint16_t y) {
                        const auto collision_code = evaluate_field_collision(
                            x, y, actor.movement_collision_record,
                            actor.movement_sample_state,
                            actor.source_record_flags, false,
                            &actor.movement_terrain_collision_code);
                        return scripting::ScriptMovementCollisionResult{
                            collision_code,
                            actor.movement_terrain_collision_code};
                    };
                context.movement_route_lookup =
                    [this, &actor](const std::uint16_t route_key) {
                        if ((route_key & 0x03U) != 0U ||
                            route_key / 4U >= field_movement_routes_.size()) {
                            source_boundary(
                                "field.actor.movement.route_key_out_of_range");
                            throw std::runtime_error(
                                "Lua requested an actor route outside the source table");
                        }
                        const auto route_index =
                            static_cast<std::size_t>(route_key / 4U);
                        const auto& route_source =
                            field_movement_route_sources_[route_index];
                        const auto sample_value = [&actor](
                                                      const std::uint8_t index) {
                            if (index >= actor.movement_sample_state.size()) {
                                throw std::runtime_error(
                                    "actor route sample is outside its local collision state");
                            }
                            return static_cast<std::int8_t>(
                                actor.movement_sample_state[index]);
                        };
                        const auto& route =
                            field_movement_routes_[route_index];
                        return scripting::ScriptMovementRouteFacts{
                            sample_value(route_source.primary_sample_index),
                            sample_value(route_source.secondary_sample_index),
                            route.primary_fallback_slot,
                            route.secondary_fallback_slot};
                    };
                context.movement_collision_recovery =
                    [&actor](const std::uint16_t, const std::uint16_t,
                             const std::int16_t collision_code,
                             const std::uint8_t) {
                        // Dynamic creature map-event recovery is not the
                        // player's B7B8 doorway path. Treat a negative actor
                        // probe as blocked and let Lua try its route fallback.
                        return scripting::ScriptMovementCollisionRecoveryResult{
                            collision_code < 0,
                            actor.source_behavior_flags};
                    };
            }
        }

        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.actor.lua_behavior_failed");
            actor.active = false;
            continue;
        }
        if (dynamic_world_actor) {
            actor.source_item_spawn_result.reset();
        }
        actor.ai_package_id = decision->package_id;
        actor.ai_weapon_item_id = decision->weapon_item_id;
        actor.ai_target_id = decision->ai_target_id.value_or("");
        actor.ai_target_heading = decision->ai_target_heading.value_or(0U);
        actor.ai_target_distance = decision->ai_target_distance.value_or(0U);
        actor.action_delay = decision->action_delay;
        if (decision->spell_action_resolution.has_value()) {
            const auto& resolution = *decision->spell_action_resolution;
            actor.combatant.action_result = resolution.roll_attempted
                ? resolution.action_result
                : actor.combatant.action_result;
            actor.combatant.mental_meter = resolution.mental_meter;
            actor.combatant.item_codes = resolution.item_codes;
            actor.combatant.item_phases = resolution.item_phases;
            if (resolution.next_action_kind.has_value() &&
                resolution.next_action_code.has_value() &&
                resolution.next_profile_id.has_value()) {
                actor.action_kind = *resolution.next_action_kind;
                actor.initial_action_code = *resolution.next_action_code;
                actor.combatant.equipped_action_code =
                    *resolution.next_action_code;
                actor.script_profile_id = *resolution.next_profile_id;
            }
            if (resolution.effect_started &&
                resolution.spell_effect_slot.has_value()) {
                const auto slot = *resolution.spell_effect_slot;
                if (slot >= field_world_effect_slots_occupied_.size() ||
                    field_world_effect_slots_occupied_[slot] ||
                    resolution.target_id.empty() ||
                    !field_spell_effect_slot_target_ids_[slot].empty()) {
                    failure_ = "Lua selected an occupied spell effect record";
                    source_boundary(
                        "field.actor.spell.effect_slot_reservation_invalid");
                    actor.active = false;
                    continue;
                }
                field_world_effect_slots_occupied_[slot] = true;
                field_spell_effect_slot_target_ids_[slot] =
                    resolution.target_id;
                source_boundary("field.actor.spell.effect_slot_reserved");
            }
            source_boundary(resolution.effect_requested
                ? "field.actor.spell.effect_requested"
                : "field.actor.spell.action_resolved");
        }
        if (dynamic_world_actor) {
            actor.source_record_flags = decision->source_record_flags;
            actor.source_behavior_flags = decision->source_behavior_flags;
            actor.source_animation_selector =
                decision->source_animation_selector;
            actor.combatant.source_flags = actor.source_record_flags;
        }
        if (decision->target_motion_effect_slot_request.has_value()) {
            const auto slot = *decision->target_motion_effect_slot_request;
            if (slot >= field_world_effect_slots_occupied_.size() ||
                field_world_effect_slots_occupied_[slot]) {
                source_boundary(
                    "field.actor.hound_effect_behavior_slot_invalid");
            } else {
                field_world_effect_slots_occupied_[slot] = true;
                actor.target_motion_effect_slot = slot;
                source_boundary("field.actor.hound_effect_behavior_started");
            }
        }
        actor.target_motion_effect_visible =
            decision->target_motion_effect_visible;
        actor.target_motion_effect_direction =
            decision->target_motion_effect_direction;
        actor.target_motion_effect_phase =
            decision->target_motion_effect_phase;
        actor.animation_direction = decision->direction;
        actor.animation_id = decision->animation_id;
        actor.animation_frame = decision->animation_frame;
        const bool transition_just_completed =
            dynamic_world_actor &&
            !actor.transition_complete && decision->transition_complete &&
            actor_flag_is_set(actor.source_spawn_state_flags,
                              ActorSpawnStateFlag::world_activity_member) &&
            !actor_flag_is_set(
                actor.source_record_flags,
                ActorServiceFlag::targets_world_actor_group);
        actor.transition_started = decision->transition_started;
        actor.transition_complete = decision->transition_complete;
        if (transition_just_completed) {
            // ROM 0xAB02 decrements FFE177 only for +0x9E bit 6 members whose
            // +0x10 bit 3 is clear. The random-encounter timeout counter below
            // is a separate host path; Club list results keep their own count.
            if (actor.temporary_world_encounter &&
                field_world_activity_timeout_state_.scene_36_gate != 0U) {
                --field_world_activity_timeout_state_.scene_36_gate;
            }
            if (actor.temporary_world_encounter) {
                source_boundary(
                    "field.random_encounter.actor_transition_completed");
            }
            if (actor.temporary_world_encounter &&
                field_world_activity_timeout_state_.scene_36_gate == 0U) {
                reset_field_entity_service_records();
                source_boundary(
                    "field.random_encounter.sequence_transitions_complete");
            }
            if (actor.actor_list_completion_group.has_value() &&
                *actor.actor_list_completion_group ==
                    field_active_route_actor_list_ &&
                field_route_actor_transitions_remaining_.has_value() &&
                *field_route_actor_transitions_remaining_ != 0U) {
                actor.actor_list_completion_group.reset();
                --*field_route_actor_transitions_remaining_;
                source_boundary("field.world_activity." +
                                field_active_route_actor_list_ +
                                "_actor_transition_completed");
                if (*field_route_actor_transitions_remaining_ == 0U) {
                    field_route_actor_transitions_remaining_.reset();
                    reset_field_entity_service_records();
                    field_pending_actor_list_callback_ =
                        field_active_route_actor_list_;
                    source_boundary("field.world_activity." +
                                    field_active_route_actor_list_ +
                                    "_callback_dispatch_pending");
                }
            }
        }
        if (decision->combat_meter_update.has_value() &&
            decision->combat_meter_update->changed) {
            const auto& update = *decision->combat_meter_update;
            const auto candidate = std::find_if(
                context.ai_target_candidates.begin(),
                context.ai_target_candidates.end(),
                [&update](const auto& value) {
                    return value.id == update.target_id &&
                           value.source_record_slot.has_value();
                });
            if (candidate == context.ai_target_candidates.end()) {
                source_boundary("field.actor.combat.target_missing");
            } else if (update.record_type < 3U) {
                const auto slot = static_cast<std::size_t>(
                    *candidate->source_record_slot);
                if (slot >= FieldPartyRosterRuntime::kSlotCount ||
                    !field_party_roster_.slot_active(slot)) {
                    source_boundary("field.actor.combat.party_target_invalid");
                } else {
                    auto& target_record =
                        field_party_roster_.records()[slot];
                    target_record.delay_meter = update.physical_meter;
                    target_record.behavior_meter = update.mental_meter;
                    if (slot == field_party_roster_.active_slot()) {
                        refresh_field_hud_runner_slot();
                    }
                    source_boundary(update.physical
                        ? "field.actor.combat.party_physical_meter_changed"
                        : "field.actor.combat.party_mental_meter_changed");
                }
            } else {
                const auto target_actor = std::find_if(
                    field_default_actors_.begin(),
                    field_default_actors_.end(),
                    [&update](const auto& value) {
                        return value.active &&
                               value.actor_instance_id == update.target_id &&
                               value.source_record_slot.has_value() &&
                               static_cast<std::uint16_t>(
                                   *value.source_record_slot) ==
                                   update.record_type;
                    });
                if (target_actor == field_default_actors_.end()) {
                    source_boundary("field.actor.combat.world_target_invalid");
                } else {
                    target_actor->combatant.physical_meter =
                        update.physical_meter;
                    target_actor->combatant.mental_meter =
                        update.mental_meter;
                    target_actor->physical_meter = update.physical_meter;
                    source_boundary(update.physical
                        ? "field.actor.combat.world_physical_meter_changed"
                        : "field.actor.combat.world_mental_meter_changed");
                }
            }
        }
        if (dynamic_world_actor &&
            decision->movement_step.has_value()) {
            const auto& step = *decision->movement_step;
            actor.movement_state.position_x = step.state.position_x;
            actor.movement_state.position_y = step.state.position_y;
            actor.movement_state.heading = step.state.heading;
            actor.movement_state.animation_heading =
                step.state.animation_heading;
            actor.movement_state.collision_context =
                step.state.collision_context;
            actor.movement_state.movement_substate =
                step.state.movement_substate;
            actor.movement_state.route_status = step.state.route_status;
            actor.movement_state.movement_flags =
                step.state.movement_flags;
            actor.movement_state.collision_flags =
                step.state.collision_flags;
            actor.movement_state.event_flags = step.state.event_flags;
            actor.movement_state.collision_extent =
                step.state.collision_extent;
            actor.source_behavior_flags = step.state.route_flags;
            actor.movement_terrain_collision_code =
                step.state.terrain_collision_code;
            actor.world_position.x = step.state.position_x;
            actor.world_position.y = step.state.position_y;
            actor.movement_state_initialized = true;
        }
        if (decision->audio_event.has_value() &&
            source_audio_event_callback_) {
            source_audio_event_callback_(*decision->audio_event);
        }
        if (decision->target_heading_audio_event.has_value() &&
            source_audio_event_callback_) {
            source_audio_event_callback_(
                *decision->target_heading_audio_event);
        }
        if (dynamic_world_actor &&
            decision->source_action_dispatch.has_value() &&
            *decision->source_action_dispatch == "item_spawn_requested") {
            service_dynamic_actor_weapon_action(actor, context, *decision);
        }
        resolve_default_actor_proximity(actor);
    }
    if (field_pending_actor_list_callback_.has_value() &&
        !field_dialogue_.open()) {
        const auto name = std::move(*field_pending_actor_list_callback_);
        field_pending_actor_list_callback_.reset();
        // route_actor_lists.lua maps complete_<name> to the list's callback.
        if (field_dialogue_.open_gameplay_task("route_actor_lists",
                                               "complete_" + name)) {
            source_boundary("field.world_activity." + name +
                            "_callback_dispatched");
        } else {
            source_boundary("field.world_activity." + name +
                            "_callback_dialogue_open_failed");
        }
    }
}

void FieldSceneRuntime::resolve_default_actor_proximity(
    DefaultFieldActorPresentation& actor) {
    constexpr std::uint16_t kSceneTwenty = 0x0014U;
    constexpr std::uint8_t kIleneContactFlag = 0x08U;
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    // ROM 0xA8CC publishes this Scene-20 actor with +0x9E bit 3. Keep the
    // interaction eligible only while that ROM-derived record and FFE182's
    // pending-list handoff are both present.
    if (!actor.active || !actor.world_activity_script_actor ||
        actor.scene_reference != scene_reference ||
        scene_reference != kSceneTwenty ||
        (actor.source_spawn_state_flags & kIleneContactFlag) == 0U ||
        !actor.source_record_slot.has_value() || !field_entity_.active ||
        field_dialogue_.open()) {
        return;
    }

    const auto make_proximity_record = [](
        const DefaultFieldActorPresentation& source) {
        EntityProximityRecord record{};
        record.x = source.world_position.x;
        record.y = source.world_position.y;
        record.entity_flags = source.source_record_flags;
        record.collision_flags = source.source_interaction_flags;
        record.special_flags = source.source_spawn_state_flags;
        record.activity_flags = source.source_behavior_flags;
        return record;
    };
    std::vector<EntityProximityRecord> records;
    std::vector<DefaultFieldActorPresentation*> record_actors;
    records.push_back(make_proximity_record(actor));
    record_actors.push_back(&actor);

    // Preserve the FF0400..FF0900 record order before appending FF0A00. The
    // proximity routine advances the source PRNG after every resolved pair,
    // even when that pair is not the player contact that starts the activity.
    std::vector<DefaultFieldActorPresentation*> later_actors;
    const auto current_slot =
        static_cast<std::size_t>(*actor.source_record_slot);
    for (auto& candidate : field_default_actors_) {
        if (!candidate.active || candidate.scene_reference != scene_reference ||
            !candidate.source_record_slot.has_value()) {
            continue;
        }
        const auto candidate_slot =
            static_cast<std::size_t>(*candidate.source_record_slot);
        if (candidate_slot > current_slot && candidate_slot <= 9U) {
            later_actors.push_back(&candidate);
        }
    }
    std::sort(later_actors.begin(), later_actors.end(),
              [](const auto* left, const auto* right) {
                  return static_cast<std::size_t>(
                             *left->source_record_slot) <
                         static_cast<std::size_t>(
                             *right->source_record_slot);
              });
    for (auto* candidate : later_actors) {
        records.push_back(make_proximity_record(*candidate));
        record_actors.push_back(candidate);
    }

    // The runner is the later player record in the source service order.
    // +0x10 bit 0 marks the live record; its high byte carries the interaction
    // eligibility bits updated by the runner selection service.
    EntityProximityRecord player_record{};
    player_record.x = field_entity_.world_position.x;
    player_record.y = field_entity_.world_position.y;
    player_record.entity_flags = static_cast<std::uint8_t>(
        0x01U | field_entity_behavior_flags_);
    player_record.player_record = true;
    records.push_back(player_record);

    EntityPairCollisionWorldState world{};
    world.activity_in_progress = field_runtime_state_.field_activity_lock;
    world.world_effect_active = field_world_effect_active_;
    world.pending_world_effect = field_pending_world_effect_;
    world.scene_activity_id = scene_reference;
    world.field_mode = field_movement_transition_state_.mode_event_code;

    const auto proximity = resolve_entity_proximity_interactions(
        records, 0U,
        {[this, &records, &world](EntityProximityRecord& current,
                                 EntityProximityRecord& candidate) {
             EntityPairCollisionEntity current_entity{
                 current.special_flags, current.entity_flags,
                 current.activity_flags};
             EntityPairCollisionEntity candidate_entity{
                 candidate.special_flags, candidate.entity_flags,
                 candidate.activity_flags};
             const auto collision = resolve_entity_pair_collision_event(
                 current_entity, candidate_entity, world,
                 {{}, [this] {
                      if (field_dialogue_.open()) {
                          return;
                      }
                      game_scripts_.set_location_context(
                          "field_map", "scene_20_ilene",
                          "field.world_event", "field_narrator");
                      begin_world_activity(field_world_activity_state_, 0U);
                      world_frame_logic_state_.resource_countdown =
                          field_world_activity_state_.timeout;
                      if (!field_dialogue_.open_gameplay_task(
                              "ilene", "complete_scene_20_activity")) {
                          failure_ = field_dialogue_.failure().empty()
                              ? "Ilene contact task could not be opened"
                              : field_dialogue_.failure();
                          source_boundary(
                              "field.world_activity.ilene_contact_open_failed");
                          return;
                      }
                      source_boundary(
                          "field.world_activity.ilene_contact_opened");
                  }});
             current.activity_flags = current_entity.activity_flags;
             candidate.activity_flags = candidate_entity.activity_flags;
             if (collision.normal_activity_requested &&
                 collision.handler ==
                     EntityPairActivityHandler::pending_world_effect_collision) {
                 source_boundary("field.entities.ilene_contact_resolved");
             }
         },
         [this] { return advance_source_random_seed(field_random_seed_); }});
    for (std::size_t index = 0U; index < record_actors.size(); ++index) {
        record_actors[index]->source_interaction_flags =
            records[index].collision_flags;
        record_actors[index]->source_behavior_flags =
            records[index].activity_flags;
    }
    field_pending_world_effect_ = world.pending_world_effect;
    field_world_effect_active_ = world.world_effect_active;
    if (proximity.pairs_resolved != 0U) {
        source_boundary("field.entities.resolve_proximity");
    }
}

void FieldSceneRuntime::service_dynamic_actor_weapon_action(
    DefaultFieldActorPresentation& actor,
    const scripting::ScriptActorBehaviorContext& context,
    const scripting::ScriptActorBehaviorDecision& decision) {
    const auto block_action = [this, &actor](const std::string_view reason) {
        actor.source_item_spawn_result =
            scripting::ScriptActorItemSpawnResult::blocked;
        source_boundary("field.actor.weapon_action." + std::string(reason));
    };
    if (!actor.active ||
        (!actor.temporary_world_encounter &&
         !actor.world_activity_script_actor) ||
        actor.action_kind != scripting::ScriptActorActionKind::item ||
        actor.initial_action_code == 0U ||
        actor.initial_action_code > 12U || actor.source_action_ammo == 0U ||
        (context.world_scene_state == 0U &&
         !actor_flag_is_set(actor.source_behavior_flags,
                            ActorBehaviorFlag::action_window_open))) {
        block_action("source_gate_rejected");
        return;
    }

    const auto target = std::find_if(
        context.ai_target_candidates.begin(),
        context.ai_target_candidates.end(),
        [&decision](const auto& candidate) {
            return decision.ai_target_id.has_value() &&
                   candidate.id == *decision.ai_target_id &&
                   candidate.source_record_slot.has_value();
        });
    if (target == context.ai_target_candidates.end()) {
        block_action("linked_target_missing");
        return;
    }

    EntitySpawnCycleState spawn_state{};
    spawn_state.parent.position_x = actor.world_position.x;
    spawn_state.parent.position_y = actor.world_position.y;
    spawn_state.parent.heading = actor.movement_state.heading;
    spawn_state.parent.flags = actor.source_record_flags;
    spawn_state.parent.field_0x56 = actor.source_action_slot;
    spawn_state.parent.field_0x57 = actor.initial_action_code;
    spawn_state.parent.field_0x9d =
        actor.combatant.auxiliary_roll_bonus;
    spawn_state.equipment = EntityEquipmentState{
        actor.source_action_ammo,
        static_cast<std::int16_t>(actor.combatant.record_type)};
    spawn_state.linked_target = SpawnEntityLinkedState{
        target->world_x, target->world_y, 0x01U, false};
    // +0x16 is the selected source list. C052/C062 are within the ordinary
    // spawn window; C042, C072, and C082 are greater than 0x3C and are gated.
    spawn_state.animation_counter = actor.source_animation_selector;
    spawn_state.global_mode = context.world_scene_state;
    spawn_state.entity_flags = actor.source_record_flags;
    if (actor.source_action_slot < actor.combatant.item_phases.size()) {
        spawn_state.source_status_flags =
            actor.combatant.item_phases[actor.source_action_slot];
    }

    std::vector<std::uint16_t> allocated_slots;
    const auto spawn_cycle = process_entity_spawn_cycle(
        spawn_state,
        [] { return true; },
        [this, &allocated_slots]() -> std::optional<std::uint16_t> {
            for (std::size_t index = 0U;
                 index < field_weapon_slots_occupied_.size(); ++index) {
                if (!field_weapon_slots_occupied_[index]) {
                    field_weapon_slots_occupied_[index] = true;
                    const auto slot = static_cast<std::uint16_t>(9U + index);
                    allocated_slots.push_back(slot);
                    return slot;
                }
            }
            return std::nullopt;
        },
        [this, &actor] {
            source_boundary("field.actor.weapon_attack_seed_scheduled:" +
                            actor.actor_instance_id);
        },
        [this] {
            (void)schedule_world_activity_timeout(
                field_world_activity_timeout_state_);
        },
        [this, &actor](const std::uint16_t event) {
            if (source_audio_event_callback_) {
                source_audio_event_callback_(event);
            }
            source_boundary("field.actor.weapon_audio:" +
                            actor.actor_instance_id + ":" +
                            std::to_string(event));
        },
        [this, &actor](const std::uint16_t) {
            source_boundary("field.actor.weapon_mode_effect_pending:" +
                            actor.actor_instance_id);
        },
        [](EntityEquipmentState& equipment,
           const std::uint8_t source_action_code) {
            return service_entity_ammo(
                equipment, source_action_code,
                EntityWeaponRoundsPerActionLookup{},
                std::function<void(std::int8_t)>{});
        });

    actor.source_action_ammo = spawn_state.equipment.current_ammo;
    for (const auto slot : allocated_slots) {
        const bool has_primary = spawn_cycle.primary.has_value() &&
            spawn_cycle.primary_slot.has_value() &&
            *spawn_cycle.primary_slot == slot;
        const bool has_secondary = spawn_cycle.secondary.has_value() &&
            spawn_cycle.secondary_slot.has_value() &&
            *spawn_cycle.secondary_slot == slot;
        if (!has_primary && !has_secondary && slot >= 9U && slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(slot - 9U)] = false;
        }
    }

    if (spawn_cycle.primary.has_value() ||
        spawn_cycle.secondary.has_value()) {
        FieldWeaponProjectilePresentation spawn{};
        spawn.shooter_facts = actor.combatant;
        spawn.shooter_actor_id = actor.actor_instance_id;
        if (spawn_cycle.primary.has_value()) {
            spawn.primary_record = *spawn_cycle.primary;
            spawn.primary_spawn_position = SceneRunnerPosition{
                spawn.primary_record.position_x,
                spawn.primary_record.position_y};
            spawn.primary_active =
                (spawn.primary_record.flags & 0x01U) != 0U;
            spawn.primary_slot = spawn_cycle.primary_slot;
            spawn.primary_fixed_position = EntityFixedPointPosition{
                static_cast<std::int32_t>(
                    static_cast<std::int16_t>(
                        spawn.primary_record.position_x)) * 0x10000,
                static_cast<std::int32_t>(
                    static_cast<std::int16_t>(
                        spawn.primary_record.position_y)) * 0x10000,
                spawn.primary_record.heading_offset_x,
                spawn.primary_record.heading_offset_y};
            spawn.primary_animation.period = static_cast<std::int16_t>(
                spawn.primary_record.behavior_defaults.behavior_timer);
        }
        if (spawn_cycle.secondary.has_value()) {
            spawn.source_record = *spawn_cycle.secondary;
            spawn.secondary_active = true;
            spawn.secondary_slot = spawn_cycle.secondary_slot;
            spawn.animation.period = static_cast<std::int16_t>(
                spawn.source_record.behavior_defaults.behavior_timer);
        }

        const auto selector = static_cast<std::uint16_t>(
            spawn.source_record.animation_resource & 0x7FFFU);
        const auto animation_index =
            static_cast<std::size_t>(selector / 4U);
        if (spawn.secondary_active &&
            (animation_index >= field_weapon_projectile_images_.size() ||
             field_weapon_projectile_images_[animation_index].empty())) {
            source_boundary("field.actor.weapon_projectile_asset_missing:" +
                            actor.actor_instance_id);
            spawn.secondary_active = false;
            if (spawn.secondary_slot.has_value() &&
                *spawn.secondary_slot >= 9U &&
                *spawn.secondary_slot <= 14U) {
                field_weapon_slots_occupied_[
                    static_cast<std::size_t>(*spawn.secondary_slot - 9U)] =
                    false;
            }
        }
        field_weapon_projectiles_.push_back(std::move(spawn));
    }

    const bool projectile_created = spawn_cycle.primary.has_value() &&
        (spawn_cycle.primary->flags & 0x01U) != 0U;
    actor.source_item_spawn_result = projectile_created
        ? scripting::ScriptActorItemSpawnResult::spawned
        : scripting::ScriptActorItemSpawnResult::blocked;
    source_boundary(projectile_created
        ? "field.actor.weapon_projectile_spawned:" + actor.actor_instance_id
        : "field.actor.weapon_projectile_blocked:" + actor.actor_instance_id);
    source_boundary("field.actor.weapon_ammo_remaining:" +
                    actor.actor_instance_id + ":" +
                    std::to_string(actor.source_action_ammo));
}

FieldSceneRuntime::DefaultActorSpriteView
FieldSceneRuntime::default_actor_sprite_view(
    const DefaultFieldActorPresentation& actor) const {
    if (!actor.active) {
        return {};
    }
    const auto direction = static_cast<std::size_t>(
        actor.animation_direction & 0x07U);
    const DefaultActorArtwork* artwork = nullptr;
    if (!actor.temporary_world_encounter &&
        !actor.world_activity_script_actor &&
        (actor.entity_kind == 5U || actor.entity_kind == 11U)) {
        const auto kind_index = actor.entity_kind == 5U ? 0U : 1U;
        artwork = &field_default_actor_artwork_[kind_index];
    } else {
        const auto found = field_dynamic_actor_artwork_.find(
            actor.sprite_stem);
        if (found == field_dynamic_actor_artwork_.end()) {
            return {};
        }
        artwork = &found->second;
    }
    if (actor.animation_id == "idle") {
        return DefaultActorSpriteView{
            &artwork->idle_images[direction],
            artwork->idle_origin_x[direction],
            artwork->idle_origin_y[direction]};
    }
    if (actor.animation_id == "walk") {
        const auto& frames = artwork->walk_images[direction];
        if (frames.empty()) {
            return {};
        }
        const auto frame = static_cast<std::size_t>(actor.animation_frame) %
                           frames.size();
        return DefaultActorSpriteView{
            &frames[frame],
            artwork->walk_origin_x[direction][frame],
            artwork->walk_origin_y[direction][frame]};
    }
    if (actor.animation_id == "spell") {
        // 0xAF7C/0xAFAE install a directional C082 selector and 0xAFE0 holds
        // that first source frame until another actor state replaces it.
        return DefaultActorSpriteView{
            &artwork->spell_images[direction],
            artwork->spell_origin_x[direction],
            artwork->spell_origin_y[direction]};
    }
    if (actor.animation_id == "weapon") {
        const auto& frames = artwork->item_action_images[direction];
        if (frames.empty()) {
            return {};
        }
        const auto frame = std::min<std::size_t>(
            actor.animation_frame, frames.size() - 1U);
        return DefaultActorSpriteView{
            &frames[frame],
            artwork->item_action_origin_x[direction][frame],
            artwork->item_action_origin_y[direction][frame]};
    }
    if (actor.animation_id == "action") {
        const auto& frames = artwork->action_images[direction];
        if (frames.empty()) {
            return {};
        }
        const auto frame = std::min<std::size_t>(
            actor.animation_frame, frames.size() - 1U);
        return DefaultActorSpriteView{
            &frames[frame],
            artwork->action_origin_x[direction][frame],
            artwork->action_origin_y[direction][frame]};
    }
    if (actor.animation_id == "hurt") {
        const auto& frames = artwork->hurt_images[direction];
        if (frames.empty()) {
            return {};
        }
        const auto frame = std::min<std::size_t>(
            actor.animation_frame, frames.size() - 1U);
        return DefaultActorSpriteView{
            &frames[frame],
            artwork->hurt_origin_x[direction][frame],
            artwork->hurt_origin_y[direction][frame]};
    }
    if (actor.animation_id != "entry" && actor.animation_id != "down") {
        return {};
    }
    if (artwork->entry_frame_count == 0U) {
        return {};
    }
    const auto frame = std::min<std::size_t>(
        actor.animation_frame,
        static_cast<std::size_t>(artwork->entry_frame_count - 1U));
    return DefaultActorSpriteView{
        &artwork->entry_images[direction][frame],
        artwork->entry_origin_x[direction][frame],
        artwork->entry_origin_y[direction][frame]};
}

void FieldSceneRuntime::refresh_world_entity_service_targets() {
    std::vector<EntityServiceTargetRecord> records(16U);
    const auto camera_x = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_x);
    const auto camera_y = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_y);
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    for (const auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene ||
            !actor.source_record_slot.has_value()) {
            continue;
        }
        const auto slot = static_cast<std::size_t>(
            *actor.source_record_slot);
        if (slot >= records.size()) {
            continue;
        }
        auto& record = records[slot];
        record.active = actor_flag_is_set(
            actor.source_record_flags, ActorServiceFlag::active);
        record.effect_locked = actor_flag_is_set(
            actor.source_record_flags, ActorServiceFlag::effect_locked);
        record.service_reserved = actor_flag_is_set(
            actor.source_record_flags,
            ActorServiceFlag::action_candidate_reserved);
        record.selected = actor_flag_is_set(
            actor.source_interaction_flags,
            ActorInteractionFlag::service_selection_latch);
        record.high_effect = actor_flag_is_set(
            actor.source_behavior_flags,
            ActorBehaviorFlag::high_effect_route);
        record.descriptor_class = static_cast<std::uint8_t>(slot);
        record.owner_linked = actor.target_selected;
        record.selection_code = actor.target_selected ? 0x8009U : 0U;
        record.effect_pending = actor_flag_is_set(
            actor.source_record_flags,
            ActorServiceFlag::targets_world_actor_group);

        EntityProjectionState projection{};
        projection.world_x = actor.world_position.x;
        projection.world_y = actor.world_position.y;
        project_entity_screen_position_standard(projection, camera_x,
                                                camera_y);
        // 0x1DC4C requires the actor record's projected +0x0C field to be
        // nonzero; the source scan has no separate viewport-visibility gate.
        record.has_action = projection.screen_y != 0U;
    }

    field_entity_target_records_ = std::move(records);
    field_entity_target_owner_.special_marker =
        field_movement_behavior_record_.target_marker < 0;
    field_entity_target_owner_.mode =
        field_movement_behavior_record_.source_type;
    field_entity_target_owner_.service_marker =
        field_movement_behavior_record_.auxiliary_marker;
    field_entity_target_owner_.owner_record = 0U;

    const auto selected = field_entity_target_owner_.selected_target;
    world_frame_logic_state_.current_entity_special =
        field_entity_target_owner_.special_marker;
    world_frame_logic_state_.current_entity_mode =
        field_entity_target_owner_.mode;
    world_frame_logic_state_.current_target_present =
        selected.has_value() && *selected < field_entity_target_records_.size();
    world_frame_logic_state_.current_target_active =
        world_frame_logic_state_.current_target_present &&
        field_entity_target_records_[*selected].active;
    world_frame_logic_state_.current_target_effect_locked =
        world_frame_logic_state_.current_target_present &&
        field_entity_target_records_[*selected].effect_locked;
}

bool FieldSceneRuntime::apply_scripted_field_entity_target_selection(
    const std::optional<std::string>& selected_actor_id) {
    const bool special_path = field_entity_target_owner_.special_marker &&
        (field_entity_target_owner_.mode == 0x0CU ||
         field_entity_target_owner_.mode == 0x04U ||
         field_entity_target_owner_.mode == 0x0BU);
    if (!special_path) {
        field_entity_target_owner_.service_marker = 0U;
    }
    field_entity_target_owner_.selected_target.reset();
    for (auto& actor : field_default_actors_) {
        if (!actor.source_record_slot.has_value()) {
            continue;
        }
        actor.target_selected = false;
        actor.source_interaction_flags = static_cast<std::uint8_t>(
            actor.source_interaction_flags & static_cast<std::uint8_t>(
                ~actor_flag_mask(
                    ActorInteractionFlag::service_selection_latch)));
    }

    bool selected = false;
    if (selected_actor_id.has_value()) {
        const auto actor = std::find_if(
            field_default_actors_.begin(), field_default_actors_.end(),
            [this, &selected_actor_id](const auto& candidate) {
                return candidate.active &&
                    candidate.scene_reference ==
                        state_.field_scene_buffers.scene_reference &&
                    candidate.source_record_slot.has_value() &&
                    candidate.actor_instance_id == *selected_actor_id;
            });
        if (actor == field_default_actors_.end()) {
            failure_ = "Lua selected a world actor outside the active field";
            source_boundary("field.target.lua_selected_actor_missing");
            return false;
        }
        const auto slot = static_cast<std::size_t>(
            *actor->source_record_slot);
        if (slot >= field_entity_target_records_.size()) {
            failure_ = "Lua selected a world actor with an invalid source slot";
            source_boundary("field.target.lua_selected_slot_out_of_range");
            return false;
        }
        actor->target_selected = true;
        actor->source_interaction_flags = static_cast<std::uint8_t>(
            actor->source_interaction_flags |
            actor_flag_mask(ActorInteractionFlag::service_selection_latch));
        field_entity_target_owner_.selected_target = slot;
        selected = true;
    }
    refresh_world_entity_service_targets();
    source_boundary(selected ? "field.target.selected_with_b"
                             : "field.target.no_available_record_with_b");
    return true;
}

std::int16_t FieldSceneRuntime::select_field_world_effect_sequence() {
    const auto first = sample_source_random_bound(field_random_seed_, 5U);
    const auto second = sample_source_random_bound(field_random_seed_, 5U);
    if (first < 0 || first > 4 || second < 0 || second > 4) {
        pending_field_encounter_outcome_.reset();
        failure_ = "ROM field encounter selector produced an invalid bounded draw";
        source_boundary("field.random_encounter.invalid_selector_draw");
        return 1;
    }
    auto outcome = game_scripts_.resolve_field_encounter(
        state_.field_scene_buffers.scene_reference,
        static_cast<std::uint8_t>(first),
        static_cast<std::uint8_t>(second));
    if (!outcome.has_value()) {
        pending_field_encounter_outcome_.reset();
        failure_ = game_scripts_.failure();
        source_boundary("field.random_encounter.selector_row_unresolved");
        return 1;
    }
    pending_field_encounter_outcome_ = std::move(*outcome);
    return pending_field_encounter_outcome_->kind ==
                   scripting::ScriptFieldEncounterOutcomeKind::dialogue_event
               ? 1
               : 0;
}

void FieldSceneRuntime::spawn_field_world_effect_sequence() {
    if (!pending_field_encounter_outcome_.has_value() ||
        pending_field_encounter_outcome_->kind !=
            scripting::ScriptFieldEncounterOutcomeKind::
                dynamic_world_effect_sequence) {
        source_boundary("field.random_encounter.dynamic_sequence_missing");
        pending_field_encounter_outcome_.reset();
        return;
    }

    const auto& outcome = *pending_field_encounter_outcome_;
    const auto& profile = state_.field_scene_buffers.profile;
    const bool source_positions_complete = std::all_of(
        outcome.actors.begin(), outcome.actors.end(),
        [](const auto& actor) {
            return actor.run_target_spawn_x.has_value() &&
                   actor.run_target_spawn_y.has_value();
        });
    const bool has_any_source_positions = std::any_of(
        outcome.actors.begin(), outcome.actors.end(),
        [](const auto& actor) {
            return actor.run_target_spawn_x.has_value() ||
                   actor.run_target_spawn_y.has_value();
        });
    if (has_any_source_positions && !source_positions_complete) {
        failure_ = "ROM actor list has incomplete source placement records";
        source_boundary("field.random_encounter.actor_position_unresolved");
        pending_field_encounter_outcome_.reset();
        return;
    }
    if (source_positions_complete !=
        (outcome.placement_policy == scripting::
             ScriptFieldEncounterPlacementPolicy::source_actor_positions)) {
        failure_ = "encounter placement policy does not match its actor positions";
        source_boundary("field.random_encounter.placement_policy_unresolved");
        pending_field_encounter_outcome_.reset();
        return;
    }

    std::vector<std::pair<std::uint16_t, std::uint16_t>> placement_candidates;
    if (!source_positions_complete) {
        const auto lookup_terrain_tile =
            [this](const std::uint16_t x, const std::uint16_t y)
                -> std::optional<std::uint8_t> {
            // The source candidate lookup addresses the compact collision
            // map using world coordinates. The shared host sampler accepts
            // pixel coordinates, so normalize the source words here.
            const auto sample = sample_terrain_collision_tiles(
                static_cast<std::uint16_t>(x >> 4U),
                static_cast<std::uint16_t>(y >> 4U), false,
                [this](const std::uint16_t tile_index) {
                    const auto& map = state_.field_scene_buffers
                                          .source_metadata
                                          .terrain_collision_map;
                    return tile_index < map.size()
                        ? map[tile_index]
                        : static_cast<std::uint8_t>(0x7FU);
                });
            return sample.selected_value;
        };
        if (outcome.placement_policy == scripting::
                ScriptFieldEncounterPlacementPolicy::screen_edge_candidates) {
            WorldEffectEdgeCandidateState edge_state{};
            // ROM 0x1E302 reads FFDDC6/FFDDC8 as the live camera origin and
            // FFDDCE/FFDDD0 as the far map bounds. Profile words +8/+10 hold
            // the bounds; the origin follows the moving camera state.
            edge_state.map_width = profile[4];
            edge_state.origin_x = static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_x);
            edge_state.origin_y = static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_y);
            edge_state.map_height = profile[5];
            build_world_effect_edge_candidates(
                edge_state, {lookup_terrain_tile});
            placement_candidates = std::move(edge_state.candidates);
        } else if (outcome.placement_policy == scripting::
                       ScriptFieldEncounterPlacementPolicy::
                           map_derived_candidates) {
            WorldEffectMapCandidateState map_state{};
            map_state.map_width = profile[4];
            map_state.camera_x = static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_x);
            map_state.camera_y = static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_y);
            map_state.map_height = profile[5];
            build_world_effect_map_candidates(
                map_state, {lookup_terrain_tile});
            placement_candidates = std::move(map_state.candidates);
        } else {
            source_boundary(
                "field.random_encounter.placement_policy_unresolved");
            pending_field_encounter_outcome_.reset();
            return;
        }
    }

    if (!source_positions_complete && placement_candidates.empty()) {
        source_boundary(
            outcome.placement_policy == scripting::
                    ScriptFieldEncounterPlacementPolicy::map_derived_candidates
                ? "field.random_encounter.no_open_map_candidates"
                : "field.random_encounter.no_open_edge_candidates");
        pending_field_encounter_outcome_.reset();
        return;
    }

    // Validate the complete ordered sequence before publishing any record.
    // The selector outcome is data, but every actor must still have an
    // extracted native sprite and a Lua actor profile before the source list
    // can start. No scenery stand-ins are created for missing rows.
    for (const auto& source_actor : outcome.actors) {
        if (source_actor.entity_kind < 4U || source_actor.entity_kind > 14U ||
            field_dynamic_actor_artwork_.find(source_actor.sprite_stem) ==
                field_dynamic_actor_artwork_.end()) {
            failure_ = "ROM encounter actor has no extracted dynamic artwork";
            source_boundary("field.random_encounter.actor_art_unresolved");
            pending_field_encounter_outcome_.reset();
            return;
        }
    }

    const auto actor_count = source_positions_complete
        ? outcome.actors.size()
        : std::min(outcome.actors.size(), placement_candidates.size());
    std::size_t spawned = 0U;
    for (std::size_t index = 0U; index < actor_count; ++index) {
        std::array<std::uint8_t, 7> occupied_actor_slots{};
        for (const auto& actor : field_default_actors_) {
            if (!actor.active || actor.scene_reference != outcome.scene_id ||
                !actor.source_record_slot.has_value()) {
                continue;
            }
            const auto slot = static_cast<std::size_t>(
                *actor.source_record_slot);
            if (slot >= 3U && slot <= 9U) {
                occupied_actor_slots[slot - 3U] = 1U;
            }
        }
        std::uint16_t actor_slot = 10U;
        for (std::size_t candidate = 0U;
             candidate < occupied_actor_slots.size(); ++candidate) {
            if (occupied_actor_slots[candidate] == 0U) {
                actor_slot = static_cast<std::uint16_t>(candidate + 3U);
                break;
            }
        }
        if (actor_slot < 3U || actor_slot > 9U) {
            source_boundary("field.random_encounter.actor_slots_exhausted");
            break;
        }

        const auto effect_slot = std::find(
            field_world_effect_slots_occupied_.begin(),
            field_world_effect_slots_occupied_.end(), false);
        if (effect_slot == field_world_effect_slots_occupied_.end()) {
            source_boundary("field.random_encounter.effect_slots_exhausted");
            break;
        }

        const auto& source_actor = outcome.actors[index];
        DefaultFieldActorPresentation actor{};
        actor.active = true;
        actor.scene_reference = outcome.scene_id;
        actor.source_order = source_actor.source_order;
        actor.source_record_slot = static_cast<FieldWorldActorSlot>(actor_slot);
        actor.entity_kind = source_actor.entity_kind;
        actor.actor_instance_id =
            "field_scene_" + std::to_string(outcome.scene_id) +
            "_encounter_" + std::to_string(field_world_encounter_sequence_) +
            "_actor_" + std::to_string(source_actor.source_order);
        actor.script_profile_id = source_actor.profile_id;
        actor.action_kind = source_actor.action_kind;
        actor.initial_action_code = source_actor.action_code;
        // Run-target descriptors at ROM 0x16A6A are paired with the actor
        // template by 0x16A26. The actor service at 0x142DC reads their first
        // two words directly as world X/Y; random encounters use edge picks.
        actor.world_position = source_positions_complete
            ? SceneRunnerPosition{
                  *source_actor.run_target_spawn_x,
                  *source_actor.run_target_spawn_y}
            : SceneRunnerPosition{
                  placement_candidates[index].first,
                  placement_candidates[index].second};
        // The common record initializer at 0x7361A supplies +0x2E=0. The
        // ordered template record contributes the actor kind/action and art.
        actor.initial_heading = 0U;
        actor.palette_line = source_actor.palette_line;
        actor.physical_meter = source_actor.physical_meter;
        actor.delay_primary_attribute =
            source_actor.delay_primary_attribute;
        actor.delay_secondary_attribute =
            source_actor.delay_secondary_attribute;
        actor.delay_capability_flags =
            source_actor.delay_capability_flags;
        actor.sprite_stem = source_actor.sprite_stem;
        actor.source_interaction_flags =
            source_actor.source_interaction_flags;
        const auto& actor_artwork =
            field_dynamic_actor_artwork_.at(source_actor.sprite_stem);
        actor.entry_animation_frame_count = actor_artwork.entry_frame_count;
        actor.walk_animation_frame_count = actor_artwork.walk_frame_count;
        actor.action_animation_frame_count =
            actor_artwork.action_frame_count;
        actor.action_animation_increment =
            actor_artwork.action_frame_increment;
        actor.action_animation_threshold =
            actor_artwork.action_frame_threshold;
        actor.action_animation_audio_event =
            actor_artwork.action_audio_event;
        actor.item_action_frame_count =
            actor_artwork.item_action_frame_count;
        actor.item_action_increment =
            actor_artwork.item_action_increment;
        actor.item_action_threshold =
            actor_artwork.item_action_threshold;
        actor.hurt_animation_frame_count =
            actor_artwork.hurt_frame_count;
        actor.hurt_animation_increment =
            actor_artwork.hurt_frame_increment;
        actor.hurt_animation_threshold =
            actor_artwork.hurt_frame_threshold;
        actor.hurt_audio_event = actor_artwork.hurt_audio_event;
        actor.down_audio_event = actor_artwork.down_audio_event;
        actor.template_offset = source_actor.template_offset;
        actor.list_offset = source_actor.sequence_list_offset;
        actor.selection_cell_offset = source_actor.selection_cell_offset;
        actor.source_record_flags = static_cast<std::uint8_t>(
            actor_flag_mask(ActorServiceFlag::active) |
            actor_flag_mask(ActorServiceFlag::unresolved_service_bit_2) |
            actor_flag_mask(ActorServiceFlag::activity_probe_enabled));
        actor.source_behavior_flags = 0U;
        actor.source_action_slot = source_actor.source_action_slot;
        actor.source_action_ammo = source_actor.source_action_ammo;
        actor.source_action_counter = source_actor.source_action_counter;
        actor.source_spawn_state_flags = actor_flag_mask(
            ActorSpawnStateFlag::world_activity_member);
        actor.combatant = source_actor.combatant;
        actor.combatant.record_type = actor_slot;
        actor.combatant.source_flags = actor.source_record_flags;
        actor.combatant.interaction_flags =
            actor.source_interaction_flags;
        actor.effect_slot = static_cast<std::uint8_t>(
            std::distance(field_world_effect_slots_occupied_.begin(),
                          effect_slot));
        actor.temporary_world_encounter = true;
        actor.ordered_overrides = source_actor.ordered_overrides;
        actor.initial_behavior_delay = source_positions_complete
            ? 0U
            : (outcome.scene_id == 0x37U
                   ? 0x28U
                   : static_cast<std::uint8_t>(
                         advance_source_random_seed(field_random_seed_) &
                         0x0FU));
        actor.animation_direction = 0U;
        actor.source_animation_selector = static_cast<std::uint16_t>(
            field_movement_direction_from_heading(actor.initial_heading) *
            4U);
        actor.movement_state.position_x = actor.world_position.x;
        actor.movement_state.position_y = actor.world_position.y;
        actor.movement_state.heading = actor.initial_heading;
        actor.movement_state.animation_heading = actor.initial_heading;
        // The common actor record at 0x7361A starts +0x08 at zero. Creature
        // continuations override it only for kind 0xD's frame-2 movement.
        actor.movement_state.collision_extent = 0;
        actor.movement_state_initialized = true;

        scripting::ScriptActorBehaviorContext context{
            actor.script_profile_id,
            actor.entity_kind,
            actor.action_kind,
            actor.initial_action_code,
            actor.initial_behavior_delay,
            false,
            false,
            actor.actor_instance_id,
            actor.initial_heading,
            true,
        };
        context.dynamic_world_encounter_actor = true;
        context.combatant = actor.combatant;
        context.movement_scene_id = outcome.scene_id;
        if (actor.source_record_slot.has_value()) {
            context.source_record_slot = static_cast<std::uint8_t>(
                *actor.source_record_slot);
            context.source_record_flags = actor.source_record_flags;
            context.source_physical_meter = actor.physical_meter;
        }
        context.action_profile_modifiers =
            script_action_profile_modifiers(state_.action_selection_profiles);
        context.action_attribute_selectors =
            script_action_attribute_selectors();
        context.combat_meter_service_blocked =
            field_movement_transition_state_.world_mode == 3U;
        context.source_record_flags = actor.source_record_flags;
        context.source_interaction_flags = actor.source_interaction_flags;
        context.source_behavior_flags = actor.source_behavior_flags;
        context.source_action_slot = actor.source_action_slot;
        context.source_action_ammo = actor.source_action_ammo;
        context.source_action_counter = actor.source_action_counter;
        context.source_animation_selector =
            actor.source_animation_selector;
        context.item_action_frame_count = actor.item_action_frame_count;
        context.item_action_increment = actor.item_action_increment;
        context.item_action_threshold = actor.item_action_threshold;
        context.source_spawn_state_flags = actor.source_spawn_state_flags;
        context.world_activity_active =
            world_frame_logic_state_.world_active != 0U;
        context.world_scene_state = world_frame_logic_state_.scene_state;
        context.entry_animation_frame_count =
            actor.entry_animation_frame_count;
        context.ai_animation_frame = actor.animation_frame;
        context.walk_animation_frame_count =
            actor.walk_animation_frame_count;
        context.action_animation_frame_count =
            actor.action_animation_frame_count;
        context.action_animation_increment =
            actor.action_animation_increment;
        context.action_animation_threshold =
            actor.action_animation_threshold;
        context.action_animation_audio_event =
            actor.action_animation_audio_event;
        context.initial_action_delay = actor.action_delay;
        context.delay_primary_attribute = actor.delay_primary_attribute;
        context.delay_secondary_attribute = actor.delay_secondary_attribute;
        context.delay_capability_flags = actor.delay_capability_flags;
        context.world_effect_slot_occupancy =
            field_world_effect_slots_occupied_;
        context.world_effect_slot_occupancy[actor.effect_slot.value()] = true;
        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.random_encounter.actor_lua_package_failed");
            break;
        }
        actor.ai_package_id = decision->package_id;
        actor.ai_weapon_item_id = decision->weapon_item_id;
        actor.ai_target_id = decision->ai_target_id.value_or("");
        actor.ai_target_heading = decision->ai_target_heading.value_or(0U);
        actor.ai_target_distance = decision->ai_target_distance.value_or(0U);
        actor.action_delay = decision->action_delay;
        actor.source_record_flags = decision->source_record_flags;
        actor.source_behavior_flags = decision->source_behavior_flags;
        actor.source_animation_selector =
            decision->source_animation_selector;
        actor.animation_direction = decision->direction;
        actor.animation_id = decision->animation_id;
        actor.animation_frame = decision->animation_frame;
        actor.transition_started = decision->transition_started;
        actor.transition_complete = decision->transition_complete;

        field_world_effect_slots_occupied_[actor.effect_slot.value()] = true;
        field_world_activity_timeout_state_.scene_36_gate =
            static_cast<std::uint8_t>(
                field_world_activity_timeout_state_.scene_36_gate + 1U);
        field_default_actors_.push_back(std::move(actor));
        (void)schedule_world_activity_timeout(
            field_world_activity_timeout_state_);
        ++spawned;
        source_boundary("field.random_encounter.actor_spawned_from_rom_list");
    }

    if (spawned != 0U) {
        // The source sets FFDF84/FFDF86 through 0x1DBAA before returning to
        // the outer 0x1DD94 service. Keep the shared typed copy in sync now.
        world_frame_logic_state_.world_active = static_cast<std::uint16_t>(
            field_world_activity_timeout_state_.timeout_marker);
        world_frame_logic_state_.cleanup_countdown = static_cast<std::int16_t>(
            field_world_activity_timeout_state_.timeout_frames);
        ++field_world_encounter_sequence_;
        refresh_world_entity_service_targets();
        state_.display_dirty_flags = static_cast<std::uint8_t>(
            state_.display_dirty_flags | 0x0EU);
        field_runtime_state_.display_dirty_flags = static_cast<std::uint8_t>(
            field_runtime_state_.display_dirty_flags | 0x0EU);
    }
    pending_field_encounter_outcome_.reset();
}

void FieldSceneRuntime::reset_field_entity_service_records() {
    // ROM 0x1DBF0 clears links/selection/effect locks and FFDF84. It does not
    // turn these actor records into scenery or remove their active bit.
    field_entity_target_owner_.selected_target.reset();
    field_entity_target_owner_.linked_target.reset();
    field_entity_target_owner_.service_marker = 0U;
    for (auto& actor : field_default_actors_) {
        actor.target_selected = false;
        actor.source_interaction_flags = static_cast<std::uint8_t>(
            actor.source_interaction_flags & static_cast<std::uint8_t>(
                ~actor_flag_mask(
                    ActorInteractionFlag::service_selection_latch)));
        actor.source_behavior_flags = static_cast<std::uint8_t>(
            actor.source_behavior_flags & static_cast<std::uint8_t>(
                ~actor_flag_mask(
                    ActorBehaviorFlag::resettable_effect_state)));
    }
    world_frame_logic_state_.world_active = 0U;
    world_frame_logic_state_.cleanup_countdown = 0;
    field_world_activity_timeout_state_.timeout_marker = 0;
    field_world_activity_timeout_state_.timeout_frames = 0U;
    refresh_world_entity_service_targets();
}

bool FieldSceneRuntime::process_pending_story_world_effect_sequence() {
    if (!field_pending_story_world_effect_.has_value()) {
        return false;
    }
    const auto effect = std::move(*field_pending_story_world_effect_);
    field_pending_story_world_effect_.reset();
    const auto scene_id = state_.field_scene_buffers.scene_reference;
    if (scene_id != effect.scene) {
        source_boundary(
            "field.world_activity.story_sequence_context_unresolved");
        return true;
    }
    const auto sequence_list_offset = effect.list;
    auto outcome = game_scripts_.resolve_field_actor_sequence(
        scene_id, sequence_list_offset);
    if (!outcome.has_value() ||
        outcome->kind != scripting::
            ScriptFieldEncounterOutcomeKind::dynamic_world_effect_sequence ||
        outcome->placement_policy != scripting::
            ScriptFieldEncounterPlacementPolicy::map_derived_candidates ||
        outcome->actors.size() != effect.actor_count ||
        std::any_of(outcome->actors.begin(), outcome->actors.end(),
                    [sequence_list_offset](const auto& actor) {
                        return actor.sequence_list_offset !=
                               sequence_list_offset;
                    })) {
        failure_ = game_scripts_.failure().empty()
            ? "Story world-effect list " + effect.name +
                  " did not resolve its " +
                  std::to_string(effect.actor_count) + " map-placed actors"
            : game_scripts_.failure();
        source_boundary(
            "field.world_activity." + effect.name + "_list_unresolved");
        return true;
    }
    pending_field_encounter_outcome_ = std::move(*outcome);
    source_boundary(
        "field.world_activity." + effect.name + "_sequence_resolved");
    spawn_field_world_effect_sequence();
    return true;
}

void FieldSceneRuntime::trigger_field_world_effect_sequence() {
    if (process_pending_story_world_effect_sequence()) {
        return;
    }
    scripting::ScriptFieldEncounterTriggerInput trigger{};
    trigger.scene_id = state_.field_scene_buffers.scene_reference;
    trigger.frame_counter = field_world_frame_counter_;
    trigger.trigger_delay_active = state_.display_service_word != 0U;
    trigger.current_entity_protected =
        (field_movement_behavior_record_.record_flags & 0x80U) != 0U;
    trigger.activity_lock = field_runtime_state_.field_activity_lock;
    trigger.sequence_mode = world_frame_logic_state_.event_lock != 0U;
    trigger.world_effects_active =
        field_world_activity_timeout_state_.timeout_marker != 0;
    trigger.placement_pending =
        field_world_activity_timeout_state_.scene_36_gate != 0U;

    auto action = game_scripts_.evaluate_field_encounter_trigger(trigger);
    if (!action.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.random_encounter.script_gate_failed");
        return;
    }
    if (*action !=
        scripting::ScriptFieldEncounterTriggerAction::sample_probability) {
        return;
    }

    // 0x1E166 advances the source PRNG only after its state and 0x3FF cadence
    // gates pass. Lua requests this draw, then owns the probability decision.
    trigger.probability_roll = advance_source_random_seed(field_random_seed_);
    trigger.probability_threshold = state_.field_scene_buffers.profile[
        field_scene_profile_word_index(
            FieldSceneProfileWord::random_event_probability_threshold)];
    action = game_scripts_.evaluate_field_encounter_trigger(trigger);
    if (!action.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.random_encounter.script_chance_failed");
        return;
    }
    switch (*action) {
    case scripting::ScriptFieldEncounterTriggerAction::blocked:
    case scripting::ScriptFieldEncounterTriggerAction::not_due:
    case scripting::ScriptFieldEncounterTriggerAction::random_miss:
        return;
    case scripting::ScriptFieldEncounterTriggerAction::forward_active_sequence:
        source_boundary("field.random_encounter.forward_existing_sequence");
        return;
    case scripting::ScriptFieldEncounterTriggerAction::select_scene_outcome:
        (void)select_field_world_effect_sequence();
        if (!pending_field_encounter_outcome_.has_value()) {
            return;
        }
        if (pending_field_encounter_outcome_->kind ==
            scripting::ScriptFieldEncounterOutcomeKind::dialogue_event) {
            failure_ = "ROM random-event dialogue handler is selected but not connected";
            source_boundary(
                "field.random_encounter.dialogue_handler_unimplemented");
            return;
        }
        spawn_field_world_effect_sequence();
        return;
    case scripting::ScriptFieldEncounterTriggerAction::sample_probability:
        failure_ = "Lua field encounter gate requested a second probability draw";
        source_boundary("field.random_encounter.repeated_probability_request");
        return;
    }
}

bool FieldSceneRuntime::load_field_weapon_spawn_assets(
    const ConversionAssetCatalog& catalog) {
    for (std::size_t animation = 0;
         animation < field_weapon_projectile_images_.size(); ++animation) {
        auto& images = field_weapon_projectile_images_[animation];
        auto& origin_x = field_weapon_projectile_origin_x_[animation];
        auto& origin_y = field_weapon_projectile_origin_y_[animation];
        images.clear();
        origin_x.clear();
        origin_y.clear();

        const auto selector = animation * 4U;
        const auto expected_frames = animation < 8U ? 3U : 11U;
        for (std::size_t frame = 0; frame < expected_frames; ++frame) {
            const auto alias =
                std::string(kFieldWeaponProjectileAliasPrefix) +
                std::to_string(selector) + "." + std::to_string(frame);
            QImage image;
            int source_origin_x = 0;
            int source_origin_y = 0;
            if (!load_sprite_image(catalog, alias, image,
                                   source_origin_x, source_origin_y)) {
                return false;
            }
            images.push_back(std::move(image));
            origin_x.push_back(source_origin_x);
            origin_y.push_back(source_origin_y);
        }
    }
    return true;
}

#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::field_weapon_projectile_selector_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.secondary_active) {
            return static_cast<std::uint16_t>(
                spawn.source_record.animation_resource & 0x7FFFU);
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<bool>
FieldSceneRuntime::field_weapon_projectile_list_finished_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.secondary_active) {
            return (spawn.source_record.animation_resource & 0x8000U) != 0U;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::field_weapon_projectile_frame_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.secondary_active) {
            return spawn.animation.frame;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::field_weapon_primary_count_for_dump() const
    noexcept {
    std::size_t count = 0U;
    for (const auto& spawn : field_weapon_projectiles_) {
        count += spawn.primary_active ? 1U : 0U;
    }
    return count;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<SceneRunnerPosition>
FieldSceneRuntime::field_weapon_primary_position_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return SceneRunnerPosition{spawn.primary_record.position_x,
                                       spawn.primary_record.position_y};
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<SceneRunnerPosition>
FieldSceneRuntime::field_weapon_primary_spawn_position_for_dump() const
    noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return spawn.primary_spawn_position;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::field_weapon_primary_heading_for_dump() const noexcept {
    for (auto spawn = field_weapon_projectiles_.rbegin();
         spawn != field_weapon_projectiles_.rend(); ++spawn) {
        if (spawn->primary_active) {
            return spawn->primary_record.heading;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<EntityHeadingOffsets>
FieldSceneRuntime::field_weapon_primary_heading_offsets_for_dump() const
    noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return EntityHeadingOffsets{
                spawn.primary_record.heading_offset_x,
                spawn.primary_record.heading_offset_y};
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::field_weapon_primary_retry_count_for_dump() const noexcept {
    for (const auto& spawn : field_weapon_projectiles_) {
        if (spawn.primary_active) {
            return spawn.primary_record.field_0x89;
        }
    }
    return std::nullopt;
}
#endif // SHADOWRUN_QT_QA_DRIVER


bool FieldSceneRuntime::load_field_hud_overlays(
    const ConversionAssetCatalog& catalog) {
    int origin_x = 0;
    int origin_y = 0;
    const auto load_slot = [this, &catalog, &origin_x, &origin_y](
                               const std::string_view prefix,
                               std::array<QImage, kFieldHudSlotCount>&
                                   images) {
        for (std::size_t slot = 0; slot < images.size(); ++slot) {
            const auto alias = std::string(prefix) + std::to_string(slot);
            if (!load_sprite_image(catalog, alias, images[slot], origin_x,
                                   origin_y)) {
                return false;
            }
        }
        return true;
    };
    if (!load_slot(kFieldHudFaceAliasPrefix, field_hud_faces_) ||
        !load_slot(kFieldHudItemAliasPrefix, field_hud_items_) ||
        !load_slot(kFieldHudUnarmedItemAliasPrefix,
                   field_hud_unarmed_items_)) {
        return false;
    }
    for (std::size_t family = 0; family < field_hud_party_faces_.size();
         ++family) {
        for (std::size_t palette = 0;
             palette < field_hud_party_faces_[family].size(); ++palette) {
            const auto alias = std::string(kFieldHudPartyFaceAliasPrefix) +
                               std::to_string(family) + ".palette." +
                               std::to_string(palette);
            if (!load_sprite_image(catalog, alias,
                                   field_hud_party_faces_[family][palette],
                                   origin_x, origin_y)) {
                return false;
            }
        }
    }
    for (std::size_t item = 0; item < field_hud_party_items_.size(); ++item) {
        for (std::size_t palette = 0;
             palette < field_hud_party_items_[item].size(); ++palette) {
            const auto alias = std::string(kFieldHudPartyItemAliasPrefix) +
                               std::to_string(item + 1U) + ".palette." +
                               std::to_string(palette);
            if (!load_sprite_image(catalog, alias,
                                   field_hud_party_items_[item][palette],
                                   origin_x, origin_y)) {
                return false;
            }
        }
    }
    for (std::size_t item = 0;
         item < field_hud_party_unarmed_items_.size(); ++item) {
        for (std::size_t palette = 0;
             palette < field_hud_party_unarmed_items_[item].size(); ++palette) {
            const auto alias =
                std::string(kFieldHudPartyUnarmedItemAliasPrefix) +
                std::to_string(item) + ".palette." +
                std::to_string(palette);
            if (!load_sprite_image(
                    catalog, alias,
                    field_hud_party_unarmed_items_[item][palette],
                    origin_x, origin_y)) {
                return false;
            }
        }
    }
    for (std::size_t spell = 0; spell < field_hud_magic_items_.size();
         ++spell) {
        for (std::size_t palette = 0;
             palette < field_hud_magic_items_[spell].size(); ++palette) {
            const auto alias = std::string(kFieldHudMagicItemAliasPrefix) +
                               std::to_string(spell) + ".palette." +
                               std::to_string(palette);
            if (!load_sprite_image(catalog, alias,
                                   field_hud_magic_items_[spell][palette],
                                   origin_x, origin_y)) {
                return false;
            }
        }
    }
    for (std::size_t meter = 0; meter < kFieldHudMeterLevelCount; ++meter) {
        const auto delay_alias =
            std::string(kFieldHudDelayMeterAliasPrefix) +
            std::to_string(meter);
        const auto primary_alias =
            std::string(kFieldHudBehaviorMeterPrimaryAliasPrefix) +
            std::to_string(meter);
        const auto companion_alias =
            std::string(kFieldHudBehaviorMeterCompanionAliasPrefix) +
            std::to_string(meter);
        if (!load_sprite_image(catalog, delay_alias,
                               field_hud_delay_meter_counters_[meter],
                               origin_x, origin_y) ||
            !load_sprite_image(
                catalog, primary_alias,
                field_hud_behavior_meter_counters_[meter], origin_x,
                origin_y) ||
            !load_sprite_image(
                catalog, companion_alias,
                field_hud_behavior_meter_counters_[
                    kFieldHudMeterLevelCount + meter],
                origin_x, origin_y)) {
            return false;
        }
    }
    for (std::size_t ammo = 0; ammo < field_hud_ammo_counters_.size();
         ++ammo) {
        const auto alias = std::string(kFieldHudAmmoCounterAliasPrefix) +
                           std::to_string(ammo);
        if (!load_sprite_image(catalog, alias, field_hud_ammo_counters_[ammo],
                               origin_x, origin_y)) {
            return false;
        }
    }
    return true;
}

bool FieldSceneRuntime::load_command_overview_assets(
    const ConversionAssetCatalog& catalog) {
    if (!load_image(catalog, kCommandOverviewBackgroundAlias,
                    command_overview_background_) ||
        !load_image(catalog, kCommandOverviewInventoryBoxesAlias,
                    command_overview_inventory_boxes_) ||
        !load_image(catalog, kCommandOverviewActionBackgroundAlias,
                    command_overview_action_background_) ||
        !load_image(catalog,
                    kCommandOverviewActionConfirmationBackgroundAlias,
                    command_overview_action_confirmation_background_) ||
        !load_image(catalog, kCommandOverviewClipsBackgroundAlias,
                    command_overview_clips_background_) ||
        !load_image(catalog, kCommandOverviewClipsTradeBackgroundAlias,
                    command_overview_clips_trade_background_) ||
        !load_image(catalog, "menu.field.font", command_overview_font_) ||
        !load_image(catalog, "menu.field.magic.font.palette.0",
                    magic_menu_font_) ||
        !load_image(catalog, "menu.field.font.palette.2",
                    hotel_checkout_font_) ||
        !load_image(catalog, "menu.pocket.secretary.background",
                    pocket_secretary_background_) ||
        !load_image(catalog, "menu.pocket.secretary.grid",
                    pocket_secretary_grid_) ||
        !load_image(catalog, "menu.field.hotel.cursor",
                    hotel_options_cursor_) ||
        !load_image(catalog, "menu.field.hotel.checkout.cursor",
                    hotel_checkout_cursor_) ||
        !load_image(catalog, "menu.field.attributes.background",
                    command_overview_attributes_background_) ||
        !load_image(catalog, "menu.field.posture.focus", posture_focus_) ||
        !load_image(catalog, "menu.field.magic.panel.background",
                    command_overview_magic_panel_background_)) {
        return false;
    }
    if (!load_image(catalog,
                    "cyberspace.matrix.system.map.background",
                    matrix_map_background_) ||
        matrix_map_background_.size() != QSize(320, 224) ||
        !load_image(catalog,
                    "cyberspace.matrix.system.map.font",
                    matrix_map_font_) ||
        matrix_map_font_.size() != QSize(128, 48) ||
        !load_image(catalog, "cyberspace.matrix.cyberdeck.panel",
                    matrix_cyberdeck_panel_) ||
        matrix_cyberdeck_panel_.size() != QSize(320, 224) ||
        !load_image(catalog,
                    "cyberspace.matrix.cyberdeck.runner.detail.headers",
                    matrix_cyberdeck_runner_detail_headers_) ||
        matrix_cyberdeck_runner_detail_headers_.size() != QSize(320, 224) ||
        !load_image(catalog,
                    "cyberspace.matrix.cyberdeck.program.icon.empty",
                    matrix_cyberdeck_empty_program_icon_) ||
        matrix_cyberdeck_empty_program_icon_.size() != QSize(24, 16)) {
        failure_ = "Matrix source artwork is unavailable";
        return false;
    }
    for (std::size_t theme = 0U;
         theme < matrix_traversal_backgrounds_.size(); ++theme) {
        const auto alias = "cyberspace.matrix.traversal.background.theme." +
                           std::to_string(theme + 1U);
        auto& background = matrix_traversal_backgrounds_[theme];
        if (!load_image(catalog, alias, background) ||
            background.size() != QSize(320, 224)) {
            failure_ = "Matrix traversal background is unavailable: " + alias;
            return false;
        }
    }
    if (!load_image(catalog, "cyberspace.matrix.traversal.hud.frame",
                    matrix_traversal_hud_frame_) ||
        matrix_traversal_hud_frame_.size() != QSize(320, 224)) {
        failure_ = "Matrix traversal HUD frame is unavailable";
        return false;
    }
    for (std::size_t model = 0U; model < matrix_world_models_.size(); ++model) {
        for (std::size_t security_color = 0U; security_color <= 4U;
             ++security_color) {
            const auto alias = "cyberspace.matrix.world.model." +
                std::to_string(model) + ".security.color." +
                std::to_string(security_color);
            auto& image = matrix_world_models_[model][security_color];
            const auto expected_size = model == 0U ? QSize(112, 64)
                                                    : QSize(96, 80);
            if (!load_image(catalog, alias, image) ||
                image.size() != expected_size) {
                failure_ = "Matrix world model is unavailable: " + alias;
                return false;
            }
        }
    }
    if (!load_image(
            catalog,
            "cyberspace.matrix.traversal.runner.world.sequence.phase.4",
            matrix_traversal_runner_entry_pose_) ||
        matrix_traversal_runner_entry_pose_.size() != QSize(320, 224)) {
        failure_ = "Matrix traversal entry runner pose is unavailable";
        return false;
    }
    if (!load_image(
            catalog,
            "cyberspace.matrix.traversal.runner.action.phase.0",
            matrix_traversal_runner_action_phase_0_) ||
        matrix_traversal_runner_action_phase_0_.size() != QSize(320, 224) ||
        !load_image(
            catalog,
            "cyberspace.matrix.traversal.runner.action.raised",
            matrix_traversal_runner_action_phase_1_) ||
        matrix_traversal_runner_action_phase_1_.size() != QSize(320, 224)) {
        failure_ = "Matrix Analyze runner action poses are unavailable";
        return false;
    }
    if (!load_image(catalog, "cyberspace.matrix.traversal.runner.idle",
                    matrix_traversal_runner_idle_) ||
        matrix_traversal_runner_idle_.size() != QSize(320, 224)) {
        failure_ = "Matrix idle runner pose is unavailable";
        return false;
    }
    for (std::size_t phase = 0U;
         phase < matrix_traversal_analyze_rings_.size(); ++phase) {
        const auto alias =
            "cyberspace.matrix.traversal.analyze.ring.phase." +
            std::to_string(phase);
        auto& image = matrix_traversal_analyze_rings_[phase];
        if (!load_image(catalog, alias, image) ||
            image.size() != QSize(320, 224)) {
            failure_ = "Matrix Analyze ring phase is unavailable: " + alias;
            return false;
        }
    }
    const auto load_matrix_world_effect_frames =
        [this, &catalog](const std::string& alias_prefix,
                         std::vector<QImage>& frames) {
            std::vector<std::pair<std::size_t, std::string>> aliases;
            for (const auto& record : catalog.records) {
                const std::string_view alias(record.alias);
                if (alias.size() <= alias_prefix.size() ||
                    alias.compare(0, alias_prefix.size(), alias_prefix) != 0) {
                    continue;
                }
                const auto suffix = alias.substr(alias_prefix.size());
                std::size_t phase = 0U;
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), phase);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != suffix.data() + suffix.size()) {
                    failure_ = "Invalid Matrix world-effect asset alias: " +
                               record.alias;
                    return false;
                }
                aliases.emplace_back(phase, record.alias);
            }
            if (aliases.empty()) {
                failure_ = "Matrix world-effect assets are unavailable: " +
                           alias_prefix;
                return false;
            }
            std::sort(aliases.begin(), aliases.end(),
                      [](const auto& left, const auto& right) {
                          return left.first < right.first;
                      });
            frames.clear();
            frames.resize(aliases.back().first + 1U);
            for (std::size_t index = 0U; index < aliases.size(); ++index) {
                const auto phase = aliases[index].first;
                if (phase != index ||
                    !load_image(catalog, aliases[index].second,
                                frames[phase]) ||
                    frames[phase].size() != QSize(320, 224)) {
                    failure_ = "Matrix world-effect phase is missing or has "
                               "the wrong size: " + aliases[index].second;
                    return false;
                }
            }
            return true;
        };
    for (std::size_t mode = 0U;
         mode < matrix_traversal_world_effect_modes_.size(); ++mode) {
        const auto alias_prefix =
            "cyberspace.matrix.traversal.world.effect.mode." +
            std::to_string(mode) + ".phase.";
        if (!load_matrix_world_effect_frames(
                alias_prefix, matrix_traversal_world_effect_modes_[mode])) {
            return false;
        }
    }
    if (!load_matrix_world_effect_frames(
            "cyberspace.matrix.traversal.world.effect.default.phase.",
            matrix_traversal_world_effect_fallback_)) {
        return false;
    }
    for (std::size_t phase = 0U;
         phase < matrix_cyberdeck_backgrounds_.size(); ++phase) {
        const auto suffix = phase == 0U
            ? std::string{}
            : ".phase." + std::to_string(phase);
        const auto background_alias =
            "cyberspace.matrix.cyberdeck.background" + suffix;
        auto& background = matrix_cyberdeck_backgrounds_[phase];
        if (!load_image(catalog, background_alias, background) ||
            background.size() != QSize(320, 224)) {
            failure_ = "Matrix deck palette background is unavailable: " +
                       background_alias;
            return false;
        }
        const auto cursor_alias =
            "cyberspace.matrix.cyberdeck.cursor" + suffix;
        auto& cursor = matrix_cyberdeck_cursors_[phase];
        if (!load_image(catalog, cursor_alias, cursor) ||
            cursor.size() != QSize(8, 8)) {
            failure_ = "Matrix deck palette cursor is unavailable: " +
                       cursor_alias;
            return false;
        }
    }
    for (std::size_t profile = 0U;
         profile < matrix_node_backgrounds_.size(); ++profile) {
        for (std::size_t theme = 1U;
             theme <= matrix_node_backgrounds_[profile].size(); ++theme) {
            const auto alias =
                "cyberspace.matrix.node.background.profile." +
                std::to_string(profile) + ".color." +
                std::to_string(theme);
            auto& background = matrix_node_backgrounds_[profile][theme - 1U];
            if (!load_image(catalog, alias, background) ||
                background.size() != QSize(320, 224)) {
                failure_ = "Matrix node background is unavailable: " + alias;
                return false;
            }
        }
    }
    for (std::size_t theme = 0U;
         theme < matrix_node_action_cursors_.size(); ++theme) {
        const auto alias =
            "cyberspace.matrix.node.action.cursor.color." +
            std::to_string(theme + 1U);
        auto& cursor = matrix_node_action_cursors_[theme];
        if (!load_image(catalog, alias, cursor) ||
            cursor.size() != QSize(8, 8)) {
            failure_ = "Matrix node-action cursor is unavailable: " + alias;
            return false;
        }
    }
    for (std::size_t row = 0U;
         row < matrix_cyberdeck_selector_rows_.size(); ++row) {
        for (std::size_t phase = 0U;
             phase < matrix_cyberdeck_selector_rows_[row].size(); ++phase) {
            const auto suffix = phase == 0U
                ? std::string{}
                : ".phase." + std::to_string(phase);
            const auto alias =
                "cyberspace.matrix.cyberdeck.selector.selected.row." +
                std::to_string(row) + suffix;
            auto& selector = matrix_cyberdeck_selector_rows_[row][phase];
            if (!load_image(catalog, alias, selector) ||
                selector.size() != QSize(320, 224)) {
                failure_ = "Matrix deck selector artwork is unavailable: " +
                           alias;
                return false;
            }
        }
    }
    for (std::size_t icon = 0U;
         icon < matrix_cyberdeck_program_icons_.size(); ++icon) {
        const auto alias = "cyberspace.matrix.program.icon." +
                           std::to_string(icon);
        auto& image = matrix_cyberdeck_program_icons_[icon];
        if (!load_image(catalog, alias, image) ||
            image.size() != QSize(24, 16)) {
            failure_ = "Matrix deck icon artwork is unavailable: " + alias;
            return false;
        }
        const auto detail_alias = "cyberspace.matrix.detail.strip." +
                                  std::to_string(icon);
        auto& detail = matrix_cyberdeck_detail_strips_[icon];
        if (!load_image(catalog, detail_alias, detail) ||
            detail.size() != QSize(48, 8)) {
            failure_ = "Matrix deck detail artwork is unavailable: " +
                       detail_alias;
            return false;
        }
    }
    for (std::size_t status = 0U;
         status < matrix_cyberdeck_footer_slot_status_.size(); ++status) {
        const auto alias = "cyberspace.matrix.cyberdeck.footer.slot.status." +
                           std::to_string(status);
        auto& image = matrix_cyberdeck_footer_slot_status_[status];
        if (!load_image(catalog, alias, image) ||
            image.size() != QSize(24, 8)) {
            failure_ = "Matrix deck footer status artwork is unavailable: " +
                       alias;
            return false;
        }
    }
    for (std::size_t palette_line = 1U; palette_line <= 3U;
         ++palette_line) {
        const auto alias = "cyberspace.matrix.map.tiles.palette." +
                           std::to_string(palette_line);
        auto& atlas = matrix_map_tile_atlases_[palette_line];
        if (!load_image(catalog, alias, atlas) ||
            atlas.size() != QSize(128, 16)) {
            failure_ = "Matrix source map tile palette is unavailable: " +
                       std::to_string(palette_line);
            return false;
        }
    }
    constexpr std::string_view kRunnerPreviewAliasPrefix =
        "menu.field.runner.preview.table.";
    for (const auto& resource : catalog.records) {
        if (resource.alias.rfind(kRunnerPreviewAliasPrefix.data(), 0U) != 0U) {
            continue;
        }
        QImage image;
        if (!load_image(catalog, resource.alias, image)) {
            return false;
        }
        runner_preview_panels_.emplace(resource.alias, std::move(image));
    }
    for (std::size_t index = 0;
         index < dismiss_runner_random_grid_patterns_.size(); ++index) {
        // E660's fixed-family RNG result selects source tables in this order:
        // 0 -> CE136 (yellow), 1 -> CE12E (red), 2 -> CE126 (blue). The
        // extracted catalog is address-ordered (CE126, CE12E, CE136), so
        // store those three images in source-selection order for both the Lua
        // route and the remaining native menu adapters.
        const auto alias = index < 3U
            ? std::string(kRandomGridFixedAliasPrefix) +
                  std::to_string(2U - index)
            : std::string(kRandomGridIndexedAliasPrefix) +
                  std::to_string(index - 3U);
        if (!load_image(catalog, alias,
                        dismiss_runner_random_grid_patterns_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_spell_icons_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.spell.icon." +
                            std::to_string(index),
                        command_overview_magic_spell_icons_[index])) {
            return false;
        }
    }
    for (std::size_t phase = 0;
         phase < command_overview_magic_cursor_frames_.size(); ++phase) {
        if (!load_image(catalog,
                        "menu.field.magic.cursor.frame." +
                            std::to_string(phase),
                        command_overview_magic_cursor_frames_[phase])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_posture_bars_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.posture.bar." +
                            std::to_string(index),
                        command_overview_magic_posture_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_force_meter_tiles_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.force.meter.tile." +
                            std::to_string(index),
                        command_overview_magic_force_meter_tiles_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_metric_bars_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.metric.bar." +
                            std::to_string(index),
                        command_overview_magic_metric_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_availability_bars_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.availability.bar." +
                            std::to_string(index),
                        command_overview_magic_availability_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_magic_damage_bars_.size(); ++index) {
        if (!load_image(catalog,
                        "menu.field.magic.damage.bar." +
                            std::to_string(index),
                        command_overview_magic_damage_bars_[index])) {
            return false;
        }
    }
    // The source inventory-box writer changes only the box outline cells;
    // its extracted plane-sized bitmap carries color-zero as alpha 0. The
    // host already has the opaque command shell underneath, so preserve that
    // shell instead of letting the overlay's black canvas erase it.
    const auto load_sprite = [this, &catalog](const std::string& alias,
                                               QImage& image) {
        if (!load_image(catalog, alias, image)) {
            return false;
        }
        return true;
    };
    const auto archetypes = game_scripts_.menu(
        "startup", "archetypes", "startup", "archetypes");
    const auto portrait_bindings =
        game_scripts_.string_map("startup", "archetype_portraits");
    if (!archetypes.has_value() || !portrait_bindings.has_value() ||
        archetypes->choices.size() != command_overview_portraits_.size() ||
        portrait_bindings->size() != archetypes->choices.size()) {
        failure_ = "startup Lua archetype portraits are incomplete";
        return false;
    }
    for (std::size_t index = 0; index < archetypes->choices.size(); ++index) {
        const auto binding = portrait_bindings->find(
            archetypes->choices[index].id);
        if (binding == portrait_bindings->end() ||
            !load_sprite(binding->second, command_overview_portraits_[index])) {
            failure_ = "startup Lua archetype portrait asset is unavailable: " +
                       archetypes->choices[index].id;
            return false;
        }
    }
    for (std::size_t index = 0; index < command_overview_focus_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewFocusAliasPrefix) +
                             std::to_string(index),
                         command_overview_focus_[index])) {
            return false;
        }
        if (!load_sprite(std::string(kCommandOverviewStatusAliasPrefix) +
                             std::to_string(index),
                         command_overview_status_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0; index < command_overview_selected_cells_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewSelectedCellAliasPrefix) +
                             std::to_string(index),
                         command_overview_selected_cells_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_action_focus_.size(); ++index) {
        // This is a source name-table block, not a transparent SAT sprite.
        // The extractor preserves color-zero pixels as opaque because the
        // Genesis writes the complete 16x16 action focus block over the
        // action row.  Keep that composition when presenting the direct
        // host image.
        if (!load_image(catalog,
                        std::string(kCommandOverviewActionFocusAliasPrefix) +
                            std::to_string(index),
                        command_overview_action_focus_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_action_confirmation_focus_.size(); ++index) {
        if (!load_image(
                catalog,
                std::string(
                    kCommandOverviewActionConfirmationFocusAliasPrefix) +
                    std::to_string(index),
                command_overview_action_confirmation_focus_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_clips_focus_.size(); ++index) {
        // 0x11B58 passes CD43A to the same complete 2x2 name-table writer.
        // These are opaque source blocks, not transparent host highlights.
        if (!load_image(catalog,
                        std::string(kCommandOverviewClipsFocusAliasPrefix) +
                            std::to_string(index),
                        command_overview_clips_focus_[index]) ||
            !load_image(
                catalog,
                std::string(kCommandOverviewClipsTradeFocusAliasPrefix) +
                    std::to_string(index),
                command_overview_clips_trade_focus_[index])) {
            return false;
        }
    }
    if (!load_sprite(std::string(kCommandOverviewCursorAlias),
                     command_overview_cursor_)) {
        return false;
    }

    for (std::size_t item_index = 0U;
         item_index < field_menu_text_.items.size(); ++item_index) {
        const auto& item = field_menu_text_.items[item_index].definition;
        if (item.code != item_index + 1U ||
            item.presentation.icon_resource_key.empty() ||
            item.presentation.detail_resource_key.empty()) {
            failure_ = "Lua item presentation catalog is incomplete at item " +
                       std::to_string(item_index + 1U);
            return false;
        }
        for (std::size_t palette = 0U; palette < 2U; ++palette) {
            const auto alias = command_overview_item_alias(
                item,
                static_cast<std::uint8_t>(palette));
            if (!load_sprite(alias, command_overview_inventory_icons_[item_index]
                                             [palette])) {
                return false;
            }
        }
        if (!load_sprite(command_overview_item_detail_alias(
                             item),
                         command_overview_item_details_[item_index])) {
            return false;
        }
    }
    for (std::size_t index = 0; index < command_overview_damage_bars_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewDamageBarAliasPrefix) +
                             std::to_string(index),
                         command_overview_damage_bars_[index]) ||
            !load_sprite(std::string(kCommandOverviewDrainBarAliasPrefix) +
                             std::to_string(index),
                         command_overview_drain_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_armor_summary_bars_.size(); ++index) {
        if (!load_sprite(
                std::string(kCommandOverviewArmorSummaryBarAliasPrefix) +
                    std::to_string(index),
                command_overview_armor_summary_bars_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0;
         index < command_overview_detail_values_.size(); ++index) {
        if (!load_sprite(std::string(kCommandOverviewDetailValueAliasPrefix) +
                             std::to_string(index),
                         command_overview_detail_values_[index])) {
            return false;
        }
    }
    for (std::size_t index = 0; index < command_overview_attack_bars_.size();
         ++index) {
        if (!load_sprite(std::string(kCommandOverviewAttackBarAliasPrefix) +
                             std::to_string(index),
                         command_overview_attack_bars_[index]) ||
            !load_sprite(std::string(kCommandOverviewDefenseBarAliasPrefix) +
                             std::to_string(index),
                         command_overview_defense_bars_[index])) {
            return false;
        }
    }
    return true;
}

void FieldSceneRuntime::set_audio_host_callbacks(
    std::function<void(std::uint16_t)> play_music,
    std::function<void(std::uint8_t)> attenuate,
    std::function<void()> fade_out) {
    source_music_callback_ = std::move(play_music);
    source_attenuation_callback_ = std::move(attenuate);
    source_fade_callback_ = std::move(fade_out);
}

void FieldSceneRuntime::request_scene_field_music() {
    // ROM 0x2830 -> 0x2900: play_music(FFDE06), FFDE06 = profile word +0x40.
    // audio.lua decides; C++ only supplies the profile word.
    constexpr std::size_t kSceneProfileMusicWord = 0x40U / 2U;
    const auto scene = state_.field_scene_buffers.scene_reference;
    const auto profile_music =
        state_.field_scene_buffers.profile[kSceneProfileMusicWord];
    const auto music = game_scripts_.scene_music(scene, profile_music);
    if (!music.has_value()) {
        if (!game_scripts_.failure().empty()) {
            source_boundary("field.audio.scene_music_unresolved");
        }
        return;
    }
    field_music_requests_.push_back(*music);
    if (source_music_callback_) {
        source_music_callback_(*music);
    }
}

void FieldSceneRuntime::service_scene_music_latch() {
    // 0x2900: tst.b FFE172; clear it and play_music(FFDE06). Several host
    // scene preparations in one frame therefore issue one song start.
    if (scene_music_latch_) {
        scene_music_latch_ = false;
        request_scene_field_music();
    }
}

void FieldSceneRuntime::request_audio_attenuation(const std::uint8_t level) {
    field_attenuation_requests_.push_back(level);
    if (source_attenuation_callback_) {
        source_attenuation_callback_(level);
    }
}

void FieldSceneRuntime::request_audio_fade() {
    ++field_fade_requests_;
    if (source_fade_callback_) {
        source_fade_callback_();
    }
}

void FieldSceneRuntime::restore_start_menu_duck() {
    // ROM 0xB7A: 0x53156 (op 0x20, level 0) after the Start menu returns.
    if (start_menu_duck_active_) {
        start_menu_duck_active_ = false;
        request_audio_attenuation(0U);
    }
}

void FieldSceneRuntime::configure_callbacks() {
    callbacks_.mode_entry_audio.run_fixed_audio_transaction =
        [this](const std::uint32_t command) {
            // The six levels and their 3-frame spacing are generated by the
            // host GEMS layer's fade_out (see flush_audio_bus below).
            source_boundary("field.audio.mode_entry.command");
            (void)command;
        };
    callbacks_.mode_entry_audio.wait_for_vblank_frames =
        [this](const std::uint16_t counter) {
            source_boundary("field.audio.mode_entry.wait");
            (void)counter;
        };
    callbacks_.mode_entry_audio.flush_audio_bus = [this] {
        source_boundary("field.audio.mode_entry.flush");
        // 0x53126 as one host request: the GEMS layer replays the
        // attenuation ramp, FF 16 and the level-0 restore over 18 frames.
        request_audio_fade();
    };
    callbacks_.mode_entry_audio.service_audio_tick = [this] {
        source_boundary("field.audio.mode_entry.service");
    };

    callbacks_.initialize_entity_order = [this] {
        source_boundary("field.entities.initialize_order");
        state_.render_entities[0].active = field_entity_.active;
        state_.render_entities[0].id = 0U;
    };
    callbacks_.prepare_field_scene_runtime = [this] {
        const auto decision = prepare_field_scene_runtime(
            field_action_counter_,
            {[this] { refresh_field_runner_state_and_assets(); },
             [this] { run_field_mode_exit_transition(); }});
        source_boundary(
            decision.path ==
                    FieldSceneRuntimePreparationPath::
                        refresh_runner_state_and_assets
                ? "field.scene.runtime_path.refresh"
                : "field.scene.runtime_path.finalize");
    };
    callbacks_.field_resources.prepare_asset_setup = [this] {
        source_boundary("field.resources.prepare_asset_setup");
    };
    callbacks_.field_resources.upload_asset_sequence = [this] {
        source_boundary("field.resources.upload_asset_sequence");
    };
    callbacks_.field_resources.refresh_animation_assets = [this] {
        source_boundary("field.resources.refresh_animation_assets");
    };
    callbacks_.field_resources.load_font = [this] {
        source_boundary("field.resources.load_font");
    };

    world_frame_callbacks_.service_audio = [this] {
        source_boundary("field.world.service_audio");
    };
    world_frame_callbacks_.reset_effect_record = [this] {
        source_boundary("field.world.reset_effect_record");
    };
    world_frame_callbacks_.begin_effect_transition = [this] {
        source_boundary("field.world.begin_effect_transition");
    };
    world_frame_callbacks_.clear_effect_slots = [this] {
        source_boundary("field.world.clear_effect_slots");
    };
    world_frame_callbacks_.clear_entity_effects = [this] {
        source_boundary("field.world.clear_entity_effects");
    };
    world_frame_callbacks_.clear_scene_pending = [this] {
        source_boundary("field.world.clear_scene_pending");
    };
    world_frame_callbacks_.finalize_entity_templates = [this] {
        source_boundary("field.world.finalize_entity_templates");
    };
    world_frame_callbacks_.service_scene_effect = [this] {
        source_boundary("field.world.service_scene_effect");
    };
    world_frame_callbacks_.copy_terrain_layers = [this] {
        source_boundary("field.world.copy_terrain_layers");
    };
    world_frame_callbacks_.process_terrain_entries = [this] {
        source_boundary("field.world.process_terrain_entries");
    };
    world_frame_callbacks_.rebuild_collision_map = [this] {
        source_boundary("field.world.rebuild_collision_map");
    };
    world_frame_callbacks_.load_scene_resources = [this] {
        source_boundary("field.world.load_scene_resources");
    };
    world_frame_callbacks_.clear_scene_transitions = [this] {
        source_boundary("field.world.clear_scene_transitions");
    };
    world_frame_callbacks_.populate_entities = [this] {
        source_boundary("field.world.populate_entities");
    };
    world_frame_callbacks_.dispatch_entity_actions = [this] {
        source_boundary("field.world.dispatch_entity_actions");
    };
    world_frame_callbacks_.decode_entity_data = [this] {
        source_boundary("field.world.decode_entity_data");
    };
    world_frame_callbacks_.mark_entities_for_render = [this] {
        source_boundary("field.world.mark_entities_for_render");
    };
    callbacks_.run_world_frame_services = [this] {
        run_world_frame_services(world_frame_state_, world_frame_callbacks_);
    };

    const auto publish_runner_positions =
        [this](const std::array<SceneRunnerPosition, 3>& positions) {
            auto& records = state_.field_scene_buffers.source_metadata
                                .initial_field_records;
            for (std::size_t index = 0; index < positions.size() &&
                                        index < records.size();
                 ++index) {
                records[index] = positions[index];
            }
            field_entity_.world_position.x = positions[0].x;
            field_entity_.world_position.y = positions[0].y;
            field_movement_state_.position_x = positions[0].x;
            field_movement_state_.position_y = positions[0].y;
        };
    callbacks_.apply_scene_profile_selection = [this, publish_runner_positions] {
        if (field_pending_scene_profile_.has_value()) {
            SceneProfileState profile{};
            SceneProfileDefaults defaults{};
            apply_scene_profile_selection(
                profile, field_pending_scene_profile_, defaults);
            state_.field_scene_buffers.camera_x = profile.camera_x;
            state_.field_scene_buffers.camera_y = profile.camera_y;
            publish_runner_positions(profile.runner_positions);
            return;
        }

        // ROM 0x2B3E copies the six FFDE08 words into the three entity slots.
        // It does not move the camera.
        const auto& scene_profile = state_.field_scene_buffers.profile;
        if (scene_profile.size() < kSceneProfileEntityPositionWord + 6U) {
            source_boundary("field.scene.profile_positions_missing");
            return;
        }
        publish_runner_positions({
            SceneRunnerPosition{
                scene_profile[kSceneProfileEntityPositionWord],
                scene_profile[kSceneProfileEntityPositionWord + 1U]},
            SceneRunnerPosition{
                scene_profile[kSceneProfileEntityPositionWord + 2U],
                scene_profile[kSceneProfileEntityPositionWord + 3U]},
            SceneRunnerPosition{
                scene_profile[kSceneProfileEntityPositionWord + 4U],
                scene_profile[kSceneProfileEntityPositionWord + 5U]}});
    };
    callbacks_.field_viewports.prepare_title_resources = [this] {
        source_boundary("field.viewport.prepare_title_resources");
    };
    callbacks_.field_viewports.copy_mode_selection_state_table = [this] {
        source_boundary("field.viewport.copy_mode_selection_state_table");
    };
    callbacks_.field_viewports.upload_mode_word_block = [this] {
        source_boundary("field.viewport.upload_mode_word_block");
    };
    callbacks_.field_viewports.render_normal_viewport = [this] {
        // ROM 0x2C92 clears FFE100 before drawing the normal viewport.
        field_camera_follow_.layer_x_dirty = false;
        render_normal_field_viewport(
            viewport_state_,
            [this] { source_boundary("field.viewport.render_normal"); });
    };
    callbacks_.field_viewports.render_alternate_viewport = [this] {
        // ROM 0x2CB2 clears FFE102 before drawing the alternate viewport.
        field_camera_follow_.layer_y_dirty = false;
        render_alternate_field_viewport(
            viewport_state_,
            [this] { source_boundary("field.viewport.render_alternate"); });
    };
    callbacks_.upload_scene_profile_block = [this] {
        source_boundary("field.scene.upload_profile_block");
    };
    callbacks_.guard_world_effect_player_spawn = [this] {
        source_boundary("field.world.guard_player_spawn");
    };

    callbacks_.entity_behavior_frames.dispatch_entity_action_candidates =
        [this] {
            source_boundary("field.entities.dispatch_action_candidates");
            advance_default_field_actors();
        };
    callbacks_.entity_behavior_frames.noop_entity_action_hook =
        [](EntityBehaviorFrameRecord& record) { return record.action_result; };
    const auto observe_entity_boundary =
        [this](const std::string_view boundary,
               EntityBehaviorFrameRecord& record) {
            source_boundary(boundary);
            (void)record;
        };
    callbacks_.entity_behavior_frames.complete_entity_behavior_transition =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.complete_transition", record);
        };
    callbacks_.entity_behavior_frames.clear_effect_slot_state =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.clear_effect_slot", record);
        };
    callbacks_.entity_behavior_frames.handle_mode_action_event =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.handle_action_event", record);
        };
    callbacks_.entity_behavior_frames.advance_entity_behavior_action =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.advance_action", record);
        };
    callbacks_.entity_behavior_frames.enter_entity_behavior_transition =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.enter_transition", record);
        };
    callbacks_.entity_behavior_frames.dispatch_entity_collision_behavior =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.dispatch_collision", record);
        };
    callbacks_.entity_behavior_frames.apply_mode_five_boundary_motion =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.apply_boundary_motion", record);
        };
    callbacks_.entity_behavior_frames.resolve_entity_proximity_interactions =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.resolve_proximity", record);
        };
    callbacks_.entity_behavior_frames.clear_entity_effect_if_forced =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.clear_forced_effect", record);
        };
    callbacks_.entity_behavior_frames.resolve_entity_fallback_movement =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.resolve_fallback", record);
        };
    callbacks_.entity_behavior_frames.run_entity_continuation =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.run_continuation", record);
        };

    callbacks_.update_field_camera = [this] { update_source_field_camera(); };
    callbacks_.entity_render_pass.project_screen_position =
        [this](RenderEntityState& entity) {
            source_boundary("field.entities.project_screen_position");
            if (!field_entity_.active || entity.id != 0U) {
                field_entity_.visible = false;
                return;
            }
            field_entity_.projection.world_x =
                field_entity_.world_position.x;
            field_entity_.projection.world_y =
                field_entity_.world_position.y;
            const auto projection = project_entity_screen_position_standard(
                field_entity_.projection,
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_x),
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_y));
            field_entity_.visible = projection.visible;
        };
    callbacks_.entity_render_pass.advance_animation =
        [this](RenderEntityState& entity) {
            (void)entity;
            advance_field_actor_animation();
        };
    callbacks_.entity_render_pass.finish_entity =
        [this](RenderEntityState& entity) {
            source_boundary("field.entities.finish_entity");
            (void)entity;
        };
    callbacks_.entity_render_pass.finish_pass = [this] {
        source_boundary("field.entities.finish_render_pass");
        // ROM 0x04658 calls 0x5D29C at the render-pass tail. Keep the mode
        // projection current, advance Lua-owned actor animation first, then
        // service the semantic Scene-55 completion gate at that boundary.
        world_frame_logic_state_.scene_state =
            field_runtime_state_.scene_runtime_state;
        advance_default_field_actor_animations();
        advance_field_weapon_projectile_animations();
        advance_tarislar_tomb_outro();
    };
    callbacks_.wait_for_vblank = [this] {
        source_boundary("field.timing.wait_for_vblank");
    };
    callbacks_.advance_title_palette_fade = [this] {
        source_boundary("field.palette.advance_transition");
    };
    callbacks_.sample_controller_input = [this] {
        source_boundary("field.controller.sample");
    };
    configure_runtime_coordinator_callbacks();
    callbacks_.phase_changed = [this](
                                  const integration::FieldSessionBootstrapPhase phase) {
        source_boundary(std::string("field.bootstrap.phase.") +
                        integration::field_session_bootstrap_phase_name(phase));
    };
}

void FieldSceneRuntime::configure_runtime_coordinator_callbacks() {
    field_runtime_callbacks_.wait_for_vblank = [this] {
        source_boundary("field.runtime.wait_for_vblank");
    };
    field_runtime_callbacks_.prepare_mode_entry_service = [this] {
        // ROM 0x0976 enters the same six-command mode-entry audio service
        // used by field bootstrap. This is a converted leaf, not a marker for
        // an unresolved transition callback.
        prepare_audio_mode_entry(callbacks_.mode_entry_audio);
    };
    field_runtime_callbacks_.toggle_field_scene_variant = [this](
                                                                  auto& runtime) {
        FieldSceneVariantState variant{};
        variant.field_mode = runtime.field_mode;
        variant.scene_id = runtime.scene_id;
        variant.camera_x = state_.field_scene_buffers.camera_x;
        variant.pending_profile = field_pending_scene_profile_;
        const auto carry = toggle_field_scene_variant(variant);
        runtime.scene_id = variant.scene_id;
        state_.field_scene_buffers.scene_reference = variant.scene_id;
        field_pending_scene_profile_ = variant.pending_profile;
        if (carry) {
            source_boundary("field.runtime.scene_variant_selected");
        }
        return carry;
    };
    field_runtime_callbacks_.initialize_world_activity_mode =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            const auto& active_metadata =
                state_.field_scene_buffers.source_metadata;
            if (!active_metadata.available ||
                !field_session_source_assets_) {
                source_boundary("field.world_activity.source_scene_unavailable");
                runtime.field_transition_requested = true;
                return;
            }

            std::optional<std::uint16_t> selected_scene;
            std::optional<WorldActivityMode> routed_destination_mode;
            auto entry_resource_override =
                FieldLocationResourceEntryOverride::none;
            if (active_field_script_route_.has_value() &&
                active_field_script_route_->kind ==
                    FieldScriptRouteKind::area_event &&
                active_field_script_route_->event_code ==
                    field_movement_transition_state_.mode_event_code &&
                active_field_script_route_->area_event_target.has_value()) {
                selected_scene = static_cast<std::uint16_t>(
                    *active_field_script_route_->area_event_target);
                routed_destination_mode =
                    active_field_script_route_->area_event_destination_mode;
                entry_resource_override =
                    active_field_script_route_->entry_resource_override;
            }

            const auto run_target_actor_group =
                field_mode_entry_run_target_actor_group_;
            const bool descriptors_pending =
                run_target_actor_group.has_value() &&
                run_target_actor_group->activity_scene_id == 15U &&
                run_target_actor_group->actor_group_area != 0U &&
                run_target_actor_group->actor_group_event != 0U &&
                run_target_actor_group->actor_list_offset != 0U &&
                run_target_actor_group->actor_resource_profile_offset != 0U;
            if (run_target_actor_group.has_value() && !descriptors_pending) {
                source_boundary(
                    "field.world_activity.run_target_descriptor_facts_invalid");
                runtime.field_transition_requested = true;
                return;
            }
            active_field_script_route_.reset();
            if (!selected_scene.has_value()) {
                source_boundary(
                    "field.world_activity.building_route_missing");
                runtime.field_transition_requested = true;
                return;
            }

            WorldActivityRoutineInput input{};
            input.initial_scene_id = *selected_scene;
            input.routed_destination_mode = routed_destination_mode;
            input.mode_event_code =
                field_movement_transition_state_.mode_event_code;
            input.descriptors_pending = descriptors_pending;
            input.timeout_override = runtime.scene_runtime_state;
            input.world_flags = field_exit_world_flags_;
            input.contact_flags = notebook_contacts_flags_;
            switch (entry_resource_override) {
            case FieldLocationResourceEntryOverride::none:
                break;
            case FieldLocationResourceEntryOverride::index_five_marker_zero:
                input.resource_overrides.force_index_five_marker_zero = true;
                break;
            case FieldLocationResourceEntryOverride::index_two_marker_three:
                input.resource_overrides.force_index_two_marker_three = true;
                break;
            case FieldLocationResourceEntryOverride::index_four:
                input.resource_overrides.force_index_four = true;
                break;
            }
            input.primary_runner_marker =
                field_party_roster_.primary_compact_marker();
            input.secondary_runner_marker =
                field_party_roster_.secondary_compact_marker();
            populate_source_world_activity_route_tables(input);
            // 0x167AE clears FFF0C8 before the resource branch.
            input.default_resource_word = 0U;

            std::uint16_t preparation_scene = *selected_scene;
            if (input.descriptors_pending) {
                // ROM 0x16814 redirects the pending actor-list route to
                // scene 15 before preparing the field buffers.
                preparation_scene = 0x0FU;
            } else if (input.timeout_override != 0U) {
                preparation_scene = input.timeout_override == 1U
                                        ? 0x36U
                                        : 0x37U;
            }
            if (preparation_scene <
                field_session_source_assets_->field_scene_resources.size()) {
                const auto& resource =
                    field_session_source_assets_
                        ->field_scene_resources[preparation_scene];
                if (resource.has_value()) {
                    input.resource_random_index_bound =
                        resource->world_activity_random_index_bound;
                    input.resource_cursor_source =
                        resource->world_activity_resource_cursor;
                }
            }

            bool prepared = false;
            std::optional<scripting::ScriptFieldEncounterOutcome>
                run_target_actor_group_outcome;
            WorldActivityRoutineCallbacks activity_callbacks{};
            activity_callbacks.schedule_world_activity_timeout = [this] {
                (void)schedule_world_activity_timeout(
                    field_world_activity_timeout_state_);
                world_frame_logic_state_.world_active =
                    static_cast<std::uint16_t>(
                        field_world_activity_timeout_state_.timeout_marker);
                world_frame_logic_state_.cleanup_countdown =
                    static_cast<std::int16_t>(
                        field_world_activity_timeout_state_.timeout_frames);
                source_boundary("field.world_activity.schedule_timeout");
            };
            bool run_target_actor_group_resolved = false;
            activity_callbacks.publish_world_activity_descriptors =
                [this, &run_target_actor_group,
                 &run_target_actor_group_resolved,
                 &run_target_actor_group_outcome] {
                if (!run_target_actor_group.has_value()) {
                    source_boundary(
                        "field.world_activity.descriptor_publish_without_group");
                    return;
                }
                const auto outcome =
                    game_scripts_.resolve_run_target_actor_group(
                        run_target_actor_group->actor_group_area,
                        run_target_actor_group->actor_group_event);
                if (!outcome.has_value() ||
                    outcome->kind != scripting::
                        ScriptFieldEncounterOutcomeKind::
                            dynamic_world_effect_sequence ||
                    outcome->scene_id !=
                        run_target_actor_group->activity_scene_id ||
                    outcome->context_group !=
                        run_target_actor_group->actor_group_area ||
                    outcome->random_sum !=
                        run_target_actor_group->actor_group_event ||
                    outcome->actors.size() != 4U ||
                    std::any_of(
                        outcome->actors.begin(), outcome->actors.end(),
                        [&run_target_actor_group](const auto& actor) {
                            return actor.sequence_list_offset !=
                                run_target_actor_group->actor_list_offset;
                        })) {
                    failure_ = game_scripts_.failure().empty()
                        ? "ROM run-target actor descriptors do not match the selected list"
                        : game_scripts_.failure();
                    source_boundary(
                        "field.world_activity.run_target_descriptors_unresolved");
                    return;
                }
                run_target_actor_group_outcome = *outcome;
                run_target_actor_group_resolved = true;
                source_boundary("field.world_activity.publish_descriptors");
            };
            activity_callbacks.snapshot_scene_state = [this] {
                snapshot_world_activity_source_scene();
            };
            activity_callbacks.prepare_field_scene_buffers =
                [this, &prepared, preparation_scene] {
                    prepared = prepare_runtime_field_scene(preparation_scene);
                };
            activity_callbacks.upload_mode_word_block = [this] {
                source_boundary("field.world_activity.upload_mode_word_block");
            };
            activity_callbacks.sample_source_random_bound =
                [this](const std::uint16_t bound)
                    -> std::optional<std::uint16_t> {
                if (bound == 0U) {
                    return std::nullopt;
                }
                const auto sample = sample_source_random_bound(
                    field_random_seed_, bound);
                if (sample < 0 || sample >= static_cast<std::int16_t>(bound)) {
                    return std::nullopt;
                }
                return static_cast<std::uint16_t>(sample);
            };
            activity_callbacks.advance_source_prng = [this]()
                -> std::optional<std::uint16_t> {
                return advance_source_random_seed(field_random_seed_);
            };

            field_security_floor_index_.reset();
            const auto source_scene_id = runtime.scene_id;
            const auto result = run_world_activity_routine(
                input, world_activity_routine_state_, activity_callbacks);
            if (notebook_contacts_flags_.has_value() &&
                world_activity_routine_state_.contact_flags.has_value()) {
                notebook_contacts_flags_ =
                    world_activity_routine_state_.contact_flags;
            }
            runtime.scene_id = world_activity_routine_state_.scene_id;
            field_world_activity_scene_entry_pending_ =
                world_activity_routine_state_.scene_id != source_scene_id;
            runtime.field_scene_gate = static_cast<std::uint16_t>(
                world_activity_routine_state_.mode);
            if (result == WorldActivityResult::world_script_selected) {
                if (!world_activity_routine_state_.selected_world_script) {
                    failure_ =
                        "ROM selected a world actor script without its index";
                    source_boundary(
                        "field.world_activity.script_actor_index_missing");
                    runtime.field_transition_requested = true;
                } else {
                    auto actors = game_scripts_.world_activity_actors_for_script(
                        world_activity_routine_state_.scene_id,
                        static_cast<std::uint16_t>(
                            *world_activity_routine_state_
                                 .selected_world_script));
                    if (!actors.has_value() || actors->empty()) {
                        failure_ = game_scripts_.failure().empty()
                            ? "selected ROM world script has no extracted actor list"
                            : game_scripts_.failure();
                        source_boundary(
                            "field.world_activity.script_actor_list_unresolved");
                        runtime.field_transition_requested = true;
                    } else {
                        pending_world_activity_script_actor_scene_ =
                            world_activity_routine_state_.scene_id;
                        pending_world_activity_script_actors_ =
                            std::move(*actors);
                        source_boundary(
                            "field.world_activity.script_actor_list_queued");
                    }
                }
            }
            if (result == WorldActivityResult::resource_activity_ready) {
                field_security_floor_index_ = static_cast<std::uint8_t>(
                    world_activity_routine_state_
                        .selected_resource_random_index);
            }
            if (!prepared) {
                source_boundary("field.world_activity.scene_prepare_unresolved");
                runtime.field_transition_requested = true;
            }
            if (descriptors_pending && prepared &&
                run_target_actor_group_resolved &&
                run_target_actor_group_outcome.has_value()) {
                const auto expected_actor_count =
                    run_target_actor_group_outcome->actors.size();
                const auto previous_actor_count = field_default_actors_.size();
                pending_field_encounter_outcome_ =
                    std::move(run_target_actor_group_outcome);
                spawn_field_world_effect_sequence();
                if (field_default_actors_.size() - previous_actor_count !=
                    expected_actor_count) {
                    failure_ = "ROM run-target actor list was not fully published";
                    source_boundary(
                        "field.world_activity.run_target_actor_publish_incomplete");
                    runtime.field_transition_requested = true;
                }
            } else if (descriptors_pending) {
                source_boundary(
                    "field.world_activity.run_target_actor_publish_failed");
                runtime.field_transition_requested = true;
            }
            if (result == WorldActivityResult::resource_activity_bound_unavailable ||
                result ==
                    WorldActivityResult::resource_activity_random_input_unavailable ||
                result ==
                    WorldActivityResult::resource_activity_sample_out_of_range) {
                source_boundary("field.world_activity.resource_selection_unresolved");
                runtime.field_transition_requested = true;
            }
            if (result == WorldActivityResult::contact_flags_unavailable) {
                source_boundary(
                    "field.world_activity.scene_entry_contact_flags_unresolved");
            }
        };
    field_runtime_callbacks_.handle_world_activity_request =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            std::array<WorldActivityRequestSlot,
                       kWorldActivityRequestSlotCount> request_slots{};
            const auto& entities = state_.behavior_entities;
            for (std::size_t index = 0U;
                 index < request_slots.size() && index < entities.size(); ++index) {
                request_slots[index].flags_byte =
                    entities[index].active ? 0x01U : 0U;
                request_slots[index].route_flags = entities[index].route_flags;
            }
            const bool activity_started =
                ::shadowrun::ghidra_conversion::handle_world_activity_request(
                    world_activity_request_state_, request_slots,
                    world_activity_request_start_state_);
            if (activity_started) {
                runtime.world_activity_countdown =
                    world_activity_request_start_state_.timer_frames;
                begin_world_activity(
                    field_world_activity_state_,
                    static_cast<std::uint16_t>(
                        world_activity_request_start_state_.activity_pointer));
                source_boundary("field.world_activity.request_started");
            }
        };
    field_runtime_callbacks_.initialize_field_scene_state =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            bool has_authored_area_destination =
                active_field_script_route_.has_value() &&
                active_field_script_route_->kind ==
                    FieldScriptRouteKind::area_event &&
                active_field_script_route_->area_event_target.has_value();
            if (!has_authored_area_destination) {
                const auto& event_metadata =
                    state_.field_scene_buffers.source_metadata;
                const auto route = game_scripts_.resolve_field_location_route(
                    FieldScriptRouteContext{
                        static_cast<std::uint8_t>(
                            state_.field_scene_buffers.scene_reference),
                        static_cast<std::uint8_t>(runtime.field_mode),
                        field_collision_event_code_from_id(
                            field_movement_transition_state_.mode_event_code),
                        event_metadata.event_table_available,
                        event_metadata.event_mode_table,
                        field_exit_alternate_result_});
                if (route.has_value()) {
                    if (route->kind == FieldScriptRouteKind::area_event &&
                        route->area_event_target.has_value()) {
                        active_field_script_route_ = *route;
                        pending_field_return_anchor_ = route->return_anchor;
                        pending_field_return_facing_ = route->return_facing;
                        has_authored_area_destination = true;
                        source_boundary(
                            "field.entry.area_event_route_resolved_by_scene_state");
                    } else if (route->kind ==
                               FieldScriptRouteKind::source_activity_return) {
                        active_field_script_route_ = *route;
                        source_boundary(
                            "field.exit.resource_activity_return_selected");
                    } else if (route->kind ==
                               FieldScriptRouteKind::source_boundary) {
                        active_field_script_route_ = *route;
                        runtime.field_transition_requested = true;
                        source_boundary(route->boundary_reason);
                        return;
                    }
                }
            }
            if (has_authored_area_destination) {
                const auto source_scene =
                    state_.field_scene_buffers.scene_reference;
                runtime.scene_id = source_scene;
                runtime.field_mode =
                    state_.field_scene_buffers.field_resource_mode;
                runtime.scene_runtime_state = 0U;
                runtime.field_scene_gate = 0U;
                runtime.field_transition_requested = false;
                if (!field_runtime_callbacks_.initialize_world_activity_mode) {
                    runtime.field_transition_requested = true;
                    source_boundary(
                        "field.entry.area_event_initializer_unavailable");
                    return;
                }

                field_runtime_callbacks_.initialize_world_activity_mode(
                    runtime);
                if (runtime.field_transition_requested) {
                    source_boundary(
                        "field.entry.area_event_initialization_failed");
                    return;
                }
                const auto destination_scene = runtime.scene_id;
                runtime.scene_refresh_marker = -1;
                field_movement_transition_state_.scene_id =
                    static_cast<std::uint8_t>(destination_scene);
                world_frame_logic_state_.world_mode =
                    static_cast<std::uint8_t>(destination_scene);
                advance_field_scene_transition_after_modal();
                if (state_.field_scene_buffers.scene_reference !=
                    destination_scene) {
                    runtime.field_transition_requested = true;
                    source_boundary(
                        "field.entry.area_event_reentry_failed");
                    return;
                }
                field_mode_entry_run_target_actor_group_.reset();
                source_boundary("field.entry.area_event_scene_entered");
                return;
            }

            if (active_field_script_route_.has_value() &&
                active_field_script_route_->kind ==
                    FieldScriptRouteKind::source_activity_return) {
                if (!restore_world_activity_source_scene(runtime)) {
                    runtime.field_transition_requested = true;
                    source_boundary(
                        "field.exit.resource_activity_return_snapshot_missing");
                } else {
                    source_boundary(
                        "field.exit.resource_activity_return_applied");
                }
                return;
            }

            if (!restore_world_activity_source_scene(runtime)) {
                runtime.field_transition_requested = true;
            }
        };
    field_runtime_callbacks_.run_field_exit_orchestrator =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            if (field_opening_auto_walk_.active) {
                // The ROM clears FFE18B after the first field-exit
                // orchestrator pass, regardless of the selected event route.
                field_opening_auto_walk_.active = false;
                source_boundary("field.opening_auto_walk.consumed");
                if (game_scripts_.task_waiting_for_native_operation()) {
                    handle_game_script_task_step(
                        game_scripts_.complete_native_operation(true));
                }
            }
            const auto anchor_refresh_required =
                run_field_exit_orchestrator(runtime);
            source_boundary(anchor_refresh_required
                                ? "field.exit.coordinator_anchor_refresh_required"
                                : "field.exit.coordinator_continue_activity");
            return anchor_refresh_required;
        };
    field_runtime_callbacks_.advance_field_mode_transition =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            // Lua world_links owns the 0x2CE0 room graph (sole source); the
            // converted routine keeps only the mode 0x13 activity gate.
            const auto lua_exit = game_scripts_.call_integers(
                "world_links", "cave_exit_flat",
                {static_cast<std::int64_t>(runtime.transition_context),
                 static_cast<std::int64_t>(runtime.mode_event_code)});
            std::optional<FieldModeTransitionExit> exit;
            if (lua_exit.has_value() && lua_exit->size() == 4U &&
                (*lua_exit)[0] >= 0) {
                exit = FieldModeTransitionExit{
                    static_cast<std::int16_t>((*lua_exit)[0]),
                    static_cast<std::uint16_t>((*lua_exit)[1]),
                    static_cast<std::uint16_t>((*lua_exit)[2]),
                    static_cast<std::uint16_t>((*lua_exit)[3])};
            }
            const auto plan = advance_field_mode_transition(
                FieldModeTransitionInput{
                    runtime.mode_event_code,
                    runtime.transition_context,
                    runtime.world_activity_flags,
                    exit},
                FieldModeTransitionCallbacks{
                    [this] {
                        const auto continuation =
                            MovementModeTransitionContinuation::
                                mode_table_world_activity;
                        begin_world_activity(
                            field_world_activity_state_,
                            static_cast<std::uint16_t>(continuation));
                        world_frame_logic_state_.resource_countdown =
                            field_world_activity_state_.timeout;
                        field_runtime_state_.world_activity_countdown =
                            field_world_activity_state_.timeout;
                        record_field_activity_start(continuation);
                    },
                    [this] {
                        (void)schedule_world_activity_timeout(
                            field_world_activity_timeout_state_);
                        world_frame_logic_state_.world_active =
                            static_cast<std::uint16_t>(
                                field_world_activity_timeout_state_
                                    .timeout_marker);
                        world_frame_logic_state_.cleanup_countdown =
                            static_cast<std::int16_t>(
                                field_world_activity_timeout_state_
                                    .timeout_frames);
                        source_boundary(
                            "field.mode_transition.schedule_timeout");
                    }});
            if (!exit.has_value()) {
                source_boundary("field.mode_transition.lua_room_graph_missing");
            }
            if (!plan.resolved || !plan.spawn_position_available ||
                !field_session_source_assets_ ||
                plan.scene_id >=
                    field_session_source_assets_->field_scene_resources.size() ||
                !field_session_source_assets_
                     ->field_scene_resources[plan.scene_id]
                     .has_value()) {
                source_boundary("field.mode_transition.destination_unresolved");
                return false;
            }

            if (plan.start_world_activity &&
                !plan.pending_actor_list.has_value()) {
                source_boundary("field.mode_transition.actor_list_missing");
                return false;
            }
            source_boundary("field.mode_transition.lua_room_graph_selected");
            runtime.transition_context = plan.next_mode_index;
            runtime.scene_id = plan.scene_id;
            field_movement_transition_state_.field_transition_context =
                plan.next_mode_index;
            field_pending_mode_transition_scene_ = plan.scene_id;
            field_pending_mode_transition_spawn_ =
                SceneRunnerPosition{plan.override_x, plan.override_y};
            field_pending_mode_transition_actor_list_ =
                plan.pending_actor_list;
            field_world_activity_scene_entry_pending_ = true;
            source_boundary("field.mode_transition.destination_selected");
            return true;
        };
    field_runtime_callbacks_.reset_field_entity_transition_words =
        [this](integration::FieldRuntimeCoordinatorState&) {
            // The source writes its invalid-anchor sentinel to the three
            // actors' previous projection pairs. The native field presenter
            // has one visible controlled actor, so invalidate that projection
            // cache and rebuild it from the actor's typed world position.
            field_entity_.projection = EntityProjectionState{};
            field_entity_.visible = false;
            field_camera_alignment_ready_ = false;
            source_boundary(
                "field.entities.transition_projection_invalidated");
        };
    field_runtime_callbacks_.snapshot_entity_anchors =
        [this](integration::FieldRuntimeCoordinatorState&) {
            field_entity_.projection.world_x =
                field_entity_.world_position.x;
            field_entity_.projection.world_y =
                field_entity_.world_position.y;
            state_.field_scene_entry.camera_x = static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_x);
            state_.field_scene_entry.camera_y = static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_y);
            state_.field_scene_entry.camera_transition_x =
                static_cast<std::uint16_t>(
                    state_.camera_transition_x);
            state_.field_scene_entry.camera_transition_y =
                static_cast<std::uint16_t>(
                    state_.camera_transition_y);
            source_boundary("field.entities.transition_anchor_snapshotted");
        };
    field_runtime_callbacks_.begin_scene_37_world_activity =
        [this](integration::FieldRuntimeCoordinatorState& runtime) {
            if (runtime.scene_id != 0x37U) {
                runtime.field_transition_requested = true;
                source_boundary(
                    "field.world_activity.scene_55_callback_scene_mismatch");
                return;
            }
            const auto continuation =
                MovementModeTransitionContinuation::
                    scene_37_antechamber_entry;
            begin_world_activity(
                field_world_activity_state_,
                static_cast<std::uint16_t>(continuation));
            world_frame_logic_state_.resource_countdown =
                field_world_activity_state_.timeout;
            record_field_activity_start(continuation);
            source_boundary(
                "field.world_activity.scene_55_antechamber_callback_queued");
        };
    field_runtime_callbacks_.run_pending_field_scene_handler = [this] {
        const auto continuation = field_activity_continuation_;
        if (continuation != MovementModeTransitionContinuation::
                                mode_table_world_activity) {
            field_activity_continuation_ =
                MovementModeTransitionContinuation::none;
        }
        if (continuation == MovementModeTransitionContinuation::
                               mode_table_world_activity) {
            // ROM 0x2CE0 installs 0x53EE2 for the mode-table activity. That
            // handler submits the Renraku text-only descriptor at 0xF4C58
            // and profile 0x12206C to the shared 0x1F246 prompt service.
            const auto scene_id = static_cast<std::uint8_t>(
                state_.field_scene_buffers.scene_reference);
            game_scripts_.set_location_context(
                "site", "salish_feathered_serpent_lair",
                "salish.renraku_cave_ambush", "renraku_strike_team",
                "cave_ambush", scene_id);
            game_scripts_.set_field_mode_context(
                scripting::ScriptFieldModeContext{
                    state_.field_scene_buffers.field_resource_mode,
                    field_movement_transition_state_.mode_event_code,
                    field_exit_alternate_result_,
                    state_.field_scene_buffers.scene_mode,
                    field_movement_transition_state_.field_transition_context});
            const bool opened = field_dialogue_.open_gameplay_task(
                "salish_spirit_eyes", "present_renraku_ambush");
            field_activity_continuation_ =
                MovementModeTransitionContinuation::none;
            if (!opened) {
                std::string reason =
                    "field.activity.mode_table_handler_0x53ee2_prompt_failed";
                const auto& detail =
                    field_dialogue_.panel().source_boundary_reason;
                if (!detail.empty()) {
                    reason += ":";
                    reason.append(detail);
                }
                source_boundary(reason);
                return integration::PendingFieldSceneHandlerResult::resumed_field;
            } else {
                source_boundary(
                    "field.activity.mode_table_handler_0x53ee2_prompt_opened");
                return integration::PendingFieldSceneHandlerResult::resumed_field;
            }
        }
        if (continuation ==
            MovementModeTransitionContinuation::scene_37_antechamber_entry) {
            if (state_.field_scene_buffers.scene_reference != 55U) {
                source_boundary(
                    "field.activity.scene_55_antechamber_scene_mismatch");
                return integration::PendingFieldSceneHandlerResult::unresolved;
            }
            game_scripts_.set_location_context(
                "site", "tarislar_tomb_antechamber",
                "tarislar.tomb.antechamber", "tarislar_apartments_narrator",
                "antechamber_arrival", 55U);
            const bool opened = field_dialogue_.open_gameplay_task(
                "tarislar_apartments",
                "present_tomb_antechamber_arrival");
            if (!opened) {
                std::string reason =
                    "field.activity.scene_55_antechamber_prompt_failed";
                const auto& detail =
                    field_dialogue_.panel().source_boundary_reason;
                if (!detail.empty()) {
                    reason += ":";
                    reason.append(detail);
                }
                source_boundary(reason);
            } else {
                source_boundary(
                    "field.activity.scene_55_antechamber_prompt_opened");
            }
            return integration::PendingFieldSceneHandlerResult::resumed_field;
        }
        if (continuation == MovementModeTransitionContinuation::
                                scene_55_tomb_credits_callback) {
            if (state_.field_scene_buffers.scene_reference != 55U) {
                source_boundary(
                    "field.activity.scene55_tomb_credits_scene_mismatch");
                return integration::PendingFieldSceneHandlerResult::unresolved;
            }
            game_scripts_.set_location_context(
                "site", "tarislar_tomb_antechamber",
                "tarislar.tomb.ending_warning", "licourtrix",
                "tomb_ending_warning", 55U);
            const bool opened = field_dialogue_.open_gameplay_task(
                "tarislar_tomb_credits", "present_ending_warning");
            if (!opened) {
                std::string reason =
                    "field.activity.scene55_tomb_credits_warning_failed";
                const auto& detail =
                    field_dialogue_.panel().source_boundary_reason;
                if (!detail.empty()) {
                    reason += ":";
                    reason.append(detail);
                }
                source_boundary(reason);
                return integration::PendingFieldSceneHandlerResult::unresolved;
            }
            source_boundary(
                "field.activity.scene55_tomb_credits_warning_opened");
            return integration::PendingFieldSceneHandlerResult::resumed_field;
        }
        if (continuation ==
            MovementModeTransitionContinuation::event_30_world_activity) {
            const auto& event_metadata =
                state_.field_scene_buffers.source_metadata;
            const auto route = game_scripts_.resolve_field_location_route(
                FieldScriptRouteContext{
                    static_cast<std::uint8_t>(
                        state_.field_scene_buffers.scene_reference),
                    state_.field_scene_buffers.field_resource_mode,
                    FieldCollisionEventCode::event_30,
                    event_metadata.event_table_available,
                    event_metadata.event_mode_table,
                    field_exit_alternate_result_});
            if (!route.has_value()) {
                source_boundary(
                    "field.activity.event30_route_script_failed");
                return integration::PendingFieldSceneHandlerResult::resumed_field;
            }
            active_field_script_route_ = *route;
            if (!open_field_location_dialogue(*route)) {
                source_boundary(
                    "field.activity.event30_location_dialogue_failed");
            }
            return integration::PendingFieldSceneHandlerResult::resumed_field;
        }
        if (continuation ==
            MovementModeTransitionContinuation::event_31_world_activity) {
            const auto scene_reference =
                state_.field_scene_buffers.scene_reference;
            // ROM 0x0DB26 sends FFDF40==0 to the ordinary four-choice field
            // terminal menu. Nonzero resource-scene mode takes the separate
            // scene-specific branch at 0x0DBF8, which seeds the Matrix node
            // and system from the current resource scene.
            if (field_movement_transition_state_.world_mode == 0U) {
                game_scripts_.set_location_context(
                    "activity", "field_terminal", "field.main_menu");
                if (!field_dialogue_.open_gameplay_task(
                        "field_activities", "ordinary_field_terminal")) {
                    source_boundary(
                        "field.activity.ordinary_terminal_open_failed");
                } else {
                    source_boundary(
                        "field.activity.ordinary_terminal_opened");
                }
                return integration::PendingFieldSceneHandlerResult::resumed_field;
            }
            if (scene_reference < 21U || scene_reference > 43U) {
                source_boundary(
                    "field.activity.event31_scene_route_unresolved");
                return integration::PendingFieldSceneHandlerResult::resumed_field;
            }
            game_scripts_.set_location_context(
                "activity", "resource_scene", "field.activity", "", "",
                static_cast<std::uint8_t>(scene_reference));
            if (!field_dialogue_.open_gameplay_task(
                    "field_activities", "resource_scene_main_menu")) {
                source_boundary(
                    "field.activity.resource_main_menu_open_failed");
            } else {
                source_boundary(
                    "field.activity.resource_main_menu_opened");
            }
            return integration::PendingFieldSceneHandlerResult::resumed_field;
        }
        if (continuation ==
                MovementModeTransitionContinuation::event_27_world_activity ||
            continuation ==
                MovementModeTransitionContinuation::event_25_world_activity) {
            const auto& event_metadata =
                state_.field_scene_buffers.source_metadata;
            const auto event_code = continuation ==
                    MovementModeTransitionContinuation::event_27_world_activity
                ? FieldCollisionEventCode::event_27
                : FieldCollisionEventCode::event_25;
            const auto route = game_scripts_.resolve_field_location_route(
                FieldScriptRouteContext{
                    static_cast<std::uint8_t>(
                        state_.field_scene_buffers.scene_reference),
                    state_.field_scene_buffers.field_resource_mode,
                    event_code,
                    event_metadata.event_table_available,
                    event_metadata.event_mode_table,
                    field_exit_alternate_result_});
            if (!route.has_value()) {
                source_boundary(continuation ==
                        MovementModeTransitionContinuation::event_27_world_activity
                    ? "field.activity.event27_route_script_failed"
                    : "field.activity.event25_route_script_failed");
                return integration::PendingFieldSceneHandlerResult::resumed_field;
            }
            active_field_script_route_ = *route;
            if (!open_field_location_dialogue(*route)) {
                source_boundary(continuation ==
                        MovementModeTransitionContinuation::event_27_world_activity
                    ? "field.activity.event27_location_dialogue_failed"
                    : "field.activity.event25_location_dialogue_failed");
            }
            return integration::PendingFieldSceneHandlerResult::resumed_field;
        }
        if (continuation ==
            MovementModeTransitionContinuation::event_29_world_activity) {
            if (!open_field_elevator_selector(
                    FieldCollisionEventCode::event_29)) {
                source_boundary("field.elevator.selector_open_unresolved");
            }
            return integration::PendingFieldSceneHandlerResult::resumed_field;
        }
        const bool opened = field_interaction_.open(
            continuation,
            state_.field_scene_buffers.scene_reference,
            field_movement_transition_state_.world_mode,
            field_movement_transition_state_.terrain_event_suppressed,
            field_movement_transition_state_.activity_selection,
            field_security_floor_index_,
            &field_world_activity_timeout_state_);
        if (!opened) {
            source_boundary("field.interaction.open_failed");
        } else {
            source_boundary("field.interaction.prompt_opened");
        }
        return integration::PendingFieldSceneHandlerResult::resumed_field;
    };
    field_runtime_callbacks_.latch_controller_events = [this] {
        source_boundary("field.runtime.latch_controller_events");
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.confirm_latch =
            field_runtime_state_.confirm_latch;
        field_controller_latch_state_.start_latch =
            field_runtime_state_.start_latch;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        field_runtime_state_.confirm_latch =
            field_controller_latch_state_.confirm_latch;
        field_runtime_state_.start_latch =
            field_controller_latch_state_.start_latch;
    };
    field_runtime_callbacks_.sample_controller_input = [this] {
        source_boundary("field.runtime.sample_controller_input");
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        field_runtime_state_.controller_held =
            field_controller_sample_state_.held;
        field_runtime_state_.controller_heading =
            field_controller_sample_state_.heading;
        return field_controller_sample_state_.events;
    };
    field_runtime_callbacks_.service_entity_behavior_frames = [this] {
        advance_player_weapon_action_gate();
        advance_field_weapon_projectiles();
        advance_field_movement();
        advance_default_field_actors();
    };
    field_runtime_callbacks_.update_field_camera = [this] {
        update_source_field_camera();
    };
    field_runtime_callbacks_.update_entity_render_pass = [this] {
        update_entity_render_pass(state_.render_entities,
                                  callbacks_.entity_render_pass);
        service_pending_field_viewports();
    };
    field_runtime_callbacks_.advance_palette_fade = [this] {
        // ROM 0x16396 services effects on every field frame. When 0xB7B8
        // projected an interaction, FFDF3A/FFDF3C hold the fade gate and its
        // signed frame countdown; expiration publishes FFDF3E for the next
        // coordinator pass.
        PaletteFadeState fade{};
        fade.active =
            field_movement_transition_state_.interaction_transition_pending;
        fade.remaining =
            field_movement_transition_state_.interaction_transition_timer;
        ::shadowrun::ghidra_conversion::advance_palette_fade(
            fade,
            {[this] {
                 source_boundary("field.runtime.service_active_effects");
             },
             [this] {
                 if (field_palette_fade_steps_ < 9U) {
                     ++field_palette_fade_steps_;
                 }
                 source_boundary("field.runtime.step_palette_toward_black");
             }});
        if (fade.dirty) {
            state_.display_dirty_flags = static_cast<std::uint8_t>(
                state_.display_dirty_flags | 0x01U);
            field_runtime_state_.display_dirty_flags =
                static_cast<std::uint8_t>(
                    field_runtime_state_.display_dirty_flags | 0x01U);
        }
        if (field_movement_transition_state_.interaction_transition_pending) {
            field_movement_transition_state_.interaction_transition_timer =
                fade.remaining;
            field_movement_transition_state_.world_interaction_lock =
                fade.active;
        }
        if (fade.completed) {
            field_movement_transition_state_.interaction_transition_pending =
                false;
            field_movement_transition_state_.interaction_transition_timer =
                fade.remaining;
            field_movement_transition_state_.world_interaction_lock = false;
            field_runtime_state_.field_transition_requested = true;
            source_boundary("field.runtime.field_transition_requested");
        }
    };
    field_runtime_callbacks_.advance_world_frame_logic = [this] {
        // FFDF84/FFDF86 are shared by the alert/timeout helper (0x1DBAA)
        // and the per-frame dispatcher (0x1DE00). Keep one native field state
        // authoritative while adapting its words to the world-frame model.
        // The scene-mode byte is shared as well: 0x097C promotes FFE186 to
        // mode 2 before re-entry, and 0x1DD94 may promote it to mode 3 here.
        world_frame_logic_state_.scene_state =
            field_runtime_state_.scene_runtime_state;
        world_frame_logic_state_.world_active = static_cast<std::uint16_t>(
            field_world_activity_timeout_state_.timeout_marker);
        world_frame_logic_state_.cleanup_countdown = static_cast<std::int16_t>(
            field_world_activity_timeout_state_.timeout_frames);
        const auto current_scene =
            state_.field_scene_buffers.scene_reference;
        const auto slot_three_actor = std::find_if(
            field_default_actors_.begin(), field_default_actors_.end(),
            [current_scene](const auto& actor) {
                return actor.scene_reference == current_scene &&
                       actor.source_record_slot ==
                           FieldWorldActorSlot::slot_3;
            });
        if (slot_three_actor != field_default_actors_.end()) {
            // ROM 0x1DD94 compares FF044C, the +0x4C meter in the slot-3
            // record at FF0400, before promoting FFE186 from 2 to 3.
            world_frame_logic_state_.region_or_mode =
                static_cast<std::int8_t>(slot_three_actor->physical_meter);
        } else if (world_frame_logic_state_.scene_state == 2U &&
                   world_frame_logic_state_.transition_marker < 0) {
            // The record's meter is a required source fact; keep the mode
            // transition at its source boundary when this typed record is not
            // available instead of letting the default zero satisfy it.
            if (world_frame_logic_state_.region_or_mode != 10) {
                source_boundary(
                    "field.world_logic.slot_three_physical_meter_unresolved");
            }
            world_frame_logic_state_.region_or_mode = 10;
        }
        const auto scene_state_before = world_frame_logic_state_.scene_state;
        const auto transition_marker_before =
            world_frame_logic_state_.transition_marker;
        const auto region_or_mode_before =
            world_frame_logic_state_.region_or_mode;
        const bool scene_55_outro_arm_requested =
            current_scene == 55U && scene_state_before == 2U &&
            transition_marker_before < 0 && region_or_mode_before <= 9;
        refresh_world_entity_service_targets();
        advance_world_frame_logic(world_frame_logic_state_,
                                  world_frame_logic_callbacks_);
        if (scene_55_outro_arm_requested &&
            world_frame_logic_state_.scene_state == 3U) {
            const auto armed = game_scripts_.advance_tarislar_tomb_outro(
                current_scene, world_frame_logic_state_.scene_state,
                std::array<std::string, 6>{}, true);
            if (!armed.has_value() ||
                *armed != scripting::ScriptTarislarTombOutroStep::armed) {
                failure_ = game_scripts_.failure();
                source_boundary("field.scene55.tomb_outro_arm_failed");
            } else {
                source_boundary("field.scene55.tomb_outro_armed");
            }
        }
        field_runtime_state_.scene_runtime_state =
            world_frame_logic_state_.scene_state;
        // The frame service decrements FFE11C before the coordinator checks
        // whether it may run the pending scene handler. Publish that updated
        // value now; copying the pre-service value back at frame end would
        // re-arm the same countdown forever.
        field_runtime_state_.world_activity_countdown =
            world_frame_logic_state_.activity_countdown;
        ++field_world_frame_counter_;
        field_world_activity_timeout_state_.timeout_marker =
            static_cast<std::int16_t>(world_frame_logic_state_.world_active);
        field_world_activity_timeout_state_.timeout_frames =
            static_cast<std::uint16_t>(
                world_frame_logic_state_.cleanup_countdown);
        // ROM 0x1DDF0 decrements FFE11C every field frame. The native activity
        // gate shares that typed countdown so 0x1E2E0 can accept a later
        // doorway or terminal after the prior activity expires.
        field_world_activity_state_.timeout = static_cast<std::uint8_t>(
            world_frame_logic_state_.resource_countdown);
    };
    field_runtime_callbacks_.handle_mode_action_event = [this] {
        if (source_audio_event_callback_) {
            // ROM 0x391A requests event 0x25 on the field C-button runner
            // selection path.
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
        }
        handle_field_mode_action_event();
    };
    field_runtime_callbacks_.run_fixed_audio_transaction =
        [this](const std::uint16_t command) {
            // ROM 0xB62/0xB66: 0x5315E(8), op 0x20, ducks the game while the
            // Start menu 0x10D30 runs [C].
            source_boundary("field.runtime.command_audio_transaction");
            start_menu_duck_active_ = true;
            request_audio_attenuation(static_cast<std::uint8_t>(command));
        };
    field_runtime_callbacks_.run_command_overview = [this] {
        open_command_overview();
        if (command_overview_.open()) {
            // The callback runs after this host frame's ordinary menu service
            // point, so the opening frame itself is the first black frame.
            command_overview_start_transition_frames_remaining_ = 26U;
            command_overview_start_transition_black_frame_ = true;
        }
    };
    field_runtime_callbacks_.service_audio_tick = [this] {
        source_boundary("field.runtime.command_audio_tick");
        // ROM 0xB7A restores level 0 when 0x10D30 returns. The host menu is
        // non-blocking: restore now only if it did not stay open.
        if (!menu_route_active_) {
            restore_start_menu_duck();
        }
    };
    field_runtime_callbacks_.select_next_entity = [this] {
        source_boundary("field.runtime.select_next_entity");
    };
    field_runtime_callbacks_.restart_field_session = [this] {
        source_boundary("field.runtime.restart_field_session");
    };
    field_runtime_callbacks_.advance_field_scene_transition = [this] {
        if (command_overview_.open() || field_interaction_.open() ||
            field_dialogue_.open() || field_elevator_selector_.is_open()) {
            // The source routine returns from the blocking command loop before
            // the field transition leaf can run. The Qt bridge keeps that
            // ordering explicit while the menu owns the next host frames.
            source_boundary(
                field_dialogue_.open()
                    ? "field.runtime.scene_transition_suspended_for_dialogue"
                    : field_elevator_selector_.is_open()
                          ? "field.runtime.scene_transition_suspended_for_elevator"
                          : field_interaction_.open()
                                ? "field.runtime.scene_transition_suspended_for_interaction"
                                : "field.runtime.scene_transition_suspended_for_command_overview");
            return;
        }
        advance_field_scene_transition_after_modal();
    };

    world_frame_logic_callbacks_.prepare_mode_entry = [this] {
        source_boundary("field.world_logic.prepare_mode_entry");
    };
    world_frame_logic_callbacks_.begin_world_activity = [this] {
        source_boundary("field.world_logic.begin_world_activity");
    };
    world_frame_logic_callbacks_.select_entity_service_target = [this] {
        // Keep the observation boundary: QA (scene-20 Electronics alert)
        // counts it to prove the world pass requested a target selection.
        source_boundary("field.world_logic.select_entity_service_target");
        refresh_world_entity_service_targets();
        const auto action = select_world_entity_service_target(
            field_entity_target_owner_, field_entity_target_records_, false);
        for (auto& actor : field_default_actors_) {
            if (!actor.source_record_slot.has_value()) {
                continue;
            }
            const auto slot = static_cast<std::size_t>(
                *actor.source_record_slot);
            actor.target_selected =
                    action == EntityServiceTargetAction::selected &&
                field_entity_target_owner_.selected_target ==
                    std::optional<std::size_t>{slot};
            if (actor.target_selected) {
                actor.source_interaction_flags = static_cast<std::uint8_t>(
                    actor.source_interaction_flags |
                    actor_flag_mask(ActorInteractionFlag::service_selection_latch));
            } else {
                actor.source_interaction_flags = static_cast<std::uint8_t>(
                    actor.source_interaction_flags & static_cast<std::uint8_t>(
                        ~actor_flag_mask(ActorInteractionFlag::service_selection_latch)));
            }
        }
        refresh_world_entity_service_targets();
        source_boundary(action == EntityServiceTargetAction::selected
                            ? "field.world_logic.target_selected"
                            : "field.world_logic.no_target_available");
    };
    world_frame_logic_callbacks_.reset_entity_service_records = [this] {
        reset_field_entity_service_records();
        source_boundary("field.world_logic.reset_entity_service_records");
    };
    world_frame_logic_callbacks_.clear_inactive_entity_effects = [this] {
        source_boundary("field.world_logic.clear_inactive_entity_effects");
    };
    world_frame_logic_callbacks_.process_terrain_work_batch = [this] {
        source_boundary("field.world_logic.process_terrain_work_batch");
    };
    world_frame_logic_callbacks_.advance_world_effect_trigger = [this] {
        source_boundary("field.world_logic.advance_world_effect_trigger");
    };
    world_frame_logic_callbacks_.trigger_world_effect_sequence = [this] {
        trigger_field_world_effect_sequence();
    };
    world_frame_logic_callbacks_.resolve_nearby_effects = [this] {
        source_boundary("field.world_logic.resolve_nearby_effects");
    };
}

void FieldSceneRuntime::install_test_deck() {
    // ROM 0x59B7E sets FF018F bit 0, FFFC28, and these FFFBBE fields.
    cyberdeck_available_ = true;
    if (field_runner_feature_flags_.has_value()) {
        *field_runner_feature_flags_ |= 0x01U;
    }
    field_matrix_passcode_flags_ = 0x0FFEU;

    auto deck = field_matrix_deck_record_.value_or(CyberdeckRecord{});
    deck.mpcp = 6U;
    deck.hardening = 1U;
    // ROM 0x59B7E writes FFFBBE+2=0x00C8, +4=0x03E8, +6=0x32,
    // +7=1, and +0x0D=0xFF. It then writes rating 4 to each of the twelve
    // program bytes at +0x0E..+0x19.
    deck.memory_megapulses = 0x00C8U;
    deck.storage_megapulses = 1000U;
    deck.load_io_speed = 0x32U;
    deck.response = 1U;
    deck.model_index = static_cast<std::int8_t>(-1);
    std::array<std::uint8_t, 12> program_ratings{};
    program_ratings.fill(4U);
    deck.program_ratings = program_ratings;
    // The ROM routine leaves persona, loaded programs, slot animation state,
    // and stored files untouched. Preserve those existing typed fields.
    field_matrix_deck_record_ = std::move(deck);
}

void FieldSceneRuntime::configure_field_menu_route() {
    scripting::ScriptMenuRouteBindings bindings;
    bindings.host.play_sound = [this](const std::uint16_t sound) {
        if (source_audio_event_callback_) {
            source_audio_event_callback_(sound);
        }
    };
    bindings.host.stop_sound = [this](const std::uint16_t sound) {
        if (source_audio_stop_callback_) {
            source_audio_stop_callback_(sound);
        }
    };
    // FieldSceneRuntime repaints the complete authored Qt menu panel each
    // frame. These callbacks are retained as presenter boundaries for the
    // later screen ports; they do not execute source tile/VDP operations.
    bindings.host.redraw = [](const std::uint32_t) {};
    bindings.host.blank = [](const std::uint16_t) {};
    bindings.host.set_window = [](const std::uint16_t) {};
    bindings.host.set_screen = [this](const std::uint16_t screen) {
        const auto matrix_entry_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::MATRIX_ENTRY);
        const auto matrix_search_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::MATRIX_SEARCH);
        const auto matrix_node_action_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::MATRIX_NODE_ACTION);
        const auto command_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::COMMAND);
        const auto debug_pick_system_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::DEBUG_PICK_SYSTEM);
        if (screen == matrix_entry_screen ||
            screen == matrix_search_screen ||
            screen == matrix_node_action_screen) {
            // A Matrix route may begin as a child of Pocket Secretary's
            // developer Pick System flow. Mark the field compositor as
            // Matrix-owned when that child enters, not only when Matrix
            // Entry is the root menu screen. The flag stays set through the
            // first Cyberdeck and node-action children until the route exits.
            field_matrix_route_active_ = true;
        } else if (screen == command_screen ||
                   screen == debug_pick_system_screen) {
            // Pull Plug can rebuild the active Command route instead of
            // closing the menu tree. Do not leave the field compositor in
            // the Matrix state after that rebuild.
            field_matrix_route_active_ = false;
        }
        const auto is_secretary_tree_screen = [](const std::uint16_t value) {
            switch (static_cast<srx::MenuScreen>(value)) {
            case srx::MenuScreen::POCKET_SECRETARY:
            case srx::MenuScreen::NOTEBOOK:
            case srx::MenuScreen::NOTEBOOK_TIPS:
            case srx::MenuScreen::MR_JOHNSONS:
            case srx::MenuScreen::NOTEBOOK_CONTACTS:
            case srx::MenuScreen::NOTEBOOK_SHADOWRUNNERS:
            case srx::MenuScreen::NOTEBOOK_CURRENT_RUN:
            case srx::MenuScreen::MESSAGE:
            case srx::MenuScreen::DISMISS_RUNNER:
            case srx::MenuScreen::SAVE_LOAD:
            case srx::MenuScreen::SAVE_SLOT:
            case srx::MenuScreen::GROUP_ITEMS:
            case srx::MenuScreen::CYBERDECK:
            case srx::MenuScreen::PULL_PLUG:
            case srx::MenuScreen::DECK_PROGRAM:
            case srx::MenuScreen::DECK_CYBERINFO:
            case srx::MenuScreen::DECK_STORAGE:
            case srx::MenuScreen::DECK_MAP:
            case srx::MenuScreen::DEBUG_MENU:
            case srx::MenuScreen::DEBUG_PICK_SYSTEM:
                return true;
            default:
                return false;
            }
        };
        const auto pocket_secretary_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::POCKET_SECRETARY);
        if (screen == pocket_secretary_screen &&
            !pocket_secretary_.open) {
            begin_pocket_secretary();
        } else if (screen == static_cast<std::uint16_t>(
                                srx::MenuScreen::DISMISS_RUNNER) &&
                   menu_route_screen_ != screen) {
            begin_pocket_secretary_dismiss_runner();
        } else if (menu_route_screen_ == static_cast<std::uint16_t>(
                                           srx::MenuScreen::DISMISS_RUNNER) &&
                   screen != static_cast<std::uint16_t>(
                                 srx::MenuScreen::DISMISS_RUNNER)) {
            dismiss_runner_ = DismissRunnerMenuState{};
        } else if (screen == static_cast<std::uint16_t>(
                                srx::MenuScreen::GROUP_ITEMS) &&
                   menu_route_screen_ != screen) {
            if (group_item_flags_.has_value()) {
                begin_pocket_secretary_group_items(
                    pocket_secretary_, *group_item_flags_);
            } else {
                source_boundary(
                    "field.pocket_secretary.group_items_flags_unresolved");
            }
        } else if (menu_route_screen_ == static_cast<std::uint16_t>(
                                           srx::MenuScreen::GROUP_ITEMS) &&
                   screen != static_cast<std::uint16_t>(
                                 srx::MenuScreen::GROUP_ITEMS)) {
            pocket_secretary_.group_items_wait = false;
        } else if (is_secretary_tree_screen(menu_route_screen_) &&
                   !is_secretary_tree_screen(screen)) {
            pocket_secretary_.open = false;
            dismiss_runner_ = DismissRunnerMenuState{};
        }

        const auto attributes_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::ATTRIBUTES_SKILLS);
        const auto cyberware_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::CYBERWARE);
        const auto was_attributes =
            menu_route_screen_ == attributes_screen;
        const auto is_attributes = screen == attributes_screen;
        const auto was_cyberware = menu_route_screen_ == cyberware_screen;
        const auto is_cyberware = screen == cyberware_screen;
        if (is_attributes && !was_attributes) {
            begin_command_overview_attributes_skills();
        } else if (was_attributes && !is_attributes) {
            command_overview_attributes_skills_.close_to_parent();
        }
        if (is_cyberware && !was_cyberware) {
            begin_command_overview_cyberware();
        } else if (was_cyberware && !is_cyberware) {
            command_overview_cyberware_active_ = false;
        }
        menu_route_screen_ = screen;
        posture_dialog_active_ =
            screen == static_cast<std::uint16_t>(srx::MenuScreen::POSTURE);
    };
    bindings.host.menu_closed = [this](
                                    const bool matrix_logout_transition_complete) {
        matrix_field_return_transition_pending_ =
            matrix_return_to_field_on_close_ &&
            !matrix_logout_transition_complete;
        matrix_return_to_field_on_close_ = false;
        if (matrix_logout_transition_complete) {
            // The Lua route already spent the ROM's 10-frame fade and
            // 54-frame display transition. Consume its confirm input, but
            // do not layer the host's separate 60-frame modal fade on top.
            field_modal_return_transition_frames_remaining_ = 0U;
            field_palette_fade_steps_ = 0U;
            suppress_field_modal_close_input();
            field_entity_animation_.redraw_requested = true;
        }
        field_matrix_route_active_ = false;
        if (field_taxi_route_active_) {
            field_taxi_route_active_ = false;
            if (field_taxi_pending_destination_.has_value()) {
                if (*field_taxi_pending_destination_ ==
                    state_.field_scene_buffers.scene_reference) {
                    field_taxi_pending_destination_.reset();
                    source_boundary("field.taxi.returned_to_current_region");
                } else if (field_elevator_scene_transition_.begin_fade_out()) {
                    field_taxi_transition_active_ = true;
                    source_boundary("field.taxi.destination_fade_out_started");
                } else {
                    field_taxi_pending_destination_.reset();
                    source_boundary("field.taxi.destination_transition_busy");
                }
            }
        }
        menu_route_active_ = false;
        menu_route_screen_ = 0U;
        restore_start_menu_duck();
        posture_dialog_active_ = false;
        command_overview_inventory_lua_route_active_ = false;
        command_overview_inventory_action_active_ = false;
        command_overview_discard_confirmation_active_ = false;
        command_overview_inventory_trade_ = InventoryTradeState{};
        command_overview_first_aid_state_ =
            CommandOverviewFirstAidState::inactive;
        command_overview_automatic_use_state_ =
            CommandOverviewAutomaticUseState::inactive;
        command_overview_action_message_ =
            CommandOverviewActionMessage::none;
        command_overview_cyberware_active_ = false;
        command_overview_attributes_skills_.close_to_parent();
        command_overview_clips_.close();
        command_overview_.close();
        pocket_secretary_.open = false;
        dismiss_runner_ = DismissRunnerMenuState{};
    };
    bindings.host.legacy_open =
        [this](const std::uint16_t screen,
               const scripting::ScriptMenuRouteArgs& args) {
            start_command_menu_legacy_screen(screen, args);
        };
    bindings.host.legacy_advance =
        [this](const std::uint16_t screen, const std::uint16_t) {
            return std::pair<bool, bool>{
                command_menu_legacy_screen_active(screen), false};
        };
    bindings.host.notebook_boundary =
        [this](const scripting::ScriptNotebookBoundary boundary) {
            switch (boundary) {
            case scripting::ScriptNotebookBoundary::event_gates_unresolved:
                source_boundary(
                    "field.pocket_secretary.notebook.event_flags_unresolved");
                break;
            case scripting::ScriptNotebookBoundary::tips_flags_unresolved:
                source_boundary(
                    "field.pocket_secretary.notebook.tips.flags_unresolved");
                break;
            case scripting::ScriptNotebookBoundary::current_run_unresolved:
                source_boundary(
                    "field.pocket_secretary.notebook.current_run.path_pending");
                break;
            case scripting::ScriptNotebookBoundary::contacts_flags_unresolved:
                source_boundary(
                    "field.pocket_secretary.notebook.contacts.flags_unresolved");
                break;
            case scripting::ScriptNotebookBoundary::shadowrunners_flags_unresolved:
                source_boundary(
                    "field.pocket_secretary.notebook.shadowrunners.flags_unresolved");
                break;
            case scripting::ScriptNotebookBoundary::
                mr_johnsons_flags_unresolved:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.flags_unresolved");
                break;
            }
        };
    bindings.game.reselect_leader = [this] {
        handle_field_mode_action_event();
        refresh_field_hud_runner_slot();
        source_boundary("field.command_overview.lua_reselected_runner");
    };
    bindings.game.leader_posture = [this] {
        return static_cast<int>(active_runner_record().posture);
    };
    bindings.game.set_leader_posture = [this](const int posture) {
        const auto clamped = static_cast<std::uint8_t>(
            std::clamp(posture, 0, 4));
        const auto current = active_runner_record().posture;
        const auto delta = static_cast<std::int8_t>(
            static_cast<int>(clamped) - static_cast<int>(current));
        if (delta != 0) {
            (void)adjust_runner_posture(active_runner_record(), delta);
            refresh_command_overview_combat_bars();
            source_boundary("field.posture.lua_adjusted");
        }
    };
    bindings.game.leader_archetype = [this] {
        return static_cast<int>(active_runner_record().archetype_index);
    };
    bindings.game.party = [this] {
        std::array<scripting::ScriptMenuPartyMember,
                   FieldPartyRosterRuntime::kSlotCount> party{};
        const auto& records = field_party_roster_.records();
        for (std::size_t slot = 0; slot < records.size(); ++slot) {
            const auto& record = records[slot];
            auto& member = party[slot];
            member.status = static_cast<std::uint8_t>(record.flags >> 8U);
            member.clips = record.ammo_clips;
            member.rounds = record.ammo_rounds;
            member.weapon_slot = record.equipped_inventory_slot;
            member.weapon_code = record.equipped_item_code;
            member.armor = record.equipped_armor_index;
            member.physical = record.delay_meter;
            member.stun = record.behavior_meter;
            member.magic_resource = record.auxiliary[0];
            member.archetype = record.archetype_index;
            member.karma = record.auxiliary[2];
            member.magic_force = record.magic_force;
            member.posture = record.posture;
            member.magic_table_index = record.magic_table_index;
            member.state_function = record.first_aid_continuation;
            member.inventory = record.inventory_item_codes;
            member.phase = record.inventory_phase_bytes;
            member.attributes = record.attributes;
            member.skills = record.skills;
        }
        return party;
    };
    bindings.game.spell_table = [this](const int table_index)
        -> std::optional<scripting::ScriptMenuSpellTable> {
        if (table_index < 0 || table_index > 0xFFFF) {
            return std::nullopt;
        }
        const auto* source_table = field_magic_spell_table(
            static_cast<std::uint16_t>(table_index));
        if (source_table == nullptr ||
            source_table->entry_count >
                scripting::ScriptMenuSpellTable{}.entries.size()) {
            return std::nullopt;
        }
        scripting::ScriptMenuSpellTable result;
        result.entry_count = source_table->entry_count;
        for (std::size_t index = 0; index < result.entry_count; ++index) {
            const auto& source_entry = source_table->entries[index];
            const auto* spell = find_us_magic_spell(source_entry.spell_code);
            if (spell == nullptr) {
                return std::nullopt;
            }
            result.entries[index] = {
                source_entry.spell_code,
                source_entry.force_limit,
                spell->availability_byte,
                spell->difficulty_modifier,
                spell->base_effect};
        }
        return result;
    };
    bindings.game.set_spell_table = [this](
        const int table_index,
        const scripting::ScriptMenuSpellTable& script_table) {
        if (table_index < 0 ||
            static_cast<std::size_t>(table_index) >=
                field_magic_spell_table_overrides_.size() ||
            script_table.entry_count > kMagicSpellTableEntryCapacity) {
            return;
        }
        MagicSpellTable updated{};
        updated.entry_count = script_table.entry_count;
        for (std::size_t index = 0; index < updated.entry_count; ++index) {
            const auto& source = script_table.entries[index];
            if (find_us_magic_spell(source.code) == nullptr) {
                return;
            }
            updated.entries[index] = {source.code, source.force_limit};
        }
        field_magic_spell_table_overrides_[
            static_cast<std::size_t>(table_index)] = updated;
    };
    bindings.game.active_slot = [this] {
        return static_cast<int>(field_party_roster_.active_slot());
    };
    bindings.game.magazine_capacity = [this](const int weapon_code) {
        if (weapon_code < 0 || weapon_code > 0xFF) {
            return 0;
        }
        return static_cast<int>(weapon_magazine_capacity_for_item(
                                    static_cast<std::uint8_t>(weapon_code))
                                    .value_or(0U));
    };
    bindings.game.set_runner_ammo = [this](const int slot,
                                           const int clips,
                                           const int rounds) {
        if (slot < 0 ||
            static_cast<std::size_t>(slot) >=
                FieldPartyRosterRuntime::kSlotCount) {
            return;
        }
        auto& record = field_party_roster_.records()[
            static_cast<std::size_t>(slot)];
        record.ammo_clips = static_cast<std::uint8_t>(
            std::clamp(clips, 0, 0xFF));
        record.ammo_rounds = static_cast<std::uint8_t>(
            std::clamp(rounds, 0, 0xFF));
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
    };
    bindings.game.slot_active_set = [this] {
        std::array<bool, FieldPartyRosterRuntime::kSlotCount> active{};
        for (std::size_t slot = 0; slot < active.size(); ++slot) {
            active[slot] = field_party_roster_.slot_active(slot);
        }
        return active;
    };
    bindings.game.commit_party = [this](
        const std::array<scripting::ScriptMenuPartyMember,
                         FieldPartyRosterRuntime::kSlotCount>& party) {
        auto& records = field_party_roster_.records();
        for (std::size_t slot = 0; slot < records.size(); ++slot) {
            auto& record = records[slot];
            const auto& member = party[slot];
            record.flags = static_cast<std::uint16_t>(
                (record.flags & 0x00FFU) |
                (static_cast<std::uint16_t>(member.status) << 8U));
            record.ammo_clips = member.clips;
            record.ammo_rounds = member.rounds;
            record.equipped_inventory_slot = static_cast<std::int8_t>(
                std::clamp(member.weapon_slot, -128, 127));
            record.equipped_item_code = member.weapon_code;
            record.equipped_armor_index = member.armor;
            record.delay_meter = member.physical;
            record.behavior_meter = member.stun;
            record.auxiliary[0] = member.magic_resource;
            record.auxiliary[2] = member.karma;
            record.archetype_index = member.archetype;
            record.magic_force = member.magic_force;
            record.posture = member.posture;
            record.magic_table_index = member.magic_table_index;
            record.first_aid_continuation = member.state_function;
            record.inventory_item_codes = member.inventory;
            record.inventory_phase_bytes = member.phase;
            record.attributes = member.attributes;
            record.skills = member.skills;
        }
        refresh_field_hud_runner_slot();
        refresh_command_overview_combat_bars();
        refresh_command_overview_item_detail();
    };
    bindings.game.roll_successes = [this](const int rolls,
                                          const int threshold) {
        const auto count = static_cast<std::uint8_t>(
            std::clamp(rolls, 0, 0xFF));
        const auto difficulty = static_cast<std::int16_t>(
            std::clamp(threshold, 0, 0x7FFF));
        return static_cast<int>(count_thresholded_random_rolls(
            count, difficulty, [this](const std::uint16_t bound) {
                return sample_source_random_bound(field_random_seed_, bound);
            }));
    };
    bindings.game.pocket_extra_row = [this] {
        return debug_menu_unlocked_ ||
               pocket_secretary_.maximum_selection > 4U;
    };
    bindings.game.in_combat = [this] {
        return world_frame_logic_state_.world_active != 0U;
    };
    bindings.game.scene_mode = [this] {
        // FFDF40: 0 outside; 0x1684C/0x1695C/0x16A04 write 1/2/3 [C].
        return static_cast<int>(field_movement_transition_state_.world_mode);
    };
    bindings.game.save_game = [this](const int slot) {
        return save_game_slot(slot);
    };
    bindings.game.load_game = [this](const int slot) {
        return load_game_slot(slot);
    };
    bindings.game.save_status = [this](const int slot) {
        return game_save_slot_status(slot);
    };
    bindings.game.set_nuyen = [this](const std::uint32_t amount) {
        field_funds_ = amount;
    };
    bindings.game.nuyen = [this] { return field_funds_; };
    bindings.game.begin_taxi_cabbie = [this](const int destination) {
        if (!field_taxi_route_active_ || destination < 0 || destination > 6 ||
            field_taxi_pending_destination_.has_value() ||
            field_taxi_cabbie_destination_.has_value() ||
            field_dialogue_.open() ||
            !field_session_source_assets_ ||
            static_cast<std::size_t>(destination) >=
                field_session_source_assets_->field_scene_resources.size()) {
            return false;
        }
        // ROM 0x20714 opens the cabbie dialogue here; 0x136B6 travels only
        // after that dialogue accepts and clears FFF0E0 [C].
        field_taxi_cabbie_destination_ =
            static_cast<std::uint16_t>(destination);
        source_boundary("field.taxi.cabbie_requested");
        return true;
    };
    bindings.game.consume_debug_unlock = [this] {
        debug_menu_unlocked_ = kDebugMenuAlwaysUnlockedForTesting;
        pocket_secretary_.maximum_selection =
            debug_menu_unlocked_ ? 5U : 4U;
    };
    bindings.game.set_contact_flags = [this](const std::uint32_t flags) {
        // ROM 0x59C48 replaces the entire FFFC2C long with its fixed seed.
        notebook_contacts_flags_ = flags;
    };
    bindings.game.set_walk_fast = [this] {
        field_walk_fast_enabled_ = true;
    };
    bindings.game.install_test_deck = [this] { install_test_deck(); };
    bindings.game.source_random_bound = [this](const int bound) {
        if (bound <= 0 || bound > 0xFFFF) {
            return 0;
        }
        return static_cast<int>(sample_source_random_bound(
            field_random_seed_, static_cast<std::uint16_t>(bound)));
    };
    bindings.game.matrix_alert_random_bound =
        bindings.game.source_random_bound;
    bindings.game.dismiss_runner = [this](const int slot) {
        if (slot >= 0 && slot <
                             static_cast<int>(FieldPartyRosterRuntime::kSlotCount)) {
            finish_pocket_secretary_dismiss_runner(
                static_cast<std::size_t>(slot));
        }
    };
    bindings.game.group_item_flags = [this]()
        -> std::optional<std::uint32_t> {
        if (!group_item_flags_.has_value()) {
            return std::nullopt;
        }
        return *group_item_flags_ & 0x7FFFU;
    };
    bindings.game.notebook_facts = [this]()
        -> std::optional<scripting::ScriptNotebookFacts> {
        scripting::ScriptNotebookFacts facts;
        facts.current_run_available = notebook_current_run_available_;
        facts.event_gates = notebook_event_gates();
        if (notebook_tip_flags_resolved_) {
            facts.tip_flags = notebook_tip_flags_;
        }
        facts.referral_flags = notebook_referral_flags_;
        facts.contact_flags = notebook_contacts_flags_;
        facts.shadowrunner_flags = notebook_shadowrunner_flags_;
        return facts;
    };
    bindings.game.cyberdeck_available = [this] {
        return cyberdeck_available_;
    };
    bindings.game.deck_enter = [this] {
        const auto saved_mode = static_cast<int>(field_matrix_deck_mode_);
        field_matrix_deck_mode_ = 0;
        return saved_mode;
    };
    bindings.game.deck_leave = [this](const int saved_mode) {
        field_matrix_deck_mode_ = static_cast<std::int8_t>(
            std::clamp(saved_mode, -1, 1));
    };
    bindings.game.set_deck_mode = [this](const int mode) {
        field_matrix_deck_mode_ = static_cast<std::int8_t>(
            std::clamp(mode, -1, 1));
    };
    bindings.game.matrix_online = [] { return false; };
    bindings.game.matrix_node = []()
        -> std::optional<scripting::ScriptMatrixNodeFacts> {
        return std::nullopt;
    };
    bindings.game.alert = [] { return -1; };
    bindings.game.world_effect_gate_negative = [this] {
        // FFDF84 is shared by the field timeout/world-activity helper and
        // Matrix. The field runtime keeps the signed source word here; Matrix
        // tests its sign at 0x19B40 rather than treating any active timeout as
        // a negative gate.
        return field_world_activity_timeout_state_.timeout_marker < 0;
    };
    bindings.game.map_seen = [] { return 0; };
    bindings.game.matrix_world_state = [this] {
        scripting::ScriptMatrixWorldState world;
        world.karma_counter = field_matrix_karma_counter_;
        world.passive_ic_counter = field_matrix_passive_ic_counter_;
        world.matrix_map_memory = field_matrix_map_memory_;
        world.field = field_matrix_field_state_;
        world.datastore = make_matrix_datastore_facts(
            notebook_tip_flags_, notebook_tip_flags_resolved_,
            field_exit_world_flags_, group_item_flags_, field_funds_,
            notebook_referral_flags_);
        const auto scene_reference =
            state_.field_scene_buffers.scene_reference;
        for (const auto& actor : field_default_actors_) {
            if (!actor.active || actor.scene_reference != scene_reference ||
                !actor.source_record_slot.has_value()) {
                continue;
            }
            world.actors.push_back(scripting::ScriptMatrixFieldActorState{
                actor.actor_instance_id, actor.source_interaction_flags});
        }
        return world;
    };
    bindings.game.commit_matrix_world_state = [this](
        const scripting::ScriptMatrixWorldState& update) {
        if (update.karma_counter.has_value()) {
            field_matrix_karma_counter_ = update.karma_counter;
        }
        if (update.passive_ic_counter.has_value()) {
            field_matrix_passive_ic_counter_ = update.passive_ic_counter;
        }
        if (update.matrix_map_memory.has_value()) {
            field_matrix_map_memory_ = update.matrix_map_memory;
        }
        if (update.field.alarm.has_value()) {
            field_matrix_field_state_.alarm = update.field.alarm;
        }
        if (update.field.security_off.has_value()) {
            field_matrix_field_state_.security_off =
                update.field.security_off;
        }
        if (update.field.maglocks_off.has_value()) {
            field_matrix_field_state_.maglocks_off =
                update.field.maglocks_off;
        }
        if (update.datastore.has_value()) {
            apply_matrix_datastore_facts(
                *update.datastore, notebook_tip_flags_,
                notebook_tip_flags_resolved_, field_exit_world_flags_,
                group_item_flags_, field_funds_, notebook_referral_flags_);
        }
        const auto scene_reference =
            state_.field_scene_buffers.scene_reference;
        for (const auto& source : update.actors) {
            const auto actor = std::find_if(
                field_default_actors_.begin(), field_default_actors_.end(),
                [scene_reference, &source](const auto& candidate) {
                    return candidate.active &&
                        candidate.scene_reference == scene_reference &&
                        candidate.source_record_slot.has_value() &&
                        candidate.actor_instance_id == source.actor_id;
                });
            if (actor != field_default_actors_.end()) {
                actor->source_interaction_flags = source.flags_9e;
            }
        }
    };
    bindings.game.deck_record = [this] {
        const auto source = field_matrix_deck_record_.value_or(
            CyberdeckRecord{});
        scripting::ScriptCyberdeckRecord record;
        record.mpcp = source.mpcp.value_or(0U);
        record.hardening = source.hardening.value_or(0U);
        record.memory = source.memory_megapulses.value_or(0U);
        record.storage = source.storage_megapulses.value_or(0U);
        record.load_io = source.load_io_speed.value_or(0U);
        record.response = source.response.value_or(0U);
        record.persona = source.persona.value_or(
            std::array<std::uint8_t, 4>{});
        record.persona_condition = source.persona_condition.value_or(0U);
        record.model = source.model_index.value_or(-1);
        record.ratings = source.program_ratings.value_or(
            std::array<std::uint8_t, 12>{});
        if (source.active_programs.has_value()) {
            record.active_programs = *source.active_programs;
        }
        if (source.data_files.has_value()) {
            for (std::size_t index = 0; index < record.files.size(); ++index) {
                record.files[index] = {
                    (*source.data_files)[index].id,
                    (*source.data_files)[index].size,
                    (*source.data_files)[index].value};
            }
        }
        return record;
    };
    bindings.game.deck_slots = [this] {
        return field_matrix_program_slots_;
    };
    bindings.game.deck_commit = [this](
        const scripting::ScriptCyberdeckRecord& source,
        const std::array<scripting::ScriptCyberdeckProgramSlot, 5>&
            program_slots) {
        CyberdeckRecord record;
        record.mpcp = source.mpcp;
        record.hardening = source.hardening;
        record.memory_megapulses = source.memory;
        record.storage_megapulses = source.storage;
        record.load_io_speed = source.load_io;
        record.response = source.response;
        record.persona = source.persona;
        record.persona_condition = source.persona_condition;
        record.model_index = source.model;
        record.program_ratings = source.ratings;
        record.active_programs = source.active_programs;
        std::array<CyberdeckDataFile, 5> files{};
        for (std::size_t index = 0; index < files.size(); ++index) {
            files[index] = {source.files[index].id, source.files[index].size,
                            source.files[index].value};
        }
        record.data_files = files;
        field_matrix_deck_record_ = std::move(record);
        field_matrix_program_slots_ = program_slots;
    };
    bindings.game.program_unloaded = [](const int) {
        // Program effects are owned by the active Matrix session. The field
        // Cyberdeck editor has no session effects to clear.
    };
    game_scripts_.set_menu_route_bindings(std::move(bindings));
}

void FieldSceneRuntime::advance_field_menu_route() {
    game_scripts_.advance_menu_route(field_controller_sample_state_.events);
    const auto route_state = game_scripts_.menu_route_snapshot();
    synchronize_command_overview_clips_from_route(route_state);
    synchronize_command_overview_inventory_from_route(route_state);
    synchronize_command_overview_magic_from_route(route_state);
    if (route_state.page.has_value() && route_state.row.has_value()) {
        command_overview_.sync_script_selection(
            static_cast<std::uint8_t>(*route_state.page),
            static_cast<std::uint8_t>(*route_state.row));
    }
    menu_route_screen_ = route_state.top_screen;
    posture_dialog_active_ =
        route_state.open &&
        menu_route_screen_ == static_cast<std::uint16_t>(
                                  srx::MenuScreen::POSTURE);
    if (menu_route_screen_ == static_cast<std::uint16_t>(
                                  srx::MenuScreen::ATTRIBUTES_SKILLS) &&
        route_state.top_column.has_value() &&
        route_state.top_row.has_value()) {
        command_overview_attributes_skills_.synchronize_cursor(
            *route_state.top_column, *route_state.top_row,
            active_runner_record());
    }
    if (route_state.top_screen == static_cast<std::uint16_t>(
                                      srx::MenuScreen::POCKET_SECRETARY) &&
        route_state.top_row.has_value()) {
        pocket_secretary_.selection = static_cast<std::uint8_t>(
            std::clamp(*route_state.top_row, 0,
                       static_cast<int>(
                           pocket_secretary_.maximum_selection)));
    }
    if (route_state.top_screen == static_cast<std::uint16_t>(
                                      srx::MenuScreen::DISMISS_RUNNER) &&
        route_state.top_row.has_value() && dismiss_runner_.open) {
        dismiss_runner_.selection = static_cast<std::uint8_t>(
            std::clamp(*route_state.top_row, 0,
                       static_cast<int>(dismiss_runner_.candidate_count)));
    }
    if (route_state.top_screen == static_cast<std::uint16_t>(
                                      srx::MenuScreen::GROUP_ITEMS)) {
        pocket_secretary_.group_item_display_order.fill(0U);
        pocket_secretary_.group_item_display_count =
            static_cast<std::uint8_t>(std::min<std::size_t>(
                route_state.top_visible_item_ids.size(),
                pocket_secretary_.group_item_display_order.size()));
        for (std::size_t index = 0;
             index < pocket_secretary_.group_item_display_count; ++index) {
            pocket_secretary_.group_item_display_order[index] =
                static_cast<std::uint8_t>(
                    route_state.top_visible_item_ids[index]);
        }
    }
    const bool no_cyberdeck_message =
        route_state.top_screen == static_cast<std::uint16_t>(
                                      srx::MenuScreen::MESSAGE) &&
        route_state.top_message == static_cast<int>(
                                       srx::MenuMessage::NO_CYBERDECK);
    if (pocket_secretary_.open) {
        if (no_cyberdeck_message) {
            pocket_secretary_.cyberdeck_unavailable_wait = true;
        } else if (route_state.top_screen != static_cast<std::uint16_t>(
                                                srx::MenuScreen::CYBERDECK)) {
            pocket_secretary_.cyberdeck_unavailable_wait = false;
        }
    }
    synchronize_pocket_secretary_notebook_from_route(route_state);
    if (route_state.top_grid_cells_present) {
        dismiss_runner_random_grid_cells_ = route_state.top_grid_cells;
    }
    if (route_state.top_grid_effect_frames.has_value()) {
        dismiss_runner_effect_frames_ = static_cast<std::uint32_t>(
            std::max(0, *route_state.top_grid_effect_frames));
    }
    if (!route_state.open && field_taxi_cabbie_destination_.has_value() &&
        !field_dialogue_.open()) {
        const auto destination = *field_taxi_cabbie_destination_;
        const auto current_region = static_cast<std::uint8_t>(
            state_.field_scene_buffers.scene_reference);
        game_scripts_.set_location_context(
            "activity", "taxi_cabbie", "field.taxi", "taxi_cabbie",
            "cabbie", current_region);
        scripting::ScriptFieldModeContext context{
            state_.field_scene_buffers.field_resource_mode,
            field_movement_transition_state_.mode_event_code,
            field_exit_alternate_result_,
            state_.field_scene_buffers.scene_mode,
            field_movement_transition_state_.field_transition_context};
        context.taxi_destination_region =
            static_cast<std::uint8_t>(destination);
        game_scripts_.set_field_mode_context(std::move(context));
        if (!field_dialogue_.open_gameplay_task("taxi_cabbie", "present")) {
            field_taxi_cabbie_destination_.reset();
            source_boundary("field.taxi.cabbie_dialogue_open_failed");
        } else {
            source_boundary("field.taxi.cabbie_dialogue_opened");
        }
    }
}

bool FieldSceneRuntime::bootstrap(
    const integration::StartupIntegrationState& startup_state) {
    ready_ = false;
    title_return_requested_ = false;
    failure_.clear();
    field_shop_menu_.close();
    field_shop_route_pending_service_id_.clear();
    field_shop_notice_active_ = false;
    field_shop_notice_service_id_.clear();
    field_shop_notice_text_key_.clear();
    field_funds_.reset();
    field_walk_fast_enabled_ = false;
    field_matrix_field_state_ = scripting::ScriptMatrixFieldState{};
    // Startup 0x007F6 calls 0x53290; field mode also calls it when 0x17490
    // detects a system change. A fresh bootstrap starts a new RAM bank, and
    // this transient memory is not included in the host's durable save record.
    field_matrix_map_memory_.reset();
    field_matrix_karma_counter_.reset();
    command_overview_magic_state_ = CommandOverviewMagicState::inactive;
    command_overview_magic_physical_defense_ = MagicDefenseStripView{};
    command_overview_magic_mana_defense_ = MagicDefenseStripView{};
    state_ = integration::FieldSessionBootstrapState{};
    field_session_source_assets_.reset();
    field_asset_catalog_.reset();
    world_frame_state_ = WorldFrameState{};
    viewport_state_ = FieldViewportState{};
    debug_menu_unlocked_ = kDebugMenuAlwaysUnlockedForTesting ||
                           startup_state.title_debug_menu_unlocked;

    if (!startup_state.field_session_source_assets.has_value()) {
        failure_ = "field-session source records are unavailable";
        source_boundary("field.bootstrap.source_records_missing");
        return false;
    }

    const auto* restored_field_state =
        startup_state.restored_field_state.has_value()
            ? &*startup_state.restored_field_state
            : nullptr;
    // ROM 0x007F0 initializes FFFBFE to 20 on the New Game route before the
    // field is entered. The current save adapter does not restore that global.
    field_funds_ = restored_field_state == nullptr
                       ? std::optional<std::uint32_t>{0x14U}
                       : std::nullopt;
    // The cold New Game path begins with zeroed work RAM. Restored snapshots
    // carry these bytes when known; older saves remain unresolved.
    field_heat_level_ = restored_field_state == nullptr
        ? std::optional<std::uint8_t>{0U}
        : std::nullopt;
    field_strike_team_counter_ = restored_field_state == nullptr
        ? std::optional<std::uint8_t>{0U}
        : std::nullopt;
    field_runner_feature_flags_ =
        restored_field_state == nullptr
            ? std::optional<std::uint8_t>{0U}
            : std::nullopt;
    field_entity_action_phase_roll_counters_ =
        restored_field_state == nullptr
            ? std::optional<std::array<std::uint8_t, 3>>{
                  std::array<std::uint8_t, 3>{}}
            : std::nullopt;
    // ROM 0x00C2C clears FFF0CE during full initialization; the warm-reset
    // short-clear path preserves it. A loaded save does not expose this byte.
    field_matrix_karma_counter_ = restored_field_state == nullptr
        ? std::optional<std::int8_t>{0}
        : std::nullopt;
    field_matrix_passive_ic_counter_ = restored_field_state == nullptr
        ? std::optional<std::int8_t>{1}
        : std::nullopt;
    pocket_secretary_ = PocketSecretaryMenuState{};
    dismiss_runner_ = DismissRunnerMenuState{};
    dismiss_runner_random_grid_effect_ = RandomGridEffectState{};
    dismiss_runner_effect_frames_ = 0U;
    dismiss_runner_random_grid_cells_.fill(-1);
    // The 0x59C48 New Game path begins without the FF018F deck-present bit.
    // The loaded-save contract does not yet expose this global, so keep that
    // branch explicit instead of treating an unknown saved bit as clear.
    cyberdeck_available_ = restored_field_state == nullptr
                               ? std::optional<bool>{false}
                               : std::nullopt;
    // The ROM starts a fresh run with FFE17F clear. A restored host save does
    // not yet carry this field fact, so leave it unknown in that path.
    if (restored_field_state == nullptr) {
        field_matrix_field_state_.maglocks_off = false;
    }
    field_matrix_passcode_flags_ = 0U;
    field_matrix_deck_record_.reset();
    group_item_flags_ = restored_field_state == nullptr
                            ? std::optional<std::uint32_t>{0U}
                            : std::nullopt;
    // ROM startup 0x084E seeds FFFC30 bit 0 for a fresh New Game.
    notebook_tip_flags_ = {{1U, 0U, 0U}};
    notebook_tip_flags_resolved_ = restored_field_state == nullptr;
    // 0x82A clears FFFC2C/27/2A; New Game setup later calls 0x59C48 to seed
    // the Contacts gate to 0x1C7F. No loaded-save adapter restores these.
    notebook_contacts_flags_ = restored_field_state == nullptr
                                   ? std::optional<std::uint32_t>{0x1C7FU}
                                   : std::nullopt;
    notebook_referral_flags_ = restored_field_state == nullptr
                                   ? std::optional<std::uint8_t>{0U}
                                   : std::nullopt;
    notebook_shadowrunner_flags_ = restored_field_state == nullptr
                                      ? std::optional<std::uint16_t>{0U}
                                      : std::nullopt;
    const auto& assets = *startup_state.field_session_source_assets;
    field_session_source_assets_ =
        std::make_unique<integration::FieldSessionSourceAssets>(assets);
    field_asset_catalog_ = std::make_unique<ConversionAssetCatalog>(
        startup_state.asset_catalog.catalog);
    const auto restored_scene_reference =
        restored_field_state != nullptr
            ? restored_field_state->world_mode
            : kInitialFieldSceneReference;
#ifdef SHADOWRUN_QT_QA_DRIVER
    const auto scene_reference =
        field_scene_reference_override_for_dump_.value_or(
            restored_scene_reference);
    field_scene_reference_override_for_dump_.reset();
#else
    const auto scene_reference = restored_scene_reference;
#endif
    const auto scene_input = integration::make_field_scene_input(
        assets, scene_reference);
    if (!scene_input.has_value()) {
        failure_ = "field scene resources are not extracted for scene " +
                   std::to_string(scene_reference);
        source_boundary("field.bootstrap.field_scene_resources_missing");
        return false;
    }
    field_movement_direction_offsets_ = assets.field_movement_direction_offsets;
    field_controller_heading_table_ = assets.field_controller_heading_table;
    field_movement_route_sources_ = assets.field_movement_routes;
    for (std::size_t index = 0; index < field_movement_routes_.size();
         ++index) {
        const auto& source_route = assets.field_movement_routes[index];
        field_movement_routes_[index] = DirectionalMovementRoute{
            0,
            0,
            source_route.primary_fallback_slot,
            source_route.secondary_fallback_slot};
    }
    field_controller_nibble_ = 0U;
    field_controller_port_byte_ = 0U;
    field_player_actor_script_initialized_ = false;
    // docs/24 R1: no host entropy. FFF9F8 keeps its boot value (cold_boot_source_random_seed, 0xFFFF with zeroed
    // work RAM) plus every title-screen draw, exactly as the ROM; the debugger records it in the SEEDS chunk.
    field_opening_auto_walk_ = FieldOpeningAutoWalkState{};
    field_mode_entry_transition_pending_ = false;
    field_mode_entry_destination_scene_.reset();
    field_mode_entry_run_target_actor_group_.reset();
    field_indexed_title_service_result_ = 0;
    field_active_story_trigger_.reset();
    field_controller_sample_state_ = ControllerSampleState{};
    field_controller_latch_state_ = ControllerLatchState{};
    field_target_heading_ = 0U;
    field_movement_state_ = EntityMovementStepState{};
    field_collision_record_ = EntityCollisionRecord{};
    field_collision_probe_state_ = CollisionProbeState{};
    field_movement_behavior_record_ = EntityBehaviorFrameRecord{};
    field_movement_transition_state_ = EntityMovementModeTransitionState{};
    const auto script_directory =
        startup_state.asset_catalog.catalog.asset_root.parent_path() /
        "gameplay" / "scripts";
    const auto dialogue_text_file =
        startup_state.asset_catalog.catalog.asset_root / "dialogue" /
        "script-text.json";
    if (!game_scripts_.load_package(script_directory,
                                   dialogue_text_file)) {
        failure_ = game_scripts_.failure();
        source_boundary("field.bootstrap.gameplay_scripts_missing");
        return false;
    }
    configure_field_menu_route();
    const auto map_rules = game_scripts_.map_event_rules();
    if (!map_rules.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.bootstrap.map_event_scripts_invalid");
        return false;
    }
    field_shop_definition_ =
        game_scripts_.shop_definition("ares_weapon_emporium");
    if (!field_shop_definition_.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.bootstrap.shop_script_invalid");
        return false;
    }
    field_map_event_system_ = FieldMapEventSystem(*map_rules);
    field_map_event_state_flags_ = 0U;
    field_palette_fade_steps_ = 0U;
    field_world_activity_scene_entry_pending_ = false;
    field_pending_mode_transition_scene_.reset();
    field_pending_mode_transition_spawn_.reset();
    field_pending_mode_transition_actor_list_.reset();
    field_pending_story_world_effect_.reset();
    field_pending_route_actor_list_.reset();
    field_pending_actor_list_transition_group_.reset();
    field_route_actor_transitions_remaining_.reset();
    field_pending_actor_list_callback_.reset();
    field_active_route_actor_list_.clear();
    field_world_activity_state_ = WorldActivityState{};
    field_world_activity_timeout_state_ = WorldActivityTimeoutState{};
    world_activity_routine_state_ = WorldActivityRoutineState{};
    world_activity_request_state_ = WorldActivityRoutineRequestState{};
    world_activity_request_start_state_ = WorldActivityStartState{};
    world_activity_return_snapshot_.reset();
    field_weapon_projectiles_.clear();
    field_weapon_slots_occupied_.fill(false);
    last_field_weapon_spawn_result_ = EntitySpawnCycleResult{};
    field_security_floor_index_.reset();
    field_activity_continuation_ = MovementModeTransitionContinuation::none;
    field_projected_terrain_mark_count_ = 0U;
    field_terrain_sample_state_ = {};
    field_terrain_boundary_active_ = false;
    field_terrain_outside_bounds_marker_ = false;
    field_action_counter_ = 0U;
    field_entity_behavior_flags_ = 0U;
    field_exit_alternate_result_ = 0;
    field_exit_world_flags_ = 0U;
    // New Game seeds FFFC30 bit 0 at ROM 0x59C48.
    field_mode_flags_ = 1U;
    field_mode_flags_resolved_ = restored_field_state == nullptr;
    field_mode_effect_flags_ = 0U;
    field_dialogue_flags_ = restored_field_state == nullptr
        ? std::optional<std::uint8_t>{0U}
        : std::optional<std::uint8_t>{
              restored_field_state->dialogue_flags};
    field_pending_world_effect_ = false;
    field_world_effect_active_ = false;
    field_auxiliary_flags_ = 0U;
    field_dialogue_register_ = restored_field_state == nullptr
        ? std::optional<std::uint8_t>{0U}
        : std::nullopt;
    field_render_effect_flags_ = 0U;
    game_scripts_.set_story_fact_accessors(
        [this](const std::string_view name) -> std::optional<bool> {
            if (name == "cyberspace_deck_present") {
                return cyberdeck_available_;
            }
            if (name == "cyberspace_mpcp_functional") {
                if (!field_matrix_deck_record_.has_value()) {
                    return std::nullopt;
                }
                return field_matrix_deck_record_->mpcp != 0U;
            }
            if (name == "cyberspace_datajack_present") {
                return (active_runner_record().cyberware_flags & 0x0001U) !=
                       0U;
            }
            if (name == "matrix_passcode_flags_resolved") {
                return field_matrix_passcode_flags_.has_value();
            }
            if (name == "matrix_current_passcode_available") {
                if (!field_matrix_passcode_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*field_matrix_passcode_flags_ & 0x0001U) != 0U;
            }
            constexpr std::string_view kMatrixPasscodePrefix =
                "matrix_passcode_bit_";
            if (name.size() > kMatrixPasscodePrefix.size() &&
                name.substr(0U, kMatrixPasscodePrefix.size()) ==
                    kMatrixPasscodePrefix) {
                unsigned bit = 0U;
                const auto suffix = name.substr(kMatrixPasscodePrefix.size());
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), bit);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != suffix.data() + suffix.size() || bit > 11U) {
                    return std::nullopt;
                }
                if (!field_matrix_passcode_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*field_matrix_passcode_flags_ & (1U << bit)) != 0U;
            }
            if (name == "vid_phone_contacts_resolved") {
                return notebook_contacts_flags_.has_value();
            }
            if (name == "vid_phone_affiliation_mafia" ||
                name == "vid_phone_affiliation_yakuza") {
                if (!field_runner_feature_flags_.has_value()) {
                    return std::nullopt;
                }
                // ROM 0x22CC8/0x22D18 write FF018F bits 2/1 when the
                // corresponding Vid-Phone pledge is accepted.
                const auto bit = name == "vid_phone_affiliation_mafia"
                                     ? 2U
                                     : 1U;
                return (*field_runner_feature_flags_ & (1U << bit)) != 0U;
            }
            constexpr std::string_view kRunnerFeatureBitPrefix =
                "runner_feature_flag_bit_";
            if (name.size() > kRunnerFeatureBitPrefix.size() &&
                name.substr(0U, kRunnerFeatureBitPrefix.size()) ==
                    kRunnerFeatureBitPrefix) {
                unsigned bit = 0U;
                const auto suffix = name.substr(kRunnerFeatureBitPrefix.size());
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), bit);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != suffix.data() + suffix.size() || bit >= 8U ||
                    !field_runner_feature_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*field_runner_feature_flags_ & (1U << bit)) != 0U;
            }
            constexpr std::string_view kVidPhoneStoryFlagPrefix =
                "vid_phone_story_flag_bit_";
            if (name.size() > kVidPhoneStoryFlagPrefix.size() &&
                name.substr(0U, kVidPhoneStoryFlagPrefix.size()) ==
                    kVidPhoneStoryFlagPrefix) {
                unsigned bit = 0U;
                const auto suffix = name.substr(kVidPhoneStoryFlagPrefix.size());
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), bit);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != suffix.data() + suffix.size() || bit >= 32U ||
                    !notebook_contacts_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*notebook_contacts_flags_ & (1U << bit)) != 0U;
            }
            if (name == "lone_star_criminal_record_active") {
                return field_heat_level_.has_value()
                           ? std::optional<bool>{
                                 *field_heat_level_ != 0U}
                           : std::nullopt;
            }
            constexpr std::string_view kPhoneContactPrefix =
                "vid_phone_contact_";
            constexpr std::string_view kPhoneCalledPrefix =
                "vid_phone_called_";
            const auto phone_fact_index = [name](
                const std::string_view prefix) -> std::optional<unsigned> {
                if (name.size() <= prefix.size() ||
                    name.substr(0U, prefix.size()) != prefix) {
                    return std::nullopt;
                }
                unsigned index = 0U;
                const auto suffix = name.substr(prefix.size());
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), index);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != suffix.data() + suffix.size() ||
                    index >= 13U) {
                    return std::nullopt;
                }
                return index;
            };
            if (const auto contact_index =
                    phone_fact_index(kPhoneContactPrefix)) {
                if (!notebook_contacts_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*notebook_contacts_flags_ &
                        (1U << *contact_index)) != 0U;
            }
            if (const auto contact_index =
                    phone_fact_index(kPhoneCalledPrefix)) {
                if (!notebook_contacts_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*notebook_contacts_flags_ &
                        (1U << (*contact_index + 13U))) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kStoryFlagBitPrefix)) {
                return (field_exit_world_flags_ & (1U << *bit)) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kFieldModeFlagBitPrefix)) {
                if (!field_mode_flags_resolved_) {
                    return std::nullopt;
                }
                return (field_mode_flags_ & (1U << *bit)) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kFieldModeEffectFlagBitPrefix)) {
                // Source FFFC34: world/dialogue effect flags.
                return (field_mode_effect_flags_ & (1U << *bit)) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kFieldDialogueFlagBitPrefix, 8U)) {
                if (!field_dialogue_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*field_dialogue_flags_ &
                        static_cast<std::uint8_t>(1U << *bit)) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kFieldAuxiliaryFlagBitPrefix)) {
                // Source FFFC40: Spirit Eyes quest activity markers.
                return (field_auxiliary_flags_ & (1U << *bit)) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kNotebookTipFlagBitPrefix, 96U)) {
                if (*bit >= 96U || !notebook_tip_flags_resolved_) {
                    return std::nullopt;
                }
                const auto group = static_cast<std::size_t>(*bit / 32U);
                const auto mask = std::uint32_t{1U} << (*bit % 32U);
                return (notebook_tip_flags_[group] & mask) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kMrJohnsonsFlagBitPrefix, 8U)) {
                if (*bit >= 8U || !notebook_referral_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*notebook_referral_flags_ &
                        static_cast<std::uint8_t>(1U << *bit)) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kRunnerInformationFlagBitPrefix, 16U)) {
                if (*bit >= 16U || !notebook_shadowrunner_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*notebook_shadowrunner_flags_ &
                        static_cast<std::uint16_t>(1U << *bit)) != 0U;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kGroupItemFlagBitPrefix)) {
                if (*bit >= 15U || !group_item_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*group_item_flags_ & (1U << *bit)) != 0U;
            }
            if (const auto trigger = story_fact_bit_index(
                    name, kStoryTriggerActivePrefix)) {
                if (!field_active_story_trigger_.has_value()) {
                    return std::nullopt;
                }
                return *field_active_story_trigger_ == *trigger;
            }
            if (name == "pending_world_effect") {
                return field_pending_world_effect_;
            }
            if (name == "world_effect_active") {
                return field_world_effect_active_;
            }
            if (name == "wanderer_vigore_jarl_intro_seen") {
                if (!field_dialogue_register_.has_value()) {
                    return std::nullopt;
                }
                return (*field_dialogue_register_ & field_flag_mask(
                    FieldDialogueRegisterFlag::
                        wanderer_vigore_jarl_intro_seen)) != 0U;
            }
            if (name == "errascoe_question_available") {
                return (field_exit_world_flags_ & field_flag_mask(
                    FieldWorldFactFlag::errascoe_question_available)) != 0U;
            }
            if (name == "vigore_lead_available") {
                return (field_exit_world_flags_ & field_flag_mask(
                    FieldWorldFactFlag::redmond_vigore_lead_available)) != 0U;
            }
            if (name == "vigore_search_pending") {
                return (field_mode_effect_flags_ & field_flag_mask(
                    FieldTitleServiceFlag::redmond_vigore_search_pending)) != 0U;
            }
            if (name == "vigore_search_consumed") {
                return (field_mode_effect_flags_ & field_flag_mask(
                    FieldTitleServiceFlag::redmond_vigore_search_consumed)) != 0U;
            }
            if (name == "vigore_search_completed") {
                return (field_exit_world_flags_ & field_flag_mask(
                    FieldWorldFactFlag::redmond_vigore_search_complete)) != 0U;
            }
            if (name == "landmark_map_owned") {
                if (!group_item_flags_.has_value()) {
                    return std::nullopt;
                }
                return (*group_item_flags_ & group_item_flag_mask(
                    GroupItemFlag::landmark_map)) != 0U;
            }
            return std::nullopt;
        },
        [this](const std::string_view name, const bool value) {
            constexpr std::string_view kPhoneCalledPrefix =
                "vid_phone_called_";
            if (name.size() > kPhoneCalledPrefix.size() &&
                name.substr(0U, kPhoneCalledPrefix.size()) ==
                    kPhoneCalledPrefix) {
                unsigned index = 0U;
                const auto suffix = name.substr(kPhoneCalledPrefix.size());
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), index);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != suffix.data() + suffix.size() ||
                    index >= 13U || !notebook_contacts_flags_.has_value()) {
                    return false;
                }
                const auto mask = static_cast<std::uint32_t>(
                    1U << (index + 13U));
                *notebook_contacts_flags_ = value
                    ? *notebook_contacts_flags_ | mask
                    : *notebook_contacts_flags_ & ~mask;
                return true;
            }
            if (name == "vid_phone_affiliation_mafia" ||
                name == "vid_phone_affiliation_yakuza") {
                if (!field_runner_feature_flags_.has_value()) {
                    return false;
                }
                const auto bit = name == "vid_phone_affiliation_mafia"
                                     ? 2U
                                     : 1U;
                const auto mask = static_cast<std::uint8_t>(1U << bit);
                *field_runner_feature_flags_ = value
                    ? static_cast<std::uint8_t>(
                          *field_runner_feature_flags_ | mask)
                    : static_cast<std::uint8_t>(
                          *field_runner_feature_flags_ &
                          static_cast<std::uint8_t>(~mask));
                return true;
            }
            constexpr std::string_view kRunnerFeatureBitPrefix =
                "runner_feature_flag_bit_";
            if (name.size() > kRunnerFeatureBitPrefix.size() &&
                name.substr(0U, kRunnerFeatureBitPrefix.size()) ==
                    kRunnerFeatureBitPrefix) {
                unsigned bit = 0U;
                const auto suffix = name.substr(kRunnerFeatureBitPrefix.size());
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), bit);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != suffix.data() + suffix.size() || bit >= 8U ||
                    !field_runner_feature_flags_.has_value()) {
                    return false;
                }
                const auto mask = static_cast<std::uint8_t>(1U << bit);
                *field_runner_feature_flags_ = value
                    ? static_cast<std::uint8_t>(
                          *field_runner_feature_flags_ | mask)
                    : static_cast<std::uint8_t>(
                          *field_runner_feature_flags_ &
                          static_cast<std::uint8_t>(~mask));
                return true;
            }
            constexpr std::string_view kVidPhoneStoryFlagPrefix =
                "vid_phone_story_flag_bit_";
            if (name.size() > kVidPhoneStoryFlagPrefix.size() &&
                name.substr(0U, kVidPhoneStoryFlagPrefix.size()) ==
                    kVidPhoneStoryFlagPrefix) {
                unsigned bit = 0U;
                const auto suffix = name.substr(kVidPhoneStoryFlagPrefix.size());
                const auto parsed = std::from_chars(
                    suffix.data(), suffix.data() + suffix.size(), bit);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != suffix.data() + suffix.size() || bit >= 32U ||
                    !notebook_contacts_flags_.has_value()) {
                    return false;
                }
                const auto mask = std::uint32_t{1U} << bit;
                *notebook_contacts_flags_ = value
                    ? *notebook_contacts_flags_ | mask
                    : *notebook_contacts_flags_ & ~mask;
                return true;
            }
            if (name == "lone_star_criminal_record_active") {
                // The source Vid-Phone command clears the full FFFBFA heat
                // byte. A boolean true cannot reconstruct its numeric level.
                if (value) {
                    return false;
                }
                field_heat_level_ = 0U;
                return true;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kStoryFlagBitPrefix)) {
                const auto mask = std::uint32_t{1U} << *bit;
                field_exit_world_flags_ = value
                    ? field_exit_world_flags_ | mask
                    : field_exit_world_flags_ & ~mask;
                return true;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kFieldModeFlagBitPrefix)) {
                if (!field_mode_flags_resolved_) {
                    return false;
                }
                const auto mask = std::uint32_t{1U} << *bit;
                field_mode_flags_ = value
                    ? field_mode_flags_ | mask
                    : field_mode_flags_ & ~mask;
                return true;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kFieldModeEffectFlagBitPrefix)) {
                const auto mask = std::uint32_t{1U} << *bit;
                field_mode_effect_flags_ = value
                    ? field_mode_effect_flags_ | mask
                    : field_mode_effect_flags_ & ~mask;
                return true;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kFieldDialogueFlagBitPrefix, 8U)) {
                if (!field_dialogue_flags_.has_value()) {
                    return false;
                }
                const auto mask = static_cast<std::uint8_t>(1U << *bit);
                *field_dialogue_flags_ = value
                    ? static_cast<std::uint8_t>(*field_dialogue_flags_ | mask)
                    : static_cast<std::uint8_t>(*field_dialogue_flags_ &
                                                static_cast<std::uint8_t>(~mask));
                return true;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kFieldAuxiliaryFlagBitPrefix)) {
                const auto mask = std::uint32_t{1U} << *bit;
                field_auxiliary_flags_ = value
                    ? field_auxiliary_flags_ | mask
                    : field_auxiliary_flags_ & ~mask;
                return true;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kNotebookTipFlagBitPrefix, 96U)) {
                if (*bit >= 96U || !notebook_tip_flags_resolved_) {
                    return false;
                }
                return set_notebook_tip_unlocked(*bit, value);
            }
            if (const auto bit = story_fact_bit_index(
                    name, kMrJohnsonsFlagBitPrefix, 8U)) {
                if (*bit >= 8U || !notebook_referral_flags_.has_value()) {
                    return false;
                }
                const auto mask = static_cast<std::uint8_t>(1U << *bit);
                *notebook_referral_flags_ = value
                    ? static_cast<std::uint8_t>(
                          *notebook_referral_flags_ | mask)
                    : static_cast<std::uint8_t>(
                          *notebook_referral_flags_ &
                          static_cast<std::uint8_t>(~mask));
                return true;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kRunnerInformationFlagBitPrefix, 16U)) {
                if (*bit >= 16U || !notebook_shadowrunner_flags_.has_value()) {
                    return false;
                }
                const auto mask = static_cast<std::uint16_t>(1U << *bit);
                *notebook_shadowrunner_flags_ = value
                    ? static_cast<std::uint16_t>(
                          *notebook_shadowrunner_flags_ | mask)
                    : static_cast<std::uint16_t>(
                          *notebook_shadowrunner_flags_ &
                          static_cast<std::uint16_t>(~mask));
                return true;
            }
            if (const auto bit = story_fact_bit_index(
                    name, kGroupItemFlagBitPrefix)) {
                if (*bit >= 15U || !group_item_flags_.has_value()) {
                    return false;
                }
                const auto mask = std::uint32_t{1U} << *bit;
                *group_item_flags_ = value
                    ? *group_item_flags_ | mask
                    : *group_item_flags_ & ~mask;
                return true;
            }
            if (const auto trigger = story_fact_bit_index(
                    name, kStoryTriggerActivePrefix)) {
                // Fixed-index commands write FFDD64 before 0x2196C. There is
                // no source operation that clears it, so only true is valid.
                if (!value) {
                    return false;
                }
                field_active_story_trigger_ = *trigger;
                return true;
            }
            if (name == "pending_world_effect") {
                field_pending_world_effect_ = value;
                return true;
            }
            if (name == "world_effect_active") {
                field_world_effect_active_ = value;
                return true;
            }
            if (name == "wanderer_vigore_jarl_intro_seen") {
                if (!field_dialogue_register_.has_value()) {
                    return false;
                }
                const auto mask = field_flag_mask(
                    FieldDialogueRegisterFlag::
                        wanderer_vigore_jarl_intro_seen);
                *field_dialogue_register_ = value
                    ? static_cast<std::uint8_t>(
                          *field_dialogue_register_ | mask)
                    : static_cast<std::uint8_t>(
                          *field_dialogue_register_ &
                          static_cast<std::uint8_t>(~mask));
                return true;
            }
            if (name == "errascoe_question_available") {
                const auto mask = field_flag_mask(
                    FieldWorldFactFlag::errascoe_question_available);
                field_exit_world_flags_ = value
                    ? field_exit_world_flags_ | mask
                    : field_exit_world_flags_ & ~mask;
                return true;
            }
            if (name == "vigore_lead_available") {
                const auto mask = field_flag_mask(
                    FieldWorldFactFlag::redmond_vigore_lead_available);
                field_exit_world_flags_ = value
                    ? field_exit_world_flags_ | mask
                    : field_exit_world_flags_ & ~mask;
                return true;
            }
            if (name == "vigore_search_pending") {
                const auto mask = field_flag_mask(
                    FieldTitleServiceFlag::redmond_vigore_search_pending);
                if (value) {
                    const auto clear_15 = field_flag_mask(
                        FieldTitleServiceFlag::mode_command_e7_clear_15);
                    const auto clear_21 = field_flag_mask(
                        FieldTitleServiceFlag::mode_command_e7_clear_21);
                    field_mode_effect_flags_ &= ~(clear_15 | clear_21);
                    field_mode_effect_flags_ |= mask;
                } else {
                    field_mode_effect_flags_ &= ~mask;
                }
                return true;
            }
            if (name == "landmark_map_owned") {
                if (!group_item_flags_.has_value()) {
                    return false;
                }
                const auto mask = group_item_flag_mask(
                    GroupItemFlag::landmark_map);
                *group_item_flags_ = value
                    ? *group_item_flags_ | mask
                    : *group_item_flags_ & ~mask;
                return true;
            }
            if (name == "vigore_search_completed") {
                if (!value) {
                    return false;
                }
                const auto consumed = field_flag_mask(
                    FieldTitleServiceFlag::redmond_vigore_search_consumed);
                if ((field_mode_effect_flags_ & consumed) != 0U) {
                    return true;
                }
                const auto pending = field_flag_mask(
                    FieldTitleServiceFlag::redmond_vigore_search_pending);
                if ((field_mode_effect_flags_ & pending) == 0U ||
                    !group_item_flags_.has_value()) {
                    return false;
                }
                const auto effect_cleanup =
                    field_flag_mask(
                        FieldTitleServiceFlag::redmond_search_cleanup_09) |
                    field_flag_mask(
                        FieldTitleServiceFlag::redmond_search_cleanup_12) |
                    pending |
                    field_flag_mask(
                        FieldTitleServiceFlag::redmond_search_cleanup_15);
                field_mode_effect_flags_ =
                    (field_mode_effect_flags_ & ~effect_cleanup) | consumed;
                const auto render_cleanup =
                    field_flag_mask(
                        FieldRenderEffectFlag::redmond_search_cleanup_09) |
                    field_flag_mask(
                        FieldRenderEffectFlag::redmond_search_cleanup_14) |
                    field_flag_mask(
                        FieldRenderEffectFlag::redmond_search_cleanup_19) |
                    field_flag_mask(
                        FieldRenderEffectFlag::redmond_search_cleanup_29);
                field_render_effect_flags_ &= ~render_cleanup;
                // FFFC1E bit 22 is marked by the script's 0x5463C tail
                // (story_flags.finish_trigger) before this fact is written.
                *group_item_flags_ |= group_item_flag_mask(
                    GroupItemFlag::landmark_map);
                return true;
            }
            return false;
        });
    field_entity_selection_state_ = RunnerSwitchState{};
    field_runner_refresh_state_ = FieldRunnerRefreshState{};
    field_mode_exit_transition_state_ = FieldModeExitTransitionState{};
    field_entity_table_occupancy_ = {};
    field_entity_table_ = {};
    field_collision_marker_index_ = 0U;
    field_pending_scene_profile_.reset();
    field_camera_follow_ = {};
    field_camera_alignment_ready_ = false;
    game_scripts_.close_menu_route();
    menu_route_active_ = false;
    menu_route_screen_ = 0U;
    restore_start_menu_duck();
    command_overview_.close();
    field_dialogue_.close();
    field_dialogue_.clear_retained_credits_page_for_title_return();
    hotel_menu_.close();
    hotel_menu_content_ = HotelMenuContent{};
    hotel_karma_skills_.close_to_parent();
    field_mode_text_.close();
    field_interaction_.reset_session();
    field_elevator_selector_ = ElevatorSelectorRuntime{};
    field_elevator_scene_transition_ = ElevatorSceneTransitionRuntime{};
    tarislar_credits_title_return_.reset();
    field_elevator_floor_group_.reset();
    field_elevator_script_destination_.reset();
    field_elevator_pending_destination_.reset();
    field_taxi_route_active_ = false;
    field_taxi_transition_active_ = false;
    field_taxi_pending_destination_.reset();
    field_taxi_cabbie_destination_.reset();
    field_phone_transition_active_ = false;
    field_phone_pending_destination_.reset();
    field_dialogue_scene_transition_active_ = false;
    field_dialogue_scene_pending_destination_.reset();
    field_party_roster_.clear();
    field_menu_text_ = FieldMenuTextCatalog{};
    command_overview_resource_variant_ = ResourceVariantState{};
    command_overview_combat_bars_ = EquipmentCombatBarResult{};
    command_overview_item_detail_ = ItemActionDetailResult{};
    command_overview_inventory_action_ = InventoryActionMenuState{};
    command_overview_selected_inventory_item_.reset();
    command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::inactive;
    command_overview_first_aid_targets_ = FirstAidTargetList{};
    command_overview_first_aid_cursor_ = 0U;
    command_overview_first_aid_message_ = FirstAidMessage::none;
    command_overview_automatic_use_state_ =
        CommandOverviewAutomaticUseState::inactive;
    command_overview_inventory_selector_suppressed_ = false;
    command_overview_discard_confirmation_ = MenuConfirmationState{};
    command_overview_inventory_trade_ = InventoryTradeState{};
    command_overview_action_message_ = CommandOverviewActionMessage::none;
    command_overview_inventory_action_active_ = false;
    command_overview_discard_confirmation_active_ = false;
    command_overview_clips_.close();
    field_joshua_archetype_selection_ = 0U;
    field_hud_slots_ = {};
    field_entity_animation_ = {};
    field_entity_animation_.increment = kFieldWalkAnimationIncrement;
    field_entity_animation_.period = kFieldEntityAnimationPeriod;
    field_entity_animation_id_ = "idle";
    field_player_animation_reset_pending_ = false;
    field_entity_animation_selector_ = 0U;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = false;
    input_ = integration::FieldSessionBootstrapInput{};
    input_.action_selection_profile_source =
        assets.action_selection_profile_source;
    input_.field_mode_selection_state_source =
        assets.field_mode_selection_state_source;
    input_.field_scene = *scene_input;
    if (restored_field_state != nullptr) {
        std::array<SceneRunnerPosition, 3> restored_positions{};
        for (std::size_t index = 0; index < restored_positions.size();
             ++index) {
            restored_positions[index] = SceneRunnerPosition{
                restored_field_state->runner_positions[index].x,
                restored_field_state->runner_positions[index].y};
        }
        input_.restored_field_scene = integration::RestoredFieldSceneInput{
            restored_field_state->world_mode,
            restored_field_state->camera_x,
            restored_field_state->camera_y,
            restored_positions};
    }

    field_entity_ = FieldEntityPresentation{};
    initialize_field_party_roster(startup_state);
    field_entity_.active = field_party_roster_.available();
    field_entity_.world_position =
        input_.field_scene.source_metadata.initial_field_records[0];
    if (field_entity_.active) {
        field_joshua_archetype_selection_ = static_cast<std::size_t>(
            startup_state.joshua_archetype.selected);
        field_entity_.projection.sprite_offset_y =
            active_runner_record()
                .projection_word_08;
    }
    // The controlled runner is copied from the selected 0x7291A archetype or
    // restored save record. Its live +0x10 flags gate field interactions in
    // 0xB7B8; 0x44BC's separate 0x05 default is for spawned world entities.
    field_entity_behavior_flags_ =
        field_entity_.active
            ? static_cast<std::uint8_t>(active_runner_record().flags >> 8U)
            : 0U;
    initialize_field_entity_selection_state();
    refresh_field_hud_runner_slot();

    const auto menu_text = load_field_menu_text_catalog(
        startup_state.asset_catalog.catalog, game_scripts_);
    if (!menu_text.complete()) {
        failure_ = menu_text.errors.empty()
                       ? "field menu text catalog is incomplete"
                       : menu_text.errors.front();
        source_boundary("field.bootstrap.menu_text_missing");
        return false;
    }
    field_menu_text_ = menu_text.catalog;
    if (!load_command_overview_menu_content()) {
        return false;
    }
    if (!load_hotel_menu_content()) {
        return false;
    }
    if (!load_pocket_secretary_menu_content()) {
        return false;
    }
    if (!load_ares_shop_menu_content()) {
        return false;
    }
    if (!load_item_menu_content()) {
        return false;
    }

    // Dialogue is optional during initial field bootstrap because the player
    // does not start inside an NPC prompt. Load it once here so the later
    // 0x1F65A callback can bind direct assets without rebuilding a catalog on
    // every prompt or reaching back into the ROM.
    (void)field_dialogue_.load_assets(startup_state.asset_catalog.catalog);
    field_mode_text_.set_script_runtime(&game_scripts_);
    (void)field_mode_text_.load_assets(startup_state.asset_catalog.catalog);
    if (!field_interaction_.load_assets(startup_state.asset_catalog.catalog)) {
        failure_ = field_interaction_.failure();
        source_boundary("field.bootstrap.interaction_assets_missing");
        return false;
    }

    const auto field_map_alias =
        std::string("field.maps.mode.") + std::to_string(scene_reference);
    const auto field_map_priority_overlay_alias =
        field_map_alias + ".priority.overlay";
    if (!load_image(startup_state.asset_catalog.catalog, field_map_alias,
                    field_map_) ||
        !load_image(startup_state.asset_catalog.catalog,
                    field_map_priority_overlay_alias,
                    field_map_priority_overlay_) ||
        !load_image(startup_state.asset_catalog.catalog, kFieldHudAlias,
                    field_hud_) ||
        !load_image(startup_state.asset_catalog.catalog,
                    "menu.field.service.background",
                    field_service_background_) ||
        !load_field_hud_overlays(startup_state.asset_catalog.catalog) ||
        !load_command_overview_assets(startup_state.asset_catalog.catalog) ||
        !load_field_weapon_spawn_assets(
            startup_state.asset_catalog.catalog) ||
        !load_field_elevator_selector_assets(
            startup_state.asset_catalog.catalog) ||
        !load_field_taxi_selector_assets(
            startup_state.asset_catalog.catalog)) {
        source_boundary("field.bootstrap.direct_assets_missing");
        return false;
    }
    if (field_map_priority_overlay_.size() != field_map_.size()) {
        failure_ = "field map priority overlay dimensions do not match map";
        source_boundary("field.bootstrap.field_map_priority_overlay_mismatch");
        return false;
    }
    for (std::size_t index = 0; index < field_service_tab_images_.size();
         ++index) {
        const auto alias = "menu.field.service.tabs.selected." +
                           std::to_string(index);
        if (!load_image(startup_state.asset_catalog.catalog, alias,
                        field_service_tab_images_[index])) {
            source_boundary("field.bootstrap.service_tab_asset_missing");
            return false;
        }
    }
    constexpr std::array<std::string_view, 8> kFieldServiceCyberwareIds{{
        "datajack", "cyber_eyes", "hand_razors", "spurs", "smartlink",
        "muscle_replacement", "dermal_plating", "wired_reflexes"}};
    for (std::size_t index = 0U;
         index < field_service_cyberware_icons_.size(); ++index) {
        const auto alias = "menu.field.service.cyberware." +
                           std::string(kFieldServiceCyberwareIds[index]);
        if (!load_image(startup_state.asset_catalog.catalog, alias,
                        field_service_cyberware_icons_[index])) {
            source_boundary("field.bootstrap.service_cyberware_icon_missing");
            return false;
        }
    }
    if (!load_image(startup_state.asset_catalog.catalog,
                    "menu.field.service.universal.clips",
                    field_service_universal_clips_icon_)) {
        source_boundary("field.bootstrap.service_clips_icon_missing");
        return false;
    }
    // The extracted HUD record is a plane-sized image: color zero is a hole
    // over the 272px playfield, but the source six-tile chrome column at
    // x=272..319 is an opaque panel.  Applying sprite transparency to the
    // whole bitmap makes that sidebar transparent over the map.
    make_field_hud_layer_transparent_outside_chrome(field_hud_);
    for (std::size_t direction = 0;
         direction < field_entity_idle_images_.size(); ++direction) {
        const auto alias = std::string(kFieldEntityIdleAliasPrefix) +
                           std::to_string(direction);
        if (!load_sprite_image(startup_state.asset_catalog.catalog, alias,
                               field_entity_idle_images_[direction],
                               field_entity_idle_origin_x_[direction],
                               field_entity_idle_origin_y_[direction])) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
        }
        const auto horizontal_flip =
            field_entity_animation_flips_horizontally(
                static_cast<std::uint8_t>(direction));
        if (horizontal_flip) {
            field_entity_idle_images_[direction] =
                flipped_image(field_entity_idle_images_[direction], Qt::Horizontal);
        }
        const auto attack_alias = std::string(kFieldEntityAttackAliasPrefix) +
                                  std::to_string(direction);
        if (!load_sprite_image(
                startup_state.asset_catalog.catalog, attack_alias,
                field_entity_attack_images_[direction],
                field_entity_attack_origin_x_[direction],
                field_entity_attack_origin_y_[direction])) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
        }
        if (horizontal_flip) {
            field_entity_attack_images_[direction] =
                flipped_image(field_entity_attack_images_[direction], Qt::Horizontal);
        }
        for (std::size_t frame = 0U; frame < 2U; ++frame) {
            const auto strike_alias =
                std::string(kFieldEntityStrikeAliasPrefix) +
                std::to_string(direction) + "." + std::to_string(frame);
            auto& strike = field_entity_strike_images_[direction][frame];
            if (!load_sprite_image(
                    startup_state.asset_catalog.catalog, strike_alias, strike,
                    field_entity_strike_origin_x_[direction][frame],
                    field_entity_strike_origin_y_[direction][frame])) {
                source_boundary("field.bootstrap.entity_assets_missing");
                return false;
            }
            if (horizontal_flip) {
                strike = flipped_image(strike, Qt::Horizontal);
            }
        }
        const auto cast_alias = std::string(kFieldEntityCastAliasPrefix) +
                                std::to_string(direction);
        if (!load_sprite_image(
                startup_state.asset_catalog.catalog, cast_alias,
                field_entity_cast_images_[direction],
                field_entity_cast_origin_x_[direction],
                field_entity_cast_origin_y_[direction])) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
        }
        if (horizontal_flip) {
            field_entity_cast_images_[direction] =
                flipped_image(field_entity_cast_images_[direction], Qt::Horizontal);
        }
        for (std::size_t frame = 0U; frame < 2U; ++frame) {
            const auto down_alias =
                std::string(kFieldEntityDownAliasPrefix) +
                std::to_string(direction) + "." + std::to_string(frame);
            auto& down = field_entity_down_images_[direction][frame];
            if (!load_sprite_image(
                    startup_state.asset_catalog.catalog, down_alias, down,
                    field_entity_down_origin_x_[direction][frame],
                    field_entity_down_origin_y_[direction][frame])) {
                source_boundary("field.bootstrap.entity_assets_missing");
                return false;
            }
            if (horizontal_flip) {
                down = flipped_image(down, Qt::Horizontal);
            }
        }
        // The ordinary movement poses come from the C062 positive-marker
        // list, extracted as separate host files. C0B2 is a different finite
        // behavior-transition sequence. Direction 5..7 descriptor records
        // set source attribute 0x0800; bake that facing into the native image
        // once when loading each typed direction set.
        std::size_t loaded_frames = 0;
        for (;;) {
            const auto walk_alias =
                std::string(kFieldEntityWalkAliasPrefix) +
                std::to_string(direction) + "." +
                std::to_string(loaded_frames);
            if (startup_state.asset_catalog.catalog.find(walk_alias) ==
                nullptr) {
                break;
            }
            QImage walk_image;
            int walk_origin_x = 0;
            int walk_origin_y = 0;
            if (!load_sprite_image(
                    startup_state.asset_catalog.catalog, walk_alias,
                    walk_image, walk_origin_x, walk_origin_y)) {
                source_boundary("field.bootstrap.entity_assets_missing");
                return false;
            }
            if (horizontal_flip) {
                walk_image = flipped_image(walk_image, Qt::Horizontal);
            }
            field_entity_walk_images_[direction].push_back(
                std::move(walk_image));
            field_entity_walk_origin_x_[direction].push_back(walk_origin_x);
            field_entity_walk_origin_y_[direction].push_back(walk_origin_y);
            ++loaded_frames;
        }
        if (loaded_frames != kFieldWalkAnimationFrameCount) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
        }
    }
    if (!load_field_default_actor_assets(
            startup_state.asset_catalog.catalog)) {
        source_boundary("field.bootstrap.default_actor_assets_missing");
        return false;
    }
    if (!load_dynamic_world_actor_artwork(
            startup_state.asset_catalog.catalog)) {
        source_boundary("field.bootstrap.dynamic_actor_assets_missing");
        return false;
    }
    const auto result = integration::bootstrap_field_session(
        input_, state_, callbacks_);
    if (result != integration::FieldSessionBootstrapResult::field_ready) {
        failure_ = std::string("field bootstrap did not reach field_ready: ") +
                   integration::field_session_bootstrap_phase_name(
                       state_.phase);
        source_boundary("field.bootstrap.incomplete");
        return false;
    }

    if (restored_field_state != nullptr) {
        // The restore path's live runner positions are separate from the
        // startup profile defaults. The conversion bootstrap publishes them
        // in typed scene metadata; the presenter consumes that state here.
        field_entity_.world_position =
            state_.field_scene_buffers.source_metadata.initial_field_records[0];
    }

    field_movement_state_.position_x = field_entity_.world_position.x;
    field_movement_state_.position_y = field_entity_.world_position.y;
    field_movement_state_.heading = 0U;
    // This is distinct from the live runner record's +0x10 flags word used by
    // menu/status code; movement and terrain probes consume the entity slot.
    field_movement_behavior_record_.active = field_entity_.active;
    field_movement_behavior_record_.behavior_flags =
        field_entity_behavior_flags_;
    field_movement_behavior_record_.current_heading = 0U;
    field_movement_behavior_record_.target_heading = 0U;
    // New Game enters 0x075E with FFDF40 still clear. The scene-derived
    // interaction mode is assigned later by 0x167AE, after a world-activity
    // handoff. The loaded-save path also clears FFDF40 in 0x594E0 before its
    // 0x03074/0x028C2 re-entry. Do not run the later scene-mode selector during
    // this initial bootstrap: scene 2's motel contact depends on mode 0 here.
    field_movement_transition_state_.world_mode = 0U;
    field_movement_transition_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_movement_transition_state_.field_origin_x =
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x);
    field_movement_transition_state_.world_activity_source_word = 0U;
    // ROM 0x59C5E leaves FFE18B set after the New Game opening. 0xACAE
    // consumes that startup state to begin the ordinary heading-zero field
    // continuation (0xB574); a restored save never takes this branch.
    field_opening_auto_walk_ = FieldOpeningAutoWalkState{};
    field_mode_entry_transition_pending_ = false;
    field_mode_entry_destination_scene_.reset();
    field_mode_entry_run_target_actor_group_.reset();
    field_indexed_title_service_result_ = 0;
    field_active_story_trigger_.reset();
    field_runtime_state_ = integration::FieldRuntimeCoordinatorState{};
    field_runtime_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.display_dirty_flags = state_.display_dirty_flags;
    world_frame_logic_state_ = WorldFrameLogicState{};
    world_frame_logic_state_.world_mode =
        state_.field_scene_buffers.scene_reference;
    field_entity_.direction =
        field_movement_direction_from_heading(field_movement_state_.heading);

    if (startup_state.new_game_opening_completed &&
        restored_field_state == nullptr) {
        field_map_event_state_flags_ |= field_map_event_flag(
            FieldMapEventStateFlag::new_game_opening_completed);
    }
    // The map image was loaded from the source layers; record them before
    // scene-entry effects can rewrite either layer.
    reset_field_map_cell_words();
    dispatch_field_map_scene_entry(scene_reference);

    ready_ = true;
    scene_music_latch_ = true;   // 0x2830 on the New Game / load entry
    if (field_opening_auto_walk_.active) {
        // Lets the host delay 0x2900's song by the 0x07F0 setup time.
        source_boundary("field.opening_auto_walk.scene_music");
    }
    service_scene_music_latch();
    if (field_opening_auto_walk_.active) {
        source_boundary("field.opening_auto_walk.armed");
    }
    source_boundary("field.bootstrap.ready");
    return true;
}

#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::bootstrap_for_scene_dump(
    const integration::StartupIntegrationState& startup_state,
    const std::uint16_t scene_reference) {
    if (scene_reference >= kFieldSceneProfileCount) {
        failure_ = "scene dump requested an out-of-range field scene";
        return false;
    }
    field_scene_reference_override_for_dump_ = scene_reference;
    return bootstrap(startup_state);
}
#endif // SHADOWRUN_QT_QA_DRIVER


void FieldSceneRuntime::set_controller_nibble(
    const std::uint8_t dpad_nibble) noexcept {
    set_controller_port_byte(static_cast<std::uint8_t>(
        (field_controller_port_byte_ & 0xF0U) | (dpad_nibble & 0x0FU)));
}

void FieldSceneRuntime::set_controller_port_byte(
    const std::uint8_t port_byte) noexcept {
    field_controller_port_byte_ = port_byte;
    field_controller_nibble_ = static_cast<std::uint8_t>(port_byte & 0x0FU);
}

#ifdef SHADOWRUN_QT_QA_DRIVER
void FieldSceneRuntime::begin_projected_field_transition_for_dump() noexcept {
    if (!ready_) {
        return;
    }
    // This is the typed output state published by the successful 0x1651E
    // projection in 0xB7B8. The QA route starts here so the recurring frame
    // loop, rather than a test-only timer, owns the following 17 VBlank steps.
    field_movement_transition_state_.interaction_transition_pending = true;
    field_movement_transition_state_.interaction_transition_timer = 0x10U;
    field_movement_transition_state_.world_interaction_lock = true;
    field_runtime_state_.display_service_word = 0xFFFFU;
    state_.display_service_word = 0xFFFFU;
    field_palette_fade_steps_ = 0U;
}
#endif // SHADOWRUN_QT_QA_DRIVER


std::optional<std::size_t> FieldSceneRuntime::projected_terrain_cell() const {
    const auto& profile = state_.field_scene_buffers.profile;
    const auto row_stride_bytes = profile[18U];
    if (row_stride_bytes == 0U) {
        return std::nullopt;
    }
    const auto offset = project_entity_y_axis(
        CoordinateProjectionInput{field_entity_.world_position.x,
                                   field_entity_.world_position.y,
                                   field_movement_state_.heading},
        row_stride_bytes);
    const auto byte_offset = offset + static_cast<std::int32_t>(row_stride_bytes);
    if (byte_offset < 0 || (byte_offset & 1) != 0) {
        return std::nullopt;
    }
    const auto word_index = static_cast<std::size_t>(byte_offset / 2);
    if (word_index >= state_.field_scene_buffers.layer_a.size()) {
        return std::nullopt;
    }
    return word_index;
}

bool FieldSceneRuntime::mark_field_terrain_cell(const std::size_t cell) {
    // ROM 0x16760: when the FF2700 word is not already 0x000B, increment
    // FFE179[FFF0C8], store 0x000B and set FFE100 so the next VBlank redraws
    // the normal viewport [C].
    constexpr std::uint16_t kMarkedTerrainWord = 0x000BU;
    auto& layer = state_.field_scene_buffers.layer_a;
    if (cell >= layer.size() || layer[cell] == kMarkedTerrainWord) {
        return false;
    }
    ++field_projected_terrain_mark_count_;
    layer[cell] = kMarkedTerrainWord;
    viewport_state_.normal_render_pending = 1U;
    // The Qt map is pre-rendered, so redraw the one 32x32 cell here.
    if (!repaint_field_map_cell(cell)) {
        source_boundary("field.terrain.mark_cell_without_donor");
    }
    return true;
}

std::size_t FieldSceneRuntime::apply_world_effect_layer_writes(
    const bool layer_b_target, const std::vector<MapWordWrite>& writes) {
    // World-effect writes land in FF4700 when the effect marks the Y layer
    // dirty, otherwise in FF2700 (caller sets FFE100/FFE102). The Qt map is
    // pre-rendered, so each changed cell is repainted from a donor cell that
    // already shows the new (FF2700, FF4700) pair, exactly as R3 does for
    // the 0x16760 terrain mark.
    auto& layer = layer_b_target ? state_.field_scene_buffers.layer_b
                                 : state_.field_scene_buffers.layer_a;
    std::size_t repainted = 0U;
    for (const auto& write : writes) {
        if (write.offset < 0 || (write.offset & 1) != 0) {
            source_boundary("field.world.layer_write_misaligned");
            continue;
        }
        const auto word_index = static_cast<std::size_t>(write.offset / 2);
        if (word_index >= layer.size()) {
            source_boundary("field.world.layer_write_out_of_range");
            continue;
        }
        layer[word_index] = write.value;
        if (word_index >= field_map_cell_words_.size()) {
            continue;   // outside the pre-rendered image (or no geometry)
        }
        const std::pair<std::uint16_t, std::uint16_t> shown =
            field_map_cell_words_[word_index];
        if (shown.first == state_.field_scene_buffers.layer_a[word_index] &&
            shown.second == state_.field_scene_buffers.layer_b[word_index]) {
            continue;   // the image already shows this pair
        }
        if (repaint_field_map_cell(word_index)) {
            ++repainted;
        } else {
            source_boundary("field.world.map_write_without_donor");
        }
    }
    return repainted;
}

void FieldSceneRuntime::reset_field_map_cell_words() {
    constexpr int kCellPixels = 32;
    field_map_cell_words_.clear();
    field_map_cell_columns_ = 0U;
    const auto& layer_a = state_.field_scene_buffers.layer_a;
    const auto& layer_b = state_.field_scene_buffers.layer_b;
    if (field_map_.isNull() || field_map_.width() % kCellPixels != 0 ||
        field_map_.height() % kCellPixels != 0) {
        return;
    }
    const auto columns =
        static_cast<std::size_t>(field_map_.width() / kCellPixels);
    const auto cells =
        columns * static_cast<std::size_t>(field_map_.height() / kCellPixels);
    const auto& profile = state_.field_scene_buffers.profile;
    const auto row_stride_bytes = profile.size() > 18U ? profile[18U] : 0U;
    if (cells > layer_a.size() || cells > layer_b.size() ||
        (row_stride_bytes != 0U && row_stride_bytes != columns * 2U)) {
        source_boundary("field.terrain.map_cell_geometry_mismatch");
        return;
    }
    field_map_cell_columns_ = columns;
    field_map_cell_words_.reserve(cells);
    for (std::size_t index = 0U; index < cells; ++index) {
        field_map_cell_words_.emplace_back(layer_a[index], layer_b[index]);
    }
}

bool FieldSceneRuntime::repaint_field_map_cell(const std::size_t cell) {
    constexpr int kCellPixels = 32;
    if (field_map_cell_columns_ == 0U || cell >= field_map_cell_words_.size()) {
        return false;
    }
    // Both words are read live: 0x16760 changes FF2700 and world effects can
    // change either layer.
    const std::pair<std::uint16_t, std::uint16_t> wanted{
        state_.field_scene_buffers.layer_a[cell],
        state_.field_scene_buffers.layer_b[cell]};
    std::optional<std::size_t> donor;
    for (std::size_t index = 0U; index < field_map_cell_words_.size();
         ++index) {
        if (index != cell && field_map_cell_words_[index] == wanted) {
            donor = index;
            break;
        }
    }
    if (!donor.has_value()) {
        return false;
    }
    const auto cell_rect = [this](const std::size_t index) {
        return QRect(
            static_cast<int>(index % field_map_cell_columns_) * kCellPixels,
            static_cast<int>(index / field_map_cell_columns_) * kCellPixels,
            kCellPixels, kCellPixels);
    };
    const auto source = cell_rect(*donor);
    const auto target = cell_rect(cell);
    for (QImage* image : {&field_map_, &field_map_priority_overlay_}) {
        if (image->isNull()) {
            continue;
        }
        if (image->format() == QImage::Format_Indexed8 ||
            image->format() == QImage::Format_Mono ||
            image->format() == QImage::Format_MonoLSB) {
            // QPainter cannot target palette images; keep the pixels.
            *image = image->convertToFormat(image->hasAlphaChannel()
                                                ? QImage::Format_ARGB32
                                                : QImage::Format_RGB32);
        }
        const auto block = image->copy(source);
        QPainter painter(image);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(target.topLeft(), block);
    }
    field_map_cell_words_[cell] = wanted;
    return true;
}

void FieldSceneRuntime::record_field_activity_start(
    const MovementModeTransitionContinuation continuation) {
    field_activity_continuation_ = continuation;
    if (continuation ==
            MovementModeTransitionContinuation::maglock_door_interaction ||
        continuation ==
            MovementModeTransitionContinuation::building_security_terminal ||
        continuation ==
            MovementModeTransitionContinuation::event_30_world_activity ||
        continuation ==
            MovementModeTransitionContinuation::event_31_world_activity ||
        continuation ==
            MovementModeTransitionContinuation::event_27_world_activity ||
        continuation ==
            MovementModeTransitionContinuation::event_25_world_activity ||
        continuation ==
            MovementModeTransitionContinuation::event_29_world_activity ||
        continuation == MovementModeTransitionContinuation::
                           scene_37_antechamber_entry ||
        continuation == MovementModeTransitionContinuation::
                           scene_55_tomb_credits_callback ||
        continuation == MovementModeTransitionContinuation::
                           mode_table_world_activity) {
        field_runtime_state_.pending_field_scene_handler = true;
    }
    // ROM 0x1E2E0/0x1E2EC stores the typed continuation in the activity
    // timer. Scene-table selection belongs to the later 0x167AE dispatcher;
    // it is not a return value of the timer helper.
    source_boundary("field.world.activity.started");
}

#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::dispatch_field_interaction_through_coordinator_for_dump(
    const FieldInteractionPrompt prompt,
    const std::uint8_t interaction_selection,
    const bool interaction_suppressed,
    const std::optional<std::uint8_t> security_floor_index) {
    if (!ready_ || field_interaction_.open()) {
        return false;
    }
    MovementModeTransitionContinuation continuation =
        MovementModeTransitionContinuation::none;
    switch (prompt) {
    case FieldInteractionPrompt::electromagnetic_lock_door:
        continuation =
            MovementModeTransitionContinuation::maglock_door_interaction;
        break;
    case FieldInteractionPrompt::building_security_terminal:
        continuation =
            MovementModeTransitionContinuation::building_security_terminal;
        break;
    case FieldInteractionPrompt::electromagnetic_lock_resolution:
        return false;
    }
    field_movement_transition_state_.activity_selection =
        interaction_selection;
    field_movement_transition_state_.terrain_event_suppressed =
        interaction_suppressed;
    field_security_floor_index_ = security_floor_index;

    // Inject the typed continuation at the point after collision/event
    // resolution. Use the production activity scheduler, pending-handler bit,
    // and field coordinator to enter the same modal route as gameplay.
    if (!begin_world_activity_if_idle(
            field_world_activity_state_,
            static_cast<std::uint16_t>(continuation))) {
        return false;
    }
    world_frame_logic_state_.resource_countdown =
        field_world_activity_state_.timeout;
    record_field_activity_start(continuation);
    advance_frame();
    return field_interaction_.open();
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::initialize_world_activity_for_dump(
    const std::uint8_t mode_event_code,
    const std::uint16_t random_seed) {
    if (!ready_ || !field_session_source_assets_ || !field_asset_catalog_) {
        return false;
    }

    // This route injects the already-resolved FFFBF9 byte at the 0x167AE
    // dispatcher. The normal movement/collision input that publishes it is
    // deliberately outside this deterministic scene-selection check.
    field_movement_transition_state_.mode_event_code = mode_event_code;
    field_random_seed_ = random_seed;
    field_runtime_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.scene_runtime_state = 0U;
    field_runtime_state_.field_scene_gate = 0U;
    field_runtime_state_.field_transition_requested = false;
    field_runtime_state_.scene_refresh_marker = 0;

    // 0x167AE consumes FFFBF9 as an active-map table index. This focused QA
    // injects that byte directly, outside the collision-driven building
    // interaction branch, so resolve the same active-map table through Lua.
    const auto& event_metadata = state_.field_scene_buffers.source_metadata;
    const auto event_route = game_scripts_.resolve_field_location_route(
        FieldScriptRouteContext{
            static_cast<std::uint8_t>(field_runtime_state_.scene_id),
            field_exit_mode_value(FieldExitMode::world_activity_table),
            field_collision_event_code_from_id(mode_event_code),
            event_metadata.event_table_available,
            event_metadata.event_mode_table});
    if (!event_route.has_value() ||
        event_route->kind != FieldScriptRouteKind::area_event ||
        event_route->area_event_destination_mode !=
            WorldActivityMode::resource_activity ||
        !event_route->area_event_target.has_value()) {
        if (event_route.has_value()) {
            failure_ =
                "injected world-activity event did not resolve to a "
                "resource area (scene=" +
                std::to_string(field_runtime_state_.scene_id) + ", mode=" +
                std::to_string(field_runtime_state_.field_mode) +
                ", event=" + std::to_string(mode_event_code) +
                ", route-kind=" +
                std::to_string(static_cast<int>(event_route->kind)) +
                ", target=" +
                (event_route->area_event_target.has_value()
                     ? std::to_string(*event_route->area_event_target)
                     : "none") +
                ")";
        } else {
            failure_ = game_scripts_.failure();
        }
        source_boundary("field.world_activity.qa_route_unresolved");
        return false;
    }
    active_field_script_route_ = *event_route;

    field_runtime_callbacks_.initialize_world_activity_mode(
        field_runtime_state_);
    if (field_runtime_state_.field_transition_requested ||
        world_activity_routine_state_.mode !=
            WorldActivityMode::resource_activity ||
        !field_security_floor_index_.has_value()) {
        return false;
    }

    // 0x0A54 supplies the nonzero marker consumed by 0x2BD2's scene/profile
    // re-entry branch. Exercise that normal preparation path after the
    // dispatcher has selected and prepared the typed resource scene.
    field_runtime_state_.scene_refresh_marker = -1;
    advance_field_scene_transition_after_modal();
    return state_.field_scene_buffers.scene_reference ==
               world_activity_routine_state_.scene_id &&
           field_security_floor_index_.value() <
               world_activity_routine_state_.resource_random_index_bound;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::return_from_world_activity_for_dump() {
    if (!ready_ || !world_activity_return_snapshot_.has_value()) {
        return false;
    }
    const auto source_scene =
        world_activity_return_snapshot_->scene_buffers.scene_reference;
    field_runtime_state_.field_transition_requested = true;
    advance_frame();
    return !world_activity_return_snapshot_.has_value() &&
           !field_runtime_state_.field_transition_requested &&
           state_.field_scene_buffers.initialized &&
           state_.field_scene_buffers.scene_reference == source_scene;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::dispatch_selected_security_terminal_for_dump() {
    if (!ready_ || !field_security_floor_index_.has_value() ||
        world_activity_routine_state_.mode !=
            WorldActivityMode::resource_activity ||
        field_interaction_.open()) {
        return false;
    }
    field_activity_continuation_ =
        MovementModeTransitionContinuation::building_security_terminal;
    field_runtime_state_.pending_field_scene_handler = true;
    field_runtime_state_.field_activity_lock = false;
    field_runtime_state_.world_activity_countdown = 0U;
    field_runtime_state_.display_service_word = 0U;
    advance_frame();
    return field_interaction_.open() &&
           field_interaction_.panel().option_count == 2U;
}
#endif // SHADOWRUN_QT_QA_DRIVER


bool FieldSceneRuntime::recover_blocked_field_movement(
    const MovementProbeResult& initial_probe) {
    field_movement_transition_state_.terrain_collision_code =
        static_cast<std::uint16_t>(initial_probe.collision_code);
    field_movement_behavior_record_.current_heading =
        field_movement_state_.heading;
    field_movement_behavior_record_.target_heading = field_target_heading_;
    for (std::size_t index = 0;
         index < field_movement_transition_state_.directional_probe_signs.size();
         ++index) {
        field_movement_transition_state_.directional_probe_signs[index] =
            static_cast<std::int8_t>(field_terrain_sample_state_[index]);
    }
    field_movement_transition_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_movement_transition_state_.field_origin_x =
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x);

    EntityMovementModeTransitionCallbacks transition_callbacks{};
    transition_callbacks.build_collision_probe_points = [this](
                                                               EntityBehaviorFrameRecord&) {
        const std::array<std::uint8_t, 3> probe_bytes{
            field_terrain_sample_state_[0], field_terrain_sample_state_[1],
            field_terrain_sample_state_[2]};
        // ROM 0xB834 -> 0xBC36 at every door contact. Lua world_links picks
        // the door row from the probe codes and, in the outdoor field,
        // returns the FFDF44..4E party points and FFDF50/52 camera. The
        // host only mirrors that RAM record; it has no copy of the math.
        record_field_door_contact(probe_bytes);
        return MovementProjectionPreparation{
            field_collision_probe_state_.selected_offset_bank};
    };
    transition_callbacks.quantize_field_delta =
        [](const std::int16_t target, const std::uint16_t origin) {
            return quantize_field_delta(target, origin);
        };
    transition_callbacks.dispatch_field_map_event =
        [this](const FieldMapCollisionEventInput& input) {
            auto collision_input = input;
            collision_input.maglocks_disabled =
                field_matrix_field_state_.maglocks_off;
            const auto decision =
                game_scripts_.resolve_field_collision_event(collision_input);
            if (!decision) {
                failure_ = game_scripts_.failure();
                source_boundary("field.map_event.lua_policy_failed");
                return std::optional<FieldMapEventPolicyDecision>{};
            }
            const auto dispatch =
                field_map_event_system_.record_collision(collision_input,
                                                        *decision);
            source_boundary("field.map_event.collision_dispatched");
            return std::optional<FieldMapEventPolicyDecision>{
                dispatch.decision};
        };
    transition_callbacks.begin_world_activity_if_idle =
        [this](const MovementModeTransitionContinuation continuation) {
            const auto started = begin_world_activity_if_idle(
                field_world_activity_state_,
                static_cast<std::uint16_t>(continuation));
            if (started) {
                world_frame_logic_state_.resource_countdown =
                    field_world_activity_state_.timeout;
                record_field_activity_start(continuation);
            }
            return started;
        };
    transition_callbacks.begin_world_activity =
        [this](const MovementModeTransitionContinuation continuation) {
            begin_world_activity(
                field_world_activity_state_,
                static_cast<std::uint16_t>(continuation));
            world_frame_logic_state_.resource_countdown =
                field_world_activity_state_.timeout;
            record_field_activity_start(continuation);
        };
    transition_callbacks.mark_projected_terrain_cell = [this] {
        // ROM 0x16760 marks the projected field work cell in the active
        // terrain layer. Keep this bridge local: the older menu conversion
        // header owns a legacy TerrainWorkEntry name that is unrelated to
        // this runtime's source-backed field scene state.
        const auto cell = projected_terrain_cell();
        return cell.has_value() && mark_field_terrain_cell(*cell);
    };
    transition_callbacks.project_entity_world_interaction = [this] {
        return project_field_world_interaction();
    };
    transition_callbacks.prepare_mode_entry_service = [this] {
        prepare_audio_mode_entry(callbacks_.mode_entry_audio);
    };

    const auto result = advance_entity_movement_and_mode_transition(
        field_movement_behavior_record_, initial_probe.candidate_x,
        initial_probe.candidate_y, field_movement_transition_state_,
        transition_callbacks);
    last_field_movement_transition_result_ = result;
    if (result.mode_event_code_published) {
        source_boundary("field.world.activity.mode_event_published");
    }
    if (result.kind ==
        EntityMovementModeTransitionResultKind::projected_interaction_pending) {
        state_.display_service_word = 0xFFFFU;
        field_runtime_state_.display_service_word = 0xFFFFU;
        field_palette_fade_steps_ = 0U;
    }
    source_boundary("field.movement.blocked_transition");
    return (field_movement_behavior_record_.route_flags & 0x01U) != 0U;
}

std::int16_t FieldSceneRuntime::evaluate_field_collision(
    const std::uint16_t candidate_x,
    const std::uint16_t candidate_y) {
    return evaluate_field_collision(candidate_x, candidate_y,
                                    field_collision_record_,
                                    field_terrain_sample_state_,
                                    field_entity_behavior_flags_, true);
}

std::int16_t FieldSceneRuntime::evaluate_field_collision(
    const std::uint16_t candidate_x,
    const std::uint16_t candidate_y,
    EntityCollisionRecord& entity_collision_record,
    std::array<std::uint8_t, 4>& sample_state,
    const std::uint8_t entity_behavior_flags,
    const bool publish_movement_state,
    std::uint16_t* terrain_collision_code_output,
    bool* terrain_boundary_active_output,
    bool* outside_bounds_output) {
    const auto collision_evaluator =
        [this, &sample_state, entity_behavior_flags, publish_movement_state,
         terrain_collision_code_output, terrain_boundary_active_output,
         outside_bounds_output](
            const std::uint16_t evaluated_x,
            const std::uint16_t evaluated_y) {
            const auto& metadata = state_.field_scene_buffers.source_metadata;
            const TerrainCollisionBounds bounds{
                metadata.terrain_x_inclusive_lower,
                metadata.terrain_x_exclusive_upper,
                metadata.terrain_y_inclusive_lower,
                metadata.terrain_y_exclusive_upper};
            const auto result = evaluate_terrain_collision(
                TerrainCollisionInput{evaluated_x,
                                      evaluated_y,
                                      entity_behavior_flags,
                                      bounds,
                                      sample_state},
                TerrainCollisionCallbacks{
                    [this](const std::uint16_t normalized_x,
                           const std::uint16_t normalized_y,
                           const bool expanded_probe) {
                        return sample_terrain_collision_tiles(
                            normalized_x, normalized_y, expanded_probe,
                            [this](const std::uint16_t tile_index) {
                                const auto& collision_map =
                                    state_.field_scene_buffers.source_metadata
                                        .terrain_collision_map;
                                if (tile_index >= collision_map.size()) {
                                    return static_cast<std::uint8_t>(0x7FU);
                                }
                                return collision_map[tile_index];
                            });
                    },
                    [this](const std::uint8_t object_id,
                           const std::int16_t object_x,
                           const std::int16_t object_y) {
                        const auto& rectangles =
                            state_.field_scene_buffers.source_metadata
                                .terrain_collision_rectangles;
                        if (object_id >= rectangles.size()) {
                            source_boundary(
                                "field.movement.terrain_object_record_missing");
                            return true;
                        }
                        const auto object_lookup =
                            [&rectangles](const std::uint16_t lookup_id) {
                                if (lookup_id >= rectangles.size()) {
                                    return TerrainObjectBounds{};
                                }
                                const auto& record = rectangles[lookup_id];
                                return TerrainObjectBounds{
                                    static_cast<std::int16_t>(record.left),
                                    static_cast<std::int16_t>(record.right),
                                    static_cast<std::int16_t>(record.top),
                                    static_cast<std::int16_t>(record.bottom)};
                            };
                        return contains_terrain_object_bounds(
                            object_id, object_x, object_y, object_lookup);
                    },
                    {}});
            if (terrain_boundary_active_output != nullptr) {
                *terrain_boundary_active_output =
                    result.state.directional_collision_marker;
            }
            if (outside_bounds_output != nullptr) {
                *outside_bounds_output = result.state.outside_bounds_marker;
            }
            if (publish_movement_state &&
                field_terrain_probe_trace_enabled_) {
                constexpr std::array<std::pair<std::int16_t, std::int16_t>, 4>
                    source_corner_offsets{{
                        {-12, -7}, {12, -7}, {-12, 7}, {12, 7}}};
                const auto normalized_x = static_cast<std::uint16_t>(
                    evaluated_x >> 4U);
                const auto normalized_y = static_cast<std::uint16_t>(
                    evaluated_y >> 4U);
                FieldTerrainProbeTrace trace{};
                trace.candidate_x_fixed = evaluated_x;
                trace.candidate_y_fixed = evaluated_y;
                trace.normalized_x = normalized_x;
                trace.normalized_y = normalized_y;
                for (std::size_t index = 0U;
                     index < source_corner_offsets.size(); ++index) {
                    trace.corner_x[index] = static_cast<std::uint16_t>(
                        static_cast<std::int32_t>(normalized_x) +
                        source_corner_offsets[index].first);
                    trace.corner_y[index] = static_cast<std::uint16_t>(
                        static_cast<std::int32_t>(normalized_y) +
                        source_corner_offsets[index].second);
                }
                trace.terrain_bytes = result.state.sample.values;
                trace.selected_tile_index =
                    result.state.sample.selected_tile_index;
                trace.raw_terrain_code = result.state.raw_terrain_code;
                trace.collision_result = result.code;
                trace.expanded_probe = (entity_behavior_flags & 0x04U) != 0U;
                trace.outside_bounds = result.state.outside_bounds_marker;
                field_terrain_probe_trace_.push_back(trace);
            }
            sample_state = field_sample_state_from_result(
                result.packed_sample_state);
            if (terrain_collision_code_output != nullptr &&
                !result.state.outside_bounds_marker) {
                *terrain_collision_code_output = static_cast<std::uint16_t>(
                    result.state.raw_terrain_code);
            }
            // ROM 0xE7C8 clears and republishes both globals for every probe.
            // 0x14F50 reads FFDFE2 only after this result is available.
            if (publish_movement_state) {
                field_terrain_boundary_active_ =
                    result.state.directional_collision_marker;
                field_terrain_outside_bounds_marker_ =
                    result.state.outside_bounds_marker;
            }
            // ROM 0xE83E publishes the raw in-bounds terrain sample to
            // FFDF64 before its collision classifiers adjust the return code.
            // 0xAC5E consumes this value on the next entity pass to release
            // B7B8's one-shot bit after the runner has moved back onto clear
            // terrain. Outside-map returns leave FFDF64 unchanged.
            if (publish_movement_state &&
                !result.state.outside_bounds_marker) {
                field_movement_transition_state_.terrain_collision_code =
                    static_cast<std::uint16_t>(
                        result.state.raw_terrain_code);
            }
            // Outside the map, 0xE7C8 returns before 0xE8A0, so FFDFE4 stays
            // at the previous sample.
            if (publish_movement_state &&
                !result.state.outside_bounds_marker) {
                field_collision_marker_index_ =
                    result.state.sample.selected_tile_index;
                // ROM 0xE8A0 publishes the selected map byte as FFDFE4.
                // B994 copies it to FFE11A when a mode-2 event is entered;
                // 0x2C1A then uses that index to advance a negative result.
                if (publish_movement_state) {
                    field_movement_transition_state_.world_activity_source_word =
                        field_collision_marker_index_;
                }
            }
            return CollisionEvaluation{
                result.code, result.packed_sample_state};
        };
    return evaluate_entity_collision(entity_collision_record, candidate_x,
                                     candidate_y, collision_evaluator);
}

std::optional<scripting::ScriptActorBehaviorDecision>
FieldSceneRuntime::heartbeat_field_player(
    const scripting::ScriptActorHeartbeatPhase phase) {
    if (!field_entity_.active) {
        return std::nullopt;
    }
    const bool movement_transition_paused =
        field_opening_auto_walk_.active &&
        field_movement_transition_state_.interaction_transition_pending;
    const bool script_movement_allowed =
        phase == scripting::ScriptActorHeartbeatPhase::behavior && ready_ &&
        !movement_transition_paused &&
        (field_opening_auto_walk_.active || field_controller_nibble_ != 0U);
    scripting::ScriptActorBehaviorContext actor_context{};
    actor_context.profile_id = "field_runner_k03_none00";
    actor_context.entity_kind = 3U;
    actor_context.action_kind = scripting::ScriptActorActionKind::none;
    actor_context.instance_id = "field_player_runner";
    actor_context.walk_fast_enabled = field_walk_fast_enabled_;
    actor_context.source_record_slot = static_cast<std::uint8_t>(
        field_party_roster_.active_slot());
    actor_context.movement_scene_id =
        state_.field_scene_buffers.scene_reference;
    actor_context.initial_heading =
        phase == scripting::ScriptActorHeartbeatPhase::render
            ? field_movement_state_.animation_heading
            : field_movement_state_.heading;
    actor_context.spawn = !field_player_actor_script_initialized_;
    actor_context.movement_allowed = script_movement_allowed;
    actor_context.phase = phase;
    if (phase == scripting::ScriptActorHeartbeatPhase::behavior) {
        const bool target_selection_pressed = ready_ &&
            (field_controller_sample_state_.events & 0x1000U) != 0U;
        if (target_selection_pressed) {
            refresh_world_entity_service_targets();
            actor_context.target_selection_requested = true;
            actor_context.target_selection_advances_cursor = true;
            actor_context.target_selection_special_marker =
                field_entity_target_owner_.special_marker;
            actor_context.target_selection_mode =
                field_entity_target_owner_.mode;
            const auto copy_slot = [](const std::optional<std::size_t> slot)
                -> std::optional<std::uint8_t> {
                if (!slot.has_value() || *slot > 0xFFU) {
                    return std::nullopt;
                }
                return static_cast<std::uint8_t>(*slot);
            };
            actor_context.target_selection_current_slot = copy_slot(
                field_entity_target_owner_.selected_target);
            actor_context.target_selection_linked_slot = copy_slot(
                field_entity_target_owner_.linked_target);
            actor_context.target_selection_owner_slot = copy_slot(
                field_entity_target_owner_.owner_record);

            actor_context.world_target_selection_candidates.reserve(
                field_entity_target_records_.size());
            for (std::size_t slot = 0U;
                 slot < field_entity_target_records_.size(); ++slot) {
                const auto& source = field_entity_target_records_[slot];
                scripting::ScriptActorTargetCandidate candidate{};
                candidate.source_record_slot =
                    static_cast<std::uint8_t>(slot);
                candidate.descriptor_class = source.descriptor_class;
                candidate.service_flags = static_cast<std::uint8_t>(
                    (source.active ? 0x01U : 0U) |
                    (source.service_reserved ? 0x10U : 0U) |
                    (source.effect_locked ? 0x40U : 0U));
                candidate.interaction_flags = source.selected ? 0x10U : 0U;
                candidate.behavior_flags = source.high_effect ? 0x80U : 0U;
                candidate.has_action = source.has_action;
                const auto actor = std::find_if(
                    field_default_actors_.begin(),
                    field_default_actors_.end(),
                    [this, slot](const auto& value) {
                        return value.active &&
                            value.scene_reference ==
                                state_.field_scene_buffers.scene_reference &&
                            value.source_record_slot.has_value() &&
                            static_cast<std::size_t>(
                                *value.source_record_slot) == slot;
                    });
                if (actor != field_default_actors_.end()) {
                    candidate.id = actor->actor_instance_id;
                    candidate.world_x = actor->world_position.x;
                    candidate.world_y = actor->world_position.y;
                    candidate.heading = actor->movement_state.heading;
                    candidate.interaction_flags =
                        actor->source_interaction_flags;
                    candidate.behavior_flags =
                        actor->source_behavior_flags;
                    candidate.spawn_state_flags =
                        actor->source_spawn_state_flags;
                    candidate.world_activity_actor =
                        actor->temporary_world_encounter ||
                        actor->world_activity_script_actor;
                }
                actor_context.world_target_selection_candidates.push_back(
                    std::move(candidate));
            }
        }
        const auto& runner = active_runner_record();
        const bool valid_weapon_slot =
            runner.equipped_inventory_slot >= 0 &&
            static_cast<std::size_t>(runner.equipped_inventory_slot) <
                runner.inventory_phase_bytes.size();
        const bool weapon_action_available =
            (runner.flags & 0x0300U) == 0x0300U && valid_weapon_slot &&
            classify_field_combat_action(
                runner.equipped_inventory_slot,
                runner.equipped_item_code) ==
                FieldCombatActionKind::weapon_ammo &&
            weapon_template_for_item_code(runner.equipped_item_code) !=
                nullptr;
        actor_context.combat_confirm_held =
            (field_controller_sample_state_.held & 0x0040U) != 0U;
        actor_context.combat_action_ready =
            runner.animation_action_gate < 0;
        actor_context.combat_action_available = weapon_action_available;
        actor_context.combat_input_blocked =
            field_combat_input_suppressed_until_release_;
        // ROM 0xAD86: negative +0x56 selects the 0xAE6E spell branch. The
        // B-selected record is the +0xA2 link it aims at.
        actor_context.player_spell_available =
            (runner.flags & 0x0300U) == 0x0300U &&
            classify_field_combat_action(runner.equipped_inventory_slot,
                                         runner.equipped_item_code) ==
                FieldCombatActionKind::magic_spell;
        // ROM 0xAD86 -> 0xB1DC: non-negative +0x56 and +0x57 == 0.
        actor_context.player_melee_available =
            (runner.flags & 0x0300U) == 0x0300U &&
            runner.equipped_inventory_slot >= 0 &&
            runner.equipped_item_code == 0U;
        actor_context.target_heading_audio_alternate =
            (field_random_seed_ & 0x8000U) != 0U;
        if (actor_context.player_spell_available ||
            actor_context.player_melee_available) {
            actor_context.player_spell_code = runner.equipped_item_code;
            actor_context.action_profile_modifiers =
                script_action_profile_modifiers(
                    state_.action_selection_profiles);
            actor_context.action_attribute_selectors =
                script_action_attribute_selectors();
            actor_context.world_effect_slot_occupancy =
                field_world_effect_slots_occupied_;
            const auto selected = field_entity_target_owner_.selected_target;
            const auto target = !selected.has_value()
                ? field_default_actors_.end()
                : std::find_if(
                      field_default_actors_.begin(),
                      field_default_actors_.end(),
                      [this, &selected](const auto& actor) {
                          return actor.active &&
                              actor.scene_reference ==
                                  state_.field_scene_buffers
                                      .scene_reference &&
                              actor.source_record_slot.has_value() &&
                              static_cast<std::size_t>(
                                  *actor.source_record_slot) == *selected;
                      });
            const auto target_slot = target == field_default_actors_.end()
                ? std::size_t{0U}
                : static_cast<std::size_t>(*target->source_record_slot);
            if (target != field_default_actors_.end() && target_slot >= 3U &&
                target_slot <= 0xFFU) {
                scripting::ScriptActorTargetCandidate candidate{};
                candidate.id = target->actor_instance_id;
                candidate.world_x = target->world_position.x;
                candidate.world_y = target->world_position.y;
                // ROM 0x15D3C self->link bearing (strike/spell aim) and
                // 0x15C58 distance via 0x14D58; +0x2E kept separately.
                const auto link = measure_entity_target_heading_and_distance(
                    field_entity_.world_position.x,
                    field_entity_.world_position.y,
                    target->world_position.x, target->world_position.y);
                candidate.heading = static_cast<std::uint16_t>(link.heading);
                candidate.distance = static_cast<std::uint16_t>(link.distance);
                candidate.source_record_heading =
                    target->movement_state.heading;
                // ROM 0x15C58 tests the record's real +0x10 bits 0 and 6.
                candidate.service_flags = target->source_record_flags;
                candidate.interaction_flags =
                    target->source_interaction_flags;
                candidate.behavior_flags = target->source_behavior_flags;
                candidate.spawn_state_flags =
                    target->source_spawn_state_flags;
                // Slots 3..9 are world records, never party lanes.
                candidate.world_activity_actor = true;
                candidate.source_record_slot =
                    static_cast<std::uint8_t>(target_slot);
                candidate.combatant = target->combatant;
                actor_context.ai_target_candidates.push_back(
                    std::move(candidate));
            }
            actor_context.action_line_clear = [this, &runner](
                const std::uint16_t target_x,
                const std::uint16_t target_y) {
                const SpawnEntityParentState parent{
                    field_entity_.world_position.x,
                    field_entity_.world_position.y,
                    field_movement_state_.heading,
                    static_cast<std::uint8_t>(runner.flags >> 8U),
                    static_cast<std::uint8_t>(runner.flags & 0x00FFU),
                    0U,
                    0U};
                return probe_action_line_clear(
                    parent, field_party_roster_.active_slot(), target_x,
                    target_y);
            };
        }
    }
    if (phase == scripting::ScriptActorHeartbeatPhase::render) {
        const bool runner_down =
            (active_runner_record().flags & kRunnerFlagsDown) != 0U;
        actor_context.animation_mode = runner_down
            ? scripting::ScriptActorAnimationMode::runner_down
            : field_entity_attack_pose_active_
            ? (field_entity_attack_pose_kind_ == kFieldPlayerPoseStrike
                   ? scripting::ScriptActorAnimationMode::strike
                   : field_entity_attack_pose_kind_ == kFieldPlayerPoseCast
                         ? scripting::ScriptActorAnimationMode::cast
                         : scripting::ScriptActorAnimationMode::attack)
            : field_entity_walking_
                  ? scripting::ScriptActorAnimationMode::walking
                  : scripting::ScriptActorAnimationMode::idle;
        if (runner_down) {
            // ROM 0xB35C: C0B2 direction = round(+0x2E / 0x2000) & 7.
            actor_context.animation_direction_override =
                static_cast<std::uint8_t>(
                    ((field_movement_state_.heading + 0x1000U) >> 13U) & 7U);
        } else if (field_entity_attack_pose_active_) {
            actor_context.animation_direction_override =
                field_entity_attack_direction_;
        } else if (field_entity_animation_selector_ <= 0x001CU &&
                   (field_entity_animation_selector_ & 0x0003U) == 0U) {
            // ROM 0xB442 writes the C052 idle selector from +0x2E but leaves
            // the separate +0x36 sprite heading untouched. Keep Lua's idle
            // facing bound to the selected C052 list after that reset.
            actor_context.animation_direction_override =
                static_cast<std::uint8_t>(
                    field_entity_animation_selector_ / 4U);
        }
        if (field_player_animation_reset_pending_) {
            actor_context.animation_accumulator_override =
                field_entity_animation_.accumulator;
        }
    }
    actor_context.controller_nibble = field_controller_nibble_;
    actor_context.controller_heading_table =
        field_controller_heading_table_;
    if (field_opening_auto_walk_.active && !movement_transition_paused) {
        actor_context.movement_heading_override =
            field_opening_auto_walk_.target_heading;
    }
    if (phase == scripting::ScriptActorHeartbeatPhase::behavior) {
        actor_context.movement_state = scripting::ScriptMovementStepState{
            field_movement_state_.position_x,
            field_movement_state_.position_y,
            field_movement_state_.heading,
            field_movement_state_.animation_heading,
            field_movement_state_.collision_context,
            field_movement_state_.movement_substate,
            field_movement_state_.route_status,
            field_movement_behavior_record_.route_flags,
            field_movement_state_.movement_flags,
            field_movement_state_.collision_flags,
            field_movement_state_.event_flags,
            field_movement_state_.collision_extent,
            field_movement_transition_state_.terrain_collision_code};
        for (std::size_t index = 0;
             index < field_movement_direction_offsets_.size(); ++index) {
            actor_context.movement_offsets[index] =
                scripting::ScriptMovementOffset{
                    field_movement_direction_offsets_[index].x,
                    field_movement_direction_offsets_[index].y};
        }
        actor_context.movement_idle_selectors =
            kFieldIdleAnimationSelectors;
        actor_context.movement_entity_flags = field_entity_behavior_flags_;
        actor_context.movement_transition_active = false;
        actor_context.movement_scene_id =
            state_.field_scene_buffers.scene_reference;
        actor_context.movement_collision_mode =
            field_movement_transition_state_.world_mode;
        actor_context.movement_services_available = script_movement_allowed;
        if (script_movement_allowed) {
            actor_context.movement_collision =
                [this](const std::uint16_t x, const std::uint16_t y) {
                    const auto collision_code =
                        evaluate_field_collision(x, y);
                    return scripting::ScriptMovementCollisionResult{
                        collision_code,
                        field_movement_transition_state_.terrain_collision_code};
                };
            actor_context.movement_route_lookup =
                [this](const std::uint16_t route_key) {
                    if ((route_key & 0x03U) != 0U ||
                        route_key / 4U >= field_movement_routes_.size()) {
                        source_boundary(
                            "field.movement.route_key_out_of_range");
                        throw std::runtime_error(
                            "Lua requested a movement route outside the source table");
                    }
                    const auto route_index =
                        static_cast<std::size_t>(route_key / 4U);
                    const auto& route_source =
                        field_movement_route_sources_[route_index];
                    const auto sample_value = [this](
                                                  const std::uint8_t sample_index) {
                        if (sample_index >=
                            field_terrain_sample_state_.size()) {
                            source_boundary(
                                "field.movement.route_sample_out_of_range");
                            throw std::runtime_error(
                                "movement route sample index is outside source state");
                        }
                        return static_cast<std::int8_t>(
                            field_terrain_sample_state_[sample_index]);
                    };
                    const auto& route = field_movement_routes_[route_index];
                    return scripting::ScriptMovementRouteFacts{
                        sample_value(route_source.primary_sample_index),
                        sample_value(route_source.secondary_sample_index),
                        route.primary_fallback_slot,
                        route.secondary_fallback_slot};
                };
            actor_context.movement_collision_recovery =
                [this](const std::uint16_t x, const std::uint16_t y,
                       const std::int16_t collision_code,
                       const std::uint8_t direction_slot) {
                    const auto movement_recovered =
                        recover_blocked_field_movement(
                            MovementProbeResult{collision_code, x, y,
                                                direction_slot});
                    return scripting::ScriptMovementCollisionRecoveryResult{
                        movement_recovered,
                        field_movement_behavior_record_.route_flags};
                };
        }
        // 0xB574 reads Quickness, FFDF84, and the projected +0x0C word.
        // The decoded ±40 list stays the host default until Lua selects.
        actor_context.combatant =
            script_combatant_facts(active_runner_record());
        actor_context.world_activity_active =
            field_world_activity_timeout_state_.timeout_marker != 0;
        actor_context.world_scene_state =
            world_frame_logic_state_.scene_state;
        actor_context.source_projection_active = caster_projection_active(
            field_entity_.world_position.x, field_entity_.world_position.y,
            static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_x),
            static_cast<std::uint16_t>(
                state_.field_scene_buffers.camera_y));
    }
    actor_context.combatant =
        script_combatant_facts(active_runner_record());
    actor_context.combat_meter_service_blocked =
        field_movement_transition_state_.world_mode == 3U;
    actor_context.ai_random_bound = [this](const std::uint16_t bound)
        -> std::optional<std::uint16_t> {
        if (bound == 0U) {
            return std::nullopt;
        }
        const auto choice =
            sample_source_random_bound(field_random_seed_, bound);
        if (choice < 0 || choice >= bound) {
            return std::nullopt;
        }
        return static_cast<std::uint16_t>(choice);
    };
    const auto actor_decision = game_scripts_.actor_heartbeat(actor_context);
    if (actor_decision.has_value()) {
        field_player_actor_script_initialized_ = true;
        if (actor_decision->target_selection_processed) {
            if (!apply_scripted_field_entity_target_selection(
                    actor_decision->selected_world_actor_id)) {
                return std::nullopt;
            }
        }
        if (phase == scripting::ScriptActorHeartbeatPhase::render &&
            actor_context.animation_accumulator_override.has_value()) {
            field_player_animation_reset_pending_ = false;
        }
    }
    return actor_decision;
}

void FieldSceneRuntime::resolve_field_party_wipe() {
    // ROM 0xC64C/0xC716: the run-failure branches (FFFBED) and the
    // DocWagon scene move (0x2A32 -> 0x136DE) are not ported [H].
    auto& records = field_party_roster_.records();
    const auto status = [this, &records](const std::size_t index) {
        const auto flags =
            static_cast<std::int64_t>(records[index].flags >> 8U);
        return field_party_roster_.slot_active(index)
            ? flags : (flags & ~std::int64_t{0x01});
    };
    const auto story = game_scripts_.story_state();
    const auto bit0 = story.find("story_flag_bit_0");
    const bool docwagon = bit0 != story.end() && bit0->second;
    const auto wipe = game_scripts_.call_integers(
        "party_down", "resolve_wipe_flat",
        {static_cast<std::int64_t>(field_funds_.value_or(0U)),
         docwagon ? 1 : 0,
         status(0U), records[0].delay_meter, records[0].behavior_meter,
         status(1U), records[1].delay_meter, records[1].behavior_meter,
         status(2U), records[2].delay_meter, records[2].behavior_meter});
    if (!wipe.has_value() || wipe->size() != 7U) {
        failure_ = game_scripts_.failure();
        source_boundary("field.party_wipe.lua_failed");
        return;
    }
    for (std::size_t index = 2U; index >= 1U; --index) {
        if ((*wipe)[2U + index] != 0 &&
            field_party_roster_.deactivate_party_member(index)) {
            source_boundary("field.party_wipe.hired_runner_dismissed");
        }
    }
    records[0].flags = static_cast<std::uint16_t>(
        (records[0].flags & 0x00ffU) |
        (static_cast<std::uint16_t>((*wipe)[0] & 0xff) << 8U));
    records[0].delay_meter = static_cast<std::uint8_t>((*wipe)[1]);
    records[0].behavior_meter = static_cast<std::uint8_t>((*wipe)[2]);
    field_party_roster_.set_active_slot(0U);
    if (field_funds_.has_value()) {
        field_funds_ = static_cast<std::uint32_t>((*wipe)[5]);
    }
    refresh_field_hud_runner_slot();
    source_boundary((*wipe)[6] != 0 ? "field.party_wipe.docwagon"
                                    : "field.party_wipe.little_chiba");
    game_scripts_.set_location_context("field_map", "party_wipe",
                                       "field.world_event", "field_narrator");
    if (!field_dialogue_.open_gameplay_task("party_down", "present_wake")) {
        source_boundary("field.party_wipe.wake_text_open_failed");
    }
}

void FieldSceneRuntime::advance_field_movement() {
    if (!field_entity_.active) {
        return;
    }
    if (field_party_wipe_countdown_ != 0U) {
        // ROM 0xAF4 blocks the player service while FFE166 is nonzero;
        // 0x1DDC6 decrements it and enters 0xC64C at zero.
        if (--field_party_wipe_countdown_ == 0U) {
            resolve_field_party_wipe();
        }
        return;
    }
    if ((active_runner_record().flags & kRunnerFlagsDown) != 0U) {
        // ROM 0xB330 installs 0xB38A at +0x3A: no input or movement.
        return;
    }
    if (field_terrain_probe_trace_enabled_) {
        field_terrain_probe_trace_.clear();
    }

    const bool movement_transition_paused =
        field_opening_auto_walk_.active &&
        field_movement_transition_state_.interaction_transition_pending;
    const auto actor_decision = heartbeat_field_player(
        scripting::ScriptActorHeartbeatPhase::behavior);
    if (!actor_decision.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.player.lua_heartbeat_failed");
        return;
    }

    service_player_spell_cast(*actor_decision);
    service_player_melee(*actor_decision);
    service_player_weapon_action(
        actor_decision->combat_action_requested);

    if (!ready_) {
        // The source keeps the walking pose only while a direction is held.
        // When the controller goes idle, the heading animation returns to
        // the directional idle list instead of looping the walk cycle while
        // the runner stands still.
        if (!field_entity_attack_pose_active_) {
            field_entity_walking_ = false;
        }
        return;
    }

    if (movement_transition_paused) {
        // Keep the startup marker alive through the fade, as the ROM does,
        // while the projected doorway event owns the field transition.
        if (!field_entity_attack_pose_active_) {
            field_entity_walking_ = false;
        }
        return;
    }

    if (!field_opening_auto_walk_.active && field_controller_nibble_ == 0U) {
        if (!field_entity_attack_pose_active_) {
            field_entity_walking_ = false;
        }
        return;
    }

    if (!actor_decision->movement_heading.has_value()) {
        return;
    }

    if (!actor_decision->movement_current_heading.has_value()) {
        failure_ = "Lua player heartbeat omitted its current heading";
        source_boundary("field.player.lua_heading_missing");
        return;
    }
    if (!actor_decision->movement_step.has_value()) {
        failure_ = "Lua player heartbeat omitted its collision-resolved step";
        source_boundary("field.player.lua_movement_step_missing");
        return;
    }

    field_target_heading_ = *actor_decision->movement_heading;
    const auto previous_x = field_movement_state_.position_x;
    const auto previous_y = field_movement_state_.position_y;
    const auto& movement_step = *actor_decision->movement_step;
    const auto& script_state = movement_step.state;
    field_movement_state_.position_x = script_state.position_x;
    field_movement_state_.position_y = script_state.position_y;
    field_movement_state_.heading = script_state.heading;
    field_movement_state_.animation_heading = script_state.animation_heading;
    field_movement_state_.collision_context = script_state.collision_context;
    field_movement_state_.movement_substate = script_state.movement_substate;
    field_movement_state_.route_status = script_state.route_status;
    field_movement_state_.movement_flags = script_state.movement_flags;
    field_movement_state_.collision_flags = script_state.collision_flags;
    field_movement_state_.event_flags = script_state.event_flags;
    field_movement_state_.collision_extent = script_state.collision_extent;
    field_movement_transition_state_.terrain_collision_code =
        script_state.terrain_collision_code;
    field_movement_behavior_record_.route_flags = script_state.route_flags;

    if (movement_step.direction_animation !=
        field_entity_animation_selector_) {
        field_entity_animation_.redraw_requested = true;
    }
    field_entity_animation_selector_ = movement_step.direction_animation;

    if (movement_step.outcome ==
        scripting::ScriptActorMovementStep::Outcome::reset) {
        field_player_animation_reset_pending_ = true;
        field_entity_animation_.redraw_requested = true;
        field_entity_attack_pose_active_ = false;
        field_entity_walking_ = false;
        source_boundary("field.movement.reset_blocked_step");
    }

    field_entity_.world_position.x = field_movement_state_.position_x;
    field_entity_.world_position.y = field_movement_state_.position_y;
    field_entity_.direction = actor_decision->direction;
    if (previous_x != field_movement_state_.position_x ||
        previous_y != field_movement_state_.position_y) {
        source_boundary("field.movement.accepted_step");
        field_entity_attack_pose_active_ = false;
        field_entity_walking_ = true;
    } else {
        field_entity_walking_ = false;
    }
}

void FieldSceneRuntime::advance_player_weapon_action_gate() {
    if (!ready_ || !field_entity_.active) {
        return;
    }

    auto& runner = active_runner_record();
    advance_active_entity_animation_action_gate(runner);
    if (runner.animation_action_gate >= 0 ||
        !field_entity_attack_pose_active_) {
        return;
    }

    field_entity_attack_pose_active_ = false;
    if (!field_entity_walking_) {
        field_entity_animation_selector_ = static_cast<std::uint16_t>(
            field_entity_attack_direction_ * 4U);
        field_entity_animation_.frame = 0U;
        field_entity_animation_.accumulator = 0;
        field_entity_animation_.redraw_requested = true;
    }
    source_boundary("field.combat.player_attack_pose_expired");
}

void FieldSceneRuntime::service_player_spell_cast(
    const scripting::ScriptActorBehaviorDecision& decision) {
    const auto dispatch = decision.source_action_dispatch.value_or("");
    // 0xAD94: no cast (blocked line, FFE186 == 3, no +0xA2) still runs 0xAF7C.
    const bool blocked = dispatch == "player_spell_blocked" ||
        dispatch == "player_spell_suppressed" ||
        dispatch == "player_spell_no_target";
    if (!decision.spell_action_resolution.has_value() && !blocked) {
        return;
    }
    auto& runner = active_runner_record();
    if (decision.spell_action_resolution.has_value()) {
        const auto& resolution = *decision.spell_action_resolution;
        if (resolution.effect_started &&
            resolution.spell_effect_slot.has_value()) {
            const auto slot = *resolution.spell_effect_slot;
            if (slot >= field_world_effect_slots_occupied_.size() ||
                field_world_effect_slots_occupied_[slot] ||
                resolution.target_id.empty() ||
                !field_spell_effect_slot_target_ids_[slot].empty()) {
                failure_ = "Lua selected an occupied spell effect record";
                source_boundary(
                    "field.combat.player_spell.effect_slot_invalid");
                return;
            }
            field_world_effect_slots_occupied_[slot] = true;
            field_spell_effect_slot_target_ids_[slot] = resolution.target_id;
            source_boundary("field.combat.player_spell.effect_slot_reserved");
        }
        // 0x1033E writes the caster's +0x9C meter and 0x10660 fetish cells.
        runner.behavior_meter = resolution.mental_meter;
        runner.inventory_item_codes = resolution.item_codes;
        runner.inventory_phase_bytes = resolution.item_phases;
        if (decision.audio_event.has_value() && source_audio_event_callback_) {
            source_audio_event_callback_(*decision.audio_event);
        }
    }
    // 0xAFAE (cast) and 0xAF7C (blocked) both call 0xABFC with mode 0.
    EntityDelayState delay{runner.animation_action_gate};
    update_entity_animation_delay(
        delay,
        EntityDelayAttributes{runner.attributes[1U], runner.attributes[4U],
                              runner.cyberware_flags},
        decode_entity_delay_scale(0U));
    runner.animation_action_gate = delay.animation_delay;
    // 0xAF7C/0xAFAE: +0x16 = C082[dir(+0x2E)], then 0xAF6C clears the clock.
    const auto cast_direction = field_movement_direction_from_heading(
        field_movement_state_.heading);
    field_entity_animation_selector_ =
        static_cast<std::uint16_t>(0x0080U + cast_direction * 4U);
    field_entity_attack_direction_ = cast_direction;
    field_entity_.direction = cast_direction;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = true;
    field_entity_attack_pose_kind_ = kFieldPlayerPoseCast;
    field_entity_animation_.accumulator = 0;
    field_entity_animation_.frame = 0U;
    field_entity_animation_.redraw_requested = true;
    refresh_field_hud_runner_slot();
    if (blocked) {
        source_boundary(dispatch == "player_spell_blocked"
            ? "field.combat.player_spell.blocked"
            : dispatch == "player_spell_suppressed"
            ? "field.combat.player_spell.scene_suppressed"
            : "field.combat.player_spell.no_target");
    } else {
        source_boundary(decision.spell_action_resolution->effect_requested
            ? "field.combat.player_spell.effect_requested"
            : "field.combat.player_spell.action_resolved");
    }
}

void FieldSceneRuntime::service_player_melee(
    const scripting::ScriptActorBehaviorDecision& decision) {
    if (!decision.source_action_dispatch.has_value() ||
        (*decision.source_action_dispatch != "player_melee" &&
         *decision.source_action_dispatch != "player_melee_hit")) {
        return;
    }
    auto& runner = active_runner_record();
    // ROM 0xB1DC: 0xABFC mode 1 first, then the 0x4C/0x55 swing request.
    EntityDelayState delay{runner.animation_action_gate};
    update_entity_animation_delay(
        delay,
        EntityDelayAttributes{runner.attributes[1U], runner.attributes[4U],
                              runner.cyberware_flags},
        decode_entity_delay_scale(1U));
    runner.animation_action_gate = delay.animation_delay;
    // ROM 0xB212..0xB23A: a linked strike turns +0x34/+0x2E to the Lua
    // bearing; +0x16 = C0A2[dir(+0x2E)], then 0xAF6C clears the frame clock.
    if (decision.strike_heading.has_value()) {
        field_movement_state_.heading = *decision.strike_heading;
        field_target_heading_ = *decision.strike_heading;
    }
    const auto strike_direction = field_movement_direction_from_heading(
        field_movement_state_.heading);
    field_entity_animation_selector_ =
        static_cast<std::uint16_t>(0x00C0U + strike_direction * 4U);
    field_entity_attack_direction_ = strike_direction;
    field_entity_.direction = strike_direction;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = true;
    field_entity_attack_pose_kind_ = kFieldPlayerPoseStrike;
    field_entity_animation_.accumulator = 0;
    field_entity_animation_.frame = 0U;
    field_entity_animation_.redraw_requested = true;
    if (source_audio_event_callback_) {
        if (decision.audio_event.has_value()) {
            source_audio_event_callback_(*decision.audio_event);
        }
        // 0xB25E: in-range strikes add the 0x4D/0x4E target cue.
        if (decision.target_heading_audio_event.has_value()) {
            source_audio_event_callback_(*decision.target_heading_audio_event);
        }
    }
    if (decision.combat_meter_update.has_value() &&
        !apply_script_combat_meter_update(*decision.combat_meter_update,
                                          "field.combat.player_melee")) {
        return;
    }
    refresh_field_hud_runner_slot();
    source_boundary(*decision.source_action_dispatch == "player_melee_hit"
        ? "field.combat.player_melee.hit"
        : "field.combat.player_melee.swing");
}

bool FieldSceneRuntime::probe_action_line_clear(
    const SpawnEntityParentState& parent,
    const std::optional<std::size_t> protected_source_slot,
    const std::uint16_t target_x,
    const std::uint16_t target_y) {
    // ROM 0x14D8E line probe, reached from the NPC 0x13FFE spell/item branch
    // and the field player's 0xAE6E cast. Lua owns the target policy.
    EntityAnimationSpawnProbeState probe_state{};
    std::optional<SpawnEntityLinkedState> linked_target{
        SpawnEntityLinkedState{target_x, target_y, 0x01U, false}};
    EntityAnimationSpawnProbeRequest request{};
    request.parent = parent;
    request.linked_target = linked_target;
    request.protected_source_candidate = protected_source_slot;

    EntityCollisionRecord probe_collision_record{};
    std::array<std::uint8_t, 4> probe_sample_state{};
    std::vector<CollisionCandidate> collision_candidates(10U);
    for (std::size_t slot = 0U;
         slot < FieldPartyRosterRuntime::kSlotCount; ++slot) {
        if (!field_party_roster_.slot_active(slot)) {
            continue;
        }
        auto position = state_.field_scene_buffers
                            .source_metadata
                            .initial_field_records[slot];
        if (slot == field_party_roster_.active_slot() &&
            field_entity_.active) {
            position = field_entity_.world_position;
        }
        const auto& runner = field_party_roster_.records()[slot];
        auto& candidate = collision_candidates[slot];
        candidate.world_x = position.x;
        candidate.world_y = position.y;
        candidate.flags = static_cast<std::uint8_t>(
            runner.flags >> 8U);
        candidate.interaction_flags =
            static_cast<std::uint8_t>(runner.flags & 0x00FFU);
        candidate.collision_enabled = runner.delay_meter != 0U;
    }
    for (const auto& target : field_default_actors_) {
        if (!target.active ||
            target.scene_reference != state_.field_scene_buffers.scene_reference ||
            !target.source_record_slot.has_value()) {
            continue;
        }
        const auto slot = static_cast<std::size_t>(
            *target.source_record_slot);
        if (slot >= collision_candidates.size()) {
            continue;
        }
        auto& candidate = collision_candidates[slot];
        candidate.world_x = target.world_position.x;
        candidate.world_y = target.world_position.y;
        candidate.flags = target.source_record_flags;
        candidate.interaction_flags =
            target.source_interaction_flags;
        candidate.behavior_flags = target.source_behavior_flags;
        candidate.collision_enabled =
            target.physical_meter != 0U;
    }

    const auto probe_result = probe_entity_spawn_collision_window(
        probe_state,
        request,
        EntityAnimationSpawnProbeCallbacks{
            [this, &probe_state,
             &probe_collision_record, &probe_sample_state](
                const std::int32_t fixed_x,
                const std::int32_t fixed_y) {
                // 0x14F26 supplies 16.16 values. The field
                // collision service takes the source 16.4 word;
                // retaining the high word preserves world units.
                const auto collision_coordinate = [](
                    const std::int32_t value) {
                    return static_cast<std::uint16_t>(
                        static_cast<std::uint32_t>(value) >> 16U);
                };
                bool terrain_boundary_active = false;
                bool outside_bounds = false;
                const auto collision_code =
                    evaluate_field_collision(
                        collision_coordinate(fixed_x),
                        collision_coordinate(fixed_y),
                        probe_collision_record,
                        probe_sample_state,
                        probe_state.scratch_record
                            .behavior_defaults.flags,
                        false,
                        nullptr,
                        &terrain_boundary_active,
                        &outside_bounds);
                return EntityTerrainProbeSample{
                    static_cast<std::uint16_t>(collision_code),
                    terrain_boundary_active, outside_bounds};
            },
            [&collision_candidates](
                const EntityCollisionSearchContext& collision) {
                return find_colliding_entity(
                    collision, collision_candidates);
            }});
    return !probe_result.boundary_unresolved &&
        !probe_result.source_carry;
}

void FieldSceneRuntime::service_player_weapon_action(
    const bool requested_by_lua) {
    if (!ready_ || !field_entity_.active) {
        return;
    }

    auto& runner = active_runner_record();

    // ROM 0xA8EA only calls 0xACAE for an active record with source +0x10 bit
    // 1 set. The ordinary field profile starts at 0x05 (active plus mode-5),
    // so merely standing with an empty weapon must not reload it.
    if (!requested_by_lua || field_combat_input_suppressed_until_release_ ||
        runner.animation_action_gate >= 0 ||
        (runner.flags & 0x0200U) == 0U ||
        (field_controller_sample_state_.held & 0x0040U) == 0U) {
        return;
    }

    // A negative equipped slot selects a D4EA0 spell row. Spell codes overlap
    // D4540 weapon codes, so classify the signed slot and code together and
    // return before either empty-magazine reload or ammo service.
    const auto action_kind = classify_field_combat_action(
        runner.equipped_inventory_slot, runner.equipped_item_code);
    if (action_kind == FieldCombatActionKind::magic_spell) {
        return;
    }
    if (action_kind != FieldCombatActionKind::weapon_ammo ||
        (runner.flags & 0x0100U) == 0U) {
        return;
    }
    if (runner.equipped_inventory_slot < 0 ||
        static_cast<std::size_t>(runner.equipped_inventory_slot) >=
            runner.inventory_phase_bytes.size()) {
        return;
    }

    const auto* weapon_template =
        weapon_template_for_item_code(runner.equipped_item_code);
    const auto attack_delay_mode = weapon_template == nullptr
        ? std::optional<std::uint16_t>{}
        : source_delay_mode(weapon_template->attack_delay_scale);
    if (!attack_delay_mode.has_value()) {
        source_boundary("field.combat.weapon_template_unresolved");
        return;
    }

    const PlayerActionDispatchCallbacks reload_callbacks{
        [this](const std::uint16_t event) {
            if (source_audio_event_callback_) {
                source_audio_event_callback_(event);
            }
        }};
    const WeaponMagazineCapacityLookup weapon_capacity_lookup =
        [this](const std::uint8_t item_code) {
            return weapon_magazine_capacity_for_item(item_code);
        };
    const WeaponReloadDelayModeLookup reload_delay_mode_lookup =
        [this](const std::uint8_t item_code) {
            const auto* weapon = weapon_template_for_item_code(item_code);
            return weapon == nullptr
                       ? std::optional<std::uint16_t>{}
                       : source_delay_mode(weapon->reload_delay_scale);
        };
    if (runner.ammo_rounds == 0U) {
        const auto reload = dispatch_player_action_if_ready(
            runner, weapon_capacity_lookup, reload_delay_mode_lookup,
            reload_callbacks);
        if (reload.status == PlayerActionDispatchStatus::serviced) {
            refresh_field_hud_runner_slot();
            source_boundary("field.combat.player_auto_reload");
        } else if (reload.status ==
                   PlayerActionDispatchStatus::weapon_template_unresolved) {
            source_boundary("field.combat.reload_template_unresolved");
        }
        return;
    }

    // The source actor's +0x16 animation timer is not the host's selected
    // movement sprite list. Use the source +0xDC action delay above as the
    // firing cadence so changing between idle and walking lists cannot reset
    // the weapon's rate.

    const auto source_status_flags = runner.inventory_phase_bytes[
        static_cast<std::size_t>(runner.equipped_inventory_slot)];
    EntitySpawnCycleState spawn_state{};
    spawn_state.parent.position_x = field_movement_state_.position_x;
    spawn_state.parent.position_y = field_movement_state_.position_y;
    spawn_state.parent.heading = field_movement_state_.heading;
    spawn_state.parent.flags = static_cast<std::uint8_t>(runner.flags >> 8U);
    spawn_state.parent.field_0x56 = static_cast<std::uint8_t>(
        runner.equipped_inventory_slot);
    spawn_state.parent.field_0x57 = runner.equipped_item_code;
    spawn_state.parent.field_0x9d = runner.magic_force;
    spawn_state.equipment = EntityEquipmentState{
        runner.ammo_rounds, static_cast<std::int16_t>(runner.record_type)};
    spawn_state.animation_counter = field_entity_animation_.frame;
    spawn_state.global_mode = static_cast<std::uint8_t>(
        field_runtime_state_.field_scene_gate);
    spawn_state.entity_flags = spawn_state.parent.flags;
    spawn_state.source_status_flags = source_status_flags;
    // ROM 0x151BE loads the shooter's +0xA2. 0x14FF8 aims the projectile at
    // that record when its active flag is set; otherwise the shot keeps the
    // shooter's facing.
    if (field_entity_target_owner_.selected_target.has_value()) {
        const auto slot = *field_entity_target_owner_.selected_target;
        if (slot < field_entity_target_records_.size() &&
            field_entity_target_records_[slot].active) {
            std::optional<SceneRunnerPosition> aim;
            const auto current_scene =
                state_.field_scene_buffers.scene_reference;
            const auto selected_actor = std::find_if(
                field_default_actors_.begin(), field_default_actors_.end(),
                [current_scene, slot](const auto& candidate) {
                    return candidate.active &&
                        candidate.scene_reference == current_scene &&
                        candidate.source_record_slot.has_value() &&
                        static_cast<std::size_t>(
                            *candidate.source_record_slot) == slot;
                });
            if (selected_actor != field_default_actors_.end()) {
                aim = selected_actor->world_position;
            } else if (slot < FieldPartyRosterRuntime::kSlotCount &&
                       field_party_roster_.slot_active(slot) &&
                       slot < state_.field_scene_buffers.source_metadata
                                  .initial_field_records.size()) {
                aim = state_.field_scene_buffers.source_metadata
                          .initial_field_records[slot];
                if (slot == field_party_roster_.active_slot() &&
                    field_entity_.active) {
                    aim = field_entity_.world_position;
                }
            }
            if (aim.has_value()) {
                spawn_state.linked_target = SpawnEntityLinkedState{
                    aim->x, aim->y, 0x01U, false};
            }
        }
    }

    const auto ammo_before = runner.ammo_rounds;
    std::vector<std::uint16_t> allocated_slots;
    const auto spawn_cycle = process_entity_spawn_cycle(
        spawn_state,
        [] { return true; },
        [this, &allocated_slots]() -> std::optional<std::uint16_t> {
            for (std::size_t index = 0;
                 index < field_weapon_slots_occupied_.size(); ++index) {
                if (!field_weapon_slots_occupied_[index]) {
                    field_weapon_slots_occupied_[index] = true;
                    const auto slot = static_cast<std::uint16_t>(9U + index);
                    allocated_slots.push_back(slot);
                    return slot;
                }
            }
            return std::nullopt;
        },
        [this] {
            // Lua computes the source 0x55640 attack seed on the primary
            // projectile's first actor heartbeat, after this spawn cycle has
            // published its typed child record.
            source_boundary("field.combat.weapon_attack_seed_scheduled");
        },
        [this] {
            (void)schedule_world_activity_timeout(
                field_world_activity_timeout_state_);
        },
        [this](const std::uint16_t event) {
            if (source_audio_event_callback_) {
                source_audio_event_callback_(event);
            }
        },
        [this](const std::uint16_t) {
            source_boundary("field.combat.mode_effect_transition_pending");
        },
        [this, &runner, &weapon_capacity_lookup](EntityEquipmentState& equipment,
                        const std::uint8_t source_action_code) {
            const EntityWeaponRoundsPerActionLookup rounds_per_action =
                [this](const std::uint8_t item_code)
                    -> std::optional<std::uint8_t> {
                    const auto* weapon =
                        weapon_template_for_item_code(item_code);
                    return weapon == nullptr
                               ? std::nullopt
                               : std::optional<std::uint8_t>{
                                     weapon->rounds_per_action};
                };
            RunnerAssetOperationResult weapon_operation =
                RunnerAssetOperationResult::no_change;
            const auto ammo_action = service_entity_ammo(
                equipment, source_action_code, rounds_per_action,
                [this, &runner, &equipment,
                 &weapon_operation,
                 &weapon_capacity_lookup](const std::int8_t meter_delta) {
                    weapon_operation = service_entity_ammo(
                        runner, meter_delta, weapon_capacity_lookup);
                    equipment.current_ammo = runner.ammo_rounds;
                });

            if (equipment.record_type >= 3) {
                runner.ammo_rounds = equipment.current_ammo;
                return ammo_action;
            }
            if (weapon_operation ==
                RunnerAssetOperationResult::ammo_changed) {
                return ammo_action;
            }
            if (weapon_operation ==
                    RunnerAssetOperationResult::missing_source_data ||
                weapon_operation ==
                    RunnerAssetOperationResult::invalid_equipped_slot) {
                return EntityAmmoServiceAction::weapon_service_unresolved;
            }
            return EntityAmmoServiceAction::no_action;
        });
    last_field_weapon_spawn_result_ = spawn_cycle;
    for (const auto slot : allocated_slots) {
        const bool has_primary_record = spawn_cycle.primary.has_value() &&
            spawn_cycle.primary_slot.has_value() &&
            *spawn_cycle.primary_slot == slot;
        const bool has_secondary_record = spawn_cycle.secondary.has_value() &&
            spawn_cycle.secondary_slot.has_value() &&
            *spawn_cycle.secondary_slot == slot;
        if (!has_primary_record && !has_secondary_record && slot >= 9U &&
            slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(slot - 9U)] = false;
        }
    }
    if (runner.ammo_rounds != ammo_before) {
        refresh_field_hud_runner_slot();
    }

    if (spawn_cycle.primary.has_value() ||
        spawn_cycle.secondary.has_value()) {
        FieldWeaponProjectilePresentation spawn{};
        spawn.shooter_facts = script_combatant_facts(runner);
        spawn.shooter_party_slot = field_party_roster_.active_slot();
        if (spawn_cycle.primary.has_value()) {
            spawn.primary_record = *spawn_cycle.primary;
            spawn.primary_spawn_position = SceneRunnerPosition{
                spawn.primary_record.position_x,
                spawn.primary_record.position_y};
            spawn.primary_active =
                (spawn.primary_record.flags & 0x01U) != 0U;
            spawn.primary_slot = spawn_cycle.primary_slot;
            spawn.primary_fixed_position = EntityFixedPointPosition{
                static_cast<std::int32_t>(
                    static_cast<std::int16_t>(
                        spawn.primary_record.position_x)) * 0x10000,
                static_cast<std::int32_t>(
                    static_cast<std::int16_t>(
                        spawn.primary_record.position_y)) * 0x10000,
                spawn.primary_record.heading_offset_x,
                spawn.primary_record.heading_offset_y};
            spawn.primary_animation.period = static_cast<std::int16_t>(
                spawn.primary_record.behavior_defaults.behavior_timer);
        }
        if (spawn_cycle.secondary.has_value()) {
            spawn.source_record = *spawn_cycle.secondary;
            spawn.secondary_active = true;
            spawn.secondary_slot = spawn_cycle.secondary_slot;
            spawn.animation.period = static_cast<std::int16_t>(
                spawn.source_record.behavior_defaults.behavior_timer);
        }

        const auto secondary_selector = static_cast<std::uint16_t>(
            spawn.source_record.animation_resource & 0x7FFFU);
        const auto secondary_animation_index = static_cast<std::size_t>(
            secondary_selector / 4U);
        if (spawn.secondary_active &&
            (secondary_animation_index >=
                 field_weapon_projectile_images_.size() ||
             field_weapon_projectile_images_[secondary_animation_index]
                 .empty())) {
            source_boundary("field.combat.weapon_projectile_asset_missing");
            spawn.secondary_active = false;
            if (spawn.secondary_slot.has_value() &&
                *spawn.secondary_slot >= 9U &&
                *spawn.secondary_slot <= 14U) {
                field_weapon_slots_occupied_[
                    static_cast<std::size_t>(*spawn.secondary_slot - 9U)] =
                    false;
            }
        }
        field_weapon_projectiles_.push_back(std::move(spawn));
    }

    const bool complete_spawn =
        spawn_cycle.action == EntitySpawnCycleAction::spawned &&
        spawn_cycle.primary.has_value() &&
        spawn_cycle.secondary.has_value();
    if (!complete_spawn) {
        if (spawn_cycle.action == EntitySpawnCycleAction::no_primary_slot ||
            spawn_cycle.action == EntitySpawnCycleAction::no_secondary_slot) {
            const auto fallback = dispatch_player_action_if_ready(
                runner, weapon_capacity_lookup, reload_delay_mode_lookup,
                reload_callbacks);
            if (fallback.status == PlayerActionDispatchStatus::serviced) {
                refresh_field_hud_runner_slot();
            } else if (fallback.status ==
                       PlayerActionDispatchStatus::weapon_template_unresolved) {
                source_boundary("field.combat.reload_template_unresolved");
            }
        }
        return;
    }

    HeadingAnimationState attack_pose{};
    const auto spawned_heading = spawn_cycle.primary->heading;
    const auto direction = field_movement_direction_from_heading(
        spawned_heading);
    set_entity_heading_with_field_animation(
        attack_pose, spawned_heading,
        *attack_delay_mode, kFieldAttackAnimationSelectors,
        [this, &runner](const std::uint16_t delay_mode) {
            EntityDelayState delay{runner.animation_action_gate};
            update_entity_animation_delay(
                delay,
                EntityDelayAttributes{runner.attributes[1U],
                                      runner.attributes[4U],
                                      runner.cyberware_flags},
                decode_entity_delay_scale(delay_mode));
            runner.animation_action_gate = delay.animation_delay;
        },
        [this] {
            field_entity_animation_.accumulator = 0;
            field_entity_animation_.frame = 0U;
            field_entity_animation_.redraw_requested = true;
        });
    field_entity_animation_selector_ = attack_pose.direction_animation;
    field_entity_attack_direction_ = direction;
    field_entity_.direction = direction;
    field_entity_walking_ = false;
    field_entity_attack_pose_active_ = true;
    field_entity_attack_pose_kind_ = kFieldPlayerPoseFirearm;
    source_boundary("field.combat.player_weapon_spawned");
}

void FieldSceneRuntime::advance_field_weapon_projectiles() {
    const auto release_slot = [this](
                                   std::optional<std::uint16_t>& slot) {
        if (slot.has_value() && *slot >= 9U && *slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(*slot - 9U)] = false;
        }
        slot.reset();
    };
    std::vector<CollisionCandidate> live_collision_targets(10U);
    std::vector<std::string> live_collision_target_ids(10U);
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    const auto& runner_positions =
        state_.field_scene_buffers.source_metadata.initial_field_records;
    for (std::size_t slot = 0U;
         slot < FieldPartyRosterRuntime::kSlotCount; ++slot) {
        if (!field_party_roster_.slot_active(slot)) {
            continue;
        }
        auto position = runner_positions[slot];
        if (slot == field_party_roster_.active_slot() && field_entity_.active) {
            position = field_entity_.world_position;
        }
        const auto& runner = field_party_roster_.records()[slot];
        auto& candidate = live_collision_targets[slot];
        candidate.world_x = position.x;
        candidate.world_y = position.y;
        candidate.flags = static_cast<std::uint8_t>(runner.flags >> 8U);
        candidate.interaction_flags =
            static_cast<std::uint8_t>(runner.flags & 0x00FFU);
        candidate.collision_enabled = runner.delay_meter != 0U;
        live_collision_target_ids[slot] =
            "party.runner." + std::to_string(slot);
    }
    for (const auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene ||
            !actor.source_record_slot.has_value()) {
            continue;
        }
        const auto slot = static_cast<std::size_t>(
            *actor.source_record_slot);
        if (slot >= live_collision_targets.size()) {
            continue;
        }
        auto& candidate = live_collision_targets[slot];
        candidate.world_x = actor.world_position.x;
        candidate.world_y = actor.world_position.y;
        candidate.flags = actor.source_record_flags;
        candidate.interaction_flags = actor.source_interaction_flags;
        candidate.behavior_flags = actor.source_behavior_flags;
        candidate.collision_enabled = actor.physical_meter != 0U;
        live_collision_target_ids[slot] = actor.actor_instance_id;
    }
    std::vector<scripting::ScriptProjectileCombatTarget>
        live_projectile_combat_targets;
    std::vector<scripting::ScriptActorTargetCandidate>
        live_party_combat_candidates;
    for (std::size_t slot = 0U;
         slot < FieldPartyRosterRuntime::kSlotCount; ++slot) {
        if (!field_party_roster_.slot_active(slot)) {
            continue;
        }
        const auto id = "party.runner." + std::to_string(slot);
        const auto facts = script_combatant_facts(
            field_party_roster_.records()[slot]);
        live_projectile_combat_targets.push_back(
            scripting::ScriptProjectileCombatTarget{
                id, static_cast<std::uint8_t>(slot), facts});
        scripting::ScriptActorTargetCandidate party_candidate{};
        party_candidate.id = id;
        party_candidate.source_record_slot =
            static_cast<std::uint8_t>(slot);
        party_candidate.combatant = facts;
        live_party_combat_candidates.push_back(
            std::move(party_candidate));
    }
    for (const auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene ||
            !actor.source_record_slot.has_value()) {
            continue;
        }
        const auto slot = static_cast<std::size_t>(
            *actor.source_record_slot);
        if (slot >= live_collision_target_ids.size()) {
            continue;
        }
        live_projectile_combat_targets.push_back(
            scripting::ScriptProjectileCombatTarget{
                actor.actor_instance_id,
                static_cast<std::uint8_t>(slot), actor.combatant});
    }
    const auto word_from_fixed = [](const std::int32_t value) {
        return static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(value) >> 16U);
    };
    const auto signed_fixed_from_word = [](const std::uint16_t word,
                                           const std::int32_t current) {
        const auto combined =
            (static_cast<std::uint32_t>(word) << 16U) |
            (static_cast<std::uint32_t>(current) & 0xFFFFU);
        return static_cast<std::int32_t>(combined);
    };

    const auto advance_record = [this, &release_slot, &live_collision_targets,
                                 &live_collision_target_ids,
                                 &live_projectile_combat_targets,
                                 &live_party_combat_candidates,
                                 &word_from_fixed, &signed_fixed_from_word](
                                    FieldWeaponProjectilePresentation& spawn,
                                    const bool primary_record) {
        auto& record = primary_record ? spawn.primary_record
                                      : spawn.source_record;
        auto& active = primary_record ? spawn.primary_active
                                      : spawn.secondary_active;
        auto& script_initialized = primary_record
            ? spawn.primary_animation_script_initialized
            : spawn.secondary_animation_script_initialized;
        auto& slot = primary_record ? spawn.primary_slot
                                    : spawn.secondary_slot;
        auto& animation = primary_record ? spawn.primary_animation
                                         : spawn.animation;
        if (!active) {
            return;
        }
        const auto disable_record = [&] {
            record.flags = static_cast<std::uint8_t>(
                record.flags & static_cast<std::uint8_t>(~0x01U));
            active = false;
            release_slot(slot);
        };
        if (!slot.has_value()) {
            source_boundary("field.combat.weapon_projectile_slot_missing");
            disable_record();
            return;
        }
        const auto selector = static_cast<std::uint16_t>(
            record.animation_resource & 0x7FFFU);
        const auto animation_index =
            static_cast<std::size_t>(selector / 4U);
        if ((selector & 0x0003U) != 0U ||
            animation_index >= field_weapon_projectile_images_.size()) {
            source_boundary(
                "field.combat.weapon_projectile_selector_invalid");
            disable_record();
            return;
        }
        const auto frame_count =
            field_weapon_projectile_images_[animation_index].size();
        if (frame_count == 0U || frame_count > 16U) {
            source_boundary("field.combat.weapon_projectile_frames_invalid");
            disable_record();
            return;
        }

        const auto instance_id = "field_weapon_slot_" +
                                 std::to_string(*slot);
        scripting::ScriptActorBehaviorContext context{};
        context.profile_id = "weapon_projectile";
        context.entity_kind = record.behavior_defaults.animation_state;
        context.action_kind = scripting::ScriptActorActionKind::none;
        context.projectile_action_code = record.field_0x57;
        context.instance_id = instance_id;
        context.initial_heading = record.heading;
        context.spawn = !script_initialized;
        context.phase = scripting::ScriptActorHeartbeatPhase::behavior;
        context.projectile_animation =
            scripting::ScriptActorProjectileAnimation{
                record.animation_resource,
                static_cast<std::uint16_t>(frame_count),
                animation.frame,
                animation.accumulator,
                animation.increment,
                animation.period,
                true};
        context.projectile_movement =
            scripting::ScriptActorProjectileMovement{
                primary_record
                    ? scripting::ScriptActorProjectileRole::primary
                    : scripting::ScriptActorProjectileRole::secondary,
                active,
                record.flags,
                record.record_flags,
                primary_record ? spawn.primary_fixed_position.world_x
                               : signed_fixed_from_word(record.position_x, 0),
                primary_record ? spawn.primary_fixed_position.world_y
                               : signed_fixed_from_word(record.position_y, 0),
                primary_record ? record.heading_offset_x : std::int16_t{0},
                primary_record ? record.heading_offset_y : std::int16_t{0},
                record.field_0x89,
                record.field_0x41,
                record.heading,
                record.saved_position_x,
                record.saved_position_y,
                record.action_result,
                record.animation_resource};
        context.projectile_movement->action_result = record.action_result;
        context.projectile_attack_seed_pending =
            primary_record && !script_initialized;
        if (primary_record) {
            context.combatant = spawn.shooter_facts;
            if (spawn.shooter_party_slot.has_value() &&
                *spawn.shooter_party_slot <
                    FieldPartyRosterRuntime::kSlotCount &&
                field_party_roster_.slot_active(
                    *spawn.shooter_party_slot)) {
                context.combatant = script_combatant_facts(
                    field_party_roster_.records()[
                        *spawn.shooter_party_slot]);
            } else if (!spawn.shooter_actor_id.empty()) {
                const auto shooter = std::find_if(
                    field_default_actors_.begin(),
                    field_default_actors_.end(),
                    [&spawn](const auto& candidate) {
                        return candidate.active &&
                               candidate.actor_instance_id ==
                                   spawn.shooter_actor_id;
                    });
                if (shooter != field_default_actors_.end()) {
                    context.combatant = shooter->combatant;
                }
            }
        }
        context.projectile_combat_targets = live_projectile_combat_targets;
        context.ai_target_candidates = live_party_combat_candidates;
        context.action_profile_modifiers =
            script_action_profile_modifiers(state_.action_selection_profiles);
        context.action_attribute_selectors =
            script_action_attribute_selectors();
        context.combat_meter_service_blocked =
            field_movement_transition_state_.world_mode == 3U;
        context.ai_random_bound = [this](const std::uint16_t bound)
            -> std::optional<std::uint16_t> {
            if (bound == 0U) {
                return std::nullopt;
            }
            const auto choice =
                sample_source_random_bound(field_random_seed_, bound);
            if (choice < 0 || choice >= bound) {
                return std::nullopt;
            }
            return static_cast<std::uint16_t>(choice);
        };
        context.projectile_terrain_probe =
            [this, &spawn, &record, primary_record](
                const std::uint16_t x, const std::uint16_t y) {
                if (!primary_record) {
                    return scripting::ScriptProjectileTerrainSample{};
                }
                const auto terrain_code = evaluate_field_collision(
                    x, y, spawn.primary_collision_record,
                    field_terrain_sample_state_, record.flags, false);
                return scripting::ScriptProjectileTerrainSample{
                    static_cast<std::uint16_t>(terrain_code),
                    field_terrain_boundary_active_,
                    field_terrain_outside_bounds_marker_};
            };
        context.projectile_collision_search =
            [this, &live_collision_targets, &live_collision_target_ids](
                const std::uint16_t x, const std::uint16_t y,
                const std::uint8_t source_flags) {
                EntityCollisionSearchContext collision_context{};
                collision_context.probe_x = x;
                collision_context.probe_y = y;
                collision_context.source_flags = source_flags;
                const auto result = find_colliding_entity(
                    collision_context, live_collision_targets);
                std::optional<std::string> hit_target_id;
                if (result.source_position_updated &&
                    result.candidate_index.has_value() &&
                    *result.candidate_index < live_collision_target_ids.size() &&
                    !live_collision_target_ids[*result.candidate_index].empty()) {
                    hit_target_id =
                        live_collision_target_ids[*result.candidate_index];
                }
                return scripting::ScriptProjectileCollisionSearchResult{
                    result.source_carry,
                    result.source_position_updated,
                    result.source_x,
                    result.source_y,
                    std::move(hit_target_id)};
            };

        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value() ||
            !decision->projectile_movement.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary(
                "field.combat.weapon_projectile_lua_behavior_failed");
            disable_record();
            return;
        }
        script_initialized = true;
        const auto& movement = decision->projectile_movement->state;
        const auto projectile_outcome =
            decision->projectile_movement->outcome;
        record.flags = movement.flags;
        record.record_flags = movement.record_flags;
        record.animation_resource = movement.animation_selector;
        record.field_0x89 = movement.retry_counter;
        record.field_0x41 = movement.linked_action_timer;
        record.action_result = movement.action_result;
        const bool attack_seed_refreshed = primary_record &&
            (context.projectile_attack_seed_pending ||
             projectile_outcome == scripting::ScriptActorProjectileOutcome::
                                       burst_followup_restarted);
        if (attack_seed_refreshed) {
            spawn.shooter_facts.action_result = movement.action_result;
            if (context.projectile_attack_seed_pending) {
                spawn.source_record.action_result = movement.action_result;
            }
            if (spawn.shooter_party_slot.has_value() &&
                *spawn.shooter_party_slot <
                    FieldPartyRosterRuntime::kSlotCount &&
                field_party_roster_.slot_active(
                    *spawn.shooter_party_slot)) {
                field_party_roster_.records()[*spawn.shooter_party_slot]
                    .action_result = movement.action_result;
            } else if (!spawn.shooter_actor_id.empty()) {
                const auto shooter = std::find_if(
                    field_default_actors_.begin(),
                    field_default_actors_.end(),
                    [&spawn](const auto& candidate) {
                        return candidate.active &&
                               candidate.actor_instance_id ==
                                   spawn.shooter_actor_id;
                    });
                if (shooter != field_default_actors_.end()) {
                    shooter->combatant.action_result =
                        movement.action_result;
                }
            }
        }
        if (primary_record) {
            record.heading = movement.heading;
            record.heading_offset_x = movement.tile_offset_x;
            record.heading_offset_y = movement.tile_offset_y;
            spawn.primary_fixed_position.world_x = movement.world_x;
            spawn.primary_fixed_position.world_y = movement.world_y;
            record.position_x = word_from_fixed(movement.world_x);
            record.position_y = word_from_fixed(movement.world_y);
            if (projectile_outcome ==
                scripting::ScriptActorProjectileOutcome::
                    burst_followup_restarted) {
                animation.frame = 0U;
                animation.accumulator = 0;
            }
        }
        if (decision->audio_event.has_value() &&
            source_audio_event_callback_) {
            source_audio_event_callback_(*decision->audio_event);
        }
        if (projectile_outcome ==
            scripting::ScriptActorProjectileOutcome::hit_resolution_pending) {
            const auto& target_id =
                decision->projectile_movement->state.hit_target_id;
            if (target_id.empty()) {
                source_boundary("field.combat.weapon_hit_target_id_missing");
            } else if (decision->combat_meter_update.has_value()) {
                if (apply_script_combat_meter_update(
                        *decision->combat_meter_update)) {
                    source_boundary("field.combat.weapon_hit_resolved:" +
                                    target_id);
                }
            } else {
                source_boundary("field.combat.weapon_damage_resolution_pending:" +
                                target_id);
            }
        }
        if (projectile_outcome ==
            scripting::ScriptActorProjectileOutcome::linked_followup_pending) {
            source_boundary(
                "field.combat.weapon_linked_action_followup_pending");
        }
        if (projectile_outcome ==
            scripting::ScriptActorProjectileOutcome::
                burst_followup_restarted) {
            source_boundary(
                "field.combat.weapon_projectile_burst_followup_started:" +
                std::to_string(static_cast<unsigned>(
                    movement.linked_action_timer)) +
                ":heading=" + std::to_string(movement.heading) +
                ":position=" +
                std::to_string(word_from_fixed(movement.world_x)) + "," +
                std::to_string(word_from_fixed(movement.world_y)) +
                ":action_result=" +
                std::to_string(static_cast<int>(movement.action_result)));
        }
        active = movement.active;
        if (!active) {
            release_slot(slot);
        }
    };

    auto spawn = field_weapon_projectiles_.begin();
    while (spawn != field_weapon_projectiles_.end()) {
        advance_record(*spawn, true);
        advance_record(*spawn, false);
        if (!spawn->primary_active && !spawn->secondary_active) {
            spawn = field_weapon_projectiles_.erase(spawn);
        } else {
            ++spawn;
        }
    }
}

bool FieldSceneRuntime::apply_script_combat_meter_update(
    const scripting::ScriptCombatMeterUpdate& update,
    const std::string_view boundary_prefix) {
    const auto report = [this, boundary_prefix](const std::string_view suffix) {
        std::string boundary(boundary_prefix);
        boundary.push_back('.');
        boundary.append(suffix);
        source_boundary(boundary);
    };
    if (!update.changed) {
        return true;
    }
    if (update.record_type < FieldPartyRosterRuntime::kSlotCount) {
        const auto slot = static_cast<std::size_t>(
            update.source_record_slot);
        if (slot >= FieldPartyRosterRuntime::kSlotCount ||
            !field_party_roster_.slot_active(slot)) {
            report("party_target_invalid");
            return false;
        }
        auto& target = field_party_roster_.records()[slot];
        if (target.record_type != update.record_type) {
            report("party_target_type_mismatch");
            return false;
        }
        target.delay_meter = update.physical_meter;
        target.behavior_meter = update.mental_meter;
        if (update.source_record_flags_updated) {
            target.flags = static_cast<std::uint16_t>(
                (target.flags & 0x00ffU) |
                (static_cast<std::uint16_t>(
                     update.source_record_flags) << 8U));
        }
        if (update.physical && target.delay_meter == 0U &&
            (target.flags & kRunnerFlagsDown) == 0U) {
            // ROM 0xAFF2 -> 0xB330: a zero +0x4C downs the runner. Lua owns
            // the flag, sound, C0B2 selector and the 0xABB6 wipe check.
            const auto& records = field_party_roster_.records();
            const auto status = [this, &records](const std::size_t index) {
                const auto flags =
                    static_cast<std::int64_t>(records[index].flags >> 8U);
                return field_party_roster_.slot_active(index)
                    ? flags : (flags & ~std::int64_t{0x01});
            };
            const auto heading = slot == field_party_roster_.active_slot()
                ? field_movement_state_.heading
                : std::uint16_t{0U};
            const auto down = game_scripts_.call_integers(
                "party_down", "runner_down_flat",
                {static_cast<std::int64_t>(slot), status(0U), status(1U),
                 status(2U), static_cast<std::int64_t>(target.kind),
                 static_cast<std::int64_t>(heading)});
            if (!down.has_value() || down->size() != 4U) {
                failure_ = game_scripts_.failure();
                report("party_runner_down_failed");
                return false;
            }
            target.flags = static_cast<std::uint16_t>(
                (target.flags & 0x00ffU) |
                (static_cast<std::uint16_t>((*down)[0] & 0xff) << 8U));
            if ((*down)[1] != 0 && source_audio_event_callback_) {
                source_audio_event_callback_(
                    static_cast<std::uint16_t>((*down)[1]));
            }
            if ((*down)[3] != 0) {
                field_party_wipe_countdown_ =
                    static_cast<std::uint8_t>((*down)[3]);
                report("party_wipe_countdown_started");
            }
            report("party_runner_down");
        }
        if (slot == field_party_roster_.active_slot()) {
            refresh_field_hud_runner_slot();
        }
        report(update.physical ? "party_physical_meter_changed"
                               : "party_mental_meter_changed");
        return true;
    }

    if (update.source_record_slot <
            static_cast<std::uint8_t>(FieldWorldActorSlot::slot_3) ||
        update.source_record_slot >
            static_cast<std::uint8_t>(FieldWorldActorSlot::slot_9)) {
        report("world_target_slot_invalid");
        return false;
    }
    const auto target = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [this, &update](const auto& actor) {
            return actor.active && actor.scene_reference ==
                       state_.field_scene_buffers.scene_reference &&
                   actor.source_record_slot.has_value() &&
                   static_cast<std::uint8_t>(*actor.source_record_slot) ==
                       update.source_record_slot &&
                   actor.actor_instance_id == update.target_id &&
                   actor.combatant.record_type == update.record_type;
        });
    if (target == field_default_actors_.end()) {
        report("world_target_missing");
        return false;
    }

    const bool physically_defeated = update.physical &&
        target->combatant.physical_meter != 0U &&
        update.physical_meter == 0U;
    const scripting::ScriptActorDefeatFacts defeat_facts{
        target->combatant.race_or_state};

    target->combatant.physical_meter = update.physical_meter;
    target->combatant.mental_meter = update.mental_meter;
    if (update.source_record_flags_updated) {
        target->source_record_flags = update.source_record_flags;
        target->combatant.source_flags = update.source_record_flags;
    }
    // This meter gates the live actor collision record. Keep the update on
    // that record as well as in the Lua-owned combat facts so later movement,
    // collision, and drawing passes see one current actor state.
    target->physical_meter = update.physical_meter;
    if (physically_defeated) {
        const auto decision =
            game_scripts_.resolve_active_run_actor_defeat(defeat_facts);
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            report("run_actor_defeat_failed");
            return false;
        }
        if (decision->action ==
            scripting::ScriptRunActorDefeatAction::objective_advanced) {
            report("run_actor_objective_advanced");
        }
    }
    report(update.physical ? "world_physical_meter_changed"
                           : "world_mental_meter_changed");
    return true;
}

void FieldSceneRuntime::advance_field_actor_animation() {
    if (field_entity_.active) {
        const auto previous_frame = field_entity_animation_.frame;
        const auto previous_animation = field_entity_animation_id_;
        const auto decision = heartbeat_field_player(
            scripting::ScriptActorHeartbeatPhase::render);
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.player.lua_animation_failed");
        } else {
            field_entity_.direction = decision->direction;
            field_entity_animation_.frame = decision->animation_frame;
            field_entity_animation_.accumulator =
                decision->animation_accumulator;
            field_entity_animation_id_ = decision->animation_id;
            field_entity_.spell_effect_visible =
                decision->spell_effect_visible;
            field_entity_.spell_effect_action_code =
                decision->spell_effect_action_code;
            field_entity_.spell_effect_phase = decision->spell_effect_phase;
            if (decision->spell_effect_resolution.has_value()) {
                if (decision->combat_meter_update.has_value() &&
                    !apply_script_combat_meter_update(
                        *decision->combat_meter_update,
                        "field.player.spell.effect")) {
                    failure_ =
                        "Lua spell effect targeted an unavailable runner record";
                    source_boundary(
                        "field.player.spell.effect_apply_failed");
                } else {
                    source_boundary("field.player.spell.effect_resolved");
                }
            }
            if (decision->spell_effect_slot_released.has_value()) {
                const auto slot = *decision->spell_effect_slot_released;
                const auto active_party_target_id =
                    "party.runner." + std::to_string(
                        field_party_roster_.active_slot());
                if (slot >= field_world_effect_slots_occupied_.size() ||
                    !field_world_effect_slots_occupied_[slot] ||
                    (field_spell_effect_slot_target_ids_[slot] !=
                         "field_player_runner" &&
                     field_spell_effect_slot_target_ids_[slot] !=
                         active_party_target_id)) {
                    failure_ = "Lua released an unowned runner spell effect record";
                    source_boundary(
                        "field.player.spell.effect_slot_release_invalid");
                } else {
                    field_world_effect_slots_occupied_[slot] = false;
                    field_spell_effect_slot_target_ids_[slot].clear();
                    source_boundary(
                        "field.player.spell.effect_slot_released");
                }
            }
            const auto selector_changed =
                decision->animation_selector.has_value() &&
                *decision->animation_selector !=
                    field_entity_animation_selector_;
            if (decision->animation_selector.has_value()) {
                field_entity_animation_selector_ =
                    *decision->animation_selector;
            }
            field_entity_animation_.redraw_requested =
                selector_changed ||
                previous_frame != field_entity_animation_.frame ||
                previous_animation != field_entity_animation_id_;
        }
    }
}

void FieldSceneRuntime::advance_tarislar_tomb_outro() {
    const auto scene_id = state_.field_scene_buffers.scene_reference;
    std::array<std::string, 6> actor_ids{};
    if (scene_id == 55U) {
        for (const auto& actor : field_default_actors_) {
            if (!actor.active || actor.scene_reference != scene_id ||
                !actor.source_record_slot.has_value()) {
                continue;
            }
            const auto slot = static_cast<std::uint8_t>(
                *actor.source_record_slot);
            if (slot < 4U || slot > 9U) {
                continue;
            }
            auto& actor_id = actor_ids[static_cast<std::size_t>(slot - 4U)];
            if (!actor_id.empty()) {
                source_boundary(
                    "field.scene55.tomb_outro_duplicate_actor_slot");
                return;
            }
            actor_id = actor.actor_instance_id;
        }
    }

    const auto step = game_scripts_.advance_tarislar_tomb_outro(
        scene_id, world_frame_logic_state_.scene_state, actor_ids, false);
    if (!step.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.scene55.tomb_outro_lua_failed");
        return;
    }
    if (*step == scripting::ScriptTarislarTombOutroStep::unresolved) {
        source_boundary("field.scene55.tomb_outro_actor_state_unresolved");
        return;
    }
    if (*step != scripting::ScriptTarislarTombOutroStep::completion_ready ||
        field_activity_continuation_ ==
            MovementModeTransitionContinuation::
                scene_55_tomb_credits_callback) {
        return;
    }

    const auto continuation =
        MovementModeTransitionContinuation::scene_55_tomb_credits_callback;
    if (!begin_world_activity_if_idle(
            field_world_activity_state_,
            static_cast<std::uint16_t>(continuation))) {
        return;
    }
    world_frame_logic_state_.resource_countdown =
        field_world_activity_state_.timeout;
    record_field_activity_start(continuation);
    source_boundary("field.scene55.tomb_outro_callback_queued");
}

void FieldSceneRuntime::advance_default_field_actor_animations() {
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    for (auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene) {
            continue;
        }
        auto context = scripting::ScriptActorBehaviorContext{
            actor.script_profile_id,
            actor.entity_kind,
            actor.action_kind,
            actor.initial_action_code,
            actor.initial_behavior_delay,
            actor.transition_started,
            actor.transition_complete,
            actor.actor_instance_id,
            actor.initial_heading,
            false,
        };
        context.movement_scene_id = current_scene;
        if (actor.source_record_slot.has_value()) {
            context.source_record_slot = static_cast<std::uint8_t>(
                *actor.source_record_slot);
        }
        if (actor.temporary_world_encounter ||
            actor.world_activity_script_actor) {
            context.dynamic_world_encounter_actor = true;
            context.world_activity_script_actor =
                actor.world_activity_script_actor;
            context.combatant = actor.combatant;
            context.source_actor_posture = actor.combatant.posture;
            context.source_record_flags = actor.source_record_flags;
            context.source_interaction_flags =
                actor.source_interaction_flags;
            context.source_behavior_flags = actor.source_behavior_flags;
            context.source_animation_selector =
                actor.source_animation_selector;
            context.source_item_spawn_result =
                actor.source_item_spawn_result;
            context.source_spawn_state_flags =
                actor.source_spawn_state_flags;
            context.world_activity_active =
                world_frame_logic_state_.world_active != 0U;
            context.world_scene_state =
                world_frame_logic_state_.scene_state;
            context.entry_animation_frame_count =
                actor.entry_animation_frame_count;
            context.ai_animation_frame = actor.animation_frame;
            context.walk_animation_frame_count =
                actor.walk_animation_frame_count;
            context.action_animation_frame_count =
                actor.action_animation_frame_count;
            context.action_animation_increment =
                actor.action_animation_increment;
            context.action_animation_threshold =
                actor.action_animation_threshold;
            context.item_action_frame_count =
                actor.item_action_frame_count;
            context.item_action_increment = actor.item_action_increment;
            context.item_action_threshold = actor.item_action_threshold;
            context.action_animation_audio_event =
                actor.action_animation_audio_event;
            context.hurt_animation_frame_count =
                actor.hurt_animation_frame_count;
            context.hurt_animation_increment =
                actor.hurt_animation_increment;
            context.hurt_animation_threshold =
                actor.hurt_animation_threshold;
            context.hurt_audio_event = actor.hurt_audio_event;
            context.down_audio_event = actor.down_audio_event;
            context.initial_action_delay = actor.action_delay;
            context.delay_primary_attribute =
                actor.delay_primary_attribute;
            context.delay_secondary_attribute =
                actor.delay_secondary_attribute;
            context.delay_capability_flags =
                actor.delay_capability_flags;
            context.world_effect_slot_occupancy =
                field_world_effect_slots_occupied_;
            context.source_projection_active = caster_projection_active(
                actor.world_position.x, actor.world_position.y,
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_x),
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_y));
            if (actor.effect_slot.has_value()) {
                context.world_effect_slot_occupancy[*actor.effect_slot] = true;
            }

            // The delayed 0x521E effect callback needs the current linked
            // combat records when its final render heartbeat reaches 0x55B78.
            // These are source actor facts only; Lua retains the pending
            // target identity and decides when the effect lands.
            context.combat_meter_service_blocked =
                field_movement_transition_state_.world_mode == 3U;
            for (std::size_t slot = 0U;
                 slot < FieldPartyRosterRuntime::kSlotCount; ++slot) {
                if (!field_party_roster_.slot_active(slot)) {
                    continue;
                }
                auto position = state_.field_scene_buffers.source_metadata
                                    .initial_field_records[slot];
                const bool active_field_runner =
                    slot == field_party_roster_.active_slot() &&
                    field_entity_.active;
                if (active_field_runner) {
                    position = field_entity_.world_position;
                }
                const auto& runner = field_party_roster_.records()[slot];
                scripting::ScriptActorTargetCandidate candidate{};
                // Preserve the behavior-pass identity so a Lua actor can
                // retain its ROM link through the separate render heartbeat.
                candidate.id = "party.runner." + std::to_string(slot);
                candidate.world_x = position.x;
                candidate.world_y = position.y;
                candidate.service_flags = static_cast<std::uint8_t>(
                    runner.flags >> 8U);
                candidate.interaction_flags = static_cast<std::uint8_t>(
                    runner.flags & 0x00FFU);
                candidate.has_action = true;
                candidate.world_activity_actor = false;
                candidate.source_record_slot =
                    static_cast<std::uint8_t>(slot);
                if (active_field_runner) {
                    candidate.source_record_heading =
                        field_movement_state_.heading;
                }
                candidate.combatant = script_combatant_facts(runner);
                context.ai_target_candidates.push_back(
                    std::move(candidate));
            }
            for (const auto& target : field_default_actors_) {
                if (!target.active || target.scene_reference != current_scene ||
                    !target.source_record_slot.has_value() ||
                    target.combatant.record_type < 3U) {
                    continue;
                }
                scripting::ScriptActorTargetCandidate candidate{};
                candidate.id = target.actor_instance_id;
                candidate.world_x = target.world_position.x;
                candidate.world_y = target.world_position.y;
                candidate.service_flags = target.source_record_flags;
                candidate.interaction_flags =
                    target.source_interaction_flags;
                candidate.behavior_flags = target.source_behavior_flags;
                candidate.spawn_state_flags =
                    target.source_spawn_state_flags;
                candidate.has_action = target.physical_meter != 0U;
                candidate.world_activity_actor = true;
                candidate.source_record_slot = static_cast<std::uint8_t>(
                    *target.source_record_slot);
                candidate.source_record_heading =
                    target.movement_state.heading;
                candidate.combatant = target.combatant;
                context.ai_target_candidates.push_back(
                    std::move(candidate));
            }
            context.ai_random_bound = [this](const std::uint16_t bound)
                -> std::optional<std::uint16_t> {
                if (bound == 0U) {
                    return std::nullopt;
                }
                const auto choice =
                    sample_source_random_bound(field_random_seed_, bound);
                if (choice < 0 || choice >= bound) {
                    return std::nullopt;
                }
                return static_cast<std::uint16_t>(choice);
            };
        }
        context.phase = scripting::ScriptActorHeartbeatPhase::render;
        context.target_selection_latched = actor.target_selected;
        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.actor.lua_animation_failed");
            actor.active = false;
            continue;
        }
        actor.animation_direction = decision->direction;
        actor.animation_id = decision->animation_id;
        actor.animation_frame = decision->animation_frame;
        if (actor.temporary_world_encounter ||
            actor.world_activity_script_actor) {
            actor.source_record_flags = decision->source_record_flags;
            actor.source_behavior_flags = decision->source_behavior_flags;
            actor.source_animation_selector =
                decision->source_animation_selector;
            actor.source_item_spawn_result.reset();
            actor.combatant.source_flags = actor.source_record_flags;
        }
        if (decision->spell_effect_resolution.has_value()) {
            if (decision->combat_meter_update.has_value() &&
                !apply_script_combat_meter_update(
                    *decision->combat_meter_update,
                    "field.actor.spell.effect")) {
                failure_ =
                    "Lua spell effect targeted an unavailable combat record";
                source_boundary("field.actor.spell.effect_apply_failed");
            } else {
                source_boundary("field.actor.spell.effect_resolved");
            }
        }
        actor.spell_effect_visible = decision->spell_effect_visible;
        actor.spell_effect_action_code =
            decision->spell_effect_action_code;
        actor.spell_effect_phase = decision->spell_effect_phase;
        if (decision->spell_effect_slot_released.has_value()) {
            const auto slot = *decision->spell_effect_slot_released;
            if (slot >= field_world_effect_slots_occupied_.size() ||
                !field_world_effect_slots_occupied_[slot] ||
                field_spell_effect_slot_target_ids_[slot] !=
                    actor.actor_instance_id) {
                failure_ = "Lua released an unowned spell effect record";
                source_boundary(
                    "field.actor.spell.effect_slot_release_invalid");
            } else {
                field_world_effect_slots_occupied_[slot] = false;
                field_spell_effect_slot_target_ids_[slot].clear();
                source_boundary("field.actor.spell.effect_slot_released");
            }
        }
        actor.target_marker_visible = decision->target_marker_visible;
        actor.target_marker_animating = decision->target_marker_animating;
        actor.target_marker_style = decision->target_marker_style;
        actor.target_marker_radius = decision->target_marker_radius;
        actor.action_delay = decision->action_delay;
        actor.target_motion_effect_visible =
            decision->target_motion_effect_visible;
        actor.target_motion_effect_direction =
            decision->target_motion_effect_direction;
        actor.target_motion_effect_phase =
            decision->target_motion_effect_phase;
        if (decision->target_motion_effect_slot_released) {
            if (!actor.target_motion_effect_slot.has_value() ||
                !field_world_effect_slots_occupied_[
                    *actor.target_motion_effect_slot]) {
                source_boundary(
                    "field.actor.hound_effect_render_release_invalid");
            } else {
                field_world_effect_slots_occupied_[
                    *actor.target_motion_effect_slot] = false;
                actor.target_motion_effect_slot.reset();
                source_boundary("field.actor.hound_effect_render_finished");
            }
        }
        actor.transition_started = decision->transition_started;
        actor.transition_complete = decision->transition_complete;
    }
}

void FieldSceneRuntime::advance_field_weapon_projectile_animations() {
    const auto release_slot = [this](
                                   std::optional<std::uint16_t>& slot) {
        if (slot.has_value() && *slot >= 9U && *slot <= 14U) {
            field_weapon_slots_occupied_[
                static_cast<std::size_t>(*slot - 9U)] = false;
        }
        slot.reset();
    };
    const auto advance_record = [this, &release_slot](
                                    SpawnEntityRecordState& record,
                                    EntityAnimationState& animation,
                                    bool& active,
                                    bool& script_initialized,
                                    std::optional<std::uint16_t>& slot) {
        if (!active) {
            return;
        }
        const auto disable_record = [&] {
            record.flags = static_cast<std::uint8_t>(
                record.flags & static_cast<std::uint8_t>(~0x01U));
            active = false;
            release_slot(slot);
        };
        if (!slot.has_value()) {
            source_boundary("field.combat.weapon_projectile_slot_missing");
            disable_record();
            return;
        }

        const auto selector = static_cast<std::uint16_t>(
            record.animation_resource & 0x7FFFU);
        const auto animation_index =
            static_cast<std::size_t>(selector / 4U);
        if ((selector & 0x0003U) != 0U ||
            animation_index >= field_weapon_projectile_images_.size()) {
            source_boundary(
                "field.combat.weapon_projectile_selector_invalid");
            disable_record();
            return;
        }
        const auto frame_count =
            field_weapon_projectile_images_[animation_index].size();
        if (frame_count == 0U || frame_count > 16U) {
            source_boundary("field.combat.weapon_projectile_frames_invalid");
            disable_record();
            return;
        }

        const auto instance_id = "field_weapon_slot_" +
                                 std::to_string(*slot);
        scripting::ScriptActorBehaviorContext context{};
        context.profile_id = "weapon_projectile";
        context.entity_kind = record.behavior_defaults.animation_state;
        context.action_kind = scripting::ScriptActorActionKind::none;
        context.action_code = 0U;
        context.instance_id = instance_id;
        context.initial_heading = record.heading;
        context.spawn = !script_initialized;
        context.phase = scripting::ScriptActorHeartbeatPhase::render;
        context.projectile_animation =
            scripting::ScriptActorProjectileAnimation{
                record.animation_resource,
                static_cast<std::uint16_t>(frame_count),
                animation.frame,
                animation.accumulator,
                animation.increment,
                animation.period,
                true};
        const auto previous_frame = animation.frame;
        const auto previous_selector = record.animation_resource;
        const auto decision = game_scripts_.actor_heartbeat(context);
        if (!decision.has_value() ||
            !decision->animation_selector.has_value() ||
            !decision->animation_increment.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.combat.weapon_projectile_lua_animation_failed");
            disable_record();
            return;
        }

        script_initialized = true;
        record.animation_resource = *decision->animation_selector;
        animation.frame = decision->animation_frame;
        animation.accumulator = decision->animation_accumulator;
        animation.increment = *decision->animation_increment;
        animation.redraw_requested =
            previous_frame != animation.frame ||
            previous_selector != record.animation_resource;
    };

    for (auto& spawn : field_weapon_projectiles_) {
        advance_record(spawn.primary_record, spawn.primary_animation,
                       spawn.primary_active,
                       spawn.primary_animation_script_initialized,
                       spawn.primary_slot);
        advance_record(spawn.source_record, spawn.animation,
                       spawn.secondary_active,
                       spawn.secondary_animation_script_initialized,
                       spawn.secondary_slot);
    }
}

void FieldSceneRuntime::update_source_field_camera() {
    const auto& profile = state_.field_scene_buffers.profile;
    if (profile.size() < 6U) {
        source_boundary("field.camera.bounds_missing");
        return;
    }
    const auto camera_x = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_x);
    const auto camera_y = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_y);
    if (!field_camera_alignment_ready_) {
        field_camera_follow_.aligned_x =
            static_cast<std::uint16_t>(camera_x & 0xFF80U);
        field_camera_follow_.aligned_y =
            static_cast<std::uint16_t>(camera_y & 0xFF80U);
        field_camera_alignment_ready_ = true;
    }
    field_camera_follow_.camera_x = camera_x;
    field_camera_follow_.camera_y = camera_y;
    field_camera_follow_.minimum_x = profile[2];
    field_camera_follow_.minimum_y = profile[3];
    field_camera_follow_.maximum_x = profile[4];
    field_camera_follow_.maximum_y = profile[5];
    field_camera_follow_.interaction_locked =
        field_movement_transition_state_.world_interaction_lock;
    field_camera_follow_ = follow_field_camera(
        field_camera_follow_, field_entity_.world_position.x,
        field_entity_.world_position.y, field_movement_state_.heading, false);
    state_.field_scene_buffers.camera_x = field_camera_follow_.camera_x;
    state_.field_scene_buffers.camera_y = field_camera_follow_.camera_y;
    const auto row_stride = profile.size() > 18U ? profile[18U] : 0U;
    if (field_camera_follow_.horizontal_edge) {
        FieldTileRowEdgeState row_edge{};
        row_edge.horizontal_direction = field_camera_follow_.horizontal_step;
        row_edge.camera_x = field_camera_follow_.camera_x;
        row_edge.camera_x_limit = static_cast<std::int16_t>(
            field_camera_follow_.maximum_x);
        row_edge.camera_y = field_camera_follow_.camera_y;
        row_edge.field_y = field_camera_follow_.camera_y;
        row_edge.row_stride_bytes = row_stride;
        render_field_tile_row_edge(
            row_edge, {[this](const FieldTileRowEdgeRequest&) {
                source_boundary("field.camera.row_edge");
            }});
    }
    if (field_camera_follow_.vertical_edge) {
        FieldTileColumnEdgeState column_edge{};
        column_edge.vertical_direction = field_camera_follow_.vertical_step;
        column_edge.field_y = field_camera_follow_.camera_y;
        column_edge.field_y_limit = static_cast<std::int16_t>(
            field_camera_follow_.maximum_y >> 4U);
        column_edge.camera_x = field_camera_follow_.camera_x;
        column_edge.camera_y = field_camera_follow_.camera_y;
        column_edge.row_stride_bytes = row_stride;
        render_field_tile_column_edge(
            column_edge, {[this](const FieldTileColumnEdgeRequest&) {
                source_boundary("field.camera.column_edge");
            }});
    }
}

void FieldSceneRuntime::service_pending_field_viewports() {
    // ROM 0x1872 runs from the VBlank entry and services FFE100/FFE102 after
    // the field-camera pass. The Qt host commits the requested viewport
    // redraw after projecting entities, then paints the scene from its native
    // map and state on the next frame.
    if (viewport_state_.normal_render_pending != 0U &&
        callbacks_.field_viewports.render_normal_viewport) {
        callbacks_.field_viewports.render_normal_viewport();
    }
    if (viewport_state_.alternate_render_pending != 0U &&
        callbacks_.field_viewports.render_alternate_viewport) {
        callbacks_.field_viewports.render_alternate_viewport();
    }
}

void FieldSceneRuntime::advance_frame() {
    if (!ready_) {
        return;
    }
    service_scene_music_latch();
    if (tarislar_credits_title_return_.is_presenting()) {
        const auto update = tarislar_credits_title_return_.advance();
        if (update.completed) {
            title_return_requested_ = true;
        }
        return;
    }
    if (field_door_return_active_ && advance_field_door_return()) {
        return;
    }
    if (field_entity_.world_position.x != field_frame_start_player_position_.x ||
        field_entity_.world_position.y != field_frame_start_player_position_.y) {
        field_pre_step_player_position_ = field_frame_start_player_position_;
    }
    field_frame_start_player_position_ = field_entity_.world_position;
    // A modal dismissal owns the next host transition interval. Fade the
    // replacement field in over sixty host frames and consume every
    // button held or pressed during that interval so its close input cannot
    // become a field action on the first live frame.
    field_modal_return_blocked_inputs_ = static_cast<std::uint8_t>(
        field_modal_return_blocked_inputs_ & field_controller_port_byte_);
    if (field_modal_return_transition_frames_remaining_ != 0U) {
        field_modal_return_blocked_inputs_ = static_cast<std::uint8_t>(
            field_modal_return_blocked_inputs_ | field_controller_port_byte_);
        --field_modal_return_transition_frames_remaining_;
        field_palette_fade_steps_ = static_cast<std::uint8_t>(
            (static_cast<unsigned>(
                 field_modal_return_transition_frames_remaining_) *
                 9U +
             (kFieldModalReturnTransitionFrames / 2U)) /
            kFieldModalReturnTransitionFrames);
        field_combat_input_suppressed_until_release_ =
            (field_modal_return_blocked_inputs_ & 0x40U) != 0U;
        if (field_modal_return_transition_frames_remaining_ == 0U) {
            source_boundary("field.modal_return.transition_completed");
        }
        return;
    }
    field_combat_input_suppressed_until_release_ =
        (field_modal_return_blocked_inputs_ & 0x40U) != 0U;
    if (field_modal_return_blocked_inputs_ != 0U) {
        source_boundary("field.modal_return.waiting_for_input_release");
        return;
    }
    if (field_elevator_scene_transition_.is_active()) {
        advance_field_elevator_scene_transition();
        return;
    }
    if (field_elevator_selector_.is_open()) {
        advance_field_elevator_selector();
        return;
    }
    if (field_interaction_.open()) {
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        (void)advance_field_interaction(
            field_controller_sample_state_.events);
        return;
    }
    if (hotel_menu_.open()) {
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        if (hotel_karma_skills_.is_open()) {
            if (hotel_karma_load_frames_ > 0U) {
                --hotel_karma_load_frames_;
                return;
            }
            (void)process_hotel_karma_screen_events(
                field_controller_sample_state_.events);
            return;
        }
        hotel_menu_.advance_animation(field_random_seed_);
        (void)process_hotel_menu_events(
            field_controller_sample_state_.events);
        return;
    }
    if (field_shop_notice_active_) {
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        if ((field_controller_sample_state_.events & 0xFF00U) != 0U) {
            source_boundary("field.shop." + field_shop_notice_service_id_ +
                            ".store_closed_dismissed");
            field_shop_notice_active_ = false;
            field_shop_notice_service_id_.clear();
            field_shop_notice_text_key_.clear();
            state_.display_service_word = 0U;
            field_runtime_state_.display_service_word = 0U;
        }
        return;
    }
    if (field_mode_text_.open()) {
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        const auto result = field_mode_text_.advance_controller_events(
            field_controller_sample_state_.events);
        if (result == FieldModeTextAdvanceResult::returned_to_route) {
            source_boundary("field.mode_text_prompt.returned_to_caller");
            if (!field_shop_route_pending_service_id_.empty()) {
                const auto service_id =
                    field_shop_route_pending_service_id_;
                field_shop_route_pending_service_id_.clear();
                // shop.lua owns the per-store menu kind, boundary tag and
                // open gate; the host only applies them.
                const auto host_profile = game_scripts_.string_map(
                    "shop", "host_open_profile", service_id);
                const auto profile_value =
                    [&host_profile](const char* key,
                                    std::string_view fallback) {
                        if (!host_profile.has_value()) {
                            return std::string(fallback);
                        }
                        const auto found = host_profile->find(key);
                        return found == host_profile->end()
                            ? std::string(fallback)
                            : found->second;
                    };
                const auto menu_kind = profile_value("menu_kind", "field_store");
                const auto shop_tag = profile_value("boundary_tag", service_id);
                if (profile_value("gate", "open") == "runner_deck_fact") {
                    if (!field_runner_feature_flags_.has_value()) {
                        field_shop_active_definition_.reset();
                        source_boundary("field.shop." + service_id +
                                        ".deck_fact_unresolved");
                        return;
                    }
                    if ((*field_runner_feature_flags_ & 0x01U) == 0U) {
                        scripting::ScriptFieldModeContext context{
                            5U, 4U, 0};
                        context.service_id = service_id;
                        context.prompt_variant = "store_closed";
                        game_scripts_.set_field_mode_context(
                            std::move(context));
                        const auto content = game_scripts_.string_map(
                            "field_mode", "mode_text_content");
                        if (!content.has_value()) {
                            field_shop_active_definition_.reset();
                            source_boundary("field.shop." + service_id +
                                            ".store_closed_text_unavailable");
                            return;
                        }
                        const auto status = content->find("status");
                        const auto text_key = content->find("text_key");
                        if (status == content->end() ||
                            status->second != "matched" ||
                            text_key == content->end() ||
                            game_scripts_.dialogue_text(text_key->second) ==
                                nullptr) {
                            field_shop_active_definition_.reset();
                            source_boundary("field.shop." + service_id +
                                            ".store_closed_text_unavailable");
                            return;
                        }
                        field_shop_notice_active_ = true;
                        field_shop_notice_service_id_ = service_id;
                        field_shop_notice_text_key_ = text_key->second;
                        field_shop_active_definition_.reset();
                        source_boundary("field.shop." + service_id +
                                        ".store_closed");
                        return;
                    }
                }
                field_shop_active_definition_ =
                    game_scripts_.shop_definition(service_id);
                const auto field_store_shape =
                    field_shop_active_definition_.has_value() &&
                    ((field_shop_active_definition_->menu_result == -1 &&
                      field_shop_active_definition_->groups.size() == 6U) ||
                     (field_shop_active_definition_->menu_result == 0 &&
                      field_shop_active_definition_->groups.size() == 4U) ||
                     (menu_kind == "wire_masters" &&
                      field_shop_active_definition_->menu_mode == 7U &&
                      field_shop_active_definition_->menu_result == 1 &&
                      field_shop_active_definition_->groups.size() == 1U &&
                      field_shop_active_definition_
                              ->groups.front().equipment_upgrade_offers.size() ==
                          8U));
                if (!field_store_shape) {
                    source_boundary("field.shop." + service_id +
                                    ".menu_unimplemented");
                    field_shop_active_definition_.reset();
                    return;
                }
                const auto shop_id = menu_kind == "medicine_lodge"
                    ? ShopId::medicine_lodge
                    : menu_kind == "wire_masters"
                          ? ShopId::wire_masters
                          : ShopId::field_store;
                const auto script_menu = game_scripts_.open_shop_menu(
                    service_id);
                if (field_shop_active_definition_.has_value() &&
                    script_menu.has_value() &&
                    field_shop_menu_.open(
                        shop_id,
                        *field_shop_active_definition_,
                        field_menu_text_.confirmation_choice_ids) &&
                    field_shop_menu_.apply_script_snapshot(*script_menu)
                        .has_value()) {
                    source_boundary(std::string("field.shop.") +
                                    std::string(shop_tag) + ".opened");
                } else {
                    (void)game_scripts_.close_shop_menu();
                    field_shop_active_definition_.reset();
                    source_boundary(std::string("field.shop.") +
                                    std::string(shop_tag) + ".open_failed");
                }
            } else if (field_mode_entry_transition_pending_) {
                if (!resume_field_mode_entry_route()) {
                    source_boundary(
                        "field.entry.area_event_route_resume_failed");
                }
                begin_field_modal_return_transition();
            }
        }
        return;
    }
    if (field_dialogue_.open()) {
        advance_hospital_grid_effect();
        // The source prompt loop waits for a high-byte controller edge. The
        // host keeps that loop non-blocking and suspends field movement while
        // the common dialogue panel owns the current prompt.
        // During the 0xC24 page wait the source does not call 0x195C, so
        // the pad is not sampled and a button held past it is a new edge.
        const bool dialogue_input_locked =
            field_dialogue_.consume_input_lockout_frame();
        if (!dialogue_input_locked) {
            field_controller_latch_state_.events =
                field_controller_sample_state_.events;
            field_controller_latch_state_.extra_events = 0U;
            (void)::shadowrun::ghidra_conversion::latch_controller_events(
                field_controller_latch_state_);
            ::shadowrun::ghidra_conversion::sample_controller_input(
                field_controller_sample_state_, field_controller_port_byte_);
        }
        const auto result = field_dialogue_.advance_controller_events(
            dialogue_input_locked
                ? std::uint16_t{0U}
                : field_controller_sample_state_.events);
        (void)process_pending_story_world_effect_sequence();
        if (result == FieldDialogueAdvanceResult::typed_outcome_requested) {
            const auto outcome = field_dialogue_.take_gameplay_task_outcome();
            if (!outcome.has_value()) {
                const std::string reason =
                    "field.dialogue.typed_outcome_missing";
                source_boundary(reason);
                (void)field_dialogue_.present_source_boundary_for_host(reason);
            } else {
                (void)dispatch_dialogue_outcome(*outcome);
            }
        } else if (result == FieldDialogueAdvanceResult::returned_to_field) {
            hospital_service_session_active_ = false;
            source_boundary("field.dialogue.returned_to_field");
            bool taxi_departure_started = false;
            if (field_taxi_cabbie_destination_.has_value()) {
                const auto destination = *field_taxi_cabbie_destination_;
                const auto task_result =
                    field_dialogue_.take_gameplay_task_result();
                field_taxi_cabbie_destination_.reset();
                if (task_result ==
                    std::optional<std::string>{"taxi_ride_accepted"}) {
                    // ROM 0x208FE writes the accepted region to FFFBF8. The
                    // field buffer/coordinator share that byte as
                    // field_resource_mode; keep their mirrors current before
                    // applying selector 0x136B6's FFFBF8 vs FFDDC4 check [C].
                    const auto accepted_field_mode =
                        game_scripts_.field_mode_context_value();
                    state_.field_scene_buffers.field_resource_mode =
                        accepted_field_mode;
                    state_.field_resource_mode = accepted_field_mode;
                    field_runtime_state_.field_mode = accepted_field_mode;
                    if (state_.field_scene_buffers.field_resource_mode ==
                        state_.field_scene_buffers.scene_reference) {
                        source_boundary(
                            "field.taxi.accepted_current_scene_no_travel");
                    } else {
                        field_taxi_pending_destination_ = destination;
                        if (field_elevator_scene_transition_.begin_fade_out()) {
                            field_taxi_transition_active_ = true;
                            taxi_departure_started = true;
                            source_boundary(
                                "field.taxi.destination_fade_out_started");
                        } else {
                            field_taxi_pending_destination_.reset();
                            source_boundary(
                                "field.taxi.destination_transition_busy");
                        }
                    }
                }
            }
            if (taxi_departure_started) {
                // The cabbie task has already accepted and started the taxi
                // route; the next host frame advances its source fade.
            } else if (field_mode_entry_transition_pending_) {
                const auto expected_result =
                    active_field_script_route_.has_value()
                        ? active_field_script_route_->
                              entry_continue_result_id
                        : std::string{};
                const bool entry_allows_transition =
                    expected_result.empty() ||
                    field_dialogue_.take_gameplay_task_result() ==
                        std::optional<std::string>{expected_result};
                if (entry_allows_transition) {
                    if (!resume_field_mode_entry_route()) {
                        source_boundary(
                            "field.entry.area_event_route_resume_failed");
                    }
                } else {
                    field_mode_entry_transition_pending_ = false;
                    field_mode_entry_destination_scene_.reset();
                    field_mode_entry_run_target_actor_group_.reset();
                    source_boundary(
                        "field.entry.location_dialogue_denied_transition");
                    advance_field_scene_transition_after_modal();
                }
                begin_field_modal_return_transition();
            } else {
                // Place first: the suspended same-scene re-entry below must
                // not consume or discard the service return point.
                (void)apply_field_service_close_return();
                advance_field_scene_transition_after_modal();
                begin_field_modal_return_transition();
            }
        } else if (result ==
                   FieldDialogueAdvanceResult::returned_from_world_activity) {
            if (!restore_world_activity_source_scene(field_runtime_state_)) {
                failure_ = "resource activity return lost its source scene";
                source_boundary("field.activity.return_source_failed");
            } else {
                begin_field_modal_return_transition();
            }
        } else if (result ==
                   FieldDialogueAdvanceResult::hotel_options_requested) {
            // Room-payment tasks are an alternate mode service outcome. They
            // enter Hotel Options without resuming a pending area-table
            // destination such as the Inn's numeric scene-53 row.
            field_mode_entry_transition_pending_ = false;
            field_mode_entry_destination_scene_.reset();
            field_mode_entry_run_target_actor_group_.reset();
            begin_hotel_menu();
        } else if (result == FieldDialogueAdvanceResult::
                       hospital_options_requested) {
            field_mode_entry_transition_pending_ = false;
            field_mode_entry_destination_scene_.reset();
            field_mode_entry_run_target_actor_group_.reset();
            (void)begin_hospital_menu_task("present_options");
        } else if (result == FieldDialogueAdvanceResult::
                       hospital_leave_confirmation_requested) {
            (void)begin_hospital_menu_task("present_leave_confirmation");
        } else if (result == FieldDialogueAdvanceResult::hospital_left) {
            hospital_service_session_active_ = false;
            scripting::DialogueOutcome outcome =
                scripting::ReturnToOriginOutcome{"source_entry", std::nullopt};
            (void)dispatch_dialogue_outcome(outcome);
        } else if (result == FieldDialogueAdvanceResult::
                       hospital_cyberware_requested) {
            hospital_service_session_active_ = false;
            open_command_overview(
                static_cast<std::uint16_t>(srx::MenuScreen::CYBERWARE));
            source_boundary("field.hospital.cyberware_opened");
        } else if (result == FieldDialogueAdvanceResult::
                       hospital_medical_attention_requested) {
            (void)begin_hospital_menu_task("present_medical_attention");
        } else if (result == FieldDialogueAdvanceResult::
                       medicine_lodge_store_requested) {
            field_shop_active_definition_ =
                game_scripts_.shop_definition("medicine_lodge");
            const auto script_menu =
                game_scripts_.open_shop_menu("medicine_lodge");
            if (field_shop_active_definition_.has_value() &&
                script_menu.has_value() &&
                field_shop_menu_.open(
                    ShopId::medicine_lodge,
                    *field_shop_active_definition_,
                    field_menu_text_.confirmation_choice_ids) &&
                field_shop_menu_.apply_script_snapshot(*script_menu)
                    .has_value()) {
                source_boundary("field.shop.medicine_lodge.opened");
            } else {
                (void)game_scripts_.close_shop_menu();
                field_shop_active_definition_.reset();
                source_boundary(
                    "field.shop.medicine_lodge.definition_unavailable");
                (void)field_dialogue_.present_source_boundary_for_host(
                    "The ROM Medicine Lodge store definition could not be "
                    "loaded from its Lua catalog.");
            }
        } else if (result == FieldDialogueAdvanceResult::
                       medicine_lodge_wise_one_followup) {
            field_pending_scene_profile_ =
                kMedicineLodgeWiseOneEntryProfile0x1668A;
            source_boundary(
                "field.location.medicine_lodge_wise_one_profile_queued");
            advance_field_scene_transition_after_modal();
            begin_field_modal_return_transition();
        } else if (result ==
                   FieldDialogueAdvanceResult::matrix_entry_requested) {
            const auto system_id =
                field_dialogue_.take_matrix_entry_system_id();
            const auto node_id = field_dialogue_.take_matrix_entry_node_id();
            if (!system_id.has_value()) {
                source_boundary("field.matrix.entry_target_missing");
            } else {
                scripting::ScriptMenuRouteStartArguments arguments{};
                arguments.matrix_system_id = *system_id;
                arguments.matrix_node_id = node_id;
                arguments.return_to_field = true;
                open_command_overview(
                    static_cast<std::uint16_t>(
                        srx::MenuScreen::MATRIX_ENTRY),
                    std::move(arguments));
                source_boundary(node_id.has_value()
                    ? "field.matrix.resource_target_entry_requested"
                    : "field.matrix.entry_requested");
            }
        } else if (result ==
                   FieldDialogueAdvanceResult::matrix_search_requested) {
            scripting::ScriptMenuRouteStartArguments arguments{};
            arguments.return_to_field = true;
            open_command_overview(
                static_cast<std::uint16_t>(srx::MenuScreen::MATRIX_SEARCH),
                std::move(arguments));
            source_boundary("field.matrix.search_requested");
        } else if (result ==
                   FieldDialogueAdvanceResult::taxi_selector_requested) {
            const auto current_region =
                state_.field_scene_buffers.field_resource_mode;
            if (current_region > 6U) {
                source_boundary("field.taxi.current_region_unresolved");
            } else {
                scripting::ScriptMenuRouteStartArguments arguments{};
                arguments.taxi_current_region =
                    static_cast<std::uint8_t>(current_region);
                open_command_overview(
                    static_cast<std::uint16_t>(
                        srx::MenuScreen::TAXI_SELECTOR),
                    std::move(arguments));
                source_boundary("field.taxi.selector_requested");
            }
        }
        return;
    }
    if (field_shop_menu_.is_open()) {
        const auto shop_tag = [this]() -> std::string {
            switch (field_shop_menu_.shop()) {
            case ShopId::field_store:
                if (field_shop_active_definition_.has_value()) {
                    const auto& id = field_shop_active_definition_->id;
                    const auto profile = game_scripts_.string_map(
                        "shop", "host_open_profile", id);
                    if (profile.has_value()) {
                        const auto tag = profile->find("boundary_tag");
                        if (tag != profile->end()) {
                            return tag->second;
                        }
                    }
                    return id;
                }
                return "field_store";
            case ShopId::medicine_lodge: return "medicine_lodge";
            case ShopId::wire_masters: return "wire_masters";
            }
            return "shop";
        }();
        const auto shop_boundary = [this, shop_tag](
                                       const std::string_view event) {
            source_boundary(std::string("field.shop.") + shop_tag + "." +
                            std::string(event));
        };
        const auto service_wire_upgrade =
            [this, &shop_boundary](const bool confirmed) {
                if (!field_shop_active_definition_.has_value() ||
                    !field_shop_menu_.pending_action().has_value() ||
                    !field_shop_menu_.pending_action()
                         ->equipment_upgrade_offer.has_value() ||
                    !field_funds_.has_value()) {
                    shop_boundary("equipment_upgrade_state_unavailable");
                    field_shop_menu_.finish_action();
                    return;
                }
                const auto table_index = active_runner_record().magic_table_index;
                const auto* const spell_table =
                    field_magic_spell_table(table_index);
                const auto input = make_field_shop_equipment_upgrade_input(
                    *field_shop_active_definition_, field_shop_menu_,
                    *field_shop_menu_.pending_action()
                         ->equipment_upgrade_offer,
                    active_runner_record(), *field_funds_, spell_table,
                    field_runner_feature_flags_.value_or(0U));
                if (!input.has_value()) {
                    shop_boundary("equipment_upgrade_source_state_unavailable");
                    field_shop_menu_.finish_action();
                    return;
                }
                auto state = input->state;
                FieldEquipmentUpgradeCallbacks callbacks{};
                callbacks.confirm_purchase = [confirmed] {
                    return confirmed;
                };
                const auto result = apply_field_equipment_upgrade(
                    input->table, state, callbacks);
                switch (result) {
                case FieldEquipmentUpgradeResult::cancelled:
                    if (!confirmed &&
                        field_shop_menu_.begin_equipment_upgrade_confirmation(
                            state.cost) ==
                            ShopTransactionEvent::confirmation_opened) {
                        shop_boundary("equipment_upgrade_confirmation");
                    } else {
                        shop_boundary("equipment_upgrade_confirmation_unavailable");
                        field_shop_menu_.finish_action();
                    }
                    return;
                case FieldEquipmentUpgradeResult::already_owned:
                    field_shop_menu_.show_transaction_message(
                        ShopTransactionMessage::cyberware_already_owned);
                    shop_boundary("equipment_upgrade_already_owned");
                    return;
                case FieldEquipmentUpgradeResult::
                    spurs_make_hand_razors_unnecessary:
                    field_shop_menu_.show_transaction_message(
                        ShopTransactionMessage::cyberware_spurs_conflict);
                    shop_boundary("equipment_upgrade_spurs_conflict");
                    return;
                case FieldEquipmentUpgradeResult::insufficient_funds:
                    field_shop_menu_.show_transaction_message(
                        ShopTransactionMessage::insufficient_funds);
                    shop_boundary("equipment_upgrade_insufficient_funds");
                    return;
                case FieldEquipmentUpgradeResult::insufficient_essence:
                    field_shop_menu_.show_transaction_message(
                        ShopTransactionMessage::cyberware_insufficient_essence);
                    shop_boundary("equipment_upgrade_insufficient_essence");
                    return;
                case FieldEquipmentUpgradeResult::invalid_selection:
                    shop_boundary("equipment_upgrade_selection_unresolved");
                    field_shop_menu_.finish_action();
                    return;
                case FieldEquipmentUpgradeResult::applied:
                    break;
                }

                std::optional<MagicSpellTable> updated_spell_table;
                if (state.archetype_status < 2) {
                    if (spell_table == nullptr ||
                        table_index >=
                            field_magic_spell_table_overrides_.size()) {
                        shop_boundary(
                            "equipment_upgrade_spell_table_unavailable");
                        field_shop_menu_.finish_action();
                        return;
                    }
                    updated_spell_table = spell_table_after_equipment_upgrade(
                        *spell_table, state);
                    if (!updated_spell_table.has_value()) {
                        shop_boundary(
                            "equipment_upgrade_spell_table_write_unavailable");
                        field_shop_menu_.finish_action();
                        return;
                    }
                }
                auto& runner = active_runner_record();
                runner.cyberware_flags = state.upgrade_flags;
                runner.set_cyberware_essence_tens(state.meter_tens);
                runner.set_cyberware_essence_ones(state.meter_ones);
                runner.auxiliary[0U] = state.saved_meter_tens;
                if (updated_spell_table.has_value()) {
                    field_magic_spell_table_overrides_[table_index] =
                        *updated_spell_table;
                }
                field_funds_ = state.funds;
                refresh_field_hud_runner_slot();
                field_shop_menu_.finish_action();
                shop_boundary("equipment_upgrade_applied");
            };
        // ROM 0x56AC0 services audio and new high-byte input inside its menu
        // loop. Keep that interaction modal so field movement cannot write
        // stale map pixels underneath the full-screen service presentation.
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        if (field_shop_menu_.transaction_confirmation_open() ||
            field_shop_menu_.transaction_waiting_for_event()) {
            if (!field_funds_.has_value()) {
                shop_boundary("transaction_wallet_unavailable");
                field_shop_menu_.clear_pending_action();
                return;
            }
            auto purchase_state = make_purchase_state(
                active_runner_record(), *field_funds_);
            auto sale_state = make_sale_state(
                active_runner_record(), field_shop_menu_.selected_cursor(),
                *field_funds_);
            const auto cost_context = field_shop_cost_context(
                field_shop_menu_.menu_mode(), active_runner_record(),
                field_runner_feature_flags_.value_or(0U));
            const auto confirmation_was_open =
                field_shop_menu_.transaction_confirmation_open();
            const auto transaction_event =
                field_shop_menu_.advance_transaction(
                    field_controller_sample_state_.events, cost_context,
                    purchase_state, sale_state, item_base_cost_lookup(),
                    item_resale_base_lookup());
            if (transaction_event ==
                ShopTransactionEvent::confirmation_moved) {
                if (source_audio_event_callback_) {
                    source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
                }
                shop_boundary("confirmation_moved");
            } else if (confirmation_was_open &&
                       !field_shop_menu_.transaction_confirmation_open() &&
                       source_audio_event_callback_) {
                // 0x57B8C requests event 0x24 on either source exit button.
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
            }

            if (transaction_event == ShopTransactionEvent::purchase_applied) {
                apply_purchase_state(purchase_state, active_runner_record());
                field_funds_ = purchase_state.funds;
                refresh_field_hud_runner_slot();
                shop_boundary("purchase_applied");
            } else if (transaction_event ==
                       ShopTransactionEvent::sale_applied) {
                apply_sale_state(sale_state, active_runner_record());
                field_funds_ = sale_state.funds;
                refresh_field_hud_runner_slot();
                shop_boundary("sale_applied");
            } else if (transaction_event ==
                       ShopTransactionEvent::equipment_upgrade_confirmed) {
                service_wire_upgrade(true);
            } else if (transaction_event ==
                       ShopTransactionEvent::message_shown) {
                shop_boundary("transaction_message");
            } else if (transaction_event ==
                       ShopTransactionEvent::cancelled) {
                shop_boundary("transaction_cancelled");
            } else if (transaction_event ==
                       ShopTransactionEvent::message_dismissed) {
                shop_boundary("message_dismissed");
            } else if (transaction_event ==
                       ShopTransactionEvent::unresolved) {
                shop_boundary("transaction_unresolved");
            }
            return;
        }
        const auto script_state = game_scripts_.advance_shop_menu(
            field_controller_sample_state_.events);
        const auto applied = script_state.has_value()
            ? field_shop_menu_.apply_script_snapshot(*script_state)
            : std::optional<ShopMenuEvent>{};
        if (!applied.has_value()) {
            shop_boundary("lua_menu_state_unavailable");
            return;
        }
        const auto menu_event = *applied;
        switch (menu_event) {
        case ShopMenuEvent::selection_moved:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
            }
            shop_boundary("selection_moved");
            break;
        case ShopMenuEvent::panel_changed:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
            }
            shop_boundary("panel_changed");
            break;
        case ShopMenuEvent::purchase_selected:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
            }
            if (field_shop_menu_.pending_action().has_value() &&
                field_shop_menu_.pending_action()->offer.has_value() &&
                field_funds_.has_value()) {
                auto purchase_state = make_purchase_state(
                    active_runner_record(), *field_funds_);
                const auto cost_context = field_shop_cost_context(
                    field_shop_menu_.menu_mode(), active_runner_record(),
                    field_runner_feature_flags_.value_or(0U));
                const auto transaction_event =
                    field_shop_menu_.begin_purchase(
                        *field_shop_menu_.pending_action()->offer,
                        cost_context, purchase_state,
                        item_base_cost_lookup());
                if (transaction_event == ShopTransactionEvent::message_shown) {
                    shop_boundary("purchase_preflight_message");
                } else if (transaction_event ==
                           ShopTransactionEvent::unresolved) {
                    shop_boundary("purchase_unresolved");
                }
            } else {
                field_shop_menu_.clear_pending_action();
                shop_boundary("purchase_state_unavailable");
            }
            break;
        case ShopMenuEvent::sale_selected:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
            }
            if (field_funds_.has_value()) {
                auto sale_state = make_sale_state(
                    active_runner_record(), field_shop_menu_.selected_cursor(),
                    *field_funds_);
                const auto cost_context = field_shop_cost_context(
                    field_shop_menu_.menu_mode(), active_runner_record(),
                    field_runner_feature_flags_.value_or(0U));
                const auto transaction_event = field_shop_menu_.begin_sale(
                    cost_context, sale_state, item_resale_base_lookup());
                if (transaction_event == ShopTransactionEvent::message_shown) {
                    shop_boundary("sale_preflight_message");
                } else if (transaction_event ==
                           ShopTransactionEvent::action_ignored) {
                    shop_boundary("sale_empty_slot");
                } else if (transaction_event ==
                           ShopTransactionEvent::unresolved) {
                    shop_boundary("sale_unresolved");
                }
            } else {
                field_shop_menu_.clear_pending_action();
                shop_boundary("sale_wallet_unavailable");
            }
            break;
        case ShopMenuEvent::progression_selected:
            if (!field_shop_menu_.pending_action().has_value() ||
                !field_shop_menu_.pending_action()->progression_offer.has_value() ||
                !field_funds_.has_value()) {
                shop_boundary("progression_state_unavailable");
                field_shop_menu_.finish_action();
                break;
            }
            {
                const auto table_index = active_runner_record().magic_table_index;
                const auto* const source_table =
                    field_magic_spell_table(table_index);
                if (source_table == nullptr ||
                    table_index >= field_magic_spell_table_overrides_.size()) {
                    shop_boundary("progression_spell_table_unavailable");
                    field_shop_menu_.finish_action();
                    break;
                }
                auto progression = make_field_shop_progression_input(
                    field_shop_menu_, active_runner_record(), *field_funds_,
                    *source_table,
                    field_runner_feature_flags_.value_or(0U));
                if (!progression.has_value()) {
                    shop_boundary("progression_source_state_unavailable");
                    field_shop_menu_.finish_action();
                    break;
                }
                const auto quote = prepare_field_action_progression_quote(
                    progression->tables, progression->state);
                if (quote.result ==
                    FieldActionProgressionQuoteResult::quoted) {
                    const auto purchase_result =
                        purchase_field_action_progression(
                            progression->tables, progression->state);
                    if (purchase_result ==
                        FieldActionProgressionPurchaseResult::applied) {
                        const auto updated_table =
                            spell_table_from_field_progression(
                                progression->state);
                        if (!updated_table.has_value()) {
                            shop_boundary("progression_table_write_unavailable");
                        } else {
                            field_magic_spell_table_overrides_[table_index] =
                                *updated_table;
                            field_funds_ = progression->state.funds;
                            refresh_field_hud_runner_slot();
                            shop_boundary("progression_applied");
                        }
                        field_shop_menu_.finish_action();
                    } else if (purchase_result ==
                               FieldActionProgressionPurchaseResult::
                                   insufficient_funds) {
                        field_shop_menu_.show_transaction_message(
                            ShopTransactionMessage::insufficient_funds);
                        shop_boundary("progression_insufficient_funds");
                    } else if (purchase_result ==
                               FieldActionProgressionPurchaseResult::
                                   runner_unavailable) {
                        field_shop_menu_.show_transaction_message(
                            ShopTransactionMessage::training_unavailable);
                        shop_boundary("progression_runner_unavailable");
                    } else {
                        shop_boundary("progression_purchase_boundary");
                        field_shop_menu_.finish_action();
                    }
                } else if (quote.result ==
                           FieldActionProgressionQuoteResult::
                               runner_unavailable) {
                    field_shop_menu_.show_transaction_message(
                        ShopTransactionMessage::training_unavailable);
                    shop_boundary("progression_runner_unavailable");
                } else if (quote.result ==
                           FieldActionProgressionQuoteResult::
                               progression_limit) {
                    shop_boundary("progression_limit");
                    field_shop_menu_.finish_action();
                } else if (quote.result ==
                           FieldActionProgressionQuoteResult::
                               offer_unavailable) {
                    shop_boundary("progression_offer_unavailable");
                    field_shop_menu_.finish_action();
                } else {
                    shop_boundary("progression_quote_unavailable");
                    field_shop_menu_.finish_action();
                }
            }
            break;
        case ShopMenuEvent::equipment_upgrade_selected:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
            }
            service_wire_upgrade(false);
            break;
        case ShopMenuEvent::offer_unavailable:
            shop_boundary("offer_unavailable");
            break;
        case ShopMenuEvent::source_action_boundary:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
            }
            shop_boundary("action_boundary");
            {
                const auto reset_state = game_scripts_.reset_shop_menu();
                if (!reset_state.has_value() ||
                    !field_shop_menu_.apply_script_snapshot(*reset_state)
                         .has_value()) {
                    shop_boundary("lua_menu_reset_unavailable");
                    return;
                }
            }
            field_shop_menu_.finish_action();
            break;
        case ShopMenuEvent::active_runner_change_requested:
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
            }
            handle_field_mode_action_event();
            refresh_field_hud_runner_slot();
            shop_boundary("active_runner_changed");
            break;
        case ShopMenuEvent::returned_to_field:
            field_shop_route_pending_service_id_.clear();
            state_.display_service_word = 0U;
            field_runtime_state_.display_service_word = 0U;
            field_shop_active_definition_.reset();
            (void)apply_field_service_close_return();
            shop_boundary("returned_to_field");
            break;
        case ShopMenuEvent::no_change:
            break;
        }
        return;
    }
    if (command_overview_.open()) {
        if (command_overview_start_transition_frames_remaining_ != 0U) {
            command_overview_start_transition_black_frame_ = true;
            --command_overview_start_transition_frames_remaining_;
            source_boundary("field.command_overview.start_black_frame");
            return;
        }
        command_overview_start_transition_black_frame_ = false;
        // This is the non-blocking host form of the source command loop's
        // wait/latch/sample sequence. Field movement and world services stay
        // suspended until Start returns from the overview.
        source_boundary("field.command_overview.wait_for_vblank");
        field_controller_latch_state_.events =
            field_controller_sample_state_.events;
        field_controller_latch_state_.extra_events = 0U;
        (void)::shadowrun::ghidra_conversion::latch_controller_events(
            field_controller_latch_state_);
        ::shadowrun::ghidra_conversion::sample_controller_input(
            field_controller_sample_state_, field_controller_port_byte_);
        if (command_overview_attributes_skills_.is_open() &&
            !menu_route_active_) {
            const auto menu_event =
                command_overview_attributes_skills_.advance(
                    field_controller_sample_state_.events,
                    active_runner_record(),
                    [this] {
                        handle_field_mode_action_event();
                        refresh_field_hud_runner_slot();
                        source_boundary(
                            "field.command_overview.attributes_skills.runner_selected");
                    });
            switch (menu_event) {
            case AttributesSkillsRuntimeEvent::no_change:
                break;
            case AttributesSkillsRuntimeEvent::selection_moved:
                source_boundary(
                    "field.command_overview.attributes_skills.selection_moved");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_prompt_opened:
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_prompt_opened");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_prompt_moved:
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_prompt_moved");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_applied:
                refresh_command_overview_combat_bars();
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_applied");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_declined:
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_declined");
                break;
            case AttributesSkillsRuntimeEvent::upgrade_blocked:
                source_boundary(
                    "field.command_overview.attributes_skills.upgrade_blocked");
                break;
            case AttributesSkillsRuntimeEvent::insufficient_karma:
                source_boundary(
                    "field.command_overview.attributes_skills.insufficient_karma");
                break;
            case AttributesSkillsRuntimeEvent::
                active_runner_selection_requested:
                command_overview_attributes_skills_.refresh(
                    active_runner_record());
                refresh_command_overview_combat_bars();
                break;
            case AttributesSkillsRuntimeEvent::returned_to_parent:
                command_overview_attributes_skills_.close_to_parent();
                // ROM 0x10E74 routes the Attributes return back through the
                // 0x10D30 command initializer. Rebuild every shared panel and
                // row here so partial page pixels cannot survive the return.
                if (!menu_route_active_) {
                    command_overview_.close();
                    open_command_overview();
                }
                source_boundary(
                    "field.command_overview.attributes_skills.returned_to_command");
                break;
            }
            return;
        }
        if (command_overview_clips_.is_open() && !menu_route_active_) {
            advance_command_overview_clips(
                field_controller_sample_state_.events);
            return;
        }
        if (!menu_route_active_ && magic_spell_interface_open()) {
            advance_command_overview_magic(
                field_controller_sample_state_.events);
            return;
        }
        if (!command_overview_inventory_lua_route_active_ &&
            command_overview_automatic_use_state_ !=
            CommandOverviewAutomaticUseState::inactive) {
            advance_command_overview_automatic_use(
                field_controller_sample_state_.events);
            return;
        }
        if (!command_overview_inventory_lua_route_active_ &&
            command_overview_first_aid_state_ !=
            CommandOverviewFirstAidState::inactive) {
            advance_command_overview_first_aid(
                field_controller_sample_state_.events);
            return;
        }
        if (!command_overview_inventory_lua_route_active_ &&
            command_overview_inventory_trade_.open) {
            advance_command_overview_inventory_trade(
                field_controller_sample_state_.events);
            return;
        }
        if (command_overview_inventory_selector_suppressed_ &&
            (field_controller_sample_state_.events & 0x0F00U) != 0U) {
            command_overview_inventory_selector_suppressed_ = false;
        }
        if (command_overview_cyberware_active_ && !menu_route_active_) {
            // ROM 0x11950 waits for one controller event after drawing the
            // +0x90 bit rows, refreshes the shared menu row, and returns.
            // Keep the wait non-blocking in the Qt event loop.
            if ((field_controller_sample_state_.events & 0xFF00U) != 0U) {
                command_overview_cyberware_active_ = false;
                source_boundary("field.command_overview.cyberware.returned");
            }
            return;
        }
        if (!menu_route_active_ && command_overview_magic_state_ ==
            CommandOverviewMagicState::magical_defense_wait) {
            // ROM 0x0D94 services VBlank/input until any newly pressed
            // high-byte event arrives, takes one final sample, then 0x11928
            // restores the selected command row.
            if ((field_controller_sample_state_.events & 0xFF00U) != 0U) {
                command_overview_magic_state_ =
                    CommandOverviewMagicState::inactive;
                source_boundary(
                    "field.command_overview.magic.magical_defense.returned");
            }
            return;
        }
        if (!menu_route_active_ && command_overview_.take_action_dialog()) {
            // Up on command pages 2..6 arms FFF115 and enters the 0x119CE
            // posture focus. Keep the command page selected underneath it.
            posture_dialog_active_ = true;
            source_boundary("field.posture.dialog_opened");
        }
        if (!menu_route_active_ && posture_dialog_active_) {
            const auto events = field_controller_sample_state_.events;
            bool check_increment = false;
            bool check_decrement = false;
            bool close_by_down = false;

            // ROM 0x119CE gives a Down edge special treatment: while Left or
            // Right is held it can still complete that horizontal adjustment;
            // otherwise the armed FFF115 gate returns focus to the command
            // row. With no Down edge, Right raises and Left lowers posture.
            if ((events & 0x0200U) != 0U) {
                if ((events & 0x0008U) != 0U) {
                    check_increment = true;
                } else if ((events & 0x0004U) != 0U) {
                    check_decrement = true;
                } else {
                    close_by_down = true;
                }
            } else {
                check_increment = true;
            }

            int posture_delta = 0;
            if (!close_by_down && check_increment) {
                if ((events & 0x0800U) != 0U) {
                    posture_delta = 1;
                } else {
                    check_decrement = true;
                }
            }
            if (!close_by_down && posture_delta == 0 && check_decrement &&
                (events & 0x0400U) != 0U) {
                posture_delta = -1;
            }

            if (posture_delta != 0) {
                if (source_audio_event_callback_) {
                    source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
                }
                (void)adjust_runner_posture(active_runner_record(),
                                            posture_delta);
                refresh_command_overview_combat_bars();
                source_boundary(posture_delta > 0
                                    ? "field.posture.dialog.raised"
                                    : "field.posture.dialog.lowered");
                return;
            }

            if (close_by_down || (events & 0x8000U) != 0U) {
                // The blocking source loop writes its final posture strip
                // through 0x11AE4 with delta zero, then redraws both combat
                // bars before restoring command navigation.
                (void)adjust_runner_posture(active_runner_record(), 0);
                posture_dialog_active_ = false;
                refresh_command_overview_combat_bars();
                source_boundary(close_by_down
                                    ? "field.posture.dialog.deselected"
                                    : "field.posture.dialog.closed");
            }
            return;
        }
        const auto secretary_screen = static_cast<srx::MenuScreen>(
            menu_route_screen_);
        const bool legacy_secretary_child_active =
            (secretary_screen == srx::MenuScreen::CYBERDECK &&
             pocket_secretary_.cyberdeck_unavailable_wait);
        const bool dismiss_runner_was_open = dismiss_runner_.open;
        if (pocket_secretary_.open &&
            (!menu_route_active_ || legacy_secretary_child_active)) {
            // The Lua route advances E660 for the active screen. Retain the
            // C++ loop only for the historical, route-less field adapter.
            if (!menu_route_active_) {
                advance_pocket_secretary_dismiss_runner_effect();
            }
            if (dismiss_runner_.open) {
                const auto event = apply_dismiss_runner_input(
                    dismiss_runner_, field_controller_sample_state_.events);
                switch (event) {
                case DismissRunnerMenuEvent::selection_moved:
                    if (source_audio_event_callback_) {
                        source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
                    }
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.selection_moved");
                    break;
                case DismissRunnerMenuEvent::abort_selected:
                    if (source_audio_event_callback_) {
                        source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
                    }
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.abort_selected");
                    break;
                case DismissRunnerMenuEvent::runner_selected:
                    if (source_audio_event_callback_) {
                        source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
                    }
                    if (dismiss_runner_.selection > 0U &&
                        dismiss_runner_.selection <=
                            dismiss_runner_.candidate_count) {
                        const auto slot = dismiss_runner_.candidate_slots[
                            dismiss_runner_.selection - 1U];
                        finish_pocket_secretary_dismiss_runner(slot);
                    }
                    break;
                case DismissRunnerMenuEvent::returned:
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.returned");
                    break;
                case DismissRunnerMenuEvent::waiting:
                    break;
                }
                // C448 returns through the parent's inner-panel redraw; the
                // lower-bezel E660 writes are outside that cleared region.
                if (!menu_route_active_) {
                    return;
                }
            }
            if (dismiss_runner_was_open) {
                advance_field_menu_route();
                return;
            }
            const auto event = apply_pocket_secretary_input(
                pocket_secretary_, field_controller_sample_state_.events,
                {[this] {
                     source_boundary(
                         "field.pocket_secretary.transfer_menu_assets");
                 },
                 [this] {
                     source_boundary("field.pocket_secretary.render_menu_grid");
                 },
                 [this] {
                     source_boundary("field.pocket_secretary.render_menu_text");
                 },
                 [this] {
                     source_boundary(
                         "field.pocket_secretary.render_menu_numbers");
                 },
                 [this](const std::uint8_t selection) {
                     source_boundary("field.pocket_secretary.handler");
                     if (selection >=
                         pocket_secretary_menu_content_.choices.size()) {
                         source_boundary(
                             "field.pocket_secretary.choice_id_unresolved");
                         return true;
                     }
                     const auto& choice_id =
                         pocket_secretary_menu_content_.choices[selection].id;
                     if (choice_id == "notebook") {
                         const auto event_gates =
                             notebook_event_gates();
                         if (!event_gates.has_value()) {
                             source_boundary(
                                 "field.pocket_secretary.notebook.event_flags_unresolved");
                         } else {
                             begin_pocket_secretary_notebook(
                                 pocket_secretary_,
                                 notebook_current_run_available_,
                                 *event_gates);
                             source_boundary(
                                 "field.pocket_secretary.notebook.opened");
                         }
                     } else if (choice_id == "cyberdeck") {
                         if (cyberdeck_available_.has_value() &&
                             !*cyberdeck_available_) {
                             begin_pocket_secretary_cyberdeck_unavailable(
                                 pocket_secretary_);
                             source_boundary(
                                 "field.pocket_secretary.cyberdeck_unavailable");
                         } else if (!cyberdeck_available_.has_value()) {
                             source_boundary(
                                 "field.pocket_secretary.cyberdeck_flag_unresolved");
                         } else {
                             source_boundary(
                                 "field.pocket_secretary.matrix_path_pending");
                         }
                     } else if (choice_id == "group_items") {
                         if (!group_item_flags_.has_value()) {
                             source_boundary(
                                 "field.pocket_secretary.group_items_flags_unresolved");
                         } else {
                             begin_pocket_secretary_group_items(
                                 pocket_secretary_, *group_item_flags_);
                             source_boundary(*group_item_flags_ == 0U
                                                 ? "field.pocket_secretary.group_items_empty"
                                                 : "field.pocket_secretary.group_items_populated");
                         }
                     } else if (choice_id == "dismiss_runner") {
                         begin_pocket_secretary_dismiss_runner();
                     } else if (choice_id == "save_load") {
                         source_boundary(
                             "field.pocket_secretary.save_load_pending");
                     }
                     return true;
                 },
                 [this] {
                     if (source_audio_event_callback_) {
                         source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
                     }
                     handle_field_mode_action_event();
                     source_boundary("field.pocket_secretary.mode_action");
                 }});
            switch (event) {
            case PocketSecretaryMenuEvent::notebook_selection_moved:
                source_boundary(
                    "field.pocket_secretary.notebook.selection_moved");
                break;
            case PocketSecretaryMenuEvent::notebook_mode_action:
                source_boundary(
                    "field.pocket_secretary.notebook.mode_action");
                break;
            case PocketSecretaryMenuEvent::notebook_current_run_no_run:
                source_boundary(
                    "field.pocket_secretary.notebook.current_run.unavailable");
                break;
            case PocketSecretaryMenuEvent::notebook_response_dismissed:
                source_boundary(
                    "field.pocket_secretary.notebook.current_run.dismissed");
                break;
            case PocketSecretaryMenuEvent::notebook_returned:
                source_boundary("field.pocket_secretary.notebook.returned");
                break;
            case PocketSecretaryMenuEvent::notebook_action_boundary:
                if (pocket_secretary_.notebook.selection >=
                    pocket_secretary_.notebook.option_count) {
                    source_boundary(
                        "field.pocket_secretary.notebook.choice_id_unresolved");
                    break;
                }
                {
                    const auto option_index = static_cast<std::size_t>(
                        pocket_secretary_.notebook.visible_options[
                            pocket_secretary_.notebook.selection]);
                    if (option_index >=
                        pocket_secretary_menu_content_.notebook_choice_ids
                            .size()) {
                        source_boundary(
                            "field.pocket_secretary.notebook.choice_id_unresolved");
                        break;
                    }
                    const auto& choice_id =
                        pocket_secretary_menu_content_.notebook_choice_ids[
                            option_index];
                    if (choice_id == "tips_and_clues") {
                        if (!notebook_tip_flags_resolved_) {
                            source_boundary(
                                "field.pocket_secretary.notebook.tips.flags_unresolved");
                            break;
                        }
                        begin_pocket_secretary_notebook_tips(
                            pocket_secretary_, notebook_tip_flags_);
                        source_boundary(
                            "field.pocket_secretary.notebook.tips.opened");
                    } else if (choice_id == "mr_johnsons") {
                        if (!notebook_referral_flags_.has_value()) {
                            source_boundary(
                                "field.pocket_secretary.notebook.mr_johnsons.flags_unresolved");
                            break;
                        }
                        begin_pocket_secretary_notebook_mr_johnsons(
                            pocket_secretary_, *notebook_referral_flags_);
                        source_boundary(
                            "field.pocket_secretary.notebook.mr_johnsons.opened");
                    } else if (choice_id == "current_run" ||
                               choice_id == "contacts" ||
                               choice_id == "shadowrunners") {
                        source_boundary(
                            "field.pocket_secretary.notebook.child_pending");
                    } else {
                        source_boundary(
                            "field.pocket_secretary.notebook.choice_id_unresolved");
                    }
                }
                break;
            case PocketSecretaryMenuEvent::notebook_tips_selection_moved:
                source_boundary(
                    "field.pocket_secretary.notebook.tips.selection_moved");
                break;
            case PocketSecretaryMenuEvent::notebook_tips_returned:
                source_boundary(
                    "field.pocket_secretary.notebook.tips.returned");
                break;
            case PocketSecretaryMenuEvent::notebook_tips_mode_action:
                source_boundary(
                    "field.pocket_secretary.notebook.tips.mode_action");
                break;
            case PocketSecretaryMenuEvent::
                notebook_mr_johnsons_selection_moved:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.selection_moved");
                break;
            case PocketSecretaryMenuEvent::notebook_mr_johnsons_detail_opened:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.detail_opened");
                break;
            case PocketSecretaryMenuEvent::
                notebook_mr_johnsons_detail_dismissed:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.detail_dismissed");
                break;
            case PocketSecretaryMenuEvent::notebook_mr_johnsons_returned:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.returned");
                break;
            case PocketSecretaryMenuEvent::notebook_mr_johnsons_mode_action:
                source_boundary(
                    "field.pocket_secretary.notebook.mr_johnsons.mode_action");
                break;
            default:
                break;
            }
            if (event == PocketSecretaryMenuEvent::exited) {
                pocket_secretary_.open = false;
                source_boundary("field.pocket_secretary.exited");
            }
            if (menu_route_active_) {
                advance_field_menu_route();
            }
            return;
        }
        if (command_overview_inventory_action_active_ &&
            !command_overview_inventory_lua_route_active_) {
            advance_command_overview_inventory_action(
                field_controller_sample_state_.events);
            refresh_command_overview_combat_bars();
            refresh_command_overview_item_detail();
            return;
        }
        const auto was_open = command_overview_.open();
        if (menu_route_active_) {
            advance_field_menu_route();
        } else {
            command_overview_.advance(
                field_controller_sample_state_.events);
        }
        if (was_open && !command_overview_.open()) {
            // The source command loop returns through the field coordinator's
            // display-service tail before the next field pass. The converted
            // coordinator publishes FFDFE8=FFFF while dispatching Start, so
            // the host must publish the corresponding completion here; if it
            // remains set, the coordinator's 0x0AEA guard permanently blocks
            // all later Start edges and the menu can only open once.
            state_.display_service_word = 0U;
            field_runtime_state_.display_service_word = 0U;
            command_overview_start_transition_frames_remaining_ = 0U;
            command_overview_start_transition_black_frame_ = false;
            source_boundary("field.command_overview.returned_to_field");
            command_overview_inventory_action_ = InventoryActionMenuState{};
            command_overview_selected_inventory_item_.reset();
            command_overview_discard_confirmation_ = MenuConfirmationState{};
            command_overview_action_message_ =
                CommandOverviewActionMessage::none;
            command_overview_inventory_action_active_ = false;
            command_overview_discard_confirmation_active_ = false;
            command_overview_first_aid_state_ =
                CommandOverviewFirstAidState::inactive;
            command_overview_first_aid_targets_ = FirstAidTargetList{};
            command_overview_first_aid_cursor_ = 0U;
            command_overview_first_aid_message_ = FirstAidMessage::none;
            command_overview_automatic_use_state_ =
                CommandOverviewAutomaticUseState::inactive;
            command_overview_inventory_selector_suppressed_ = false;
            command_overview_clips_.close();
            command_overview_cyberware_active_ = false;
            command_overview_cyberware_ = CyberwareMenuModel{};
            command_overview_magic_state_ =
                CommandOverviewMagicState::inactive;
            command_overview_magic_loop_ = MagicMenuLoopState{};
            command_overview_magic_selected_spell_.reset();
            command_overview_magic_force_ = MagicForceState{};
            pocket_secretary_.open = false;
            dismiss_runner_ = DismissRunnerMenuState{};
            posture_dialog_active_ = false;
            if (matrix_field_return_transition_pending_) {
                matrix_field_return_transition_pending_ = false;
                begin_field_modal_return_transition();
            } else if (!field_taxi_transition_active_ &&
                       !field_dialogue_.open()) {
                advance_field_scene_transition_after_modal();
            }
        } else if (!menu_route_active_ &&
                   command_overview_.take_action_dispatch_pending()) {
            // ROM 0x10E4E publishes audio command 0x24 before dispatching the
            // A/C action selected on the command overview. FBA6 itself has
            // no additional sound call in its Magical Defense fallback.
            if (source_audio_event_callback_) {
                source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::SELECT));
            }
            if (command_overview_.view().source_state.primary_page < 2U) {
                begin_command_overview_inventory_action();
            } else if (command_overview_.view().source_state.primary_page ==
                       2U) {
                begin_command_overview_clips();
            } else if (command_overview_.view().source_state.primary_page ==
                       6U) {
                begin_pocket_secretary();
            } else if (command_overview_.view().source_state.primary_page ==
                       4U) {
                begin_command_overview_cyberware();
            } else if (command_overview_.view().source_state.primary_page ==
                       5U) {
                begin_command_overview_magic();
            } else if (command_overview_.view().source_state.primary_page ==
                       3U) {
                begin_command_overview_attributes_skills();
            } else {
                source_boundary(
                    "field.command_overview.non_inventory_action_pending");
            }
        }
        refresh_command_overview_combat_bars();
        refresh_command_overview_item_detail();
        return;
    }
    if (field_entity_.active) {
        handle_field_map_event_activations(
            field_map_event_system_.evaluate_actor_scope(
                FieldMapEventTriggerContext{
                    state_.field_scene_buffers.scene_reference,
                    field_entity_.world_position.x,
                    field_entity_.world_position.y,
                    field_map_event_state_flags_,
                    true}));
    }
    field_runtime_state_.field_activity_lock =
        state_.field_lock != 0U || field_opening_auto_walk_.active;
    field_runtime_state_.world_activity_countdown =
        world_frame_logic_state_.activity_countdown;
    field_runtime_state_.display_service_word = state_.display_service_word;
    field_runtime_state_.scene_id =
        state_.field_scene_buffers.scene_reference;
    field_runtime_state_.field_mode =
        state_.field_scene_buffers.field_resource_mode;
    field_runtime_state_.mode_event_code =
        field_movement_transition_state_.mode_event_code;
    field_runtime_state_.transition_context =
        field_movement_transition_state_.field_transition_context;
    field_runtime_state_.world_activity_flags = field_exit_world_flags_;
    field_runtime_state_.mode_transition_gate =
        state_.field_scene_buffers.scene_mode;
    world_frame_logic_state_.scene_state =
        field_runtime_state_.scene_runtime_state;

    const auto result = integration::advance_field_runtime_frame(
        field_runtime_state_, field_runtime_callbacks_);
    state_.display_dirty_flags = field_runtime_state_.display_dirty_flags;
    state_.display_service_word = field_runtime_state_.display_service_word;
    world_frame_logic_state_.activity_countdown =
        field_runtime_state_.world_activity_countdown;
    if (result == integration::FieldRuntimeFrameResult::callback_unresolved) {
        source_boundary("field.runtime.callback_unresolved");
    }
}

void FieldSceneRuntime::paint_field_scene_entities(
    QPainter& painter, const QRect& clip_rect) const {
    struct SceneSprite {
        std::int16_t depth = 0;
        std::uint16_t source_order = 0;
        int x = 0;
        int y = 0;
        const QImage* image = nullptr;
        QImage mirrored_image{};
    };
    std::vector<SceneSprite> sprites;
    const auto camera_x = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_x);
    const auto camera_y = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_y);
    const auto current_scene =
        state_.field_scene_buffers.scene_reference;

    if (field_entity_.active && field_entity_.visible) {
        const auto direction = static_cast<std::size_t>(
            field_entity_.direction %
            field_entity_idle_images_.size());
        const auto walking = field_entity_animation_id_ == "walk";
        const auto attack_pose = field_entity_animation_id_ == "attack";
        const auto strike_pose = field_entity_animation_id_ == "strike";
        const auto cast_pose = field_entity_animation_id_ == "cast";
        const auto down_pose = field_entity_animation_id_ == "runner_down";
        const auto strike_frame = static_cast<std::size_t>(
            field_entity_animation_.frame & 1U);
        const auto down_frame = static_cast<std::size_t>(
            field_entity_animation_.frame == 0U ? 0U : 1U);
        const auto movement_frames =
            field_entity_walk_images_[direction].size();
        const auto frame = movement_frames == 0U
            ? 0U
            : static_cast<std::size_t>(field_entity_animation_.frame) %
                  movement_frames;
        const auto& image = down_pose
            ? field_entity_down_images_[direction][down_frame]
            : strike_pose
            ? field_entity_strike_images_[direction][strike_frame]
            : cast_pose ? field_entity_cast_images_[direction]
            : attack_pose
            ? field_entity_attack_images_[direction]
            : walking ? field_entity_walk_images_[direction][frame]
                      : field_entity_idle_images_[direction];
        const auto origin_x = down_pose
            ? field_entity_down_origin_x_[direction][down_frame]
            : strike_pose
            ? field_entity_strike_origin_x_[direction][strike_frame]
            : cast_pose ? field_entity_cast_origin_x_[direction]
            : attack_pose
            ? field_entity_attack_origin_x_[direction]
            : walking ? field_entity_walk_origin_x_[direction][frame]
                      : field_entity_idle_origin_x_[direction];
        const auto origin_y = down_pose
            ? field_entity_down_origin_y_[direction][down_frame]
            : strike_pose
            ? field_entity_strike_origin_y_[direction][strike_frame]
            : cast_pose ? field_entity_cast_origin_y_[direction]
            : attack_pose
            ? field_entity_attack_origin_y_[direction]
            : walking ? field_entity_walk_origin_y_[direction][frame]
                      : field_entity_idle_origin_y_[direction];
        EntityProjectionState projection{};
        projection.world_x = field_entity_.world_position.x;
        projection.world_y = field_entity_.world_position.y;
        projection.sprite_offset_y = field_entity_.projection.sprite_offset_y;
        if (project_entity_screen_position_standard(
                projection, camera_x, camera_y).visible) {
            const auto depth = (field_entity_behavior_flags_ & 0x50U) != 0U
                                   ? std::int16_t{0}
                                   : static_cast<std::int16_t>(
                                         field_entity_.world_position.y);
            sprites.push_back(SceneSprite{
                depth,
                0U,
                static_cast<int>(projection.screen_x) -
                    kGenesisSpriteOrigin + origin_x,
                static_cast<int>(projection.screen_y) -
                    kGenesisSpriteOrigin + origin_y,
                &image});
            if (field_entity_.spell_effect_visible &&
                field_entity_.spell_effect_phase <
                    kFieldSpellEffectPhaseCapacity &&
                current_scene < field_spell_effect_images_by_scene_.size()) {
                const auto profile = std::find_if(
                    kFieldSpellEffectProfiles.cbegin(),
                    kFieldSpellEffectProfiles.cend(),
                    [this](const auto& candidate) {
                        return candidate.action_code ==
                            field_entity_.spell_effect_action_code;
                    });
                if (profile != kFieldSpellEffectProfiles.cend()) {
                    const auto profile_index = static_cast<std::size_t>(
                        std::distance(kFieldSpellEffectProfiles.cbegin(),
                                      profile));
                    const auto phase = field_entity_.spell_effect_phase;
                    const auto& effect =
                        field_spell_effect_images_by_scene_[current_scene]
                                                           [profile_index];
                    sprites.push_back(SceneSprite{
                        depth,
                        1U,
                        static_cast<int>(projection.screen_x) -
                            kGenesisSpriteOrigin + effect.origin_x[phase],
                        static_cast<int>(projection.screen_y) -
                            kGenesisSpriteOrigin + effect.origin_y[phase],
                        &effect.images[phase]});
                }
            }
        }
    }

    for (const auto& actor : field_default_actors_) {
        if (actor.scene_reference != current_scene) {
            continue;
        }
        const auto sprite = default_actor_sprite_view(actor);
        if (sprite.image == nullptr || sprite.image->isNull()) {
            continue;
        }

        EntityProjectionState projection{};
        projection.world_x = actor.world_position.x;
        projection.world_y = actor.world_position.y;
        if (!project_entity_screen_position_standard(
                projection, camera_x, camera_y)
                 .visible) {
            continue;
        }
        // ROM 0x5C7E appends the marker descriptors before 0x622C appends
        // the actor body. The Genesis SAT gives earlier linked sprites
        // overlap priority; this painter composites later sprites on top, so
        // add the body first and marker corners after it to preserve that
        // source priority without letting the actor cover its own selector.
        sprites.push_back(SceneSprite{
            static_cast<std::int16_t>(actor.world_position.y),
            static_cast<std::uint16_t>(actor.source_order + 1U),
            static_cast<int>(projection.screen_x) - kGenesisSpriteOrigin +
                sprite.origin_x,
            static_cast<int>(projection.screen_y) - kGenesisSpriteOrigin +
                sprite.origin_y,
            sprite.image});

        if (actor.spell_effect_visible &&
            actor.spell_effect_phase < kFieldSpellEffectPhaseCapacity &&
            current_scene < field_spell_effect_images_by_scene_.size()) {
            const auto profile = std::find_if(
                kFieldSpellEffectProfiles.cbegin(),
                kFieldSpellEffectProfiles.cend(),
                [&actor](const auto& candidate) {
                    return candidate.action_code ==
                        actor.spell_effect_action_code;
                });
            if (profile != kFieldSpellEffectProfiles.cend()) {
                const auto profile_index = static_cast<std::size_t>(
                    std::distance(kFieldSpellEffectProfiles.cbegin(),
                                  profile));
                const auto phase = actor.spell_effect_phase;
                const auto& effect =
                    field_spell_effect_images_by_scene_[current_scene]
                                                       [profile_index];
                sprites.push_back(SceneSprite{
                    static_cast<std::int16_t>(actor.world_position.y),
                    static_cast<std::uint16_t>(actor.source_order + 1U),
                    static_cast<int>(projection.screen_x) -
                        kGenesisSpriteOrigin + effect.origin_x[phase],
                    static_cast<int>(projection.screen_y) -
                        kGenesisSpriteOrigin + effect.origin_y[phase],
                    &effect.images[phase]});
            }
        }

        if (actor.target_motion_effect_visible &&
            current_scene < field_hound_close_attack_images_by_scene_.size() &&
            actor.target_motion_effect_direction < 8U &&
            actor.target_motion_effect_phase >= 1U &&
            actor.target_motion_effect_phase <= 3U) {
            const auto& effect =
                field_hound_close_attack_images_by_scene_[current_scene]
                    [actor.target_motion_effect_direction];
            const auto phase = actor.target_motion_effect_phase;
            sprites.push_back(SceneSprite{
                static_cast<std::int16_t>(actor.world_position.y),
                static_cast<std::uint16_t>(actor.source_order + 1U),
                static_cast<int>(projection.screen_x) - kGenesisSpriteOrigin +
                    effect.origin_x[phase],
                static_cast<int>(projection.screen_y) - kGenesisSpriteOrigin +
                    effect.origin_y[phase],
                &effect.images[phase]});
        }

        if (actor.target_marker_visible &&
            current_scene < field_target_marker_images_by_scene_.size() &&
            actor.target_marker_style < 4U) {
            struct TargetMarkerCorner {
                int x;
                int y;
                int horizontal_spread;
                int vertical_spread;
                bool flip_x;
                bool flip_y;
            };
            // The four 0x5C7E descriptor lists share these positions and
            // flips; only their tile/palette line changes with action code.
            constexpr std::array<TargetMarkerCorner, 4> kCorners{{
                {-14, -16, -1, -1, false, true},
                {8, -16, 1, -1, true, true},
                {-14, 12, -1, 1, false, false},
                {8, 12, 1, 1, true, false},
            }};
            const auto& marker_image =
                field_target_marker_images_by_scene_[current_scene]
                                                    [actor.target_marker_style];
            const auto marker_anchor_x =
                static_cast<int>(projection.screen_x) - kGenesisSpriteOrigin;
            const auto marker_anchor_y =
                static_cast<int>(projection.screen_y) - kGenesisSpriteOrigin -
                (actor.entity_kind == 0x0FU ? 0x20 : 0x0C);
            const auto radius = actor.target_marker_animating
                                    ? static_cast<int>(actor.target_marker_radius)
                                    : 0;
            for (const auto& corner : kCorners) {
                SceneSprite marker{};
                marker.depth = static_cast<std::int16_t>(
                    actor.world_position.y);
                marker.source_order = static_cast<std::uint16_t>(
                    actor.source_order + 1U);
                marker.x = marker_anchor_x + corner.x +
                           corner.horizontal_spread * radius;
                marker.y = marker_anchor_y + corner.y +
                           corner.vertical_spread * radius;
                marker.image = &marker_image;
                auto flips = Qt::Orientations{};
                if (corner.flip_x) {
                    flips |= Qt::Horizontal;
                }
                if (corner.flip_y) {
                    flips |= Qt::Vertical;
                }
                marker.mirrored_image = flipped_image(marker_image, flips);
                sprites.push_back(std::move(marker));
            }
        }
    }

    const auto append_spawn_record =
        [this, camera_x, camera_y, &sprites](
            const SpawnEntityRecordState& record,
            const std::uint16_t frame,
            const bool active,
            const std::optional<std::uint16_t>& source_slot) {
            if (!active || (record.flags & 0x01U) == 0U) {
                return;
            }
            const auto selector = static_cast<std::uint16_t>(
                record.animation_resource & 0x7FFFU);
            if ((selector & 0x0003U) != 0U) {
                return;
            }
            const auto animation_index =
                static_cast<std::size_t>(selector / 4U);
            if (animation_index >= field_weapon_projectile_images_.size()) {
                return;
            }
            const auto frame_index = static_cast<std::size_t>(frame);
            const auto& images =
                field_weapon_projectile_images_[animation_index];
            if (frame_index >= images.size()) {
                return;
            }

            EntityProjectionState projection{};
            projection.world_x = record.position_x;
            projection.world_y = record.position_y;
            if (!project_entity_screen_position_standard(
                     projection, camera_x, camera_y)
                     .visible) {
                return;
            }
            const auto depth = (record.flags & 0x50U) != 0U
                                   ? std::int16_t{0}
                                   : static_cast<std::int16_t>(
                                         record.position_y);
            sprites.push_back(SceneSprite{
                depth,
                source_slot.value_or(0xFFFFU),
                static_cast<int>(projection.screen_x) -
                    kGenesisSpriteOrigin +
                    field_weapon_projectile_origin_x_[animation_index]
                                                       [frame_index],
                static_cast<int>(projection.screen_y) -
                    kGenesisSpriteOrigin +
                    field_weapon_projectile_origin_y_[animation_index]
                                                       [frame_index],
                &images[frame_index]});
        };

    for (const auto& spawn : field_weapon_projectiles_) {
        append_spawn_record(spawn.primary_record,
                            spawn.primary_animation.frame,
                            spawn.primary_active, spawn.primary_slot);
        append_spawn_record(spawn.source_record, spawn.animation.frame,
                            spawn.secondary_active, spawn.secondary_slot);
    }

    // ROM 0x5B1E stably sorts by descending effective depth, then 0x4658
    // emits that order into the SAT. The Genesis VDP gives lower SAT indices
    // priority, while QPainter gives later draws priority, so paint backwards.
    // Flags 0x10/0x40 force depth to zero.
    std::stable_sort(sprites.begin(), sprites.end(),
                     [](const SceneSprite& left,
                        const SceneSprite& right) {
                         if (left.depth != right.depth) {
                             return left.depth > right.depth;
                         }
                         return left.source_order < right.source_order;
                     });
    if (!clip_rect.isEmpty()) {
        painter.save();
        painter.setClipRect(clip_rect, Qt::IntersectClip);
    }
    for (auto sprite = sprites.rbegin(); sprite != sprites.rend(); ++sprite) {
        if (sprite->image != nullptr) {
            painter.drawImage(
                sprite->x, sprite->y,
                sprite->mirrored_image.isNull() ? *sprite->image
                                                : sprite->mirrored_image);
        }
    }
    if (!clip_rect.isEmpty()) {
        painter.restore();
    }
}

void FieldSceneRuntime::paint_field_hud_overlays(QPainter& painter) const {
    const auto image_for = [this](const FieldHudOverlay& overlay)
        -> const QImage* {
        switch (overlay.kind) {
        case FieldHudOverlayKind::portrait:
            return overlay.asset_index < field_hud_party_faces_.size() &&
                    overlay.palette_variant <
                        field_hud_party_faces_[overlay.asset_index].size()
                ? &field_hud_party_faces_[overlay.asset_index]
                                 [overlay.palette_variant]
                : nullptr;
        case FieldHudOverlayKind::physical_bar:
            return overlay.asset_index < field_hud_delay_meter_counters_.size()
                ? &field_hud_delay_meter_counters_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::mental_bar:
            return overlay.asset_index <
                    field_hud_behavior_meter_counters_.size()
                ? &field_hud_behavior_meter_counters_[overlay.asset_index]
                : nullptr;
        case FieldHudOverlayKind::item:
            return overlay.asset_index < field_hud_party_items_.size() &&
                    overlay.palette_variant <
                        field_hud_party_items_[overlay.asset_index].size()
                ? &field_hud_party_items_[overlay.asset_index]
                                  [overlay.palette_variant]
                : nullptr;
        case FieldHudOverlayKind::unarmed_item:
            return overlay.asset_index <
                       field_hud_party_unarmed_items_.size() &&
                    overlay.palette_variant <
                        field_hud_party_unarmed_items_[overlay.asset_index]
                            .size()
                ? &field_hud_party_unarmed_items_[overlay.asset_index]
                                           [overlay.palette_variant]
                : nullptr;
        case FieldHudOverlayKind::magic_item:
            return overlay.asset_index < field_hud_magic_items_.size() &&
                    overlay.palette_variant <
                        field_hud_magic_items_[overlay.asset_index].size()
                ? &field_hud_magic_items_[overlay.asset_index]
                                 [overlay.palette_variant]
                : nullptr;
        case FieldHudOverlayKind::ammo:
            return overlay.asset_index < field_hud_ammo_counters_.size()
                ? &field_hud_ammo_counters_[overlay.asset_index]
                : nullptr;
        }
        return nullptr;
    };
    auto live_slots = field_hud_slots_;
    for (std::size_t slot = 0; slot < live_slots.size(); ++slot) {
        live_slots[slot] = field_hud_slot_state(slot);
    }
    for (const auto& overlay : compose_field_hud_overlays(live_slots)) {
        const auto* image = image_for(overlay);
        if (image == nullptr || image->isNull()) {
            continue;
        }
        painter.drawImage(overlay.x, overlay.y, *image);
    }
}

std::optional<std::uint8_t> FieldSceneRuntime::selected_shop_item_code() const {
    if (!field_shop_menu_.offers_panel_active()) {
        return std::nullopt;
    }
    std::uint8_t item_code = 0U;
    if (field_shop_menu_.selected_group() == ShopActionGroup::sell) {
        const auto cursor = field_shop_menu_.selected_cursor();
        const auto& item_codes = active_runner_record().inventory_item_codes;
        if (cursor >= item_codes.size()) {
            return std::nullopt;
        }
        item_code = item_codes[cursor];
    } else {
        const auto offer = field_shop_menu_.offer(
            field_shop_menu_.selected_group(),
            field_shop_menu_.selected_cursor());
        if (!offer.has_value() ||
            offer->type == static_cast<std::uint8_t>('E')) {
            return std::nullopt;
        }
        item_code = static_cast<std::uint8_t>(offer->type + 1U);
    }
    if (item_code == 0U || item_code > field_menu_text_.items.size()) {
        return std::nullopt;
    }
    return item_code;
}

std::optional<std::uint8_t>
FieldSceneRuntime::selected_shop_weapon_capacity() const {
    const auto item_code = selected_shop_item_code();
    if (!item_code.has_value()) {
        return std::nullopt;
    }
    return weapon_magazine_capacity_for_item(*item_code);
}

std::optional<std::uint8_t>
FieldSceneRuntime::weapon_magazine_capacity_for_item(
    const std::uint8_t item_code) const {
    const auto* weapon = weapon_template_for_item_code(item_code);
    return weapon == nullptr
               ? std::nullopt
               : std::optional<std::uint8_t>{weapon->magazine_capacity};
}

const scripting::ScriptWeaponTemplate*
FieldSceneRuntime::weapon_template_for_item_code(
    const std::uint8_t item_code) const {
    if (item_code == 0U || item_code > field_menu_text_.items.size()) {
        return nullptr;
    }
    const auto& item = field_menu_text_.items[
        static_cast<std::size_t>(item_code - 1U)].definition;
    return item.code == item_code && item.weapon.has_value()
               ? &*item.weapon
               : nullptr;
}

std::optional<std::uint8_t> FieldSceneRuntime::selected_shop_item_uses() const {
    const auto item_code = selected_shop_item_code();
    if (!item_code.has_value()) {
        return std::nullopt;
    }
    const auto& item = field_menu_text_.items[
        static_cast<std::size_t>(*item_code - 1U)].definition;
    if (item.code != *item_code || !item.detail.has_value() ||
        item.detail->kind != "uses") {
        return std::nullopt;
    }
    return item.detail->value;
}

FieldActionBaseCostLookup FieldSceneRuntime::item_base_cost_lookup() const {
    return [this](const std::uint8_t catalog_index) {
        if (catalog_index >= field_menu_text_.items.size()) {
            return 0U;
        }
        const auto& item = field_menu_text_.items[catalog_index].definition;
        return item.code == static_cast<std::uint8_t>(catalog_index + 1U)
                   ? static_cast<std::uint32_t>(item.pricing.purchase_base)
                   : 0U;
    };
}

FieldActionBaseCostLookup FieldSceneRuntime::item_resale_base_lookup() const {
    return [this](const std::uint8_t catalog_index) {
        if (catalog_index >= field_menu_text_.items.size()) {
            return 0U;
        }
        const auto& item = field_menu_text_.items[catalog_index].definition;
        return item.code == static_cast<std::uint8_t>(catalog_index + 1U)
                   ? static_cast<std::uint32_t>(item.pricing.resale_base)
                   : 0U;
    };
}

void FieldSceneRuntime::paint_field_shop(QPainter& painter) const {
    if (field_service_background_.isNull() ||
        !field_shop_active_definition_.has_value()) {
        return;
    }
    painter.drawImage(0, 0, field_service_background_);

    // The Lua service definition chooses the ROM title and category labels;
    // this presenter places those named text records over shared service art.
    const auto* title = game_scripts_.dialogue_text(
        field_shop_active_definition_->title_key);
    if (title != nullptr) {
        draw_command_overview_text(painter, 64, 16, *title);
    }
    const auto group = field_shop_menu_.selected_group();
    const auto group_index = field_shop_menu_.selected_group_index();
    if (group_index >= field_shop_active_definition_->groups.size()) {
        return;
    }
    const auto& group_definition =
        field_shop_active_definition_->groups[group_index];
    const bool sell_group = group_definition.id == "sell";
    const bool progression_group =
        !group_definition.progression_offers.empty();
    const bool wire_upgrade_group =
        !group_definition.equipment_upgrade_offers.empty();
    if (!wire_upgrade_group &&
        group_index < field_service_tab_images_.size()) {
        painter.drawImage(64, 128, field_service_tab_images_[group_index]);
    }
    if (field_shop_menu_.offers_panel_active()) {
            const bool transaction_replaces_detail =
                !wire_upgrade_group &&
                (field_shop_menu_.transaction_confirmation_open() ||
                 field_shop_menu_.transaction_message() !=
                     ShopTransactionMessage::none);
            const auto cursor = field_shop_menu_.selected_cursor();
            const auto selected_upgrade_offer = wire_upgrade_group
                ? field_shop_menu_.equipment_upgrade_offer(group, cursor)
                : std::optional<scripting::ScriptShopEquipmentUpgradeOffer>{};
            const auto selected_offer = sell_group || progression_group
                ? std::optional<FieldActionCostRecord>{}
                : field_shop_menu_.offer(group, cursor);
            const auto selected_progression_offer = progression_group
                ? field_shop_menu_.progression_offer(group, cursor)
                : std::optional<scripting::ScriptShopProgressionOffer>{};
            std::uint8_t selected_item_code = 0U;
            if (sell_group) {
                if (cursor < active_runner_record().inventory_item_codes.size()) {
                    selected_item_code =
                        active_runner_record().inventory_item_codes[cursor];
                }
            } else if (selected_offer.has_value() &&
                       selected_offer->type !=
                           static_cast<std::uint8_t>('E')) {
                selected_item_code = static_cast<std::uint8_t>(
                    selected_offer->type + 1U);
            }

            if (wire_upgrade_group && selected_upgrade_offer.has_value()) {
                const auto& upgrade = *selected_upgrade_offer;
                if (upgrade.category < field_service_cyberware_icons_.size()) {
                    painter.drawImage(
                        80, 32,
                        field_service_cyberware_icons_[upgrade.category]);
                }
                const auto* selected_name = game_scripts_.dialogue_text(
                    upgrade.selected_name_key);
                if (selected_name != nullptr) {
                    draw_command_overview_text(painter, 128, 32,
                                               *selected_name);
                }
                const auto* description = game_scripts_.dialogue_text(
                    upgrade.description_key);
                if (description != nullptr) {
                    draw_command_overview_text(painter, 24, 56,
                                               *description);
                }

                const auto show_upgrade_quote =
                    !field_shop_menu_.transaction_confirmation_open() &&
                    field_shop_menu_.transaction_message() ==
                        ShopTransactionMessage::none;
                if (show_upgrade_quote) {
                    const auto* price_heading = game_scripts_.dialogue_text(
                        field_shop_active_definition_->text_keys
                            .purchase_price_heading);
                    if (price_heading != nullptr) {
                        draw_command_overview_text(painter, 80, 112,
                                                   *price_heading);
                    }
                    const auto cost = apply_field_action_cost_discounts(
                        upgrade.base_cost,
                        field_shop_cost_context(
                            field_shop_menu_.menu_mode(),
                            active_runner_record(),
                            field_runner_feature_flags_.value_or(0U)));
                    draw_command_overview_number(
                        painter, command_overview_font_, 176, 112, cost,
                        NumericVisualAlignment::right, true);

                    const auto& thresholds =
                        field_shop_active_definition_
                            ->equipment_upgrade_meter_thresholds;
                    if (thresholds.size() == 16U) {
                        FieldEquipmentUpgradeTable table{};
                        std::copy(thresholds.begin(), thresholds.end(),
                                  table.meter_thresholds.begin());
                        const auto essence_cost =
                            field_equipment_upgrade_display_essence_cost(
                                table, upgrade.category,
                                active_runner_record().cyberware_flags);
                        const auto* essence_heading =
                            game_scripts_.dialogue_text(
                                field_shop_active_definition_->text_keys
                                    .cyberware_essence_heading);
                        const auto* essence_cost_heading =
                            game_scripts_.dialogue_text(
                                field_shop_active_definition_->text_keys
                                    .cyberware_essence_cost_heading);
                        if (essence_heading != nullptr) {
                            draw_command_overview_text(
                                painter, 72, 88, *essence_heading);
                        }
                        if (essence_cost_heading != nullptr) {
                            draw_command_overview_text(
                                painter, 72, 96, *essence_cost_heading);
                        }
                        const auto& runner = active_runner_record();
                        draw_command_overview_number(
                            painter, command_overview_font_, 184, 88,
                            runner.cyberware_essence_tens());
                        draw_command_overview_number(
                            painter, command_overview_font_, 200, 88,
                            runner.cyberware_essence_ones());
                        if (essence_cost.has_value()) {
                            draw_command_overview_number(
                                painter, command_overview_font_, 184, 96,
                                *essence_cost);
                            draw_command_overview_number(
                                painter, command_overview_font_, 200, 96,
                                0U);
                        }
                    }
                }
            } else if (progression_group &&
                       selected_progression_offer.has_value()) {
                // ROM 0x57D7E uses the offer byte directly as the D4EA0 row
                // index for its name, details, and glyph.
                const auto spell_code = static_cast<std::size_t>(
                    selected_progression_offer->action_code);
                std::optional<FieldActionProgressionQuoteOutcome>
                    progression_quote;
                const auto* const spell_table = field_magic_spell_table(
                    active_runner_record().magic_table_index);
                if (!transaction_replaces_detail && spell_table != nullptr) {
                    auto progression = make_field_shop_progression_input(
                        field_shop_menu_, active_runner_record(),
                        field_funds_.value_or(0U), *spell_table,
                        field_runner_feature_flags_.value_or(0U));
                    if (progression.has_value()) {
                        progression_quote =
                            prepare_field_action_progression_quote(
                                progression->tables, progression->state);
                    }
                }

                const bool runner_unavailable = progression_quote.has_value() &&
                    progression_quote->result ==
                        FieldActionProgressionQuoteResult::runner_unavailable;
                if (!runner_unavailable &&
                    spell_code < command_overview_magic_spell_icons_.size()) {
                    painter.drawImage(
                        128, 32,
                        command_overview_magic_spell_icons_[spell_code]);
                }
                if (!runner_unavailable &&
                    spell_code < field_menu_text_.magic_spell_names.size()) {
                    draw_command_overview_text(
                        painter, 104, 144,
                        field_menu_text_.magic_spell_names[spell_code]);
                }
                if (!transaction_replaces_detail && !runner_unavailable &&
                    spell_code <
                        field_menu_text_.magic_spell_descriptions.size()) {
                    draw_command_overview_text(
                        painter, 24, 56,
                        field_menu_text_.magic_spell_descriptions[spell_code]);
                }

                if (!transaction_replaces_detail &&
                    progression_quote.has_value()) {
                    const auto draw_shop_text = [this, &painter](
                        const std::string& key, const int x, const int y) {
                        const auto* text = game_scripts_.dialogue_text(key);
                        if (text != nullptr) {
                            draw_command_overview_text(painter, x, y, *text);
                        }
                    };
                    switch (progression_quote->result) {
                    case FieldActionProgressionQuoteResult::runner_unavailable:
                        draw_shop_text(
                            field_shop_active_definition_->text_keys
                                .progression_training_unavailable,
                            64, 88);
                        break;
                    case FieldActionProgressionQuoteResult::progression_limit:
                        draw_shop_text(
                            field_shop_active_definition_->text_keys
                                .progression_maximum_level,
                            64, 104);
                        break;
                    case FieldActionProgressionQuoteResult::quoted:
                        draw_shop_text(
                            field_shop_active_definition_->text_keys
                                .progression_level_label,
                            112, 64);
                        draw_command_overview_number(
                            painter, command_overview_font_, 160, 64,
                            progression_quote->quote->next_progression_level);
                        draw_shop_text(
                            field_shop_active_definition_->text_keys
                                .progression_cost_heading,
                            56, 112);
                        draw_command_overview_number(
                            painter, command_overview_font_, 208, 112,
                            progression_quote->quote->cost,
                            NumericVisualAlignment::right, true);
                        break;
                    case FieldActionProgressionQuoteResult::invalid_offer_selection:
                    case FieldActionProgressionQuoteResult::missing_record_slot:
                    case FieldActionProgressionQuoteResult::offer_unavailable:
                        break;
                    }
                }
            }

            if (!wire_upgrade_group && selected_offer.has_value() &&
                selected_offer->type == static_cast<std::uint8_t>('E')) {
                // ROM 0x586F6 uses the dedicated CE28E grid, 1AACE2
                // description, 1AACCE title, and record quantity for the
                // Universal Gun Clips row. These are source text/assets, not
                // a fabricated inventory item-code mapping.
                const auto& clips_description =
                    field_menu_text_.field_service_universal_clips_description;
                painter.drawImage(128, 32,
                                  field_service_universal_clips_icon_);
                if (!transaction_replaces_detail) {
                    draw_command_overview_text(
                        painter, 24, 56, clips_description);
                }
                draw_command_overview_text(
                    painter, 104, 144,
                    field_menu_text_.field_service_universal_clips_title);
                if (!transaction_replaces_detail) {
                    // The ROM-derived Lua description places its currency
                    // glyph on the final line. Anchor the amount immediately
                    // to its left so the last digit cannot overwrite it.
                    const auto currency_marker = clips_description.rfind('$');
                    if (currency_marker != std::string::npos) {
                        const auto line_break =
                            clips_description.rfind('\n', currency_marker);
                        const auto line_begin = line_break == std::string::npos
                            ? 0U
                            : line_break + 1U;
                        const auto marker_x = 24 + static_cast<int>(
                            (currency_marker - line_begin) * 8U);
                        const auto marker_row = static_cast<int>(
                            std::count(clips_description.begin(),
                                       clips_description.begin() +
                                           static_cast<std::ptrdiff_t>(
                                               currency_marker),
                                       '\n'));
                        draw_command_overview_number(
                            painter, command_overview_font_, marker_x,
                            56 + marker_row * 8, selected_offer->quantity,
                            NumericVisualAlignment::right);
                    }
                }
            }
            else if (!wire_upgrade_group && selected_item_code > 0U &&
                     selected_item_code <= field_menu_text_.items.size()) {
                const auto item_index =
                    static_cast<std::size_t>(selected_item_code - 1U);
                const auto& item_name = field_menu_text_.items[item_index].name;
                const auto& item_category =
                    field_menu_text_.items[item_index].category;
                const auto space = item_name.find(' ');
                if (!item_category.empty()) {
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_name.size() * 8U)) / 2,
                        144, item_name);
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_category.size() * 8U)) / 2,
                        152, item_category);
                } else if (space != std::string::npos && space > 0U &&
                           space + 1U < item_name.size()) {
                    const auto first_line =
                        std::string_view(item_name).substr(0U, space);
                    const auto second_line =
                        std::string_view(item_name).substr(space + 1U);
                    draw_command_overview_text(
                        painter, (320 - static_cast<int>(first_line.size() * 8U)) / 2,
                        144, first_line);
                    draw_command_overview_text(
                        painter, (320 - static_cast<int>(second_line.size() * 8U)) / 2,
                        152, second_line);
                } else {
                    draw_command_overview_text(
                        painter,
                        (320 - static_cast<int>(item_name.size() * 8U)) / 2,
                        144, item_name);
                }
                if (selected_item_code <=
                    command_overview_item_details_.size()) {
                    painter.drawImage(
                        124, 32,
                        command_overview_item_details_[item_index]);
                }

                if (!transaction_replaces_detail) {
                    const auto weapon_capacity =
                        selected_shop_weapon_capacity();
                    const auto item_uses = selected_shop_item_uses();
                    if (weapon_capacity.has_value()) {
                        // A named Lua weapon template supplies stock capacity;
                        // the active runner's loaded rounds stay in the HUD.
                        draw_command_overview_text(
                            painter, 72, 56,
                            field_menu_text_.item_detail_labels[0]);
                        draw_command_overview_number(
                            painter, command_overview_font_, 104, 56,
                            *weapon_capacity);
                    } else if (item_uses.has_value()) {
                        draw_command_overview_text(
                            painter, 72, 56,
                            field_menu_text_.item_detail_labels[1]);
                        draw_command_overview_number(
                            painter, command_overview_font_, 104, 56,
                            *item_uses);
                    }
                    const auto detail_text = select_field_action_detail_text(
                        static_cast<std::uint8_t>(selected_item_code - 1U));
                    if (detail_text.has_value() &&
                        detail_text->table_index <
                            field_menu_text_.field_action_detail_texts.size()) {
                        draw_command_overview_text(
                            painter, 24, 80,
                            field_menu_text_.field_action_detail_texts[
                                detail_text->table_index]);
                    }
                }

                if (!transaction_replaces_detail && !sell_group &&
                    !progression_group &&
                    selected_offer.has_value()) {
                    draw_command_overview_text(
                        painter, 80, 112,
                        field_menu_text_.shop_purchase_price_heading);
                    const auto amount = calculate_field_action_record_cost(
                        *selected_offer,
                        field_shop_cost_context(field_shop_menu_.menu_mode(),
                                          active_runner_record(),
                                          field_runner_feature_flags_.value_or(
                                              0U)),
                        item_base_cost_lookup());
                    draw_command_overview_number(
                        painter, command_overview_font_, 184, 112, amount,
                        NumericVisualAlignment::right, true);
                }
            }
    } else {
        for (std::size_t row = 0; row < group_definition.title_keys.size();
             ++row) {
            const auto* heading = game_scripts_.dialogue_text(
                group_definition.title_keys[row]);
            if (heading != nullptr) {
                draw_command_overview_text(
                    painter, 64, 144 + static_cast<int>(row) * 8, *heading);
            }
        }
    }

    const auto stock_group = field_shop_menu_.offers_panel_active()
        ? group
        : field_shop_menu_.displayed_offer_group();
    const auto stock_group_index =
        field_shop_menu_.offers_panel_active()
            ? field_shop_menu_.selected_group_index()
            : field_shop_menu_.displayed_offer_group_index();
    const bool stock_is_sell =
        stock_group_index < field_shop_active_definition_->groups.size() &&
        field_shop_active_definition_->groups[stock_group_index].id == "sell";
    const bool stock_is_progression =
        stock_group_index < field_shop_active_definition_->groups.size() &&
        !field_shop_active_definition_->groups[stock_group_index]
             .progression_offers.empty();
    const bool stock_is_wire_upgrade =
        stock_group_index < field_shop_active_definition_->groups.size() &&
        !field_shop_active_definition_->groups[stock_group_index]
             .equipment_upgrade_offers.empty();
    const auto count = stock_is_wire_upgrade
        ? field_shop_menu_.equipment_upgrade_offer_count(stock_group)
        : stock_is_sell
              ? active_runner_record().inventory_item_codes.size()
              : stock_is_progression
                    ? field_shop_menu_.progression_offer_count(stock_group)
                    : field_shop_menu_.offer_count(stock_group);
    for (std::size_t index = 0; index < count && index < 8U; ++index) {
        if (stock_is_wire_upgrade) {
            if (index < field_service_cyberware_icons_.size()) {
                painter.drawImage(
                    16 + static_cast<int>(index) * 32, 168,
                    field_service_cyberware_icons_[index]);
            }
            continue;
        }
        if (stock_is_sell) {
            const auto item_code =
                active_runner_record().inventory_item_codes[index];
            if (item_code > 0U &&
                item_code <= command_overview_inventory_icons_.size()) {
                painter.drawImage(
                    16 + static_cast<int>(index) * 32, 168,
                    command_overview_inventory_icons_[item_code - 1U][0]);
            }
            continue;
        }
        if (stock_is_progression) {
            const auto offer =
                field_shop_menu_.progression_offer(stock_group, index);
            if (!offer.has_value()) {
                continue;
            }
            const auto spell_code = static_cast<std::size_t>(
                offer->action_code);
            if (spell_code < command_overview_magic_spell_icons_.size()) {
                painter.drawImage(
                    16 + static_cast<int>(index) * 32, 168,
                    command_overview_magic_spell_icons_[spell_code]);
            }
            continue;
        }
        const auto offer = field_shop_menu_.offer(stock_group, index);
        if (!offer.has_value()) {
            continue;
        }
        if (offer->type == static_cast<std::uint8_t>('E')) {
            painter.drawImage(
                16 + static_cast<int>(index) * 32, 168,
                field_service_universal_clips_icon_);
            continue;
        }
        const auto item_code = static_cast<std::uint8_t>(offer->type + 1U);
        if (item_code > 0U &&
            item_code <= command_overview_inventory_icons_.size()) {
            painter.drawImage(
                16 + static_cast<int>(index) * 32, 168,
                command_overview_inventory_icons_[item_code - 1U][0]);
        }
    }

    if (field_shop_menu_.offers_panel_active()) {
        // The Genesis selector hugs each 24x16 item image. Draw a single
        // warm-yellow pixel around that footprint; the surrounding 32x24
        // cell and neighboring controls remain clear.
        const auto cursor = field_shop_menu_.selected_cursor();
        const auto selectable = stock_is_wire_upgrade || !sell_group ||
            (cursor < active_runner_record().inventory_item_codes.size() &&
             active_runner_record().inventory_item_codes[cursor] != 0U);
        if (selectable && cursor < count && cursor < 8U) {
            constexpr int kIconWidth = 24;
            constexpr int kIconHeight = 16;
            const auto x = 16 + static_cast<int>(cursor) * 32;
            constexpr int y = 168;
            const QColor selector_color(196, 190, 39);
            painter.fillRect(x, y, kIconWidth, 1, selector_color);
            painter.fillRect(x, y + kIconHeight - 1, kIconWidth, 1,
                             selector_color);
            painter.fillRect(x, y, 1, kIconHeight, selector_color);
            painter.fillRect(x + kIconWidth - 1, y, 1, kIconHeight,
                             selector_color);
        }
    }

    paint_field_hud_overlays(painter);
    if (field_funds_.has_value()) {
        draw_command_overview_number(
            painter, command_overview_font_, 144, 200, *field_funds_,
            NumericVisualAlignment::left, true);
        draw_command_overview_text(painter, 200, 200, "$");
    }

    if (field_shop_menu_.transaction_confirmation_open()) {
        if (wire_upgrade_group) {
            const auto* heading = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .cyberware_purchase_heading);
            if (heading != nullptr) {
                draw_command_overview_text(painter, 48, 88, *heading);
            }
            draw_command_overview_number(
                painter, command_overview_font_, 216, 88,
                field_shop_menu_.transaction_amount(),
                NumericVisualAlignment::right, true);
        } else {
            draw_command_overview_text(
                painter, 24, 80, field_menu_text_.shop_transaction_prompt);
        }
        if (sell_group && !wire_upgrade_group) {
            draw_command_overview_text(
                painter, 56, 88, field_menu_text_.shop_sale_quote_heading);
            draw_command_overview_number(
                painter, command_overview_font_, 224, 88,
                field_shop_menu_.transaction_amount(),
                NumericVisualAlignment::right, true);
        }
        draw_command_overview_text(
            painter, 128, 96, field_menu_text_.action_choice_yes);
        draw_command_overview_text(
            painter, 128, 112, field_menu_text_.action_choice_no);
        draw_command_overview_text(
            painter, 112,
            field_shop_menu_.transaction_confirmation().selected_option == 0U
                ? 96
                : 112,
            ">");
    } else {
        switch (field_shop_menu_.transaction_message()) {
        case ShopTransactionMessage::no_inventory_slot:
            draw_command_overview_text(
                painter, 96, 88, field_menu_text_.shop_no_inventory_slot);
            break;
        case ShopTransactionMessage::insufficient_funds:
            draw_command_overview_text(
                painter, 56, 88, field_menu_text_.shop_insufficient_funds);
            break;
        case ShopTransactionMessage::special_action_limit:
            draw_command_overview_text(
                painter, 56, 88,
                field_menu_text_.shop_special_action_limit);
            break;
        case ShopTransactionMessage::protected_sale:
            draw_command_overview_text(
                painter, 48, 96, field_menu_text_.shop_protected_sale);
            break;
        case ShopTransactionMessage::training_unavailable: {
            const auto* message = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .progression_training_unavailable);
            if (message != nullptr) {
                draw_command_overview_text(painter, 64, 88, *message);
            }
            break;
        }
        case ShopTransactionMessage::cyberware_already_owned: {
            const auto* message = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .cyberware_already_owned);
            if (message != nullptr) {
                draw_command_overview_text(painter, 64, 88, *message);
            }
            break;
        }
        case ShopTransactionMessage::cyberware_spurs_conflict: {
            const auto* message = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .cyberware_spurs_conflict);
            if (message != nullptr) {
                draw_command_overview_text(painter, 72, 88, *message);
            }
            break;
        }
        case ShopTransactionMessage::cyberware_insufficient_essence: {
            const auto* message = game_scripts_.dialogue_text(
                field_shop_active_definition_->text_keys
                    .cyberware_insufficient_essence);
            if (message != nullptr) {
                draw_command_overview_text(painter, 64, 88, *message);
            }
            break;
        }
        case ShopTransactionMessage::none:
            break;
        }
    }
}

void FieldSceneRuntime::paint_field_shop_notice(QPainter& painter) const {
    if (!field_shop_notice_active_ || field_service_background_.isNull()) {
        return;
    }
    painter.drawImage(0, 0, field_service_background_);
    const auto* title = game_scripts_.dialogue_text(
        "shop.service.option." + field_shop_notice_service_id_);
    if (title != nullptr) {
        draw_command_overview_text(painter, 64, 16, *title);
    }
    const auto* message = game_scripts_.dialogue_text(
        field_shop_notice_text_key_);
    if (message != nullptr) {
        // ROM 0x56C12 writes the message at name-table word 0xC316. On the
        // source 64-cell plane this is pixel origin (88, 48).
        draw_command_overview_text(painter, 88, 48, *message);
    }
    paint_field_hud_overlays(painter);
    if (field_funds_.has_value()) {
        draw_command_overview_number(
            painter, command_overview_font_, 144, 200, *field_funds_,
            NumericVisualAlignment::left, true);
        draw_command_overview_text(painter, 200, 200, "$");
    }
}

void FieldSceneRuntime::draw_command_overview_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text) const {
    draw_tinted_field_text(painter, x, y, text, QColor(0, 255, 0));
}

void FieldSceneRuntime::draw_tinted_field_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text, const QColor color) const {
    if (command_overview_font_.isNull() || command_overview_font_.width() < 8 ||
        command_overview_font_.height() < 8) {
        return;
    }
    auto cursor = x;
    auto cursor_y = y;
    for (const unsigned char code : text) {
        if (code == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        if (code < 0x20U || code > 0x7EU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 > command_overview_font_.width() ||
            source_y + 8 > command_overview_font_.height()) {
            cursor += 8;
            continue;
        }
        QImage tinted_glyph(8, 8, QImage::Format_ARGB32_Premultiplied);
        tinted_glyph.fill(Qt::transparent);
        for (int glyph_y = 0; glyph_y < 8; ++glyph_y) {
            for (int glyph_x = 0; glyph_x < 8; ++glyph_x) {
                const auto source_pixel = command_overview_font_.pixelColor(
                    source_x + glyph_x, source_y + glyph_y);
                const auto coverage = std::max(
                    {source_pixel.red(), source_pixel.green(),
                     source_pixel.blue()});
                if (source_pixel.alpha() == 0 || coverage == 0) {
                    continue;
                }
                const auto alpha = static_cast<int>(
                    (static_cast<std::uint32_t>(source_pixel.alpha()) *
                     static_cast<std::uint32_t>(coverage)) /
                    255U);
                auto tinted_color = color;
                tinted_color.setAlpha(alpha);
                tinted_glyph.setPixelColor(glyph_x, glyph_y,
                                           tinted_color);
            }
        }
        painter.drawImage(cursor, cursor_y, tinted_glyph);
        cursor += 8;
    }
}

void FieldSceneRuntime::draw_field_menu_font_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text, const QImage& font) const {
    if (font.isNull() || font.width() < 8 || font.height() < 8) {
        return;
    }
    auto cursor = x;
    auto cursor_y = y;
    for (const auto character : text) {
        if (character == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 > font.width() || source_y + 8 > font.height()) {
            cursor += 8;
            continue;
        }
        painter.drawImage(cursor, cursor_y, font,
                          source_x, source_y, 8, 8);
        cursor += 8;
    }
}

void FieldSceneRuntime::draw_hotel_menu_text(
    QPainter& painter, const int x, const int y,
    const std::string_view text,
    const HotelMenuPaletteLine palette_line) const {
    const auto& font = palette_line == HotelMenuPaletteLine::checkout
                           ? hotel_checkout_font_
                           : command_overview_font_;
    if (font.isNull() || font.width() < 8 || font.height() < 8) {
        return;
    }
    auto cursor = x;
    auto cursor_y = y;
    for (const auto character : text) {
        if (character == '\n') {
            cursor = x;
            cursor_y += 8;
            continue;
        }
        const auto code = static_cast<unsigned char>(character);
        if (code < 0x20U || code > 0x7FU) {
            cursor += 8;
            continue;
        }
        const auto glyph = static_cast<int>(code - 0x20U);
        const auto source_x = (glyph % 16) * 8;
        const auto source_y = (glyph / 16) * 8;
        if (source_x + 8 <= font.width() &&
            source_y + 8 <= font.height()) {
            // Hotel pages select the CRAM line in the source text writer.
            // Keep the extracted RGBA colors intact for both green options
            // text and the purple nested checkout prompt.
            painter.drawImage(cursor, cursor_y, font, source_x, source_y, 8,
                              8);
        }
        cursor += 8;
    }
}

void FieldSceneRuntime::refresh_command_overview_combat_bars() {
    if (!command_overview_.open()) {
        command_overview_combat_bars_ = EquipmentCombatBarResult{};
        return;
    }

    const auto& record = active_runner_record();
    EquipmentCombatInput input{};
    input.attributes = record.attributes;
    // The source combat adapter names the sparse +0x81..+0x8C window. The
    // live-record projection stores +0x80..+0x85 and +0x87..+0x8C as the
    // typed skill array, with +0x86 retained in auxiliary[1].
    input.skills = {record.skills[1],  record.skills[2], record.skills[3],
                    record.skills[4],  record.skills[5], record.auxiliary[1],
                    record.skills[6],  record.skills[7], record.skills[8],
                    record.skills[9],  record.skills[10], record.skills[11]};
    input.inventory_item_codes = record.inventory_item_codes;
    input.inventory_phase_bytes = record.inventory_phase_bytes;
    input.display_state = record.equipped_inventory_slot;
    input.active_item_code = record.equipped_item_code;
    input.posture = record.posture;
    // The command overview's source selector is outside the eight-cell item
    // grid. This keeps the normal higher-mode defense branch from treating a
    // highlighted possession cell as an action lookup.
    input.selected_inventory_slot =
        static_cast<std::uint8_t>(record.inventory_item_codes.size());
    input.cyberware_flags = record.cyberware_flags;
    input.action_flags = static_cast<std::uint8_t>(record.flags & 0xFFU);
    // ROM 0x12CEE's ordinary command-overview caller is in the higher-mode
    // branch. It uses +0x56/+0x57 directly for the attack strip and still
    // performs the typed defense calculation below it.
    input.source_mode = 2U;
    // When +0x56 is negative, 0x12D74/0x12FA4 uses the active action's
    // Sorcery, Magic cap, and current force before applying item-focus and
    // slot-7 totem data. Keep those values sourced from the selected live
    // runner record so class/race presets and the action's force remain part
    // of the displayed number.
    input.special_item18_base = record.skills[0U]; // +0x80 Sorcery.
    input.roll_count_cap = record.auxiliary[0U]; // +0x7F Magic cap.
    input.auxiliary_roll_bonus = record.magic_force; // +0x9D current force.
    if (input.display_state < 0 && input.active_item_code > 0U) {
        const auto* spell = find_us_magic_spell(static_cast<std::uint8_t>(
            input.active_item_code - 1U));
        if (spell != nullptr) {
            input.special_magic_spell = *spell;
        }
    }
    input.item_catalog = &us_item_catalog();
    input.selector_table = &us_special_item_modifier_selector_table();
    command_overview_combat_bars_ = compute_equipment_combat_bars(input);
}

void FieldSceneRuntime::refresh_command_overview_item_detail() {
    command_overview_item_detail_ = ItemActionDetailResult{};
    if (!command_overview_.open() ||
        command_overview_.view().source_state.primary_page >= 2U) {
        return;
    }

    const auto slot =
        command_overview_.view().source_state.secondary_page * 2U +
        command_overview_.view().source_state.primary_page;
    if (slot >= active_runner_record().inventory_item_codes.size()) {
        return;
    }
    ItemActionDetailInput input{};
    input.item_code = active_runner_record().inventory_item_codes[slot];
    for (std::size_t index = 0;
         index < active_runner_record().inventory_phase_bytes.size();
         ++index) {
        input.phase_bytes[index] =
            active_runner_record().inventory_phase_bytes[index];
    }
    // The ninth byte in the source phase span is active record +0x76, the
    // one-based armor ordinal. Preserve it as typed data for 0x12A8C's
    // item-detail value selection.
    input.phase_bytes[8] = active_runner_record().equipped_armor_index;
    input.phase_slot = 8U;
    command_overview_item_detail_ = render_item_action_details(
        input, us_item_catalog(), command_overview_resource_variant_);
}

void FieldSceneRuntime::paint_command_overview_clips(
    QPainter& painter) const {
    // ROM 0x12C36 starts at name-table C118 (screen 96,16) and clears the
    // source 20x20 cell region. The field HUD is composed afterward as a
    // separate shared layer.
    painter.fillRect(96, 16, 160, 160, Qt::black);
    // The clear includes the top 16 pixels of the shared posture panel.
    // ROM 0x11B5C re-enters 0x11900 after that clear, rebuilding this shell
    // before installing the Clips row. Restore the panel as one unit here.
    paint_command_overview_posture_panel(painter);

    // The parent command tabs occupy the full 160 px panel. CD33E/CD37A are
    // only 15 cells wide, so replacing just their centered 120 px strip can
    // leave parts of the old five-button row at the sides. Rebuild the whole
    // panel row, keeping only the shared frame edges before drawing a child
    // row. Result messages restore the parent row from this same background.
    constexpr QRect kButtonPanelRect{96, 184, 160, 16};
    constexpr QRect kButtonPanelLeftFrameRect{96, 184, 24, 16};
    constexpr QRect kButtonPanelRightFrameRect{240, 184, 16, 16};
    painter.fillRect(kButtonPanelRect, Qt::black);
    painter.drawImage(kButtonPanelLeftFrameRect, command_overview_background_,
                      kButtonPanelLeftFrameRect);
    painter.drawImage(kButtonPanelRightFrameRect, command_overview_background_,
                      kButtonPanelRightFrameRect);

    const auto draw_lines = [this, &painter](const auto& lines,
                                              const int x,
                                              const int y) {
        for (std::size_t index = 0; index < lines.size(); ++index) {
            draw_command_overview_text(
                painter, x, y + static_cast<int>(index * 8U), lines[index]);
        }
    };

    if (command_overview_clips_.reload_confirmation_open()) {
        // ROM 0x11D06/0x127B0 replaces CD33E with the dedicated CD3B6 Yes/No
        // row; 0x127FC selects one of its two source focus blocks.
        painter.drawImage(kCommandOverviewClipsBackgroundX,
                          kCommandOverviewClipsBackgroundY,
                          command_overview_action_confirmation_background_);
        const auto choice_index =
            command_overview_clips_.reload_confirmation_yes() ? 0U : 1U;
        painter.drawImage(
            kCommandOverviewActionConfirmationFocusX[choice_index],
            kCommandOverviewActionConfirmationFocusY,
            command_overview_action_confirmation_focus_[choice_index]);
        draw_lines(field_menu_text_.clips_reload_prompt, 120, 72);
        draw_command_overview_text(
            painter, 112, 200,
            centered_command_footer(
                command_overview_clips_.reload_confirmation_yes()
                    ? field_menu_text_.action_choice_yes
                    : field_menu_text_.action_choice_no));
        return;
    }

    if (command_overview_clips_.trade_open()) {
        // ROM 0x11E7E installs the nested trade row from CD37A.
        painter.drawImage(kCommandOverviewClipsBackgroundX,
                          kCommandOverviewClipsBackgroundY,
                          command_overview_clips_trade_background_);
        // ROM 0x11E34 renders the source trade prompt, then 0x1209A updates
        // the recipient name when the nested selector moves. The source
        // D1B71 ???? text is retained only as the fallback for a record whose
        // projected +0x58 name field is empty.
        draw_command_overview_text(painter, 144, 72,
                                   field_menu_text_.clips_trade_prompt[0]);
        draw_command_overview_text(painter, 144, 80,
                                   field_menu_text_.clips_trade_prompt[1]);
        const auto recipient = command_overview_clips_.trade_recipient_record();
        if (recipient < FieldPartyRosterRuntime::kSlotCount &&
            !field_party_roster_.name(recipient).empty()) {
            draw_command_overview_text(
                painter, 120, 88,
                centered_command_text(field_party_roster_.name(recipient),
                                      15U));
        } else {
            draw_command_overview_text(painter, 120, 88,
                                       centered_command_text(
                                           field_menu_text_.clips_trade_prompt[2],
                                           15U));
        }
        draw_command_overview_number(
            painter, command_overview_font_, 200, 72,
            command_overview_clips_.trade_quantity());
        const auto selection = command_overview_clips_.trade_selection();
        if (selection >= static_cast<std::uint8_t>(ClipsCommand::reload) &&
            selection <= static_cast<std::uint8_t>(ClipsCommand::cancel)) {
            const auto focus_index = static_cast<std::size_t>(selection - 2U);
            painter.drawImage(kCommandOverviewClipsFocusX[focus_index],
                              kCommandOverviewClipsFocusY,
                              command_overview_clips_trade_focus_[focus_index]);
        }
        // ROM 0x11EA0/0x11EF8 select the nested Trade caption from D1E24.
        // Keep the visible footer in step with the selected button art.
        const auto footer_index =
            selection >= 2U && selection <= 5U
                ? static_cast<std::size_t>(selection - 2U)
                : 0U;
        draw_command_overview_text(painter, 112, 200,
                                   centered_command_footer(
                                       field_menu_text_.clips_trade_commands[
                                           footer_index]));
        return;
    }

    if (command_overview_clips_.message() != ClipsMenuMessage::none) {
        // ROM 0x12C36 clears only the center window through y=176. At 0x11CB6
        // the result path uploads CD2C6, restoring the shared command tabs
        // while the result text waits for a fresh controller edge. Keep the
        // parent row here; showing the child Clips/Trade row would be stale.
        painter.drawImage(kButtonPanelRect, command_overview_background_,
                          kButtonPanelRect);
        switch (command_overview_clips_.message()) {
        case ClipsMenuMessage::out_of_ammo: {
            const auto active_slot = field_party_roster_.active_slot();
            draw_command_overview_text(
                painter, 112, 80,
                field_party_roster_.name(active_slot).empty()
                    ? field_menu_text_.runner_names[0]
                    : field_party_roster_.name(active_slot));
            draw_lines(field_menu_text_.clips_out_of_ammo, 112, 88);
            break;
        }
        case ClipsMenuMessage::item_does_not_use_clips:
            draw_lines(field_menu_text_.clips_item_no_clips, 136, 80);
            break;
        case ClipsMenuMessage::reload_success:
            draw_lines(field_menu_text_.clips_reload_success, 112, 80);
            break;
        case ClipsMenuMessage::exchange_complete:
            draw_lines(field_menu_text_.clips_exchange_complete, 112, 80);
            break;
        case ClipsMenuMessage::no_party_members:
            draw_lines(field_menu_text_.clips_no_party, 120, 72);
            break;
        case ClipsMenuMessage::function_aborted:
            draw_command_overview_text(
                painter, 128, 88, field_menu_text_.clips_function_aborted);
            break;
        case ClipsMenuMessage::none:
            break;
        }
        return;
    }

    draw_command_overview_text(painter, 136, 40,
                               field_menu_text_.clips_title);
    draw_command_overview_text(painter, 136, 48,
                               field_menu_text_.clips_separator);

    // ROM 0x11B98 installs the top-level Clips row from CD33E. It has the
    // same footprint as the inventory action row but different button art.
    painter.drawImage(kCommandOverviewClipsBackgroundX,
                      kCommandOverviewClipsBackgroundY,
                      command_overview_clips_background_);

    draw_command_overview_text(painter, 136, 72,
                               field_menu_text_.clips_counter_label);
    draw_command_overview_number(
        painter, command_overview_font_, 208, 72,
        active_runner_record().ammo_clips);
    const auto selection = command_overview_clips_.selection();
    if (selection >= static_cast<std::uint8_t>(ClipsCommand::reload) &&
        selection <= static_cast<std::uint8_t>(ClipsCommand::cancel)) {
        const auto focus_index = static_cast<std::size_t>(selection - 2U);
        painter.drawImage(kCommandOverviewClipsFocusX[focus_index],
                          kCommandOverviewClipsFocusY,
                          command_overview_clips_focus_[focus_index]);
    }
    // ROM 0x11BC2/0x11C1A select the matching action caption from D1DDC.
    // Keep the visible footer in step with the selected button art.
    const auto footer_index =
        selection >= 2U && selection <= 5U
            ? static_cast<std::size_t>(selection - 2U)
            : 0U;
    draw_command_overview_text(painter, 112, 200,
                               centered_command_footer(
                                   field_menu_text_.clips_commands[
                                       footer_index]));
}

void FieldSceneRuntime::paint_command_overview_posture_panel(
    QPainter& painter) const {
    // ROM 0x11900 redraws the common lower shell after Clips clears the
    // center window. The posture frame is part of that shared panel; drawing
    // this crop prevents the child clear from leaving a black band above the
    // action buttons.
    constexpr QRect kPosturePanelRect{96, 160, 160, 24};
    painter.drawImage(kPosturePanelRect, command_overview_background_,
                      kPosturePanelRect);

    const auto posture = std::min<std::size_t>(
        active_runner_record().posture,
        command_overview_status_bars_.size() - 1U);
    painter.drawImage(176, 168, command_overview_status_bars_[posture]);
    draw_command_overview_text(painter, 136, 176,
                               field_menu_text_.status_names[posture]);
    if (posture_dialog_active_) {
        painter.drawImage(120, 168, posture_focus_);
    }
}

void FieldSceneRuntime::paint_attributes_skills_menu(
    QPainter& painter, const AttributesSkillsMenuController& menu) const {
    // ROM 0xD01C clears both planes and installs the command-menu view. The
    // Hotel option enters the shared page body at 0xD024, where the upgrade
    // detail fields are enabled by the live selection-data state.
    // Paint it as the page's base so no command buttons or stale field HUD
    // pixels can remain underneath the new menu.
    painter.drawImage(0, 0, command_overview_attributes_background_);

    const auto& record = active_runner_record();
    const auto slot = field_party_roster_.active_slot();
    const auto runner_name = field_party_roster_.name(slot).empty()
                                 ? field_menu_text_.runner_names[0]
                                 : field_party_roster_.name(slot);
    draw_command_overview_text(painter, 80, 24, runner_name);
    if (record.species_index < field_menu_text_.race_names.size()) {
        draw_command_overview_text(
            painter, 80, 32,
            field_menu_text_.race_names[record.species_index]);
    }
    if (record.archetype_index < field_menu_text_.class_names.size()) {
        draw_command_overview_text(
            painter, 128, 32,
            field_menu_text_.class_names[record.archetype_index]);
    }
    // The source calls 0x110EC with VDP destination E132. That is the same
    // runner panel family used at E104 on the command page, shifted to x=200.
    if (slot < field_hud_faces_.size() &&
        !field_hud_faces_[slot].isNull()) {
        painter.drawImage(200, 16, field_hud_faces_[slot]);
    }

    const auto grid = menu.grid(record);
    const auto cursor = menu.state().cursor;
    for (std::size_t column = 0; column < kAttributesSkillsColumnCount;
         ++column) {
        for (std::size_t row = 0; row < kAttributesSkillsRowCount; ++row) {
            const auto& cell = grid[column][row];
            if (cell.label_index >=
                field_menu_text_.attribute_skill_labels.size()) {
                continue;
            }
            const auto x = column == 0U ? 24 : 176;
            const auto y = row < 4U
                               ? 56 + static_cast<int>(row) * 8
                               : 104 + static_cast<int>(row - 4U) * 8;
            if (cursor.column == column && cursor.row == row) {
                // 0xD5DC paints the focused label and value after the base
                // grid. Leave this cell clear here so transparent glyphs
                // from the green base text cannot show through the purple
                // selection layer.
                continue;
            }
            draw_command_overview_text(
                painter, x, y,
                field_menu_text_.attribute_skill_labels[cell.label_index]);
            // The source places the static Essence value one tile to the
            // left of the other right-column values. When Essence is focused,
            // 0xD5DC replaces it with the live and secondary values below.
            const auto value_x = cell.label_index == 6U
                                     ? 272
                                     : (column == 0U ? 136 : 280);
            draw_command_overview_number(
                painter, command_overview_font_, value_x, y, cell.value);
        }
    }

    const auto draw_detail_text = [&](const int x, const int y,
                                      const std::string_view text,
                                      const std::uint16_t text_mode) {
        const auto palette_line = text_mode == 0x4000U
                                      ? HotelMenuPaletteLine::checkout
                                      : HotelMenuPaletteLine::status_and_options;
        draw_hotel_menu_text(painter, x, y, text, palette_line);
    };
    CyberwareDetailCallbacks detail_callbacks{};
    detail_callbacks.render_selected_label =
        [&](const CyberwareDetailLabel& label,
            const std::uint16_t text_mode) {
            std::size_t label_index = 0U;
            int x = 24;
            int y = 56;
            if (label.section == CyberwareDetailLabelSection::attribute) {
                label_index = label.index;
                const auto column = static_cast<int>(label.index / 4U);
                const auto row = static_cast<int>(label.index % 4U);
                x = column == 0 ? 24 : 176;
                y = 56 + row * 8;
            } else {
                label_index = 8U + label.index;
                const auto column = static_cast<int>(label.index / 6U);
                const auto row = static_cast<int>(label.index % 6U);
                x = column == 0 ? 24 : 176;
                y = 104 + row * 8;
            }
            if (label_index < field_menu_text_.attribute_skill_labels.size()) {
                draw_detail_text(
                    x, y, field_menu_text_.attribute_skill_labels[label_index],
                    text_mode);
            }
        };
    detail_callbacks.render_numeric_value =
        [&](const CyberwareDetailNumericField field, const std::uint16_t value,
            const std::uint16_t text_mode) {
            int x = 0;
            int y = 0;
            switch (field) {
            case CyberwareDetailNumericField::selected_attribute_or_skill:
            case CyberwareDetailNumericField::magic:
                x = cursor.column == 0U ? 136 : 280;
                y = cursor.row < 4U
                        ? 56 + static_cast<int>(cursor.row) * 8
                        : 104 + static_cast<int>(cursor.row - 4U) * 8;
                break;
            case CyberwareDetailNumericField::essence:
                x = 272;
                y = 72;
                break;
            case CyberwareDetailNumericField::essence_secondary:
                x = 288;
                y = 72;
                break;
            case CyberwareDetailNumericField::quickness_cyberware_modifier:
                x = 160;
                y = 64;
                break;
            case CyberwareDetailNumericField::strength_cyberware_modifier:
                x = 160;
                y = 72;
                break;
            case CyberwareDetailNumericField::body_cyberware_modifier:
                x = 160;
                y = 56;
                break;
            }
            if (text_mode == 0x4000U) {
                draw_numeric_visual(
                    painter, hotel_checkout_font_, x, y, value,
                    NumericVisualAlignment::left, false);
            } else {
                draw_command_overview_number(
                    painter, command_overview_font_, x, y, value);
            }
        };
    detail_callbacks.render_cyberware_modifier_label =
        [&](const CyberwareDetailModifierField field,
            const std::uint16_t text_mode) {
            const auto y = field == CyberwareDetailModifierField::body
                               ? 56
                               : field == CyberwareDetailModifierField::quickness
                                     ? 64
                                     : 72;
            // The ROM resource at 0xC259D is the one-character cyberware
            // marker "c". The enum identifies its stat row; it is not a
            // visible word label.
            draw_detail_text(144, y, "c", text_mode);
        };
    const auto detail_values = read_cyberware_detail_values(record);
    (void)render_attribute_skill_detail_panel(
        detail_values, {cursor.column, cursor.row}, 0x4000U,
        detail_callbacks);

    draw_command_overview_number(
        painter, command_overview_font_, 120, 160,
        static_cast<unsigned>(record.auxiliary[2U]));

    const auto& menu_state = menu.state();
    const auto& selected = selected_attribute_skill_cell(
        grid, menu_state.cursor);
    if (selected.label_index <
        field_menu_text_.attribute_skill_descriptions.size()) {
        // 0xC1B70 description, three 8-px rows from (24,168) [C: emulator
        // run3 8600 rows 168/176/184].
        draw_hotel_menu_text(
            painter, 24, 168,
            field_menu_text_.attribute_skill_descriptions[selected.label_index],
            HotelMenuPaletteLine::checkout);
    }

    if (menu.allows_karma_upgrades()) {
        const auto upgrade = menu.preview(record);
        if (upgrade.status == AttributeSkillUpgradeStatus::available) {
            draw_hotel_menu_text(
                painter, 64, 200,
                field_menu_text_.attributes_upgrade_header,
                HotelMenuPaletteLine::checkout);
            draw_numeric_visual(
                painter, hotel_checkout_font_, 160, 200,
                static_cast<unsigned>(upgrade.next_value));
            draw_numeric_visual(
                painter, hotel_checkout_font_, 248, 200,
                static_cast<unsigned>(upgrade.karma_cost));
        }
    }

    switch (menu.allows_karma_upgrades()
                ? menu.message()
                : AttributesSkillsMenuMessage::none) {
    case AttributesSkillsMenuMessage::none:
        break;
    case AttributesSkillsMenuMessage::maximum_level:
        draw_hotel_menu_text(
            painter, 48, 200,
            field_menu_text_.attributes_upgrade_maximum_level_message,
            HotelMenuPaletteLine::checkout);
        break;
    case AttributesSkillsMenuMessage::mage_only_skill:
        draw_hotel_menu_text(
            painter, 72, 176,
            field_menu_text_.attributes_upgrade_mage_only_message,
            HotelMenuPaletteLine::checkout);
        break;
    case AttributesSkillsMenuMessage::insufficient_karma:
        draw_hotel_menu_text(
            painter, 72, 176,
            field_menu_text_.attributes_upgrade_insufficient_karma_message,
            HotelMenuPaletteLine::checkout);
        break;
    }

    if (menu.allows_karma_upgrades() && menu.upgrade_prompt_open()) {
        // D53C writes its choice text at CAb2 and the cursor at CAAE.
        // Keep the existing Attributes/Skills detail beneath this nested
        // prompt, matching the source's partial redraw of the lower panel.
        draw_hotel_menu_text(
            painter, 96, 176, field_menu_text_.attributes_upgrade_prompt,
            HotelMenuPaletteLine::checkout);
        draw_hotel_menu_text(painter, 200, 168,
                             field_menu_text_.action_choice_yes,
                             HotelMenuPaletteLine::checkout);
        draw_hotel_menu_text(painter, 200, 184,
                             field_menu_text_.action_choice_no,
                             HotelMenuPaletteLine::checkout);
        const auto cursor_y =
            menu.upgrade_prompt_selection() == 0U
                ? 168
                : 184;
        painter.drawImage(184, cursor_y, hotel_checkout_cursor_);
    }
}

const MagicSpellTable* FieldSceneRuntime::field_magic_spell_table(
    const std::uint16_t table_index) const noexcept {
    if (table_index >= field_magic_spell_table_overrides_.size()) {
        return nullptr;
    }
    const auto& override_table =
        field_magic_spell_table_overrides_[table_index];
    return override_table.has_value()
               ? &*override_table
               : find_us_magic_spell_table(table_index);
}

void FieldSceneRuntime::paint_command_overview_magic(
    QPainter& painter) const {
    // FBA6 clears Plane A before installing this 34x28 layer. The command
    // overview shell is therefore absent beneath transparent Magic cells;
    // its separately presented right-hand field HUD is composed afterward.
    painter.drawImage(0, 0, command_overview_magic_panel_background_);
    const auto* const lua_view =
        menu_route_active_ && menu_route_magic_view_.has_value()
            ? &*menu_route_magic_view_
            : nullptr;
    const auto* table = field_magic_spell_table(
        active_runner_record().magic_table_index);
    if (table != nullptr) {
        for (std::size_t index = 0;
             index < table->entry_count &&
             index < kMagicSpellTableEntryCapacity;
             ++index) {
            const auto spell_code = table->entries[index].spell_code;
            if (spell_code >= command_overview_magic_spell_icons_.size()) {
                continue;
            }
            const auto column = index % 7U;
            const auto row = index / 7U;
            painter.drawImage(
                32 + static_cast<int>(column * 32U),
                168 + static_cast<int>(row * 24U),
                command_overview_magic_spell_icons_[spell_code]);
        }
    }

    const auto selected_spell_code =
        lua_view != nullptr
            ? lua_view->spell_code
            : command_overview_magic_selected_spell_.has_value()
                  ? static_cast<int>(command_overview_magic_selected_spell_->spell_code)
                  : -1;
    if (selected_spell_code < 0 || selected_spell_code > 0xFF) {
        return;
    }
    const auto spell_code = static_cast<std::uint8_t>(selected_spell_code);
    const auto* spell = find_us_magic_spell(spell_code);
    if (spell_code < field_menu_text_.magic_spell_names.size()) {
        // ROM 0x10276 sends the selected D4EA0 glyph triplet to C194, which
        // is the 24x24 cell beside the spell title. The menu row below uses
        // the same named glyph assets at its separate list destinations.
        if (spell_code < command_overview_magic_spell_icons_.size()) {
            painter.drawImage(
                80, 24, command_overview_magic_spell_icons_[spell_code]);
        }
        draw_field_menu_font_text(
            painter, 112, 32,
            field_menu_text_.magic_spell_names[spell_code],
            magic_menu_font_);
        if (magic_force_selection_open() ||
            magic_posture_selection_open()) {
            // ROM 0x102D4 replaces the spell description with the action
            // heading at C390 while Force/Posture selection is active.
            // paint() starts from the named panel background each frame, so
            // omitting the old description also clears both stale text rows.
            draw_field_menu_font_text(
                painter, 64, 56,
                field_menu_text_.magic_force_posture_heading,
                magic_menu_font_);
        } else {
            draw_field_menu_font_text(
                painter, 32, 48,
                field_menu_text_.magic_spell_descriptions[spell_code],
                magic_menu_font_);
        }
    }
    // ROM 0x102EC passes each complete encoded label block to C0DA. The
    // 0x80 bytes inside them preserve the line breaks and empty Force-to-
    // Posture row; this is not the fallback heading used by 0x102D4.
    draw_field_menu_font_text(
        painter, 32, 80, field_menu_text_.magic_panel_metric_labels,
        magic_menu_font_);
    draw_field_menu_font_text(
        painter, 160, 104, field_menu_text_.magic_force_posture_labels,
        magic_menu_font_);
    draw_numeric_visual(painter, magic_menu_font_, 216, 104,
                        magic_current_force());

    const auto& runner = active_runner_record();
    MagicMenuValueRefreshInput success_input{};
    success_input.current_force = magic_current_force();
    success_input.action_cap = runner.auxiliary[0];
    success_input.active_entity = magic_action_record_from_runner(runner);
    if (lua_view != nullptr) {
        if (lua_view->success_row >= 0 &&
            static_cast<std::size_t>(lua_view->success_row) <
                command_overview_magic_metric_bars_.size()) {
            painter.drawImage(
                32, 88,
                command_overview_magic_metric_bars_[
                    static_cast<std::size_t>(lua_view->success_row)]);
        }
    } else {
    MagicMenuValueRefreshCallbacks success_callbacks{};
    success_callbacks.resolve_menu_metric =
        [this, &runner, &success_input](
        const std::uint16_t value) -> std::optional<std::uint16_t> {
        const auto* spell_table = field_magic_spell_table(
            runner.magic_table_index);
        if (spell_table == nullptr ||
            !command_overview_magic_selected_spell_.has_value()) {
            return std::nullopt;
        }

        // FBA6 copies this count-and-pair table into FFF9FA. 0x104C8 then
        // selects an even byte using its source row*6+column expression;
        // preserve that source indexing through the typed lookup helper.
        std::vector<std::uint8_t> source_action_indices;
        source_action_indices.reserve(2U +
                                      spell_table->entry_count * 2U);
        source_action_indices.push_back(0U);
        source_action_indices.push_back(static_cast<std::uint8_t>(
            spell_table->entry_count - 1U));
        for (std::size_t index = 0;
             index < spell_table->entry_count &&
             index < kMagicSpellTableEntryCapacity;
             ++index) {
            source_action_indices.push_back(
                spell_table->entries[index].spell_code);
            source_action_indices.push_back(
                spell_table->entries[index].force_limit);
        }

        MenuMetricLookupInput metric_input{};
        metric_input.menu_row = command_overview_magic_loop_.selection.row;
        metric_input.menu_column =
            command_overview_magic_loop_.selection.column;
        metric_input.source_action_indices =
            std::move(source_action_indices);
        metric_input.value = value;
        metric_input.entity = success_input.active_entity;
        metric_input.item_catalog = &us_item_catalog();
        metric_input.selector_table =
            &us_special_item_modifier_selector_table();
        const auto metric = resolve_menu_metric_action(metric_input);
        if (!metric.has_value() || metric->modifiers.source_data_missing) {
            return std::nullopt;
        }
        return metric->modifiers.value;
    };
    const auto success =
        refresh_magic_menu_success_or_defense_values(success_input,
                                                     success_callbacks);
    if (!success.grid_requests.empty() &&
        success.grid_requests.front().row <
            command_overview_magic_metric_bars_.size()) {
        painter.drawImage(
            32, 88,
            command_overview_magic_metric_bars_[
                success.grid_requests.front().row]);
    }
    }

    if (lua_view != nullptr) {
        if (lua_view->physical_defense_row >= 0 &&
            static_cast<std::size_t>(lua_view->physical_defense_row) <
                command_overview_magic_metric_bars_.size()) {
            painter.drawImage(
                32, 128,
                command_overview_magic_metric_bars_[static_cast<std::size_t>(
                    lua_view->physical_defense_row)]);
        }
        if (lua_view->mana_defense_value != 0 &&
            lua_view->mana_defense_row >= 0 &&
            static_cast<std::size_t>(lua_view->mana_defense_row) <
                command_overview_magic_metric_bars_.size()) {
            painter.drawImage(
                32, 144,
                command_overview_magic_metric_bars_[static_cast<std::size_t>(
                    lua_view->mana_defense_row)]);
        }
    } else {
    MagicMenuValueRefreshInput defense_input{};
    defense_input.refresh_defense_strips = true;
    defense_input.active_entity = success_input.active_entity;
    defense_input.attribute_77 = defense_input.active_entity.attribute_77;
    defense_input.attribute_7c = defense_input.active_entity.attribute_7c;
    const auto& party_records = field_party_roster_.records();
    for (std::size_t index = 0;
         index < defense_input.ready_entities.size(); ++index) {
        defense_input.ready_entities[index] =
            magic_action_record_from_runner(party_records[index]);
    }
    const auto defense =
        refresh_magic_menu_success_or_defense_values(defense_input);
    for (const auto& request : defense.grid_requests) {
        if (request.row >= command_overview_magic_metric_bars_.size()) {
            continue;
        }
        if (request.strip == MagicMenuValueStrip::defense_physical) {
            painter.drawImage(32, 128,
                              command_overview_magic_metric_bars_[request.row]);
        } else if (request.strip == MagicMenuValueStrip::defense_mana) {
            painter.drawImage(32, 144,
                              command_overview_magic_metric_bars_[request.row]);
        }
    }
    }

    // 0x1088C draws the spell's availability label and seven-tile row only
    // for nonnegative +0x25 values. The negative branch clears that region.
    const auto availability =
        lua_view != nullptr ? lua_view->availability
                            : spell != nullptr ? spell->availability_byte : -1;
    if (spell != nullptr && availability >= 0) {
        draw_field_menu_font_text(
            painter, 160, 80, field_menu_text_.magic_availability_label,
            magic_menu_font_);
        const auto availability_index =
            static_cast<std::size_t>(availability);
        if (availability_index <
            command_overview_magic_availability_bars_.size()) {
            painter.drawImage(
                160, 88,
                command_overview_magic_availability_bars_[availability_index]);
        }
    }

    // 0x105E6 writes `None` at C68C for nonpositive residuals, otherwise the
    // selected seven-tile C86F8 row at C688. The typed calculation consumes
    // the active runner and spell records; the extracted rows supply pixels.
    if (spell != nullptr && lua_view != nullptr) {
        if (lua_view->effect_value <= 0) {
            draw_field_menu_font_text(painter, 48, 104,
                                      field_menu_text_.none_label,
                                      magic_menu_font_);
        } else if (lua_view->effect_row >= 0 &&
                   static_cast<std::size_t>(lua_view->effect_row) <
                       command_overview_magic_damage_bars_.size()) {
            painter.drawImage(
                32, 104,
                command_overview_magic_damage_bars_[static_cast<std::size_t>(
                    lua_view->effect_row)]);
        }
    } else if (spell != nullptr) {
        const MagicActionEffectValueInput effect_input{
            command_overview_magic_force_.current_force,
            magic_action_record_from_runner(runner), *spell};
        const auto effect = refresh_magic_action_effect_value(effect_input);
        if (!effect.residual_valid || effect.residual_effect <= 0) {
            draw_field_menu_font_text(painter, 48, 104,
                                      field_menu_text_.none_label,
                                      magic_menu_font_);
        } else if (effect.damage_grid.has_value() &&
                   effect.damage_grid->effect_row <
                       command_overview_magic_damage_bars_.size()) {
            painter.drawImage(
                32, 104,
                command_overview_magic_damage_bars_[
                    effect.damage_grid->effect_row]);
        }
    }

    const auto posture = std::min<std::size_t>(
        runner.posture, command_overview_magic_posture_bars_.size() - 1U);
    painter.drawImage(160, 136,
                      command_overview_magic_posture_bars_[posture]);
    if (posture < field_menu_text_.status_names.size()) {
        draw_field_menu_font_text(
            painter, 160, 144, field_menu_text_.status_names[posture],
            magic_menu_font_);
    }

    // ROM 0x10172 lays out the endpoint, current-force fill, marker, one
    // separator cell, remaining fill, and final endpoint at C728. These are
    // direct source tile assets; only their positions depend on typed state.
    const auto maximum_force = std::max<std::uint16_t>(
        1U, command_overview_magic_force_.maximum_force);
    const auto current_force = std::clamp<std::uint16_t>(
        command_overview_magic_force_.current_force, 1U, maximum_force);
    const auto force_tile_x = [](const std::uint16_t index) {
        return 160 + static_cast<int>(index) * 8;
    };
    painter.drawImage(force_tile_x(0U), 112,
                      command_overview_magic_force_meter_tiles_[0]);
    for (std::uint16_t index = 1U; index < current_force; ++index) {
        painter.drawImage(force_tile_x(index), 112,
                          command_overview_magic_force_meter_tiles_[2]);
    }
    const auto marker = magic_force_selection_open() ? 3U : 4U;
    painter.drawImage(force_tile_x(current_force), 112,
                      command_overview_magic_force_meter_tiles_[marker]);
    for (std::uint16_t index = 0U;
         index < maximum_force - current_force; ++index) {
        painter.drawImage(
            force_tile_x(static_cast<std::uint16_t>(current_force + 2U + index)),
            112, command_overview_magic_force_meter_tiles_[2]);
    }
    painter.drawImage(force_tile_x(
                          static_cast<std::uint16_t>(maximum_force + 2U)),
                      112, command_overview_magic_force_meter_tiles_[1]);

    const auto selection = magic_spell_selection();
    const auto phase = static_cast<std::size_t>(
        magic_cursor_phase() %
        command_overview_magic_cursor_frames_.size());
    painter.drawImage(
        static_cast<int>(selection.column) * 32,
        static_cast<int>(selection.row) * 24,
        command_overview_magic_cursor_frames_[phase]);
}

void FieldSceneRuntime::paint_command_overview(QPainter& painter) const {
    if (!command_overview_.open()) {
        painter.fillRect(0, 0, 320, 224, Qt::black);
        return;
    }
    if (magic_spell_interface_open()) {
        painter.fillRect(0, 0, 320, 224, Qt::black);
        // The ROM cleared the previous command Plane A before writing Magic.
        // Transparent cells expose the cleared/black underlay; the HUD plane
        // remains a separate pass in paint().
        paint_command_overview_magic(painter);
        return;
    }
    if (command_overview_attributes_skills_.is_open()) {
        painter.fillRect(0, 0, 320, 224, Qt::black);
        paint_attributes_skills_menu(
            painter, command_overview_attributes_skills_.controller());
        return;
    }
    const auto routed_screen = static_cast<srx::MenuScreen>(
        menu_route_screen_);
    const bool matrix_search = field_matrix_route_active_ &&
        routed_screen == srx::MenuScreen::MATRIX_SEARCH;
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (field_taxi_route_active_ &&
        menu_route_screen_ == static_cast<std::uint16_t>(
                                 srx::MenuScreen::TAXI_SELECTOR)) {
        painter.fillRect(QRect(0, 0, 320, 224), Qt::black);
        if (field_taxi_map_.isNull()) {
            return;
        }
        painter.drawImage(kCabMapPlateScreenX, kCabMapPlateScreenY,
                          field_taxi_map_);
        const auto routed_menu = game_scripts_.menu_route_snapshot();
        const auto region = routed_menu.top_selection;
        const auto phase = routed_menu.top_page;
        if (!region.has_value() || *region < 0 || *region > 6 ||
            !phase.has_value() || *phase < 0 || *phase > 4) {
            source_boundary("field.taxi.selection_unresolved");
            return;
        }
        const auto plan = cab_mode_selection_plan(
            static_cast<std::uint8_t>(*region),
            static_cast<std::uint8_t>(*phase));
        constexpr int kTile = 8;
        const auto x = plan.cursor_x;
        const auto y = plan.cursor_y;
        const auto width = plan.cursor_width;
        const auto height = plan.cursor_height;
        painter.drawImage(x, y, field_taxi_cursor_tiles_[0]);
        painter.drawImage(x + width - kTile, y,
                          field_taxi_cursor_tiles_[2]);
        painter.drawImage(x, y + height - kTile,
                          field_taxi_cursor_tiles_[5]);
        painter.drawImage(x + width - kTile, y + height - kTile,
                          field_taxi_cursor_tiles_[7]);
        for (int edge = x + kTile; edge <= x + width - 2 * kTile;
             edge += kTile) {
            painter.drawImage(edge, y, field_taxi_cursor_tiles_[1]);
            painter.drawImage(edge, y + height - kTile,
                              field_taxi_cursor_tiles_[6]);
        }
        for (int edge = y + kTile; edge <= y + height - 2 * kTile;
             edge += kTile) {
            painter.drawImage(x, edge, field_taxi_cursor_tiles_[3]);
            painter.drawImage(x + width - kTile, edge,
                              field_taxi_cursor_tiles_[4]);
        }
        if (routed_menu.top_caption_visible.value_or(false)) {
            draw_tinted_field_text(painter, plan.caption_x,
                                   plan.caption_y, plan.caption,
                                   QColor(Qt::white));
        }
        return;
    }
    if (pocket_secretary_.open || field_matrix_route_active_) {
        // ROM 0xC2A6 writes C1418 at C216 with C0DA. The embedded 0x80
        // markers advance eight pixels per row, leaving one blank row
        // between options. C376 writes its right cursor tile at C3A2, x=136;
        // C1320 maps tile 0x05FF to the source lowercase-x glyph.
        if (matrix_search) {
            // The console is an overlay on the field; restore the map and HUD
            // outside the central frame after laying down its opaque art.
            paint_field_console_background(painter);
        } else {
            painter.drawImage(0, 0, pocket_secretary_background_);
            painter.drawImage(24, 24, pocket_secretary_grid_);
        }
        // ROM 0xE660's C13E8/C13F8 destinations map to these twelve 8x8
        // cells in the lower shared frame. Each call replaces one adjacent
        // 16x8 pair; all previous pairs remain until the panel is rebuilt.
        constexpr std::array<std::array<int, 2>, 12> kEffectCellOrigins{{
            {{40, 200}}, {{72, 200}}, {{184, 200}}, {{216, 200}},
            {{40, 208}}, {{72, 208}}, {{184, 208}}, {{216, 208}},
            {{56, 200}}, {{200, 200}}, {{56, 208}}, {{200, 208}},
        }};
        for (std::size_t location = 0;
             location < dismiss_runner_random_grid_cells_.size();
             ++location) {
            const auto pattern = dismiss_runner_random_grid_cells_[location];
            if (pattern < 0 ||
                static_cast<std::size_t>(pattern) >=
                    dismiss_runner_random_grid_patterns_.size()) {
                continue;
            }
            painter.drawImage(
                kEffectCellOrigins[location][0],
                kEffectCellOrigins[location][1],
                dismiss_runner_random_grid_patterns_[
                    static_cast<std::size_t>(pattern)]);
        }
        const auto routed_menu = game_scripts_.menu_route_snapshot();
        if (field_matrix_route_active_ &&
            routed_screen == srx::MenuScreen::MATRIX_ENTRY) {
            // 0xE47A leaves the shared interface while it plays the jack-in
            // cue and waits 20 VBlanks. Clear the whole frame so field/menu
            // controls cannot remain visible beneath the transition.
            painter.fillRect(QRect(0, 0, 320, 224), Qt::black);
            return;
        }
        if (field_matrix_route_active_ &&
            routed_screen == srx::MenuScreen::MATRIX_SEARCH) {
            // The source search loop writes the message inside the field
            // console, then repeatedly replaces its decimal permutation.
            std::array<std::string_view, 3> lines{};
            std::size_t line_count = 0U;
            std::size_t start = 0U;
            while (start <= routed_menu.top_text.size() && line_count < 3U) {
                const auto end = routed_menu.top_text.find('|', start);
                lines[line_count++] = std::string_view(routed_menu.top_text)
                                          .substr(
                                              start,
                                              end == std::string::npos
                                                  ? std::string::npos
                                                  : end - start);
                if (end == std::string::npos) break;
                start = end + 1U;
            }
            const auto draw_centered = [this, &painter](
                                           const int y,
                                           const std::string_view text) {
                const auto x = 136 -
                    static_cast<int>(text.size()) * 4;
                draw_command_overview_text(painter, x, y, text);
            };
            bool first_line_wrapped = false;
            if (line_count > 0U) {
                const auto split = lines[0].rfind(' ', 18U);
                if (split != std::string_view::npos) {
                    draw_centered(56, lines[0].substr(0U, split));
                    draw_centered(64, lines[0].substr(split + 1U));
                    first_line_wrapped = true;
                } else {
                    draw_centered(56, lines[0]);
                }
            }
            if (line_count > 1U) {
                draw_centered(first_line_wrapped ? 80 : 72, lines[1]);
            }
            if (line_count > 2U) {
                draw_centered(120, lines[2]);
            }
            return;
        }
        if (routed_screen == srx::MenuScreen::DEBUG_PICK_SYSTEM) {
            const QColor background(3, 12, 22);
            const QColor panel(7, 27, 39);
            const QColor edge(28, 117, 137);
            const QColor selector(196, 190, 39);
            painter.fillRect(QRect(0, 0, 320, 224), background);
            painter.fillRect(QRect(12, 12, 296, 200), panel);
            painter.setPen(edge);
            painter.drawRect(QRect(12, 12, 295, 199));
            draw_command_overview_text(
                painter, 24, 24,
                routed_menu.top_title.empty()
                    ? std::string_view("Pick System")
                    : std::string_view(routed_menu.top_title));
            painter.fillRect(QRect(20, 40, 280, 1), edge);
            draw_command_overview_text(painter, 96, 84, "System");
            painter.setPen(selector);
            painter.drawRect(QRect(148, 72, 41, 19));
            draw_command_overview_text(
                painter, 160, 84,
                routed_menu.top_text.empty()
                    ? std::string_view("00")
                    : std::string_view(routed_menu.top_text));
            return;
        }
        if (routed_screen == srx::MenuScreen::MATRIX_NODE_ACTION &&
            paint_matrix_traversal_view(
                painter, matrix_traversal_backgrounds_, matrix_world_models_,
                matrix_traversal_world_effect_modes_,
                matrix_traversal_world_effect_fallback_,
                matrix_traversal_hud_frame_,
                matrix_traversal_runner_entry_pose_,
                matrix_traversal_runner_action_phase_0_,
                matrix_traversal_runner_action_phase_1_,
                matrix_traversal_runner_idle_,
                matrix_traversal_analyze_rings_, matrix_map_font_,
                matrix_cyberdeck_program_icons_,
                matrix_cyberdeck_empty_program_icon_,
                matrix_cyberdeck_footer_slot_status_, routed_menu)) {
            return;
        }
        if (routed_screen == srx::MenuScreen::MATRIX_NODE_ACTION &&
            [&]() {
                const auto& status = routed_menu.top_matrix_node_status;
                const auto profile = static_cast<std::size_t>(
                    status.has_value()
                        ? std::clamp(status->background_profile, 0, 4)
                        : 0);
                const auto theme = static_cast<std::size_t>(
                    status.has_value()
                        ? std::clamp(status->palette_theme, 1, 4) - 1
                        : 0);
                return paint_matrix_node_action_panel(
                    painter, matrix_map_font_,
                    matrix_node_backgrounds_[profile][theme],
                    matrix_node_action_cursors_[theme], routed_menu);
            }()) {
            return;
        }
        if (routed_screen == srx::MenuScreen::DECK_MAP &&
            routed_menu.top_matrix_map_cells_present) {
            if (paint_matrix_system_map(
                    painter, matrix_map_background_, matrix_map_font_,
                    matrix_map_tile_atlases_, routed_menu)) {
                return;
            }
        }
        const bool cyberdeck_route =
            routed_screen == srx::MenuScreen::CYBERDECK ||
            routed_screen == srx::MenuScreen::PULL_PLUG ||
            routed_screen == srx::MenuScreen::DECK_PROGRAM ||
            routed_screen == srx::MenuScreen::DECK_CYBERINFO ||
            routed_screen == srx::MenuScreen::DECK_STORAGE ||
            routed_screen == srx::MenuScreen::DECK_MAP;
        if (cyberdeck_route) {
            const auto draw_matrix_text = [this, &painter](
                                              const int x, const int y,
                                              const std::string_view value) {
                auto current_x = x;
                for (const auto character : value) {
                    const auto code = static_cast<unsigned char>(character);
                    if (code < 0x20U || code > 0x7EU) {
                        current_x += 8;
                        continue;
                    }
                    const auto glyph = static_cast<int>(code - 0x20U);
                    painter.drawImage(
                        QPoint(current_x, y), matrix_map_font_,
                        QRect((glyph % 16) * 8, (glyph / 16) * 8, 8, 8));
                    current_x += 8;
                }
            };
            const auto draw_matrix_compact_text = [this, &painter](
                                                      const int x,
                                                      const int y,
                                                      const std::string_view value) {
                auto current_x = x;
                for (const auto character : value) {
                    const auto code = static_cast<unsigned char>(character);
                    if (code < 0x20U || code > 0x7EU) {
                        current_x += 5;
                        continue;
                    }
                    const auto glyph = static_cast<int>(code - 0x20U);
                    painter.drawImage(
                        QPoint(current_x, y), matrix_map_font_,
                        QRect((glyph % 16) * 8, (glyph / 16) * 8, 8, 8));
                    current_x += 5;
                }
            };
            const auto draw_matrix_rows = [&draw_matrix_text](
                                              const int x, const int y,
                                              const std::string& text,
                                              const int line_height,
                                              const int max_lines) {
                std::size_t start = 0U;
                int line = 0;
                while (start <= text.size() && line < max_lines) {
                    const auto end = text.find('|', start);
                    draw_matrix_text(
                        x, y + line * line_height,
                        std::string_view(text).substr(
                            start, end == std::string::npos
                                       ? std::string::npos
                                       : end - start));
                    ++line;
                    if (end == std::string::npos) break;
                    start = end + 1U;
                }
            };
            const auto draw_wrapped_matrix_description =
                [&draw_matrix_text](const int x, const int y,
                                    const std::string& text) {
                    constexpr std::size_t kColumns = 18U;
                    std::size_t start = 0U;
                    int line = 0;
                    while (start <= text.size() && line < 12) {
                        const auto end = text.find('|', start);
                        auto remaining = std::string_view(text).substr(
                            start, end == std::string::npos
                                       ? std::string::npos
                                       : end - start);
                        while (!remaining.empty() && line < 12) {
                            if (remaining.size() <= kColumns) {
                                draw_matrix_text(x, y + line * 8, remaining);
                                ++line;
                                break;
                            }
                            auto split = remaining.rfind(' ', kColumns);
                            if (split == std::string_view::npos || split == 0U) {
                                split = kColumns;
                            }
                            draw_matrix_text(x, y + line * 8,
                                             remaining.substr(0U, split));
                            remaining.remove_prefix(
                                split + (split < remaining.size() &&
                                                 remaining[split] == ' '
                                             ? 1U
                                             : 0U));
                            ++line;
                        }
                        if (end == std::string::npos) break;
                        start = end + 1U;
                        if (line < 12 && start == text.size()) {
                            ++line;
                        }
                    }
                };

            // 0x18282 writes D9C96 to Plane B and 0x182A6 adds the D8316 HUD
            // footer. These ROM streams are composed offline into a named
            // full-screen background; the host does not decode them here.
            const auto deck_palette_phase = static_cast<std::size_t>(
                std::clamp(
                    routed_menu.top_matrix_deck_palette_phase.value_or(0),
                    0, 3));
            painter.fillRect(QRect(0, 0, 320, 224), QColor(Qt::black));
            // The source-composed Plane-B art contains the opaque deck frame
            // and detail window. Keep the panel-grid underlay beneath it; the
            // source text, selected icon, and cursor are layered above both.
            painter.drawImage(0, 0, matrix_cyberdeck_panel_);
            painter.drawImage(
                0, 0, matrix_cyberdeck_backgrounds_[deck_palette_phase]);

            if (routed_screen == srx::MenuScreen::CYBERDECK ||
                routed_screen == srx::MenuScreen::PULL_PLUG ||
                routed_screen == srx::MenuScreen::DECK_CYBERINFO ||
                routed_screen == srx::MenuScreen::DECK_PROGRAM ||
                routed_screen == srx::MenuScreen::DECK_STORAGE) {
                // 0x17708 writes the five 3x2 installed/empty slot records
                // at y=184, then 0x19856 writes each 3-tile status strip at
                // y=208. The five slots begin at x=80 with 48-pixel spacing.
                // The ROM's 0x1822C pull-plug prompt redraws its text over the
                // existing deck screen without replacing this footer.
                if (routed_menu.top_matrix_deck_active_programs_present) {
                    for (std::size_t slot = 0U; slot < 5U; ++slot) {
                        const auto program = routed_menu
                                                 .top_matrix_deck_active_programs[
                                                     slot];
                        const auto& image = program >= 0 && program < 12
                            ? matrix_cyberdeck_program_icons_[
                                  static_cast<std::size_t>(program)]
                            : matrix_cyberdeck_empty_program_icon_;
                        painter.drawImage(
                            80 + static_cast<int>(slot) * 48, 184, image);
                    }
                }
                if (routed_menu
                        .top_matrix_deck_program_slot_status_present) {
                    for (std::size_t slot = 0U; slot < 5U; ++slot) {
                        const auto status = routed_menu
                                                .top_matrix_deck_program_slot_status[
                                                    slot];
                        if (status >=
                            matrix_cyberdeck_footer_slot_status_.size()) {
                            continue;
                        }
                        painter.drawImage(
                            80 + static_cast<int>(slot) * 48, 208,
                            matrix_cyberdeck_footer_slot_status_[status]);
                    }
                }

                // ROM 0x17708's D8316 footer already contains the silver
                // portrait in this 4x4 cell. Keep the ROM-composed background
                // art intact; the field-menu runner preview is unrelated art.
            }

            const auto column = std::clamp(
                routed_menu.top_column.value_or(0), 0, 3);
            const auto row = std::clamp(routed_menu.top_row.value_or(0), 0, 3);
            const auto selected = static_cast<std::size_t>(row * 4 + column);
            const bool has_deck_info =
                routed_menu.top_matrix_run_info.has_value() ||
                routed_menu.top_matrix_deck_stats.has_value();
            if (routed_screen == srx::MenuScreen::PULL_PLUG) {
                // The source enters 0x18450 from the selected grid cell and
                // leaves the 0x189D8 Command row, its icons, and the selected
                // Pull Plug cursor visible while the prompt is active.
                constexpr int kPullPlugGridRow = 3;
                constexpr int kPullPlugGridColumn = 3;
                painter.drawImage(
                    0, 0,
                    matrix_cyberdeck_selector_rows_[kPullPlugGridRow]
                                                       [deck_palette_phase]);
                for (std::size_t index = 0U;
                     index < matrix_cyberdeck_program_icons_.size(); ++index) {
                    if (index < 12U &&
                        (!routed_menu
                              .top_matrix_deck_program_ratings_present ||
                         routed_menu
                                 .top_matrix_deck_program_ratings[index] == 0U)) {
                        continue;
                    }
                    painter.drawImage(
                        16 + static_cast<int>(index % 4U) * 32,
                        24 + static_cast<int>(index / 4U) * 40,
                        matrix_cyberdeck_program_icons_[index]);
                }
                painter.drawImage(
                    12 + kPullPlugGridColumn * 32,
                    20 + kPullPlugGridRow * 40,
                    matrix_cyberdeck_cursors_[deck_palette_phase]);
            }
            const auto draw_retained_deck_grid = [&](const int retained_row) {
                const auto parent_row = std::clamp(retained_row, 0, 3);
                painter.drawImage(
                    0, 0,
                    matrix_cyberdeck_selector_rows_[
                        static_cast<std::size_t>(parent_row)]
                                                  [deck_palette_phase]);
                for (std::size_t index = 0U;
                     index < matrix_cyberdeck_program_icons_.size(); ++index) {
                    if (index < 12U &&
                        (!routed_menu
                              .top_matrix_deck_program_ratings_present ||
                         routed_menu
                                 .top_matrix_deck_program_ratings[index] == 0U)) {
                        continue;
                    }
                    painter.drawImage(
                        16 + static_cast<int>(index % 4U) * 32,
                        24 + static_cast<int>(index / 4U) * 40,
                        matrix_cyberdeck_program_icons_[index]);
                }
            };
            if (routed_screen == srx::MenuScreen::DECK_CYBERINFO) {
                // ROM 0x18AA6 rewrites only the right panel from $C126. The
                // selected deck row, program icons, and 0x17708 footer remain
                // displayed from the parent deck loop. FFF0D1 is still the
                // parent row (distinct from the CyberInfo cursor FFF0DC).
                draw_retained_deck_grid(
                    routed_menu.top_matrix_deck_parent_row.value_or(3));
            }
            if (routed_screen == srx::MenuScreen::DECK_PROGRAM) {
                // ROM 0x19238 calls 0x18998 to redraw the right panel at
                // $C628; the selected parent grid row and 0x17708 footer
                // remain visible while the loaded-program prompt is active.
                draw_retained_deck_grid(
                    routed_menu.top_matrix_deck_parent_row.value_or(0));
            }
            if (routed_screen == srx::MenuScreen::DECK_STORAGE) {
                // ROM 0x18D1A enters 0x1897C before drawing the storage panel
                // at $C126. The parent row, program grid, and 0x17708 footer
                // therefore remain visible behind this right-panel screen.
                draw_retained_deck_grid(
                    routed_menu.top_matrix_deck_parent_row.value_or(3));
            }
            if (routed_screen == srx::MenuScreen::CYBERDECK &&
                has_deck_info) {
                // The Run Info and Deck Stats branches return from 0x18AA6
                // after updating the right panel. Their call to 0x1897C
                // writes the panel region beginning at $C126; it does not
                // clear the parent program grid or its selected row.
                draw_retained_deck_grid(row);
            }
            if (routed_screen == srx::MenuScreen::CYBERDECK &&
                !has_deck_info) {
                // ROM 0x189D8 writes the selected row's 9-tile strip. The
                // icon grid is 3x2 tiles with 32x40 pixel cell spacing.
                painter.drawImage(
                    0, 0,
                    matrix_cyberdeck_selector_rows_[
                        static_cast<std::size_t>(row)][deck_palette_phase]);
                for (std::size_t index = 0U;
                     index < matrix_cyberdeck_program_icons_.size(); ++index) {
                    if (index < 12U &&
                        (!routed_menu
                              .top_matrix_deck_program_ratings_present ||
                         routed_menu
                                 .top_matrix_deck_program_ratings[index] == 0U)) {
                        continue;
                    }
                    painter.drawImage(
                        16 + static_cast<int>(index % 4U) * 32,
                        24 + static_cast<int>(index / 4U) * 40,
                        matrix_cyberdeck_program_icons_[index]);
                }

                const bool selected_program_available =
                    selected >= 12U ||
                    (routed_menu
                         .top_matrix_deck_program_ratings_present &&
                     routed_menu
                             .top_matrix_deck_program_ratings[selected] != 0U);
                if (routed_menu.top_text == "No available slots!" ||
                    routed_menu.top_text == "No available memory!") {
                    // ROM 0x19238 writes the first line from 0xE383D at
                    // $C628 and the second line from 0xE384A/0xE3851 at
                    // $C6A8.
                    draw_matrix_text(160, 96, "No available");
                    draw_matrix_text(
                        160, 104,
                        routed_menu.top_text == "No available slots!"
                            ? "slots!"
                            : "memory!");
                } else if (!routed_menu.top_text.empty()) {
                    draw_matrix_rows(176, 64, routed_menu.top_text, 8, 14);
                } else if (selected_program_available) {
                    painter.drawImage(
                        184, 16, matrix_cyberdeck_program_icons_[selected]);
                    if (selected < 12U) {
                        if (selected < routed_menu.top_option_labels.size()) {
                            draw_matrix_compact_text(
                                216, 24,
                                routed_menu.top_option_labels[selected]);
                            if (routed_menu
                                    .top_matrix_deck_program_ratings_present) {
                                draw_matrix_text(
                                    264, 24,
                                    std::to_string(routed_menu
                                                       .top_matrix_deck_program_ratings[
                                                           selected]));
                            }
                        }
                        if (routed_menu
                                .top_matrix_deck_program_detail_values_present) {
                            constexpr std::array<int, 3> kValueX{{272, 272,
                                                                 256}};
                            for (std::size_t line = 0U;
                                 line < routed_menu
                                            .top_matrix_deck_program_detail_values
                                            .size();
                                 ++line) {
                                draw_matrix_text(
                                    kValueX[line],
                                    40 + static_cast<int>(line) * 8,
                                    routed_menu
                                        .top_matrix_deck_program_detail_values[
                                            line]);
                            }
                        }
                    } else if (selected < routed_menu.top_option_labels.size()) {
                        draw_matrix_compact_text(
                            216, 24,
                            routed_menu.top_option_labels[selected]);
                    }
                    if (selected < 12U) {
                        if (routed_menu
                                .top_matrix_deck_detail_labels_present) {
                            for (std::size_t line = 0U;
                                 line < routed_menu
                                           .top_matrix_deck_detail_labels.size();
                                 ++line) {
                                draw_matrix_text(
                                    152, 40 + static_cast<int>(line) * 8,
                                    routed_menu
                                        .top_matrix_deck_detail_labels[line]);
                            }
                        }
                        if (routed_menu
                                .top_matrix_deck_runner_detail_labels_present) {
                            for (std::size_t line = 0U;
                                 line < routed_menu
                                            .top_matrix_deck_runner_detail_labels
                                            .size();
                                 ++line) {
                                draw_matrix_compact_text(
                                    152, 72 + static_cast<int>(line) * 8,
                                    routed_menu
                                        .top_matrix_deck_runner_detail_labels[
                                            line]);
                            }
                        }
                        if (routed_menu
                                .top_matrix_deck_runner_details_present) {
                            for (std::size_t line = 0U;
                                 line < routed_menu
                                           .top_matrix_deck_runner_details.size();
                                 ++line) {
                                draw_matrix_text(
                                    208, 72 + static_cast<int>(line) * 8,
                                    routed_menu
                                        .top_matrix_deck_runner_details[line]);
                            }
                        }
                    }
                    draw_wrapped_matrix_description(
                        160, 96,
                        routed_menu.top_matrix_deck_description);
                } else {
                    draw_matrix_rows(176, 64,
                                     std::string("  Empty|Utility Slot"),
                                     8, 2);
                }
                if (routed_menu
                        .top_matrix_storage_empty_message_lines.size() ==
                    2U) {
                    // ROM 0xE3A6C writes at $C52C and advances by 0x80.
                    draw_matrix_text(
                        176, 80,
                        routed_menu
                            .top_matrix_storage_empty_message_lines[0]);
                    draw_matrix_text(
                        176, 88,
                        routed_menu
                            .top_matrix_storage_empty_message_lines[1]);
                }
                painter.drawImage(12 + column * 32, 20 + row * 40,
                                  matrix_cyberdeck_cursors_[deck_palette_phase]);
                return;
            }

            if (routed_screen == srx::MenuScreen::PULL_PLUG) {
                // ROM 0x1846E writes the optional Black IC warning at Plane A
                // $C228 (x=160,y=32); 0x18480 writes the question at $C426
                // (x=152,y=64); and 0x18492 writes Yes/No at $C638
                // (x=224,y=96), with 16-pixel row spacing. 0xC376 places the
                // green text cursor at $C632 (x=200,y=96+16*FFF0DC).
                std::string prompt = routed_menu.top_title;
                const auto warning_separator = prompt.find(
                    "|Do you really want|");
                if (prompt.compare(0U, 8U, "WARNING:") == 0 &&
                    warning_separator != std::string::npos) {
                    draw_matrix_rows(
                        160, 32,
                        prompt.substr(0U, warning_separator), 8, 2);
                    prompt.erase(0U, warning_separator + 1U);
                }
                draw_matrix_rows(152, 64, prompt, 8, 2);
                for (std::size_t option = 0U;
                     option < routed_menu.top_option_labels.size() &&
                     option < 2U;
                     ++option) {
                    const auto y = 96 + static_cast<int>(option) * 16;
                    if (routed_menu.top_row == static_cast<int>(option)) {
                        draw_matrix_text(200, y, ">");
                    }
                    draw_matrix_text(224, y,
                                     routed_menu.top_option_labels[option]);
                }
                if (paint_matrix_logout_transition_art(painter,
                                                       routed_menu)) {
                    return;
                }
                if (routed_menu.top_matrix_transition_fade_step.has_value()) {
                    const auto fade_step = std::clamp(
                        *routed_menu.top_matrix_transition_fade_step, 0, 5);
                    painter.fillRect(
                        painter.viewport(),
                        QColor(0, 0, 0, fade_step * 51));
                }
                return;
            }

            if (routed_screen == srx::MenuScreen::DECK_PROGRAM) {
                // ROM 0x19238 reads the prompt at 0xE3859 into $C628 (the
                // embedded 0x80 advances to its second line), then streams
                // 0xE3823, 0xE382E, and 0xE387E into $C72E, $C7AA, and $C8BA.
                // The text origins are (160,96/104), (184,112), (168,120),
                // and (232,136/152). FFF0DC starts at 1, so No is selected.
                const auto program = routed_menu.top_program_index.value_or(-1);
                if (program >= 0 &&
                    static_cast<std::size_t>(program) <
                        matrix_cyberdeck_program_icons_.size()) {
                    painter.drawImage(
                        184, 16,
                        matrix_cyberdeck_program_icons_[
                            static_cast<std::size_t>(program)]);
                    if (routed_menu
                            .top_matrix_deck_parent_program_name_present) {
                        draw_matrix_compact_text(
                            216, 24,
                            routed_menu.top_matrix_deck_parent_program_name);
                    }
                    if (program < 12 &&
                        routed_menu
                            .top_matrix_deck_program_ratings_present) {
                        draw_matrix_text(
                            264, 24,
                            std::to_string(routed_menu
                                               .top_matrix_deck_program_ratings[
                                                   static_cast<std::size_t>(program)]));
                    }
                    if (routed_menu
                            .top_matrix_deck_detail_labels_present) {
                        for (std::size_t line = 0U;
                             line < routed_menu
                                        .top_matrix_deck_detail_labels.size();
                             ++line) {
                            draw_matrix_text(
                                152, 40 + static_cast<int>(line) * 8,
                                routed_menu
                                    .top_matrix_deck_detail_labels[line]);
                        }
                    }
                    if (routed_menu
                            .top_matrix_deck_program_detail_values_present) {
                        constexpr std::array<int, 3> kValueX{{272, 272, 256}};
                        for (std::size_t line = 0U;
                             line < routed_menu
                                        .top_matrix_deck_program_detail_values
                                        .size();
                             ++line) {
                            draw_matrix_text(
                                kValueX[line],
                                40 + static_cast<int>(line) * 8,
                                routed_menu
                                    .top_matrix_deck_program_detail_values[line]);
                        }
                    }
                    if (routed_menu
                            .top_matrix_deck_runner_detail_labels_present) {
                        for (std::size_t line = 0U;
                             line < routed_menu
                                        .top_matrix_deck_runner_detail_labels
                                        .size();
                             ++line) {
                            draw_matrix_compact_text(
                                152, 72 + static_cast<int>(line) * 8,
                                routed_menu
                                    .top_matrix_deck_runner_detail_labels[line]);
                        }
                    }
                    if (routed_menu
                            .top_matrix_deck_runner_details_present) {
                        for (std::size_t line = 0U;
                             line < routed_menu
                                        .top_matrix_deck_runner_details.size();
                             ++line) {
                            draw_matrix_text(
                                208, 72 + static_cast<int>(line) * 8,
                                routed_menu
                                    .top_matrix_deck_runner_details[line]);
                        }
                    }
                }
                draw_matrix_text(160, 96, "Program is already");
                draw_matrix_text(160, 104, "loaded in memory.");
                draw_matrix_text(184, 112, "Erase from");
                draw_matrix_text(168, 120, "active memory?");
                for (std::size_t option = 0U;
                     option < routed_menu.top_option_labels.size() &&
                     option < 2U;
                     ++option) {
                    const auto y = 136 + static_cast<int>(option) * 16;
                    if (routed_menu.top_row == static_cast<int>(option)) {
                        draw_matrix_text(200, y, ">");
                    }
                    draw_matrix_text(232, y,
                                     routed_menu.top_option_labels[option]);
                }
                return;
            }

            if (routed_screen == srx::MenuScreen::DECK_STORAGE) {
                // ROM 0x18D22 streams the panel rows at $C126, $C1A6, $C226,
                // $C2A6, $C3A6, and $C426. The blank row before DataFiles is
                // retained as a gap in these explicit native coordinates.
                constexpr std::array<int, 6> kStoragePanelY{{
                    16, 24, 32, 40, 56, 64}};
                if (routed_menu.top_matrix_storage_panel_lines_present) {
                    for (std::size_t line = 0U;
                         line < kStoragePanelY.size(); ++line) {
                        draw_matrix_text(
                            152, kStoragePanelY[line],
                            routed_menu.top_matrix_storage_panel_lines[line]);
                    }
                }
                const auto draw_storage_value = [&draw_matrix_text](
                                                    const int y,
                                                    const int value) {
                    const auto text = std::to_string(value);
                    const auto x = 272 -
                        static_cast<int>(text.size() - 1U) * 8;
                    draw_matrix_text(x, y, text);
                };
                if (routed_menu.top_matrix_storage_total.has_value()) {
                    draw_storage_value(
                        32, *routed_menu.top_matrix_storage_total);
                }
                if (routed_menu.top_matrix_storage_available.has_value()) {
                    draw_storage_value(
                        40, *routed_menu.top_matrix_storage_available);
                }
                if (routed_menu.top_matrix_storage_phase == "view") {
                    const auto file_count = std::min(
                        routed_menu.top_matrix_storage_file_names.size(),
                        routed_menu.top_matrix_storage_file_sizes.size());
                    for (std::size_t file = 0U; file < file_count; ++file) {
                        const auto row_y = 72 + static_cast<int>(file) * 16;
                        draw_matrix_text(
                            152, row_y,
                            routed_menu.top_matrix_storage_file_names[file]);
                        draw_matrix_text(
                            152, row_y + 8,
                            routed_menu.top_matrix_storage_separator);
                        draw_storage_value(
                            row_y + 8,
                            routed_menu.top_matrix_storage_file_sizes[file]);
                    }
                    return;
                }
                if (routed_menu.top_matrix_storage_phase == "erase") {
                    if (routed_menu
                            .top_matrix_storage_erase_name_present) {
                        draw_matrix_text(
                            152, 80,
                            routed_menu.top_matrix_storage_erase_name);
                    }
                    draw_matrix_text(
                        152, 88,
                        routed_menu.top_matrix_storage_separator);
                    if (routed_menu.top_matrix_storage_erase_size.has_value()) {
                        draw_storage_value(
                            88,
                            *routed_menu.top_matrix_storage_erase_size);
                    }
                    for (std::size_t option = 0U;
                         option < routed_menu.top_option_labels.size() &&
                         option < 2U;
                         ++option) {
                        const auto y = 104 + static_cast<int>(option) * 16;
                        if (routed_menu.top_row ==
                            static_cast<int>(option)) {
                            draw_matrix_text(200, y, ">");
                        }
                        draw_matrix_text(
                            216, y,
                            routed_menu.top_option_labels[option]);
                    }
                    return;
                }
                for (std::size_t option = 0U;
                     option < routed_menu.top_option_labels.size() &&
                     option < 2U;
                     ++option) {
                    const auto y = 80 + static_cast<int>(option) * 16;
                    if (routed_menu.top_row == static_cast<int>(option)) {
                        draw_matrix_text(200, y, ">");
                    }
                    draw_matrix_text(
                        216, y, routed_menu.top_option_labels[option]);
                }
                return;
            }

            if (routed_menu.top_matrix_run_info.has_value()) {
                (void)paint_matrix_run_info_text(
                    painter, matrix_map_font_, routed_menu);
                return;
            }
            if (routed_menu.top_matrix_deck_stats.has_value()) {
                (void)paint_matrix_deck_stats_text(
                    painter, matrix_map_font_, routed_menu);
                return;
            }
            if (!routed_menu.top_text.empty()) {
                const bool map_lines =
                    routed_screen == srx::MenuScreen::DECK_MAP &&
                    std::count(routed_menu.top_text.begin(),
                               routed_menu.top_text.end(), '|') == 9;
                draw_matrix_rows(map_lines ? 104 : 176,
                                 map_lines ? 52 : 64,
                                 routed_menu.top_text,
                                 map_lines ? 12 : 8,
                                 map_lines ? 10 : 16);
                return;
            }

            const bool cyber_info_selector =
                routed_screen == srx::MenuScreen::DECK_CYBERINFO;
            if (cyber_info_selector) {
                // ROM 0x18AB0 reads 0xE3886 at $C32C. Its 0x80 row
                // control places '[' plus the 12-glyph rule at y=56. The
                // following controls put Deck Stats and Run Info at y=72/88.
                draw_matrix_text(176, 56,
                                 std::string("[") + std::string(12U, '\\'));
            }
            if (!routed_menu.top_title.empty()) {
                draw_matrix_text(176, 48, routed_menu.top_title);
            }
            for (std::size_t option = 0U;
                 option < routed_menu.top_option_labels.size() && option < 8U;
                 ++option) {
                const auto y = (cyber_info_selector ? 72 : 80) +
                               static_cast<int>(option) * 16;
                if (routed_menu.top_row == static_cast<int>(option)) {
                    draw_matrix_text(cyber_info_selector ? 176 : 160,
                                     y, "> ");
                }
                draw_matrix_text(176, y,
                                 routed_menu.top_option_labels[option]);
            }
            return;
        }
        const bool save_load_screen =
            menu_route_screen_ == static_cast<std::uint16_t>(
                                      srx::MenuScreen::SAVE_LOAD) ||
            menu_route_screen_ == static_cast<std::uint16_t>(
                                      srx::MenuScreen::SAVE_SLOT);
        const bool save_load_message =
            menu_route_screen_ == static_cast<std::uint16_t>(
                                      srx::MenuScreen::MESSAGE) &&
            !routed_menu.top_title.empty();
        if (save_load_screen || save_load_message) {
            // Replace the Pocket Secretary rows with a clean child panel.
            painter.drawImage(24, 24, pocket_secretary_grid_);
            if (save_load_message) {
                std::size_t line_start = 0U;
                int line = 0;
                while (line_start <= routed_menu.top_title.size() && line < 4) {
                    const auto line_end = routed_menu.top_title.find('|', line_start);
                    const auto text = routed_menu.top_title.substr(
                        line_start,
                        line_end == std::string::npos
                            ? std::string::npos
                            : line_end - line_start);
                    draw_command_overview_text(
                        painter, 40, 72 + line * 8, text);
                    ++line;
                    if (line_end == std::string::npos) break;
                    line_start = line_end + 1U;
                }
                return;
            }
            if (!routed_menu.top_title.empty()) {
                draw_command_overview_text(
                    painter, 88, 32, routed_menu.top_title);
                draw_command_overview_text(
                    painter, 72, 40,
                    pocket_secretary_menu_content_.separator);
            }
            for (std::size_t row = 0;
                 row < routed_menu.top_option_labels.size(); ++row) {
                draw_command_overview_text(
                    painter, 88, 56 + static_cast<int>(row) * 16,
                    routed_menu.top_option_labels[row]);
            }
            const auto selected_row = routed_menu.top_row.value_or(0);
            if (selected_row >= 0 &&
                static_cast<std::size_t>(selected_row) <
                    routed_menu.top_option_labels.size()) {
                draw_command_overview_text(
                    painter, 72, 56 + selected_row * 16, "x");
            }
            return;
        }
        if (menu_route_screen_ == static_cast<std::uint16_t>(
                                      srx::MenuScreen::DEBUG_MENU)) {
            const auto debug_menu = game_scripts_.menu_route_snapshot();
            if (!debug_menu.top_title.empty()) {
                draw_command_overview_text(
                    painter, 88, 32, debug_menu.top_title);
                draw_command_overview_text(
                    painter, 72, 40,
                    pocket_secretary_menu_content_.separator);
            }
            for (std::size_t row = 0;
                 row < debug_menu.top_option_labels.size(); ++row) {
                draw_command_overview_text(
                    painter, 88, 56 + static_cast<int>(row) * 16,
                    debug_menu.top_option_labels[row]);
            }
            const auto selected_row = debug_menu.top_row.value_or(0);
            if (selected_row >= 0 &&
                static_cast<std::size_t>(selected_row) <
                    debug_menu.top_option_labels.size()) {
                draw_command_overview_text(
                    painter, 72, 56 + selected_row * 16, "x");
            }
            return;
        }
        if (pocket_secretary_.notebook.open) {
            const auto draw_notebook_record_page =
                [this, &painter](
                    const NotebookMenuState::RecordPageState& page,
                    const std::size_t record_count,
                    const std::size_t title_index,
                    const auto& names,
                    const auto& details) {
                    if (!page.open) return false;
                    draw_command_overview_text(
                        painter, 88, 32,
                        field_menu_text_.notebook_labels[title_index]);
                    draw_command_overview_text(
                        painter, 72, 40,
                        field_menu_text_.notebook_tips_separator);
                    if (page.detail_wait) {
                        // ROM 0xC38A clears the inner record area before
                        // drawing the chosen source description. This removes
                        // every list row and selector during the wait.
                        painter.drawImage(QRect(24, 48, 224, 136),
                                          pocket_secretary_grid_,
                                          QRect(0, 24, 224, 136));
                        if (page.selection < page.visible_count) {
                            const auto source_index =
                                page.visible_source_indices[page.selection];
                            if (source_index < record_count &&
                                source_index < details.size()) {
                                draw_command_overview_text(
                                    painter, 48, 56,
                                    details[source_index]);
                            }
                        }
                        return true;
                    }
                    for (std::size_t row = 0; row < page.visible_count; ++row) {
                        const auto source_index =
                            page.visible_source_indices[row];
                        const auto y = 56 + static_cast<int>(row) * 16;
                        if (source_index < record_count &&
                            source_index < names.size()) {
                            draw_command_overview_text(
                                painter, 88, y, names[source_index]);
                        } else if (source_index == record_count) {
                            draw_command_overview_text(
                                painter, 88, y,
                                field_menu_text_
                                    .notebook_record_page_labels[0]);
                        } else if (source_index == record_count + 1U) {
                            draw_command_overview_text(
                                painter, 88, y,
                                field_menu_text_
                                    .notebook_record_page_labels[1]);
                        }
                    }
                    if (page.selection < page.visible_count) {
                        draw_command_overview_text(
                            painter, 72,
                            56 + static_cast<int>(page.selection) * 16, "x");
                    }
                    return true;
                };
            if (draw_notebook_record_page(
                    pocket_secretary_.notebook.contacts,
                    kNotebookContactCount, 3U,
                    field_menu_text_.notebook_contact_names,
                    field_menu_text_.notebook_contact_details)) {
                return;
            }
            if (draw_notebook_record_page(
                    pocket_secretary_.notebook.shadowrunners,
                    kNotebookShadowrunnerCount, 5U,
                    field_menu_text_.notebook_shadowrunner_names,
                    field_menu_text_.notebook_shadowrunner_details)) {
                return;
            }
            if (pocket_secretary_.notebook.mr_johnsons.open) {
                const auto& page =
                    pocket_secretary_.notebook.mr_johnsons;
                draw_command_overview_text(
                    painter, 88, 32, field_menu_text_.notebook_labels[4]);
                draw_command_overview_text(
                    painter, 72, 40,
                    field_menu_text_.notebook_tips_separator);
                if (page.detail_wait) {
                    // ROM F404 writes the 28x17 lower inner-panel grid at
                    // C306 before C0DA places the selected detail at C38C.
                    // This clears the old list and cursor while preserving
                    // the title/separator and shared outer frame.
                    painter.drawImage(QRect(24, 48, 224, 136),
                                      pocket_secretary_grid_,
                                      QRect(0, 24, 224, 136));
                    if (page.selection < page.visible_count) {
                        const auto source_index =
                            page.visible_source_indices[page.selection];
                        if (source_index <
                            field_menu_text_.notebook_mr_johnson_details
                                .size()) {
                            draw_command_overview_text(
                                painter, 48, 56,
                                field_menu_text_
                                    .notebook_mr_johnson_details[source_index]);
                        }
                    }
                    return;
                }
                for (std::size_t row = 0; row < page.visible_count; ++row) {
                    const auto source_index =
                        page.visible_source_indices[row];
                    if (source_index >=
                        field_menu_text_.notebook_mr_johnson_names.size()) {
                        continue;
                    }
                    const auto y = 56 + static_cast<int>(row) * 16;
                    draw_command_overview_text(
                        painter, 88, y,
                        field_menu_text_
                            .notebook_mr_johnson_names[source_index]);
                }
                if (page.selection < page.visible_count) {
                    draw_command_overview_text(
                        painter, 72,
                        56 + static_cast<int>(page.selection) * 16, "x");
                }
                return;
            }
            if (pocket_secretary_.notebook.tips.open) {
                const auto& tips = pocket_secretary_.notebook.tips;
                draw_command_overview_text(
                    painter, 88, 32, field_menu_text_.notebook_tips_title);
                draw_command_overview_text(
                    painter, 72, 40,
                    field_menu_text_.notebook_tips_separator);
                draw_command_overview_text(
                    painter, 136, 184,
                    field_menu_text_.notebook_tips_page_count_label);
                if (tips.visible_count != 0U &&
                    tips.selection < tips.visible_count) {
                    const auto source_index =
                        tips.visible_source_indices[tips.selection];
                    draw_command_overview_text(
                        painter, 40, 56,
                        field_menu_text_.notebook_tips[source_index]);
                }
                const auto draw_right_aligned_decimal =
                    [this, &painter](const int right_edge,
                                     const int y,
                                     const std::size_t value) {
                        draw_command_overview_number(
                            painter, command_overview_font_, right_edge + 8,
                            y, static_cast<std::uint32_t>(value),
                            NumericVisualAlignment::right);
                    };
                draw_right_aligned_decimal(
                    184, 184,
                    static_cast<std::size_t>(tips.selection) + 1U);
                draw_right_aligned_decimal(232, 184, tips.visible_count);
                return;
            }
            if (pocket_secretary_.notebook.current_run.open) {
                const auto& current_run =
                    pocket_secretary_.notebook.current_run;
                painter.drawImage(QRect(24, 48, 224, 136),
                                  pocket_secretary_grid_,
                                  QRect(0, 24, 224, 136));
                if (current_run.payment_amount >= 0 &&
                    !current_run.johnson_label.empty() &&
                    !current_run.johnson_name.empty() &&
                    !current_run.run_type_label.empty() &&
                    !current_run.run_type_name.empty() &&
                    !current_run.payment_label.empty() &&
                    !current_run.currency_symbol.empty()) {
                    // ROM 0x1EE64 places these values into the source text
                    // layout at C208, C21A, C29A, and C322. The host uses the
                    // same 8-pixel cells and draws the saved amount rightward
                    // from the source's least-significant-digit position.
                    draw_command_overview_text(
                        painter, 32, 32, current_run.johnson_label);
                    draw_command_overview_text(
                        painter, 104, 32, current_run.johnson_name);
                    draw_command_overview_text(
                        painter, 56, 40, current_run.run_type_label);
                    draw_command_overview_text(
                        painter, 104, 40, current_run.run_type_name);
                    draw_command_overview_text(
                        painter, 64, 48, current_run.payment_label);
                    draw_command_overview_number(
                        painter, command_overview_font_, 136, 48,
                        static_cast<std::uint32_t>(
                            current_run.payment_amount),
                        NumericVisualAlignment::right, true);
                    draw_command_overview_text(
                        painter, 144, 48, current_run.currency_symbol);
                    for (std::size_t line = 0;
                         line < current_run.body_lines.size(); ++line) {
                        draw_command_overview_text(
                            painter, 40,
                            64 + static_cast<int>(line) * 8,
                            current_run.body_lines[line]);
                    }
                }
                return;
            }
            if (pocket_secretary_.notebook.no_current_run_wait) {
                for (std::size_t line = 0;
                     line < field_menu_text_.notebook_no_run_lines.size();
                     ++line) {
                    draw_command_overview_text(
                        painter, 48, 48 + static_cast<int>(line) * 8,
                        field_menu_text_.notebook_no_run_lines[line]);
                }
                return;
            }
            // ROM 0xF33E clears through C3E6, writes C567E at C216, and
            // writes its separator at C292. C0DA advances 0x80 source bytes
            // (one eight-pixel row) at each marker. Current Run and Tips &
            // Clues therefore start at y=56 and y=72; F33E places each
            // enabled event-gated row another 16 pixels below the previous
            // row. F236's C376 cursor base is C392, eight pixels left of the
            // first option, and uses the same 16-pixel row stride.
            draw_command_overview_text(
                painter, 88, 32, field_menu_text_.notebook_labels[0]);
            draw_command_overview_text(
                painter, 72, 40, pocket_secretary_menu_content_.separator);
            for (std::size_t row = 0;
                 row < pocket_secretary_.notebook.option_count; ++row) {
                const auto label_index = static_cast<std::size_t>(
                    pocket_secretary_.notebook.visible_options[row]) + 1U;
                if (label_index >= field_menu_text_.notebook_labels.size()) {
                    continue;
                }
                const auto y = 56 + static_cast<int>(row) * 16;
                draw_command_overview_text(
                    painter, 88, y,
                    field_menu_text_.notebook_labels[label_index]);
            }
            if (pocket_secretary_.notebook.selection <
                pocket_secretary_.notebook.option_count) {
                draw_command_overview_text(
                    painter, 72,
                    56 + static_cast<int>(
                             pocket_secretary_.notebook.selection) * 16,
                    "x");
            }
            return;
        }
        if (dismiss_runner_.open) {
            // ROM C448 writes its fixed C1499/C14A4/Separator/C14BD block
            // through C0DA at C216. Abort remains at C416 (y=64); each active
            // FF0200/FF0300 name is written one row below it, at y=80+16n.
            // Redrawing the cleared shared inner grid first prevents the old
            // Pocket Secretary buttons from remaining beneath this child.
            draw_command_overview_text(
                painter, 88, 32, field_menu_text_.dismiss_runner_labels[0]);
            draw_command_overview_text(
                painter, 88, 40, field_menu_text_.dismiss_runner_labels[1]);
            draw_command_overview_text(
                painter, 72, 48, field_menu_text_.dismiss_runner_separator);
            draw_command_overview_text(
                painter, 88, 64, field_menu_text_.dismiss_runner_labels[2]);
            for (std::size_t row = 0;
                 row < dismiss_runner_.candidate_count; ++row) {
                const auto slot = dismiss_runner_.candidate_slots[row];
                draw_command_overview_text(
                    painter, 88, 80 + static_cast<int>(row) * 16,
                    field_party_roster_.name(slot));
            }
            if (dismiss_runner_.selection <=
                dismiss_runner_.candidate_count) {
                draw_command_overview_text(
                    painter, 72,
                    64 + static_cast<int>(dismiss_runner_.selection) * 16,
                    "x");
            }
            if (const auto slot = dismiss_runner_preview_slot();
                slot.has_value() &&
                *slot < field_party_roster_.records().size()) {
                const auto& record = field_party_roster_.records()[*slot];
                constexpr std::uint16_t kPaletteLineMask = 0x6000U;
                const auto panel_attributes =
                    record.command_overview_panel_parameter;
                // ROM 0x110EC maps +0x3E values through <=0, ==1, and >=2
                // layout branches. All three select a portrait-panel origin;
                // recruited records commonly carry 3 or 4, so larger values
                // must remain valid here.
                if ((panel_attributes &
                            static_cast<std::uint16_t>(~kPaletteLineMask)) !=
                           0U) {
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.preview_attributes_unresolved");
                } else if (record.hud_portrait_table_pointer == 0U) {
                    source_boundary(
                        "field.pocket_secretary.dismiss_runner.preview_table_unresolved");
                } else {
                    const auto* preview =
                        runner_preview_panel_for_record(record);
                    if (preview == nullptr) {
                        source_boundary(
                            "field.pocket_secretary.dismiss_runner.preview_asset_unavailable");
                    } else {
                        painter.drawImage(kRunnerPreviewPanelX,
                                          kRunnerPreviewPanelY, *preview);
                    }
                }
            }
            return;
        }
        if (pocket_secretary_.cyberdeck_unavailable_wait) {
            // 0x18150 calls C3E6 before writing this response at C40E, which
            // replaces the parent menu plane contents until the wait ends.
            for (std::size_t line = 0;
                 line < field_menu_text_
                           .pocket_secretary_cyberdeck_unavailable_lines.size();
                 ++line) {
                draw_command_overview_text(
                    painter, 56, 64 + static_cast<int>(line) * 8,
                    field_menu_text_
                        .pocket_secretary_cyberdeck_unavailable_lines[line]);
            }
            return;
        }
        if (pocket_secretary_.group_items_wait) {
            if (pocket_secretary_.group_item_flags == 0U) {
                for (std::size_t line = 0;
                     line < field_menu_text_
                                .pocket_secretary_group_items_empty_lines.size();
                     ++line) {
                    draw_command_overview_text(
                        painter, 56, 80 + static_cast<int>(line) * 8,
                        field_menu_text_
                            .pocket_secretary_group_items_empty_lines[line]);
                }
            } else {
                for (std::size_t row = 0;
                     row < pocket_secretary_.group_item_display_count; ++row) {
                    const auto item_index =
                        pocket_secretary_.group_item_display_order[row];
                    if (item_index >= field_menu_text_
                                          .pocket_secretary_group_item_names
                                          .size()) {
                        continue;
                    }
                    draw_command_overview_text(
                        painter, 32, 32 + static_cast<int>(row) * 16,
                        field_menu_text_
                            .pocket_secretary_group_item_names[item_index]);
                }
            }
            return;
        }
        draw_command_overview_text(
            painter, 88, 32, pocket_secretary_menu_content_.title);
        draw_command_overview_text(
            painter, 88, 40, pocket_secretary_menu_content_.separator);
        for (std::size_t row = 0;
             row < pocket_secretary_menu_content_.choices.size(); ++row) {
            draw_command_overview_text(
                painter, 88, 56 + static_cast<int>(row) * 16,
                pocket_secretary_menu_content_.choices[row].label);
        }
        if (pocket_secretary_.selection <=
            pocket_secretary_.maximum_selection) {
            // ROM 0xC376 writes the selector into plane-B column 9 (x=72),
            // in the gutter before the option labels begin at x=88.
            draw_command_overview_text(
                painter, 72,
                56 + static_cast<int>(pocket_secretary_.selection) * 16,
                "x");
        }
        return;
    }

    painter.drawImage(0, 0, command_overview_background_);
    painter.drawImage(0, 0, command_overview_inventory_boxes_);

    // ROM 0x110BC writes the active runner's 8x10 portrait at E104, which is
    // the fixed 16,16 host location. The source template selection, rather
    // than the field slot number, chooses Joshua's portrait.
    if (field_joshua_archetype_selection_ <
            command_overview_portraits_.size() &&
        !command_overview_portraits_[field_joshua_archetype_selection_]
             .isNull()) {
        painter.drawImage(
            16, 16,
            command_overview_portraits_[field_joshua_archetype_selection_]);
    }

    draw_command_overview_text(
        painter, 128, 8,
        field_party_roster_.name(field_party_roster_.active_slot()).empty()
            ? field_menu_text_.runner_names[0]
            : field_party_roster_.name(field_party_roster_.active_slot()));

    // 0x11928/0x11AE4 places the five-state bar and status name. Keeping this
    // in a helper lets child screens restore the shared panel after clearing
    // their content region.
    paint_command_overview_posture_panel(painter);

    const auto draw_item_detail_rows =
        [this, &painter](const std::uint8_t item_code,
                         const std::optional<std::size_t> selected_slot,
                         const bool include_art) {
            if (item_code == 0U || item_code > field_menu_text_.items.size()) {
                return;
            }
            const auto& item = field_menu_text_.items[
                static_cast<std::size_t>(item_code - 1U)].definition;
            if (item.code != item_code) {
                return;
            }
            if (include_art && selected_slot.has_value() &&
                command_overview_item_detail_.returned_item_code == item_code &&
                command_overview_item_detail_.item_art_written) {
                painter.drawImage(144, 24,
                                  command_overview_item_details_[item_code - 1U]);
            }

            // The Lua item owns the firearm's named Damage row and Power
            // profile; loaded rounds remain live actor state in the HUD.
            if (item.weapon.has_value()) {
                const auto& weapon = *item.weapon;
                const auto damage_index = static_cast<std::size_t>(
                    weapon.damage_bar_index);
                if (damage_index < command_overview_damage_bars_.size()) {
                    draw_command_overview_text(painter, 112, 80,
                                                field_menu_text_.damage_label);
                    painter.drawImage(168, 80,
                                      command_overview_damage_bars_[damage_index]);
                }
                if (!weapon.power_label.empty()) {
                    draw_command_overview_text(painter, 112, 88,
                                                field_menu_text_.power_label);
                    draw_command_overview_text(painter, 168, 88,
                                               weapon.power_label);
                }
            }

            if (!include_art || !selected_slot.has_value()) {
                return;
            }
            std::optional<std::size_t> label_index;
            if (item_code < 0x0DU) {
                label_index = 0U;
            } else if (item_code < 0x14U) {
                label_index = 1U;
            } else if (item_code == 0x14U ||
                       (item_code >= 0x18U && item_code < 0x29U)) {
                label_index = 2U;
            }
            if (!label_index.has_value()) {
                return;
            }
            draw_command_overview_text(
                painter, 112, 48,
                field_menu_text_.item_detail_labels[*label_index]);
            const auto phase = active_runner_record()
                                   .inventory_phase_bytes[*selected_slot];
            if (phase < command_overview_detail_values_.size()) {
                painter.drawImage(120, 56,
                                  command_overview_detail_values_[phase]);
            }
        };
    const auto draw_armor_summary = [this, &painter](
                                        const std::uint8_t armor_index) {
        if (armor_index != 0U && armor_index <= field_menu_text_.armor_labels.size()) {
            const auto& armor = field_menu_text_.armor_labels[armor_index - 1U];
            for (std::size_t row = 0; row < armor.size(); ++row) {
                if (!armor[row].empty()) {
                    draw_command_overview_text(
                        painter, 128, 128 + static_cast<int>(row * 8U),
                        armor[row]);
                }
            }
        } else {
            draw_command_overview_text(painter, 152, 136,
                                       field_menu_text_.no_armor_label);
        }
        draw_command_overview_text(painter, 136, 144,
                                   field_menu_text_.weapons_label);
        draw_command_overview_text(painter, 136, 152,
                                   field_menu_text_.guns_label);
        const scripting::ScriptArmorTemplate* armor = nullptr;
        for (const auto& item : field_menu_text_.items) {
            if (item.definition.armor.has_value() &&
                item.definition.armor->armor_index == armor_index) {
                armor = &*item.definition.armor;
                break;
            }
        }
        if (armor == nullptr) {
            return;
        }
        if (armor->secondary_modifier <
            command_overview_armor_summary_bars_.size()) {
            painter.drawImage(176, 144,
                              command_overview_armor_summary_bars_[
                                  armor->secondary_modifier]);
        }
        if (armor->primary_modifier <
            command_overview_armor_summary_bars_.size()) {
            painter.drawImage(176, 152,
                              command_overview_armor_summary_bars_[
                                  armor->primary_modifier]);
        }
    };

    const auto& view = command_overview_.view();
    if (view.source_state.primary_page >= 2U &&
        !command_overview_clips_.is_open()) {
        const auto race = active_runner_record().species_index;
        const auto character_class = active_runner_record().archetype_index;
        if (race < field_menu_text_.race_names.size()) {
            draw_command_overview_text(
                painter, 128, 24, field_menu_text_.race_names[race]);
        }
        if (character_class < field_menu_text_.class_names.size()) {
            draw_command_overview_text(
                painter, 176, 24,
                field_menu_text_.class_names[character_class]);
        }
        draw_command_overview_text(painter, 128, 40,
                                   field_menu_text_.physical_label);
        draw_command_overview_text(painter, 128, 48,
                                   field_menu_text_.mental_label);
        draw_command_overview_text(
            painter, 208, 40,
            std::to_string(std::min<unsigned>(
                100U, static_cast<unsigned>(
                          active_runner_record().delay_meter) * 10U)) +
                "%");
        draw_command_overview_text(
            painter, 208, 48,
            std::to_string(std::min<unsigned>(
                100U, static_cast<unsigned>(
                          active_runner_record().behavior_meter) * 10U)) +
                "%");
        const auto equipped_item_code =
            active_runner_record().equipped_item_code;
        const bool special_spell_action =
            active_runner_record().equipped_inventory_slot < 0 &&
            equipped_item_code > 0U &&
            static_cast<std::size_t>(equipped_item_code) <=
                field_menu_text_.magic_spell_names.size();
        if (special_spell_action) {
            // ROM 0x111E8 indexes D4EA0 when record +0x56 is negative. The
            // same +0x57 value indexes D4540 only for a nonnegative slot.
            const auto spell_code = static_cast<std::size_t>(
                equipped_item_code - 1U);
            draw_command_overview_text(
                painter, 128, 64,
                field_menu_text_.magic_spell_names[spell_code]);
        } else if (equipped_item_code > 0U &&
                   equipped_item_code <= field_menu_text_.items.size()) {
            // Ordinary equipment uses the one-based D4540 item row.
            const auto item_index =
                static_cast<std::size_t>(equipped_item_code - 1U);
            draw_command_overview_text(
                painter, 128, 64, field_menu_text_.items[item_index].name);
            if (!field_menu_text_.items[item_index].category.empty()) {
                draw_command_overview_text(
                    painter, 128, 72,
                    field_menu_text_.items[item_index].category);
            }
        }
        if (special_spell_action) {
            const auto* spell = find_us_magic_spell(
                static_cast<std::uint8_t>(equipped_item_code - 1U));
            if (spell != nullptr && spell->availability_byte >= 0) {
                // ROM 0x12EAE takes the signed D4EA0 +0x25 row through the
                // ordinary Damage strip, while 0x13000 selects the Power
                // label from current force +0x9D. Negative source rows exit
                // before these two detail cells are written.
                draw_command_overview_text(
                    painter, 112, 80, field_menu_text_.damage_label);
                const auto damage_row = static_cast<std::size_t>(
                    spell->availability_byte);
                if (damage_row < command_overview_damage_bars_.size()) {
                    painter.drawImage(
                        168, 80,
                        command_overview_damage_bars_[damage_row]);
                }
                const MagicActionEffectValueInput effect_input{
                    active_runner_record().magic_force,
                    magic_action_record_from_runner(active_runner_record()),
                    *spell};
                const auto effect =
                    refresh_magic_action_effect_value(effect_input);
                if (effect.residual_valid && effect.residual_effect <= 0) {
                    draw_command_overview_text(
                        painter, 168, 80, field_menu_text_.none_label);
                }
                draw_command_overview_text(
                    painter, 112, 88, field_menu_text_.power_label);
                if (!field_menu_text_.power_labels.empty()) {
                    const auto force_row = std::min<std::size_t>(
                        active_runner_record().magic_force == 0U
                            ? 0U
                            : active_runner_record().magic_force - 1U,
                        field_menu_text_.power_labels.size() - 1U);
                    draw_command_overview_text(
                        painter, 168, 88,
                        field_menu_text_.power_labels[force_row]);
                }
            }
        } else {
            draw_item_detail_rows(equipped_item_code, std::nullopt, false);
        }
        draw_command_overview_text(painter, 112, 104,
                                   field_menu_text_.attack_label);
        draw_command_overview_text(painter, 112, 112,
                                   field_menu_text_.defense_label);
        if (command_overview_combat_bars_.attack_written &&
            command_overview_combat_bars_.attack_bar <
                command_overview_attack_bars_.size()) {
            painter.drawImage(
                144, 104,
                command_overview_attack_bars_[
                    command_overview_combat_bars_.attack_bar]);
        }
        if (command_overview_combat_bars_.defense_written &&
            command_overview_combat_bars_.defense_bar <
                command_overview_defense_bars_.size()) {
            painter.drawImage(
                144, 112,
                command_overview_defense_bars_[
                    command_overview_combat_bars_.defense_bar]);
        }
        draw_armor_summary(active_runner_record().equipped_armor_index);
    } else if (!command_overview_clips_.is_open()) {
        const auto slot = view.source_state.secondary_page * 2U +
                          view.source_state.primary_page;
        if (slot < active_runner_record().inventory_item_codes.size()) {
            const auto item_code =
                active_runner_record().inventory_item_codes[slot];
            if (item_code >= 1U && item_code <= field_menu_text_.items.size()) {
                draw_command_overview_text(
                    painter, 128, 64,
                    field_menu_text_.items[item_code - 1U].name);
                draw_command_overview_text(
                    painter, 128, 72,
                    field_menu_text_.items[item_code - 1U].category);
                draw_item_detail_rows(item_code, slot, true);
                // ROM 0x12A8C sends selected armor to 0x11352. It previews
                // the selected D4D50 row in the lower armor summary at
                // C820/C922/C92C/C9AC, then restores the equipped record.
                // Repaint that panel from typed state rather than placing a
                // second Wpns/Guns pair beside the selected-item text.
                const auto& selected_item = field_menu_text_.items[
                    static_cast<std::size_t>(item_code - 1U)].definition;
                if (selected_item.armor.has_value()) {
                    draw_armor_summary(
                        selected_item.armor->armor_index);
                }
            }
        }
    }

    // The equipment-cell blocks are opaque name-table records installed by
    // 0x11624 before the inventory item glyph rows. Paint them under the
    // direct item sprites so the selected/equipped frame cannot erase its
    // icon.
    const auto draw_equipment_border = [this, &painter](const std::size_t slot) {
        if (slot >= command_overview_selected_cells_.size() * 4U) {
            return;
        }
        const auto overlay = ((slot / 2U) & 1U) == 0U ? 0U : 1U;
        painter.drawImage(
            8 + static_cast<int>((slot % 2U) * 40U),
            112 + static_cast<int>((slot / 2U) * 24U),
            command_overview_selected_cells_[overlay]);
    };
    if (active_runner_record().equipped_inventory_slot >= 0) {
        draw_equipment_border(static_cast<std::size_t>(
            active_runner_record().equipped_inventory_slot));
    }
    if (active_runner_record().equipped_armor_index != 0U) {
        const auto armor_code = static_cast<std::uint8_t>(
            0x2BU + active_runner_record().equipped_armor_index);
        for (std::size_t slot = 0;
             slot < active_runner_record().inventory_item_codes.size();
             ++slot) {
            if (active_runner_record().inventory_item_codes[slot] ==
                armor_code) {
                draw_equipment_border(slot);
                break;
            }
        }
    }

    // The source inventory is an 8-cell, two-column/four-row grid. Direct
    // item aliases are already palette-decoded; no Genesis VRAM or palette
    // operation is reproduced here.
    constexpr std::array<std::array<int, 2>, 8> kInventoryIconPositions{{
        {{16, 120}}, {{56, 120}}, {{16, 144}}, {{56, 144}},
        {{16, 168}}, {{56, 168}}, {{16, 192}}, {{56, 192}},
    }};
    const auto icon_variant = command_overview_uses_alternate_icon_palette(
                                  active_runner_record())
                                  ? 1U
                                  : 0U;
    for (std::size_t slot = 0;
         slot < active_runner_record().inventory_item_codes.size();
         ++slot) {
        const auto item_code = active_runner_record().inventory_item_codes[
            slot];
        if (item_code == 0U || item_code > 50U) {
            continue;
        }
        const auto& icon =
            command_overview_inventory_icons_[item_code - 1U][icon_variant];
        painter.drawImage(kInventoryIconPositions[slot][0],
                          kInventoryIconPositions[slot][1], icon);
    }

    if (view.source_state.primary_page < 2U &&
        !command_overview_inventory_selector_suppressed_) {
        // ROM 0x11592 builds the cursor SAT entry from the 0xD29F6 list:
        // x = FFF0D0*0x28 + 0x008C, y = FFF0D1*0x18 + 0x00F4. The Genesis
        // SAT origin adds 0x80 on both axes. The 24x16 item graphic starts
        // four pixels inside that 32x24 sprite footprint. Section 06 frames
        // 2580, 2610, and 2640 show a one-pixel warm-gold outline hugging the
        // item graphic; outlining the larger SAT footprint leaves the broad
        // rectangular margin that is absent from those captures.
        const auto x = 16 + static_cast<int>(
                               view.source_state.primary_page * 40U);
        const auto y = 120 + static_cast<int>(
                                view.source_state.secondary_page * 24U);
        constexpr int kOutlineWidth = 24;
        constexpr int kOutlineHeight = 16;
        const QColor selector_color(196, 190, 39);
        painter.fillRect(x, y, kOutlineWidth, 1, selector_color);
        painter.fillRect(x, y + kOutlineHeight - 1,
                         kOutlineWidth, 1, selector_color);
        painter.fillRect(x, y, 1, kOutlineHeight, selector_color);
        painter.fillRect(x + kOutlineWidth - 1, y, 1, kOutlineHeight,
                         selector_color);
    }

    if (command_overview_cyberware_active_) {
        // ROM 0x11950 rebuilds the center name-plane region before writing
        // C1A2/C328/C31C. Keep the command screen's separate portrait,
        // inventory, posture, tab, and right-HUD layers in the composition;
        // replace only the central text panel so old status text cannot show
        // through the Cyberware rows.
        painter.drawImage(96, 16, command_overview_background_, 96, 16,
                          168, 144);
        draw_command_overview_text(
            painter, 136, 24, field_menu_text_.cyberware_title_lines[0]);
        draw_command_overview_text(
            painter, 136, 32, field_menu_text_.cyberware_title_lines[1]);
        if (command_overview_cyberware_.shows_no_cyberware_message) {
            draw_command_overview_text(
                painter, 160, 48, field_menu_text_.cyberware_empty_message);
        } else {
            for (std::size_t row = 0;
                 row < command_overview_cyberware_.installed_row_indices.size();
                 ++row) {
                const auto bit = command_overview_cyberware_
                                     .installed_row_indices[row];
                if (bit >= field_menu_text_.cyberware_entries.size()) {
                    continue;
                }
                draw_command_overview_text(
                    painter, 112, 48 + static_cast<int>(row) * 16,
                    field_menu_text_.cyberware_entries[bit]);
            }
        }
    }
    const bool clips_owns_button_panel = command_overview_clips_.is_open();
    if (clips_owns_button_panel) {
        paint_command_overview_clips(painter);
    } else if (!command_overview_inventory_action_active_ &&
        view.source_state.primary_page >= 2U &&
        view.source_state.primary_page <= 6U) {
        const auto focus_index = static_cast<std::size_t>(
            view.source_state.primary_page - 2U);
        painter.drawImage(128 + static_cast<int>(focus_index * 24U), 184,
                          command_overview_focus_[focus_index]);
    }
    // Each child menu owns this button panel exclusively. In particular, do
    // not repaint an old inventory-action row after Clips has replaced it.
    if (!clips_owns_button_panel &&
        command_overview_inventory_action_active_) {
        if (command_overview_discard_confirmation_active_) {
            painter.drawImage(
                kCommandOverviewActionBackgroundX,
                kCommandOverviewActionBackgroundY,
                command_overview_action_confirmation_background_);
            const auto choice_index = static_cast<std::size_t>(
                command_overview_discard_confirmation_.selected_option == 0U
                    ? 0U
                    : 1U);
            painter.drawImage(
                kCommandOverviewActionConfirmationFocusX[choice_index],
                kCommandOverviewActionConfirmationFocusY,
                command_overview_action_confirmation_focus_[choice_index]);
            // 0x126A2 writes D1CD3 at Plane A C7A0 (128,120).
            draw_command_overview_text(
                painter, 128, 120, field_menu_text_.action_discard_prompt);
            if (command_overview_selected_inventory_item_.has_value() &&
                active_runner_record().equipped_inventory_slot >= 0 &&
                static_cast<std::size_t>(
                    active_runner_record().equipped_inventory_slot) ==
                    command_overview_selected_inventory_item_->slot) {
                // 0x126AE writes D1CBB at C620; its 0x80 line break separates
                // the warning at y=96 from the suffix at y=104.
                draw_command_overview_text(
                    painter, 128, 96,
                    field_menu_text_.action_item_equipped_prefix);
                draw_command_overview_text(
                    painter, 128, 104,
                    field_menu_text_.action_item_equipped_suffix);
            }
            draw_command_overview_text(
                painter, 112, 200,
                centered_command_footer(
                    command_overview_discard_confirmation_
                                .selected_option == 0U
                        ? field_menu_text_.action_choice_yes
                        : field_menu_text_.action_choice_no));
        } else {
            painter.drawImage(kCommandOverviewActionBackgroundX,
                              kCommandOverviewActionBackgroundY,
                              command_overview_action_background_);
            const auto action_selector =
                command_overview_inventory_action_.action_selector;
            if (action_selector >= 2U && action_selector <= 5U) {
                const auto focus_index = static_cast<std::size_t>(
                    action_selector - 2U);
                painter.drawImage(kCommandOverviewActionFocusX[focus_index],
                                  kCommandOverviewActionFocusY,
                                  command_overview_action_focus_[focus_index]);
            }
            // ROM 0x117D0/0x11712 shows the Inventory footer for the item
            // action row. First Aid changes it only after entering its child.
            const auto inventory_page = view.source_state.primary_page;
            if (inventory_page < field_menu_text_.command_labels.size()) {
                draw_command_overview_text(
                    painter, 112, 200,
                    centered_command_footer(
                        field_menu_text_.command_labels[inventory_page]));
            }
        }
    } else if (!clips_owns_button_panel &&
               command_overview_inventory_trade_.open) {
        painter.drawImage(kCommandOverviewActionBackgroundX,
                          kCommandOverviewActionBackgroundY,
                          command_overview_action_confirmation_background_);
        const auto choice_index = command_overview_inventory_trade_.yes_selected
            ? 0U
            : 1U;
        painter.drawImage(
            kCommandOverviewActionConfirmationFocusX[choice_index],
            kCommandOverviewActionConfirmationFocusY,
            command_overview_action_confirmation_focus_[choice_index]);
        if (command_overview_inventory_trade_.recipient_inventory_full) {
            const auto recipient =
                command_overview_inventory_trade_.recipient_slot;
            if (recipient < FieldPartyRosterRuntime::kSlotCount) {
                draw_command_overview_text(
                    painter, 120, 104, field_party_roster_.name(recipient));
            }
            draw_command_overview_text(
                painter, 120, 112,
                field_menu_text_.action_trade_full_inventory_message);
        } else {
            draw_command_overview_text(
                painter, 152, 120,
                field_menu_text_.action_trade_recipient_label);
            const auto recipient =
                command_overview_inventory_trade_.recipient_slot;
            if (recipient < FieldPartyRosterRuntime::kSlotCount) {
                draw_command_overview_text(
                    painter, 128, 128, field_party_roster_.name(recipient));
                const auto& recipient_record =
                    field_party_roster_.records()[recipient];
                const auto& panel_attributes =
                    recipient_record.command_overview_panel_parameter;
                constexpr std::uint16_t kPaletteLineMask = 0x6000U;
                if ((panel_attributes &
                     static_cast<std::uint16_t>(~kPaletteLineMask)) != 0U) {
                    source_boundary(
                        "field.inventory.trade.preview_attributes_unresolved");
                } else if (recipient_record.hud_portrait_table_pointer == 0U) {
                    source_boundary(
                        "field.inventory.trade.preview_table_unresolved");
                } else if (const auto* preview =
                               runner_preview_panel_for_record(
                                   recipient_record);
                           preview == nullptr) {
                    source_boundary(
                        "field.inventory.trade.preview_asset_unavailable");
                } else {
                    // ROM 0x1287E reaches 0x110E8 for a recipient with room;
                    // the same typed 32x32 extracted panel used by Dismiss
                    // Runner is projected at the source HUD coordinate.
                    painter.drawImage(kRunnerPreviewPanelX,
                                      kRunnerPreviewPanelY, *preview);
                }
            }
            if (command_overview_selected_inventory_item_.has_value() &&
                active_runner_record().equipped_inventory_slot >= 0 &&
                static_cast<std::size_t>(
                    active_runner_record().equipped_inventory_slot) ==
                    command_overview_selected_inventory_item_->slot) {
                draw_command_overview_text(
                    painter, 128, 96,
                    field_menu_text_.action_item_equipped_prefix);
                draw_command_overview_text(
                    painter, 128, 104,
                    field_menu_text_.action_item_equipped_suffix);
            }
        }
        draw_command_overview_text(
            painter, 112, 200,
            centered_command_footer(
                command_overview_inventory_trade_.yes_selected
                    ? field_menu_text_.action_choice_yes
                    : field_menu_text_.action_choice_no));
    } else if (!clips_owns_button_panel &&
               command_overview_action_message_ !=
               CommandOverviewActionMessage::none) {
        if (command_overview_action_message_ ==
            CommandOverviewActionMessage::cannot_discard) {
            draw_command_overview_text(
                painter, 128, 96, field_menu_text_.action_cannot_discard);
        } else if (command_overview_action_message_ ==
                   CommandOverviewActionMessage::cannot_trade) {
            draw_command_overview_text(
                painter, 136, 96, field_menu_text_.action_cannot_trade);
        }
    } else if (!clips_owns_button_panel &&
               command_overview_first_aid_state_ ==
                   CommandOverviewFirstAidState::inactive &&
               view.source_state.primary_page <
               field_menu_text_.command_labels.size()) {
        draw_command_overview_text(
            painter, 112, 200,
            centered_command_footer(
                field_menu_text_.command_labels[
                    view.source_state.primary_page]));
    }

    if (command_overview_magic_state_ ==
        CommandOverviewMagicState::magical_defense_wait) {
        // ROM 0xFBA6 rebuilds the common menu row (0x11900), clears the
        // central menu grid (0x12C36), then writes C89C0 at C31C and the two
        // 13-tile strips at C51E/C69E. Replace the Qt panel's previous status
        // text/details with its named blank background before drawing this
        // screen; otherwise model, weapon, and Atk/Def content leaks through
        // the Magical Defense title and bars.
        constexpr QRect kMagicDefenseContentRect{96, 16, 168, 144};
        painter.drawImage(kMagicDefenseContentRect,
                          command_overview_background_,
                          kMagicDefenseContentRect);
        painter.fillRect(112, 48,
                         static_cast<int>(
                             field_menu_text_.magic_defense_title.size()) * 8,
                         8, Qt::black);
        draw_command_overview_text(painter, 112, 48,
                                   field_menu_text_.magic_defense_title);
        const auto draw_strip = [this, &painter](
                                    const MagicDefenseStripView& strip,
                                    const int y) {
            if (strip.bar_row < command_overview_defense_bars_.size()) {
                painter.drawImage(
                    120, y, command_overview_defense_bars_[strip.bar_row]);
            }
        };
        draw_strip(command_overview_magic_physical_defense_, 80);
        draw_strip(command_overview_magic_mana_defense_, 104);
    }

    if (command_overview_first_aid_state_ !=
        CommandOverviewFirstAidState::inactive) {
        if (command_overview_first_aid_state_ ==
            CommandOverviewFirstAidState::target_selection) {
            draw_command_overview_text(
                painter, 128, 88, field_menu_text_.first_aid_prompt);
            for (std::size_t row = 0;
                 row < command_overview_first_aid_targets_.active_count;
                 ++row) {
                const auto record_index = command_overview_first_aid_targets_
                                              .active_record_indices[row];
                if (record_index < FieldPartyRosterRuntime::kSlotCount) {
                    draw_command_overview_text(
                        painter, 128, 104 + static_cast<int>(row) * 8,
                        field_party_roster_.name(record_index));
                }
            }
            draw_command_overview_text(
                painter, 152,
                104 + static_cast<int>(
                          command_overview_first_aid_targets_.active_count) * 8,
                field_menu_text_.first_aid_cancel);
            const auto selected_row = command_overview_first_aid_cursor_;
            const auto cursor_x = selected_row ==
                                          command_overview_first_aid_targets_
                                              .cancel_row()
                                      ? 144
                                      : 120;
            draw_command_overview_text(
                painter, cursor_x,
                104 + static_cast<int>(selected_row) * 8, ">");
        } else if (command_overview_first_aid_message_ ==
                   FirstAidMessage::medkit_used) {
            draw_command_overview_text(
                painter, 144, 104,
                field_menu_text_.first_aid_medkit_used);
        } else if (command_overview_first_aid_message_ ==
                   FirstAidMessage::slap_patch_used) {
            draw_command_overview_text(
                painter, 144, 104,
                field_menu_text_.first_aid_slap_patch_used);
        } else if (command_overview_first_aid_message_ ==
                   FirstAidMessage::target_not_injured) {
            draw_command_overview_text(
                painter, 104, 104,
                field_menu_text_.first_aid_not_injured);
        }

        if (command_overview_first_aid_cursor_ <
            command_overview_first_aid_targets_.active_count) {
            const auto target_index = command_overview_first_aid_targets_
                                          .active_record_indices[
                                              command_overview_first_aid_cursor_];
            // This First Aid child panel currently projects the selected
            // Joshua face. The separate three-slot HUD asset families do not
            // imply that recruited target portraits are wired into this page.
            if (target_index == 0U &&
                field_joshua_archetype_selection_ <
                    field_hud_faces_.size() &&
                !field_hud_faces_[field_joshua_archetype_selection_].isNull()) {
                painter.drawImage(
                    144, 144,
                    field_hud_faces_[field_joshua_archetype_selection_]);
            }
        }
        draw_command_overview_text(
            painter, 104, 200, field_menu_text_.first_aid_label);
    }
    if (command_overview_automatic_use_state_ ==
        CommandOverviewAutomaticUseState::result_wait) {
        draw_command_overview_text(
            painter, 128, 96,
            field_menu_text_.action_automatic_use_message);
    }
}

void FieldSceneRuntime::paint(QPainter& target_painter) const {
    if (!ready_ || field_map_.isNull()) {
        target_painter.fillRect(0, 0, 320, 224, Qt::black);
        return;
    }

    QImage scene(320, 224, QImage::Format_ARGB32_Premultiplied);
    scene.fill(Qt::black);
    QPainter scene_painter(&scene);
    auto& painter = scene_painter;
    const auto apply_palette_fade = [this, &scene, &scene_painter,
                                     &target_painter] {
        std::uint8_t palette_passes = 0U;
        bool toward_black = true;
        bool transition_active = false;
        if (tarislar_credits_title_return_.is_presenting()) {
            palette_passes =
                tarislar_credits_title_return_.palette_passes_to_black();
            transition_active = palette_passes != 0U;
        } else if (field_door_return_active_) {
            palette_passes = field_door_return_palette_level();
            transition_active = palette_passes != 0U;
        } else
        switch (field_elevator_scene_transition_.phase()) {
        case ElevatorSceneTransitionPhase::fading_to_black:
            palette_passes = std::min<std::uint8_t>(
                field_elevator_scene_transition_.palette_step(), 7U);
            transition_active = true;
            break;
        case ElevatorSceneTransitionPhase::destination_handoff:
            palette_passes = 7U;
            transition_active = true;
            break;
        case ElevatorSceneTransitionPhase::fading_from_black:
            palette_passes = std::min<std::uint8_t>(
                field_elevator_scene_transition_.palette_step(), 7U);
            toward_black = false;
            transition_active = true;
            break;
        case ElevatorSceneTransitionPhase::idle:
            if (field_palette_fade_steps_ != 0U) {
                // 0x16396 uses 0x1088 for the interaction fade. The modal
                // return counter runs in the opposite direction; preserve
                // its existing host duration while applying one CRAM level
                // per source palette step and saturating after seven levels.
                const auto completed_steps =
                    field_modal_return_transition_frames_remaining_ != 0U
                        ? static_cast<std::uint8_t>(
                              9U - std::min<std::uint8_t>(
                                       field_palette_fade_steps_, 9U))
                        : field_palette_fade_steps_;
                palette_passes = std::min<std::uint8_t>(completed_steps, 7U);
                toward_black =
                    field_modal_return_transition_frames_remaining_ == 0U;
                transition_active = true;
            }
            break;
        }

        scene_painter.end();
        if (hotel_menu_.open()) {
            field_door_return_modal_frame_ = scene;
        } else if (field_door_return_active_ &&
                   field_door_return_frame_ < checkout_fade_in_start_frame_ &&
                   !field_door_return_modal_frame_.isNull()) {
            // The checkout page itself fades out (0x131C) before the reload.
            scene = field_door_return_modal_frame_;
        }
        if (tarislar_credits_title_return_.is_presenting() &&
            field_dialogue_.has_retained_credits_page_for_title_return()) {
            QPainter credits_painter(&scene);
            field_dialogue_.paint_retained_credits_page_for_title_return(
                credits_painter);
            credits_painter.end();
        }
        if (transition_active) {
            scene = transition_genesis_cram_image(
                scene, palette_passes, toward_black);
        }
        target_painter.drawImage(0, 0, scene);
    };
    if (field_elevator_selector_.is_open()) {
        const auto animation_frame = static_cast<std::size_t>(
            field_elevator_selector_.animation_frame());
        const auto current_floor = static_cast<std::size_t>(
            field_elevator_selector_.current_floor());
        const auto selected_floor = static_cast<std::size_t>(
            field_elevator_selector_.selected_floor());
        painter.drawImage(
            0, 0, field_elevator_selector_backgrounds_[animation_frame]);
        switch (field_elevator_selector_.navigation_feedback()) {
        case ElevatorSelectorNavigationFeedback::previous_floor_active:
            painter.drawImage(0, 0,
                              field_elevator_selector_feedback_layers_[0U]);
            break;
        case ElevatorSelectorNavigationFeedback::previous_floor_rest:
            painter.drawImage(0, 0,
                              field_elevator_selector_feedback_layers_[1U]);
            break;
        case ElevatorSelectorNavigationFeedback::next_floor_active:
            painter.drawImage(0, 0,
                              field_elevator_selector_feedback_layers_[2U]);
            break;
        case ElevatorSelectorNavigationFeedback::next_floor_rest:
            painter.drawImage(0, 0,
                              field_elevator_selector_feedback_layers_[3U]);
            break;
        case ElevatorSelectorNavigationFeedback::none:
            break;
        }
        // The ROM writes the arrow feedback after the floor name-table
        // updates, but the selector labels occupy the higher-priority Plane-B
        // layer. Keep both digits visible above the temporary rail graphics.
        painter.drawImage(
            0, 0, field_elevator_selector_current_floor_layers_[current_floor]);
        painter.drawImage(
            0, 0,
            field_elevator_selector_selected_floor_layers_[selected_floor]);
        painter.drawImage(0, 0, field_hud_);
        paint_field_hud_overlays(painter);
        apply_palette_fade();
        return;
    }
    if (field_interaction_.open()) {
        field_interaction_.paint(painter);
        apply_palette_fade();
        return;
    }
    if (hotel_karma_skills_.is_open()) {
        if (hotel_karma_load_frames_ > 0U) {
            // The Karma child loads with the display black (Lua timing).
            painter.fillRect(QRect(0, 0, 320, 224), Qt::black);
            apply_palette_fade();
            return;
        }
        paint_attributes_skills_menu(painter,
                                     hotel_karma_skills_.controller());
        apply_palette_fade();
        return;
    }
    if (hotel_menu_.open()) {
        hotel_menu_.paint(
            painter, pocket_secretary_background_, pocket_secretary_grid_,
            // The motel's CFFE palette load leaves CRAM line 2 green, so
            // the checkout prompt and its cursor match the options ink here
            // [C: emulator run4 frame 8320; the purple line-2 assets belong
            // to other menu palettes].
            hotel_options_cursor_,
            hotel_options_cursor_, dismiss_runner_random_grid_patterns_,
            hotel_menu_content_,
            [this](QPainter& target, const int x, const int y,
                   const std::string_view text,
                   const HotelMenuPaletteLine) {
                draw_hotel_menu_text(
                    target, x, y, text,
                    HotelMenuPaletteLine::status_and_options);
            },
            [this](QPainter& target) {
                // Window-plane HUD column (x >= 272) as in the field.
                target.save();
                target.setClipRect(QRect(272, 0, 48, 224));
                target.drawImage(0, 0, field_hud_);
                paint_field_hud_overlays(target);
                target.restore();
            });
        apply_palette_fade();
        return;
    }
    if (field_mode_text_.open()) {
        field_mode_text_.paint(painter);
        apply_palette_fade();
        return;
    }
    if (field_shop_notice_active_) {
        paint_field_shop_notice(painter);
        apply_palette_fade();
        return;
    }
    if (field_shop_menu_.is_open()) {
        paint_field_shop(painter);
        apply_palette_fade();
        return;
    }
    if (field_dialogue_.open()) {
        if (field_dialogue_.uses_hospital_menu_layout()) {
            paint_hospital_menu(painter);
        } else if (field_dialogue_.uses_field_console_menu_layout()) {
            paint_field_console_background(painter);

            const auto& panel = field_dialogue_.panel();
            const auto title = field_dialogue_.field_console_menu_title();
            const auto title_y = panel.compact_cursor_list ? 32 : 48;
            const auto rows_y = panel.compact_cursor_list ? 48 : 72;
            if (!title.empty()) {
                const auto title_x = 136 -
                    static_cast<int>(title.size()) * 4;
                draw_command_overview_text(painter, title_x, title_y, title);
                const auto underline_width = std::min<int>(
                    static_cast<int>(title.size()) * 8 - 16, 80);
                if (underline_width > 0) {
                    const auto underline_x = 136 - underline_width / 2;
                    painter.setPen(QColor(0, 255, 0));
                    for (int x = underline_x; x < underline_x + underline_width;
                         x += 4) {
                        painter.drawPoint(x, title_y + 10);
                    }
                }
            }
            for (std::size_t row = 0U; row < panel.option_count; ++row) {
                const auto y = rows_y + static_cast<int>(row) * 16;
                if (panel.cursor_selection && panel.selected_option == row) {
                    draw_command_overview_text(painter, 80, y, ">");
                }
                draw_command_overview_text(
                    painter, 96, y, panel.options[row].source_label);
            }
        } else {
            field_dialogue_.paint(painter);
        }
        apply_palette_fade();
        return;
    }
    if (command_overview_.open()) {
        if (command_overview_start_transition_black_frame_) {
            scene_painter.end();
            target_painter.drawImage(0, 0, scene);
            return;
        }
        paint_command_overview(painter);
        if (command_overview_attributes_skills_.is_open()) {
            apply_palette_fade();
            return;
        }
        // The field command loop updates its menu plane without replacing
        // the field's right-hand HUD plane. Preserve the current runner face,
        // item, and counters above the command panel. Pocket Secretary is a
        // separate full-screen composition and owns its own backdrop.
        if (!pocket_secretary_.open && !field_taxi_route_active_ &&
            !field_matrix_route_active_) {
            painter.drawImage(0, 0, field_hud_);
            paint_field_hud_overlays(painter);
        }
        apply_palette_fade();
        return;
    }
    paint_field_scene(painter);
    apply_palette_fade();
}

#ifdef SHADOWRUN_QT_QA_DRIVER
void FieldSceneRuntime::paint_field_scene_for_dump(
    QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, Qt::black);
    if (!ready_ || field_map_.isNull()) {
        return;
    }
    paint_field_scene(painter);
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::default_actor_count_for_dump() const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.active && actor.scene_reference == scene_reference;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::dynamic_world_actor_count_for_dump() const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.active && actor.temporary_world_encounter &&
                   actor.scene_reference == scene_reference;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::world_activity_script_actor_count_for_dump()
    const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.active && actor.world_activity_script_actor &&
                   actor.scene_reference == scene_reference;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<SceneRunnerPosition>
FieldSceneRuntime::world_activity_script_actor_position_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active && candidate.world_activity_script_actor &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
        ? std::nullopt
        : std::optional<SceneRunnerPosition>{actor->world_position};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::world_activity_script_actor_profile_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active && candidate.world_activity_script_actor &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
        ? std::nullopt
        : std::optional<std::string>{actor->script_profile_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<FieldWorldActorSlot>
FieldSceneRuntime::world_activity_script_actor_record_slot_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active && candidate.world_activity_script_actor &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
        ? std::nullopt
        : actor->source_record_slot;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::world_activity_script_actor_behavior_flags_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active && candidate.world_activity_script_actor &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
        ? std::nullopt
        : std::optional<std::uint8_t>{actor->source_behavior_flags};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::vector<SceneRunnerPosition>
FieldSceneRuntime::active_default_actor_positions_for_dump() const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    std::vector<SceneRunnerPosition> positions;
    for (const auto& actor : field_default_actors_) {
        if (actor.active && actor.scene_reference == scene_reference) {
            positions.push_back(actor.world_position);
        }
    }
    return positions;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::selected_default_actor_id_for_dump() const {
    if (!field_entity_target_owner_.selected_target.has_value()) {
        return std::nullopt;
    }
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto selected_slot = *field_entity_target_owner_.selected_target;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, selected_slot](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_record_slot.has_value() &&
                   static_cast<std::size_t>(
                       *candidate.source_record_slot) == selected_slot;
        });
    if (actor == field_default_actors_.end()) {
        return std::nullopt;
    }
    return actor->actor_instance_id;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<FieldTargetMarkerDebugState>
FieldSceneRuntime::selected_default_actor_target_marker_for_dump() const noexcept {
    if (!field_entity_target_owner_.selected_target.has_value()) {
        return std::nullopt;
    }
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto selected_slot = *field_entity_target_owner_.selected_target;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, selected_slot](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_record_slot.has_value() &&
                   static_cast<std::size_t>(
                       *candidate.source_record_slot) == selected_slot;
        });
    if (actor == field_default_actors_.end()) {
        return std::nullopt;
    }
    EntityProjectionState projection{};
    projection.world_x = actor->world_position.x;
    projection.world_y = actor->world_position.y;
    project_entity_screen_position_standard(
        projection,
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_x),
        static_cast<std::uint16_t>(state_.field_scene_buffers.camera_y));
    return FieldTargetMarkerDebugState{
        actor->target_marker_visible,
        actor->target_marker_animating,
        actor->target_marker_style,
        actor->target_marker_radius,
        actor->entity_kind,
        projection.screen_x,
        projection.screen_y};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<FieldCreatureAttackEffectDebugState>
FieldSceneRuntime::default_actor_creature_attack_effect_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.active &&
                   candidate.scene_reference == scene_reference &&
                   candidate.source_order == source_order &&
                   candidate.script_profile_id ==
                       "hell_hound_k0c_none00";
        });
    if (actor == field_default_actors_.end()) {
        return std::nullopt;
    }
    return FieldCreatureAttackEffectDebugState{
        actor->target_motion_effect_visible,
        actor->target_motion_effect_direction,
        actor->target_motion_effect_phase,
        actor->target_motion_effect_slot,
        actor->action_delay};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::set_default_actor_world_position_for_dump(
    const std::size_t active_actor_index, const std::uint16_t x,
    const std::uint16_t y) noexcept {
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    std::size_t active_index = 0U;
    for (auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene) {
            continue;
        }
        if (active_index == active_actor_index) {
            actor.world_position = SceneRunnerPosition{x, y};
            actor.movement_state.position_x = x;
            actor.movement_state.position_y = y;
            actor.movement_state_initialized = true;
            return true;
        }
        ++active_index;
    }
    return false;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::set_field_spell_effect_for_dump(
    const std::uint8_t action_code, const std::uint8_t phase,
    const bool visible) noexcept {
    if (!visible) {
        field_entity_.spell_effect_visible = false;
        field_entity_.spell_effect_action_code = 0U;
        field_entity_.spell_effect_phase = 0U;
        return true;
    }

    const auto profile = std::find_if(
        kFieldSpellEffectProfiles.cbegin(), kFieldSpellEffectProfiles.cend(),
        [action_code](const auto& candidate) {
            return candidate.action_code == action_code;
        });
    if (profile == kFieldSpellEffectProfiles.cend() ||
        phase >= profile->phase_count) {
        return false;
    }

    field_entity_.spell_effect_visible = true;
    field_entity_.spell_effect_action_code = action_code;
    field_entity_.spell_effect_phase = phase;
    return true;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::set_selected_default_actor_world_position_for_dump(
    const std::uint16_t x, const std::uint16_t y) noexcept {
    if (!field_entity_target_owner_.selected_target.has_value()) {
        return false;
    }
    const auto current_scene = state_.field_scene_buffers.scene_reference;
    const auto selected_slot = *field_entity_target_owner_.selected_target;
    for (auto& actor : field_default_actors_) {
        if (!actor.active || actor.scene_reference != current_scene ||
            !actor.source_record_slot.has_value() ||
            static_cast<std::size_t>(*actor.source_record_slot) !=
                selected_slot) {
            continue;
        }
        actor.world_position = SceneRunnerPosition{x, y};
        actor.movement_state.position_x = x;
        actor.movement_state.position_y = y;
        actor.movement_state_initialized = true;
        return true;
    }
    return false;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::resolve_and_spawn_field_encounter_for_dump(
    const std::uint8_t first_roll, const std::uint8_t second_roll,
    const std::uint16_t random_seed) {
    if (!ready_) {
        failure_ = "field encounter QA requested before field bootstrap";
        return 0U;
    }
    const auto scene_reference =
        state_.field_scene_buffers.scene_reference;
    field_random_seed_ = random_seed;
    auto outcome = game_scripts_.resolve_field_encounter(
        scene_reference, first_roll, second_roll);
    if (!outcome.has_value()) {
        failure_ = game_scripts_.failure();
        source_boundary("field.random_encounter.qa_selector_unresolved");
        return 0U;
    }
    if (outcome->kind != scripting::ScriptFieldEncounterOutcomeKind::
                             dynamic_world_effect_sequence) {
        failure_ = "field encounter QA selector did not resolve an actor sequence";
        source_boundary("field.random_encounter.qa_outcome_not_actor_sequence");
        return 0U;
    }

    const auto count_before = default_actor_count_for_dump();
    pending_field_encounter_outcome_ = std::move(*outcome);
    // This diagnostic entry point injects the already-resolved Lua selector
    // rolls. The 0x1E166 chance gate and 0x59986 frame eligibility remain
    // exercised by the live field loop, not by this focused spawn QA.
    spawn_field_world_effect_sequence();
    const auto count_after = default_actor_count_for_dump();
    return count_after >= count_before ? count_after - count_before : 0U;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t
FieldSceneRuntime::default_actor_transition_started_count_for_dump() const
    noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.scene_reference == scene_reference &&
                   actor.active &&
                   actor.transition_started;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t
FieldSceneRuntime::default_actor_transition_complete_count_for_dump() const
    noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    return static_cast<std::size_t>(std::count_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference](const DefaultFieldActorPresentation& actor) {
            return actor.scene_reference == scene_reference &&
                   actor.active &&
                   actor.transition_complete;
        }));
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_direction_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{actor->animation_direction};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_animation_frame_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{
                     actor->animation_frame};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::default_actor_animation_id_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::string>{actor->animation_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<FieldWorldActorSlot>
FieldSceneRuntime::default_actor_record_slot_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : actor->source_record_slot;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::default_actor_script_profile_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end() ||
                   actor->script_profile_id.empty()
               ? std::nullopt
               : std::optional<std::string>{actor->script_profile_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::default_actor_ai_package_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end() ||
                   actor->ai_package_id.empty()
               ? std::nullopt
               : std::optional<std::string>{actor->ai_package_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::string>
FieldSceneRuntime::default_actor_ai_target_id_for_dump(
    const std::uint16_t source_order) const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end() || actor->ai_target_id.empty()
               ? std::nullopt
               : std::optional<std::string>{actor->ai_target_id};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<bool>
FieldSceneRuntime::default_actor_transition_complete_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<bool>{actor->transition_complete};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<bool>
FieldSceneRuntime::default_actor_has_ai_target_pool_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<bool>{actor->ai_target_valid};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::default_actor_ai_target_distance_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end() || actor->ai_target_id.empty()
               ? std::nullopt
               : std::optional<std::uint16_t>{actor->ai_target_distance};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_weapon_item_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : actor->ai_weapon_item_id;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_source_action_ammo_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active && candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{actor->source_action_ammo};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint16_t>
FieldSceneRuntime::default_actor_source_animation_selector_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active && candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint16_t>{
                     actor->source_animation_selector};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::optional<std::uint8_t>
FieldSceneRuntime::default_actor_physical_meter_for_dump(
    const std::uint16_t source_order) const noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            const DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order;
        });
    return actor == field_default_actors_.end()
               ? std::nullopt
               : std::optional<std::uint8_t>{actor->physical_meter};
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::set_default_actor_physical_meter_for_dump(
    const std::uint16_t source_order,
    const std::uint8_t physical_meter) noexcept {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto actor = std::find_if(
        field_default_actors_.begin(), field_default_actors_.end(),
        [scene_reference, source_order](
            DefaultFieldActorPresentation& candidate) {
            return candidate.scene_reference == scene_reference &&
                   candidate.active &&
                   candidate.source_order == source_order &&
                   candidate.temporary_world_encounter;
        });
    if (actor == field_default_actors_.end()) {
        return false;
    }
    actor->physical_meter = physical_meter;
    actor->combatant.physical_meter = physical_meter;
    return true;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
std::size_t FieldSceneRuntime::visible_default_actor_count_for_dump() const {
    const auto scene_reference = state_.field_scene_buffers.scene_reference;
    const auto camera_x = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_x);
    const auto camera_y = static_cast<std::uint16_t>(
        state_.field_scene_buffers.camera_y);
    std::size_t visible = 0U;
    for (const auto& actor : field_default_actors_) {
        if (actor.scene_reference != scene_reference) {
            continue;
        }
        const auto sprite = default_actor_sprite_view(actor);
        if (sprite.image == nullptr || sprite.image->isNull()) {
            continue;
        }
        EntityProjectionState projection{};
        projection.world_x = actor.world_position.x;
        projection.world_y = actor.world_position.y;
        if (!project_entity_screen_position_standard(
                 projection, camera_x, camera_y)
                 .visible) {
            continue;
        }
        const auto left = static_cast<int>(projection.screen_x) -
                          kGenesisSpriteOrigin + sprite.origin_x;
        const auto top = static_cast<int>(projection.screen_y) -
                         kGenesisSpriteOrigin + sprite.origin_y;
        if (left < 272 && left + sprite.image->width() > 0 && top < 224 &&
            top + sprite.image->height() > 0) {
            ++visible;
        }
    }
    return visible;
}
#endif // SHADOWRUN_QT_QA_DRIVER


void FieldSceneRuntime::paint_field_console_background(
    QPainter& painter) const {
    painter.drawImage(0, 0, pocket_secretary_background_);
    painter.drawImage(24, 24, pocket_secretary_grid_);

    // Cyberspace uses the silver console as an overlay on the field. The
    // extracted Pocket Secretary bitmap is opaque outside that console, so
    // redraw the field only in the exposed screen regions after the frame.
    QRegion exposed(QRect(0, 0, 320, 224));
    exposed -= QRegion(QRect(6, 7, 261, 178));
    // The source keeps two compact terminal controls over the field beneath
    // the window: the console icon and the small map/route control.
    exposed -= QRegion(QRect(146, 184, 38, 26));
    exposed -= QRegion(QRect(216, 184, 32, 26));
    painter.save();
    painter.setClipRegion(exposed);
    paint_field_scene(painter);
    painter.restore();
}

void FieldSceneRuntime::paint_field_scene(QPainter& painter) const {
    const auto& scene = state_.field_scene_buffers;
    const auto camera_x = std::max(
        0, static_cast<int>(scene.camera_x >> 4U));
    const auto camera_y = std::max(
        0, static_cast<int>(scene.camera_y >> 4U));
    const auto max_x = std::max(0, field_map_.width() - 320);
    const auto max_y = std::max(0, field_map_.height() - 224);
    const auto viewport_x = std::min(camera_x, max_x);
    const auto viewport_y = std::min(camera_y, max_y);
    // ROM 0x481E builds ordinary field actors with sprite priority clear.
    // Preserve the source map's high-priority tiles over those actors; the
    // same draw order must hold for every scene loaded from a map resource.
    paint_field_map_composition(
        painter, field_map_, field_map_priority_overlay_,
        QRect(viewport_x, viewport_y, 320, 224),
        [this](QPainter& target) { paint_field_scene_entities(target); });

    // 0x334C supplies the static chrome. 0x35D4/0x35F0/0x36FC/0x3674 stamp
    // the live face, item, and counter overlays at the $F000 nametable
    // cells. New-game keeps slots 1 and 2 inactive.
    painter.drawImage(0, 0, field_hud_);
    paint_field_hud_overlays(painter);
    // VDP register $91 places the HUD in the low-priority window plane. ROM
    // sprite priority therefore lets actors cover it where they extend into
    // columns 34-39 (x=272..319). The map pass above already draws each actor
    // against the playfield; redraw only the window area after its contents.
    paint_field_scene_entities(painter, QRect(272, 0, 48, 224));
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
