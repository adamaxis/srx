#include "shadowrun_ghidra/menus/startup_menu_conversion.hpp"

#include "shadowrun_ghidra/core/selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void call_if_present(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

bool action_pressed(const std::uint8_t sampled_controller_byte) {
    return (sampled_controller_byte & kStartupMenuActionMask) != 0U;
}

std::int8_t vertical_step(const std::uint8_t sampled_controller_byte) {
    if ((sampled_controller_byte & kStartupMenuUp) != 0U) {
        return -1;
    }
    if ((sampled_controller_byte & kStartupMenuDown) != 0U) {
        return 1;
    }
    return 0;
}

} // namespace

void enter_start_options_menu(
    StartOptionsMenuState& state,
    const StartOptionsMenuCallbacks& callbacks) {
    state.selected = StartOptionsChoice::new_game;
    call_if_present(callbacks.copy_start_options_resource_metadata);
    call_if_present(callbacks.refresh_start_options_resource_metadata);
    call_if_present(callbacks.transfer_start_options_menu_tiles);
    call_if_present(callbacks.load_startup_menu_font);
    call_if_present(callbacks.draw_new_game_load_game_grid);
    call_if_present(callbacks.draw_start_options_title);
    if (callbacks.draw_start_options_cursor) {
        callbacks.draw_start_options_cursor(state.selected);
    }
}

StartOptionsMenuEvent advance_start_options_menu(
    StartOptionsMenuState& state,
    const std::uint8_t sampled_controller_byte,
    const StartOptionsMenuCallbacks& callbacks) {
    const auto step = vertical_step(sampled_controller_byte);
    if (step != 0) {
        state.selected = static_cast<StartOptionsChoice>(
            move_wrapped_menu_cursor(
                static_cast<std::uint8_t>(state.selected), step, 1U));
        if (callbacks.draw_start_options_cursor) {
            callbacks.draw_start_options_cursor(state.selected);
        }
        if (action_pressed(sampled_controller_byte)) {
            return state.selected == StartOptionsChoice::new_game
                ? StartOptionsMenuEvent::enter_joshua_archetype_menu
                : StartOptionsMenuEvent::enter_save_load_menu;
        }
        return StartOptionsMenuEvent::selection_moved;
    }

    if (!action_pressed(sampled_controller_byte)) {
        return StartOptionsMenuEvent::waiting;
    }
    return state.selected == StartOptionsChoice::new_game
        ? StartOptionsMenuEvent::enter_joshua_archetype_menu
        : StartOptionsMenuEvent::enter_save_load_menu;
}

void enter_save_load_menu(
    SaveLoadMenuState& state,
    const SaveLoadMenuCallbacks& callbacks) {
    state.selected = SaveSlotChoice::slot_one;
    state.message = SaveLoadMessage::none;
    state.load_flag_published = false;
    call_if_present(callbacks.draw_save_load_title);
    if (callbacks.draw_save_load_cursor) {
        callbacks.draw_save_load_cursor(state.selected);
    }
}

SaveLoadMenuEvent advance_save_load_menu(
    SaveLoadMenuState& state,
    const std::uint8_t sampled_controller_byte,
    const SaveLoadMenuCallbacks& callbacks) {
    // ROM 0x59242 JMP 0xD94: after the no-save or success copy, any new
    // controller event returns to 0x59092. Empty SRAM (FFF0DC = 0xFF) then
    // BRA 0x590B6 back to New Game/Load Game.
    if (state.message != SaveLoadMessage::none) {
        if (sampled_controller_byte == 0U) {
            return SaveLoadMenuEvent::waiting;
        }
        return SaveLoadMenuEvent::message_acknowledged;
    }

    const auto step = vertical_step(sampled_controller_byte);
    if (step != 0) {
        state.selected = static_cast<SaveSlotChoice>(
            move_wrapped_menu_cursor(
                static_cast<std::uint8_t>(state.selected), step, 1U));
        if (callbacks.draw_save_load_cursor) {
            callbacks.draw_save_load_cursor(state.selected);
        }
        // 0x591A6/0x591B8 branch back through 0x591C6 after movement. The
        // source does not inspect A/B/C from that same sampled frame.
        return SaveLoadMenuEvent::selection_moved;
    } else if (!action_pressed(sampled_controller_byte)) {
        return SaveLoadMenuEvent::waiting;
    }

    // 0x59202 redraws the selected Save Slot row before 0x59360 runs. A
    // missing callback is unresolved source work, not an invented empty
    // save, so callers can report the conversion gap explicitly.
    call_if_present(callbacks.draw_save_load_grid);
    if (!callbacks.try_load_save_slot) {
        return SaveLoadMenuEvent::callback_unresolved;
    }

    const auto attempt = callbacks.try_load_save_slot(state.selected);
    if (attempt == SaveLoadAttempt::loaded) {
        state.message = SaveLoadMessage::save_loaded;
        state.load_flag_published = true;
        if (callbacks.draw_save_load_message) {
            callbacks.draw_save_load_message(state.message);
        }
        call_if_present(callbacks.wait_for_controller_event);
        return SaveLoadMenuEvent::save_loaded;
    }
    if (attempt == SaveLoadAttempt::empty_or_invalid) {
        state.message = SaveLoadMessage::no_save_data;
        state.load_flag_published = false;
        if (callbacks.draw_save_load_message) {
            callbacks.draw_save_load_message(state.message);
        }
        call_if_present(callbacks.wait_for_controller_event);
        return SaveLoadMenuEvent::no_save_data;
    }
    return SaveLoadMenuEvent::callback_unresolved;
}

void enter_joshua_archetype_menu(
    JoshuaArchetypeMenuState& state,
    const JoshuaArchetypeMenuCallbacks& callbacks) {
    state.selected = JoshuaArchetypeChoice::samurai;
    call_if_present(callbacks.draw_joshua_archetype_grid);
    call_if_present(callbacks.draw_joshua_archetype_title);
    if (callbacks.draw_joshua_archetype_cursor) {
        callbacks.draw_joshua_archetype_cursor(state.selected);
    }
    if (callbacks.preview_joshua_archetype_resources) {
        callbacks.preview_joshua_archetype_resources(state.selected);
    }
}

JoshuaArchetypeMenuEvent advance_joshua_archetype_menu(
    JoshuaArchetypeMenuState& state,
    const std::uint8_t sampled_controller_byte,
    const JoshuaArchetypeMenuCallbacks& callbacks) {
    const auto step = vertical_step(sampled_controller_byte);
    if (step != 0) {
        state.selected = static_cast<JoshuaArchetypeChoice>(
            move_wrapped_menu_cursor(
                static_cast<std::uint8_t>(state.selected), step, 2U));
        if (callbacks.draw_joshua_archetype_cursor) {
            callbacks.draw_joshua_archetype_cursor(state.selected);
        }
        if (callbacks.preview_joshua_archetype_resources) {
            callbacks.preview_joshua_archetype_resources(state.selected);
        }
        // 0x59282/0x59294 branch back through 0x592A2 after movement. The
        // source does not inspect A/B/C from that same sampled frame.
        return JoshuaArchetypeMenuEvent::selection_moved;
    } else if (!action_pressed(sampled_controller_byte)) {
        return JoshuaArchetypeMenuEvent::waiting;
    }

    if (!callbacks.initialize_joshua_archetype) {
        return JoshuaArchetypeMenuEvent::callback_unresolved;
    }
    callbacks.initialize_joshua_archetype(state.selected);
    return JoshuaArchetypeMenuEvent::joshua_archetype_initialized;
}

} // namespace shadowrun::ghidra_conversion
