#include "shadowrun_ghidra/entities/behavior_animation_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    const std::array<std::uint16_t, 8> primary_animations{
        0x1000, 0x1002, 0x1004, 0x1006,
        0x1008, 0x100A, 0x100C, 0x100E};
    const std::array<std::uint16_t, 8> auxiliary_animations{
        0x2000, 0x2002, 0x2004, 0x2006,
        0x2008, 0x200A, 0x200C, 0x200E};
    const std::array<std::uint16_t, 8> immediate_animations{
        0x3000, 0x3002, 0x3004, 0x3006,
        0x3008, 0x300A, 0x300C, 0x300E};

    DelayedBehaviorAnimationState delayed{};
    prepare_entity_delayed_animation_primary(
        delayed, {0, 1, 0}, 0x2000, primary_animations);
    if (expect(delayed.heading == 0x2000) != 0 ||
        expect(delayed.direction_animation == 0x1002) != 0 ||
        expect(delayed.delay.animation_delay == 106) != 0 ||
        expect(delayed.continuation ==
               EntityBehaviorContinuation::DelayedCollisionPrimary) != 0 ||
        expect(delayed.animation.frame_timer == 0) != 0 ||
        expect((delayed.animation.render_flags & 1U) != 0U) != 0) {
        return 1;
    }

    prepare_entity_delayed_animation_alternate(
        delayed, {40, 80, 0}, 0x4000, primary_animations);
    if (expect(delayed.direction_animation == 0x1004) != 0 ||
        expect(delayed.delay.animation_delay == 15) != 0 ||
        expect(delayed.continuation ==
               EntityBehaviorContinuation::DelayedCollisionAlternate) != 0) {
        return 1;
    }

    EntityBehaviorState auxiliary{0, 1, 0, 0, 0,
                                  EntityBehaviorContinuation::ImmediateCollision,
                                  {9, 0, 4}};
    std::vector<int> effects;
    prepare_entity_auxiliary_behavior(
        auxiliary, auxiliary_animations, immediate_animations, 0x4000,
        [&effects]() { effects.push_back(1); },
        [&effects]() { effects.push_back(2); });
    if (expect(auxiliary.countdown == 8) != 0 ||
        expect(auxiliary.direction_animation == 0x2004) != 0 ||
        expect(auxiliary.continuation ==
               EntityBehaviorContinuation::AuxiliaryCounterExpiry) != 0 ||
        expect(effects.empty()) != 0) {
        return 1;
    }

    auxiliary.auxiliary_state = 0;
    auxiliary.flags = 0;
    prepare_entity_auxiliary_behavior(
        auxiliary, auxiliary_animations, immediate_animations, 0x2000,
        [&effects]() { effects.push_back(3); },
        [&effects]() { effects.push_back(4); });
    if (expect((auxiliary.flags & 0x50U) == 0x10U) != 0 ||
        expect(auxiliary.direction_animation == 0x3002) != 0 ||
        expect(auxiliary.continuation ==
               EntityBehaviorContinuation::ImmediateCollision) != 0 ||
        expect(effects == std::vector<int>({3, 4})) != 0) {
        return 1;
    }

    auxiliary.flags = 0x10;
    effects.clear();
    prepare_entity_auxiliary_behavior(
        auxiliary, auxiliary_animations, immediate_animations, 0,
        [&effects]() { effects.push_back(5); },
        [&effects]() { effects.push_back(6); });
    if (expect(effects.empty()) != 0) {
        return 1;
    }

    auxiliary.flags = 0;
    std::uint16_t action_code = 0;
    enter_entity_behavior_transition(
        auxiliary, immediate_animations, 0x0000, 0x004D,
        [&effects]() { effects.push_back(7); },
        [&effects, &action_code](const std::uint16_t code) {
            effects.push_back(8);
            action_code = code;
        });
    if (expect(action_code == 0x004D) != 0 ||
        expect((auxiliary.flags & 0x50U) == 0x10U) != 0 ||
        expect(auxiliary.direction_animation == 0x3000) != 0 ||
        expect(effects == std::vector<int>({7, 8})) != 0) {
        return 1;
    }

    std::vector<int> service_order;
    service_entity_auxiliary_behavior(
        [&service_order] { service_order.push_back(1); },
        [&service_order] { service_order.push_back(2); });
    if (service_order != std::vector<int>({1, 2})) {
        return 1;
    }

    return 0;
}
