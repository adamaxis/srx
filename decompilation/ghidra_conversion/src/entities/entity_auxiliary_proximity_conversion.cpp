#include "shadowrun_ghidra/entities/entity_auxiliary_proximity_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

struct Window {
    std::uint16_t left = 0;
    std::uint16_t right = 0;
};

Window build_window(const std::uint16_t coordinate, const std::uint16_t radius,
                    const std::uint16_t width) {
    const auto left = coordinate < radius
                          ? 0U
                          : static_cast<std::uint16_t>(coordinate - radius);
    return Window{
        static_cast<std::uint16_t>(left),
        static_cast<std::uint16_t>(left + width)};
}

bool is_inside(const std::uint16_t value, const Window window) {
    return value >= window.left && value < window.right;
}

} // namespace

EntityAuxiliaryProximityResult service_nearby_entity_auxiliary_behavior(
    const EntityAuxiliaryProximitySource& source,
    const std::array<EntityAuxiliaryProximityCandidate, 10>& candidates,
    const EntityAuxiliaryProximityCallbacks& callbacks) {
    const bool narrow_window = source.behavior_type == 0x0EU;
    const auto radius = static_cast<std::uint16_t>(
        narrow_window ? 0x0140U : 0x0280U);
    const auto width = static_cast<std::uint16_t>(
        narrow_window ? 0x0280U : 0x0500U);
    const auto x_window = build_window(source.world_x, radius, width);
    const auto y_window = build_window(source.world_y, radius, width);
    EntityAuxiliaryProximityResult result{};

    for (std::size_t index = 0; index < candidates.size(); ++index) {
        const auto& candidate = candidates[index];
        if (!candidate.active || candidate.excluded) {
            continue;
        }
        if (!is_inside(candidate.world_x, x_window) ||
            !is_inside(candidate.world_y, y_window)) {
            continue;
        }
        ++result.serviced_candidates;
        if (callbacks.service_candidate) {
            callbacks.service_candidate(index);
        }
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
