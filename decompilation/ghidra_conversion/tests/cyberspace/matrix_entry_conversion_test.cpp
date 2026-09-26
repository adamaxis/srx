#include "shadowrun_ghidra/cyberspace/matrix_entry_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    MatrixEntryState state{};
    std::vector<std::uint32_t> copied_sources;
    std::vector<MatrixDisplayCommand> display_commands;
    std::vector<int> events;

    enter_matrix_screen(
        state,
        {[&](const MatrixDisplayCommand command) {
             display_commands.push_back(command);
         },
         [&](const std::uint32_t source) { copied_sources.push_back(source); },
         [&] { events.push_back(1); },
         [&] { events.push_back(2); },
         [&](const std::uint32_t source) { copied_sources.push_back(source); },
         [&] { events.push_back(3); },
         [&] { events.push_back(4); },
         [&] { events.push_back(5); },
         [&] { events.push_back(6); }});

    if (!expect(state.menu_slot_count == 10U &&
                state.display_dirty_flags == 0x01U &&
                state.overlay_marker == 0U && state.returned_to_field &&
                copied_sources == std::vector<std::uint32_t>{0xE2E3AU,
                                                              0xD29D6U} &&
                display_commands.size() == 5U &&
                display_commands[0] == MatrixDisplayCommand::prepare_matrix &&
                display_commands[1] ==
                    MatrixDisplayCommand::prepare_resource_upload &&
                display_commands[2] == MatrixDisplayCommand::matrix_loop &&
                display_commands[3] ==
                    MatrixDisplayCommand::prepare_field_return &&
                display_commands[4] == MatrixDisplayCommand::field_return &&
                events == std::vector<int>{1, 2, 3, 4, 5, 6})) {
        return 1;
    }

    MatrixEntryState no_callbacks{};
    enter_matrix_screen(no_callbacks, {});
    if (!expect(no_callbacks.menu_slot_count == 10U &&
                no_callbacks.display_dirty_flags == 0x01U &&
                no_callbacks.returned_to_field)) {
        return 2;
    }

    return 0;
}
