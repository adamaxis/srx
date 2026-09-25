#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct NearbyWorldEffectRecord {
    std::uint16_t x = 0;                 // FF164C +0x00
    std::uint16_t interaction_code = 0;  // FF164C +0x02
    std::uint16_t y = 0;                 // FF164C +0x04
    std::uint16_t slot_offset = 0;       // FF164C +0x06
    bool active = false;                 // +0x08 bit 0
    bool negative_marker = false;        // +0x02 signed test in source record
};

struct NearbyEffectInteractionResult {
    // ROM 0x133A returns the next interaction probe coordinates in D0/D1.
    std::uint16_t next_probe_x = 0;
    std::uint16_t next_probe_y = 0;
};

struct NearbyEffectResolutionCallbacks {
    // ROM 0x133A: resolve the interaction code against the nearby entity.
    std::function<std::optional<NearbyEffectInteractionResult>(
        const NearbyWorldEffectRecord&, std::size_t record_index)>
        resolve_interaction;
    // ROM 0x530DE with command 0x24.
    std::function<void()> service_audio;
    // ROM 0x1E514 after the special interaction code 0x65.
    std::function<void(std::uint16_t slot_offset)> clear_effect_slot;
};

struct NearbyEffectResolutionState {
    std::uint16_t probe_x = 0;
    std::uint16_t probe_y = 0;
    std::size_t interactions_resolved = 0;
    std::size_t effects_consumed = 0;
};

// ROM 0x1DF34: scan one ten-entry projected-effect list for a strict +/-0x100
// proximity match, resolve its interaction, and consume the matching effect
// when the source record is not marked negative.
void resolve_nearby_world_effects(
    NearbyEffectResolutionState& state,
    std::vector<NearbyWorldEffectRecord>& records,
    std::uint16_t probe_x,
    std::uint16_t probe_y,
    const NearbyEffectResolutionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
