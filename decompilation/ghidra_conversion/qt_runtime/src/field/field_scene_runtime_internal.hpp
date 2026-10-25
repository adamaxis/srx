#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "field_scene_runtime.hpp"

namespace shadowrun::ghidra_conversion::qt_runtime::field_scene_detail {

// The game loop seeds FFDDC4 with scene 2 before 0x03084. Loaded saves replace
// that scene reference with the saved row; both paths use the same typed scene
// resource selection below.
inline constexpr std::uint16_t kInitialFieldSceneReference = 2U;
inline constexpr std::uint8_t kFieldModalReturnTransitionFrames = 60U;

inline constexpr std::string_view kFieldHudAlias = "field.hud.background";
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

} // namespace shadowrun::ghidra_conversion::qt_runtime::field_scene_detail
