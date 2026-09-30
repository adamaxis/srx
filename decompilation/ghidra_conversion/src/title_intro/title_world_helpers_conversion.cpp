#include "shadowrun_ghidra/title_intro/title_world_helpers_conversion.hpp"

#include <algorithm>
#include <array>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::array<std::uint32_t, 12> kMatrixTransitionSpriteData{
    0x0001DB4CU, 0x0001DB56U, 0x0001DB60U, 0x0001DB6AU,
    0x0001DB6AU, 0x0001DB6AU, 0x0001DB6AU, 0x0001DB6AU,
    0x0001DB6AU, 0x0001DB88U, 0x0001DB7EU, 0x0001DB74U};

bool decrement_word_until_negative(std::uint16_t& value) {
    value = static_cast<std::uint16_t>(value - 1U);
    return (value & 0x8000U) != 0U;
}

bool render_matrix_transition_sprite(
    MatrixProgramTransitionState& state,
    const MatrixProgramTransitionCallbacks& callbacks) {
    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(
                                 0x0001DB1CU, state.stage)
                           : resolve_matrix_transition_sprite_data(
                                 state.stage);
    if (!token.has_value()) {
        return false;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(MatrixTransitionSpriteBuild{
            0x0001DB1CU,
            state.stage,
            state.resource_token,
            0x0147U,
            0x2000U,
            *token});
    }
    return true;
}

bool render_figure_sprite(
    const std::uint32_t source_table_token,
    const std::uint16_t phase,
    const std::uint16_t sprite_key,
    const std::uint16_t x,
    const std::uint16_t y,
    const TitleFigureAnimationCallbacks& callbacks,
    const bool append) {
    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(source_table_token,
                                                            phase)
                           : std::nullopt;
    if (!token.has_value()) {
        return false;
    }
    if (append && callbacks.append_sprite_record) {
        callbacks.append_sprite_record(sprite_key);
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleFigureSpriteBuild{
            source_table_token, sprite_key, x, y, 0x6000U, *token});
    }
    return true;
}

constexpr std::array<std::int16_t, 10> kNoiseLimits{
    0, 1, 2, 4, 6, 8, 16, 32, 64, 128};

std::optional<std::int16_t> noise_limit(const std::uint8_t index) {
    if (index >= kNoiseLimits.size()) {
        return std::nullopt;
    }
    return kNoiseLimits[index];
}

bool update_noise_wave(std::vector<std::int16_t>& wave,
                       const std::size_t expected_count,
                       const std::int16_t limit,
                       std::uint32_t& random_bits) {
    if (wave.size() < expected_count) {
        return false;
    }
    std::int32_t prior_value = 0;
    for (std::size_t index = 0; index < expected_count; ++index) {
        const bool carry = (random_bits & 0x80000000U) != 0U;
        random_bits = (random_bits << 1U) | (carry ? 1U : 0U);
        auto next_value = static_cast<std::int32_t>(wave[index]) +
                          (carry ? -1 : 1);
        auto difference = prior_value - next_value;
        if (difference < 0) {
            if (limit <= -difference) {
                --next_value;
            }
        } else if (limit <= difference) {
            ++next_value;
        }
        wave[index] = static_cast<std::int16_t>(
            static_cast<std::uint16_t>(next_value));
        prior_value = next_value;
    }
    return true;
}

} // namespace

std::optional<std::uint32_t> resolve_matrix_transition_sprite_data(
    const std::uint16_t stage) {
    if (stage >= kMatrixTransitionSpriteData.size()) {
        return std::nullopt;
    }
    return kMatrixTransitionSpriteData[stage];
}

MatrixProgramTransitionAction advance_matrix_program_transition(
    MatrixProgramTransitionState& state,
    const MatrixProgramTransitionCallbacks& callbacks) {
    if (!decrement_word_until_negative(state.frame_countdown)) {
        return render_matrix_transition_sprite(state, callbacks)
                   ? MatrixProgramTransitionAction::rendered
                   : MatrixProgramTransitionAction::sprite_data_unresolved;
    }

    state.frame_countdown = 0x000CU;
    ++state.stage;
    if (state.stage == 6U) {
        if (callbacks.clear_program_slot) {
            callbacks.clear_program_slot(state.selected_slot);
        }
        if (callbacks.refresh_program_grid) {
            callbacks.refresh_program_grid();
        }
    }
    if (state.stage >= 0x000CU) {
        if (callbacks.refresh_mode_glyph) {
            callbacks.refresh_mode_glyph();
        }
        state.mode_request_pending = false;
        if (callbacks.render_menu_rows) {
            callbacks.render_menu_rows();
        }
        if (callbacks.initialize_world_effect_mode) {
            callbacks.initialize_world_effect_mode();
        }
        return MatrixProgramTransitionAction::finalized;
    }
    return render_matrix_transition_sprite(state, callbacks)
               ? MatrixProgramTransitionAction::rendered
               : MatrixProgramTransitionAction::sprite_data_unresolved;
}

TitleFigureAnimationAction advance_title_figure_animation(
    TitleFigureAnimationState& state,
    const TitleFigureAnimationCallbacks& callbacks) {
    if (callbacks.advance_support_animation) {
        callbacks.advance_support_animation();
    }
    if (decrement_word_until_negative(state.frame_countdown)) {
        state.frame_countdown = 0U;
        ++state.phase;
        if (state.phase > 4U) {
            state.phase = 0U;
        }
    }

    if (!render_figure_sprite(0x0001DAF4U, state.phase, 0x0780U, 0x00F8U,
                              0x00E4U, callbacks, true) ||
        !render_figure_sprite(0x0001DB08U, state.phase, 0x0780U, 0x0148U,
                              0x00E4U, callbacks, false) ||
        !render_figure_sprite(0x000EA490U, state.phase, 0x07C0U, 0x00E2U,
                              0x0114U, callbacks, true) ||
        !render_figure_sprite(0x000EA4A4U, state.phase, 0x07C0U, 0x015EU,
                              0x0114U, callbacks, false)) {
        return TitleFigureAnimationAction::sprite_data_unresolved;
    }
    return TitleFigureAnimationAction::rendered;
}

TitleBurstAnimationAction advance_title_burst_animation(
    TitleBurstAnimationState& state,
    const TitleBurstAnimationCallbacks& callbacks) {
    if (decrement_word_until_negative(state.frame_countdown)) {
        state.frame_countdown = 4U;
        ++state.phase;
        if (state.phase >= 4U) {
            if (callbacks.clear_completion_marker) {
                callbacks.clear_completion_marker();
            }
            return TitleBurstAnimationAction::completed;
        }
    }

    if (callbacks.append_sprite_record) {
        callbacks.append_sprite_record(0x0780U);
    }
    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(0x0001D970U,
                                                            state.phase)
                           : std::nullopt;
    if (!token.has_value()) {
        return TitleBurstAnimationAction::sprite_data_unresolved;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleBurstSpriteBuild{
            0x0001D970U, state.phase, 0x0780U, state.position_x,
            state.position_y, 0x6000U, *token});
    }
    return TitleBurstAnimationAction::rendered;
}

TitleNoiseAction advance_title_noise_stream(
    TitleNoiseState& state,
    const TitleNoiseCallbacks& callbacks) {
    if (decrement_word_until_negative(state.frame_countdown)) {
        state.frame_countdown = state.frame_reload;
        state.remaining_frames = static_cast<std::uint8_t>(
            state.remaining_frames - 1U);
        if (state.remaining_frames == 0U) {
            state.record_flags = static_cast<std::uint8_t>(
                state.record_flags & ~0x04U);
            if (callbacks.finalize_menu_slot_code_six) {
                callbacks.finalize_menu_slot_code_six();
            }
            if (callbacks.reset_word_stream_area) {
                callbacks.reset_word_stream_area();
            }
            return TitleNoiseAction::finalized;
        }
    }

    if (!callbacks.advance_source_prng) {
        return TitleNoiseAction::prng_unresolved;
    }
    const auto first_random = callbacks.advance_source_prng();
    const auto second_random = callbacks.advance_source_prng();
    if (!first_random.has_value() || !second_random.has_value()) {
        return TitleNoiseAction::prng_unresolved;
    }
    auto random_bits = (static_cast<std::uint32_t>(*first_random) << 16U) |
                       *second_random;
    const auto first_limit = noise_limit(static_cast<std::uint8_t>(
        state.remaining_frames + 1U));
    if (!first_limit ||
        !update_noise_wave(state.cb42_wave, 0xB0U, *first_limit, random_bits)) {
        return TitleNoiseAction::waveform_unresolved;
    }

    const auto second_limit = kNoiseLimits[
        std::min<std::uint8_t>(state.remaining_frames, 2U)];
    const auto third_random = callbacks.advance_source_prng();
    const auto fourth_random = callbacks.advance_source_prng();
    if (!third_random.has_value() || !fourth_random.has_value()) {
        return TitleNoiseAction::prng_unresolved;
    }
    random_bits = (static_cast<std::uint32_t>(*third_random) << 16U) |
                  *fourth_random;
    if (!update_noise_wave(state.cb40_wave, 0xC9U, second_limit, random_bits)) {
        return TitleNoiseAction::waveform_unresolved;
    }
    if (callbacks.write_word_stream) {
        callbacks.write_word_stream();
    }
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x0CU);
    return TitleNoiseAction::updated;
}

} // namespace shadowrun::ghidra_conversion
