#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <charconv>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

using namespace shadowrun::ghidra_conversion::scripting;

bool expect(const bool condition, const char* message) {
    if (!condition) std::cerr << message << '\n';
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

bool is_menu(const ScriptTaskStep& step, const std::string_view menu_id) {
    return step.status == ScriptTaskStep::Status::awaiting_native_operation &&
           step.operation.has_value() &&
           step.operation->type == ScriptNativeOperation::Type::ui_menu &&
           step.operation->menu.has_value() &&
           step.operation->menu->id == menu_id;
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

bool menu_resolves(GameScriptRuntime& scripts, const ScriptTaskStep& step,
                   const std::string_view expected_id,
                   const std::size_t expected_choices) {
    if (!expect(is_menu(step, expected_id),
                "Vid-Phone graph yielded an unexpected menu")) {
        return false;
    }
    const auto profile = scripts.resolve_dialogue_menu(*step.operation->menu);
    if (!expect(profile.has_value() &&
                    profile->choices.size() == expected_choices &&
                    scripts.dialogue_text(profile->title_key) != nullptr,
                "Vid-Phone graph menu title or choice count did not resolve")) {
        return false;
    }
    for (const auto& choice : profile->choices) {
        if (!expect(scripts.dialogue_text(choice.label_key) != nullptr,
                    "Vid-Phone graph menu has a missing ROM label")) {
            return false;
        }
    }
    return true;
}

ScriptTaskStep begin_only_contact(GameScriptRuntime& scripts) {
    auto task = scripts.start_task("field_activities", "vid_phone");
    if (task.status != ScriptTaskStep::Status::awaiting_native_operation ||
        !task.operation.has_value() ||
        task.operation->type != ScriptNativeOperation::Type::ui_menu ||
        !task.operation->menu.has_value() ||
        task.operation->menu->id != "contacts" ||
        task.operation->menu->choices.size() != 1U) {
        return task;
    }
    return scripts.complete_native_menu_input("confirm");
}

bool test_max_graph(GameScriptRuntime& scripts,
                    std::uint32_t& contact_flags) {
    constexpr std::uint32_t kMaxAvailability = 1U << 10U;
    constexpr std::uint32_t kMaxHistory = 1U << 23U;
    contact_flags = kMaxAvailability;

    auto task = begin_only_contact(scripts);
    if (!expect(menu_resolves(scripts, task, "call_max_first_call", 2U) &&
                    (contact_flags & kMaxHistory) != 0U,
                "Max first-call graph did not set its history or present copy")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("services");
    if (!menu_resolves(scripts, task, "max_services", 2U)) return false;
    task = scripts.complete_native_menu_choice("hardware");
    if (!menu_resolves(scripts, task, "max_hardware_intro", 1U)) return false;
    task = scripts.complete_native_menu_choice("more");
    if (!menu_resolves(scripts, task, "max_hardware_categories", 3U)) {
        return false;
    }
    task = scripts.complete_native_menu_choice("smgs");
    if (!menu_resolves(scripts, task, "max_smg_selector", 3U)) return false;
    task = scripts.complete_native_menu_choice("hk227");
    if (!expect(is_service_call(task, "vid_phone.purchase_max_hk227_s"),
                "Max HK227 choice did not request its typed transaction")) {
        return false;
    }
    task = scripts.complete_native_service_result("purchased");
    if (!menu_resolves(scripts, task, "max_purchase_result", 2U)) return false;
    task = scripts.complete_native_menu_choice("goodbye");
    if (!expect(is_say(task, "goodbye", "vid_phone_max"),
                "Max successful purchase did not offer the ROM farewell")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    return expect(task.status == ScriptTaskStep::Status::completed,
                  "Max call did not return from the Vid-Phone activity");
}

bool test_professor_graph(GameScriptRuntime& scripts,
                          std::uint32_t& contact_flags) {
    constexpr std::uint32_t kProfessorAvailability = 1U << 12U;
    constexpr std::uint32_t kProfessorHistory = 1U << 25U;
    contact_flags = kProfessorAvailability;

    auto task = begin_only_contact(scripts);
    if (!expect(menu_resolves(
                    scripts, task,
                    "call_professor_jefferson_first_call", 2U) &&
                    (contact_flags & kProfessorHistory) != 0U,
                "Professor first-call graph did not set history or present copy")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("services");
    if (!menu_resolves(scripts, task, "professor_services", 3U)) return false;
    task = scripts.complete_native_menu_choice("paranormal_creatures");
    for (const auto menu_id : { "professor_creature_page_1",
                                "professor_creature_page_2" }) {
        if (!menu_resolves(scripts, task, menu_id, 1U)) return false;
        task = scripts.complete_native_menu_choice("more");
    }
    if (!menu_resolves(scripts, task, "professor_creature_page_3", 2U)) {
        return false;
    }
    task = scripts.complete_native_menu_choice("goodbye");
    if (!expect(is_say(task, "goodbye", "vid_phone_professor_jefferson"),
                "Professor lesson did not offer Services/Goodbye at its end")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "Professor lesson call did not return to the phonebook")) {
        return false;
    }

    contact_flags = kProfessorAvailability | kProfessorHistory;
    task = begin_only_contact(scripts);
    if (!menu_resolves(
            scripts, task,
            "call_professor_jefferson_repeat_call", 2U)) {
        return false;
    }
    task = scripts.complete_native_menu_choice("services");
    if (!menu_resolves(scripts, task, "professor_services", 3U)) return false;
    task = scripts.complete_native_menu_choice("club");
    if (!menu_resolves(scripts, task, "professor_club_information", 1U)) {
        return false;
    }
    task = scripts.complete_native_menu_choice("more");
    if (!menu_resolves(scripts, task, "professor_club_offer", 3U)) return false;
    task = scripts.complete_native_menu_choice("accept");
    if (!expect(is_service_call(task, "vid_phone.purchase_riannons_discount"),
                "Professor Club offer did not request command 0x88")) {
        return false;
    }
    task = scripts.complete_native_service_result("discount_enabled");
    if (!expect(is_say(task, "club_success",
                       "vid_phone_professor_jefferson"),
                "Professor command 0x88 did not present its success page")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.status == ScriptTaskStep::Status::completed,
                "Professor club success page did not finish the call")) {
        return false;
    }

    task = begin_only_contact(scripts);
    if (!menu_resolves(
            scripts, task,
            "call_professor_jefferson_repeat_call", 2U)) {
        return false;
    }
    task = scripts.complete_native_menu_choice("services");
    task = scripts.complete_native_menu_choice("former_student");
    if (!menu_resolves(scripts, task, "professor_student_offer", 3U)) {
        return false;
    }
    task = scripts.complete_native_menu_choice("accept");
    if (!expect(is_service_call(task, "vid_phone.hire_gregory_wilns"),
                "Professor former-student offer did not request command 0x89")) {
        return false;
    }
    task = scripts.complete_native_service_result("hired");
    if (!menu_resolves(scripts, task, "professor_student_result", 2U)) {
        return false;
    }
    task = scripts.complete_native_menu_choice("goodbye");
    if (!expect(is_say(task, "goodbye", "vid_phone_professor_jefferson"),
                "Professor student result did not show the ROM farewell")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    return expect(task.status == ScriptTaskStep::Status::completed,
                  "Professor student result did not return to the phonebook");
}

} // namespace

int main() {
    GameScriptRuntime scripts;
    if (!expect(scripts.load_package(
                    SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
                    std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                        "dialogue" / "script-text.json"),
                "Vid-Phone graph package did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    std::uint32_t contact_flags = 0U;
    scripts.set_story_fact_accessors(
        [&contact_flags](const std::string_view name) -> std::optional<bool> {
            if (name == "vid_phone_contacts_resolved") return true;
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
            if (!index.has_value()) return false;
            const auto mask = static_cast<std::uint32_t>(
                1U << (*index + 13U));
            contact_flags = value ? contact_flags | mask
                                  : contact_flags & ~mask;
            return true;
        });
    scripts.set_location_context(
        "field_map", "scene_0", "field.main_menu", "");

    if (!test_max_graph(scripts, contact_flags)) {
        std::cerr << scripts.failure() << '\n';
        return 2;
    }
    if (!test_professor_graph(scripts, contact_flags)) {
        std::cerr << scripts.failure() << '\n';
        return 3;
    }
    std::cout << "Max and Professor Jefferson JSON graph host integration passed\n";
    return 0;
}
