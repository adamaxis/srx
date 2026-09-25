#include "shadowrun_ghidra/nearby_effect_conversion.hpp"

#include <cstdint>
#include <optional>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<NearbyWorldEffectRecord> records(12U);
    records[0] = {0x0100U, 0x24U, 0x0200U, 0x0010U, true, false};
    records[1] = {0x0100U, 0x65U, 0x0200U, 0x0020U, true, false};
    records[2] = {0x0100U, 0x30U, 0x0200U, 0x0030U, true, true};
    records[3] = {0x0100U, 0x31U, 0x0200U, 0x0040U, true, false};
    int calls = 0;
    int audio = 0;
    std::uint16_t cleared_slot = 0;
    NearbyEffectResolutionState state{};
    resolve_nearby_world_effects(
        state, records, 0x0100U, 0x0200U,
        NearbyEffectResolutionCallbacks{
            [&](const NearbyWorldEffectRecord& record, const std::size_t) {
                ++calls;
                return std::optional<NearbyEffectInteractionResult>{
                    NearbyEffectInteractionResult{0x0120U, 0x0220U}};
            },
            [&] { ++audio; },
            [&](const std::uint16_t slot) { cleared_slot = slot; }});

    // The source uses strict bounds: the exact center is accepted, and all
    // four matching records are visited before later probe movement matters.
    if (calls != 4 || state.interactions_resolved != 4U ||
        state.effects_consumed != 3U || audio != 3 || records[0].active ||
        records[1].active || !records[2].active || records[3].active ||
        cleared_slot != 0x0020U || state.probe_x != 0x0120U ||
        state.probe_y != 0x0220U) {
        return 1;
    }

    // A coordinate exactly on a +/-0x100 boundary is not a match, and only
    // the first ten source slots are scanned.
    records[0].active = true;
    records[0].x = 0x0200U;
    records[0].y = 0x0200U;
    records[2].active = false;
    records[10].active = true;
    int boundary_calls = 0;
    resolve_nearby_world_effects(
        state, records, 0x0100U, 0x0200U,
        NearbyEffectResolutionCallbacks{
            [&](const NearbyWorldEffectRecord&, const std::size_t) {
                ++boundary_calls;
                return std::optional<NearbyEffectInteractionResult>{
                    NearbyEffectInteractionResult{}};
            },
            {},
            {}});
    if (boundary_calls != 0 || records[10].active == false) {
        return 2;
    }
    return 0;
}
