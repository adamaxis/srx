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
                        FieldMapEventScriptAction::start_new_game_opening_walk,
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
        return 4;
    }
    if (!expect(scripts.story_state().at("opening_dialogue_seen"),
                "Lua could not set a story flag through game.state")) {
        return 4;
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
        return 5;
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
        return 6;
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
        return 7;
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
        return 8;
    }
    scripts.set_story_state(repeat_visit);
    scripts.set_location_context("motel", "downtown_motel");
    const auto ordinary_motel_manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(ordinary_motel_manager.has_value() &&
                    ordinary_motel_manager->choices.size() == 2U,
                "Michael quest hook leaked into another motel")) {
        return 9;
    }
    scripts.set_location_context("motel", "stoker_motel");
    const auto package_manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(package_manager.has_value() &&
                    package_manager->choices.size() == 3U &&
                    package_manager->choices[2].id == "package",
                "unpaid manager menu lacks the package choice")) {
        std::cerr << scripts.failure() << '\n';
        return 9;
    }
    scripts.set_story_state(paid_but_not_collected);
    const auto paid_package_manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(paid_package_manager.has_value() &&
                    paid_package_manager->choices.size() == 3U,
                "debt payment prematurely removed the uncollected package")) {
        return 10;
    }
    scripts.set_story_state(completed);
    const auto completed_package_manager =
        scripts.menu("motel", "manager_choices", "stoker.motel");
    if (!expect(completed_package_manager.has_value() &&
                    completed_package_manager->choices.size() == 2U,
                "completed package facts did not remove the special choice")) {
        return 11;
    }
    return 0;
}
