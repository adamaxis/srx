#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <array>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

using namespace shadowrun::ghidra_conversion::scripting;

constexpr std::array<std::string_view, 13> kContactIds{{
    "capt_quinton_jaymes", "alesandro_hobbs", "sharkey", "kipp_david",
    "wilma_temmenhoff", "macklemann", "gregory_wilns",
    "mr_faradouchi", "agira_tetsumi", "henry_j_culver", "max",
    "chillicut", "professor_jefferson",
}};

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

std::optional<unsigned> parse_index(const std::string_view fact,
                                    const std::string_view prefix) {
    if (fact.size() <= prefix.size() ||
        fact.substr(0U, prefix.size()) != prefix) {
        return std::nullopt;
    }
    unsigned index = 0U;
    const auto suffix = fact.substr(prefix.size());
    const auto parsed = std::from_chars(
        suffix.data(), suffix.data() + suffix.size(), index);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != suffix.data() + suffix.size() || index >= 13U) {
        return std::nullopt;
    }
    return index;
}

bool is_menu(const ScriptTaskStep& step) {
    return step.status == ScriptTaskStep::Status::awaiting_native_operation &&
           step.operation.has_value() &&
           step.operation->type == ScriptNativeOperation::Type::ui_menu &&
           step.operation->menu.has_value();
}

bool is_cursor_menu(const ScriptTaskStep& step) {
    return is_menu(step) && step.operation->menu->input_mode ==
                                 ScriptMenuInputMode::cursor;
}

bool is_say(const ScriptTaskStep& step, const std::string_view event_id,
            const std::string_view speaker_id) {
    return step.status == ScriptTaskStep::Status::awaiting_native_operation &&
           step.operation.has_value() &&
           step.operation->type == ScriptNativeOperation::Type::ui_say &&
           step.operation->dialogue.has_value() &&
           step.operation->dialogue->event_id == event_id &&
           step.operation->dialogue->speaker_id == speaker_id;
}

bool is_service_call(const ScriptTaskStep& step,
                     const std::string_view service_id) {
    return step.status == ScriptTaskStep::Status::awaiting_native_operation &&
           step.operation.has_value() &&
           step.operation->type == ScriptNativeOperation::Type::service_call &&
           step.operation->service_id == service_id;
}

bool test_alesandro_service_tree(GameScriptRuntime& scripts,
                                 std::uint32_t& contact_flags) {
    contact_flags = 1U << 1U;
    auto task = scripts.start_task("field_activities", "vid_phone");
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(is_menu(task) && task.operation->menu->speaker_id ==
                    "vid_phone_alesandro_hobbs" &&
                    task.operation->menu->id ==
                        "call_alesandro_hobbs_first_call" &&
                    (contact_flags & (1U << 14U)) != 0U,
                "Alesandro call did not set history or select his speaker")) {
        return false;
    }
    const auto call_profile = scripts.resolve_dialogue_menu(
        *task.operation->menu);
    if (!expect(call_profile.has_value() &&
                    scripts.dialogue_text(call_profile->title_key) != nullptr,
                "Alesandro first-call copy did not resolve")) {
        return false;
    }

    task = scripts.complete_native_menu_choice("services");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "alesandro_services",
                "Alesandro Services did not open his contact menu")) {
        return false;
    }
    auto profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(profile.has_value() && profile->choices.size() == 3U &&
                    scripts.dialogue_text(profile->title_key) != nullptr,
                "Alesandro service options or title did not resolve")) {
        return false;
    }
    for (const auto& choice : profile->choices) {
        if (!expect(scripts.dialogue_text(choice.label_key) != nullptr,
                    "Alesandro service option is missing its label")) {
            return false;
        }
    }

    task = scripts.complete_native_menu_choice("corporation_information");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "alesandro_security_topics",
                "corporation information did not open its topic choices")) {
        return false;
    }
    profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(profile.has_value() && profile->choices.size() == 3U &&
                    scripts.dialogue_text(profile->title_key) != nullptr,
                "corporation procedures text or source topics did not resolve")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("second_security_type");
    if (!expect(is_say(task, "security_technological",
                       "vid_phone_alesandro_hobbs"),
                "second security type did not show Alesandro's ROM line")) {
        return false;
    }
    const auto technological_text = scripts.dialogue_event_text_keys(
        "vid_phone_alesandro_hobbs", "field.vid_phone",
        "security_technological");
    if (!expect(technological_text.has_value() &&
                    scripts.dialogue_text(technological_text->title_key) !=
                        nullptr &&
                    scripts.dialogue_text(technological_text->title_key)->find(
                        "Maglock") != std::string::npos,
                "technological security text did not include the complete ROM record")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "alesandro_services",
                "security explanation did not return to Alesandro's services")) {
        return false;
    }

    task = scripts.complete_native_menu_choice("maglock_passkey");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "alesandro_passkey_offer",
                "Maglock Passkey option did not open its two-tier offer")) {
        return false;
    }
    profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(profile.has_value() && profile->choices.size() == 3U &&
                    scripts.dialogue_text(profile->title_key) != nullptr &&
                    scripts.dialogue_text(profile->choices[0].label_key) !=
                        nullptr &&
                    scripts.dialogue_text(profile->choices[1].label_key) !=
                        nullptr,
                "level-3/level-5 offer copy or choices did not resolve")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("level_3_passkey");
    if (!expect(is_service_call(
                    task, "vid_phone.purchase_maglock_passkey_level_3"),
                "level-3 selection did not request the passkey purchase")) {
        return false;
    }
    task = scripts.complete_native_service_result("purchased");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "alesandro_purchase_result",
                "passkey purchase did not show its delivery result")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("services");
    task = scripts.complete_native_menu_choice("seattle_official");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "alesandro_official_contact_confirmation",
                "official offer did not show its confirmation choices")) {
        return false;
    }
    profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(profile.has_value() && profile->choices.size() == 2U &&
                    scripts.dialogue_text(profile->title_key) != nullptr &&
                    scripts.dialogue_text(profile->title_key)->find("8,000") !=
                        std::string::npos,
                "official introduction or 8,000 offer text did not resolve")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("pay_for_macklemann");
    if (!expect(is_service_call(task, "vid_phone.hire_macklemann"),
                "Macklemann offer did not request the contact service")) {
        return false;
    }
    task = scripts.complete_native_service_result("hired");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "alesandro_macklemann_result",
                "Macklemann introduction did not show its result screen")) {
        return false;
    }
    profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(profile.has_value() &&
                    scripts.dialogue_text(profile->title_key) != nullptr &&
                    scripts.dialogue_text(profile->title_key)->find(
                        "Sergeant Mackelmann") != std::string::npos,
                "Macklemann's name and security hint were not imported")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("goodbye");
    return expect(task.status == ScriptTaskStep::Status::completed,
                  "Macklemann result Goodbye did not finish Alesandro's call");
}

bool test_sharkey_service_tree(GameScriptRuntime& scripts,
                               std::uint32_t& contact_flags) {
    contact_flags = 1U << 2U;
    auto task = scripts.start_task("field_activities", "vid_phone");
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "call_sharkey_first_call" &&
                    task.operation->menu->speaker_id == "vid_phone_sharkey",
                "Sharkey call did not select her ROM contact profile")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("services");
    if (!expect(is_menu(task) &&
                    task.operation->menu->id == "sharkey_services",
                "Sharkey Services did not open the three ROM service choices")) {
        return false;
    }
    auto profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(profile.has_value() && profile->choices.size() == 3U &&
                    scripts.dialogue_text(profile->title_key) != nullptr,
                "Sharkey's root service copy or choices did not resolve")) {
        return false;
    }
    for (const auto& choice : profile->choices) {
        if (!expect(scripts.dialogue_text(choice.label_key) != nullptr,
                    "Sharkey's root service label is missing ROM text")) {
            return false;
        }
    }

    task = scripts.complete_native_menu_choice("pass_salish");
    if (!expect(is_menu(task),
                "Salish choice did not produce its offer menu")) {
        return false;
    }
    profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(task.operation->menu->id == "sharkey_salish_passage" &&
                    profile.has_value() && profile->choices.size() == 3U &&
                    scripts.dialogue_text(profile->title_key) != nullptr &&
                    scripts.dialogue_text(profile->title_key)->find("3,000") !=
                        std::string::npos,
                "Salish-Shidhe passage did not show its source price and choices")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("pay_for_passage");
    if (!expect(is_service_call(task, "vid_phone.pass_salish_shidhe"),
                "Salish passage did not call ROM command 0x6E's native service")) {
        return false;
    }
    task = scripts.complete_native_service_result("insufficient");
    if (!expect(is_say(task, "insufficient_funds", "vid_phone_sharkey"),
                "insufficient passage funds did not show Sharkey's ROM response")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(is_menu(task) &&
                    task.operation->menu->id == "sharkey_services",
                "failed passage payment did not return to Sharkey's services")) {
        return false;
    }

    task = scripts.complete_native_menu_choice("pass_sinsearach");
    task = scripts.complete_native_menu_choice("pay_for_passage");
    if (!expect(is_service_call(task, "vid_phone.pass_sinsearach"),
                "Sinsearach passage did not call ROM command 0x6F's service")) {
        return false;
    }
    task = scripts.complete_native_service_result("passage_requested");
    if (!expect(is_say(task, "passage_result", "vid_phone_sharkey"),
                "successful passage did not show the wilderness arrival text")) {
        return false;
    }
    const auto passage_text = scripts.dialogue_event_text_keys(
        "vid_phone_sharkey", "field.vid_phone", "passage_result");
    if (!expect(passage_text.has_value() &&
                    scripts.dialogue_text(passage_text->title_key) != nullptr &&
                    scripts.dialogue_text(passage_text->title_key)->find(
                        "wilderness") != std::string::npos,
                "passage result does not contain the full ROM travel copy")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.status == ScriptTaskStep::Status::completed &&
                    task.result_id == "field_phone_passage_requested",
                "passage acknowledgement did not return a typed map request")) {
        return false;
    }

    task = scripts.start_task("field_activities", "vid_phone");
    task = scripts.complete_native_menu_input("confirm");
    task = scripts.complete_native_menu_choice("services");
    task = scripts.complete_native_menu_choice("smart_goggles");
    if (!expect(is_menu(task),
                "Smart Goggles choice did not produce its offer menu")) {
        return false;
    }
    profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(task.operation->menu->id ==
                    "sharkey_smart_goggles_offer" && profile.has_value() &&
                    profile->choices.size() == 3U &&
                    scripts.dialogue_text(profile->title_key) != nullptr &&
                    scripts.dialogue_text(profile->title_key)->find("3,000") !=
                        std::string::npos,
                "Smart Goggles offer did not resolve its source text and options")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("confirm_purchase");
    if (!expect(is_service_call(task, "vid_phone.purchase_smart_goggles"),
                "Smart Goggles confirmation did not request the item purchase")) {
        return false;
    }
    task = scripts.complete_native_service_result("purchased");
    if (!expect(is_say(task, "smart_goggles_result", "vid_phone_sharkey"),
                "Smart Goggles purchase did not show its ROM result text")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(is_menu(task) &&
                    task.operation->menu->id == "sharkey_services",
                "goggles purchase result did not cleanly redraw Services")) {
        return false;
    }

    task = scripts.complete_native_menu_choice("smart_goggles");
    task = scripts.complete_native_menu_choice("confirm_purchase");
    task = scripts.complete_native_service_result("out_of_stock");
    if (!expect(is_say(task, "smart_goggles_inventory_full",
                       "vid_phone_sharkey"),
                "full inventory did not show the ROM goggles stock response")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(is_menu(task) &&
                    task.operation->menu->id == "sharkey_services",
                "full inventory response did not return to a clean Services menu")) {
        return false;
    }

    task = scripts.complete_native_menu_choice("pass_salish");
    task = scripts.complete_native_menu_choice("other_subject");
    return expect(task.status == ScriptTaskStep::Status::completed,
                  "Sharkey's Other Subject option did not end the call");
}

} // namespace

int main() {
    GameScriptRuntime scripts;
    if (!expect(scripts.load_package(
                    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
                    std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                        "dialogue" / "script-text.lua"),
                "Vid-Phone Lua package did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    for (const auto contact_id : kContactIds) {
        const auto speaker_id = "vid_phone_" + std::string(contact_id);
        const auto profile = scripts.speaker_profile(speaker_id);
        if (!expect(profile.has_value() &&
                        profile->text_profile == "field.vid_phone" &&
                        !profile->portrait_alias.empty(),
                    "a Vid-Phone contact has no Lua speaker/portrait profile")) {
            return 16;
        }
        for (const auto event_id : {"first_call", "repeat_call"}) {
            const auto event = scripts.dialogue_event_text_keys(
                speaker_id, profile->text_profile, event_id);
            if (!expect(event.has_value() &&
                            scripts.dialogue_text(event->title_key) != nullptr,
                        "a Vid-Phone contact is missing first/repeat call text")) {
                return 17;
            }
        }
    }

    std::uint32_t contact_flags = 0U;
    std::uint16_t matrix_passcode_flags = 0U;
    bool cyberdeck_present = false;
    bool mpcp_functional = false;
    bool datajack_present = false;
    scripts.set_story_fact_accessors(
        [&contact_flags, &matrix_passcode_flags, &cyberdeck_present,
         &mpcp_functional, &datajack_present](const std::string_view name)
            -> std::optional<bool> {
            if (name == "cyberspace_deck_present") return cyberdeck_present;
            if (name == "cyberspace_mpcp_functional") {
                return mpcp_functional;
            }
            if (name == "cyberspace_datajack_present") {
                return datajack_present;
            }
            if (name == "matrix_passcode_flags_resolved") return true;
            if (name == "matrix_current_passcode_available") {
                return (matrix_passcode_flags & 1U) != 0U;
            }
            if (const auto index = parse_index(name, "matrix_passcode_bit_")) {
                return (matrix_passcode_flags & (1U << *index)) != 0U;
            }
            if (name == "vid_phone_contacts_resolved") {
                return true;
            }
            if (const auto index = parse_index(name, "vid_phone_contact_")) {
                return (contact_flags & (1U << *index)) != 0U;
            }
            if (const auto index = parse_index(name, "vid_phone_called_")) {
                return (contact_flags & (1U << (*index + 13U))) != 0U;
            }
            return std::nullopt;
        },
        [&contact_flags](const std::string_view name, const bool value) {
            const auto index = parse_index(name, "vid_phone_called_");
            if (!index.has_value()) {
                return false;
            }
            const auto mask = static_cast<std::uint32_t>(1U << (*index + 13U));
            contact_flags = value ? contact_flags | mask
                                  : contact_flags & ~mask;
            return true;
        });
    scripts.set_location_context(
        "field_map", "scene_2", "field.main_menu", "");

    auto task = scripts.start_task(
        "field_activities", "ordinary_field_terminal");
    if (!expect(is_cursor_menu(task) &&
                    task.operation->menu->choices.size() == 4U,
                "ordinary field terminal did not open its four Lua choices")) {
        std::cerr << scripts.failure() << '\n';
        return 2;
    }
    task = scripts.complete_native_menu_input("down");
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(task.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    task.operation.has_value() &&
                    task.operation->type ==
                        ScriptNativeOperation::Type::ui_say &&
                    task.operation->dialogue.has_value() &&
                    task.operation->dialogue->speaker_id == "field_narrator" &&
                    task.operation->dialogue->text_profile ==
                        "field.world_event" &&
                    task.operation->dialogue->event_id ==
                        "phone_no_contacts",
                "empty contact list did not yield the ROM no-contact message")) {
        std::cerr << scripts.failure() << '\n';
        return 3;
    }
    const auto no_contacts = scripts.dialogue_event_text_keys(
        "field_narrator", "field.world_event", "phone_no_contacts");
    if (!expect(no_contacts.has_value() &&
                    scripts.dialogue_text(no_contacts->title_key) != nullptr,
                "no-contact message has no imported ROM text")) {
        std::cerr << scripts.failure() << '\n';
        return 4;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(is_cursor_menu(task) &&
                    task.operation->menu->id == "ordinary",
                "no-contact message did not return to the field menu")) {
        std::cerr << scripts.failure() << '\n';
        return 5;
    }
    task = scripts.complete_native_menu_input("cancel");
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "field menu Exit did not finish the ordinary terminal script")) {
        std::cerr << scripts.failure() << '\n';
        return 6;
    }

    contact_flags = 0x1FFFU;
    task = scripts.start_task("field_activities", "vid_phone");
    if (!expect(is_cursor_menu(task) &&
                    task.operation->menu->id == "contacts" &&
                    task.operation->menu->choices.size() == 8U &&
                    task.operation->menu->text_profile == "field.vid_phone" &&
                    task.operation->menu->speaker_id.empty(),
                "contact page one did not yield its eight-row phone layout")) {
        std::cerr << scripts.failure() << '\n';
        return 7;
    }
    const auto first_page = scripts.resolve_dialogue_menu(
        *task.operation->menu);
    if (!expect(first_page.has_value() &&
                    first_page->layout == ScriptMenuLayout::compact_list &&
                    first_page->title_key.empty() &&
                    first_page->choices.front().label_key ==
                        "pocket_secretary.notebook.contact.name.option.record_0" &&
                    first_page->choices.back().label_key ==
                        "vid_phone.contact.page_next",
                "contact page one labels or compact layout did not resolve")) {
        for (const auto& choice : task.operation->menu->choices) {
            std::cerr << "contact choice: " << choice.id << '\n';
        }
        std::cerr << "profile=" << task.operation->menu->text_profile
                  << " speaker=" << task.operation->menu->speaker_id << '\n';
        std::cerr << scripts.failure() << '\n';
        return 8;
    }
    for (const auto& choice : first_page->choices) {
        if (!expect(scripts.dialogue_text(choice.label_key) != nullptr,
                    "a first-page contact label is missing ROM text")) {
            return 18;
        }
    }
    for (unsigned row = 0U; row < 7U; ++row) {
        task = scripts.complete_native_menu_input("down");
    }
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(is_cursor_menu(task) &&
                    task.operation->menu->choices.size() == 7U &&
                    task.operation->menu->choices.front().id ==
                        "mr_faradouchi" &&
                    task.operation->menu->choices.back().id == "prior_screen",
                "contact page two did not place Prior Screen after the contacts")) {
        std::cerr << scripts.failure() << '\n';
        return 9;
    }
    const auto second_page = scripts.resolve_dialogue_menu(
        *task.operation->menu);
    if (!expect(second_page.has_value(),
                "contact page two labels did not resolve")) {
        return 19;
    }
    for (const auto& choice : second_page->choices) {
        if (!expect(scripts.dialogue_text(choice.label_key) != nullptr,
                    "a second-page contact label is missing ROM text")) {
            return 20;
        }
    }

    task = scripts.complete_native_menu_input("down");
    task = scripts.complete_native_menu_input("down");
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(is_menu(task) &&
                    task.operation->menu->text_profile == "field.vid_phone" &&
                    task.operation->menu->speaker_id ==
                        "vid_phone_henry_j_culver" &&
                    task.operation->menu->id ==
                        "call_henry_j_culver_first_call" &&
                    (contact_flags & (1U << 22U)) != 0U,
                "first contact call did not set history or speaker profile")) {
        std::cerr << scripts.failure() << '\n';
        return 10;
    }
    const auto henry_menu = scripts.resolve_dialogue_menu(
        *task.operation->menu);
    if (!expect(henry_menu.has_value() &&
                    henry_menu->choices.size() == 2U &&
                    henry_menu->title_key ==
                        "vid_phone.contact.henry_j_culver.first_call" &&
                    scripts.dialogue_text(henry_menu->title_key) != nullptr &&
                    scripts.speaker_profile("vid_phone_henry_j_culver")
                            .has_value() &&
                    scripts.speaker_profile("vid_phone_henry_j_culver")
                            ->portrait_alias ==
                        "dialogue.portraits.vid.phone.henry.j.culver",
                "first call copy or portrait profile did not resolve")) {
        std::cerr << scripts.failure() << '\n';
        return 11;
    }
    task = scripts.complete_native_menu_choice("goodbye");
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "Henry call did not return from the Vid-Phone activity")) {
        return 26;
    }

    task = scripts.start_task("field_activities", "vid_phone");
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(is_menu(task) &&
                    task.operation->menu->speaker_id ==
                        "vid_phone_capt_quinton_jaymes" &&
                    task.operation->menu->id ==
                        "call_capt_quinton_jaymes_first_call" &&
                    (contact_flags & (1U << 13U)) != 0U,
                "first Captain Quinton call did not set its call-history bit")) {
        std::cerr << scripts.failure() << '\n';
        return 12;
    }
    task = scripts.complete_native_menu_choice("services");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_services",
                "Captain Quinton Services did not open its source menu")) {
        std::cerr << scripts.failure() << '\n';
        return 27;
    }
    auto menu_profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(menu_profile.has_value() && menu_profile->choices.size() == 3U &&
                    menu_profile->title_key ==
                        "vid_phone.capt_quinton_jaymes.services",
                "Captain Quinton Services title or choices did not resolve")) {
        return 28;
    }
    for (const auto& choice : menu_profile->choices) {
        if (!expect(scripts.dialogue_text(choice.label_key) != nullptr,
                    "a Captain Quinton service label is missing ROM text")) {
            return 29;
        }
    }

    task = scripts.complete_native_menu_choice("hollywood_information");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_hollywood_intro",
                "Hollywood information did not show its first ROM prompt")) {
        return 45;
    }
    menu_profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(menu_profile.has_value() && menu_profile->choices.size() == 1U &&
                    scripts.dialogue_text(menu_profile->title_key) != nullptr &&
                    scripts.dialogue_text(menu_profile->choices.front().label_key)
                        != nullptr,
                "Hollywood opening copy or More label did not resolve")) {
        return 46;
    }
    task = scripts.complete_native_menu_choice("more");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_hollywood_topics",
                "Hollywood More choice did not open its topic prompt")) {
        return 47;
    }
    menu_profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(menu_profile.has_value() && menu_profile->choices.size() == 3U &&
                    scripts.dialogue_text(menu_profile->title_key) != nullptr,
                "Hollywood topic prompt did not resolve its three choices")) {
        return 48;
    }
    for (const auto& choice : menu_profile->choices) {
        if (!expect(scripts.dialogue_text(choice.label_key) != nullptr,
                    "a Hollywood topic choice has no ROM label")) {
            return 49;
        }
    }
    task = scripts.complete_native_menu_choice("weller_prison_break");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_weller_intro",
                "Weller choice did not reach its ROM information node")) {
        return 50;
    }
    task = scripts.complete_native_menu_choice("more");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_weller_result",
                "Weller More choice did not reach the advice result node")) {
        return 51;
    }
    menu_profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(menu_profile.has_value() &&
                    menu_profile->choices.size() == 2U &&
                    scripts.dialogue_text(menu_profile->title_key) != nullptr,
                "Weller advice or its Services/Goodbye choices did not resolve")) {
        return 52;
    }
    task = scripts.complete_native_menu_choice("services");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_services",
                "Weller result Services choice did not return to Quinton")) {
        return 53;
    }
    task = scripts.complete_native_menu_choice("hollywood_information");
    task = scripts.complete_native_menu_choice("more");
    task = scripts.complete_native_menu_choice("disconnect");
    if (!expect(is_say(task, "services_disconnected",
                       "vid_phone_capt_quinton_jaymes"),
                "disconnect topic did not show the ROM terminal message")) {
        return 54;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "terminal disconnect message did not end this phone call")) {
        return 55;
    }

    task = scripts.start_task("field_activities", "vid_phone");
    task = scripts.complete_native_menu_input("confirm");
    task = scripts.complete_native_menu_choice("services");
    task = scripts.complete_native_menu_choice("firepower");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_firepower_intro",
                "firepower did not show Quinton's first source prompt")) {
        return 30;
    }
    task = scripts.complete_native_menu_choice("more");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_firepower_items",
                "firepower More choice did not open the item prompt")) {
        return 31;
    }
    task = scripts.complete_native_menu_choice("frag_grenades");
    if (!expect(is_service_call(task, "vid_phone.purchase_frag_grenades"),
                "Frag Grenade choice did not request its native transaction")) {
        std::cerr << scripts.failure() << '\n';
        return 32;
    }
    task = scripts.complete_native_service_result("purchased");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_purchase_result",
                "successful purchase did not show its ROM result prompt")) {
        return 33;
    }
    menu_profile = scripts.resolve_dialogue_menu(*task.operation->menu);
    if (!expect(menu_profile.has_value() &&
                    menu_profile->title_key ==
                        "vid_phone.capt_quinton_jaymes.purchase_result" &&
                    scripts.dialogue_text(menu_profile->title_key) != nullptr,
                "purchase result copy was not imported from the ROM")) {
        return 34;
    }
    task = scripts.complete_native_menu_choice("services");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_services",
                "purchase result Services choice did not return to the root")) {
        return 35;
    }

    task = scripts.complete_native_menu_choice("firepower");
    task = scripts.complete_native_menu_choice("more");
    task = scripts.complete_native_menu_choice("no_thanks");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_services",
                "declining firepower did not return to the Services menu")) {
        return 56;
    }
    task = scripts.complete_native_menu_choice("contacts");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_contacts_offer",
                "contacts service did not show the rigger offer")) {
        return 36;
    }
    task = scripts.complete_native_menu_choice("decline_rigger_contact");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_services",
                "declining the rigger offer did not return to Services")) {
        return 57;
    }
    task = scripts.complete_native_menu_choice("contacts");
    task = scripts.complete_native_menu_choice("pay_for_rigger_contact");
    if (!expect(is_service_call(task, "vid_phone.hire_sharkey"),
                "rigger payment choice did not request the native service")) {
        return 37;
    }
    task = scripts.complete_native_service_result("hired");
    if (!expect(is_menu(task) && task.operation->menu->id ==
                    "capt_quinton_sharkey_result",
                "successful rigger payment did not show the Sharkey result")) {
        return 38;
    }
    task = scripts.complete_native_menu_choice("goodbye");
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "Vid-Phone Goodbye did not finish the call activity")) {
        std::cerr << scripts.failure() << '\n';
        return 39;
    }

    task = scripts.start_task("field_activities", "vid_phone");
    task = scripts.complete_native_menu_input("confirm");
    task = scripts.complete_native_menu_choice("goodbye");
    if (!expect(task.status == ScriptTaskStep::Status::completed &&
                    (contact_flags & (1U << 13U)) != 0U,
                "repeat Captain Quinton call did not return cleanly")) {
        return 13;
    }

    task = scripts.start_task("field_activities", "vid_phone");
    task = scripts.complete_native_menu_input("confirm");
    task = scripts.complete_native_menu_choice("services");
    task = scripts.complete_native_menu_choice("firepower");
    task = scripts.complete_native_menu_choice("more");
    task = scripts.complete_native_menu_choice("hk227_s");
    if (!expect(is_service_call(task, "vid_phone.purchase_hk227_s"),
                "HK227-S option did not request its transaction")) {
        return 40;
    }
    task = scripts.complete_native_service_result("insufficient");
    if (!expect(is_say(task, "purchase_insufficient",
                       "vid_phone_capt_quinton_jaymes"),
                "insufficient credits did not show Quinton's ROM response")) {
        return 41;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "insufficient-funds response did not finish the phone call")) {
        return 42;
    }

    task = scripts.start_task("field_activities", "vid_phone");
    task = scripts.complete_native_menu_input("confirm");
    task = scripts.complete_native_menu_choice("services");
    task = scripts.complete_native_menu_choice("firepower");
    task = scripts.complete_native_menu_choice("more");
    task = scripts.complete_native_menu_choice("frag_grenades");
    task = scripts.complete_native_service_result("out_of_stock");
    if (!expect(is_say(task, "purchase_out_of_stock",
                       "vid_phone_capt_quinton_jaymes"),
                "full-stock response did not show Quinton's ROM text")) {
        return 43;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "out-of-stock response did not finish the phone call")) {
        return 44;
    }

    if (!test_alesandro_service_tree(scripts, contact_flags)) {
        std::cerr << scripts.failure() << '\n';
        return 58;
    }
    if (!test_sharkey_service_tree(scripts, contact_flags)) {
        std::cerr << scripts.failure() << '\n';
        return 59;
    }

    contact_flags = 0x00FFU;
    task = scripts.start_task("field_activities", "vid_phone");
    for (unsigned row = 0U; row < 7U; ++row) {
        task = scripts.complete_native_menu_input("down");
    }
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(is_cursor_menu(task) &&
                    task.operation->menu->choices.size() == 2U &&
                    task.operation->menu->choices.front().id ==
                        "mr_faradouchi" &&
                    task.operation->menu->choices.back().id == "prior_screen",
                "eight-contact page two did not retain its Prior Screen row")) {
        std::cerr << scripts.failure() << '\n';
        return 15;
    }
    task = scripts.complete_native_menu_input("cancel");

    contact_flags |= 1U << 20U;
    task = scripts.start_task("field_activities", "vid_phone");
    for (unsigned row = 0U; row < 7U; ++row) {
        task = scripts.complete_native_menu_input("down");
    }
    task = scripts.complete_native_menu_input("confirm");
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(is_say(task, "repeat_call", "vid_phone_mr_faradouchi"),
                "ROM call-history bit 20 did not select Faradouchi's repeat record")) {
        std::cerr << scripts.failure() << '\n';
        return 21;
    }
    task = scripts.complete_native_operation(true);

    const auto open_cyberspace_menu = [&scripts]() {
        auto current = scripts.start_task(
            "field_activities", "ordinary_field_terminal");
        current = scripts.complete_native_menu_input("down");
        current = scripts.complete_native_menu_input("down");
        return scripts.complete_native_menu_input("confirm");
    };
    auto cyberspace = open_cyberspace_menu();
    if (!expect(is_say(cyberspace, "cyberspace_no_deck", "field_narrator"),
                "Cyberspace without a deck did not show its ROM denial")) {
        return 22;
    }
    scripts.complete_native_operation(true);
    scripts.complete_native_menu_input("cancel");

    cyberdeck_present = true;
    // The no-MPCP message is the next prerequisite in ROM order.
    cyberspace = open_cyberspace_menu();
    if (!expect(is_say(cyberspace, "cyberspace_no_mpcp", "field_narrator"),
                "a deck with fried MPCP chips did not show its ROM denial")) {
        return 23;
    }
    scripts.complete_native_operation(true);
    scripts.complete_native_menu_input("cancel");

    mpcp_functional = true;
    cyberspace = open_cyberspace_menu();
    if (!expect(is_say(cyberspace, "cyberspace_no_datajack", "field_narrator"),
                "a deck without a datajack did not show its ROM denial")) {
        return 24;
    }
    scripts.complete_native_operation(true);
    scripts.complete_native_menu_input("cancel");

    datajack_present = true;
    matrix_passcode_flags = 0x0002U;
    cyberspace = open_cyberspace_menu();
    if (!expect(is_cursor_menu(cyberspace) &&
                    cyberspace.operation->menu->id == "cyberspace" &&
                    cyberspace.operation->menu->choices.size() == 3U,
                "the complete access gate did not open the Cyberspace choices")) {
        std::cerr << scripts.failure() << '\n';
        return 25;
    }
    cyberspace = scripts.complete_native_menu_input("confirm");
    if (!expect(is_cursor_menu(cyberspace) &&
                    cyberspace.operation->menu->id == "passcodes" &&
                    cyberspace.operation->menu->choices.size() == 1U,
                "Enter Passcode did not list known ROM passcodes")) {
        std::cerr << scripts.failure() << '\n';
        return 26;
    }
    cyberspace = scripts.complete_native_menu_input("confirm");
    if (!expect(cyberspace.status == ScriptTaskStep::Status::completed &&
                    cyberspace.result_id ==
                        std::optional<std::string>{"matrix_entry_7"},
                "Aztechnology passcode did not request its ROM system")) {
        std::cerr << scripts.failure() << '\n';
        return 27;
    }

    cyberspace = open_cyberspace_menu();
    cyberspace = scripts.complete_native_menu_input("down");
    cyberspace = scripts.complete_native_menu_input("confirm");
    if (!expect(cyberspace.status == ScriptTaskStep::Status::completed &&
                    cyberspace.result_id ==
                        std::optional<std::string>{
                            "matrix_system_search_requested"},
                "System Search did not return its route request")) {
        std::cerr << scripts.failure() << '\n';
        return 28;
    }
    return 0;
}
