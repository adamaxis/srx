#include "shadowrun_ghidra/entities/entity_boundary_transition_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    EntityBoundaryTransitionState linked{true, 0xFFFFU, 0U};
    std::vector<int> order;
    service_entity_boundary_transition(
        linked,
        {[&order] { order.push_back(1); },
         [&order] { order.push_back(2); }});
    if (order != std::vector<int>{1, 2} || linked.continuation != 0U) {
        return 1;
    }

    EntityBoundaryTransitionState near_boundary{false, 0x003CU, 0U};
    service_entity_boundary_transition(near_boundary, {});
    if (near_boundary.continuation != 0xB45CU) {
        return 2;
    }

    EntityBoundaryTransitionState expired{false, 0x003DU, 0x1234U};
    service_entity_boundary_transition(expired, {});
    if (expired.continuation != 0x1234U) {
        return 3;
    }

    return 0;
}
