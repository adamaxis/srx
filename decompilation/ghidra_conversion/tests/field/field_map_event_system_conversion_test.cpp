#include "shadowrun_ghidra/field/field_map_event_system_conversion.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    {
        const auto flag = field_map_event_flag(
            FieldMapEventStateFlag::new_game_opening_completed);
        FieldMapEventSystem system({{
            0x00020001U,
            2U,
            FieldMapEventTrigger::scene_entry,
            FieldMapEventCoordinateScope{0x3CE0U, 0x3CE0U,
                                         0x2D00U, 0x2D00U},
            flag,
            FieldMapEventRepeatPolicy::once_per_session,
            FieldMapEventScriptAction::start_new_game_opening_walk,
        }});
        auto activations = system.dispatch_scene_entry(
            {2U, 0x3CE0U, 0x2D00U, 0U, true});
        if (!expect(activations.empty(),
                    "opening walk must wait for the completed intro flag")) {
            return 1;
        }
        activations = system.dispatch_scene_entry(
            {1U, 0x3CE0U, 0x2D00U, flag, true});
        if (!expect(activations.empty(),
                    "opening walk must be scoped to scene 2")) {
            return 1;
        }
        activations = system.dispatch_scene_entry(
            {2U, 0x3CE1U, 0x2D00U, flag, true});
        if (!expect(activations.empty(),
                    "opening walk must match the source spawn coordinate")) {
            return 1;
        }
        activations = system.dispatch_scene_entry(
            {2U, 0x3CE0U, 0x2D00U, flag, true});
        if (!expect(activations.size() == 1U &&
                        activations[0].event_id == 0x00020001U &&
                        activations[0].action ==
                            FieldMapEventScriptAction::
                                start_new_game_opening_walk,
                    "completed New Game entry must start the one-shot walk")) {
            return 1;
        }
        activations = system.dispatch_scene_entry(
            {2U, 0x3CE0U, 0x2D00U, flag, true});
        if (!expect(activations.empty(),
                    "opening event must fire only once per session")) {
            return 1;
        }
    }

    {
        constexpr std::uint32_t kAreaEventId = 0x00090017U;
        constexpr std::uint32_t kHasKeyFlag = 1U << 7U;
        std::vector<FieldMapEventRule> rules{{
            kAreaEventId,
            9U,
            FieldMapEventTrigger::actor_within_scope,
            FieldMapEventCoordinateScope{0x1000U, 0x10FFU,
                                         0x2000U, 0x20FFU},
            kHasKeyFlag,
            FieldMapEventRepeatPolicy::once_per_scene_visit,
            FieldMapEventScriptAction::dispatch_native_script}};
        FieldMapEventSystem system(std::move(rules));
        system.begin_scene_visit(9U);

        auto activations = system.evaluate_actor_scope(
            {9U, 0x1080U, 0x2080U, 0U, true});
        if (!expect(activations.empty(),
                    "area event must respect required story state")) {
            return 1;
        }
        activations = system.evaluate_actor_scope(
            {9U, 0x1080U, 0x2080U, kHasKeyFlag, true});
        if (!expect(activations.size() == 1U &&
                        activations[0].event_id == kAreaEventId,
                    "area event must fire when actor and condition match")) {
            return 1;
        }
        activations = system.evaluate_actor_scope(
            {9U, 0x1080U, 0x2080U, kHasKeyFlag, true});
        if (!expect(activations.empty(),
                    "once-per-visit area event must not repeat in its scope")) {
            return 1;
        }
        system.begin_scene_visit(9U);
        activations = system.evaluate_actor_scope(
            {9U, 0x1080U, 0x2080U, kHasKeyFlag, true});
        if (!expect(activations.size() == 1U,
                    "once-per-visit event must re-arm on a new visit")) {
            return 1;
        }
    }

    {
        FieldMapEventSystem system;
        auto decision = system.dispatch_collision(
            {0x14U, 1U, 0U, 0U, 0x00C0U, 0x5800U, 0x0200U,
             0U, false, false, false});
        if (!expect(decision.sequence == 1U &&
                        decision.decision.event_code == 0U &&
                        decision.decision.action ==
                            FieldMapEventAction::project_world_interaction &&
                        system.collision_dispatch_count() == 1U &&
                        system.last_collision_dispatch().has_value() &&
                        system.last_collision_dispatch()->input.raw_collision_code ==
                            0x00C0U,
                    "collision events must enter and be recorded by the system")) {
            return 1;
        }

        decision = system.dispatch_collision(
            {0x14U, 1U, 0U, 0U, 0x0082U, 0x4718U, 0x1048U,
             0U, false, false, false});
        if (!expect(decision.decision.action ==
                        FieldMapEventAction::begin_maglock_interaction &&
                        decision.decision.event_code == 2U,
                    "scene-20 collision must select the maglock handler")) {
            return 1;
        }

        decision = system.dispatch_collision(
            {47U, 1U, 0U, 0U, 0x00B9U, 0x1000U, 0x1000U,
             0U, false, false, false});
        if (!expect(decision.decision.event_code == 8U &&
                        decision.decision.raw_collision_code == 0x00A8U &&
                        decision.decision.rewrite_direction_probe &&
                        decision.decision.rewrite_quantized_delta,
                    "context-free event 25 must preserve its event-8 rewrite")) {
            return 1;
        }
    }

    {
        FieldMapEventSystem system;
        std::array<std::uint8_t, 32> scene2_table{};
        scene2_table[0] = 0x14U;
        const auto scene2_door = system.resolve_script_route(
            {2U, 2U, 0U, true, scene2_table});
        if (!expect(scene2_door.kind == FieldScriptRouteKind::area_event &&
                        scene2_door.area_event_target ==
                            std::optional<std::uint8_t>{0x14U},
                    "scene-2 collision must resolve through its authored row")) {
            return 1;
        }
        const auto scene20_unmapped = system.resolve_script_route(
            {0x14U, 1U, 0U, false, {}});
        if (!expect(scene20_unmapped.kind ==
                        FieldScriptRouteKind::no_route,
                    "scene-20 event 0 must remain unresolved without a table")) {
            return 1;
        }
    }

    return 0;
}
