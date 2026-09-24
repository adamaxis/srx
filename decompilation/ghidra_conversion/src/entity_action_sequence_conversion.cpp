#include "shadowrun_ghidra/entity_action_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void service_entity_action_menu(const std::function<void()>& read_action,
                                const std::function<void()>& render_menu) {
    if (read_action) {
        read_action();
    }
    if (render_menu) {
        render_menu();
    }
}

void render_entity_action_menu(const std::function<void()>& update_menu,
                               const std::function<void()>& render_text,
                               const std::function<void()>& write_grid) {
    if (update_menu) {
        update_menu();
    }
    if (render_text) {
        render_text();
    }
    if (write_grid) {
        write_grid();
    }
}

void advance_entity_action_menu(
    EntityActionMenuState& state,
    const std::function<void()>& render_text,
    const std::function<void()>& service_action,
    const std::function<void()>& write_grid) {
    if (render_text) {
        render_text();
    }
    if (service_action) {
        service_action();
    }
    if (state.available_count > 1U && write_grid) {
        write_grid();
    }
}

} // namespace shadowrun::ghidra_conversion
