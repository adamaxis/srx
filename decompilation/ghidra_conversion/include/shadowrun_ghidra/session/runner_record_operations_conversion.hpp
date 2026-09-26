#pragma once

#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

enum class RunnerAssetOperationResult {
    no_change,
    ammo_changed,
    inventory_phase_changed,
    equipment_cleared,
    invalid_equipped_slot,
};

// ROM 0x0D82A reads these values from the active live record while building
// the Attributes/Skills view. Drawing text remains a separate concern.
struct AttributeSkillValues {
    std::uint8_t race_index = 0;
    std::uint8_t species_index = 0;
    std::uint8_t archetype_index = 0;
    std::array<std::uint8_t, kRunnerRecordAttributeCount> attributes{};
    std::array<std::uint8_t, kRunnerRecordSkillCount> skills{};
};

AttributeSkillValues read_attributes_and_skills(
    const RunnerRecordFields& record);

// ROM 0x0D5DC and the inline entry at 0x11950 read the cyberware bitfield.
// Keep all sixteen source bits available to callers; screens choose groups.
struct CyberwareValues {
    std::uint16_t flags = 0;
    std::array<bool, 16U> installed{};
};

CyberwareValues read_cyberware_detail(const RunnerRecordFields& record);

// ROM 0x036FC. `delta` is the signed source meter delta and `ammo_limit` is
// the selected item row's extracted maximum-round value.
RunnerAssetOperationResult advance_entity_asset_meter(
    RunnerRecordFields& record, std::int8_t delta,
    std::uint8_t ammo_limit);

// ROM 0x038AC. Record types below three delegate to 0x036FC; other record
// types consume one round directly.
RunnerAssetOperationResult service_entity_ammo(
    RunnerRecordFields& record, std::int8_t meter_delta,
    std::uint8_t ammo_limit);

// ROM 0x107CE's record mutation, without its VDP/menu side effects.
std::uint8_t adjust_runner_posture(RunnerRecordFields& record,
                                   std::int8_t delta);

enum class RunnerPostureLoopResult {
    waiting,
    adjusted,
    finished,
};

// ROM 0x10722: the posture sub-loop gives up/down priority to posture
// mutation, while the source confirm/cancel masks finish the loop. Rendering
// and vblank service remain outside this state conversion.
RunnerPostureLoopResult apply_runner_posture_input(
    RunnerRecordFields& record, std::uint16_t input);

} // namespace shadowrun::ghidra_conversion
