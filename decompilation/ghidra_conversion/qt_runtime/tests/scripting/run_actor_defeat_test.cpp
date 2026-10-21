#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"
#include "enums.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

namespace {

using shadowrun::ghidra_conversion::scripting::GameScriptRuntime;
using shadowrun::ghidra_conversion::scripting::ScriptActorDefeatFacts;
using shadowrun::ghidra_conversion::scripting::ScriptNativeOperation;
using shadowrun::ghidra_conversion::scripting::ScriptRunActorDefeatAction;
using shadowrun::ghidra_conversion::scripting::ScriptRunJohnsonReturnAction;
using shadowrun::ghidra_conversion::scripting::ScriptTaskStep;

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

} // namespace

int main() {
    GameScriptRuntime scripts;
    const auto script_root =
        std::filesystem::path(SHADOWRUN_GAMEPLAY_SCRIPT_ROOT);
    const auto dialogue_text =
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
        "dialogue" / "script-text.json";
    if (!expect(scripts.load_package(script_root, dialogue_text),
                "gameplay Lua package did not load")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }
    scripts.set_location_context(
        "building", "jump_house", "gunderson.jump_house", "gunderson");

    auto task = scripts.start_task("run_registry", "prepare_new_game_offer");
    if (!expect(task.operation.has_value() &&
                    task.operation->service_id == "random.bounded_8",
                "ghoul bounty did not start from the ROM offer pool")) {
        return 2;
    }
    task = scripts.complete_native_service_result("roll_0");
    if (!expect(task.operation.has_value() &&
                    task.operation->service_id == "runner.negotiation_skill",
                "ghoul bounty skipped the source Negotiation lookup")) {
        return 3;
    }
    task = scripts.complete_native_service_result("skill_3");
    if (!expect(task.operation.has_value() &&
                    task.operation->service_id == "random.bounded_10",
                "ghoul bounty skipped its source negotiation roll")) {
        return 4;
    }
    task = scripts.complete_native_service_result("roll_2");
    if (!expect(task.status == ScriptTaskStep::Status::completed &&
                    task.result_id ==
                        std::optional<std::string>{"ghoul_bounty"},
                "ghoul bounty was not selected for the test offer")) {
        return 5;
    }

    task = scripts.start_task("run_ghoul_bounty", "present_offer");
    if (!expect(task.operation.has_value() &&
                    task.operation->type == ScriptNativeOperation::Type::ui_say,
                "ghoul bounty did not present its offer text")) {
        return 6;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.operation.has_value() &&
                    task.operation->type == ScriptNativeOperation::Type::ui_menu,
                "ghoul bounty did not open its acceptance prompt")) {
        return 7;
    }
    task = scripts.complete_native_menu_choice("yes");
    if (!expect(task.operation.has_value() &&
                    task.operation->type == ScriptNativeOperation::Type::ui_say,
                "accepted ghoul bounty skipped its confirmation text")) {
        return 8;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.status == ScriptTaskStep::Status::completed &&
                    task.result_id == std::optional<std::string>{"accepted"},
                "ghoul bounty did not become the active run")) {
        return 9;
    }

    constexpr auto ghoul_race_or_state = static_cast<std::int8_t>(
        srx::v(srx::ActorRaceOrState::GHOUL_BOUNTY_TARGET));
    constexpr auto other_race_or_state = static_cast<std::int8_t>(
        srx::v(srx::ActorRaceOrState::GHOUL_BOUNTY_TARGET) - 1);
    const auto no_kills = scripts.resolve_active_run_johnson_return(
        static_cast<std::uint8_t>(srx::v(srx::RunOfferContext::JUMP_HOUSE)));
    if (!expect(no_kills.has_value() &&
                    no_kills->action ==
                        ScriptRunJohnsonReturnAction::no_action &&
                    scripts.call_string(
                        "run_registry", "active_run_id") ==
                        std::optional<std::string>{"ghoul_bounty"},
                "the bounty settled without an eligible defeat")) {
        std::cerr << scripts.failure() << '\n';
        return 10;
    }
    const auto other = scripts.resolve_active_run_actor_defeat(
        ScriptActorDefeatFacts{other_race_or_state});
    const auto first_ghoul = scripts.resolve_active_run_actor_defeat(
        ScriptActorDefeatFacts{ghoul_race_or_state});
    if (!expect(other.has_value() &&
                    other->action == ScriptRunActorDefeatAction::no_action &&
                    other->objective_progress == 0U &&
                    first_ghoul.has_value() &&
                    first_ghoul->action ==
                        ScriptRunActorDefeatAction::objective_advanced &&
                    first_ghoul->objective_progress == 1U,
                "the bounty counted a non-ghoul or missed the first ghoul")) {
        return 10;
    }

    const auto saved = scripts.save_run_registry_state();
    const auto second_ghoul = scripts.resolve_active_run_actor_defeat(
        ScriptActorDefeatFacts{ghoul_race_or_state});
    if (!expect(saved.has_value() &&
                    scripts.validate_run_registry_state(*saved) &&
                    second_ghoul.has_value() &&
                    second_ghoul->objective_progress == 2U &&
                    scripts.restore_run_registry_state(*saved),
                "ghoul objective progress did not survive save/restore")) {
        std::cerr << scripts.failure() << '\n';
        return 11;
    }

    const auto max_defeats = static_cast<std::uint8_t>(
        srx::v(srx::GhoulBounty::MAX_DEFEATS));
    for (std::uint8_t count = 2U; count <= max_defeats; ++count) {
        const auto decision = scripts.resolve_active_run_actor_defeat(
            ScriptActorDefeatFacts{ghoul_race_or_state});
        if (!expect(decision.has_value() &&
                        decision->action ==
                            ScriptRunActorDefeatAction::objective_advanced &&
                        decision->objective_progress == count,
                    "ghoul bounty count did not advance to its ROM cap")) {
            return 12;
        }
    }
    const auto beyond_cap = scripts.resolve_active_run_actor_defeat(
        ScriptActorDefeatFacts{ghoul_race_or_state});
    if (!expect(beyond_cap.has_value() &&
                    beyond_cap->action == ScriptRunActorDefeatAction::no_action &&
                    beyond_cap->objective_progress == max_defeats,
                "ghoul bounty counted beyond the ROM's twenty-defeat cap")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }

    const auto wrong_johnson = scripts.resolve_active_run_johnson_return(
        static_cast<std::uint8_t>(srx::v(srx::RunOfferContext::WANDERER)));
    if (!expect(wrong_johnson.has_value() &&
                    wrong_johnson->action ==
                        ScriptRunJohnsonReturnAction::no_action &&
                    wrong_johnson->reward_nuyen == 0U &&
                    scripts.call_string(
                        "run_registry", "active_run_id") ==
                        std::optional<std::string>{"ghoul_bounty"},
                "a nonmatching Johnson context settled the bounty")) {
        std::cerr << scripts.failure() << '\n';
        return 14;
    }

    const auto completed = scripts.resolve_active_run_johnson_return(
        static_cast<std::uint8_t>(srx::v(srx::RunOfferContext::JUMP_HOUSE)));
    if (!expect(completed.has_value() &&
                    completed->action ==
                        ScriptRunJohnsonReturnAction::completed &&
                    completed->run_id == "ghoul_bounty" &&
                    completed->objective_progress == max_defeats &&
                    completed->reward_nuyen == 400U &&
                    completed->runner_progress_delta == 1U &&
                    scripts.call_string(
                        "run_registry", "active_run_id") ==
                        std::optional<std::string>{"none"},
                "matching Johnson return did not settle the bounty once")) {
        std::cerr << scripts.failure() << '\n';
        return 15;
    }

    auto return_dialogue = scripts.start_task("johnson", "interaction");
    if (!expect(return_dialogue.operation.has_value() &&
                    return_dialogue.operation->type ==
                        ScriptNativeOperation::Type::ui_say &&
                    return_dialogue.operation->dialogue.has_value() &&
                    return_dialogue.operation->dialogue->inline_text.find(
                        "Looks like you made it through that run.") !=
                        std::string::npos,
                "Johnson return skipped the source result dialogue")) {
        std::cerr << scripts.failure() << '\n';
        return 16;
    }
    return_dialogue = scripts.complete_native_operation(true);
    const auto return_menu = return_dialogue.operation.has_value() &&
                                     return_dialogue.operation->menu.has_value()
                                 ? scripts.resolve_dialogue_menu(
                                       *return_dialogue.operation->menu)
                                 : std::nullopt;
    if (!expect(return_dialogue.operation.has_value() &&
                    return_dialogue.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    return_menu.has_value() &&
                    return_menu->title_key ==
                        "gunderson.greeting.after_run_return",
                "Johnson return did not transition to the follow-up menu")) {
        std::cerr << scripts.failure() << '\n';
        std::cerr << "status=" << static_cast<int>(return_dialogue.status);
        if (return_dialogue.operation.has_value()) {
            std::cerr << " operation="
                      << static_cast<int>(return_dialogue.operation->type);
            if (return_dialogue.operation->menu.has_value()) {
                std::cerr << " menu=" << return_dialogue.operation->menu->id
                          << " title="
                          << return_dialogue.operation->menu->title_key;
            }
            if (return_dialogue.operation->dialogue.has_value()) {
                std::cerr << " event="
                          << return_dialogue.operation->dialogue->event_id
                          << " text_profile="
                          << return_dialogue.operation->dialogue->text_profile;
            }
        }
        std::cerr << '\n';
        return 17;
    }
    return_dialogue = scripts.complete_native_menu_choice("leave");
    if (!expect(return_dialogue.status == ScriptTaskStep::Status::completed &&
                    return_dialogue.result_id ==
                        std::optional<std::string>{"leave"},
                "Johnson return follow-up did not return to the field")) {
        std::cerr << scripts.failure() << '\n';
        return 18;
    }

    const auto no_repeat_dialogue =
        scripts.start_task("johnson", "interaction");
    const auto normal_menu = no_repeat_dialogue.operation.has_value() &&
                                     no_repeat_dialogue.operation->menu.has_value()
                                 ? scripts.resolve_dialogue_menu(
                                       *no_repeat_dialogue.operation->menu)
                                 : std::nullopt;
    if (!expect(no_repeat_dialogue.operation.has_value() &&
                    no_repeat_dialogue.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    normal_menu.has_value() &&
                    normal_menu->title_key == "gunderson.greeting",
                "completed Johnson-return dialogue replayed on re-entry")) {
        std::cerr << scripts.failure() << '\n';
        return 19;
    }

    std::cout << "ghoul bounty actor defeat and Johnson-return test passed\n";
    return 0;
}
