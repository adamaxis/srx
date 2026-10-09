#include "shadowrun_ghidra/menus/startup_menu_conversion.hpp"

#include <string>
#include <vector>

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(startup_menu_vram_x(kStartOptionsPlaneVram) == 104 &&
                startup_menu_vram_y(kStartOptionsPlaneVram) == 56 &&
                startup_menu_vram_x(kStartOptionsCursorVram) == 104 &&
                startup_menu_cursor_y(kStartOptionsCursorVram, 0) == 80 &&
                startup_menu_cursor_y(kStartOptionsCursorVram, 1) == 96 &&
                kStartOptionsLayout[2].row == 3 &&
                kStartOptionsLayout[0].text_key ==
                    "startup.options.title" &&
                kStartOptionsLayout[1].text_key ==
                    "startup.options.separator" &&
                kStartOptionsLayout[2].choice_index == 0U &&
                kStartOptionsLayout[3].choice_index == 1U)) {
        return 13;
    }
    if (!expect(startup_menu_vram_x(kJoshuaArchetypePlaneVram) == 152 &&
                startup_menu_vram_y(kJoshuaArchetypePlaneVram) == 56 &&
                startup_menu_cursor_y(kJoshuaArchetypeCursorVram, 0) == 88 &&
                startup_menu_cursor_y(kJoshuaArchetypeCursorVram, 1) == 104 &&
                startup_menu_cursor_y(kJoshuaArchetypeCursorVram, 2) == 120 &&
                kJoshuaArchetypeLayout[3].choice_index == 0U &&
                kJoshuaArchetypeLayout[4].choice_index == 1U &&
                kJoshuaArchetypeLayout[5].choice_index == 2U &&
                kJoshuaArchetypeLayout[6].row == 9 &&
                kJoshuaArchetypeLayout[6].text_key ==
                    "startup.archetype.gator_shaman.suffix" &&
                startup_menu_line_y(kJoshuaArchetypePlaneVram, 8) == 120 &&
                startup_menu_line_y(kJoshuaArchetypePlaneVram, 9) == 128)) {
        return 14;
    }
    if (!expect(startup_menu_vram_x(kSaveLoadPlaneVram) == 152 &&
                startup_menu_vram_y(kSaveLoadPlaneVram) == 112 &&
                startup_menu_vram_x(kSaveLoadCursorVram) == 136 &&
                startup_menu_cursor_y(kSaveLoadCursorVram, 1) == 128 &&
                kSaveLoadLayout[0].choice_index == 0U &&
                kSaveLoadLayout[1].choice_index == 1U &&
                startup_menu_vram_x(kSaveLoadMessagePlaneVram) == 72 &&
                startup_menu_vram_y(kSaveLoadMessagePlaneVram) == 72 &&
                kNoSaveDataLayout[0].text_key ==
                    "startup.message.no_save.0" &&
                kSaveSuccessLayout[2].text_key ==
                    "startup.message.save_success.2")) {
        return 15;
    }

    std::vector<std::string> start_entry_order;
    StartOptionsMenuState start{};
    enter_start_options_menu(
        start,
        {[&] { start_entry_order.emplace_back("metadata_copy"); },
         [&] { start_entry_order.emplace_back("metadata_refresh"); },
         [&] { start_entry_order.emplace_back("new_load_tiles"); },
         [&] { start_entry_order.emplace_back("startup_font"); },
         [&] { start_entry_order.emplace_back("new_load_grid"); },
         [&] { start_entry_order.emplace_back("new_load_title"); },
         [&](const StartOptionsChoice choice) {
             start_entry_order.emplace_back(
                 choice == StartOptionsChoice::new_game ? "new_cursor"
                                                         : "load_cursor");
         }});
    if (!expect(start.selected == StartOptionsChoice::new_game &&
                start_entry_order ==
                    std::vector<std::string>{"metadata_copy",
                                             "metadata_refresh",
                                             "new_load_tiles", "startup_font",
                                             "new_load_grid", "new_load_title",
                                             "new_cursor"})) {
        return 2;
    }

    if (!expect(advance_start_options_menu(start, kStartupMenuDown, {}) ==
                    StartOptionsMenuEvent::selection_moved &&
                start.selected == StartOptionsChoice::load_game &&
                advance_start_options_menu(
                    start, kStartupMenuA,
                    {}) == StartOptionsMenuEvent::enter_save_load_menu &&
                advance_start_options_menu(
                    start, kStartupMenuUp | kStartupMenuA,
                    {}) ==
                    StartOptionsMenuEvent::enter_joshua_archetype_menu)) {
        return 3;
    }

    SaveLoadMenuState save_load{};
    std::vector<std::string> save_load_order;
    SaveSlotChoice attempted_slot = SaveSlotChoice::slot_one;
    SaveLoadMenuCallbacks save_load_callbacks;
    save_load_callbacks.draw_save_load_title =
        [&] { save_load_order.emplace_back("save_title"); };
    save_load_callbacks.draw_save_load_cursor =
        [&](const SaveSlotChoice choice) {
            save_load_order.emplace_back(choice == SaveSlotChoice::slot_one
                                              ? "slot_one_cursor"
                                              : "slot_two_cursor");
        };
    save_load_callbacks.draw_save_load_grid =
        [&] { save_load_order.emplace_back("save_grid"); };
    save_load_callbacks.try_load_save_slot =
        [&](const SaveSlotChoice choice) {
            attempted_slot = choice;
            save_load_order.emplace_back("try_slot");
            return SaveLoadAttempt::empty_or_invalid;
        };
    save_load_callbacks.draw_save_load_message =
        [&](const SaveLoadMessage message) {
            save_load_order.emplace_back(
                message == SaveLoadMessage::no_save_data ? "no_save"
                                                          : "other_message");
        };
    save_load_callbacks.wait_for_controller_event =
        [&] { save_load_order.emplace_back("wait_for_event"); };
    enter_save_load_menu(
        save_load, save_load_callbacks);
    if (!expect(save_load.selected == SaveSlotChoice::slot_one &&
                save_load_order ==
                    std::vector<std::string>{"save_title", "slot_one_cursor"})) {
        return 4;
    }
    if (!expect(advance_save_load_menu(save_load, kStartupMenuDown,
                                       save_load_callbacks) ==
                    SaveLoadMenuEvent::selection_moved &&
                save_load.selected == SaveSlotChoice::slot_two)) {
        return 5;
    }
    bool save_load_attempted_during_move = false;
    SaveLoadMenuCallbacks save_load_precedence_callbacks;
    save_load_precedence_callbacks.draw_save_load_cursor =
        [](const SaveSlotChoice) {};
    save_load_precedence_callbacks.try_load_save_slot =
        [&](const SaveSlotChoice) {
            save_load_attempted_during_move = true;
            return SaveLoadAttempt::loaded;
        };
    SaveLoadMenuState save_load_precedence_state{};
    if (!expect(advance_save_load_menu(
                    save_load_precedence_state,
                    kStartupMenuDown | kStartupMenuA,
                    save_load_precedence_callbacks) ==
                    SaveLoadMenuEvent::selection_moved &&
                save_load_precedence_state.selected == SaveSlotChoice::slot_two &&
                !save_load_attempted_during_move)) {
        return 6;
    }
    if (!expect(advance_save_load_menu(save_load, kStartupMenuA,
                                       save_load_callbacks) ==
                    SaveLoadMenuEvent::no_save_data &&
                attempted_slot == SaveSlotChoice::slot_two &&
                save_load.message == SaveLoadMessage::no_save_data &&
                !save_load.load_flag_published &&
                save_load_order.size() == 7U &&
                save_load_order[2] == "slot_two_cursor" &&
                save_load_order[3] == "save_grid" &&
                save_load_order[4] == "try_slot" &&
                save_load_order[5] == "no_save" &&
                save_load_order[6] == "wait_for_event")) {
        return 7;
    }
    if (!expect(advance_save_load_menu(save_load, 0U, save_load_callbacks) ==
                    SaveLoadMenuEvent::waiting &&
                save_load.message == SaveLoadMessage::no_save_data)) {
        return 16;
    }
    if (!expect(advance_save_load_menu(save_load, kStartupMenuA,
                                       save_load_callbacks) ==
                    SaveLoadMenuEvent::message_acknowledged)) {
        return 17;
    }

    JoshuaArchetypeMenuState archetype{};
    std::vector<JoshuaArchetypeChoice> previews;
    JoshuaArchetypeChoice initialized = JoshuaArchetypeChoice::samurai;
    enter_joshua_archetype_menu(
        archetype,
        {[&] {},
         [&] {},
         [&](const JoshuaArchetypeChoice) {},
         [&](const JoshuaArchetypeChoice choice) { previews.push_back(choice); },
         [&](const JoshuaArchetypeChoice choice) { initialized = choice; }});
    if (!expect(previews ==
                    std::vector<JoshuaArchetypeChoice>{
                        JoshuaArchetypeChoice::samurai})) {
        return 8;
    }
    if (!expect(advance_joshua_archetype_menu(
                    archetype, kStartupMenuDown,
                    {{}, {}, {},
                     [&](const JoshuaArchetypeChoice choice) {
                         previews.push_back(choice);
                     },
                     [&](const JoshuaArchetypeChoice choice) {
                         initialized = choice;
                     }}) == JoshuaArchetypeMenuEvent::selection_moved &&
                archetype.selected == JoshuaArchetypeChoice::decker &&
                previews.back() == JoshuaArchetypeChoice::decker)) {
        return 9;
    }
    bool archetype_initialized_during_move = false;
    JoshuaArchetypeMenuCallbacks archetype_precedence_callbacks;
    archetype_precedence_callbacks.draw_joshua_archetype_cursor =
        [](const JoshuaArchetypeChoice) {};
    archetype_precedence_callbacks.preview_joshua_archetype_resources =
        [](const JoshuaArchetypeChoice) {};
    archetype_precedence_callbacks.initialize_joshua_archetype =
        [&](const JoshuaArchetypeChoice) {
            archetype_initialized_during_move = true;
        };
    JoshuaArchetypeMenuState archetype_precedence_state{};
    if (!expect(advance_joshua_archetype_menu(
                    archetype_precedence_state,
                    kStartupMenuDown | kStartupMenuA,
                    archetype_precedence_callbacks) ==
                    JoshuaArchetypeMenuEvent::selection_moved &&
                !archetype_initialized_during_move)) {
        return 10;
    }
    if (!expect(advance_joshua_archetype_menu(
                    archetype, kStartupMenuA,
                    {{}, {}, {}, {},
                     [&](const JoshuaArchetypeChoice choice) {
                         initialized = choice;
                     }}) ==
                    JoshuaArchetypeMenuEvent::joshua_archetype_initialized &&
                initialized == JoshuaArchetypeChoice::decker)) {
        return 11;
    }

    return 0;
}
