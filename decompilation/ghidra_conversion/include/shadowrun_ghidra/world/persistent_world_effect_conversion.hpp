#pragma once

#include "shadowrun_ghidra/world/world_effect_record_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct PersistentWorldEffectSpriteSlot {
    std::optional<std::uint32_t> sprite_data;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

struct PersistentWorldEffectState {
    WorldEffectRecordState record{};
    std::uint8_t ring_index = 0;       // record +0x08, six-slot circular index
    std::uint16_t frame_counter = 0;   // FFDF88
    std::uint16_t transition_state = 0; // FFE0D6
    std::array<PersistentWorldEffectSpriteSlot, 6> slots{};
};

struct PersistentWorldEffectCallbacks {
    std::function<std::int16_t(std::uint8_t easing_index)> resolve_easing;
    std::function<std::optional<std::uint32_t>(
        std::uint8_t source_variant, std::uint8_t animation_phase)>
        resolve_sprite_source;
    std::function<void(std::uint32_t sprite_data_token,
                       std::uint16_t x, std::uint16_t y,
                       std::uint16_t build_flags)>
        build_sprite;
    std::function<void()> run_menu_selection_variant;
};

enum class PersistentWorldEffectAction {
    inactive,
    rendered,
    record_unresolved,
    sprite_source_unresolved,
    finalized,
};

// ROM 0x1CC24: tick the persistent effect record, update its six-slot
// circular sprite work area every fourth global frame, draw seven entries
// beginning after the selected slot, and finalize through the menu boundary.
PersistentWorldEffectAction advance_persistent_world_effect(
    PersistentWorldEffectState& state,
    const PersistentWorldEffectCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
