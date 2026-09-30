#include "shadowrun_ghidra/title_intro/title_world_helpers_conversion.hpp"

#include <algorithm>
#include <array>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::array<std::uint32_t, 12> kMatrixTransitionSpriteData{
    0x0001DB4CU, 0x0001DB56U, 0x0001DB60U, 0x0001DB6AU,
    0x0001DB6AU, 0x0001DB6AU, 0x0001DB6AU, 0x0001DB6AU,
    0x0001DB6AU, 0x0001DB88U, 0x0001DB7EU, 0x0001DB74U};

constexpr std::array<std::uint32_t, 5> kTitleFigureLeftUpper{
    0x000E9EFCU, 0x000EA050U, 0x000EA194U, 0x000EA308U, 0x000EA43CU};
constexpr std::array<std::uint32_t, 5> kTitleFigureRightUpper{
    0x000E9F26U, 0x000EA072U, 0x000EA1BEU, 0x000EA332U, 0x000EA466U};
constexpr std::array<std::uint32_t, 5> kTitleFigureLeftLower{
    0x000EA578U, 0x000EA65CU, 0x000EA720U, 0x000EA7F4U, 0x000EA8E8U};
constexpr std::array<std::uint32_t, 5> kTitleFigureRightLower{
    0x000EA59AU, 0x000EA67EU, 0x000EA74AU, 0x000EA81EU, 0x000EA90AU};

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
    if (append && callbacks.append_sprite_record) {
        callbacks.append_sprite_record(sprite_key);
    }
    const auto token = callbacks.resolve_sprite_data
                           ? callbacks.resolve_sprite_data(source_table_token,
                                                            phase)
                           : resolve_title_figure_sprite_data(source_table_token,
                                                               phase);
    if (!token.has_value()) {
        return false;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleFigureSpriteBuild{
            source_table_token, sprite_key, x, y, 0x6000U, *token});
    }
    return true;
}

constexpr std::array<std::int16_t, 10> kNoiseLimits{
    0, 1, 2, 4, 6, 8, 16, 32, 64, 128};

std::optional<std::int16_t> noise_limit(const std::int16_t index) {
    if (index < 0 || index >= static_cast<std::int16_t>(kNoiseLimits.size())) {
        return std::nullopt;
    }
    return kNoiseLimits[static_cast<std::size_t>(index)];
}

bool update_noise_wave(std::vector<std::int16_t>& wave,
                       const std::size_t expected_count,
                       const std::int16_t limit,
                       std::uint32_t& random_bits) {
    if (wave.size() < expected_count) {
        return false;
    }
    std::int16_t prior_value = 0;
    for (std::size_t index = 0; index < expected_count; ++index) {
        const bool carry = (random_bits & 0x80000000U) != 0U;
        random_bits = (random_bits << 1U) | (carry ? 1U : 0U);
        auto next_value = static_cast<std::int16_t>(static_cast<std::uint16_t>(
            static_cast<std::int32_t>(wave[index]) + (carry ? -1 : 1)));
        const auto difference = static_cast<std::int16_t>(
            static_cast<std::uint16_t>(static_cast<std::int32_t>(prior_value) -
                                       next_value));
        if (difference < 0) {
            const auto magnitude = static_cast<std::int16_t>(
                static_cast<std::uint16_t>(-
                    static_cast<std::int32_t>(difference)));
            if (limit <= magnitude) {
                next_value = static_cast<std::int16_t>(
                    static_cast<std::uint16_t>(next_value - 1));
            }
        } else if (limit <= difference) {
            next_value = static_cast<std::int16_t>(
                static_cast<std::uint16_t>(next_value + 1));
        }
        wave[index] = next_value;
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

std::optional<std::uint32_t> resolve_title_figure_sprite_data(
    const std::uint32_t source_table_token, const std::uint16_t phase) {
    const std::array<std::uint32_t, 5>* table = nullptr;
    switch (source_table_token) {
    case 0x0001DAF4U:
        table = &kTitleFigureLeftUpper;
        break;
    case 0x0001DB08U:
        table = &kTitleFigureRightUpper;
        break;
    case 0x000EA490U:
        table = &kTitleFigureLeftLower;
        break;
    case 0x000EA4A4U:
        table = &kTitleFigureRightLower;
        break;
    default:
        return std::nullopt;
    }
    if (phase >= table->size()) {
        return std::nullopt;
    }
    return (*table)[phase] + 8U;
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
        state.remaining_frames = static_cast<std::int8_t>(
            state.remaining_frames - 1);
        if (state.remaining_frames == 0U) {
            state.record_flags = static_cast<std::uint8_t>(
                state.record_flags & ~0x04U);
            if (callbacks.finalize_menu_slot_code_six) {
                callbacks.finalize_menu_slot_code_six();
            }
            if (callbacks.reset_word_stream_area) {
                callbacks.reset_word_stream_area();
            }
            state.dirty_flags = static_cast<std::uint8_t>(
                state.dirty_flags | 0x0CU);
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
    const auto first_limit = noise_limit(static_cast<std::int16_t>(
        state.remaining_frames) + 1);
    if (!first_limit ||
        !update_noise_wave(state.cb42_wave, 0xB0U, *first_limit, random_bits)) {
        return TitleNoiseAction::waveform_unresolved;
    }

    auto second_limit_index = static_cast<std::int16_t>(
        state.remaining_frames);
    if (second_limit_index >= 3) {
        second_limit_index = 2;
    }
    const auto second_limit = noise_limit(second_limit_index);
    if (!second_limit) {
        return TitleNoiseAction::waveform_unresolved;
    }
    const auto third_random = callbacks.advance_source_prng();
    const auto fourth_random = callbacks.advance_source_prng();
    if (!third_random.has_value() || !fourth_random.has_value()) {
        return TitleNoiseAction::prng_unresolved;
    }
    random_bits = (static_cast<std::uint32_t>(*third_random) << 16U) |
                  *fourth_random;
    if (!update_noise_wave(state.cb40_wave, 0xC9U, *second_limit,
                           random_bits)) {
        return TitleNoiseAction::waveform_unresolved;
    }
    if (callbacks.write_word_stream) {
        callbacks.write_word_stream();
    }
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x0CU);
    return TitleNoiseAction::updated;
}

} // namespace shadowrun::ghidra_conversion
