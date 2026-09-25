#include "shadowrun_ghidra/pocket_secretary_menu_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void initialize_pocket_secretary_menu(
    PocketSecretaryMenuState& state, const bool extended_menu,
    const PocketSecretaryMenuCallbacks& callbacks) {
    call_once(callbacks.transfer_menu_assets);
    call_once(callbacks.render_menu_grid);
    call_once(callbacks.render_menu_text);
    call_once(callbacks.render_menu_numbers);
    state.selection = 0U;
    state.maximum_selection = extended_menu ? 5U : 4U;
    state.open = true;
}

PocketSecretaryMenuEvent apply_pocket_secretary_input(
    PocketSecretaryMenuState& state, const std::uint16_t input,
    const PocketSecretaryMenuCallbacks& callbacks) {
    if (!state.open) {
        return PocketSecretaryMenuEvent::exited;
    }

    constexpr std::uint16_t kMovePrevious = 0x0100U;
    constexpr std::uint16_t kMoveNext = 0x0200U;
    constexpr std::uint16_t kDispatchAction = 0x4000U;
    constexpr std::uint16_t kModeAction = 0x2000U;
    constexpr std::uint16_t kExit = 0x8000U;

    if ((input & kMovePrevious) != 0U || (input & kMoveNext) != 0U) {
        const auto delta = (input & kMovePrevious) != 0U ? -1 : 1;
        const auto next = static_cast<int>(state.selection) + delta;
        if (next < 0) {
            state.selection = state.maximum_selection;
        } else if (next > static_cast<int>(state.maximum_selection)) {
            state.selection = 0U;
        } else {
            state.selection = static_cast<std::uint8_t>(next);
        }
        return PocketSecretaryMenuEvent::selection_moved;
    }

    if ((input & kDispatchAction) != 0U) {
        const auto keep_open = callbacks.dispatch_menu_handler
                                   ? callbacks.dispatch_menu_handler(
                                         state.selection)
                                   : true;
        if (!keep_open) {
            state.open = false;
            return PocketSecretaryMenuEvent::exited;
        }
        call_once(callbacks.render_menu_grid);
        call_once(callbacks.render_menu_text);
        call_once(callbacks.render_menu_numbers);
        return PocketSecretaryMenuEvent::action_dispatched;
    }

    if ((input & kModeAction) != 0U) {
        call_once(callbacks.handle_mode_action_event);
        return PocketSecretaryMenuEvent::mode_action;
    }

    if ((input & kExit) != 0U) {
        state.open = false;
        return PocketSecretaryMenuEvent::exited;
    }

    return PocketSecretaryMenuEvent::waiting;
}

} // namespace shadowrun::ghidra_conversion
