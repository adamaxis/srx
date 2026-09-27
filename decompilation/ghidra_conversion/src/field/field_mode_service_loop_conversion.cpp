#include "shadowrun_ghidra/field/field_mode_service_loop_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

constexpr std::uint16_t bit(const FieldModeServiceInput input) {
    return static_cast<std::uint16_t>(input);
}

void restore_field_presentation(const FieldModeServiceCallbacks& callbacks) {
    call_if_present(callbacks.upload_field_asset_sequence);
    call_if_present(callbacks.refresh_field_animation_assets);
    call_if_present(callbacks.upload_field_asset_setup);
    call_if_present(callbacks.set_display_control, 0x9191U);
}

void prepare_service_setup(FieldModeServiceState& state,
                           const FieldModeServiceCallbacks& callbacks) {
    state.navigation.panel_cursor = 0;
    call_if_present(callbacks.prepare_numeric_menu_block);
}

void render_current_selection(FieldModeServiceState& state,
                              const FieldModeServiceCallbacks& callbacks) {
    // 0x587DA stores FFF0D0 in FFF0B8 before it chooses the presentation
    // branch. Keep that state transition outside the unresolved renderer.
    state.navigation.saved_selection = state.navigation.primary_cursor;
    call_if_present(callbacks.render_selection_presentation, state);
}

bool has_action_input(const std::uint16_t input) {
    return (input & (bit(FieldModeServiceInput::confirm) |
                     bit(FieldModeServiceInput::alternate_confirm))) != 0U;
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

    call_if_present(callbacks.set_display_control, 0x9100U);
    call_if_present(callbacks.transfer_menu_asset_batch);
    call_if_present(callbacks.upload_resource_pair);
    call_if_present(callbacks.refresh_mode_palette);
    if (state.navigation.mode_result == 1) {
        call_if_present(callbacks.write_wrapped_resource_block);
    }
    if (state.navigation.mode_result == 2) {
        call_if_present(callbacks.refresh_mode_resource_metadata);
        call_if_present(callbacks.refresh_mode_resource_state);
        call_if_present(callbacks.decode_and_upload_resource);
    }
    call_if_present(callbacks.load_font);
    call_if_present(callbacks.render_encoded_text);

    for (;;) {
        if (state.navigation.mode_result == 0) {
            call_if_present(callbacks.select_numeric_menu_table);
        }
        call_if_present(callbacks.reset_upload_state);
        state.service_result = 0;
        state.navigation.primary_cursor = 0;
        state.navigation.saved_selection = 0;
        state.selection_marker = -1;
        if (callbacks.read_mode_palette_bank) {
            copy_field_mode_palette_bank(callbacks.read_mode_palette_bank(),
                                            state.palette_bank_words,
                                            state.navigation.display_dirty_flags);
        }
        call_if_present(callbacks.draw_menu_panel);
        prepare_service_setup(state, callbacks);

        if (state.navigation.mode_result == 2 && !state.has_deck) {
            call_if_present(callbacks.render_encoded_text);
            call_if_present(callbacks.wait_for_controller_event);
            restore_field_presentation(callbacks);
            state.returned_to_field = true;
            return FieldModeServiceLoopAction::returned_to_field;
        }

        if (state.navigation.mode_result == 1) {
            state.navigation.panel_cursor = 1;
            refresh_field_mode_selection_sprites(state.navigation,
                                                 callbacks.navigation_callbacks);
        } else {
            refresh_field_mode_panel_grid(state.navigation,
                                          callbacks.navigation_callbacks);
            render_field_mode_selection(state.navigation,
                                        callbacks.navigation_callbacks);
        }
        render_current_selection(state, callbacks);

        for (;;) {
            call_if_present(callbacks.wait_for_vblank);
            call_if_present(callbacks.latch_controller_events);
            const auto input = callbacks.sample_controller_input
                                   ? callbacks.sample_controller_input()
                                   : std::uint16_t{0};

            if ((input & bit(FieldModeServiceInput::primary_decrement)) != 0U) {
                call_if_present(callbacks.service_audio);
                advance_field_mode_primary_cursor(
                    state.navigation, -1, callbacks.navigation_callbacks);
                // ROM 0x56C54 gives this direction branch its own return to
                // the vblank loop. Do not consume lower-priority bits from
                // the same sampled controller word.
                continue;
            }
            if ((input & bit(FieldModeServiceInput::panel_decrement)) != 0U) {
                call_if_present(callbacks.service_audio);
                if (state.navigation.mode_result != 1) {
                    advance_field_mode_panel_cursor(
                        state.navigation, -1, callbacks.navigation_callbacks);
                }
                continue;
            }
            if ((input & bit(FieldModeServiceInput::primary_increment)) != 0U) {
                call_if_present(callbacks.service_audio);
                advance_field_mode_primary_cursor(
                    state.navigation, 1, callbacks.navigation_callbacks);
                continue;
            }
            if ((input & bit(FieldModeServiceInput::panel_increment)) != 0U) {
                call_if_present(callbacks.service_audio);
                if (state.navigation.mode_result != 1) {
                    advance_field_mode_panel_cursor(
                        state.navigation, 1, callbacks.navigation_callbacks);
                }
                continue;
            }

            if (!has_action_input(input)) {
                if ((input & bit(FieldModeServiceInput::restart_setup)) != 0U) {
                    call_if_present(callbacks.service_audio);
                    call_if_present(callbacks.restart_graphics_service);
                    break;
                }
                if ((input & bit(FieldModeServiceInput::return_to_field)) != 0U) {
                    restore_field_presentation(callbacks);
                    state.returned_to_field = true;
                    return FieldModeServiceLoopAction::returned_to_field;
                }
                continue;
            }

            call_if_present(callbacks.service_audio);
            if (state.navigation.panel_cursor == 0) {
                if (state.navigation.primary_cursor == 3 &&
                    state.navigation.mode_result < 0 &&
                    state.selection_marker < 0) {
                    call_if_present(callbacks.render_encoded_text);
                } else {
                    render_current_selection(state, callbacks);
                }
                continue;
            }

            call_if_present(callbacks.run_selected_action, state);
            if (state.service_result < 0) {
                continue;
            }
            call_if_present(callbacks.reset_upload_state);
            state.navigation.primary_cursor = state.navigation.saved_selection;
            break;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
