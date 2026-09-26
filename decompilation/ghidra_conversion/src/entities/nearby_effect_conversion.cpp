#include "shadowrun_ghidra/entities/nearby_effect_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

bool within_effect_radius(const std::uint16_t coordinate,
                          const std::uint16_t probe) {
    // The source uses unsigned 16-bit subtraction/addition and strict
    // comparisons, so retain those operations rather than using a host
    // absolute-distance helper.
    const auto lower = static_cast<std::uint16_t>(coordinate - 0x0100U);
    const auto upper = static_cast<std::uint16_t>(coordinate + 0x0100U);
    return lower < probe && probe < upper;
}

} // namespace

void resolve_nearby_world_effects(
    NearbyEffectResolutionState& state,
    std::vector<NearbyWorldEffectRecord>& records,
    const std::uint16_t probe_x,
    const std::uint16_t probe_y,
    const NearbyEffectResolutionCallbacks& callbacks) {
    state.probe_x = probe_x;
    state.probe_y = probe_y;
    state.interaction_attempts = 0U;
    state.interactions_resolved = 0U;
    state.effects_consumed = 0U;

    const auto count = std::min<std::size_t>(10U, records.size());
    for (std::size_t index = 0; index < count; ++index) {
        auto& record = records[index];
        if (!record.active || !within_effect_radius(record.x, state.probe_x) ||
            !within_effect_radius(record.y, state.probe_y) ||
            !callbacks.apply_interaction) {
            continue;
        }

        ++state.interaction_attempts;
        const auto result = callbacks.apply_interaction(record.interaction_code);
        if (!result.succeeded()) {
            // 0x1DF34 branches back to the next record on D7 < 0.  It does
            // not consume or audio-service a full-inventory interaction.
            continue;
        }
        ++state.interactions_resolved;
        if (callbacks.service_audio) {
            callbacks.service_audio();
        }
        record.active = false;
        ++state.effects_consumed;
        if (record.interaction_code == 0x65U && callbacks.clear_effect_slot) {
            callbacks.clear_effect_slot(record.slot_offset);
        }

        // The source returns after any successfully consumed effect.  Only a
        // negative D7 result reaches the next record.
        break;
    }
}

} // namespace shadowrun::ghidra_conversion
