#include "shadowrun_ghidra/world_effect_mode_animation_conversion.hpp"

#include <cstdint>
#include <optional>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectModeAnimationState default_mode{};
    default_mode.mode = 1U;
    default_mode.frame_countdown = 2;
    default_mode.position_x = 0x0120U;
    default_mode.position_y = 0x0100U;
    default_mode.render_flags = 0x0002U;
    std::vector<WorldEffectModeSpriteBuild> builds;
    int appends = 0;
    WorldEffectModeAnimationCallbacks callbacks{};
    callbacks.append_sprite_record = [&](const std::uint16_t key) {
        appends += key == 0x06E0U ? 1 : 100;
    };
    callbacks.resolve_sprite_data =
        [](const std::uint8_t table_index) -> std::optional<std::uint32_t> {
        return 0xF1AEE000U + table_index;
    };
    callbacks.build_sprite = [&](const WorldEffectModeSpriteBuild& build) {
        builds.push_back(build);
    };
    if (advance_world_effect_mode_animation(default_mode, callbacks) !=
            WorldEffectModeAnimationAction::sprite_rendered ||
        appends != 1 || builds.size() != 1U || builds[0].table_index != 1U ||
        builds[0].x != 0x012U || builds[0].y != 0x00FU ||
        default_mode.position_x != 0x0126U ||
        default_mode.position_y != 0x00FEU) {
        return 1;
    }

    WorldEffectModeAnimationState mode_two{};
    mode_two.mode = 2U;
    mode_two.frame_countdown = 0x10;
    mode_two.position_x = 0x0100U;
    mode_two.position_y = 0x0200U;
    mode_two.render_flags = 0U;
    builds.clear();
    if (advance_world_effect_mode_animation(mode_two, callbacks) !=
            WorldEffectModeAnimationAction::sprite_rendered ||
        builds.size() != 1U || builds[0].table_index != 4U ||
        mode_two.position_x != 0x00FAU || mode_two.position_y != 0x021AU) {
        return 2;
    }

    WorldEffectModeAnimationState mode_two_late = mode_two;
    mode_two_late.frame_countdown = 0x0FU;
    builds.clear();
    if (advance_world_effect_mode_animation(mode_two_late, callbacks) !=
            WorldEffectModeAnimationAction::sprite_rendered ||
        builds.size() != 1U || builds[0].table_index != 8U) {
        return 3;
    }

    WorldEffectModeAnimationState mode_two_to_three{};
    mode_two_to_three.mode = 2U;
    mode_two_to_three.frame_countdown = 0;
    mode_two_to_three.title_mode_visible = true;
    mode_two_to_three.audio_transition_result = 0;
    int intro_calls = 0;
    int audio_command = 0;
    int title_rebuilds = 0;
    callbacks = {};
    callbacks.advance_intro_state = [&]() -> std::optional<std::int16_t> {
        ++intro_calls;
        return 0;
    };
    callbacks.service_audio_frame =
        [&](const std::uint16_t command) -> std::optional<std::int16_t> {
        audio_command = command;
        return -1;
    };
    callbacks.build_title_sprites_twice = [&] { ++title_rebuilds; };
    if (advance_world_effect_mode_animation(mode_two_to_three, callbacks) !=
            WorldEffectModeAnimationAction::title_sprites_rebuilt ||
        intro_calls != 1 || audio_command != 0x18 || title_rebuilds != 1 ||
        mode_two_to_three.mode != 3U ||
        mode_two_to_three.frame_countdown != 7 ||
        !mode_two_to_three.records_pending ||
        mode_two_to_three.refresh_countdown != 8U) {
        return 4;
    }

    WorldEffectModeAnimationState mode_three_to_five{};
    mode_three_to_five.mode = 3U;
    mode_three_to_five.frame_countdown = 0;
    mode_three_to_five.transition_state = 0U;
    int initialized = 0;
    WorldEffectModeBehaviorPair pair{};
    callbacks = {};
    callbacks.initialize_behavior_record = [&] { ++initialized; };
    callbacks.attach_mode_five_behavior_pair =
        [&](const WorldEffectModeBehaviorPair& value) { pair = value; };
    if (advance_world_effect_mode_animation(mode_three_to_five, callbacks) !=
            WorldEffectModeAnimationAction::inline_behavior_initialized ||
        mode_three_to_five.mode != 5U ||
        mode_three_to_five.frame_countdown != 0x0FU || initialized != 1 ||
        pair.draw_descriptor != 0x1C2AEU ||
        pair.update_descriptor != 0x1C2A8U) {
        return 5;
    }

    WorldEffectModeAnimationState mode_four{};
    mode_four.mode = 4U;
    mode_four.frame_countdown = 1;
    int descriptor_d = 0;
    callbacks = {};
    callbacks.attach_behavior_descriptor_d = [&] { ++descriptor_d; };
    if (advance_world_effect_mode_animation(mode_four, callbacks) !=
            WorldEffectModeAnimationAction::descriptor_d_attached ||
        descriptor_d != 1 || mode_four.mode != 4U ||
        mode_four.frame_countdown != 0) {
        return 6;
    }

    WorldEffectModeAnimationState mode_six{};
    mode_six.mode = 6U;
    mode_six.frame_countdown = 0;
    int finished = 0;
    callbacks = {};
    callbacks.finish_mode_six = [&] { ++finished; };
    if (advance_world_effect_mode_animation(mode_six, callbacks) !=
            WorldEffectModeAnimationAction::mode_finished || finished != 1 ||
        mode_six.mode != 0U) {
        return 7;
    }

    WorldEffectModeAnimationState expired_default{};
    expired_default.mode = 1U;
    expired_default.frame_countdown = 0;
    expired_default.position_x = 0x100U;
    expired_default.position_y = 0x100U;
    expired_default.render_flags = 0U;
    callbacks = {};
    callbacks.resolve_sprite_data =
        [](const std::uint8_t table_index) -> std::optional<std::uint32_t> {
        return static_cast<std::uint32_t>(table_index);
    };
    if (advance_world_effect_mode_animation(expired_default, callbacks) !=
            WorldEffectModeAnimationAction::sprite_rendered ||
        expired_default.mode != 2U || expired_default.frame_countdown != 0x1DU ||
        expired_default.position_x != 0x00FAU ||
        expired_default.position_y != 0x011AU) {
        return 8;
    }

    return 0;
}
