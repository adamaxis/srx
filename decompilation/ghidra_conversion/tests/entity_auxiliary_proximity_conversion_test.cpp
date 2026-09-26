#include "shadowrun_ghidra/entity_auxiliary_proximity_conversion.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityAuxiliaryProximitySource source{
        0x1000U, 0x2000U, 0x03U};
    std::array<EntityAuxiliaryProximityCandidate, 10> candidates{};
    candidates[0] = {true, false, 0x0D80U, 0x1E00U};
    candidates[1] = {true, false, 0x127FU, 0x227FU};
    candidates[2] = {true, false, 0x1280U, 0x2280U};
    candidates[3] = {true, true, 0x1000U, 0x2000U};
    candidates[4] = {false, false, 0x1000U, 0x2000U};
    candidates[5] = {true, false, 0x1100U, 0x2100U};
    std::vector<std::size_t> serviced;
    const auto result = service_nearby_entity_auxiliary_behavior(
        source, candidates,
        {[&serviced](const std::size_t index) { serviced.push_back(index); }});
    if (!expect(result.serviced_candidates == 3U &&
                serviced == std::vector<std::size_t>({0U, 1U, 5U}))) {
        return 1;
    }

    source.behavior_type = 0x0EU;
    serviced.clear();
    const auto narrow = service_nearby_entity_auxiliary_behavior(
        source, candidates,
        {[&serviced](const std::size_t index) { serviced.push_back(index); }});
    if (!expect(narrow.serviced_candidates == 1U &&
                serviced == std::vector<std::size_t>({5U}))) {
        return 2;
    }

    EntityAuxiliaryProximitySource clamped_source{
        0x0100U, 0x0120U, 0x03U};
    std::array<EntityAuxiliaryProximityCandidate, 10> clamped_candidates{};
    clamped_candidates[0] = {true, false, 0x0000U, 0x0000U};
    clamped_candidates[1] = {true, false, 0x04FFU, 0x04FFU};
    clamped_candidates[2] = {true, false, 0x0500U, 0x04FFU};
    const auto clamped = service_nearby_entity_auxiliary_behavior(
        clamped_source, clamped_candidates, {});
    if (!expect(clamped.serviced_candidates == 2U)) {
        return 3;
    }

    return 0;
}
