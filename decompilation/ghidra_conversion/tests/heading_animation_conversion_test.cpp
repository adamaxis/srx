#include "shadowrun_ghidra/heading_animation_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    const std::array<std::uint16_t, 8> field_animations{
        0x2000, 0x2002, 0x2004, 0x2006,
        0x2008, 0x200A, 0x200C, 0x200E};
    const std::array<std::uint16_t, 8> alternate_animations{
        0x3000, 0x3002, 0x3004, 0x3006,
        0x3008, 0x300A, 0x300C, 0x300E};

    shadowrun::ghidra_conversion::HeadingAnimationState field_state{};
    std::vector<int> field_events;
    shadowrun::ghidra_conversion::set_entity_heading_with_field_animation(
        field_state, 0x2000, 0x0B, field_animations,
        [&field_events](const std::uint16_t delay_mode) {
            field_events.push_back(static_cast<int>(delay_mode));
        },
        [&field_events]() { field_events.push_back(2); });

    if (expect(field_state.heading == 0x2000) != 0 ||
        expect(field_state.direction_animation == 0x2002) != 0 ||
        expect(field_state.continuation ==
               shadowrun::ghidra_conversion::MovementContinuation::
                   ReevaluateCollision) != 0 ||
        expect(field_events == std::vector<int>{0, 2}) != 0) {
        return 1;
    }

    field_events.clear();
    shadowrun::ghidra_conversion::set_entity_heading_with_field_animation(
        field_state, 0x0000, 0x05, field_animations,
        [&field_events](const std::uint16_t delay_mode) {
            field_events.push_back(static_cast<int>(delay_mode));
        },
        [&field_events]() { field_events.push_back(2); });
    if (expect(field_state.direction_animation == 0x2000) != 0 ||
        expect(field_events == std::vector<int>{1, 2}) != 0) {
        return 1;
    }

    shadowrun::ghidra_conversion::HeadingAnimationState alternate_state{};
    std::vector<int> alternate_events;
    shadowrun::ghidra_conversion::set_entity_heading_with_alternate_animation(
        alternate_state, 0x4000, alternate_animations,
        [&alternate_events](const std::uint16_t delay_mode) {
            alternate_events.push_back(static_cast<int>(delay_mode));
        },
        [&alternate_events]() { alternate_events.push_back(2); });
    if (expect(alternate_state.direction_animation == 0x3004) != 0 ||
        expect(alternate_events == std::vector<int>{2, 0}) != 0) {
        return 1;
    }

    return 0;
}
