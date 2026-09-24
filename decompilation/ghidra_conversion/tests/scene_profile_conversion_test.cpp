#include "shadowrun_ghidra/scene_profile_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(scene_uses_alternate_profile(6) &&
                scene_uses_alternate_profile(9) &&
                scene_uses_alternate_profile(0x2C) &&
                scene_uses_alternate_profile(0x31) &&
                !scene_uses_alternate_profile(5) &&
                !scene_uses_alternate_profile(10) &&
                !scene_uses_alternate_profile(0x2B) &&
                !scene_uses_alternate_profile(0x32) &&
                !scene_uses_alternate_profile(-1))) {
        return 1;
    }

    const SceneProfileDefaults defaults{
        0x1111U, 0x2222U, {{{0x0100U, 0x0200U}, {0x0300U, 0x0400U},
                            {0x0500U, 0x0600U}}}};
    SceneProfileState state{0U, 0U, {}, 0x1234, 0x5678};
    std::optional<std::array<std::uint16_t, 8>> no_pending_profile;
    apply_scene_profile_selection(state, no_pending_profile, defaults);
    if (!expect(state.camera_x == 0x1111U && state.camera_y == 0x2222U &&
                state.runner_positions[2].y == 0x0600U &&
                state.camera_transition_x == 0x1234 &&
                state.camera_transition_y == 0x5678)) {
        return 2;
    }

    auto pending_profile = std::optional<std::array<std::uint16_t, 8>>{
        std::array<std::uint16_t, 8>{0x12FFU, 0x23FFU, 0x1001U, 0x1002U,
                                     0x2001U, 0x2002U, 0x3001U, 0x3002U}};
    apply_scene_profile_selection(state, pending_profile, defaults);
    if (!expect(state.camera_x == 0x1280U && state.camera_y == 0x2380U &&
                state.runner_positions[0].x == 0x1001U &&
                state.runner_positions[2].y == 0x3002U &&
                state.camera_transition_x == -1 &&
                state.camera_transition_y == -1 && !pending_profile.has_value())) {
        return 3;
    }

    return 0;
}
