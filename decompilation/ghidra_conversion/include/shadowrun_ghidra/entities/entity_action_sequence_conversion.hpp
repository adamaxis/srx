#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// ROM 0x12E9E: read the current entity action before entering its menu/render
// service.
void service_entity_action_menu(const std::function<void()>& read_action,
                                const std::function<void()>& render_menu);

// ROM 0x12F50: update the action menu, render its text, then submit its grid.
void render_entity_action_menu(const std::function<void()>& update_menu,
                               const std::function<void()>& render_text,
                               const std::function<void()>& write_grid);

struct EntityActionMenuState {
    std::uint8_t available_count = 0;
};

// ROM 0x12FA4: render the text, service the action state, and write the grid
// only when more than one action remains.
void advance_entity_action_menu(
    EntityActionMenuState& state,
    const std::function<void()>& render_text,
    const std::function<void()>& service_action,
    const std::function<void()>& write_grid);

} // namespace shadowrun::ghidra_conversion
