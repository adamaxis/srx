#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

// The startup routines test the sampled low controller byte with BTST #8,
// #9, #C, #D, and #E. These are the Genesis Up/Down/B/C/A masks after the
// byte-sized bit-number wrapping performed by the 68000 instruction.
constexpr std::uint8_t kStartupMenuUp = 0x01U;
constexpr std::uint8_t kStartupMenuDown = 0x02U;
constexpr std::uint8_t kStartupMenuB = 0x10U;
constexpr std::uint8_t kStartupMenuC = 0x20U;
constexpr std::uint8_t kStartupMenuA = 0x40U;
constexpr std::uint8_t kStartupMenuActionMask =
    kStartupMenuB | kStartupMenuC | kStartupMenuA;

enum class StartOptionsChoice : std::uint8_t {
    new_game = 0,
    load_game = 1,
};

const char* start_options_choice_name(StartOptionsChoice choice);

struct StartOptionsMenuState {
    StartOptionsChoice selected = StartOptionsChoice::new_game;
};

struct StartOptionsMenuCallbacks {
    // ROM 0x59098..0x590B0: prepare the resource metadata, four startup-menu
    // tile batches, and the title font before the New Game/Load Game panel.
    std::function<void()> copy_start_options_resource_metadata;
    std::function<void()> refresh_start_options_resource_metadata;
    std::function<void()> transfer_start_options_menu_tiles;
    std::function<void()> load_startup_menu_font;

    // ROM 0x590B6/0x590CC/0x590E4: draw this specific panel and its cursor.
    std::function<void()> draw_new_game_load_game_grid;
    std::function<void()> draw_start_options_title;
    std::function<void(StartOptionsChoice)> draw_start_options_cursor;
};

enum class StartOptionsMenuEvent {
    waiting,
    selection_moved,
    enter_joshua_archetype_menu,
    enter_save_load_menu,
};

// ROM 0x59092: enter the New Game/Load Game menu in source call order.
void enter_start_options_menu(
    StartOptionsMenuState& state,
    const StartOptionsMenuCallbacks& callbacks);

// ROM 0x590EA..0x59170: apply one sampled controller byte to the New Game /
// Load Game selector. A, B, and C all dispatch the currently selected row in
// the source; Up/Down movement is applied before that dispatch when both are
// present in one sampled frame.
StartOptionsMenuEvent advance_start_options_menu(
    StartOptionsMenuState& state,
    std::uint8_t sampled_controller_byte,
    const StartOptionsMenuCallbacks& callbacks);

enum class SaveSlotChoice : std::uint8_t {
    slot_one = 0,
    slot_two = 1,
};

const char* save_slot_choice_name(SaveSlotChoice choice);

enum class SaveLoadMessage {
    none,
    no_save_data,
    save_loaded,
};

struct SaveLoadMenuState {
    SaveSlotChoice selected = SaveSlotChoice::slot_one;
    SaveLoadMessage message = SaveLoadMessage::none;
    bool load_flag_published = false;
};

enum class SaveLoadAttempt {
    loaded,
    empty_or_invalid,
    unresolved,
};

struct SaveLoadMenuCallbacks {
    // ROM 0x59172/0x59202: the source redraws the Save/Load layout only when
    // a slot action is accepted, then asks 0x59360 to validate and load the
    // selected save image.
    std::function<void()> draw_save_load_grid;
    std::function<void()> draw_save_load_title;
    std::function<void(SaveSlotChoice)> draw_save_load_cursor;
    std::function<SaveLoadAttempt(SaveSlotChoice)> try_load_save_slot;
    std::function<void(SaveLoadMessage)> draw_save_load_message;

    // ROM 0x59242: the no-save and success branches both wait for a new
    // controller event before returning to the caller.
    std::function<void()> wait_for_controller_event;
};

enum class SaveLoadMenuEvent {
    waiting,
    selection_moved,
    save_loaded,
    no_save_data,
    callback_unresolved,
};

// ROM 0x59172/0x59182..0x591D0: enter the two-slot Save/Load screen. The
// layout is intentionally not drawn here; the source first draws it after a
// slot action, at 0x59202.
void enter_save_load_menu(
    SaveLoadMenuState& state,
    const SaveLoadMenuCallbacks& callbacks);

// ROM 0x59194..0x59242: move between Save Slot 1 and Save Slot 2, then run
// the source save validation/load boundary for the selected slot.
SaveLoadMenuEvent advance_save_load_menu(
    SaveLoadMenuState& state,
    std::uint8_t sampled_controller_byte,
    const SaveLoadMenuCallbacks& callbacks);

enum class JoshuaArchetypeChoice : std::uint8_t {
    samurai = 0,
    decker = 1,
    gator_shaman = 2,
};

const char* joshua_archetype_choice_name(JoshuaArchetypeChoice choice);

struct JoshuaArchetypeMenuState {
    JoshuaArchetypeChoice selected = JoshuaArchetypeChoice::samurai;
};

struct JoshuaArchetypeMenuCallbacks {
    // ROM 0x59248..0x5925E: draw the archetype layout and its title.
    std::function<void()> draw_joshua_archetype_grid;
    std::function<void()> draw_joshua_archetype_title;
    std::function<void(JoshuaArchetypeChoice)> draw_joshua_archetype_cursor;

    // ROM 0x592B2..0x592D4: update the visible resource/portrait preview for
    // the selected Samurai, Decker, or Gator Shaman choice.
    std::function<void(JoshuaArchetypeChoice)>
        preview_joshua_archetype_resources;

    // ROM 0x59308..0x59310 -> 0x42B0: copy the selected 0x100-byte Joshua
    // archetype template into the live protagonist record.
    std::function<void(JoshuaArchetypeChoice)> initialize_joshua_archetype;
};

enum class JoshuaArchetypeMenuEvent {
    waiting,
    selection_moved,
    joshua_archetype_initialized,
    callback_unresolved,
};

// ROM 0x59248: enter the three-choice Joshua archetype screen.
void enter_joshua_archetype_menu(
    JoshuaArchetypeMenuState& state,
    const JoshuaArchetypeMenuCallbacks& callbacks);

// ROM 0x59270..0x59316: move between Samurai, Decker, and Gator Shaman, then
// initialize Joshua from the selected extracted template on A/B/C.
JoshuaArchetypeMenuEvent advance_joshua_archetype_menu(
    JoshuaArchetypeMenuState& state,
    std::uint8_t sampled_controller_byte,
    const JoshuaArchetypeMenuCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
