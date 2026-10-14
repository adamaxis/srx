#include "shadowrun_ghidra/session/runner_selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

constexpr std::uint16_t kPreviousRunner = 0x0400U; // source bit 10
constexpr std::uint16_t kNextRunner = 0x0800U; // source bit 11
constexpr std::uint16_t kConfirmRunner = 0x7000U; // source bits 12, 13, 14
constexpr std::uint16_t kReturnToMenu = 0x8000U; // source bit 15

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void initialize_runner_selection_screen(
    RunnerSelectionScreenState& state,
    const RunnerSelectionScreenCallbacks& callbacks) {
    call_once(callbacks.clear_background);
    call_once(callbacks.copy_background_asset);
    state.background_dirty = true;
    call_once(callbacks.upload_background_asset);

    if (callbacks.render_grid) {
        callbacks.render_grid({0U, 0U, true});
    }
    call_once(callbacks.render_header_text);
    call_once(callbacks.update_runner_animation);
    call_once(callbacks.build_sprite_records_once);
    call_once(callbacks.select_primary_runner_record_table);

    if (callbacks.render_grid) {
        for (std::uint8_t row = 0; row < 10U; ++row) {
            for (std::uint8_t column = 0; column < 14U; ++column) {
                callbacks.render_grid({row, column, false});
            }
        }
    }
}

bool refresh_runner_selection_sprites(
    RunnerSelectionScreenState& state,
    const RunnerSelectionScreenCallbacks& callbacks) {
    call_once(callbacks.build_sprite_records_once);
    if (!callbacks.resolve_selected_runner_sprite_source) {
        return false;
    }

    const auto source =
        callbacks.resolve_selected_runner_sprite_source(state.active_runner);
    if (!source.has_value()) {
        return false;
    }

    if (callbacks.build_runner_sprites) {
        callbacks.build_runner_sprites({
            static_cast<std::int16_t>(static_cast<std::int16_t>(source->screen_x) * 0x10),
            static_cast<std::int16_t>(static_cast<std::int16_t>(source->screen_y) * 0x10),
            0x2000U,
        });
    }
    state.sprite_records_dirty = true;
    return true;
}

RunnerSelectionResult run_runner_selection_screen(
    RunnerSelectionScreenState& state,
    const RunnerSelectionScreenCallbacks& callbacks,
    std::int32_t* menu_result) {
    initialize_runner_selection_screen(state, callbacks);
    refresh_runner_selection_sprites(state, callbacks);

    if (!callbacks.sample_controller_events) {
        return RunnerSelectionResult::unavailable;
    }

    for (;;) {
        call_once(callbacks.wait_for_vblank);
        call_once(callbacks.latch_controller_events);
        const auto events = callbacks.sample_controller_events();

        if ((events & kPreviousRunner) != 0U) {
            if (callbacks.move_runner_selection) {
                const auto new_runner = callbacks.move_runner_selection(-1);
                if (new_runner) {
                    state.active_runner = *new_runner;
                    refresh_runner_selection_sprites(state, callbacks);
                }
            }
            continue;
        }

        if ((events & kConfirmRunner) != 0U) {
            state.selection_published = true;
            call_once(callbacks.publish_runner_selection);
            if (!state.alternate_presenter) {
                call_once(callbacks.render_field_hud);
            }
            return RunnerSelectionResult::runner_selected;
        }

        if ((events & kNextRunner) != 0U) {
            if (callbacks.move_runner_selection) {
                const auto new_runner = callbacks.move_runner_selection(1);
                if (new_runner) {
                    state.active_runner = *new_runner;
                }
            }
            refresh_runner_selection_sprites(state, callbacks);
            continue;
        }

        if ((events & kReturnToMenu) != 0U && !state.alternate_presenter) {
            call_once(callbacks.upload_menu_resources);
            const auto result = callbacks.render_menu_footer
                                    ? callbacks.render_menu_footer()
                                    : 0;
            if (menu_result) {
                *menu_result = result;
            }
            return RunnerSelectionResult::menu_returned;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
