#include "shadowrun_ghidra/title_intro/title_world_helpers_conversion.hpp"

#include <cstdint>
#include <optional>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    MatrixProgramTransitionState matrix{};
    matrix.frame_countdown = 0U;
    matrix.stage = 5U;
    matrix.selected_slot = 3U;
    matrix.resource_token = 0x42U;
    int clears = 0;
    int grids = 0;
    int finalizes = 0;
    MatrixProgramTransitionCallbacks matrix_callbacks{};
    matrix_callbacks.clear_program_slot = [&](const std::uint16_t slot) {
        clears += slot == 3U ? 1 : 100;
    };
    matrix_callbacks.refresh_program_grid = [&] { ++grids; };
    matrix_callbacks.resolve_sprite_data =
        [](const std::uint16_t stage) -> std::optional<std::uint32_t> {
        return 0x1DB1CU + stage;
    };
    if (advance_matrix_program_transition(matrix, matrix_callbacks) !=
            MatrixProgramTransitionAction::rendered ||
        matrix.stage != 6U || matrix.frame_countdown != 0x0CU || clears != 1 ||
        grids != 1) {
        return 1;
    }
    matrix.stage = 0x0BU;
    matrix.frame_countdown = 0U;
    matrix_callbacks.refresh_mode_glyph = [&] { ++finalizes; };
    matrix_callbacks.render_menu_rows = [&] { ++finalizes; };
    matrix_callbacks.initialize_world_effect_mode = [&] { ++finalizes; };
    matrix.mode_request_pending = true;
    if (advance_matrix_program_transition(matrix, matrix_callbacks) !=
            MatrixProgramTransitionAction::finalized ||
        matrix.mode_request_pending || finalizes != 3) {
        return 2;
    }

    TitleFigureAnimationState figure{};
    figure.frame_countdown = 0U;
    int support = 0;
    int figure_appends = 0;
    int figure_builds = 0;
    TitleFigureAnimationCallbacks figure_callbacks{};
    figure_callbacks.advance_support_animation = [&] { ++support; };
    figure_callbacks.resolve_sprite_data =
        [](const std::uint32_t table, const std::uint16_t phase)
        -> std::optional<std::uint32_t> { return table + phase; };
    figure_callbacks.append_sprite_record =
        [&](const std::uint16_t key) {
            figure_appends += (key == 0x780U || key == 0x7C0U) ? 1 : 100;
        };
    figure_callbacks.build_sprite =
        [&](const TitleFigureSpriteBuild&) { ++figure_builds; };
    if (advance_title_figure_animation(figure, figure_callbacks) !=
            TitleFigureAnimationAction::rendered ||
        figure.phase != 1U || support != 1 || figure_appends != 2 ||
        figure_builds != 4) {
        return 3;
    }

    TitleBurstAnimationState burst{};
    burst.frame_countdown = 0U;
    burst.position_x = 0x004AU;
    burst.position_y = 0x004CU;
    int burst_appends = 0;
    int burst_builds = 0;
    int burst_finished = 0;
    TitleBurstAnimationCallbacks burst_callbacks{};
    burst_callbacks.resolve_sprite_data =
        [](const std::uint32_t table, const std::uint16_t phase)
        -> std::optional<std::uint32_t> {
        return table + phase + 8U;
    };
    burst_callbacks.append_sprite_record = [&](const std::uint16_t key) {
        burst_appends += key == 0x0780U ? 1 : 100;
    };
    burst_callbacks.build_sprite = [&](const TitleBurstSpriteBuild& build) {
        if (build.source_table_token != 0x1D970U || build.phase != 1U ||
            build.x != 0x004AU || build.y != 0x004CU ||
            build.build_flags != 0x6000U ||
            build.sprite_data_token != 0x1D979U) {
            burst_builds += 100;
        } else {
            ++burst_builds;
        }
    };
    burst_callbacks.clear_completion_marker = [&] { ++burst_finished; };
    if (advance_title_burst_animation(burst, burst_callbacks) !=
            TitleBurstAnimationAction::rendered ||
        burst.phase != 1U || burst.frame_countdown != 4U ||
        burst_appends != 1 || burst_builds != 1 || burst_finished != 0) {
        return 4;
    }
    burst.phase = 3U;
    burst.frame_countdown = 0U;
    if (advance_title_burst_animation(burst, burst_callbacks) !=
            TitleBurstAnimationAction::completed ||
        burst.phase != 4U || burst_appends != 1 || burst_builds != 1 ||
        burst_finished != 1) {
        return 5;
    }

    TitleNoiseState noise{};
    noise.frame_countdown = 1U;
    noise.frame_reload = 4U;
    noise.remaining_frames = 3U;
    noise.cb42_wave.assign(0xB0U, 0);
    noise.cb40_wave.assign(0xC9U, 0);
    int random_calls = 0;
    int writes = 0;
    TitleNoiseCallbacks noise_callbacks{};
    noise_callbacks.advance_source_prng = [&]() -> std::optional<std::uint16_t> {
        ++random_calls;
        return 0U;
    };
    noise_callbacks.write_word_stream = [&] { ++writes; };
    if (advance_title_noise_stream(noise, noise_callbacks) !=
            TitleNoiseAction::updated ||
        random_calls != 4 || writes != 1 || noise.dirty_flags != 0x0CU ||
        noise.frame_countdown != 0U) {
        return 6;
    }

    TitleNoiseState finish{};
    finish.frame_countdown = 0U;
    finish.frame_reload = 3U;
    finish.remaining_frames = 1U;
    finish.record_flags = 0xFFU;
    int finished = 0;
    finish.cb42_wave.assign(0xB0U, 0);
    finish.cb40_wave.assign(0xC9U, 0);
    noise_callbacks = {};
    noise_callbacks.finalize_menu_slot_code_six = [&] { ++finished; };
    noise_callbacks.reset_word_stream_area = [&] { ++finished; };
    if (advance_title_noise_stream(finish, noise_callbacks) !=
            TitleNoiseAction::finalized ||
        (finish.record_flags & 0x04U) != 0U || finished != 2) {
        return 7;
    }
    return 0;
}
