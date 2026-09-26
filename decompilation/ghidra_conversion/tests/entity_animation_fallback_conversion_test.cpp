#include "shadowrun_ghidra/entity_animation_fallback_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityAnimationFallbackState forward_state{0, 0, 0};
    std::vector<int> forward_calls;
    const auto forward_result =
        prepare_entity_animation_fallback_target(
            forward_state,
            EntityAnimationFallbackReference{0x1000U, 0x2000U, 0xA000U, 0},
            EntityAnimationFallbackReference{0x3000U, 0x4000U, 0U, 0},
            EntityAnimationFallbackCallbacks{
                [&forward_calls](const std::uint16_t x,
                                 const std::uint16_t y) {
                    forward_calls.push_back(1);
                    if (x != 0x10DFU || y != 0x2000U) {
                        return static_cast<std::int16_t>(1);
                    }
                    return static_cast<std::int16_t>(0);
                },
                [&forward_calls]() {
                    forward_calls.push_back(2);
                    return false;
                }});
    if (expect(forward_result.action ==
                   EntityAnimationFallbackAction::target_prepared &&
               forward_result.selection ==
                   EntityAnimationFallbackTargetSelection::forward_projection &&
               !forward_result.source_carry &&
               forward_state.target_x == 0x10DFU &&
               forward_state.target_y == 0x2000U &&
               forward_calls == std::vector<int>{1, 2}) != 0) {
        return 1;
    }

    EntityAnimationFallbackState reverse_state{0, 0, 0};
    std::vector<int> reverse_calls;
    const auto reverse_result =
        prepare_entity_animation_fallback_target(
            reverse_state,
            EntityAnimationFallbackReference{0x1000U, 0x2000U, 0xA000U, 0},
            EntityAnimationFallbackReference{},
            EntityAnimationFallbackCallbacks{
                [&reverse_calls](const std::uint16_t x,
                                 const std::uint16_t y) {
                    reverse_calls.push_back(
                        x == 0x10DFU && y == 0x2000U ? 1 : 2);
                    return static_cast<std::int16_t>(
                        x == 0x10DFU ? 1 : 0);
                },
                [&reverse_calls]() {
                    reverse_calls.push_back(3);
                    return false;
                }});
    if (expect(reverse_result.selection ==
                   EntityAnimationFallbackTargetSelection::reverse_projection &&
               reverse_state.target_x == 0x0F21U &&
               reverse_state.target_y == 0x2000U &&
               reverse_calls == std::vector<int>{1, 2, 3}) != 0) {
        return 2;
    }

    EntityAnimationFallbackState blocked_state{0, 0, 0};
    const auto blocked_result =
        prepare_entity_animation_fallback_target(
            blocked_state,
            std::nullopt,
            EntityAnimationFallbackReference{0x2200U, 0x3300U, 0x4000U, 0},
            EntityAnimationFallbackCallbacks{
                [](const std::uint16_t, const std::uint16_t) {
                    return static_cast<std::int16_t>(1);
                },
                []() { return true; }});
    if (expect(blocked_result.action ==
                   EntityAnimationFallbackAction::close_range_boundary &&
               blocked_result.selection ==
                   EntityAnimationFallbackTargetSelection::reference_position &&
               blocked_result.source_carry &&
               blocked_state.target_x == 0x2200U &&
               blocked_state.target_y == 0x3300U) != 0) {
        return 3;
    }

    const auto unresolved = prepare_entity_animation_fallback_target(
        blocked_state,
        std::nullopt,
        EntityAnimationFallbackReference{},
        EntityAnimationFallbackCallbacks{});
    if (expect(unresolved.action ==
                   EntityAnimationFallbackAction::boundary_unresolved &&
               unresolved.boundary_unresolved &&
               !unresolved.source_carry) != 0) {
        return 4;
    }

    return 0;
}
