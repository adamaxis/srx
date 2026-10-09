#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;
    using namespace shadowrun::ghidra_conversion::scripting;

    GameScriptRuntime scripts;
    if (!expect(scripts.load_package(
                    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
                    std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                        "dialogue" / "script-text.lua"),
                "gameplay Lua package did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto field_interaction_text_keys =
        scripts.string_map("field_interactions", "text_keys");
    bool field_interaction_text_map_valid =
        field_interaction_text_keys.has_value() &&
        field_interaction_text_keys->size() == 22U;
    if (field_interaction_text_map_valid) {
        for (const auto& entry : *field_interaction_text_keys) {
            field_interaction_text_map_valid =
                field_interaction_text_map_valid &&
                scripts.dialogue_text(entry.second) != nullptr;
        }
    }
    if (!expect(
            field_interaction_text_map_valid &&
                field_interaction_text_keys->at("door_title") ==
                    "field.maglock.door.title" &&
                field_interaction_text_keys->at("security_floor_fifth") ==
                    "field.security-terminal.floor.4",
            "Lua field-interaction outcomes or ROM text keys did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto stoker_speaker = scripts.speaker_profile("stoker");
    if (!expect(stoker_speaker.has_value() &&
                    stoker_speaker->id == "stoker" &&
                    stoker_speaker->name == "Stoker" &&
                    stoker_speaker->text_profile == "stoker.motel" &&
                    stoker_speaker->portrait_alias ==
                        "dialogue.portraits.stoker.motel",
                "Lua dialogue speaker properties did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto arrival_keys = scripts.dialogue_event_text_keys(
        "stoker", "stoker.motel", "arrival");
    const auto farewell_keys = scripts.dialogue_event_text_keys(
        "stoker", "stoker.motel", "leave");
    if (!expect(arrival_keys.has_value() &&
                    arrival_keys->title_key ==
                        "stoker.motel.arrival.title" &&
                    arrival_keys->continuation_key ==
                        "stoker.motel.arrival.more" &&
                    farewell_keys.has_value() &&
                    farewell_keys->title_key == "stoker.motel.leave.title" &&
                    farewell_keys->continuation_key.empty(),
                "Lua dialogue events did not resolve explicit text keys")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    ScriptMenuDefinition stoker_manager_menu{};
    stoker_manager_menu.id = "manager";
    stoker_manager_menu.text_profile = "stoker.motel";
    stoker_manager_menu.speaker_id = "stoker";
    stoker_manager_menu.choices = {
        {"crash", {}}, {"leave", {}}, {"package", {}}};
    const auto stoker_manager_content =
        scripts.resolve_dialogue_menu(stoker_manager_menu);
    ScriptMenuDefinition stoker_debt_menu{};
    stoker_debt_menu.id = "debt";
    stoker_debt_menu.text_profile = "stoker.motel";
    stoker_debt_menu.speaker_id = "stoker";
    stoker_debt_menu.choices = {
        {"debt-payment", {}}, {"jump-house", {}}};
    const auto stoker_debt_content =
        scripts.resolve_dialogue_menu(stoker_debt_menu);
    if (!expect(stoker_manager_content.has_value() &&
                    stoker_manager_content->title_key ==
                        "stoker.motel.manager.title" &&
                    stoker_manager_content->choices.size() == 3U &&
                    stoker_manager_content->choices[0].label_key ==
                        "stoker.motel.option.crash" &&
                    stoker_manager_content->choices[1].label_key ==
                        "stoker.motel.option.leave" &&
                    stoker_manager_content->choices[2].label_key ==
                        "stoker.motel.option.package" &&
                    stoker_debt_content.has_value() &&
                    stoker_debt_content->title_key ==
                        "stoker.motel.debt.title" &&
                    stoker_debt_content->choices[0].label_key ==
                        "stoker.motel.option.debt-payment" &&
                    stoker_debt_content->choices[1].label_key ==
                        "stoker.motel.option.jump-house",
                "Lua dialogue menus did not resolve explicit choice text")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }
    stoker_manager_menu.speaker_id.clear();
    const auto shared_manager_content =
        scripts.resolve_dialogue_menu(stoker_manager_menu);
    if (!expect(shared_manager_content.has_value() &&
                    shared_manager_content->title_key ==
                        "stoker.motel.manager.title" &&
                    shared_manager_content->choices.size() == 3U,
                "motel menu text was incorrectly coupled to a speaker")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto mode_prompt_content = [&](const std::uint8_t mode,
                                         const std::uint8_t event_code,
                                         const std::int8_t alternate_result) {
        scripts.set_field_mode_context(
            ScriptFieldModeContext{mode, event_code, alternate_result});
        return scripts.string_map("field_mode", "mode_text_content");
    };
    const auto ares_intro = mode_prompt_content(2U, 11U, 0);
    const auto ares_handoff = mode_prompt_content(2U, 11U, -1);
    const auto ares_payment = mode_prompt_content(2U, 11U, 1);
    const auto unmatched_mode_prompt = mode_prompt_content(2U, 2U, 0);
    const auto valid_mode_prompt = [&](const auto& content,
                                       const std::string_view text_key) {
        return content.has_value() && content->size() == 3U &&
               content->at("status") == "matched" &&
               content->at("text_key") == text_key &&
               content->at("portrait_alias") ==
                   "field.mode.entry.ares.sprite" &&
               scripts.dialogue_text(text_key) != nullptr;
    };
    if (!expect(valid_mode_prompt(ares_intro,
                                  "field.mode.ares.store_intro") &&
                    valid_mode_prompt(ares_handoff,
                                      "field.mode.ares.package_handoff") &&
                    valid_mode_prompt(ares_payment,
                                      "field.mode.ares.package_payment") &&
                    unmatched_mode_prompt.has_value() &&
                    unmatched_mode_prompt->at("status") == "unmatched" &&
                    unmatched_mode_prompt->at("text_key") == "none" &&
                    unmatched_mode_prompt->at("portrait_alias") == "none",
                "Lua field-mode context did not select the ROM Ares text branches")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto* arrival_text =
        scripts.dialogue_text("stoker.motel.arrival.title");
    const auto* fifth_floor_text =
        scripts.dialogue_text("field.security-terminal.floor.4");
    if (!expect(arrival_text != nullptr &&
                    arrival_text->find("Stoker's Coffin Motel") !=
                        std::string::npos &&
                    fifth_floor_text != nullptr &&
                    fifth_floor_text->find("fifth floor.") !=
                        std::string::npos,
                "ROM-derived Lua dialogue content did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto startup_options =
        scripts.menu("startup", "options", "startup", "options");
    const auto startup_save_slots =
        scripts.menu("startup", "save_slots", "startup", "save_slots");
    const auto startup_archetypes =
        scripts.menu("startup", "archetypes", "startup", "archetypes");
    const auto startup_text_keys =
        scripts.string_map("startup", "text_keys");
    const auto opening_text_keys =
        scripts.string_map("startup", "opening_text_keys");
    const auto startup_text_key_matches =
        [&startup_text_keys](const std::string_view id,
                             const std::string_view key) {
            if (!startup_text_keys.has_value()) {
                return false;
            }
            const auto found = startup_text_keys->find(std::string(id));
            return found != startup_text_keys->end() &&
                   found->second == key;
        };
    const auto* startup_title =
        scripts.dialogue_text("startup.options.title");
    const auto* startup_separator =
        scripts.dialogue_text("startup.options.separator");
    const auto* startup_gator =
        scripts.dialogue_text("startup.option.gator_shaman");
    const auto* startup_gator_suffix =
        scripts.dialogue_text("startup.archetype.gator_shaman.suffix");
    const auto* startup_no_save =
        scripts.dialogue_text("startup.message.no_save.3");
    const auto* startup_save_success =
        scripts.dialogue_text("startup.message.save_success.2");
    if (!expect(startup_options.has_value() &&
                    startup_options->title_key ==
                        "startup.options.title" &&
                    startup_options->choices.size() == 2U &&
                    startup_options->choices[0].id == "new_game" &&
                    startup_options->choices[0].label_key ==
                        "startup.option.new_game" &&
                    startup_options->choices[1].id == "load_game" &&
                    startup_options->choices[1].label_key ==
                        "startup.option.load_game" &&
                    startup_save_slots.has_value() &&
                    startup_save_slots->title_key.empty() &&
                    startup_save_slots->choices.size() == 2U &&
                    startup_save_slots->choices[0].id == "slot_one" &&
                    startup_save_slots->choices[0].label_key ==
                        "startup.option.slot_one" &&
                    startup_save_slots->choices[1].id == "slot_two" &&
                    startup_save_slots->choices[1].label_key ==
                        "startup.option.slot_two" &&
                    startup_archetypes.has_value() &&
                    startup_archetypes->choices.size() == 3U &&
                    startup_archetypes->choices[0].id == "samurai" &&
                    startup_archetypes->choices[0].label_key ==
                        "startup.option.samurai" &&
                    startup_archetypes->choices[1].id == "decker" &&
                    startup_archetypes->choices[1].label_key ==
                        "startup.option.decker" &&
                    startup_archetypes->choices[2].id == "gator_shaman" &&
                    startup_archetypes->choices[2].label_key ==
                        "startup.option.gator_shaman" &&
                    startup_text_keys.has_value() &&
                    startup_text_keys->size() == 13U &&
                    startup_text_key_matches(
                        "options_title", "startup.options.title") &&
                    startup_text_key_matches(
                        "options_separator", "startup.options.separator") &&
                    startup_text_key_matches(
                        "archetype_heading_choose",
                        "startup.archetype.heading.choose") &&
                    startup_text_key_matches(
                        "archetype_heading_archetype",
                        "startup.archetype.heading.archetype") &&
                    startup_text_key_matches(
                        "archetype_separator",
                        "startup.archetype.separator") &&
                    startup_text_key_matches(
                        "archetype_gator_suffix",
                        "startup.archetype.gator_shaman.suffix") &&
                    startup_text_key_matches(
                        "no_save_0", "startup.message.no_save.0") &&
                    startup_text_key_matches(
                        "no_save_1", "startup.message.no_save.1") &&
                    startup_text_key_matches(
                        "no_save_2", "startup.message.no_save.2") &&
                    startup_text_key_matches(
                        "no_save_3", "startup.message.no_save.3") &&
                    startup_text_key_matches(
                        "save_success_0", "startup.message.save_success.0") &&
                    startup_text_key_matches(
                        "save_success_1", "startup.message.save_success.1") &&
                    startup_text_key_matches(
                        "save_success_2", "startup.message.save_success.2") &&
                    startup_title != nullptr &&
                    *startup_title == " Start Options" &&
                    startup_separator != nullptr &&
                    *startup_separator ==
                        std::string("[") + std::string(13U, '\\') &&
                    startup_gator != nullptr &&
                    *startup_gator == "  Gator" &&
                    startup_gator_suffix != nullptr &&
                    *startup_gator_suffix == "    Shaman" &&
                    startup_no_save != nullptr &&
                    *startup_no_save == "     saved here!" &&
                    startup_save_success != nullptr &&
                    *startup_save_success ==
                        "to run in the shadows.",
                "startup Lua menu choices or original text did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    struct OpeningStoryTextExpectation {
        std::string_view id;
        std::string_view text;
    };
    constexpr std::array<OpeningStoryTextExpectation, 22>
        kOpeningStoryTextExpectations{{
            {"time", "The Time:  January 31, 2058"},
            {"place", "The Place: Seattle, UCAS"},
            {"team_cover", "Under the cover of darkness, a team"},
            {"team_runners", "of shadowrunners moves through"},
            {"team_wilderness", "the Salish-Shidhe wilderness."},
            {"without_warning", "Without warning, they are"},
            {"ambushed", "ambushed by unknown forces."},
            {"massacre", "The massacre is over quickly, but"},
            {"captured", "captured on tri-vid by one of the"},
            {"slain", "slain member's cybereyes."},
            {"tri_vid_recovered", "The tri-vid was recovered and made"},
            {"national_news", "national news.  Michael, your"},
            {"brother_last_die", "brother, was the last to die."},
            {"spending", "Spending your last Nuyen to go to"},
            {"vow", "Seattle, you vow to avenge your"},
            {"brother_death", "brother's death."},
            {"landing", "Landing at Sea-Tac Airport,"},
            {"flag_cab", "you flag a cab to bring you to"},
            {"last_credstick", "Michael's last Credstick"},
            {"transaction", "transaction."},
            {"motel", "Stoker's Coffin Motel, located"},
            {"redmond", "in the Redmond Barrens."},
        }};
    bool opening_story_content_valid =
        opening_text_keys.has_value() &&
        opening_text_keys->size() == kOpeningStoryTextExpectations.size();
    if (opening_story_content_valid) {
        for (const auto& expected : kOpeningStoryTextExpectations) {
            const auto key =
                opening_text_keys->find(std::string(expected.id));
            const auto* text = key == opening_text_keys->end()
                                   ? nullptr
                                   : scripts.dialogue_text(key->second);
            opening_story_content_valid = opening_story_content_valid &&
                                          key != opening_text_keys->end() &&
                                          key->second ==
                                              "opening.story." +
                                                  std::string(expected.id) &&
                                          text != nullptr &&
                                          *text == expected.text;
        }
    }
    if (!expect(opening_story_content_valid,
                "opening story captions or ROM text keys did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto* ares_store_intro =
        scripts.dialogue_text("field.mode.ares.store_intro");
    const auto* ares_package_handoff =
        scripts.dialogue_text("field.mode.ares.package_handoff");
    const auto* ares_package_payment =
        scripts.dialogue_text("field.mode.ares.package_payment");
    if (!expect(ares_store_intro != nullptr &&
                    *ares_store_intro ==
                        "<A large man with the Ares logo on his huge coveralls "
                        "grins at you over the counter.>\n\n"
                        "You'll love this place, sir.  None'a that wimpy stuff "
                        "here, no way!\n\nYou just point out what you want." &&
                    ares_package_handoff != nullptr &&
                    *ares_package_handoff ==
                        "<The receptionist hands a package to you.>\n\n"
                        "Please sign for it and it's all yours.\n\n"
                        "<You scrawl a fake name and move on.>" &&
                    ares_package_payment != nullptr &&
                    *ares_package_payment ==
                        "<The receptionist takes the package and looks it "
                        "over.\n\nHappy, she transfers nuyen to your "
                        "credstick account and wishes you a good day.>",
                "ROM-imported Ares mode text did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto hotel_options =
        scripts.menu("hotel", "options", "hotel", "options");
    const auto hotel_checkout =
        scripts.menu("hotel", "checkout", "hotel", "checkout");
    const auto hotel_text_keys = scripts.string_map("hotel", "text_keys");
    const auto* hotel_title = scripts.dialogue_text("hotel.options.title");
    const auto* hotel_separator =
        scripts.dialogue_text("hotel.options.separator");
    const auto* checkout_question =
        scripts.dialogue_text("hotel.checkout.title");
    const auto* medical_attention =
        scripts.dialogue_text("hotel.status.medical_attention_suffix");
    const auto* recovery_last =
        scripts.dialogue_text("hotel.status.recovery.4");
    if (!expect(hotel_options.has_value() &&
                    hotel_options->title_key == "hotel.options.title" &&
                    hotel_options->choices.size() == 2U &&
                    hotel_options->choices[0].id == "karma_screen" &&
                    hotel_options->choices[1].id == "check_out" &&
                    hotel_options->choices[0].label_key ==
                        "hotel.option.karma_screen" &&
                    hotel_options->choices[1].label_key ==
                        "hotel.option.check_out" &&
                    hotel_checkout.has_value() &&
                    hotel_checkout->title_key == "hotel.checkout.title" &&
                    hotel_checkout->choices.size() == 2U &&
                    hotel_checkout->choices[0].id == "checkout_yes" &&
                    hotel_checkout->choices[0].label_key ==
                        "hotel.option.checkout_yes" &&
                    hotel_checkout->choices[1].id == "checkout_no" &&
                    hotel_checkout->choices[1].label_key ==
                        "hotel.option.checkout_no" &&
                    hotel_text_keys.has_value() &&
                    hotel_text_keys->size() == 5U &&
                    hotel_text_keys->at("options_separator") ==
                        "hotel.options.separator" &&
                    hotel_text_keys->at("medical_attention_suffix") ==
                        "hotel.status.medical_attention_suffix" &&
                    hotel_text_keys->at("recovery_4") ==
                        "hotel.status.recovery.4" &&
                    hotel_title != nullptr &&
                    *hotel_title == " Hotel Options" &&
                    hotel_separator != nullptr &&
                    *hotel_separator ==
                        std::string("[") + std::string(14U, '\\') &&
                    checkout_question != nullptr &&
                    *checkout_question == "Do you wish to|  check out?" &&
                    medical_attention != nullptr &&
                    *medical_attention == " needs medical attention!" &&
                    recovery_last != nullptr &&
                    *recovery_last == "   wiz about now.",
                "Hotel menu choices or ROM-imported Lua text did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto pocket_secretary_root = scripts.menu(
        "pocket_secretary", "root", "pocket_secretary", "root");
    const auto pocket_secretary_root_text = scripts.menu(
        "pocket_secretary", "root_text_entries",
        "pocket_secretary.root.text", "catalog");
    const auto* pocket_secretary_title =
        scripts.dialogue_text("pocket_secretary.root.title");
    const auto* pocket_secretary_separator =
        scripts.dialogue_text("pocket_secretary.root.text.option.separator");
    constexpr std::array<std::string_view, 5> kPocketSecretaryFieldChoiceIds{{
        "notebook", "group_items", "cyberdeck", "dismiss_runner",
        "save_load"}};
    constexpr std::array<std::string_view, 5> kPocketSecretaryLabels{{
        "Notebook", "Group Items", "Cyberdeck", "Dismiss Runner",
        "Save/Load Game"}};
    constexpr std::array<std::string_view, 5>
        kPocketSecretaryLabelKeys{{
            "pocket_secretary.option.notebook",
            "pocket_secretary.option.group_items",
            "pocket_secretary.option.cyberdeck",
            "pocket_secretary.option.dismiss_runner",
            "pocket_secretary.option.save_load"}};
    bool pocket_secretary_rows_match =
        pocket_secretary_root.has_value() &&
        pocket_secretary_root->choices.size() ==
            kPocketSecretaryFieldChoiceIds.size();
    if (pocket_secretary_rows_match) {
        for (std::size_t index = 0;
             index < kPocketSecretaryFieldChoiceIds.size(); ++index) {
            const auto& choice = pocket_secretary_root->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            pocket_secretary_rows_match = pocket_secretary_rows_match &&
                choice.id == kPocketSecretaryFieldChoiceIds[index] &&
                choice.label_key == kPocketSecretaryLabelKeys[index] &&
                label != nullptr && *label == kPocketSecretaryLabels[index];
        }
    }
    if (!expect(pocket_secretary_root.has_value() &&
                    pocket_secretary_root->title_key ==
                        "pocket_secretary.root.title" &&
                    pocket_secretary_root_text.has_value() &&
                    pocket_secretary_root_text->title_key.empty() &&
                    pocket_secretary_root_text->choices.size() == 1U &&
                    pocket_secretary_root_text->choices[0].label_key ==
                        "pocket_secretary.root.text.option.separator" &&
                    pocket_secretary_rows_match &&
                    pocket_secretary_title != nullptr &&
                    *pocket_secretary_title == " Main Menu" &&
                    pocket_secretary_separator != nullptr &&
                    *pocket_secretary_separator ==
                        std::string("[") + std::string(10U, '\\'),
                "Pocket Secretary Lua menu or ROM text did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto notebook_options = scripts.menu(
        "pocket_secretary", "notebook", "pocket_secretary.notebook",
        "options");
    constexpr std::array<std::string_view, 5> kNotebookChoiceIds{{
        "current_run", "tips_and_clues", "contacts", "mr_johnsons",
        "shadowrunners"}};
    constexpr std::array<std::string_view, 5> kNotebookChoiceLabels{{
        "Current Run", "Tips & Clues", "Contacts", "Mr. Johnsons",
        "Shadowrunners"}};
    constexpr std::array<std::string_view, 5> kNotebookChoiceLabelKeys{{
        "pocket_secretary.notebook.option.current_run",
        "pocket_secretary.notebook.option.tips_and_clues",
        "pocket_secretary.notebook.option.contacts",
        "pocket_secretary.notebook.option.mr_johnsons",
        "pocket_secretary.notebook.option.shadowrunners"}};
    bool notebook_rows_match = notebook_options.has_value() &&
        notebook_options->choices.size() == kNotebookChoiceIds.size();
    if (notebook_rows_match) {
        const auto* title = scripts.dialogue_text(notebook_options->title_key);
        notebook_rows_match = title != nullptr && *title == "  Notebook";
        for (std::size_t index = 0; index < kNotebookChoiceIds.size(); ++index) {
            const auto& choice = notebook_options->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            notebook_rows_match = notebook_rows_match &&
                choice.id == kNotebookChoiceIds[index] && label != nullptr &&
                choice.label_key == kNotebookChoiceLabelKeys[index] &&
                *label == kNotebookChoiceLabels[index];
        }
    }
    if (!expect(notebook_options.has_value() &&
                    notebook_options->title_key ==
                        "pocket_secretary.notebook.options.title" &&
                    notebook_rows_match,
                "Lua Notebook choices or ROM labels changed order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto command_root = scripts.menu(
        "command_overview", "root", "field.command_overview", "root");
    constexpr std::array<std::string_view, 7> kCommandChoiceIds{{
        "inventory_left", "inventory_right", "clips", "attributes_skills",
        "cyberware", "magic", "pocket_secretary"}};
    constexpr std::array<std::string_view, 7> kCommandLabels{{
        "Inventory", "Inventory", "Clips", "Attributes/Skills",
        "Cyberware", "Magic", "Pocket Secretary"}};
    constexpr std::array<std::string_view, 7> kCommandLabelKeys{{
        "field.command_overview.option.inventory_left",
        "field.command_overview.option.inventory_right",
        "field.command_overview.option.clips",
        "field.command_overview.option.attributes_skills",
        "field.command_overview.option.cyberware",
        "field.command_overview.option.magic",
        "field.command_overview.option.pocket_secretary"}};
    bool command_rows_match = command_root.has_value() &&
        command_root->choices.size() == kCommandChoiceIds.size();
    if (command_rows_match) {
        for (std::size_t index = 0; index < kCommandChoiceIds.size(); ++index) {
            const auto& choice = command_root->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            command_rows_match = command_rows_match &&
                choice.id == kCommandChoiceIds[index] && label != nullptr &&
                choice.label_key == kCommandLabelKeys[index] &&
                *label == kCommandLabels[index];
        }
    }
    if (!expect(command_root.has_value() &&
                    command_root->title_key.empty() && command_rows_match,
                "Lua command overview choices or labels changed order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto clips_commands =
        scripts.menu("clips", "commands", "clips", "commands");
    const auto clips_trade_commands = scripts.menu(
        "clips", "trade_commands", "clips.trade", "commands");
    const auto clips_text_entries =
        scripts.menu("clips", "text_entries", "clips.text", "catalog");
    constexpr std::array<std::string_view, 4> kClipsCommandIds{{
        "reload", "trade", "distribute", "cancel"}};
    constexpr std::array<std::string_view, 4> kClipsCommandLabelKeys{{
        "clips.option.reload", "clips.option.trade",
        "clips.option.distribute", "clips.option.cancel"}};
    constexpr std::array<std::string_view, 4> kClipsCommandLabels{{
        "Reload", "Trade", "Distribute", "Cancel"}};
    constexpr std::array<std::string_view, 4> kClipsTradeCommandIds{{
        "increase_clips", "decrease_clips", "trade_clips",
        "change_recipient"}};
    constexpr std::array<std::string_view, 4> kClipsTradeCommandLabelKeys{{
        "clips.trade.option.increase_clips",
        "clips.trade.option.decrease_clips",
        "clips.trade.option.trade_clips",
        "clips.trade.option.change_recipient"}};
    constexpr std::array<std::string_view, 4> kClipsTradeCommandLabels{{
        "Increase Clips", "Decrease Clips", "Trade Clips",
        "Change Recipient"}};
    constexpr std::array<std::string_view, 21> kClipsTextIds{{
        "header_title", "header_separator", "counter_label",
        "trade_prompt_action", "trade_prompt_clips_to",
        "trade_prompt_recipient", "exchange_complete_first",
        "exchange_complete_second", "reload_prompt_ammo_remaining",
        "reload_prompt_ammo_suffix", "reload_prompt_question",
        "reload_success_first", "reload_success_second",
        "out_of_ammo_name_suffix", "out_of_ammo_clips",
        "item_without_clips_first", "item_without_clips_second",
        "item_without_clips_third", "no_party_first", "no_party_second",
        "no_party_third"}};
    constexpr std::array<std::string_view, 21> kClipsTextLabelKeys{{
        "clips.text.option.header_title",
        "clips.text.option.header_separator",
        "clips.text.option.counter_label",
        "clips.text.option.trade_prompt_action",
        "clips.text.option.trade_prompt_clips_to",
        "clips.text.option.trade_prompt_recipient",
        "clips.text.option.exchange_complete_first",
        "clips.text.option.exchange_complete_second",
        "clips.text.option.reload_prompt_ammo_remaining",
        "clips.text.option.reload_prompt_ammo_suffix",
        "clips.text.option.reload_prompt_question",
        "clips.text.option.reload_success_first",
        "clips.text.option.reload_success_second",
        "clips.text.option.out_of_ammo_name_suffix",
        "clips.text.option.out_of_ammo_clips",
        "clips.text.option.item_without_clips_first",
        "clips.text.option.item_without_clips_second",
        "clips.text.option.item_without_clips_third",
        "clips.text.option.no_party_first",
        "clips.text.option.no_party_second",
        "clips.text.option.no_party_third",
    }};
    bool clips_rows_match =
        clips_commands.has_value() && clips_trade_commands.has_value() &&
        clips_text_entries.has_value() &&
        clips_commands->choices.size() == kClipsCommandIds.size() &&
        clips_trade_commands->choices.size() ==
            kClipsTradeCommandIds.size() &&
        clips_text_entries->choices.size() == kClipsTextLabelKeys.size();
    if (clips_rows_match) {
        for (std::size_t index = 0; index < kClipsCommandIds.size(); ++index) {
            const auto* command_label = scripts.dialogue_text(
                clips_commands->choices[index].label_key);
            const auto* trade_label = scripts.dialogue_text(
                clips_trade_commands->choices[index].label_key);
            clips_rows_match = clips_rows_match &&
                clips_commands->choices[index].id == kClipsCommandIds[index] &&
                clips_commands->choices[index].label_key ==
                    kClipsCommandLabelKeys[index] &&
                command_label != nullptr &&
                *command_label == kClipsCommandLabels[index] &&
                clips_trade_commands->choices[index].id ==
                    kClipsTradeCommandIds[index] &&
                clips_trade_commands->choices[index].label_key ==
                    kClipsTradeCommandLabelKeys[index] &&
                trade_label != nullptr &&
                *trade_label == kClipsTradeCommandLabels[index];
        }
        for (std::size_t index = 0; index < kClipsTextLabelKeys.size();
             ++index) {
            clips_rows_match = clips_rows_match &&
                clips_text_entries->choices[index].id ==
                    kClipsTextIds[index] &&
                clips_text_entries->choices[index].label_key ==
                    kClipsTextLabelKeys[index];
        }
    }
    if (!expect(clips_rows_match,
                "Lua Clips selectors or ROM labels changed order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto confirmation =
        scripts.menu("ui", "confirmation", "ui", "confirmation");
    constexpr std::array<std::string_view, 2> kConfirmationIds{{"yes", "no"}};
    constexpr std::array<std::string_view, 2> kConfirmationLabels{{
        "Yes", "No"}};
    bool confirmation_rows_match = confirmation.has_value() &&
        confirmation->choices.size() == kConfirmationIds.size();
    if (confirmation_rows_match) {
        for (std::size_t index = 0; index < kConfirmationIds.size(); ++index) {
            const auto& choice = confirmation->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            confirmation_rows_match = confirmation_rows_match &&
                choice.id == kConfirmationIds[index] && label != nullptr &&
                *label == kConfirmationLabels[index];
        }
    }
    if (!expect(confirmation_rows_match,
                "Lua confirmation choices or ROM labels changed order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto inventory_actions =
        scripts.menu("items", "inventory_actions", "items",
                     "inventory_actions");
    constexpr std::array<std::string_view, 4> kInventoryActionIds{{
        "equip_or_use", "discard", "trade", "cancel"}};
    bool inventory_action_rows_match = inventory_actions.has_value() &&
        inventory_actions->choices.size() == kInventoryActionIds.size();
    if (inventory_action_rows_match) {
        for (std::size_t index = 0; index < kInventoryActionIds.size();
             ++index) {
            inventory_action_rows_match = inventory_action_rows_match &&
                inventory_actions->choices[index].id ==
                    kInventoryActionIds[index];
        }
    }
    if (!expect(inventory_action_rows_match,
                "Lua inventory action IDs changed source order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto magic_spells =
        scripts.menu("magic", "spells", "magic.spells", "catalog");
    constexpr std::array<std::string_view, 13> kMagicSpellIds{{
        "flame_dart", "flame_bolt", "hellblast", "invisibility", "sleep",
        "stink", "super_barrier", "mana_zap", "mana_blast", "mana_storm",
        "rockskin", "heal_wounds", "confusion"}};
    constexpr std::array<std::string_view, 13> kMagicSpellNames{{
        "Flame Dart", "Flame Bolt", "Hellblast", "Invisibility", "Sleep",
        "Stink", "Super Barrier", "Mana Zap", "Mana Blast", "Mana Storm",
        "Rockskin", "Heal Wounds", "Confusion"}};
    bool magic_spell_rows_match = magic_spells.has_value() &&
        magic_spells->choices.size() == kMagicSpellIds.size();
    if (magic_spell_rows_match) {
        for (std::size_t index = 0; index < kMagicSpellIds.size(); ++index) {
            const auto& choice = magic_spells->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            magic_spell_rows_match = magic_spell_rows_match &&
                choice.id == kMagicSpellIds[index] && label != nullptr &&
                *label == kMagicSpellNames[index] &&
                scripts.dialogue_text("magic.spells.description." +
                                      choice.id) != nullptr;
        }
    }
    if (!expect(magic_spell_rows_match,
                "Lua Magic spell IDs or ROM text changed source order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    constexpr std::array<std::string_view, 12> kShopServiceIds{{
        "weapons_world", "ares_weapon_emporium", "crime_mall",
        "medicine_lodge", "riannons", "merlins_lore", "microtronics",
        "wire_masters", "little_chiba", "dr_bobs",
        "council_island_hospital", "seattle_general_hospital"}};
    constexpr std::array<std::string_view, 12> kShopServiceLabels{{
        "Weapons World", "Ares Weapon Emporium", "Crime Mall",
        "Medicine Lodge", "Riannon's", "Merlin's Lore", "Microtronics",
        "Wire-Masters", "Little Chiba", "Dr. Bob's",
        "Council Island Hospital", "Seattle General Hospital"}};
    const auto shop_services =
        scripts.menu("shop", "services", "shop.service", "catalog");
    bool shop_service_rows_match = shop_services.has_value() &&
        shop_services->choices.size() == kShopServiceIds.size();
    if (shop_service_rows_match) {
        for (std::size_t index = 0; index < kShopServiceIds.size(); ++index) {
            const auto& choice = shop_services->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            shop_service_rows_match = shop_service_rows_match &&
                choice.id == kShopServiceIds[index] && label != nullptr &&
                choice.label_key ==
                    "shop.service.option." +
                        std::string(kShopServiceIds[index]) &&
                *label == kShopServiceLabels[index];
        }
    }
    if (!expect(shop_services.has_value() &&
                    shop_services->title_key.empty() &&
                    shop_service_rows_match &&
                    scripts.dialogue_text("shop.ares.title") == nullptr &&
                    scripts.dialogue_text("shop.service.action-title.12") ==
                        nullptr,
                "Lua shop service IDs or ROM title order changed")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    constexpr std::array<std::string_view, 32> kShopActionHeadingIds{{
        "ares_weapons_display", "ares_weapons_available",
        "ares_armor_display", "ares_armor_available",
        "ares_other_items_display", "ares_other_items_equipment",
        "ares_weapon_accessories_display",
        "ares_weapon_accessories_accessories",
        "ares_firearm_accessories_select",
        "ares_firearm_accessories_to_accessorize", "ares_sell_item_from",
        "ares_sell_inventory", "combat_utilities_display",
        "combat_utilities", "defense_utilities_display", "defense_utilities",
        "mask_sense_utilities_display", "mask_sense_utilities",
        "cyberdecks_display", "cyberdecks", "cyberdeck_upgrades_display",
        "cyberdeck_upgrades", "cyberdeck_upgrades_second_display",
        "cyberdeck_upgrades_second", "magic_spells_display", "magic_spells",
        "magic_items_display", "magic_items", "magic_foci_display",
        "magic_foci", "sell_item_from_secondary",
        "sell_inventory_secondary"}};
    constexpr std::array<std::string_view, 32> kShopActionHeadingLabels{{
        "      Display       ", " Available Weapons  ",
        "      Display       ", "  Available Armor   ",
        " Display Available  ", "     Equipment      ",
        "   Display Weapon   ", "    Accessories     ",
        "   Select Firearm   ", "   To Accessorize   ",
        "   Sell Item From   ", "     Inventory      ",
        " Display Available  ", "  Combat Utilities  ",
        " Display Available  ", " Defense Utilities  ",
        " Display Available  ", "Mask/Sense Utilities",
        " Display Available  ", "    Cyberdecks      ",
        " Display Available  ", " Cyberdeck Upgrades ",
        " Display Available  ", " Cyberdeck Upgrades ",
        " Display Available  ", "    Magic Spells    ",
        " Display Available  ", "    Magic Items     ",
        " Display Available  ", "    Magic Foci      ",
        "   Sell Item From   ", "     Inventory      "}};
    const auto shop_action_headings = scripts.menu(
        "shop", "action_headings", "shop.action_heading", "catalog");
    bool shop_action_heading_rows_match = shop_action_headings.has_value() &&
        shop_action_headings->choices.size() ==
            kShopActionHeadingIds.size();
    if (shop_action_heading_rows_match) {
        for (std::size_t index = 0; index < kShopActionHeadingIds.size();
             ++index) {
            const auto& choice = shop_action_headings->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            shop_action_heading_rows_match =
                shop_action_heading_rows_match &&
                choice.id == kShopActionHeadingIds[index] &&
                label != nullptr &&
                choice.label_key ==
                    "shop.action_heading.option." +
                        std::string(kShopActionHeadingIds[index]) &&
                *label == kShopActionHeadingLabels[index];
        }
    }
    if (!expect(shop_action_headings.has_value() &&
                    shop_action_headings->title_key.empty() &&
                    shop_action_heading_rows_match,
                "Lua shop headings or 32-row ROM order changed")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto shop = scripts.shop_definition("ares_weapon_emporium");
    if (!expect(shop.has_value() && shop->groups.size() == 6U,
                "Lua shop catalog did not define the Ares selector groups") ||
        !expect(shop->title_key ==
                    "shop.service.option.ares_weapon_emporium" &&
                    shop->groups[0].title_keys ==
                        std::array<std::string, 2>{
                            "shop.action_heading.option.ares_weapons_display",
                            "shop.action_heading.option.ares_weapons_available"} &&
                    shop->groups[5].title_keys ==
                        std::array<std::string, 2>{
                            "shop.action_heading.option.ares_sell_item_from",
                            "shop.action_heading.option.ares_sell_inventory"},
                "Lua shop title or category headings are not bound") ||
        !expect(shop->text_keys.transaction_prompt ==
                    "shop.transaction.prompt" &&
                    shop->text_keys.universal_clips_title ==
                        "shop.universal_clips.title" &&
                    shop->text_keys.universal_clips_description ==
                        "shop.universal_clips.description" &&
                    shop->text_keys.no_inventory_slot ==
                        "shop.transaction.no_inventory_slot" &&
                    shop->text_keys.insufficient_funds ==
                        "shop.transaction.insufficient_funds" &&
                    shop->text_keys.special_action_limit ==
                        "shop.transaction.special_action_limit" &&
                    shop->text_keys.protected_sale ==
                        "shop.transaction.protected_sale" &&
                    shop->text_keys.sale_quote_heading ==
                        "shop.transaction.sale_quote_heading" &&
                    shop->text_keys.purchase_price_heading ==
                        "shop.transaction.purchase_price_heading",
                "Lua shop transaction messages are not assigned") ||
        !expect(shop->groups[0].id == "weapons" &&
                    shop->groups[0].offers.size() == 7U &&
                    shop->groups[1].id == "other_items" &&
                    shop->groups[1].offers.size() == 4U &&
                    shop->groups[2].id == "armor" &&
                    shop->groups[2].offers.size() == 2U &&
                    shop->groups[5].id == "sell",
                "Lua shop catalog group order or stock was not preserved") ||
        !expect(shop->groups[0].offers[2].type == 0x06U &&
                    shop->groups[0].offers[2].quantity == 0x0FU &&
                    shop->groups[1].offers[0].type == 0x45U &&
                    shop->groups[2].offers[1].type == 0x2DU &&
                    shop->groups[2].offers[1].quantity == 0x0CU,
                "Lua shop offer records did not retain their native values")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }
    const auto* ares_title = scripts.dialogue_text(shop->title_key);
    const auto* transaction_prompt =
        scripts.dialogue_text(shop->text_keys.transaction_prompt);
    const auto* purchase_price =
        scripts.dialogue_text(shop->text_keys.purchase_price_heading);
    const auto* first_category_title =
        scripts.dialogue_text(shop->groups[0].title_keys[1]);
    if (!expect(ares_title != nullptr && *ares_title == "Ares Weapon Emporium" &&
                    transaction_prompt != nullptr &&
                    *transaction_prompt == "Complete Transaction?" &&
                    purchase_price != nullptr &&
                    *purchase_price == "Price        $" &&
                    first_category_title != nullptr &&
                    *first_category_title == " Available Weapons  " &&
                    scripts.dialogue_text(
                        shop->text_keys.universal_clips_title) != nullptr &&
                    *scripts.dialogue_text(
                        shop->text_keys.universal_clips_title) ==
                        "Universal\nGun Clips" &&
                    scripts.dialogue_text(
                        shop->text_keys.universal_clips_description) != nullptr &&
                    scripts.dialogue_text(
                        shop->text_keys.universal_clips_description)->find(
                        "Cost per Clip") != std::string::npos,
                "Lua shop text keys do not resolve to source-derived copy")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto items = scripts.item_catalog();
    if (!expect(items.has_value() && items->items.size() == 50U &&
                    items->detail_text_keys.size() == 32U &&
                    items->detail_label_keys.size() == 3U &&
                    items->items[0].code == 1U &&
                    items->items[0].name_key == "items.name.1" &&
                    items->items[49].code == 50U &&
                    items->items[49].category_key == "items.category.50" &&
                    items->detail_text_keys[31] == "items.action_detail.31",
                "Lua item catalog does not map the source item/detail tables") ||
        !expect(scripts.dialogue_text(items->items[0].name_key) != nullptr &&
                    *scripts.dialogue_text(items->items[0].name_key) ==
                        "Streetline" &&
                    scripts.dialogue_text(items->items[0].category_key) !=
                        nullptr &&
                    *scripts.dialogue_text(items->items[0].category_key) ==
                        "Special" &&
                    scripts.dialogue_text(items->items[15].category_key) !=
                        nullptr &&
                    scripts.dialogue_text(items->items[15].category_key)->empty() &&
                    scripts.dialogue_text(items->detail_text_keys[0]) != nullptr &&
                    *scripts.dialogue_text(items->detail_text_keys[0]) ==
                        "A small, cheap hold-out\npistol.\nLight damage, low power.",
                "Lua item text does not preserve ROM names, blank cells, and detail copy")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto rules = scripts.map_event_rules();
    if (!expect(rules.has_value() && rules->size() == 1U,
                "script package did not define its opening map event")) {
        std::cerr << scripts.failure() << '\n';
        return 2;
    }
    const auto& opening = rules->front();
    if (!expect(opening.scene_id == 2U &&
                    opening.trigger == FieldMapEventTrigger::scene_entry &&
                    opening.scope.has_value() &&
                    opening.scope->min_x == 0x3CE0U &&
                    opening.scope->max_x == 0x3CE0U &&
                    opening.scope->min_y == 0x2D00U &&
                    opening.scope->max_y == 0x2D00U &&
                    opening.repeat_policy ==
                        FieldMapEventRepeatPolicy::once_per_session &&
                    opening.action ==
                        FieldMapEventScriptAction::run_lua_handler &&
                    opening.handler == "new_game_opening_walk",
                "opening map event does not match its typed source profile")) {
        return 3;
    }
    const auto opening_flag = field_map_event_flag(
        FieldMapEventStateFlag::new_game_opening_completed);
    FieldMapEventSystem event_system(*rules);
    if (!expect(event_system.dispatch_scene_entry(
                    {2U, 0x3CE0U, 0x2D00U, 0U, true}).empty() &&
                    event_system.dispatch_scene_entry(
                        {2U, 0x3CE0U, 0x2D00U, opening_flag, true}).size() == 1U &&
                    event_system.dispatch_scene_entry(
                        {2U, 0x3CE0U, 0x2D00U, opening_flag, true}).empty(),
                "loaded opening rule did not enforce its fact and one-shot policy")) {
        return 3;
    }

    const auto task =
        scripts.start_task("field_events", opening.handler);
    if (!expect(task.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    task.operation.has_value() &&
                    task.operation->type ==
                        ScriptNativeOperation::Type::actor_walk_heading &&
                    task.operation->actor_id == "player" &&
                    task.operation->heading == 0U &&
                    scripts.task_waiting_for_native_operation(),
                "opening handler did not yield its typed player movement")) {
        std::cerr << scripts.failure() << '\n';
        return 4;
    }
    const auto resumed = scripts.complete_native_operation(true);
    if (!expect(resumed.status == ScriptTaskStep::Status::completed &&
                    !scripts.task_waiting_for_native_operation(),
                "opening handler did not resume after native movement")) {
        std::cerr << scripts.failure() << '\n';
        return 4;
    }

    const StoryFacts new_game{
        {"opening_dialogue_seen", false},
        {"michael_debt_paid", false},
        {"michael_package_collected", false},
    };
    scripts.set_location_context("motel", "stoker_motel");
    scripts.set_story_state(new_game);
    const auto first_visit =
        scripts.call_string("michael_package", "motel_entry");
    if (!expect(first_visit == "opening_dialogue",
                "first motel visit did not select the opening dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 5;
    }
    if (!expect(scripts.story_state().at("opening_dialogue_seen"),
                "Lua could not set a story flag through game.state")) {
        return 5;
    }

    const StoryFacts repeat_visit{
        {"opening_dialogue_seen", true},
        {"michael_debt_paid", false},
        {"michael_package_collected", false},
    };
    scripts.set_story_state(repeat_visit);
    const auto repeat =
        scripts.call_string("michael_package", "motel_entry");
    if (!expect(repeat == "package_offer",
                "repeat unpaid visit lost the Michael package offer")) {
        return 6;
    }

    const StoryFacts paid_but_not_collected{
        {"opening_dialogue_seen", true},
        {"michael_debt_paid", true},
        {"michael_package_collected", false},
    };
    scripts.set_story_state(paid_but_not_collected);
    if (!expect(scripts.call_string("michael_package", "motel_entry") ==
                    "package_offer",
                "paid debt hid an uncollected package")) {
        return 7;
    }

    const StoryFacts completed{
        {"opening_dialogue_seen", true},
        {"michael_debt_paid", true},
        {"michael_package_collected", true},
    };
    scripts.set_story_state(completed);
    if (!expect(scripts.call_string("michael_package", "motel_entry") ==
                    "ordinary_motel",
                "completed package quest did not return to motel service")) {
        return 8;
    }

    scripts.set_story_state(completed);
    scripts.set_location_context("motel", "stoker_motel");
    const auto manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(manager.has_value() && manager->choices.size() == 2U &&
                    manager->text_profile == "stoker.motel" &&
                    manager->speaker_id.empty() &&
                    manager->title_key == "stoker.motel.manager.title" &&
                    manager->choices[0].id == "crash" &&
                    manager->choices[0].label_key ==
                        "stoker.motel.option.crash" &&
                    manager->choices[1].id == "leave",
                "ordinary manager menu does not have its two source choices")) {
        std::cerr << scripts.failure() << '\n';
        return 9;
    }
    scripts.set_story_state(repeat_visit);
    scripts.set_location_context("motel", "downtown_motel");
    const auto ordinary_motel_manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(ordinary_motel_manager.has_value() &&
                    ordinary_motel_manager->choices.size() == 2U,
                "Michael quest hook leaked into another motel")) {
        return 10;
    }
    scripts.set_location_context("motel", "stoker_motel");
    const auto package_manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(package_manager.has_value() &&
                    package_manager->choices.size() == 3U &&
                    package_manager->choices[2].id == "package",
                "unpaid manager menu lacks the package choice")) {
        std::cerr << scripts.failure() << '\n';
        return 10;
    }
    scripts.set_story_state(paid_but_not_collected);
    const auto paid_package_manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(paid_package_manager.has_value() &&
                    paid_package_manager->choices.size() == 3U,
                "debt payment prematurely removed the uncollected package")) {
        return 11;
    }
    scripts.set_story_state(completed);
    const auto completed_package_manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(completed_package_manager.has_value() &&
                    completed_package_manager->choices.size() == 2U,
                "completed package facts did not remove the special choice")) {
        return 12;
    }

    scripts.set_story_state(repeat_visit);
    scripts.set_location_context(
        "motel", "stoker_motel", "stoker.motel", "stoker");
    const auto stoker_interaction =
        scripts.start_task("michael_package", "stoker_interaction");
    if (!expect(stoker_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    stoker_interaction.operation.has_value() &&
                    stoker_interaction.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    stoker_interaction.operation->menu.has_value() &&
                    stoker_interaction.operation->menu->choices.size() == 3U &&
                    stoker_interaction.operation->menu->speaker_id == "stoker",
                "repeat Stoker visit did not yield the shared motel menu")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    auto repeat_choice = scripts.complete_native_menu_choice("package");
    if (!expect(repeat_choice.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    repeat_choice.operation.has_value() &&
                    repeat_choice.operation->type ==
                        ScriptNativeOperation::Type::service_call &&
                    repeat_choice.operation->service_id ==
                        "michael_package.prepare_debt_offer",
                "Michael choice did not request its native quest-state service")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    repeat_choice = scripts.complete_native_service_result("ready");
    if (!expect(repeat_choice.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    repeat_choice.operation.has_value() &&
                    repeat_choice.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    repeat_choice.operation->menu.has_value() &&
                    repeat_choice.operation->menu->id == "debt" &&
                    repeat_choice.operation->menu->choices.size() == 2U,
                "Michael service did not resume into the scripted debt menu")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    repeat_choice = scripts.complete_native_menu_choice("jump-house");
    if (!expect(repeat_choice.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    repeat_choice.operation.has_value() &&
                    repeat_choice.operation->type ==
                        ScriptNativeOperation::Type::service_call &&
                    repeat_choice.operation->service_id ==
                        "michael_package.jump_house_referral",
                "Jump House choice did not yield its native referral service")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    repeat_choice = scripts.complete_native_service_result("ready");
    if (!expect(repeat_choice.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    repeat_choice.operation.has_value() &&
                    repeat_choice.operation->type ==
                        ScriptNativeOperation::Type::ui_say &&
                    repeat_choice.operation->dialogue.has_value() &&
                    repeat_choice.operation->dialogue->event_id ==
                        "jump-house",
                "referral service did not yield the script-selected dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    repeat_choice = scripts.complete_native_operation(true);
    if (!expect(repeat_choice.status == ScriptTaskStep::Status::completed,
                "Michael's scripted referral dialogue did not complete")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    scripts.set_story_state(new_game);
    const auto first_stoker_interaction =
        scripts.start_task("michael_package", "stoker_interaction");
    if (!expect(first_stoker_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    first_stoker_interaction.operation.has_value() &&
                    first_stoker_interaction.operation->type ==
                        ScriptNativeOperation::Type::ui_say &&
                    first_stoker_interaction.operation->dialogue.has_value() &&
                    first_stoker_interaction.operation->dialogue->event_id ==
                        "arrival" &&
                    scripts.story_state().at("opening_dialogue_seen"),
                "first Stoker visit did not set its flag and yield arrival dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    auto first_continuation = scripts.complete_native_operation(true);
    if (!expect(first_continuation.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    first_continuation.operation.has_value() &&
                    first_continuation.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    first_continuation.operation->menu.has_value() &&
                    first_continuation.operation->menu->choices.size() == 3U,
                "first-entry dialogue did not continue into the shared menu")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    first_continuation = scripts.complete_native_menu_choice("leave");
    if (!expect(first_continuation.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    first_continuation.operation.has_value() &&
                    first_continuation.operation->type ==
                        ScriptNativeOperation::Type::ui_say &&
                    first_continuation.operation->dialogue.has_value() &&
                    first_continuation.operation->dialogue->event_id ==
                        "leave",
                "first-visit motel Leave choice did not yield farewell dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    first_continuation = scripts.complete_native_operation(true);
    if (!expect(first_continuation.status ==
                        ScriptTaskStep::Status::completed &&
                    first_continuation.result_id ==
                        std::optional<std::string>{"leave"},
                "first Stoker visit did not finish after farewell dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    const auto manager_interaction =
        scripts.start_task("motel", "manager_interaction");
    if (!expect(manager_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    manager_interaction.operation.has_value() &&
                    manager_interaction.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    manager_interaction.operation->menu.has_value() &&
                    manager_interaction.operation->menu->title_key.empty() &&
                    manager_interaction.operation->menu->choices.size() == 3U,
                "shared motel script did not yield an unmapped stable-id menu")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    const auto manager_menu_content = scripts.resolve_dialogue_menu(
        *manager_interaction.operation->menu);
    if (!expect(manager_menu_content.has_value() &&
                    manager_menu_content->title_key ==
                        "stoker.motel.manager.title" &&
                    manager_menu_content->choices[0].label_key ==
                        "stoker.motel.option.crash" &&
                    manager_menu_content->choices[1].label_key ==
                        "stoker.motel.option.leave" &&
                    manager_menu_content->choices[2].label_key ==
                        "stoker.motel.option.package",
                "Lua did not map the shared motel menu text explicitly")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    const auto menu_selection =
        scripts.complete_native_menu_choice("package");
    if (!expect(menu_selection.status == ScriptTaskStep::Status::completed &&
                    menu_selection.result_id ==
                        std::optional<std::string>{"package"},
                "native menu choice did not resume Lua with its stable id")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    auto room_interaction =
        scripts.start_task("motel", "manager_interaction");
    if (!expect(room_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    room_interaction.operation.has_value() &&
                    room_interaction.operation->type ==
                        ScriptNativeOperation::Type::ui_menu,
                "motel interaction did not offer its manager menu")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    room_interaction = scripts.complete_native_menu_choice("crash");
    if (!expect(room_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    room_interaction.operation.has_value() &&
                    room_interaction.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    room_interaction.operation->menu.has_value() &&
                    room_interaction.operation->menu->id == "room" &&
                    room_interaction.operation->menu->choices.size() == 2U,
                "Crash choice did not continue into the generic room menu")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    room_interaction = scripts.complete_native_menu_choice("pay");
    if (!expect(room_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    room_interaction.operation.has_value() &&
                    room_interaction.operation->type ==
                        ScriptNativeOperation::Type::service_call &&
                    room_interaction.operation->service_id ==
                        "motel.pay_room",
                "room menu did not yield its native payment service")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    room_interaction = scripts.complete_native_service_result("paid");
    if (!expect(room_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    room_interaction.operation.has_value() &&
                    room_interaction.operation->type ==
                        ScriptNativeOperation::Type::ui_say &&
                    room_interaction.operation->dialogue.has_value() &&
                    room_interaction.operation->dialogue->event_id ==
                        "room.paid",
                "payment result did not select its scripted result dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    room_interaction = scripts.complete_native_operation(true);
    if (!expect(room_interaction.status ==
                        ScriptTaskStep::Status::completed &&
                    room_interaction.result_id ==
                        std::optional<std::string>{"hotel-options"},
                "successful room payment did not request hotel options")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    const auto leave_interaction =
        scripts.start_task("motel", "manager_interaction");
    if (!expect(leave_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    leave_interaction.operation.has_value() &&
                    leave_interaction.operation->type ==
                        ScriptNativeOperation::Type::ui_menu,
                "shared motel interaction did not open its manager menu")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    auto leave_step = scripts.complete_native_menu_choice("leave");
    if (!expect(leave_step.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    leave_step.operation.has_value() &&
                    leave_step.operation->type ==
                        ScriptNativeOperation::Type::ui_say &&
                    leave_step.operation->dialogue.has_value() &&
                    leave_step.operation->dialogue->speaker_id == "stoker" &&
                    leave_step.operation->dialogue->text_profile ==
                        "stoker.motel" &&
                    leave_step.operation->dialogue->event_id == "leave",
                "shared leave choice did not resolve location dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    leave_step = scripts.complete_native_operation(true);
    if (!expect(leave_step.status == ScriptTaskStep::Status::completed &&
                    leave_step.result_id ==
                        std::optional<std::string>{"leave"},
                "motel leave flow did not return its stable choice ID")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    scripts.set_story_state(completed);
    scripts.set_location_context("motel", "downtown_motel",
                                 "downtown.motel");
    auto generic_motel =
        scripts.start_task("motel", "manager_interaction");
    if (!expect(generic_motel.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    generic_motel.operation.has_value() &&
                    generic_motel.operation->menu.has_value() &&
                    generic_motel.operation->menu->choices.size() == 2U,
                "shared motel handler did not work without Stoker context")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    generic_motel = scripts.complete_native_menu_choice("leave");
    if (!expect(generic_motel.status == ScriptTaskStep::Status::completed &&
                    generic_motel.result_id ==
                        std::optional<std::string>{"leave"},
                "generic motel Leave incorrectly required a speaker dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    StoryFacts native_story{
        {"opening_dialogue_seen", false},
        {"michael_debt_paid", false},
        {"michael_package_collected", false},
    };
    scripts.set_story_fact_accessors(
        [&native_story](const std::string_view name)
            -> std::optional<bool> {
            const auto fact = native_story.find(std::string(name));
            return fact == native_story.end()
                       ? std::nullopt
                       : std::optional<bool>{fact->second};
        },
        [&native_story](const std::string_view name, const bool value) {
            const auto fact = native_story.find(std::string(name));
            if (fact == native_story.end()) {
                return false;
            }
            fact->second = value;
            return true;
        });
    const auto native_first_visit =
        scripts.call_string("michael_package", "motel_entry");
    if (!expect(native_first_visit == "opening_dialogue" &&
                    native_story.at("opening_dialogue_seen"),
                "Lua did not update the bound native story state directly")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    native_story["michael_debt_paid"] = true;
    native_story["michael_package_collected"] = true;
    const auto native_completed_visit =
        scripts.call_string("michael_package", "motel_entry");
    if (!expect(native_completed_visit == "ordinary_motel",
                "Lua did not read current facts from the native story state")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    GameScriptRuntime sequence_scripts;
    if (!expect(sequence_scripts.load_package(
                    SHADOWRUN_GAMEPLAY_SCRIPT_FIXTURE_ROOT),
                "coroutine fixture did not load")) {
        std::cerr << sequence_scripts.failure() << '\n';
        return 14;
    }
    auto sequence_step =
        sequence_scripts.start_task("field_events", "walk_twice");
    if (!expect(sequence_step.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    sequence_step.operation.has_value() &&
                    sequence_step.operation->heading == 0U,
                "first sequential actor operation did not yield")) {
        std::cerr << sequence_scripts.failure() << '\n';
        return 14;
    }
    sequence_step = sequence_scripts.complete_native_operation(true);
    if (!expect(sequence_step.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    sequence_step.operation.has_value() &&
                    sequence_step.operation->heading == 1U,
                "script did not resume and yield its second actor operation")) {
        std::cerr << sequence_scripts.failure() << '\n';
        return 14;
    }
    sequence_step = sequence_scripts.complete_native_operation(true);
    if (!expect(sequence_step.status == ScriptTaskStep::Status::completed,
                "script did not finish after sequential native operations")) {
        std::cerr << sequence_scripts.failure() << '\n';
        return 14;
    }

    sequence_step = sequence_scripts.start_task(
        "field_events", "service_roundtrip");
    if (!expect(sequence_step.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    sequence_step.operation.has_value() &&
                    sequence_step.operation->type ==
                        ScriptNativeOperation::Type::service_call &&
                    sequence_step.operation->service_id ==
                        "inventory.collect",
                "service operation did not yield its namespaced action id")) {
        std::cerr << sequence_scripts.failure() << '\n';
        return 14;
    }
    sequence_step =
        sequence_scripts.complete_native_service_result("accepted");
    if (!expect(sequence_step.status == ScriptTaskStep::Status::completed &&
                    sequence_step.result_id ==
                        std::optional<std::string>{"item-collected"},
                "service result did not resume Lua with its stable result id")) {
        std::cerr << sequence_scripts.failure() << '\n';
        return 14;
    }
    return 0;
}
