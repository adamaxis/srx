#include "shadowrun_ghidra/collision_probe_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<CollisionProbeOffset, 3> offsets{
        CollisionProbeOffset{-4, 2},
        CollisionProbeOffset{8, -6},
        CollisionProbeOffset{16, 10}};
    CollisionProbeState state{
        {{{0xAAAAU, 0xBBBBU}, {0xCCCCU, 0xDDDDU}, {0xEEEEU, 0xFFFFU}}},
        0x1111U,
        0x2222U};
    build_collision_probe_points(
        state, false, 0x1000U, 0x2000U, offsets, 0x3456U, 0x789AU);
    if (!expect(state.points[0].x == 0x0FFCU &&
                state.points[0].y == 0x2002U &&
                state.points[1].x == 0x1008U &&
                state.points[1].y == 0x1FFAU &&
                state.points[2].x == 0x1010U &&
                state.points[2].y == 0x200AU &&
                state.camera_x == 0x3456U && state.camera_y == 0x789AU)) {
        return 1;
    }

    const auto before = state;
    build_collision_probe_points(
        state, true, 0x5000U, 0x6000U, offsets, 0xAAAAU, 0xBBBBU);
    if (!expect(state.points[0].x == before.points[0].x &&
                state.points[0].y == before.points[0].y &&
                state.points[1].x == before.points[1].x &&
                state.points[1].y == before.points[1].y &&
                state.points[2].x == before.points[2].x &&
                state.points[2].y == before.points[2].y &&
                state.camera_x == before.camera_x &&
                state.camera_y == before.camera_y)) {
        return 2;
    }

    return 0;
}
