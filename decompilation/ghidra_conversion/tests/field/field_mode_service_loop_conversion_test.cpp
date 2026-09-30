#include "shadowrun_ghidra/field/field_mode_service_loop_conversion.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint32_t, 8> source{1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U};
    std::array<std::uint32_t, 8> destination{};
    std::uint8_t dirty_flags = 0x04U;
    copy_field_mode_palette_bank(source, destination, dirty_flags);
    if (!expect(destination == source && dirty_flags == 0x05U)) {
        return 1;
    }

    FieldModeServiceState normal{};
    normal.action_selection.menu_result = 0;
    std::vector<std::string> order;
    // The source returns to the vblank loop immediately after a directional
    // branch. Confirm in the same sample must therefore be ignored here.
    std::vector<std::uint16_t> inputs{0x4400U, 0x0800U, 0x2000U, 0x8000U};
    std::size_t input_index = 0;
    int actions = 0;
    int restart_calls = 0;
    int selection_renders = 0;
    const FieldModeServiceCallbacks callbacks{
        [&](std::uint16_t control) {
            order.emplace_back(control == 0x9100U ? "display_begin"
                                                   : "display_field");
        },
        [&] { order.emplace_back("transfer_assets"); },
        [&] { order.emplace_back("upload_pair"); },
        [&] { order.emplace_back("palette"); },
        [&] { order.emplace_back("wrapped"); },
        [&] { order.emplace_back("metadata"); },
        [&] { order.emplace_back("resource_state"); },
        [&] { order.emplace_back("decode"); },
        [&] { order.emplace_back("font"); },
        [&] { order.emplace_back("text"); },
        [&] { order.emplace_back("numeric_table"); },
        [&] { order.emplace_back("reset_upload"); },
        [&] { return source; },
        [&] { order.emplace_back("panel"); },
        [&] { order.emplace_back("prepare_numeric"); },
        [&] { order.emplace_back("wait_controller"); },
        [&] { order.emplace_back("field_sequence"); },
        [&] { order.emplace_back("field_animation"); },
        [&] { order.emplace_back("field_setup"); },
        [&](FieldModeServiceState&) { ++selection_renders; },
        FieldActionSelectionNavigationCallbacks{
            [&] { order.emplace_back("prepare_selection"); },
            [&] { order.emplace_back("grid"); },
            [&] { order.emplace_back("selection_text"); },
            [&] { order.emplace_back("sprites"); },
            [&] { order.emplace_back("nav_reset"); },
        },
        [&] { order.emplace_back("vblank"); },
        [&] { order.emplace_back("latch"); },
        [&] {
            const auto value = inputs[input_index];
            ++input_index;
            return value;
        },
        [&] { order.emplace_back("audio"); },
        [&] {
            ++restart_calls;
            order.emplace_back("restart");
        },
        [&](FieldModeServiceState&) { ++actions; },
    };

    if (!expect(run_field_mode_service(normal, callbacks) ==
                    FieldModeServiceLoopAction::returned_to_field &&
                normal.returned_to_field && normal.action_selection.action_cursor == 0 &&
                normal.action_selection.action_panel == 0 && restart_calls == 1 &&
                selection_renders == 2 && normal.palette_bank_words == source &&
                normal.action_selection.display_dirty_flags == 1U && actions == 0)) {
        return 2;
    }

    FieldModeServiceState alternate{};
    alternate.action_selection.menu_result = 1;
    alternate.has_deck = true;
    inputs = {0x4000U, 0x4000U, 0x8000U};
    input_index = 0;
    int action_attempts = 0;
    int action_selection_renders = 0;
    const auto alternate_callbacks = [&]() {
        auto copy = callbacks;
        copy.render_field_action_selection = [&](FieldModeServiceState&) {
            ++action_selection_renders;
            order.emplace_back("action_selection");
        };
        copy.dispatch_selected_field_action = [&](FieldModeServiceState& state) {
            ++action_attempts;
            state.service_result = action_attempts == 1 ? -1 : 0;
        };
        return copy;
    }();
    if (!expect(run_field_mode_service(alternate, alternate_callbacks) ==
                    FieldModeServiceLoopAction::returned_to_field &&
                alternate.action_selection.action_panel == 1 && action_attempts == 2 &&
                action_selection_renders == 2 && alternate.returned_to_field)) {
        return 3;
    }

    FieldModeServiceState no_deck{};
    no_deck.action_selection.menu_result = 2;
    no_deck.has_deck = false;
    inputs = {0x0080U};
    input_index = 0;
    if (!expect(run_field_mode_service(no_deck, callbacks) ==
                    FieldModeServiceLoopAction::returned_to_field &&
                no_deck.returned_to_field)) {
        return 4;
    }

    return 0;
}
