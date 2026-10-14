#include "shadowrun_ghidra/menus/hotel_options_menu_conversion.hpp"

#include <deque>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    HotelOptionsMenuState input_state{};
    const auto simultaneous_options_input = apply_hotel_options_input(
        input_state, HotelOptionsInputFrame{true, true, true, true, true});
    if (!expect(input_state.selection == HotelOption::check_out &&
                simultaneous_options_input.selection_moved &&
                simultaneous_options_input.confirmed &&
                simultaneous_options_input.action)) {
        return 4;
    }
    HotelCheckoutPromptState input_checkout_state{};
    const auto simultaneous_checkout_input = apply_hotel_checkout_input(
        input_checkout_state, HotelOptionsInputFrame{true, true, true, true,
                                                     true});
    if (!expect(input_checkout_state.selection == HotelCheckoutSelection::yes &&
                simultaneous_checkout_input ==
                    HotelCheckoutResult::checked_out)) {
        return 5;
    }
    HotelCheckoutPromptState default_no_state{};
    const auto default_no_input = apply_hotel_checkout_input(
        default_no_state, HotelOptionsInputFrame{false, false, false, true,
                                                 false});
    if (!expect(default_no_state.selection == HotelCheckoutSelection::no &&
                default_no_input == HotelCheckoutResult::stay_at_hotel)) {
        return 6;
    }

    std::deque<HotelOptionsInputFrame> hotel_frames{
        {false, true, true, false, false},
    };
    std::vector<std::string> hotel_order;
    HotelOptionsMenuState hotel_state{};
    HotelOptionsMenuCallbacks hotel_callbacks{};
    hotel_callbacks.reset_mode_display_buffers =
        [&] { hotel_order.emplace_back("reset"); };
    hotel_callbacks.prepare_hotel_options_display =
        [&] { hotel_order.emplace_back("prepare"); };
    hotel_callbacks.service_hotel_entity_status =
        [&] { hotel_order.emplace_back("entity-status"); };
    hotel_callbacks.write_hotel_options_grid =
        [&] { hotel_order.emplace_back("grid"); };
    hotel_callbacks.render_hotel_options_title =
        [&] { hotel_order.emplace_back("title"); };
    hotel_callbacks.render_cursor = [&](const HotelOption option) {
        hotel_order.emplace_back(option == HotelOption::karma_screen
                                     ? "cursor-karma"
                                     : "cursor-checkout");
    };
    hotel_callbacks.wait_for_vblank = [&] { hotel_order.emplace_back("vblank"); };
    hotel_callbacks.latch_controller_events =
        [&] { hotel_order.emplace_back("latch"); };
    hotel_callbacks.sample_controller_input = [&] {
        hotel_order.emplace_back("sample");
        const auto frame = hotel_frames.front();
        hotel_frames.pop_front();
        return frame;
    };
    hotel_callbacks.advance_random_grid_effect =
        [&] { hotel_order.emplace_back("random"); };
    hotel_callbacks.run_hotel_checkout_prompt = [&] {
        hotel_order.emplace_back("checkout");
        return HotelMenuHandlerOutcome::return_to_field;
    };

    const auto hotel_result =
        run_hotel_options_menu(hotel_state, hotel_callbacks);
    if (!expect(hotel_result.exit == HotelOptionsMenuExit::returned_to_field &&
                hotel_result.final_selection == HotelOption::check_out &&
                hotel_state.menu_result == -1 &&
                hotel_order == std::vector<std::string>{
                                    "reset", "prepare", "entity-status",
                                    "prepare", "grid", "title",
                                    "cursor-karma", "vblank", "latch",
                                    "sample", "random", "cursor-checkout",
                                    "checkout"})) {
        return 1;
    }

    HotelCheckoutPromptState checkout_state{};
    std::deque<HotelOptionsInputFrame> checkout_frames{
        {true, false, false, false, false},
        {false, false, false, true, false},
    };
    std::vector<std::string> checkout_order;
    HotelCheckoutPromptCallbacks checkout_callbacks{};
    checkout_callbacks.write_checkout_prompt_grid =
        [&] { checkout_order.emplace_back("grid"); };
    checkout_callbacks.render_checkout_question =
        [&] { checkout_order.emplace_back("question"); };
    checkout_callbacks.render_checkout_choices =
        [&] { checkout_order.emplace_back("choices"); };
    checkout_callbacks.render_cursor =
        [&](const HotelCheckoutSelection selection) {
            checkout_order.emplace_back(selection == HotelCheckoutSelection::yes
                                            ? "cursor-yes"
                                            : "cursor-no");
        };
    checkout_callbacks.wait_for_vblank =
        [&] { checkout_order.emplace_back("vblank"); };
    checkout_callbacks.latch_controller_events =
        [&] { checkout_order.emplace_back("latch"); };
    checkout_callbacks.sample_controller_input = [&] {
        checkout_order.emplace_back("sample");
        const auto frame = checkout_frames.front();
        checkout_frames.pop_front();
        return frame;
    };
    checkout_callbacks.advance_random_grid_effect =
        [&] { checkout_order.emplace_back("random"); };

    if (!expect(run_hotel_checkout_prompt(checkout_state, checkout_callbacks) ==
                    HotelCheckoutResult::checked_out &&
                checkout_state.selection == HotelCheckoutSelection::yes &&
                checkout_order == std::vector<std::string>{
                                        "grid", "question", "choices",
                                        "cursor-no", "vblank", "latch",
                                        "sample", "random", "cursor-yes",
                                        "vblank", "latch", "sample",
                                        "random"})) {
        return 2;
    }

    HotelCheckoutPromptState stay_state{};
    std::deque<HotelOptionsInputFrame> stay_frames{
        {false, true, false, false, false},
        {false, true, false, false, true},
    };
    HotelCheckoutPromptCallbacks stay_callbacks{};
    stay_callbacks.sample_controller_input = [&] {
        const auto frame = stay_frames.front();
        stay_frames.pop_front();
        return frame;
    };
    if (!expect(run_hotel_checkout_prompt(stay_state, stay_callbacks) ==
                    HotelCheckoutResult::stay_at_hotel &&
                stay_state.selection == HotelCheckoutSelection::no)) {
        return 3;
    }

    return 0;
}
