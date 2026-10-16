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
    scripts.set_story_fact_accessors(
        [&contact_flags](const std::string_view name)
            -> std::optional<bool> {
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
    for (unsigned row = 0U; row < 4U; ++row) {
        task = scripts.complete_native_menu_input("down");
    }
    task = scripts.complete_native_menu_input("confirm");
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
    task = scripts.complete_native_menu_choice("goodbye");
    task = scripts.complete_native_menu_input("confirm");
    if (!expect(is_menu(task) &&
                    task.operation->menu->id ==
                        "call_capt_quinton_jaymes_repeat_call" &&
                    (contact_flags & (1U << 13U)) != 0U,
                "repeat Captain Quinton call did not select repeat dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    task = scripts.complete_native_menu_choice("goodbye");
    task = scripts.complete_native_menu_input("cancel");
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "Vid-Phone menu did not return cleanly to the field")) {
        std::cerr << scripts.failure() << '\n';
        return 14;
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
    return 0;
}
