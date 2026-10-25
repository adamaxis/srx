#pragma once

#include <cstddef>
#include <cstdint>

#include "field_scene_runtime.hpp"

namespace shadowrun::ghidra_conversion::qt_runtime::field_scene_detail {

// The game loop seeds FFDDC4 with scene 2 before 0x03084. Loaded saves replace
// that scene reference with the saved row; both paths use the same typed scene
// resource selection below.
inline constexpr std::uint16_t kInitialFieldSceneReference = 2U;
inline constexpr std::uint8_t kFieldModalReturnTransitionFrames = 60U;

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
