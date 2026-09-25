#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct PocketSecretaryMenuState {
    std::uint8_t selection = 0;
    std::uint8_t maximum_selection = 4;
    bool open = false;
};

struct PocketSecretaryMenuCallbacks {
    // ROM 0xC2A6's initial resource/grid/text/number presentation order.
    std::function<void()> transfer_menu_assets;
    std::function<void()> render_menu_grid;
    std::function<void()> render_menu_text;
    std::function<void()> render_menu_numbers;

    // The handler table at ROM 0xC1400. Return false when the handler has
    // set the source cursor to its negative exit sentinel.
    std::function<bool(std::uint8_t)> dispatch_menu_handler;
    std::function<void()> handle_mode_action_event;
};

enum class PocketSecretaryMenuEvent {
    waiting,
    selection_moved,
    action_dispatched,
    mode_action,
    exited,
};

// ROM 0xC2A6: FFF101 selects the five-entry or six-entry Pocket Secretary
// menu. The source always resets FFF0DC to the first selection on entry.
void initialize_pocket_secretary_menu(
    PocketSecretaryMenuState& state, bool extended_menu,
    const PocketSecretaryMenuCallbacks& callbacks);

// ROM 0xC2D4..0xC374: apply one sampled controller event. Source bit 8 has
// priority over bit 9, then action (bit 14), mode action (bit 13), and exit
// (bit 15). Cursor movement wraps between zero and the source maximum index.
PocketSecretaryMenuEvent apply_pocket_secretary_input(
    PocketSecretaryMenuState& state, std::uint16_t input,
    const PocketSecretaryMenuCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
