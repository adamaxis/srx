#include "shadowrun_ghidra/scene_profile_conversion.hpp"

namespace shadowrun::ghidra_conversion {

bool scene_uses_alternate_profile(const std::int16_t scene_id) {
    return (scene_id >= 6 && scene_id <= 9) ||
           (scene_id >= 0x2C && scene_id <= 0x31);
}

void apply_scene_profile_selection(
    SceneProfileState& state,
    std::optional<std::array<std::uint16_t, 8>>& pending_profile,
    const SceneProfileDefaults& defaults) {
    if (!pending_profile.has_value()) {
        state.camera_x = defaults.camera_x;
        state.camera_y = defaults.camera_y;
        state.runner_positions = defaults.runner_positions;
        return;
    }

    const auto& words = *pending_profile;
    state.camera_x = static_cast<std::uint16_t>(words[0] & 0xFF80U);
    state.camera_y = static_cast<std::uint16_t>(words[1] & 0xFF80U);
    for (std::size_t index = 0; index < state.runner_positions.size();
         ++index) {
        state.runner_positions[index] = {words[2U + index * 2U],
                                         words[3U + index * 2U]};
    }
    state.camera_transition_x = -1;
    state.camera_transition_y = -1;
    pending_profile.reset();
}

} // namespace shadowrun::ghidra_conversion
