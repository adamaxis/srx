#include "shadowrun_ghidra/entities/collision_probe_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const CollisionProbeOffsetBank offset_banks{{
        {{{-4, 2}, {8, -6}, {16, 10}}},
        {{{100, 100}, {100, 100}, {100, 100}}},
        {{{200, 200}, {200, 200}, {200, 200}}},
        {{{-20, 4}, {30, -8}, {12, 18}}}}};
    const std::array<std::uint8_t, 3> probe_bytes{0x20U, 0x40U, 0xE0U};
    CollisionProbeState state{
        {{{0xAAAAU, 0xBBBBU}, {0xCCCCU, 0xDDDDU}, {0xEEEEU, 0xFFFFU}}},
        0x1111U,
        0x2222U};
    build_collision_probe_points(
        state, false, 0x1000U, 0x2000U, probe_bytes, offset_banks,
        0x3456U, 0x789AU);
    if (!expect(state.points[0].x == 0x0FECU &&
                state.points[0].y == 0x2004U &&
                state.points[1].x == 0x101EU &&
                state.points[1].y == 0x1FF8U &&
                state.points[2].x == 0x100CU &&
                state.points[2].y == 0x2012U &&
                state.camera_x == 0x3456U && state.camera_y == 0x789AU &&
                state.selected_offset_bank == 3U)) {
        return 1;
    }

    const auto before = state;
    build_collision_probe_points(
        state, true, 0x5000U, 0x6000U, probe_bytes, offset_banks,
        0xAAAAU, 0xBBBBU);
    if (!expect(state.points[0].x == before.points[0].x &&
                state.points[0].y == before.points[0].y &&
                state.points[1].x == before.points[1].x &&
                state.points[1].y == before.points[1].y &&
                state.points[2].x == before.points[2].x &&
                state.points[2].y == before.points[2].y &&
                state.camera_x == before.camera_x &&
                state.camera_y == before.camera_y &&
                state.selected_offset_bank == 3U)) {
        return 2;
    }

    CollisionProbeState gated{};
    const std::array<std::uint8_t, 3> first_negative{0x90U, 0x10U, 0x20U};
    build_collision_probe_points(
        gated, true, 0x1000U, 0x2000U, first_negative, offset_banks,
        0x1111U, 0x2222U);
    if (!expect(gated.selected_offset_bank == 0U &&
                gated.points[0].x == 0U && gated.camera_x == 0U)) {
        return 3;
    }

    return 0;
}
