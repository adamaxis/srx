#include "shadowrun_ghidra/field/field_mode_service_loop_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

constexpr std::uint16_t bit(const FieldActionSelectionServiceInput input) {
    return static_cast<std::uint16_t>(input);
}

void restore_field_presentation(const FieldModeServiceCallbacks& callbacks) {
    call_if_present(callbacks.upload_field_asset_sequence);
    call_if_present(callbacks.refresh_field_animation_assets);
    call_if_present(callbacks.upload_field_asset_setup);
    call_if_present(callbacks.set_display_control, 0x9191U);
}

void prepare_field_action_service_setup(
    FieldModeServiceState& state,
    const FieldModeServiceCallbacks& callbacks) {
    state.action_selection.action_panel = 0;
    call_if_present(callbacks.prepare_numeric_menu_block);
}

void render_current_field_action_selection(
    FieldModeServiceState& state,
    const FieldModeServiceCallbacks& callbacks) {
    // 0x587DA stores the action cursor FFF0D0 in selected-action FFF0B8
    // before it chooses the presentation branch. Keep that state transition
    // outside the unresolved renderer.
    state.action_selection.selected_action = state.action_selection.action_cursor;
    call_if_present(callbacks.render_field_action_selection_presentation,
                    state);
}

bool has_field_action_confirm_input(const std::uint16_t input) {
    return (input &
            (bit(FieldActionSelectionServiceInput::confirm) |
             bit(FieldActionSelectionServiceInput::alternate_confirm))) != 0U;
}

} // namespace

void copy_field_mode_palette_bank(
    const std::array<std::uint32_t, 8>& source,
    std::array<std::uint32_t, 8>& destination,
    std::uint8_t& display_dirty_flags) {
    destination = source;
    display_dirty_flags = static_cast<std::uint8_t>(display_dirty_flags | 0x01U);
}

FieldModeServiceLoopAction run_field_mode_service(
    FieldModeServiceState& state,
    const FieldModeServiceCallbacks& callbacks) {
    state.returned_to_field = false;

    auto action_selection_callbacks = callbacks.action_selection_callbacks;
    if (callbacks.render_field_action_selection) {
        action_selection_callbacks.render_field_action_selection = [&] {
            callbacks.render_field_action_selection(state);
        };
    }

    call_if_present(callbacks.set_display_control, 0x9100U);
    call_if_present(callbacks.transfer_menu_asset_batch);
    call_if_present(callbacks.upload_resource_pair);
    call_if_present(callbacks.refresh_mode_palette);
    if (state.action_selection.menu_result == 1) {
        call_if_present(callbacks.write_wrapped_resource_block);
    }
    if (state.action_selection.menu_result == 2) {
        call_if_present(callbacks.refresh_mode_resource_metadata);
        call_if_present(callbacks.refresh_mode_resource_state);
        call_if_present(callbacks.decode_and_upload_resource);
    }
    call_if_present(callbacks.load_font);
    call_if_present(callbacks.render_encoded_text);

    for (;;) {
        if (state.action_selection.menu_result == 0) {
            call_if_present(callbacks.select_numeric_menu_table);
        }
        call_if_present(callbacks.reset_upload_state);
        state.service_result = 0;
        state.action_selection.action_cursor = 0;
        state.action_selection.selected_action = 0;
        state.selection_marker = -1;
        if (callbacks.read_mode_palette_bank) {
            copy_field_mode_palette_bank(
                callbacks.read_mode_palette_bank(), state.palette_bank_words,
                state.action_selection.display_dirty_flags);
        }
        call_if_present(callbacks.draw_menu_panel);
        prepare_field_action_service_setup(state, callbacks);

        if (state.action_selection.menu_result == 2 && !state.has_deck) {
            call_if_present(callbacks.render_encoded_text);
            call_if_present(callbacks.wait_for_controller_event);
            restore_field_presentation(callbacks);
            state.returned_to_field = true;
            return FieldModeServiceLoopAction::returned_to_field;
        }

        if (state.action_selection.menu_result == 1) {
            state.action_selection.action_panel = 1;
            refresh_field_action_selection_sprites(
                state.action_selection, action_selection_callbacks);
        } else {
            refresh_field_action_selection_panel_grid(
                state.action_selection, action_selection_callbacks);
            render_field_action_selection_panel(
                state.action_selection, action_selection_callbacks);
        }
        render_current_field_action_selection(state, callbacks);

        for (;;) {
            call_if_present(callbacks.wait_for_vblank);
            call_if_present(callbacks.latch_controller_events);
            const auto input = callbacks.sample_controller_input
                                   ? callbacks.sample_controller_input()
                                   : std::uint16_t{0};

            if ((input & bit(FieldActionSelectionServiceInput::
                                 action_cursor_decrement)) != 0U) {
                call_if_present(callbacks.service_audio);
                advance_field_action_selection_cursor(
                    state.action_selection, -1, action_selection_callbacks);
                // ROM 0x56C54 gives this direction branch its own return to
                // the vblank loop. Do not consume lower-priority bits from
                // the same sampled controller word.
                continue;
            }
            if ((input & bit(FieldActionSelectionServiceInput::
                                 action_panel_decrement)) != 0U) {
                call_if_present(callbacks.service_audio);
                if (state.action_selection.menu_result != 1) {
                    advance_field_action_selection_panel(
                        state.action_selection, -1, action_selection_callbacks);
                }
                continue;
            }
            if ((input & bit(FieldActionSelectionServiceInput::
                                 action_cursor_increment)) != 0U) {
                call_if_present(callbacks.service_audio);
                advance_field_action_selection_cursor(
                    state.action_selection, 1, action_selection_callbacks);
                continue;
            }
            if ((input & bit(FieldActionSelectionServiceInput::
                                 action_panel_increment)) != 0U) {
                call_if_present(callbacks.service_audio);
                if (state.action_selection.menu_result != 1) {
                    advance_field_action_selection_panel(
                        state.action_selection, 1, action_selection_callbacks);
                }
                continue;
            }

            if (!has_field_action_confirm_input(input)) {
                if ((input & bit(FieldActionSelectionServiceInput::
                                     restart_setup)) != 0U) {
                    call_if_present(callbacks.service_audio);
                    if (callbacks.advance_active_field_runner_selection) {
                        callbacks.advance_active_field_runner_selection();
                    } else {
                        call_if_present(callbacks.restart_graphics_service);
                    }
                    break;
                }
                if ((input & bit(FieldActionSelectionServiceInput::
                                     return_to_field)) != 0U) {
                    restore_field_presentation(callbacks);
                    state.returned_to_field = true;
                    return FieldModeServiceLoopAction::returned_to_field;
                }
                continue;
            }

            call_if_present(callbacks.service_audio);
            if (state.action_selection.action_panel == 0) {
                if (state.action_selection.action_cursor == 3 &&
                    state.action_selection.menu_result < 0 &&
                    state.selection_marker < 0) {
                    call_if_present(callbacks.render_encoded_text);
                } else {
                    render_current_field_action_selection(state, callbacks);
                }
                continue;
            }

            call_if_present(callbacks.dispatch_selected_field_action, state);
            if (state.service_result < 0) {
                continue;
            }
            call_if_present(callbacks.reset_upload_state);
            state.action_selection.action_cursor = state.action_selection.selected_action;
            break;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
