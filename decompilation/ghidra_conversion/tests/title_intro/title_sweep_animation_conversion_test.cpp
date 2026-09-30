#include "shadowrun_ghidra/title_intro/title_sweep_animation_conversion.hpp"

#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto source_frame_zero = resolve_title_sweep_frame(0U);
    const auto source_frame_one = resolve_title_sweep_frame(1U);
    const auto source_frame_sweep = resolve_title_sweep_frame(8U);
    const auto source_frame_end = resolve_title_sweep_frame(31U);
    if (!source_frame_zero.has_value() ||
        source_frame_zero->sprite_data_token != 0x1D480U ||
        source_frame_zero->horizontal_offset != 0 ||
        source_frame_zero->next_delay != 5U || !source_frame_one.has_value() ||
        source_frame_one->sprite_data_token != 0x1D47CU ||
        source_frame_one->next_delay != 20U || !source_frame_sweep.has_value() ||
        source_frame_sweep->sprite_data_token != 0x1D4A8U ||
        source_frame_sweep->horizontal_offset != -6 ||
        !source_frame_end.has_value() ||
        source_frame_end->sprite_data_token != 0x1D47CU ||
        resolve_title_sweep_frame(32U).has_value()) {
        return 1;
    }

    TitleSweepAnimationState sweep{};
    sweep.frame_delay = 0;
    sweep.phase = 0x03U;
    sweep.base_x = 0x100U;
    sweep.base_y = 0x200U;
    int builds = 0;
    TitleSweepAnimationCallbacks callbacks{};
    callbacks.resolve_frame =
        [](const std::uint16_t phase) -> std::optional<TitleSweepFrame> {
        return TitleSweepFrame{0x1C6BCU, 6, 3, 0x4321U + phase};
    };
    callbacks.build_sprite = [&](const TitleSweepSpriteBuild& sprite) {
        if (sprite.phase != 4U || sprite.y != 0x200U ||
            sprite.build_flags != 0x6000U ||
            (sprite.x != 0x106U && sprite.x != 0x0FAU)) {
            builds += 100;
        }
        ++builds;
    };
    if (advance_title_sweep_animation(sweep, callbacks) !=
            TitleSweepAnimationAction::rendered ||
        sweep.phase != 4U || sweep.frame_delay != 3 || builds != 2) {
        return 2;
    }

    sweep.record_flags = 0x08U;
    sweep.frame_delay = 0;
    builds = 0;
    if (advance_title_sweep_animation(sweep, callbacks) !=
            TitleSweepAnimationAction::waiting || builds != 0) {
        return 3;
    }

    // With no resolver callback, the conversion must use the exact ROM
    // frame record rather than silently treating the source as unavailable.
    TitleSweepAnimationState extracted{};
    extracted.frame_delay = 0;
    extracted.base_x = 0x120U;
    extracted.base_y = 0x220U;
    int extracted_builds = 0;
    TitleSweepAnimationCallbacks extracted_callbacks{};
    extracted_callbacks.build_sprite =
        [&](const TitleSweepSpriteBuild& sprite) {
            if (sprite.phase != 1U || sprite.sprite_data_token != 0x1D47CU ||
                sprite.x != 0x120U || sprite.y != 0x220U ||
                sprite.build_flags != 0x6000U) {
                extracted_builds += 100;
            }
            ++extracted_builds;
        };
    if (advance_title_sweep_animation(extracted, extracted_callbacks) !=
            TitleSweepAnimationAction::rendered ||
        extracted.phase != 1U || extracted.frame_delay != 20 ||
        extracted_builds != 2) {
        return 4;
    }

    TitleSweepAnimationState finish{};
    finish.phase = 0x1FU;
    finish.frame_delay = 0;
    finish.record_flags = 0xFFU;
    finish.transition_state = 0x1234U;
    int finalized = 0;
    callbacks = {};
    callbacks.attach_behavior_descriptor_b = [&] { ++finalized; };
    callbacks.return_to_title_state = [&] { ++finalized; };
    if (advance_title_sweep_animation(finish, callbacks) !=
            TitleSweepAnimationAction::finalized ||
        finish.record_flags != 0xF6U || finish.transition_state != 0U ||
        finish.frame_delay != -1 ||
        finalized != 2) {
        return 5;
    }
    return 0;
}
