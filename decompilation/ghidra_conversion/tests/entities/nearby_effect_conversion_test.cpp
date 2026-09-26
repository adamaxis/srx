#include "shadowrun_ghidra/entities/nearby_effect_conversion.hpp"

#include <cstdint>
#include <vector>

using namespace shadowrun::ghidra_conversion;

int main() {
    std::vector<NearbyWorldEffectRecord> records(12U);
    records[0] = {0x0100U, 0x24U, 0x0200U, 0x0010U, true};
    records[1] = {0x0100U, 0x65U, 0x0200U, 0x0020U, true};
    records[2] = {0x0100U, 0xC005U, 0x0200U, 0x0030U, true};
    records[3] = {0x0100U, 0x31U, 0x0200U, 0x0040U, true};
    int calls = 0;
    int audio = 0;
    std::uint16_t cleared_slot = 0;
    NearbyEffectResolutionState state{};
    resolve_nearby_world_effects(
        state, records, 0x0100U, 0x0200U,
        NearbyEffectResolutionCallbacks{
            [&](const std::uint16_t command) {
                ++calls;
                WorldRewardResult result{};
                result.source_status = command == 0x31U ? -1 : 0;
                return result;
            },
            [&] { ++audio; },
            [&](const std::uint16_t slot) { cleared_slot = slot; }});

    // The source accepts the center point, applies the first command, and
    // returns immediately after a successful consume.  Signed commands are
    // still successful; only D7=-1 continues the scan.
    if (calls != 1 || state.interaction_attempts != 1U ||
        state.interactions_resolved != 1U || state.effects_consumed != 1U ||
        audio != 1 || records[0].active || !records[1].active ||
        !records[2].active || !records[3].active || cleared_slot != 0U ||
        state.probe_x != 0x0100U || state.probe_y != 0x0200U) {
        return 1;
    }

    // A full inventory result continues to the next eligible source slot.
    records[0].active = true;
    records[0].interaction_code = 0x31U;
    records[1].active = true;
    records[1].interaction_code = 0x65U;
    int retry_calls = 0;
    audio = 0;
    cleared_slot = 0U;
    resolve_nearby_world_effects(
        state, records, 0x0100U, 0x0200U,
        NearbyEffectResolutionCallbacks{
            [&](const std::uint16_t command) {
                ++retry_calls;
                WorldRewardResult result{};
                result.source_status = command == 0x31U ? -1 : 0;
                return result;
            },
            [&] { ++audio; },
            [&](const std::uint16_t slot) { cleared_slot = slot; }});
    if (retry_calls != 2 || state.interaction_attempts != 2U ||
        state.interactions_resolved != 1U || state.effects_consumed != 1U ||
        audio != 1 || !records[0].active || records[1].active ||
        cleared_slot != 0x0020U) {
        return 2;
    }

    // A coordinate exactly on a +/-0x100 boundary is not a match, and only
    // the first ten source slots are scanned.
    records[0].active = true;
    records[0].x = 0x0200U;
    records[0].y = 0x0200U;
    records[1].active = false;
    records[2].active = false;
    records[3].active = false;
    records[10].active = true;
    int boundary_calls = 0;
    resolve_nearby_world_effects(
        state, records, 0x0100U, 0x0200U,
        NearbyEffectResolutionCallbacks{
            [&](const std::uint16_t) {
                ++boundary_calls;
                return WorldRewardResult{};
            },
            {},
            {}});
    if (boundary_calls != 0 || !records[10].active) {
        return 3;
    }

    return 0;
}
