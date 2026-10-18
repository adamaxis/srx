#include "shadowrun_ghidra/entities/movement_route_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    const MovementResultPolicy policy{};

    std::vector<std::uint16_t> route_keys;
    std::vector<std::uint16_t> probed_slots;
    const auto lookup = [&route_keys](const std::uint16_t key) {
        route_keys.push_back(key);
        return DirectionalMovementRoute{0, 1, 4, 6};
    };
    const auto probe = [&probed_slots](const std::uint16_t slot) {
        probed_slots.push_back(slot);
        return MovementProbeResult{1, 0x1111, 0x2222,
                                   static_cast<std::uint8_t>(slot)};
    };
    const auto primary_fallback = evaluate_entity_movement_route(
        0x1000, policy, lookup, probe);
    if (!expect(primary_fallback.carry_set) ||
        !expect(primary_fallback.fallback_probe.has_value()) ||
        !expect(primary_fallback.fallback_probe->candidate_x == 0x1111) ||
        !expect(route_keys == std::vector<std::uint16_t>{0}) ||
        !expect(probed_slots == std::vector<std::uint16_t>{8})) {
        return 1;
    }

    probed_slots.clear();
    const auto secondary_fallback = evaluate_entity_movement_route(
            0x1000, policy,
            [](const std::uint16_t) {
                return DirectionalMovementRoute{1, 0, 4, 6};
            },
            [&probed_slots](const std::uint16_t slot) {
                probed_slots.push_back(slot);
                return MovementProbeResult{-9, 0x3333, 0x4444,
                                           static_cast<std::uint8_t>(slot)};
            });
    if (!expect(!secondary_fallback.carry_set) ||
        !expect(secondary_fallback.fallback_probe.has_value()) ||
        !expect(secondary_fallback.fallback_probe->candidate_y == 0x4444) ||
        !expect(probed_slots == std::vector<std::uint16_t>{12})) {
        return 2;
    }

    probed_slots.clear();
    const auto blocked_fallback = evaluate_entity_movement_route(
            0x1000, policy,
            [](const std::uint16_t) {
                return DirectionalMovementRoute{1, -1, 4, 5};
            },
            [&probed_slots](const std::uint16_t slot) {
                probed_slots.push_back(slot);
                return MovementProbeResult{2, 0x5555, 0x6666,
                                           static_cast<std::uint8_t>(slot)};
            });
    if (!expect(blocked_fallback.carry_set) ||
        !expect(blocked_fallback.fallback_probe.has_value()) ||
        !expect(probed_slots == std::vector<std::uint16_t>{10})) {
        return 3;
    }

    probed_slots.clear();
    const auto active_route = evaluate_entity_movement_route(
            0x1000, policy,
            [](const std::uint16_t) {
                return DirectionalMovementRoute{1, 1, 4, 10};
            },
            [&probed_slots](const std::uint16_t slot) {
                probed_slots.push_back(slot);
                return MovementProbeResult{-9, 0x7777, 0x8888,
                                           static_cast<std::uint8_t>(slot)};
            });
    if (!expect(active_route.carry_set) ||
        !expect(!active_route.fallback_probe.has_value()) ||
        !expect(probed_slots.empty())) {
        return 4;
    }

    return 0;
}
