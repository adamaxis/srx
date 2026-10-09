#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

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

    const auto hotel_options =
        scripts.menu("hotel", "options", "hotel", "options");
    const auto hotel_checkout =
        scripts.menu("hotel", "checkout", "hotel", "checkout");
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
                    hotel_options->choices.size() == 2U &&
                    hotel_options->choices[0].id == "karma_screen" &&
                    hotel_options->choices[1].id == "check_out" &&
                    hotel_options->choices[0].label_key ==
                        "hotel.option.karma_screen" &&
                    hotel_options->choices[1].label_key ==
                        "hotel.option.check_out" &&
                    hotel_checkout.has_value() &&
                    hotel_checkout->choices.size() == 2U &&
                    hotel_checkout->choices[0].id == "checkout_yes" &&
                    hotel_checkout->choices[1].id == "checkout_no" &&
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

    const auto shop = scripts.shop_definition("ares_weapon_emporium");
    if (!expect(shop.has_value() && shop->groups.size() == 6U,
                "Lua shop catalog did not define the Ares selector groups") ||
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
                    manager_interaction.operation->menu->title_key ==
                        "stoker.motel.manager.title" &&
                    manager_interaction.operation->menu->choices.size() == 3U,
                "shared motel script did not yield its property-profile menu")) {
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
