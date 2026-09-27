#pragma once

#include "shadowrun_ghidra/core/selection_conversion.hpp"

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class HotelOption : std::uint8_t {
    karma_screen = 0,
    check_out = 1,
};

struct HotelOptionsInputFrame {
    bool move_up = false;
    bool move_down = false;
    bool confirm = false; // source controller bit 0x0E
    bool alternate_confirm = false; // source controller bit 0x0C
    bool action = false;  // source controller bit 0x0D
};

enum class HotelMenuHandlerOutcome {
    redraw_options,
    return_to_field,
};

struct HotelOptionsMenuState {
    HotelOption selection = HotelOption::karma_screen;
    std::int8_t menu_result = 0; // source FFF0DC
    std::uint8_t entity_status_marker = 0; // source FFF10D, cleared at entry
};

struct HotelOptionsMenuCallbacks {
    // ROM 0xCF3E, 0xCFFE, and 0xEF08. The entity-status service is kept as a
    // named boundary because it has its own source routine and presents the
    // active party before the Hotel Options screen is rebuilt.
    std::function<void()> reset_mode_display_buffers;
    std::function<void()> prepare_hotel_options_display;
    std::function<void()> service_hotel_entity_status;

    // ROM 0xCF52 and 0xCF56: draw the Hotel Options grid and its title text.
    std::function<void()> write_hotel_options_grid;
    std::function<void()> render_hotel_options_title;
    std::function<void(HotelOption)> render_cursor;

    // The source frame order is vblank, controller latch, controller sample,
    // then the random grid effect at 0xE660.
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<HotelOptionsInputFrame()> sample_controller_input;
    std::function<void()> advance_random_grid_effect;

    // The pointer table at 0xC14C8 dispatches Karma Screen to 0xD024. That
    // address enters the existing Attributes/Skills/Cyberware routine after
    // its shared setup at 0xD01C.
    std::function<HotelMenuHandlerOutcome()> run_hotel_karma_screen;

    // The pointer table dispatches Check Out to the confirmation routine at
    // 0xDA6A. Its typed result replaces the source's deliberate stack unwind.
    std::function<HotelMenuHandlerOutcome()> run_hotel_checkout_prompt;

    // ROM 0xCFF4: B/action invokes the field-mode action service. Hotel
    // Options has no separate Start/cancel branch in this source loop.
    std::function<void()> handle_mode_action_event;
};

enum class HotelOptionsMenuExit {
    returned_to_field,
};

struct HotelOptionsMenuResult {
    HotelOptionsMenuExit exit = HotelOptionsMenuExit::returned_to_field;
    HotelOption final_selection = HotelOption::karma_screen;
};

// ROM 0xCF38: run the specifically identified Hotel Options menu. The native
// loop uses typed options and handler outcomes rather than the source's
// handler-pointer table and FFF0DC/FFF10D RAM bytes.
HotelOptionsMenuResult run_hotel_options_menu(
    HotelOptionsMenuState& state,
    const HotelOptionsMenuCallbacks& callbacks);

enum class HotelCheckoutSelection : std::uint8_t {
    yes = 0,
    no = 1,
};

struct HotelCheckoutPromptState {
    HotelCheckoutSelection selection = HotelCheckoutSelection::no;
};

struct HotelCheckoutPromptCallbacks {
    // ROM 0xDA6A first writes the prompt grid, then renders the question and
    // Yes/No choice text before placing the cursor at No.
    std::function<void()> write_checkout_prompt_grid;
    std::function<void()> render_checkout_question;
    std::function<void()> render_checkout_choices;
    std::function<void(HotelCheckoutSelection)> render_cursor;

    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<HotelOptionsInputFrame()> sample_controller_input;
    std::function<void()> advance_random_grid_effect;
};

enum class HotelCheckoutResult {
    stay_at_hotel,
    checked_out,
};

// ROM 0xDA6A: ask "Do you wish to check out?". The source initializes the
// selection to No, wraps the two choices, and treats controller bits 0x0E,
// 0x0C, and 0x0D as confirmation. Selecting Yes unwinds the Hotel menu and
// is returned explicitly as checked_out here.
HotelCheckoutResult run_hotel_checkout_prompt(
    HotelCheckoutPromptState& state,
    const HotelCheckoutPromptCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
