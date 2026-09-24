#include "shadowrun_ghidra/post_intro_mode_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    PostIntroModeState state{};
    state.current_mode = 1U;
    std::vector<std::uint8_t> rendered;
    std::vector<std::uint8_t> events{0x09U, 0x06U, 0x20U, 0x80U};
    std::vector<int> order;
    int action_count = 0;
    PostIntroModeCallbacks callbacks{};
    callbacks.resolve_initial_mode = [](std::uint8_t) { return 3U; };
    callbacks.decode_and_upload_source_stream = [&] { order.push_back(1); };
    callbacks.render_hud_sprite_stream = [&] { order.push_back(2); };
    callbacks.wait_for_vblank = [&] { order.push_back(3); };
    callbacks.latch_controller_events = [&] { order.push_back(4); };
    callbacks.sample_controller_input = [&] { order.push_back(5); };
    callbacks.read_controller_events = [&] {
        const auto event = events.front();
        events.erase(events.begin());
        return event;
    };
    callbacks.service_world_state = [&] { order.push_back(6); };
    callbacks.render_mode_selection =
        [&](std::uint8_t mode) { rendered.push_back(mode); };
    callbacks.handle_action_event = [&] { ++action_count; };
    callbacks.transfer_menu_asset_batch = [&] { order.push_back(7); };
    callbacks.initialize_scene_mode_state = [&] { order.push_back(8); };
    callbacks.prepare_mode_entry = [&] { order.push_back(9); };

    if (run_post_intro_mode(state, callbacks) !=
            PostIntroModeResult::returned_after_transfer ||
        state.selected_mode != 3U ||
        rendered != std::vector<std::uint8_t>{3U, 2U, 3U} ||
        action_count != 1 || state.exit_marker_written || order.size() < 2U ||
        order.front() != 1 || order[1] != 2 || order.back() != 9) {
        return 1;
    }

    PostIntroModeState alternate{};
    alternate.alternate_branch = true;
    std::uint32_t frame_count = 0;
    std::vector<std::uint8_t> alternate_modes;
    PostIntroModeCallbacks alternate_callbacks{};
    alternate_callbacks.resolve_initial_mode = [](std::uint8_t) { return 6U; };
    alternate_callbacks.wait_for_vblank = [&] { ++frame_count; };
    alternate_callbacks.render_mode_selection =
        [&](std::uint8_t mode) { alternate_modes.push_back(mode); };
    if (run_post_intro_mode(alternate, alternate_callbacks) !=
            PostIntroModeResult::alternate_branch_complete ||
        frame_count != 0x00A3U || alternate.alternate_first_frame != 0x0041U ||
        alternate.alternate_second_frame != 0x0061U || alternate.title_state != 0U ||
        alternate_modes != std::vector<std::uint8_t>{4U, 2U}) {
        return 2;
    }
    return 0;
}
