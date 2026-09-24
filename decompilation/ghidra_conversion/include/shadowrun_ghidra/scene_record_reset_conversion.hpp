#pragma once

#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

struct SceneRecordResetState {
    std::array<std::uint16_t, 10> transition_words{};
    std::array<std::uint8_t, 10> pending_flags{};
};

// ROM 0x164DC: clear the leading transition word in ten scene records.
void clear_scene_record_transition_words(SceneRecordResetState& state);

// ROM 0x16DEE: clear bit 0 in the ten scene-record pending flags.
void clear_scene_record_pending_flags(SceneRecordResetState& state);

} // namespace shadowrun::ghidra_conversion
