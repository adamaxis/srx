#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <iostream>

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
    if (!expect(scripts.load_package(SHADOWRUN_GAMEPLAY_SCRIPT_ROOT),
                "gameplay Lua package did not load")) {
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
        "motel", "stoker_motel", "stoker.motel");
    const auto opening_interaction =
        scripts.start_task("michael_package", "opening_interaction");
    if (!expect(opening_interaction.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    opening_interaction.operation.has_value() &&
                    opening_interaction.operation->type ==
                        ScriptNativeOperation::Type::ui_say &&
                    opening_interaction.operation->dialogue.has_value() &&
                    opening_interaction.operation->dialogue->speaker_id ==
                        "stoker" &&
                    opening_interaction.operation->dialogue->text_profile ==
                        "stoker.motel" &&
                    opening_interaction.operation->dialogue->event_id ==
                        "arrival",
                "opening interaction did not yield its speaker/event dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    auto opening_continuation = scripts.complete_native_operation(true);
    if (!expect(opening_continuation.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    opening_continuation.operation.has_value() &&
                    opening_continuation.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    opening_continuation.operation->menu.has_value() &&
                    opening_continuation.operation->menu->choices.size() == 3U,
                "opening dialogue did not continue into the motel menu")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    opening_continuation = scripts.complete_native_menu_choice("package");
    if (!expect(opening_continuation.status ==
                        ScriptTaskStep::Status::completed &&
                    opening_continuation.result_id ==
                        std::optional<std::string>{"package"},
                "opening interaction did not finish with the chosen ID")) {
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
    return 0;
}
