#include "shadowrun_ghidra/movement_route_conversion.hpp"

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
        return static_cast<std::int16_t>(1);
    };
    if (!expect(evaluate_entity_movement_route(
            0x1000, policy, lookup, probe)) ||
        !expect(route_keys == std::vector<std::uint16_t>{0}) ||
        !expect(probed_slots == std::vector<std::uint16_t>{4})) {
        return 1;
    }

    probed_slots.clear();
    if (!expect(evaluate_entity_movement_route(
            0x1000, policy,
            [](const std::uint16_t) {
                return DirectionalMovementRoute{1, 0, 4, 8};
            },
            [&probed_slots](const std::uint16_t slot) {
                probed_slots.push_back(slot);
                return static_cast<std::int16_t>(-9);
            }) == false) ||
        !expect(probed_slots == std::vector<std::uint16_t>{8})) {
        return 2;
    }

    probed_slots.clear();
    if (!expect(evaluate_entity_movement_route(
            0x1000, policy,
            [](const std::uint16_t) {
                return DirectionalMovementRoute{1, -1, 4, 10};
            },
            [&probed_slots](const std::uint16_t slot) {
                probed_slots.push_back(slot);
                return static_cast<std::int16_t>(2);
            })) ||
        !expect(probed_slots == std::vector<std::uint16_t>{10})) {
        return 3;
    }

    probed_slots.clear();
    if (!expect(evaluate_entity_movement_route(
            0x1000, policy,
            [](const std::uint16_t) {
                return DirectionalMovementRoute{1, 1, 4, 10};
            },
            [&probed_slots](const std::uint16_t slot) {
                probed_slots.push_back(slot);
                return static_cast<std::int16_t>(-9);
            })) ||
        !expect(probed_slots.empty())) {
        return 4;
    }

    return 0;
}
