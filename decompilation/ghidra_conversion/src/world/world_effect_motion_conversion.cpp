#include "shadowrun_ghidra/world/world_effect_motion_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::array<std::uint8_t, 32> kWorldEffectMotionSourceIndices{
    0U, 0U, 1U, 1U, 1U, 2U, 2U, 2U,
    2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U,
    2U, 2U, 3U, 3U, 3U, 3U, 3U, 3U,
    3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U};
constexpr std::array<std::uint32_t, 9> kWorldEffectMotionSpriteData{
    0x0001D862U, 0x0001D86CU, 0x0001D876U,
    0x0001D880U, 0x0001D88AU, 0x0001D894U,
    0x0001D89EU, 0x0001D8A8U, 0x0001D8B2U};

} // namespace

std::optional<std::uint32_t> resolve_world_effect_motion_sprite_data(
    const std::uint16_t phase, const bool horizontal_motion_is_negative) {
    if (phase >= kWorldEffectMotionSourceIndices.size()) {
        return std::nullopt;
    }
    auto source_index = kWorldEffectMotionSourceIndices[phase];
    if (!horizontal_motion_is_negative) {
        source_index = static_cast<std::uint8_t>(source_index + 5U);
    }
    if (source_index >= kWorldEffectMotionSpriteData.size()) {
        return std::nullopt;
    }
    return kWorldEffectMotionSpriteData[source_index];
}

WorldEffectMotionAction advance_world_effect_motion(
    WorldEffectMotionState& state,
    const WorldEffectMotionCallbacks& callbacks) {
    state.frame_countdown = static_cast<std::uint16_t>(
        state.frame_countdown - 1U);
    if ((state.frame_countdown & 0x8000U) != 0U) {
        state.completion_marker = 0U;
        state.transition_state = 0U;
        if (callbacks.run_menu_selection_service) {
            callbacks.run_menu_selection_service();
        }
        return WorldEffectMotionAction::finalized;
    }

    ++state.motion_phase;
    const auto sprite = callbacks.resolve_sprite_data
                            ? callbacks.resolve_sprite_data(
                                  state.motion_phase,
                                  state.horizontal_offset < 0)
                            : resolve_world_effect_motion_sprite_data(
                                  state.motion_phase,
                                  state.horizontal_offset < 0);
    if (!sprite.has_value()) {
        return WorldEffectMotionAction::sprite_data_unresolved;
    }
    state.position_x = static_cast<std::uint16_t>(
        state.position_x + state.horizontal_offset);
    state.position_y = static_cast<std::uint16_t>(state.position_y - 0x001AU);
    if (callbacks.build_sprite) {
        callbacks.build_sprite(*sprite,
                               static_cast<std::uint16_t>(
                                   state.position_x >> 4U),
                               static_cast<std::uint16_t>(
                                   state.position_y >> 4U),
                               0x2000U);
    }
    return WorldEffectMotionAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
