#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct SecondaryWorldEffectRecord {
    bool active = false;             // FF15CC
    std::uint8_t phase_delay = 0;   // FF15D0
    std::uint8_t animation_phase = 0; // FF15CF
    std::uint8_t source_variant = 0; // FF15CB
    std::uint8_t frame_delay = 0;   // FF15CA
    std::uint8_t lifetime = 0;       // FF15D1
    std::int16_t position_x = 0;    // FF15C6
    std::int16_t position_y = 0;    // FF15C8
    std::int16_t velocity_x = 0;    // FF15D2
    std::int16_t velocity_y = 0;    // FF15D4
};

struct SpecialWorldEffectSetup {
    std::uint16_t position_x = 0;    // FFE060
    std::uint32_t behavior_token = 0; // behavior record +4
    std::uint8_t behavior_stage = 0; // behavior record +0x0D
    std::uint16_t position_y = 0;    // FFE062
    std::int16_t completion_marker = 0; // FFE05E
    std::uint16_t motion_phase = 0;  // FFE064
    std::uint16_t frame_countdown = 0; // FFE066
};

struct WorldEffectActivationState {
    std::uint8_t control_flags = 0; // FF1592, bit 6 selects the secondary path
    bool transition_work_active = false; // FFE010
    std::int8_t setup_marker = 0;   // FFF0B7
    bool transition_pending = false; // FFE0E2
    SecondaryWorldEffectRecord secondary{};
    SpecialWorldEffectSetup special{};
};

struct WorldEffectActivationCallbacks {
    // Inline special setup at the source's 0x1C850/0x1C826 boundary.
    std::function<void(std::uint16_t audio_command)> play_audio;
    std::function<void()> initialize_world_effect_motion;
    // 0x1CB6A receives D5=0x20, D6=-0x28 and D1=0. The spawn routine owns
    // the actual record fields; this callback is its native boundary.
    std::function<bool(std::int16_t offset_x, std::int16_t offset_y,
                       std::int16_t source_word)> spawn_effect_record;
    std::function<void(std::int16_t y_delta)> adjust_spawn_record_y;
};

enum class WorldEffectActivationAction {
    idle_secondary_record,
    secondary_record_started,
    special_effect_started,
    motion_initialization_requested,
    spawn_requested,
};

// ROM 0x1CB20: select the secondary seed path, special setup path, motion
// initializer, or record-spawn path based on the source flags and markers.
WorldEffectActivationAction advance_world_effect_activation(
    WorldEffectActivationState& state,
    const WorldEffectActivationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
