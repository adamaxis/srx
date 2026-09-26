#include "shadowrun_ghidra/menus/menu_slot_batch_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

void attach(BehaviorRecordState& state,
            const BehaviorRecordCallbacks& callbacks,
            const std::uint32_t update,
            const std::uint32_t draw,
            const bool service_audio) {
    if (service_audio) {
        call_once(callbacks.service_audio);
    }
    initialize_behavior_record(state);
    state.update_descriptor = update;
    state.draw_descriptor = draw;
    call_once(callbacks.initialize_record);
}

} // namespace

void apply_menu_slot_command(MenuSlotCommandState& state,
                             const MenuSlotCommandCallbacks& callbacks) {
    const auto slot = static_cast<std::size_t>(state.selected_slot);
    if (slot < state.slot_codes.size()) {
        switch (state.slot_codes[slot]) {
        case 3:
            state.mode_flags = static_cast<std::uint8_t>(state.mode_flags &
                                                          ~0x20U);
            state.prompt_state = 0;
            break;
        case 5:
            state.prompt_state = 0;
            break;
        case 6:
            state.mode_flags = static_cast<std::uint8_t>(state.mode_flags &
                                                          ~0x04U);
            break;
        case 7:
            state.mode_flags = static_cast<std::uint8_t>(state.mode_flags &
                                                          ~0x02U);
            state.prompt_state = 0;
            break;
        default:
            break;
        }
        state.slot_codes[slot] = 0xFFU;
        state.slot_timer = 6;
    }
    call_once(callbacks.write_repeated_grid);
    call_once(callbacks.write_grid);
    call_once(callbacks.render_rows);
}

bool clear_ready_menu_slot_timer(MenuSlotCommandState& state,
                                 const std::int8_t input_timer,
                                 const std::function<void()>& write_grid) {
    if (input_timer >= 0 && state.slot_timer != 6U) {
        return false;
    }
    state.slot_timer = 0;
    call_once(write_grid);
    return true;
}

void clear_menu_status_at(const std::function<void()>& clear_status,
                          const std::function<void()>& write_grid) {
    call_once(clear_status);
    call_once(write_grid);
}

void publish_menu_selection_then_text(
    const std::function<void()>& set_selection,
    const std::function<void()>& render_text) {
    call_once(set_selection);
    call_once(render_text);
}

void refresh_mode_metadata_leaf(const std::function<void()>& refresh) {
    call_once(refresh);
}

void run_menu_service_then_prepare(
    const std::function<void()>& menu_service,
    const std::function<void()>& prepare_sequence) {
    call_once(menu_service);
    call_once(prepare_sequence);
}

bool finalize_menu_slot_code(MenuSlotCommandState& state,
                             const std::uint8_t requested_code,
                             const std::int16_t value,
                             const MenuSlotFinalizeCallbacks& callbacks) {
    for (std::size_t index = state.slot_codes.size(); index-- > 0;) {
        if (state.slot_codes[index] != requested_code) {
            continue;
        }
        state.slot_codes[index] = 0;
        state.slot_timer = static_cast<std::uint8_t>(value * 10);
        call_once(callbacks.clear_visual);
        call_once(callbacks.refresh_number_or_grid);
        call_once(callbacks.clear_status);
        return true;
    }
    return false;
}

bool run_menu_selection_code_four(
    MenuSelectionCodeFourState& state,
    const bool source_gate_is_nonzero,
    const std::int16_t refresh_value,
    const MenuSelectionCodeFourCallbacks& callbacks) {
    if (state.selection_delta < 1) {
        if (callbacks.publish_default_selection_and_text) {
            callbacks.publish_default_selection_and_text();
        }
    } else {
        const auto mode = callbacks.select_hud_mode
                               ? callbacks.select_hud_mode()
                               : static_cast<std::uint8_t>(0xFFU);
        if (mode == 8U && source_gate_is_nonzero) {
            const auto sum = static_cast<std::int16_t>(state.footer_offset) +
                             state.selection_delta;
            const bool overflow = sum > 127 || sum < -128;
            const auto wrapped = static_cast<std::int8_t>(sum);
            if (wrapped == 0 || overflow != (wrapped < 0)) {
                state.footer_offset = 0;
            } else if (wrapped > 10) {
                state.footer_offset = 10;
            } else {
                state.footer_offset = wrapped;
            }
        } else {
            const auto sum = static_cast<std::int16_t>(state.footer_offset) +
                             state.selection_delta;
            const bool overflow = sum > 127 || sum < -128;
            const auto wrapped = static_cast<std::int8_t>(sum);
            if (wrapped == 0 || overflow != (wrapped < 0)) {
                state.footer_offset = 0;
            } else if (wrapped > 10) {
                state.footer_offset = 10;
            } else {
                state.footer_offset = wrapped;
            }
            if (callbacks.write_footer_grid) {
                callbacks.write_footer_grid();
            }
            if (callbacks.render_footer_rows) {
                callbacks.render_footer_rows();
            }
        }
        if (callbacks.publish_fixed_selection_and_text) {
            callbacks.publish_fixed_selection_and_text();
        }
    }

    for (std::size_t slot = state.slot_codes.size(); slot-- > 0U;) {
        if (state.slot_codes[slot] != 4U) {
            continue;
        }
        if (callbacks.clear_visual) {
            callbacks.clear_visual(slot);
        }
        state.pending_values[slot] = 0U;
        state.offsets[slot] = static_cast<std::int16_t>(refresh_value * 10);
        if (callbacks.refresh_number_or_grid) {
            callbacks.refresh_number_or_grid(slot);
        }
        if (callbacks.clear_status) {
            callbacks.clear_status(slot);
        }
        return true;
    }
    return false;
}

bool advance_menu_countdown(MenuCountdownState& state,
                            const std::function<void()>& on_expiry) {
    if (state.countdown == 0) {
        return false;
    }
    --state.countdown;
    if (state.countdown != 0) {
        return false;
    }
    call_once(on_expiry);
    return true;
}

void clear_selected_menu_record(std::vector<std::uint8_t>& records,
                                const std::size_t selected_index) {
    const auto offset = selected_index * 0x14U;
    if (offset < records.size()) {
        records[offset] = 0;
    }
}

void initialize_effect_defaults(EffectDefaultsState& state) {
    state.active = true;
    state.origin_x = 0x1200;
    state.origin_y = 0x0D00;
    state.horizontal_step = -10;
    state.vertical_step = 0x1E;
    state.lifetime = -1;
    state.palette = 0;
}

void reset_effect_entity(EffectEntityResetState& state) {
    state.active = true;
    state.accumulator = 0;
    state.entity_value = 0;
}

bool arm_effect_after_threshold(EffectDefaultsState& state,
                                const std::uint16_t source_value) {
    if (source_value < 0x7140U) {
        return false;
    }
    state.active = true;
    state.palette = 0;
    return true;
}

void reset_word_stream_area(
    WordStreamResetState& state,
    const std::function<void()>& write_word_stream) {
    state.words.assign(0x1C0U, 0);
    call_once(write_word_stream);
    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x0CU);
}

void initialize_behavior_record(BehaviorRecordState& state) {
    state.width = 4;
    state.height = 4;
    state.origin_x = 0;
    state.origin_y = 0;
}

void attach_behavior_descriptor_a(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks) {
    attach(state, callbacks, 0x1C33AU, 0x1C330U, true);
}

void attach_behavior_descriptor_b(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks) {
    attach(state, callbacks, 0x1C29EU, 0x1C294U, false);
}

void attach_behavior_descriptor_c(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks) {
    attach(state, callbacks, 0x1C2D2U, 0x1C2C8U, false);
}

void attach_behavior_descriptor_d(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks) {
    attach(state, callbacks, 0x1C31EU, 0x1C318U, false);
}

void attach_behavior_descriptor_e(BehaviorRecordState& state,
                                  const BehaviorRecordCallbacks& callbacks) {
    attach(state, callbacks, 0x1C34AU, 0x1C344U, false);
}

} // namespace shadowrun::ghidra_conversion
