#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

bool verify_dynamic_run_module(const std::filesystem::path& source_directory,
                               const std::filesystem::path& dialogue_text) {
    using namespace shadowrun::ghidra_conversion::scripting;

    std::error_code error;
    const auto unique_suffix = std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count();
    const auto temporary_directory =
        std::filesystem::temp_directory_path(error) /
        ("shadowrun_dynamic_run_" + std::to_string(unique_suffix));
    if (error || !std::filesystem::create_directories(
                     temporary_directory, error) || error) {
        return expect(false, "could not create temporary Lua run package");
    }
    struct RemoveTemporaryDirectory {
        std::filesystem::path path;
        ~RemoveTemporaryDirectory() {
            std::error_code cleanup_error;
            std::filesystem::remove_all(path, cleanup_error);
        }
    } cleanup{temporary_directory};

    for (std::filesystem::recursive_directory_iterator iterator(
             source_directory, error),
         end;
         !error && iterator != end; iterator.increment(error)) {
        if (iterator->is_regular_file(error) &&
            iterator->path().extension() == ".lua") {
            const auto relative_path =
                iterator->path().lexically_relative(source_directory);
            const auto destination = temporary_directory / relative_path;
            std::filesystem::create_directories(destination.parent_path(),
                                                error);
            if (error) {
                return expect(false,
                              "could not create nested Lua module directory");
            }
            std::filesystem::copy_file(
                iterator->path(), destination,
                std::filesystem::copy_options::overwrite_existing, error);
            if (error) {
                return expect(false,
                              "could not copy Lua modules to temporary package");
            }
        }
    }
    if (error) {
        return expect(false, "could not enumerate Lua modules for plugin test");
    }

    std::filesystem::create_directories(temporary_directory / "data", error);
    if (error) {
        return expect(false, "could not create nested Lua probe module");
    }
    {
        std::ofstream module(temporary_directory / "data" / "probe.lua",
                            std::ios::binary);
        module << "local E = require('enums')\n"
                  "local Probe = {}\n"
                  "Probe.__index = Probe\n"
                  "setmetatable(Probe, { __call = function(_, value)\n"
                  "  return setmetatable({ value = value, posture = E.Posture.NEUTRAL }, Probe)\n"
                  "end })\n"
                  "local M = {}\n"
                  "function M.verify()\n"
                  "  local instance = Probe(13)\n"
                  "  return require('data.probe') == M and instance.value == 13 and\n"
                  "         instance.posture == E.Posture.NEUTRAL\n"
                  "end\n"
                  "return M\n";
        if (!module) {
            return expect(false, "could not write nested Lua probe module");
        }
    }
    {
        std::ofstream module(temporary_directory / "data" / "cycle.lua",
                             std::ios::binary);
        module << "return require('data.cycle')\n";
        if (!module) {
            return expect(false, "could not write circular Lua probe module");
        }
    }

    {
        std::ofstream plugin(temporary_directory / "run_dynamic_probe.lua",
                             std::ios::binary);
        plugin << "local Probe = { new_game_weight = 1,\n"
                  "  new_game_payment = { base_payment = 40, negotiation_scale = 1 } }\n"
                  "function Probe.prepare_offer(context)\n"
                  "  local offer = game.modules.run_support.new_offer(\n"
                  "    'dynamic_probe', 'test', 'probe', 'probe_complete')\n"
                  "  offer.context = context\n"
                  "  game.runs.registry.current_offer = offer\n"
                  "  return offer.id\n"
                  "end\n"
                  "function Probe.present_offer() return 'probe_presented' end\n"
                  "function Probe.module_probe()\n"
                  "  local module_ok = require('data.probe').verify()\n"
                  "  local items = require('items')\n"
                  "  local records = require('data.items').by_id\n"
                  "  local weapons = require('data.weapons')\n"
                  "  local spells = require('data.spells').by_type\n"
                  "  local accessories = require('data.accessories').by_id\n"
                  "  local profiles = require('data.actor_profiles').by_id\n"
                  "  local magic = require('magic')\n"
                  "  for code = 1, 50 do\n"
                  "    local item = items.by_code[code]\n"
                  "    if item == nil or item.source_record ~= records[code] or\n"
                  "       item.pricing.purchase_base ~= records[code].price then return 'item catalog mismatch' end\n"
                  "    if code <= 15 then\n"
                  "      local template = weapons.get(code)\n"
                  "      if template == nil or item.source_weapon_template ~= template or\n"
                  "         template.clip_size ~= records[code].b24 or\n"
                  "         template.damage_class ~= records[code].b25 or\n"
                  "         template.power ~= records[code].b26 or\n"
                  "         template.band ~= records[code].b27 then return 'weapon ROM fields mismatch' end\n"
                  "      if code <= 12 and (item.weapon == nil or\n"
                  "         item.weapon.template ~= template or\n"
                  "         item.weapon.magazine_capacity ~= template.clip_size or\n"
                  "         item.weapon.damage_bar_index ~= template.damage_class or\n"
                  "         item.weapon.rounds_per_action ~= template.rounds_per_shot) then return 'firearm host view mismatch' end\n"
                  "    end\n"
                  "    if item.armor ~= nil and\n"
                  "       (item.armor.primary_modifier ~= records[code].b25 or\n"
                  "        item.armor.secondary_modifier ~= records[code].b26) then return 'armor ROM fields mismatch' end\n"
                  "  end\n"
                  "  for code = 1, 14 do\n"
                  "    local definition = magic.spell_action_definition(code)\n"
                  "    if definition == nil or definition.source_record ~= spells[code - 1] then return 'spell record mismatch' end\n"
                  "    if code == 14 and (definition.spell_id ~= 'barrier' or definition.availability ~= -1 or definition.effect_table_index ~= 1 or definition.difficulty_modifier ~= 2 or definition.base_effect ~= 3 or definition.target_effect ~= 0 or definition.attribute_selector ~= 0) then return 'Barrier ROM action profile mismatch' end\n"
                  "  end\n"
                  "  for id = 0, 5 do\n"
                  "    local accessory = accessories[id]\n"
                  "    if accessory == nil or accessory.id ~= id or\n"
                  "       type(accessory:base_price(0)) ~= 'number' or\n"
                  "       type(accessory:base_price(1)) ~= 'number' or\n"
                  "       type(accessory:base_price(2)) ~= 'number' then return 'accessory price record mismatch' end\n"
                  "  end\n"
                  "  if profiles['corp_guard_k05_item06'] == nil or\n"
                  "     profiles['corp_guard_k05_item06']:weapon_record() ~= weapons.get(6) then return 'actor weapon profile mismatch' end\n"
                  "  local safe = pcall(require, '..')\n"
                  "  local cycle = pcall(require, 'data.cycle')\n"
                  "  return module_ok and not safe and not cycle and 'module_catalog_ok' or 'module_probe_failed'\n"
                  "end\n"
                  "return Probe\n";
        if (!plugin) {
            return expect(false, "could not write temporary Lua run module");
        }
    }

    GameScriptRuntime extensible_scripts;
    if (!extensible_scripts.load_package(temporary_directory,
                                        dialogue_text)) {
        std::cerr << extensible_scripts.failure() << '\n';
        return expect(false, "temporary Lua run package did not load");
    }
    const auto run_ids =
        extensible_scripts.string_list("run_registry", "available_run_ids");
    const std::vector<std::string> expected_run_ids{
        "bodyguard_escort", "dynamic_probe", "employee_recruitment",
        "gang_contract", "ghoul_bounty", "matrix_objective",
        "package_delivery", "package_recovery"};
    if (!expect(run_ids.has_value() && *run_ids == expected_run_ids,
                "new run module was not discovered from the Lua package")) {
        return false;
    }

    auto selection = extensible_scripts.start_task(
        "run_registry", "prepare_new_game_offer");
    if (!expect(selection.status ==
                    ScriptTaskStep::Status::awaiting_native_operation &&
                    selection.operation.has_value() &&
                    selection.operation->service_id == "random.bounded_9",
                "Lua extension weight was not added to the offer pool")) {
        return false;
    }
    selection = extensible_scripts.complete_native_service_result("roll_8");
    const auto module_probe = extensible_scripts.call_string(
        "run_dynamic_probe", "module_probe");
    if (!module_probe.has_value() ||
        *module_probe != "module_catalog_ok") {
        std::cerr << "module probe result: "
                  << module_probe.value_or("<no string result>") << "; "
                  << extensible_scripts.failure() << '\n';
    }
    return expect(selection.status == ScriptTaskStep::Status::completed &&
                      selection.result_id ==
                          std::optional<std::string>{"dynamic_probe"} &&
                      extensible_scripts.call_string(
                          "run_registry", "current_offer_id") ==
                          std::optional<std::string>{"dynamic_probe"} &&
                      extensible_scripts.call_string(
                          "run_dynamic_probe", "present_offer") ==
                          std::optional<std::string>{"probe_presented"} &&
                      module_probe == std::optional<std::string>{
                                          "module_catalog_ok"},
                  "new Lua run was not selected and dispatched independently");
}

bool verify_package_delivery_field_events(
    const std::filesystem::path& script_directory,
    const std::filesystem::path& dialogue_text) {
    using namespace shadowrun::ghidra_conversion::scripting;

    GameScriptRuntime scripts;
    if (!scripts.load_package(script_directory, dialogue_text)) {
        std::cerr << scripts.failure() << '\n';
        return expect(false, "run event Lua package did not load");
    }
    scripts.set_location_context(
        "building", "jump_house", "gunderson.jump_house", "gunderson");

    auto task = scripts.start_task("run_registry", "prepare_new_game_offer");
    if (!expect(task.operation.has_value() &&
                    task.operation->service_id == "random.bounded_8",
                "package event fixture did not start from the source offer pool")) {
        return false;
    }
    task = scripts.complete_native_service_result("roll_2");
    if (!expect(task.operation.has_value() &&
                    task.operation->service_id == "runner.negotiation_skill",
                "package offer skipped the source Negotiation lookup")) {
        return false;
    }
    task = scripts.complete_native_service_result("skill_3");
    task = scripts.complete_native_service_result("roll_2");
    if (!expect(task.operation.has_value() &&
                    task.operation->service_id == "random.bounded_10",
                "package offer skipped the source origin-location roll")) {
        return false;
    }
    task = scripts.complete_native_service_result("roll_4");
    if (!expect(task.operation.has_value() &&
                    task.operation->service_id == "random.bounded_12",
                "package offer skipped the source client roll")) {
        return false;
    }
    task = scripts.complete_native_service_result("roll_0");
    if (!expect(task.operation.has_value() &&
                    task.operation->service_id == "random.bounded_10",
                "package offer skipped the source destination-location roll")) {
        return false;
    }
    task = scripts.complete_native_service_result("roll_5");
    const auto route = scripts.string_map(
        "run_registry", "current_route_ids");
    const auto payment = scripts.string_map(
        "run_registry", "current_payment_values");
    if (!expect(task.status == ScriptTaskStep::Status::completed &&
                    task.result_id ==
                        std::optional<std::string>{"package_delivery"} &&
                    route.has_value() &&
                    route->at("origin_area") == "area_2" &&
                    route->at("origin_location") == "location_4" &&
                    route->at("destination_area") == "area_2" &&
                    route->at("destination_location") == "location_5" &&
                    payment.has_value() &&
                    payment->at("negotiated_amount") == "45" &&
                    payment->at("runner_progress_delta") == "1",
                "package event fixture does not match its selected ROM offer")) {
        return false;
    }

    task = scripts.start_task("run_package_delivery", "present_offer");
    if (!expect(task.operation.has_value() &&
                    task.operation->type == ScriptNativeOperation::Type::ui_say,
                "package offer did not present its source-derived description")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.operation.has_value() &&
                    task.operation->type == ScriptNativeOperation::Type::ui_menu,
                "package offer did not open the acceptance prompt")) {
        return false;
    }
    task = scripts.complete_native_menu_choice("yes");
    if (!expect(task.operation.has_value() &&
                    task.operation->type == ScriptNativeOperation::Type::ui_say,
                "accepted package offer did not run its confirmation dialogue")) {
        return false;
    }
    task = scripts.complete_native_operation(true);
    if (!expect(task.status == ScriptTaskStep::Status::completed &&
                    task.result_id == std::optional<std::string>{"accepted"},
                "package offer did not become an active run after acceptance")) {
        return false;
    }

    const auto before_origin = scripts.resolve_active_run_field_event(2U, 5U);
    const auto origin = scripts.resolve_active_run_field_event(2U, 4U);
    const auto between = scripts.resolve_active_run_field_event(2U, 6U);
    const auto destination = scripts.resolve_active_run_field_event(2U, 5U);
    const auto after_completion = scripts.resolve_active_run_field_event(2U, 5U);
    return expect(
        before_origin.has_value() &&
            before_origin->action == ScriptRunFieldEventAction::no_action &&
            origin.has_value() &&
            origin->action ==
                ScriptRunFieldEventAction::objective_advanced &&
            origin->run_id == "package_delivery" && between.has_value() &&
            between->action == ScriptRunFieldEventAction::no_action &&
            destination.has_value() &&
            destination->action == ScriptRunFieldEventAction::completed &&
            destination->run_id == "package_delivery" &&
            destination->reward_nuyen == 45U &&
            destination->runner_progress_delta == 1U &&
            after_completion.has_value() &&
            after_completion->action ==
                ScriptRunFieldEventAction::no_action &&
            scripts.call_string("run_registry", "active_run_id") ==
                std::optional<std::string>{"none"} &&
            scripts.call_string(
                "run_registry", "last_completed_offer_id") ==
                std::optional<std::string>{"package_delivery"},
        "package run did not collect at origin, pay once at destination, and clear");
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

    const auto redmond_creature_sequence =
        scripts.resolve_field_encounter(0U, 0U, 0U);
    const auto barrens_creature_sequence =
        scripts.resolve_field_encounter(2U, 1U, 1U);
    const auto lone_star_sequence =
        scripts.resolve_field_encounter(1U, 1U, 1U);
    const auto elven_mage_sequence =
        scripts.resolve_field_encounter(3U, 0U, 0U);
    if (!redmond_creature_sequence.has_value() ||
        !barrens_creature_sequence.has_value() ||
        !lone_star_sequence.has_value() ||
        !elven_mage_sequence.has_value()) {
        std::cerr << "encounter lookup presence: redmond="
                  << redmond_creature_sequence.has_value()
                  << " barrens=" << barrens_creature_sequence.has_value()
                  << " lone_star=" << lone_star_sequence.has_value()
                  << " elven_mage=" << elven_mage_sequence.has_value()
                  << " detail=" << scripts.failure() << '\n';
        return 1;
    }
    if (!expect(
            redmond_creature_sequence.has_value() &&
                redmond_creature_sequence->kind ==
                    ScriptFieldEncounterOutcomeKind::
                        dynamic_world_effect_sequence &&
                redmond_creature_sequence->scene_id == 0U &&
                redmond_creature_sequence->random_sum == 0U &&
                redmond_creature_sequence->actors.size() == 2U &&
                redmond_creature_sequence->actors[0].actor_name ==
                    "Hell Hound" &&
                redmond_creature_sequence->actors[0].entity_kind == 12U &&
                redmond_creature_sequence->actors[0].physical_meter == 10U &&
                redmond_creature_sequence->actors[1].actor_name == "vampire" &&
                redmond_creature_sequence->actors[1].entity_kind == 9U &&
                redmond_creature_sequence->actors[1].physical_meter == 10U &&
                barrens_creature_sequence.has_value() &&
                barrens_creature_sequence->actors.size() == 1U &&
                barrens_creature_sequence->actors[0].actor_name == "weenr1" &&
                barrens_creature_sequence->actors[0].physical_meter == 10U &&
                lone_star_sequence.has_value() &&
                lone_star_sequence->actors.size() == 2U &&
                lone_star_sequence->actors[0].actor_name == "lone11" &&
                lone_star_sequence->actors[0].physical_meter == 10U &&
                lone_star_sequence->actors[1].actor_name == "lonemage" &&
                lone_star_sequence->actors[1].action_kind ==
                    ScriptActorActionKind::spell &&
                lone_star_sequence->actors[1].physical_meter == 10U &&
                lone_star_sequence->actors[1].combatant
                        .command_attribute_77 == 3U &&
                lone_star_sequence->actors[1].combatant
                        .command_attribute_78 == 5U &&
                lone_star_sequence->actors[1].combatant
                        .command_attribute_79 == 2U &&
                lone_star_sequence->actors[1].combatant
                        .command_attribute_7b == 6U &&
                lone_star_sequence->actors[1].combatant
                        .command_attribute_7c == 6U &&
                lone_star_sequence->actors[1].combatant
                        .action_phase_extension ==
                    std::array<std::uint8_t, 10>{
                        0U, 3U, 5U, 2U, 5U, 6U, 6U, 5U, 6U, 6U} &&
                lone_star_sequence->actors[1].combatant.action_skills ==
                    std::array<std::uint8_t, 4>{2U, 0U, 0U, 0U} &&
                lone_star_sequence->actors[1].combatant
                        .auxiliary_roll_bonus == 2U &&
                elven_mage_sequence.has_value() &&
                elven_mage_sequence->actors.size() == 1U &&
                elven_mage_sequence->actors[0].combatant
                        .auxiliary_roll_bonus == 2U,
            "ROM field encounter catalog did not resolve creature and Lone Star sequences")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    ScriptFieldEncounterTriggerInput encounter_trigger{};
    encounter_trigger.scene_id = 2U;
    encounter_trigger.frame_counter = 10U;
    const auto gate_requests_roll =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.frame_counter = 9U;
    const auto gate_waits_for_cadence =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.frame_counter = 10U;
    encounter_trigger.trigger_delay_active = true;
    const auto gate_respects_delay =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.trigger_delay_active = false;
    encounter_trigger.current_entity_protected = true;
    const auto gate_respects_protected_actor =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.current_entity_protected = false;
    encounter_trigger.activity_lock = true;
    const auto gate_respects_activity_lock =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.activity_lock = false;
    encounter_trigger.world_effects_active = true;
    const auto gate_respects_active_world_effect =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.sequence_mode = true;
    const auto gate_still_rolls_for_active_sequence =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.sequence_mode = false;
    encounter_trigger.world_effects_active = false;
    encounter_trigger.placement_pending = true;
    const auto gate_respects_placement_lock =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.placement_pending = false;
    encounter_trigger.probability_roll = 4U;
    encounter_trigger.probability_threshold = 5U;
    const auto gate_accepts_below_threshold =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.probability_roll = 5U;
    const auto gate_rejects_at_threshold =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    encounter_trigger.sequence_mode = true;
    encounter_trigger.probability_roll = 0U;
    const auto gate_forwards_without_resampling_outcome =
        scripts.evaluate_field_encounter_trigger(encounter_trigger);
    if (!expect(
            gate_requests_roll ==
                ScriptFieldEncounterTriggerAction::sample_probability &&
                gate_waits_for_cadence ==
                    ScriptFieldEncounterTriggerAction::not_due &&
                gate_respects_delay ==
                    ScriptFieldEncounterTriggerAction::blocked &&
                gate_respects_protected_actor ==
                    ScriptFieldEncounterTriggerAction::blocked &&
                gate_respects_activity_lock ==
                    ScriptFieldEncounterTriggerAction::blocked &&
                gate_respects_active_world_effect ==
                    ScriptFieldEncounterTriggerAction::blocked &&
                gate_still_rolls_for_active_sequence ==
                    ScriptFieldEncounterTriggerAction::sample_probability &&
                gate_respects_placement_lock ==
                    ScriptFieldEncounterTriggerAction::blocked &&
                gate_accepts_below_threshold ==
                    ScriptFieldEncounterTriggerAction::select_scene_outcome &&
                gate_rejects_at_threshold ==
                    ScriptFieldEncounterTriggerAction::random_miss &&
                gate_forwards_without_resampling_outcome ==
                    ScriptFieldEncounterTriggerAction::forward_active_sequence,
            "Lua ROM encounter gates did not preserve eligibility and random thresholds")) {
        std::cerr << scripts.failure() << '\n';
        return 2;
    }

    const auto run_ids =
        scripts.string_list("run_registry", "available_run_ids");
    const std::vector<std::string> expected_run_ids{
        "bodyguard_escort", "employee_recruitment", "gang_contract",
        "ghoul_bounty", "matrix_objective", "package_delivery",
        "package_recovery"};
    if (!expect(run_ids.has_value() && *run_ids == expected_run_ids,
                "separate Lua run modules were not auto-registered")) {
        std::cerr << scripts.failure() << '\n';
        return 2;
    }
    if (!verify_dynamic_run_module(
            SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
            std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                "dialogue" / "script-text.lua")) {
        return 3;
    }
    if (!verify_package_delivery_field_events(
            SHADOWRUN_GAMEPLAY_SCRIPT_ROOT,
            std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
                "dialogue" / "script-text.lua")) {
        std::cerr << scripts.failure() << '\n';
        return 3;
    }
    auto run_selection =
        scripts.start_task("run_registry", "prepare_new_game_offer");
    if (!expect(
            run_selection.status ==
                ScriptTaskStep::Status::awaiting_native_operation &&
                run_selection.operation.has_value() &&
                run_selection.operation->type ==
                    ScriptNativeOperation::Type::service_call &&
                run_selection.operation->service_id ==
                    "random.bounded_8",
            "Lua run selection did not request the source bounded random service")) {
        std::cerr << scripts.failure() << '\n';
        return 3;
    }
    run_selection = scripts.complete_native_service_result("roll_4");
    if (!expect(run_selection.operation.has_value() &&
                    run_selection.operation->service_id ==
                        "runner.negotiation_skill",
                "ROM offer selection did not read the primary Negotiation skill")) {
        return 4;
    }
    run_selection = scripts.complete_native_service_result("skill_3");
    if (!expect(run_selection.operation.has_value() &&
                    run_selection.operation->service_id ==
                        "random.bounded_4",
                "courier offer did not use the ROM skill-scaled payment bound")) {
        return 4;
    }
    run_selection = scripts.complete_native_service_result("roll_3");
    if (!expect(
            run_selection.status ==
                ScriptTaskStep::Status::awaiting_native_operation &&
                run_selection.operation.has_value() &&
                run_selection.operation->service_id ==
                    "random.bounded_10",
            "courier run did not request its origin-location roll after negotiation")) {
        std::cerr << scripts.failure() << '\n';
        return 4;
    }
    run_selection = scripts.complete_native_service_result("roll_9");
    if (!expect(
            run_selection.status ==
                ScriptTaskStep::Status::awaiting_native_operation &&
                run_selection.operation.has_value() &&
                run_selection.operation->service_id ==
                    "random.bounded_12",
            "courier run did not request a client from the ROM name table")) {
        std::cerr << scripts.failure() << '\n';
        return 5;
    }
    run_selection = scripts.complete_native_service_result("roll_0");
    if (!expect(
            run_selection.status ==
                ScriptTaskStep::Status::awaiting_native_operation &&
                run_selection.operation.has_value() &&
                run_selection.operation->service_id ==
                    "random.bounded_10",
            "courier run did not request the destination-location roll")) {
        std::cerr << scripts.failure() << '\n';
        return 6;
    }
    run_selection = scripts.complete_native_service_result("roll_5");
    const auto courier_route =
        scripts.string_map("run_registry", "current_route_ids");
    const auto courier_payment =
        scripts.string_map("run_registry", "current_payment_values");
    if (!expect(run_selection.status == ScriptTaskStep::Status::completed &&
                    run_selection.result_id ==
                        std::optional<std::string>{"package_delivery"} &&
                    scripts.call_string("run_registry", "current_offer_id") ==
                        std::optional<std::string>{"package_delivery"} &&
                    scripts.call_string(
                        "run_registry", "current_completion_condition") ==
                        std::optional<std::string>{
                            "package_delivered_safely"} &&
                    courier_route.has_value() &&
                    courier_route->at("origin_location") == "location_11" &&
                    courier_route->at("origin_area") == "area_2" &&
                    courier_route->at("client") == "client_0" &&
                    courier_route->at("destination_location") ==
                        "location_5" &&
                    courier_route->at("destination_area") ==
                        "area_2" && courier_payment.has_value() &&
                    courier_payment->at("base_amount") == "40" &&
                    courier_payment->at("negotiated_amount") == "55" &&
                    courier_payment->at("negotiation_bound") == "4" &&
                    courier_payment->at("negotiation_roll") == "3" &&
                    courier_payment->at("negotiation_skill") == "3",
                "weighted run selection did not initialize its own Lua offer")) {
        std::cerr << scripts.failure() << '\n';
        return 7;
    }

    run_selection =
        scripts.start_task("run_registry", "prepare_new_game_offer");
    if (!expect(run_selection.operation.has_value() &&
                    run_selection.operation->service_id ==
                        "random.bounded_8",
                "ghoul contract selection did not start at the ROM pool")) {
        return 8;
    }
    run_selection = scripts.complete_native_service_result("roll_0");
    if (!expect(run_selection.operation.has_value() &&
                    run_selection.operation->service_id ==
                        "runner.negotiation_skill",
                "ghoul offer did not read the primary Negotiation skill")) {
        return 9;
    }
    run_selection = scripts.complete_native_service_result("skill_3");
    if (!expect(run_selection.operation.has_value() &&
                    run_selection.operation->service_id ==
                        "random.bounded_10",
                "ghoul offer did not use its ROM negotiation scale")) {
        return 9;
    }
    run_selection = scripts.complete_native_service_result("roll_2");
    const auto bounty_route =
        scripts.string_map("run_registry", "current_route_ids");
    const auto bounty_payment =
        scripts.string_map("run_registry", "current_payment_values");
    if (!expect(run_selection.status == ScriptTaskStep::Status::completed &&
                    run_selection.result_id ==
                        std::optional<std::string>{"ghoul_bounty"} &&
                    scripts.call_string(
                        "run_registry", "current_completion_condition") ==
                        std::optional<std::string>{
                            "return_to_johnson_for_payment"} &&
                    bounty_route.has_value() &&
                    bounty_route->at("origin_location") == "none" &&
                    bounty_payment.has_value() &&
                    bounty_payment->at("base_amount") == "10" &&
                    bounty_payment->at("negotiated_amount") == "20" &&
                    bounty_payment->at("negotiation_bound") == "10",
                "ghoul bounty did not keep its distinct Lua objective")) {
        std::cerr << scripts.failure() << '\n';
        return 9;
    }

    run_selection =
        scripts.start_task("run_registry", "prepare_new_game_offer");
    run_selection = scripts.complete_native_service_result("roll_1");
    if (!expect(run_selection.operation.has_value() &&
                    run_selection.operation->service_id ==
                        "runner.negotiation_skill",
                "escort offer did not read the primary Negotiation skill")) {
        return 10;
    }
    run_selection = scripts.complete_native_service_result("skill_3");
    if (!expect(run_selection.operation.has_value() &&
                    run_selection.operation->service_id ==
                        "random.bounded_4",
                "escort offer did not use its ROM negotiation scale")) {
        return 10;
    }
    run_selection = scripts.complete_native_service_result("roll_3");
    run_selection = scripts.complete_native_service_result("roll_4");
    run_selection = scripts.complete_native_service_result("roll_6");
    run_selection = scripts.complete_native_service_result("roll_5");
    const auto escort_route =
        scripts.string_map("run_registry", "current_route_ids");
    const auto escort_payment =
        scripts.string_map("run_registry", "current_payment_values");
    if (!expect(run_selection.status == ScriptTaskStep::Status::completed &&
                    run_selection.result_id ==
                        std::optional<std::string>{"bodyguard_escort"} &&
                    scripts.call_string(
                        "run_registry", "current_completion_condition") ==
                        std::optional<std::string>{
                            "client_reaches_destination_alive"} &&
                    escort_route.has_value() &&
                    escort_route->at("origin_location") == "location_4" &&
                    escort_route->at("origin_area") == "area_2" &&
                    escort_route->at("client") == "client_6" &&
                    escort_route->at("destination_location") ==
                        "location_5" &&
                    escort_route->at("destination_area") ==
                        "area_2" && escort_payment.has_value() &&
                    escort_payment->at("base_amount") == "45" &&
                    escort_payment->at("negotiated_amount") == "60",
                "escort contract did not use its Lua route and client data")) {
        std::cerr << scripts.failure() << '\n';
        return 11;
    }

    struct ExpectedFirstOffer {
        std::string_view run_id;
        std::string_view base_amount;
        std::string_view negotiation_bound;
    };
    constexpr std::array<ExpectedFirstOffer, 8> kExpectedFirstOffers{{
        {"ghoul_bounty", "10", "10"},
        {"bodyguard_escort", "45", "4"},
        {"package_delivery", "35", "4"},
        {"bodyguard_escort", "50", "4"},
        {"package_delivery", "40", "4"},
        {"bodyguard_escort", "55", "4"},
        {"package_delivery", "45", "4"},
        {"ghoul_bounty", "20", "7"},
    }};
    for (std::size_t slot = 0U; slot < kExpectedFirstOffers.size(); ++slot) {
        run_selection =
            scripts.start_task("run_registry", "prepare_new_game_offer");
        if (!expect(run_selection.operation.has_value() &&
                        run_selection.operation->service_id ==
                            "random.bounded_8",
                    "first-run source table did not use its eight slots")) {
            return 12;
        }
        run_selection = scripts.complete_native_service_result(
            "roll_" + std::to_string(slot));
        if (!expect(run_selection.operation.has_value() &&
                        run_selection.operation->service_id ==
                            "runner.negotiation_skill",
                    "selected run row skipped Negotiation lookup")) {
            return 12;
        }
        run_selection = scripts.complete_native_service_result("skill_3");
        const auto expected_bound = std::string("random.bounded_") +
            std::string(kExpectedFirstOffers[slot].negotiation_bound);
        if (!expect(run_selection.operation.has_value() &&
                        run_selection.operation->service_id == expected_bound,
                    "selected run row used the wrong negotiation scale")) {
            return 12;
        }
        run_selection = scripts.complete_native_service_result("roll_0");
        if (kExpectedFirstOffers[slot].run_id != "ghoul_bounty") {
            if (!expect(run_selection.operation.has_value() &&
                            run_selection.operation->service_id ==
                                "random.bounded_10",
                        "route run did not request an origin after payment")) {
                return 12;
            }
            run_selection = scripts.complete_native_service_result("roll_1");
            if (!expect(run_selection.operation.has_value() &&
                            run_selection.operation->service_id ==
                                "random.bounded_12",
                        "route run did not request its source client roll")) {
                return 12;
            }
            run_selection = scripts.complete_native_service_result("roll_0");
            if (!expect(run_selection.operation.has_value() &&
                            run_selection.operation->service_id ==
                                "random.bounded_10",
                        "route run did not request its destination roll")) {
                return 12;
            }
            run_selection = scripts.complete_native_service_result("roll_2");
        }
        const auto payment =
            scripts.string_map("run_registry", "current_payment_values");
        if (!expect(run_selection.status == ScriptTaskStep::Status::completed &&
                        run_selection.result_id == std::optional<std::string>{
                            std::string(kExpectedFirstOffers[slot].run_id)} &&
                        payment.has_value() &&
                        payment->at("base_amount") ==
                            kExpectedFirstOffers[slot].base_amount &&
                        payment->at("negotiated_amount") ==
                            kExpectedFirstOffers[slot].base_amount &&
                        payment->at("negotiation_bound") ==
                            kExpectedFirstOffers[slot].negotiation_bound,
                    "one of the eight source payment rows was reconstructed incorrectly")) {
            return 12;
        }
    }

    FieldScriptRouteContext scene2_location{};
    scene2_location.scene_id = 2U;
    scene2_location.field_mode = 2U;
    scene2_location.event_table_available = true;
    scene2_location.event_table[0] = 20U;
    scene2_location.event_table[1] = 26U;
    scene2_location.event_table[2] = 26U;
    scene2_location.event_table[11] = 51U;
    scene2_location.event_code = FieldCollisionEventCode::event_2;
    const auto stoker_route =
        scripts.resolve_field_location_route(scene2_location);
    scene2_location.event_code = FieldCollisionEventCode::event_11;
    const auto ares_route =
        scripts.resolve_field_location_route(scene2_location);
    scene2_location.event_code = FieldCollisionEventCode::event_0;
    const auto scene20_route =
        scripts.resolve_field_location_route(scene2_location);
    scene2_location.event_code = FieldCollisionEventCode::event_1;
    const auto scene26_route =
        scripts.resolve_field_location_route(scene2_location);
    scene2_location.event_code = FieldCollisionEventCode::no_event;
    const auto missing_event_route =
        scripts.resolve_field_location_route(scene2_location);

    FieldScriptRouteContext unmapped_location{};
    unmapped_location.scene_id = 20U;
    unmapped_location.field_mode = 2U;
    const auto unmapped_route =
        scripts.resolve_field_location_route(unmapped_location);

    FieldScriptRouteContext other_map_location{};
    other_map_location.scene_id = 4U;
    other_map_location.field_mode = 4U;
    other_map_location.event_code = FieldCollisionEventCode::event_2;
    other_map_location.event_table_available = true;
    other_map_location.event_table[2] = 13U;
    const auto other_map_route =
        scripts.resolve_field_location_route(other_map_location);
    other_map_location.event_table_available = false;
    const auto missing_table_route =
        scripts.resolve_field_location_route(other_map_location);

    const auto route_has_destination_mode =
        [&scripts, &other_map_location](
            const std::uint8_t target,
            const WorldActivityMode expected_mode) {
            other_map_location.event_table_available = true;
            other_map_location.event_table[2U] = target;
            const auto route =
                scripts.resolve_field_location_route(other_map_location);
            return route.has_value() &&
                   route->kind == FieldScriptRouteKind::area_event &&
                   route->area_event_target == target &&
                   route->area_event_destination_mode == expected_mode;
        };
    const bool destination_ranges_match =
        route_has_destination_mode(20U, WorldActivityMode::field) &&
        route_has_destination_mode(21U,
                                   WorldActivityMode::resource_activity) &&
        route_has_destination_mode(43U,
                                   WorldActivityMode::resource_activity) &&
        route_has_destination_mode(44U,
                                   WorldActivityMode::scripted_activity) &&
        route_has_destination_mode(55U,
                                   WorldActivityMode::scripted_activity) &&
        route_has_destination_mode(56U, WorldActivityMode::halted) &&
        route_has_destination_mode(255U, WorldActivityMode::halted);

    if (!expect(
            stoker_route.has_value() &&
                stoker_route->kind == FieldScriptRouteKind::dialogue &&
                stoker_route->script ==
                    FieldScriptId::stoker_motel_interaction &&
                !stoker_route->area_event_target.has_value() &&
                ares_route.has_value() &&
                ares_route->kind == FieldScriptRouteKind::field_service &&
                ares_route->script == FieldScriptId::ares_weapon_emporium &&
                scene20_route.has_value() &&
                scene20_route->kind == FieldScriptRouteKind::area_event &&
                scene20_route->area_event_target == 20U &&
                scene20_route->area_event_destination_mode ==
                    WorldActivityMode::field &&
                scene26_route.has_value() &&
                scene26_route->kind == FieldScriptRouteKind::area_event &&
                scene26_route->area_event_target == 26U &&
                scene26_route->area_event_destination_mode ==
                    WorldActivityMode::resource_activity &&
                missing_event_route.has_value() &&
                missing_event_route->kind == FieldScriptRouteKind::no_route &&
                missing_event_route->event_code == 255U &&
                unmapped_route.has_value() &&
                unmapped_route->kind == FieldScriptRouteKind::no_route &&
                !unmapped_route->area_event_target.has_value() &&
                other_map_route.has_value() &&
                other_map_route->kind == FieldScriptRouteKind::area_event &&
                other_map_route->area_event_target == 13U &&
                other_map_route->area_event_destination_mode ==
                    WorldActivityMode::field &&
                missing_table_route.has_value() &&
                missing_table_route->kind == FieldScriptRouteKind::no_route &&
                destination_ranges_match,
            "Lua did not resolve ROM-backed field location routes and destination modes")) {
        std::cerr << scripts.failure() << '\n';
        return 2;
    }

    std::vector<std::vector<std::uint16_t>> source_elevator_groups(56U);
    for (std::uint16_t scene_id = 21U; scene_id <= 25U; ++scene_id) {
        source_elevator_groups[scene_id] = {21U, 22U, 23U, 24U};
    }
    for (std::uint16_t scene_id = 26U; scene_id <= 29U; ++scene_id) {
        source_elevator_groups[scene_id] = {26U, 27U, 28U};
    }
    for (std::uint16_t scene_id = 30U; scene_id <= 34U; ++scene_id) {
        source_elevator_groups[scene_id] = {30U, 31U, 32U, 33U};
    }
    for (std::uint16_t scene_id = 35U; scene_id <= 40U; ++scene_id) {
        source_elevator_groups[scene_id] =
            {35U, 36U, 37U, 38U, 39U};
    }
    for (std::uint16_t scene_id = 41U; scene_id <= 43U; ++scene_id) {
        source_elevator_groups[scene_id] = {41U, 42U};
    }
    const auto nonmember_elevator_entry =
        scripts.resolve_field_location_entry(
            25U, FieldCollisionEventCode::event_29,
            source_elevator_groups);
    const auto member_elevator_entry =
        scripts.resolve_field_location_entry(
            22U, FieldCollisionEventCode::event_29,
            source_elevator_groups);
    const auto three_floor_member_entry =
        scripts.resolve_field_location_entry(
            28U, FieldCollisionEventCode::event_29,
            source_elevator_groups);
    const auto five_floor_member_entry =
        scripts.resolve_field_location_entry(
            36U, FieldCollisionEventCode::event_29,
            source_elevator_groups);
    const auto four_floor_member_entry =
        scripts.resolve_field_location_entry(
            32U, FieldCollisionEventCode::event_29,
            source_elevator_groups);
    const auto two_floor_member_entry =
        scripts.resolve_field_location_entry(
            42U, FieldCollisionEventCode::event_29,
            source_elevator_groups);
    const auto no_group_entry = scripts.resolve_field_location_entry(
        5U, FieldCollisionEventCode::event_29, source_elevator_groups);
    const auto non_elevator_entry =
        scripts.resolve_field_location_entry(
            25U, FieldCollisionEventCode::event_28,
            source_elevator_groups);
    const auto elevator_destination =
        nonmember_elevator_entry.has_value()
            ? scripts.resolve_elevator_destination(
                  nonmember_elevator_entry->elevator_group, 3U)
            : std::optional<std::uint16_t>{};
    const auto invalid_elevator_destination =
        nonmember_elevator_entry.has_value()
            ? scripts.resolve_elevator_destination(
                  nonmember_elevator_entry->elevator_group, 4U)
            : std::optional<std::uint16_t>{};
    const auto two_floor_destination =
        two_floor_member_entry.has_value()
            ? scripts.resolve_elevator_destination(
                  two_floor_member_entry->elevator_group, 0U)
            : std::optional<std::uint16_t>{};
    if (!expect(
            nonmember_elevator_entry.has_value() &&
                nonmember_elevator_entry->action ==
                    ScriptLocationEntryAction::open_elevator_selector &&
                nonmember_elevator_entry->elevator_group.scene_ids ==
                    std::vector<std::uint16_t>{21U, 22U, 23U, 24U} &&
                nonmember_elevator_entry->elevator_group.entry_floor == 0U &&
                member_elevator_entry.has_value() &&
                member_elevator_entry->elevator_group.entry_floor == 1U &&
                three_floor_member_entry.has_value() &&
                three_floor_member_entry->elevator_group.entry_floor == 2U &&
                five_floor_member_entry.has_value() &&
                five_floor_member_entry->elevator_group.entry_floor == 1U &&
                four_floor_member_entry.has_value() &&
                four_floor_member_entry->elevator_group.entry_floor == 2U &&
                two_floor_member_entry.has_value() &&
                two_floor_member_entry->elevator_group.entry_floor == 1U &&
                no_group_entry.has_value() &&
                no_group_entry->action ==
                    ScriptLocationEntryAction::no_action &&
                non_elevator_entry.has_value() &&
                non_elevator_entry->action ==
                    ScriptLocationEntryAction::no_action &&
                elevator_destination == 24U &&
                !invalid_elevator_destination.has_value() &&
                two_floor_destination == 41U,
            "Lua did not resolve ROM-backed elevator entry and destinations")) {
        std::cerr << scripts.failure() << '\n';
        return 105;
    }

    const auto collision = [](
                                const FieldCollisionEventCode event_code,
                                const std::uint16_t scene_id = 0U) {
        FieldMapCollisionEventInput input{};
        input.scene_id = scene_id;
        input.raw_collision_code =
            field_collision_event_value(event_code);
        return input;
    };
    const auto resolves_as = [&scripts](
                                 const FieldMapCollisionEventInput& input,
                                 const FieldMapEventAction action) {
        const auto decision = scripts.resolve_field_collision_event(input);
        return decision.has_value() && decision->action == action;
    };

    auto collision_input = collision(FieldCollisionEventCode::event_31);
    auto collision_decision =
        scripts.resolve_field_collision_event(collision_input);
    if (!expect(collision_decision.has_value() &&
                    collision_decision->action ==
                        FieldMapEventAction::begin_event_31_activity &&
                    collision_decision->source_event_code ==
                        FieldCollisionEventCode::event_31 &&
                    collision_decision->event_code ==
                        FieldCollisionEventCode::event_31,
                "Lua must map event 31 to its typed activity action")) {
        std::cerr << scripts.failure() << '\n';
        return 3;
    }
    collision_input = collision(FieldCollisionEventCode::event_30);
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::begin_event_30_activity),
                "Lua must map event 30 to its typed activity action")) {
        std::cerr << scripts.failure() << '\n';
        return 4;
    }
    collision_input = collision(FieldCollisionEventCode::event_27);
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::begin_event_27_activity),
                "Lua must map event 27 to its typed activity action")) {
        std::cerr << scripts.failure() << '\n';
        return 5;
    }
    collision_input = collision(FieldCollisionEventCode::event_28);
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::begin_event_28_activity),
                "Lua must begin an idle event-28 transition")) {
        std::cerr << scripts.failure() << '\n';
        return 6;
    }
    collision_input = collision(FieldCollisionEventCode::event_29);
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::begin_event_29_activity),
                "Lua must begin an idle event-29 transition")) {
        std::cerr << scripts.failure() << '\n';
        return 7;
    }
    collision_input.activity_sequence_active = true;
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::skip_world_interaction),
                "Lua must suppress a second event-29 activity")) {
        std::cerr << scripts.failure() << '\n';
        return 8;
    }
    collision_input = collision(FieldCollisionEventCode::event_28);
    collision_input.activity_sequence_active = true;
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::skip_world_interaction),
                "Lua must suppress event 28 during an activity")) {
        std::cerr << scripts.failure() << '\n';
        return 9;
    }
    collision_input = collision(FieldCollisionEventCode::event_26);
    collision_input.raw_collision_code = 218U;
    collision_decision =
        scripts.resolve_field_collision_event(collision_input);
    if (!expect(collision_decision.has_value() &&
                    collision_decision->action ==
                        FieldMapEventAction::mark_projected_terrain &&
                    !collision_decision->projected_terrain_event_published,
                "Lua must retain the source terrain-marker suppression")) {
        std::cerr << scripts.failure() << '\n';
        return 10;
    }
    collision_input.raw_collision_code = 26U;
    collision_input.world_effect_control_bit = true;
    collision_decision =
        scripts.resolve_field_collision_event(collision_input);
    if (!expect(collision_decision.has_value() &&
                    !collision_decision->projected_terrain_event_published,
                "Lua must honor the world-effect terrain suppression")) {
        std::cerr << scripts.failure() << '\n';
        return 11;
    }
    constexpr std::array<FieldCollisionEventCode, 4> maglock_events{
        FieldCollisionEventCode::event_2,
        FieldCollisionEventCode::event_3,
        FieldCollisionEventCode::event_4,
        FieldCollisionEventCode::event_5,
    };
    for (const auto event_code : maglock_events) {
        collision_input = collision(event_code, 20U);
        collision_input.maglocks_disabled = false;
        if (!expect(resolves_as(
                        collision_input,
                        FieldMapEventAction::begin_maglock_interaction),
                    "Lua must route each enabled scene-20 lock code to the maglock handler")) {
            std::cerr << scripts.failure() << '\n';
            return 12;
        }
    }
    collision_input = collision(FieldCollisionEventCode::event_2, 20U);
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::begin_maglock_interaction),
                "Lua must preserve the maglock prompt when restored FFE17F is unknown")) {
        std::cerr << scripts.failure() << '\n';
        return 20;
    }
    collision_input = collision(FieldCollisionEventCode::event_4, 20U);
    collision_input.terrain_event_suppressed = true;
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::project_world_interaction),
                "Lua must honor terrain suppression before maglock dispatch")) {
        std::cerr << scripts.failure() << '\n';
        return 13;
    }
    for (const auto event_code : maglock_events) {
        collision_input = collision(event_code, 20U);
        collision_input.maglocks_disabled = true;
        collision_decision =
            scripts.resolve_field_collision_event(collision_input);
        if (!expect(collision_decision.has_value() &&
                        collision_decision->action ==
                            FieldMapEventAction::project_world_interaction &&
                        collision_decision->mode_event_code_published,
                    "FFE17F must bypass each event-2-through-5 maglock prompt")) {
            std::cerr << scripts.failure() << '\n';
            return 19;
        }
    }
    collision_input = collision(FieldCollisionEventCode::event_24);
    if (!expect(resolves_as(
                    collision_input,
                    FieldMapEventAction::begin_security_terminal),
                "Lua must route event 24 to the security terminal")) {
        std::cerr << scripts.failure() << '\n';
        return 14;
    }
    collision_input = collision(FieldCollisionEventCode::event_25);
    collision_input.field_transition_context = 19U;
    collision_decision =
        scripts.resolve_field_collision_event(collision_input);
    if (!expect(collision_decision.has_value() &&
                    collision_decision->action ==
                        FieldMapEventAction::begin_event_25_activity,
                "Lua must select event-25 activity in its source context")) {
        std::cerr << scripts.failure() << '\n';
        return 15;
    }
    collision_input.field_transition_context = 0U;
    collision_decision =
        scripts.resolve_field_collision_event(collision_input);
    if (!expect(collision_decision.has_value() &&
                    collision_decision->action ==
                        FieldMapEventAction::project_world_interaction &&
                    collision_decision->event_code ==
                        FieldCollisionEventCode::event_8 &&
                    collision_decision->raw_collision_code == 168U &&
                    collision_decision->rewrite_direction_probe &&
                    collision_decision->direction_probe_value == -88 &&
                    collision_decision->rewrite_quantized_delta &&
                    collision_decision->quantized_delta == 168U,
                "Lua must preserve the context-free event-25 rewrite")) {
        std::cerr << scripts.failure() << '\n';
        return 16;
    }
    collision_input = collision(FieldCollisionEventCode::event_0);
    collision_input.world_mode = 2U;
    collision_input.interaction_gate = 1U;
    collision_decision =
        scripts.resolve_field_collision_event(collision_input);
    if (!expect(collision_decision.has_value() &&
                    collision_decision->action ==
                        FieldMapEventAction::skip_world_interaction &&
                    collision_decision->mode_event_code_published,
                "Lua must suppress gated generic interactions")) {
        std::cerr << scripts.failure() << '\n';
        return 17;
    }
    collision_input = collision(FieldCollisionEventCode::event_1);
    collision_input.field_global_flags = 80U;
    collision_decision =
        scripts.resolve_field_collision_event(collision_input);
    if (!expect(collision_decision.has_value() &&
                    collision_decision->action ==
                        FieldMapEventAction::begin_field_mode_exit &&
                    !collision_decision->mode_event_code_published,
                "Lua must select field-mode exit from the typed state fact")) {
        std::cerr << scripts.failure() << '\n';
        return 18;
    }

    const auto actor_context = [](
                                   const std::string_view profile_id,
                                   const std::string_view instance_id,
                                   const std::uint16_t entity_kind,
                                   const ScriptActorActionKind action_kind,
                                   const std::uint8_t action_code,
                                   const std::uint8_t entry_delay,
                                   const bool spawn) {
        ScriptActorBehaviorContext context{};
        context.profile_id = profile_id;
        context.entity_kind = entity_kind;
        context.action_kind = action_kind;
        context.action_code = action_code;
        context.entry_delay = entry_delay;
        context.instance_id = instance_id;
        context.spawn = spawn;
        return context;
    };
    std::vector<std::string> observed_heartbeats;
    scripts.set_actor_heartbeat_observer(
        [&observed_heartbeats](const std::string_view actor_id,
                               const std::string_view profile_id,
                               const ScriptActorHeartbeatPhase phase) {
            const auto phase_name =
                phase == ScriptActorHeartbeatPhase::behavior
                    ? "behavior"
                    : "render";
            observed_heartbeats.push_back(
                std::string(actor_id) + "|" + std::string(profile_id) + "|" +
                phase_name);
        });
    auto guard_wait_context = actor_context(
        "corp_guard_k05_item06", "guard_wait", 5U,
        ScriptActorActionKind::item, 6U, 1U, true);
    const auto guard_wait = scripts.actor_heartbeat(guard_wait_context);
    guard_wait_context.spawn = false;
    const auto guard_wait_tick = scripts.actor_heartbeat(guard_wait_context);
    if (!expect(observed_heartbeats.size() == 2U &&
                    observed_heartbeats[0] ==
                        "guard_wait|corp_guard_k05_item06|behavior" &&
                    observed_heartbeats[1] ==
                        "guard_wait|corp_guard_k05_item06|behavior",
                "actor Lua heartbeats must report stable actor, profile, and phase")) {
        return 19;
    }

    auto guard_entry_context = actor_context(
        "corp_guard_k05_item06", "guard_entry", 5U,
        ScriptActorActionKind::item, 6U, 0U, true);
    const auto guard_entry_spawn =
        scripts.actor_heartbeat(guard_entry_context);
    guard_entry_context.spawn = false;
    const auto guard_entry = scripts.actor_heartbeat(guard_entry_context);
    const auto guard_hold = scripts.actor_heartbeat(guard_entry_context);
    guard_entry_context.phase = ScriptActorHeartbeatPhase::render;
    std::optional<ScriptActorBehaviorDecision> guard_entry_last_frame;
    for (std::size_t frame = 0; frame < 9U; ++frame) {
        guard_entry_last_frame =
            scripts.actor_heartbeat(guard_entry_context);
    }
    if (!expect(std::find(observed_heartbeats.begin(),
                          observed_heartbeats.end(),
                          "guard_entry|corp_guard_k05_item06|render") !=
                    observed_heartbeats.end(),
                "render heartbeats must be present in the actor observer")) {
        return 19;
    }

    auto pedestrian_context = actor_context(
        "pedestrian_k0b_none00", "pedestrian_entry", 11U,
        ScriptActorActionKind::none, 0U, 0U, true);
    const auto pedestrian_spawn = scripts.actor_heartbeat(pedestrian_context);
    pedestrian_context.spawn = false;
    const auto pedestrian_entry = scripts.actor_heartbeat(pedestrian_context);

    const auto unsupported_guard_action = scripts.actor_heartbeat(actor_context(
        "corp_guard_k05_item06", "unsupported_guard_action", 5U,
        ScriptActorActionKind::item, 5U, 0U, true));
    const auto unsupported_profile = scripts.actor_heartbeat(actor_context(
        "corp_guard_k05_item06", "unsupported_profile", 11U,
        ScriptActorActionKind::item, 6U, 0U, true));

    const auto target_marker_style_for_action =
        [&scripts, &actor_context](const std::string_view actor_id,
                   const std::string_view profile_id,
                   const std::uint16_t entity_kind,
                   const ScriptActorActionKind action_kind,
                   const std::uint8_t action_code)
            -> std::optional<ScriptActorBehaviorDecision> {
        auto context = actor_context(profile_id, actor_id, entity_kind, action_kind,
                                     action_code, 0U, true);
        context.dynamic_world_encounter_actor = true;
        context.source_record_flags = 0x01U;
        context.entry_animation_frame_count = 4U;
        context.walk_animation_frame_count = 4U;
        if (!scripts.actor_heartbeat(context).has_value()) {
            std::cerr << "target marker actor spawn failed for " << profile_id
                      << ": " << scripts.failure() << '\n';
            return std::nullopt;
        }
        context.spawn = false;
        context.phase = ScriptActorHeartbeatPhase::render;
        context.target_selection_latched = true;
        auto decision = scripts.actor_heartbeat(context);
        if (!decision.has_value()) {
            std::cerr << "target marker render failed for " << profile_id
                      << ": " << scripts.failure() << '\n';
        }
        return decision;
    };
    const auto target_marker_action_0 = target_marker_style_for_action(
        "target_marker_action_0", "hell_hound_k0c_none00", 12U,
        ScriptActorActionKind::none, 0U);
    const auto target_marker_action_3 = target_marker_style_for_action(
        "target_marker_action_3", "eye5r_2_k07_item04", 7U,
        ScriptActorActionKind::item, 4U);
    const auto target_marker_action_6 = target_marker_style_for_action(
        "target_marker_action_6", "rat_shaman_k0b_spell06", 11U,
        ScriptActorActionKind::spell, 6U);
    const auto target_marker_action_9 = target_marker_style_for_action(
        "target_marker_action_9", "elven_mage_k06_spell09", 6U,
        ScriptActorActionKind::spell, 9U);

    auto target_marker_animation_context = actor_context(
        "marker_animation_k06_none00", "target_marker_animation", 6U,
        ScriptActorActionKind::none, 0U, 0U, true);
    target_marker_animation_context.dynamic_world_encounter_actor = true;
    target_marker_animation_context.source_record_flags = 0x01U;
    target_marker_animation_context.entry_animation_frame_count = 4U;
    target_marker_animation_context.walk_animation_frame_count = 4U;
    const auto target_marker_spawn =
        scripts.actor_heartbeat(target_marker_animation_context);
    target_marker_animation_context.spawn = false;
    target_marker_animation_context.phase =
        ScriptActorHeartbeatPhase::render;
    target_marker_animation_context.target_selection_latched = true;
    const auto target_marker_first =
        scripts.actor_heartbeat(target_marker_animation_context);
    std::optional<ScriptActorBehaviorDecision> target_marker_last_animated;
    for (std::size_t frame = 1U; frame < 9U; ++frame) {
        target_marker_last_animated =
            scripts.actor_heartbeat(target_marker_animation_context);
    }
    const auto target_marker_static =
        scripts.actor_heartbeat(target_marker_animation_context);
    target_marker_animation_context.target_selection_latched = false;
    const auto target_marker_cleared =
        scripts.actor_heartbeat(target_marker_animation_context);

    const auto target_candidate = [](
                                       const std::uint8_t slot,
                                       const std::string& actor_id,
                                       const bool active,
                                       const bool has_action) {
        ScriptActorTargetCandidate candidate{};
        candidate.id = actor_id;
        candidate.source_record_slot = slot;
        candidate.descriptor_class = slot;
        candidate.service_flags = active ? 0x01U : 0U;
        candidate.has_action = has_action;
        return candidate;
    };
    auto target_selection_context = actor_context(
        "field_runner_k03_none00", "field_target_selection", 3U,
        ScriptActorActionKind::none, 0U, 0U, true);
    target_selection_context.target_selection_requested = true;
    target_selection_context.target_selection_advances_cursor = true;
    target_selection_context.target_selection_current_slot = 4U;
    for (std::uint8_t slot = 0U; slot < 16U; ++slot) {
        target_selection_context.world_target_selection_candidates.push_back(
            target_candidate(slot, "", false, false));
    }
    target_selection_context.world_target_selection_candidates[5] =
        target_candidate(5U, "reserved_actor", true, true);
    target_selection_context.world_target_selection_candidates[5].service_flags |= 0x10U;
    target_selection_context.world_target_selection_candidates[6] =
        target_candidate(6U, "next_actor", true, true);
    const auto target_selection_next =
        scripts.actor_heartbeat(target_selection_context);
    target_selection_context.spawn = false;
    target_selection_context.target_selection_current_slot = 9U;
    target_selection_context.world_target_selection_candidates[3] =
        target_candidate(3U, "first_actor", true, false);
    target_selection_context.world_target_selection_candidates[4] =
        target_candidate(4U, "wrapped_actor", true, true);
    const auto target_selection_wrap =
        scripts.actor_heartbeat(target_selection_context);

    auto special_target_selection_context = actor_context(
        "field_runner_k03_none00", "field_special_target_selection", 3U,
        ScriptActorActionKind::none, 0U, 0U, true);
    special_target_selection_context.target_selection_requested = true;
    special_target_selection_context.target_selection_advances_cursor = true;
    special_target_selection_context.target_selection_special_marker = true;
    special_target_selection_context.target_selection_mode = 0x0cU;
    special_target_selection_context.target_selection_current_slot = 15U;
    for (std::uint8_t slot = 0U; slot < 16U; ++slot) {
        special_target_selection_context.world_target_selection_candidates.push_back(
            target_candidate(slot, "", false, false));
    }
    special_target_selection_context.world_target_selection_candidates[0] =
        target_candidate(0U, "special_actor", true, false);
    const auto target_selection_special =
        scripts.actor_heartbeat(special_target_selection_context);

    auto hound_effect_context = actor_context(
        "hell_hound_k0c_none00", "hell_hound_effect_cadence", 12U,
        ScriptActorActionKind::none, 0U, 0U, true);
    hound_effect_context.dynamic_world_encounter_actor = true;
    hound_effect_context.source_record_flags = 0x01U;
    hound_effect_context.movement_allowed = true;
    hound_effect_context.entry_animation_frame_count = 1U;
    hound_effect_context.walk_animation_frame_count = 1U;
    hound_effect_context.delay_primary_attribute = 8U;
    hound_effect_context.delay_secondary_attribute = 2U;
    hound_effect_context.ai_target_candidates.push_back(
        ScriptActorTargetCandidate{
            "party.runner.0", 0x1000U, 0x1000U, 0x2000U, 0x0100U,
            0x01U, 0U, 0U, 0U, true, false});
    hound_effect_context.ai_random_bound =
        [](const std::uint16_t bound) -> std::optional<std::uint16_t> {
        return bound == 0U
            ? std::nullopt
            : std::optional<std::uint16_t>{0U};
    };
    const auto hound_effect_spawn =
        scripts.actor_heartbeat(hound_effect_context);
    hound_effect_context.spawn = false;
    auto hound_entry = scripts.actor_heartbeat(hound_effect_context);
    hound_effect_context.phase = ScriptActorHeartbeatPhase::render;
    for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
        hound_entry = scripts.actor_heartbeat(hound_effect_context);
    }
    hound_effect_context.phase = ScriptActorHeartbeatPhase::behavior;
    std::optional<ScriptActorBehaviorDecision> hound_effect_start;
    for (std::uint8_t service = 0U;
         service < 100U &&
         (!hound_effect_start.has_value() ||
          !hound_effect_start->target_motion_effect_slot_request.has_value());
         ++service) {
        hound_effect_start =
            scripts.actor_heartbeat(hound_effect_context);
    }

    std::array<std::uint8_t, 4> hound_effect_visible_counts{};
    bool hound_effect_released = false;
    hound_effect_context.world_effect_slot_occupancy[0] = true;
    hound_effect_context.phase = ScriptActorHeartbeatPhase::render;
    for (std::uint8_t frame = 0U; frame < 22U; ++frame) {
        const auto decision = scripts.actor_heartbeat(hound_effect_context);
        if (!decision.has_value()) {
            break;
        }
        if (decision->target_motion_effect_visible &&
            decision->target_motion_effect_phase <
                hound_effect_visible_counts.size()) {
            ++hound_effect_visible_counts[
                decision->target_motion_effect_phase];
        }
        hound_effect_released =
            hound_effect_released ||
            decision->target_motion_effect_slot_released;
    }

    hound_effect_context.world_effect_slot_occupancy[0] = false;
    hound_effect_context.phase = ScriptActorHeartbeatPhase::behavior;
    bool hound_effect_repeated_before_cooldown = false;
    for (std::uint8_t service = 0U; service < 90U; ++service) {
        const auto decision = scripts.actor_heartbeat(hound_effect_context);
        hound_effect_repeated_before_cooldown =
            hound_effect_repeated_before_cooldown ||
            (decision.has_value() &&
             decision->target_motion_effect_slot_request.has_value());
    }
    std::optional<ScriptActorBehaviorDecision> hound_effect_after_cooldown;
    for (std::uint8_t service = 0U;
         service < 15U &&
         (!hound_effect_after_cooldown.has_value() ||
          !hound_effect_after_cooldown->target_motion_effect_slot_request.has_value());
         ++service) {
        hound_effect_after_cooldown =
            scripts.actor_heartbeat(hound_effect_context);
    }
    if (!expect(
            hound_effect_spawn.has_value() &&
                hound_entry.has_value() &&
                hound_effect_start.has_value() &&
                hound_effect_start->target_motion_effect_slot_request == 0U &&
                hound_effect_start->action_delay == 90 &&
                hound_effect_visible_counts[1] == 7U &&
                hound_effect_visible_counts[2] == 7U &&
                hound_effect_visible_counts[3] == 7U &&
                hound_effect_released &&
                !hound_effect_repeated_before_cooldown &&
                hound_effect_after_cooldown.has_value() &&
                hound_effect_after_cooldown->target_motion_effect_slot_request == 0U &&
                hound_effect_after_cooldown->action_delay == 90,
            "Hell Hound close effect did not follow its source slot, phase, and +0xDC cadence")) {
        std::cerr << scripts.failure() << '\n';
        return 28;
    }

    const auto dynamic_motion_trace =
        [&scripts, &actor_context](
            const std::string_view actor_id,
            const std::string_view profile_id,
            const std::uint16_t entity_kind,
            const ScriptActorActionKind action_kind,
            const std::uint8_t action_code,
            const std::uint16_t target_distance,
            const std::uint16_t direct_step_cost,
            const std::uint16_t perpendicular_step_cost,
            const std::int16_t initial_action_delay = 0,
            const std::int16_t ai_animation_frame = 0) {
            std::vector<ScriptActorBehaviorDecision> trace;
            auto context = actor_context(
                profile_id, actor_id, entity_kind, action_kind, action_code,
                0U, true);
            context.dynamic_world_encounter_actor = true;
            context.source_record_flags = 0x25U;
            context.entry_animation_frame_count = 1U;
            context.walk_animation_frame_count = 4U;
            context.initial_action_delay = initial_action_delay;
            context.ai_animation_frame = ai_animation_frame;
            context.movement_allowed = true;
            context.ai_target_candidates.push_back(
                ScriptActorTargetCandidate{
                    "party.runner.0", 0x1800U, 0x1800U, 0x2000U,
                    target_distance, 0x01U, 0U, 0U, 0U, true, false,
                    direct_step_cost,
                    perpendicular_step_cost, target_distance,
                    static_cast<std::uint16_t>(target_distance * 2U)});
            context.ai_random_bound =
                [](const std::uint16_t bound)
                    -> std::optional<std::uint16_t> {
                return bound == 0U
                    ? std::nullopt
                    : std::optional<std::uint16_t>{0U};
            };
            if (!scripts.actor_heartbeat(context).has_value()) {
                return trace;
            }
            context.spawn = false;
            if (!scripts.actor_heartbeat(context).has_value()) {
                return trace;
            }
            context.phase = ScriptActorHeartbeatPhase::render;
            for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
                if (!scripts.actor_heartbeat(context).has_value()) {
                    return trace;
                }
            }
            context.phase = ScriptActorHeartbeatPhase::behavior;
            for (std::uint16_t pass = 0U; pass < 220U; ++pass) {
                const auto decision = scripts.actor_heartbeat(context);
                if (!decision.has_value()) {
                    trace.clear();
                    return trace;
                }
                trace.push_back(*decision);
                if (decision->transition_complete &&
                    decision->movement_heading.has_value() &&
                    decision->ai_target_id.has_value()) {
                    if (decision->target_motion_delay == 0U &&
                        *decision->movement_heading == 0x6000U) {
                        break;
                    }
                }
            }
            return trace;
        };
    const auto item_one_motion = dynamic_motion_trace(
        "motion_item_one", "eye5r_1_k07_item01", 7U,
        ScriptActorActionKind::item, 1U, 0x0700U, 0x0700U, 0x0E00U);
    const auto bear_totem_motion = dynamic_motion_trace(
        "motion_bear_totem", "bear_shaman_k0b_item2b", 11U,
        ScriptActorActionKind::item, 43U, 0x0700U, 0x0700U, 0x0E00U);
    const auto mage_spell_motion = dynamic_motion_trace(
        "motion_mage_spell", "rat_shaman_k0b_spell05", 11U,
        ScriptActorActionKind::spell, 5U, 0x0600U, 0x0600U, 0x0C00U);
    const auto close_motion = dynamic_motion_trace(
        "motion_close_route", "eye5r_1_k07_item01", 7U,
        ScriptActorActionKind::item, 1U, 0x0400U, 0x0400U, 0x01DFU);
    const auto hell_hound_close_motion = dynamic_motion_trace(
        "motion_hound_close", "hell_hound_k0c_none00", 12U,
        ScriptActorActionKind::none, 0U, 0x0200U, 0x0200U, 0x0400U,
        120);
    const auto hell_hound_mid_motion = dynamic_motion_trace(
        "motion_hound_mid", "hell_hound_k0c_none00", 12U,
        ScriptActorActionKind::none, 0U, 0x0300U, 0x0300U, 0x0600U,
        120);
    const auto hell_hound_outer_motion = dynamic_motion_trace(
        "motion_hound_outer", "hell_hound_k0c_none00", 12U,
        ScriptActorActionKind::none, 0U, 0x05FFU, 0x05FFU, 0x0BFEU,
        120);
    const auto hell_hound_far_motion = dynamic_motion_trace(
        "motion_hound_far", "hell_hound_k0c_none00", 12U,
        ScriptActorActionKind::none, 0U, 0x0600U, 0x0600U, 0x0C00U,
        1000);
    const auto gargoyle_close_motion = dynamic_motion_trace(
        "motion_gargoyle_close", "gargoyle_k0d_none00", 13U,
        ScriptActorActionKind::none, 0U, 0x01FFU, 0x01FFU, 0x03FEU,
        120, 1);
    const auto gargoyle_frame_two_motion = dynamic_motion_trace(
        "motion_gargoyle_frame_two", "gargoyle_k0d_none00", 13U,
        ScriptActorActionKind::none, 0U, 0x01FFU, 0x01FFU, 0x03FEU,
        120, 2);
    const auto wendigo_close_motion = dynamic_motion_trace(
        "motion_wendigo_close", "wendigo_k0e_none00", 14U,
        ScriptActorActionKind::none, 0U, 0x01FFU, 0x01FFU, 0x03FEU,
        120, 1);
    const auto generic_close_motion = dynamic_motion_trace(
        "motion_generic_close", "eye5r_1_k07_none00", 7U,
        ScriptActorActionKind::none, 0U, 0x01FFU, 0x01FFU, 0x03FEU,
        120);
    const auto generic_boundary_motion = dynamic_motion_trace(
        "motion_generic_boundary", "eye5r_1_k07_none00", 7U,
        ScriptActorActionKind::none, 0U, 0x0200U, 0x0200U, 0x0400U,
        120);
    const auto find_motion_decision = [](
                                           const auto& trace,
                                           const std::uint8_t delay,
                                           const std::uint16_t heading) {
        for (const auto& decision : trace) {
            if (decision.transition_complete &&
                decision.ai_target_id.has_value() &&
                decision.target_motion_delay == delay &&
                decision.movement_heading == heading) {
                return &decision;
            }
        }
        return static_cast<const ScriptActorBehaviorDecision*>(nullptr);
    };
    const auto* item_one_initial = find_motion_decision(
        item_one_motion, 5U, 0x2000U);
    const auto* item_one_expired = find_motion_decision(
        item_one_motion, 0U, 0x6000U);
    const auto* bear_totem_initial = find_motion_decision(
        bear_totem_motion, 0U, 0x6000U);
    const auto* mage_spell_after_delay = find_motion_decision(
        mage_spell_motion, 0U, 0x6000U);
    const auto mage_spell_unresolved = std::find_if(
        mage_spell_motion.begin(), mage_spell_motion.end(),
        [](const auto& decision) {
            return decision.transition_complete &&
                   decision.ai_target_id.has_value() &&
                   decision.target_motion_delay == 5U &&
                   !decision.movement_heading.has_value();
        });
    const bool close_route_stopped = std::any_of(
        close_motion.begin(), close_motion.end(),
        [](const auto& decision) {
            return decision.transition_complete &&
                   decision.ai_target_id.has_value() &&
                   !decision.movement_heading.has_value();
        });
    const auto find_engaged_decision = [](const auto& trace) {
        return std::find_if(
            trace.begin(), trace.end(), [](const auto& decision) {
                return decision.transition_complete &&
                       decision.ai_target_id.has_value() &&
                       decision.ai_target_engaged;
            });
    };
    const auto* hound_close_engagement = [&]() {
        const auto found = find_engaged_decision(hell_hound_close_motion);
        return found == hell_hound_close_motion.end() ? nullptr : &*found;
    }();
    const auto* hound_mid_engagement = [&]() {
        const auto found = find_engaged_decision(hell_hound_mid_motion);
        return found == hell_hound_mid_motion.end() ? nullptr : &*found;
    }();
    const auto* hound_outer_engagement = [&]() {
        const auto found = find_engaged_decision(hell_hound_outer_motion);
        return found == hell_hound_outer_motion.end() ? nullptr : &*found;
    }();
    const auto* gargoyle_engagement = [&]() {
        const auto found = find_engaged_decision(gargoyle_close_motion);
        return found == gargoyle_close_motion.end() ? nullptr : &*found;
    }();
    const auto* wendigo_engagement = [&]() {
        const auto found = find_engaged_decision(wendigo_close_motion);
        return found == wendigo_close_motion.end() ? nullptr : &*found;
    }();
    const auto* generic_engagement = [&]() {
        const auto found = find_engaged_decision(generic_close_motion);
        return found == generic_close_motion.end() ? nullptr : &*found;
    }();
    auto c0a2_action_context = actor_context(
        "eye5r_1_k07_none00", "c0a2_action", 7U,
        ScriptActorActionKind::none, 0U, 0U, true);
    c0a2_action_context.dynamic_world_encounter_actor = true;
    c0a2_action_context.source_record_flags = 0x25U;
    c0a2_action_context.entry_animation_frame_count = 1U;
    c0a2_action_context.walk_animation_frame_count = 4U;
    c0a2_action_context.action_animation_frame_count = 2U;
    c0a2_action_context.action_animation_increment = 0x0200U;
    c0a2_action_context.action_animation_threshold = 0x1000U;
    c0a2_action_context.action_animation_audio_event = 0x004CU;
    c0a2_action_context.initial_action_delay = 0;
    c0a2_action_context.delay_primary_attribute = 4U;
    c0a2_action_context.delay_secondary_attribute = 6U;
    c0a2_action_context.movement_allowed = true;
    c0a2_action_context.ai_target_candidates.push_back(
        ScriptActorTargetCandidate{
            "party.runner.0", 0x1800U, 0x1800U, 0x2000U,
            0x0100U, 0x01U, 0U, 0U, 0U, true, false,
            0x0100U, 0x0200U, 0x0100U, 0x0200U});
    c0a2_action_context.ai_random_bound =
        [](const std::uint16_t bound)
            -> std::optional<std::uint16_t> {
        return bound == 0U
            ? std::nullopt
            : std::optional<std::uint16_t>{0U};
    };
    auto c0a2_alternate_context = c0a2_action_context;
    c0a2_alternate_context.instance_id = "eye5r_1_k07_item02";
    c0a2_alternate_context.target_heading_audio_alternate = true;
    const auto c0a2_action_spawn =
        scripts.actor_heartbeat(c0a2_action_context);
    c0a2_action_context.spawn = false;
    const auto c0a2_entry_start =
        scripts.actor_heartbeat(c0a2_action_context);
    c0a2_action_context.phase = ScriptActorHeartbeatPhase::render;
    for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
        (void)scripts.actor_heartbeat(c0a2_action_context);
    }
    c0a2_action_context.phase = ScriptActorHeartbeatPhase::behavior;
    std::optional<ScriptActorBehaviorDecision> c0a2_action_start;
    for (std::uint8_t pass = 0U; pass < 100U; ++pass) {
        const auto decision = scripts.actor_heartbeat(c0a2_action_context);
        if (!decision.has_value()) {
            break;
        }
        if (decision->animation_id == "action") {
            c0a2_action_start = decision;
            break;
        }
    }
    const auto c0a2_alternate_spawn =
        scripts.actor_heartbeat(c0a2_alternate_context);
    c0a2_alternate_context.spawn = false;
    (void)scripts.actor_heartbeat(c0a2_alternate_context);
    c0a2_alternate_context.phase = ScriptActorHeartbeatPhase::render;
    for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
        (void)scripts.actor_heartbeat(c0a2_alternate_context);
    }
    c0a2_alternate_context.phase = ScriptActorHeartbeatPhase::behavior;
    std::optional<ScriptActorBehaviorDecision> c0a2_alternate_action_start;
    for (std::uint8_t pass = 0U; pass < 100U; ++pass) {
        const auto decision =
            scripts.actor_heartbeat(c0a2_alternate_context);
        if (!decision.has_value()) {
            break;
        }
        if (decision->animation_id == "action") {
            c0a2_alternate_action_start = decision;
            break;
        }
    }
    c0a2_action_context.phase = ScriptActorHeartbeatPhase::render;
    std::optional<ScriptActorBehaviorDecision> c0a2_action_frame_zero;
    std::optional<ScriptActorBehaviorDecision> c0a2_action_frame_one;
    std::optional<ScriptActorBehaviorDecision> c0a2_action_complete;
    for (std::uint8_t frame = 0U; frame < 8U; ++frame) {
        c0a2_action_frame_zero =
            scripts.actor_heartbeat(c0a2_action_context);
    }
    for (std::uint8_t frame = 0U; frame < 8U; ++frame) {
        c0a2_action_frame_one =
            scripts.actor_heartbeat(c0a2_action_context);
    }
    for (std::uint8_t frame = 0U; frame < 8U; ++frame) {
        c0a2_action_complete =
            scripts.actor_heartbeat(c0a2_action_context);
    }
    const bool c0a2_action_contract =
        c0a2_action_spawn.has_value() &&
        c0a2_entry_start.has_value() &&
        c0a2_action_start.has_value() &&
        c0a2_action_start->ai_target_engaged &&
        c0a2_action_start->audio_event == 0x004CU &&
        c0a2_action_start->target_heading_audio_event == 0x004DU &&
        c0a2_action_start->action_delay == 45 &&
        c0a2_action_start->animation_frame == 0U &&
        c0a2_alternate_spawn.has_value() &&
        c0a2_alternate_action_start.has_value() &&
        c0a2_alternate_action_start->audio_event == 0x004CU &&
        c0a2_alternate_action_start->target_heading_audio_event == 0x004EU &&
        c0a2_action_frame_zero.has_value() &&
        c0a2_action_frame_zero->animation_id == "action" &&
        c0a2_action_frame_zero->animation_frame == 1U &&
        c0a2_action_frame_one.has_value() &&
        c0a2_action_frame_one->animation_frame == 1U &&
        c0a2_action_complete.has_value() &&
        c0a2_action_complete->animation_frame == 1U;
    const bool generic_boundary_rejected = std::none_of(
        generic_boundary_motion.begin(), generic_boundary_motion.end(),
        [](const auto& decision) {
            return decision.transition_complete &&
                   decision.ai_target_id.has_value() &&
                   decision.ai_target_engaged;
        });
    const auto gargoyle_frame_two_hold = std::find_if(
        gargoyle_frame_two_motion.begin(), gargoyle_frame_two_motion.end(),
        [](const auto& decision) {
            return decision.transition_complete &&
                   decision.ai_target_id.has_value() &&
                   !decision.ai_target_engaged &&
                   decision.animation_accumulator == 0x1000;
        });
    const bool hound_far_keeps_pursuit = std::any_of(
        hell_hound_far_motion.begin(), hell_hound_far_motion.end(),
        [](const auto& decision) {
            return decision.transition_complete &&
                   decision.ai_target_id.has_value() &&
                   !decision.ai_target_engaged &&
                   decision.movement_heading.has_value();
        });
    bool item_one_heading_slews_between_route_updates = false;
    for (std::size_t index = 0U; index + 1U < item_one_motion.size(); ++index) {
        const auto& first = item_one_motion[index];
        const auto& second = item_one_motion[index + 1U];
        if (first.movement_heading == 0x2000U &&
            first.movement_current_heading == 0x0800U &&
            first.movement_route_distance == 0x0700U &&
            second.movement_heading == 0x2000U &&
            second.movement_current_heading == 0x1000U &&
            second.target_motion_delay == first.target_motion_delay) {
            item_one_heading_slews_between_route_updates = true;
            break;
        }
    }
    constexpr char kHexDigits[] = "0123456789abcdef";
    bool spell_motion_catalog_matches = true;
    const auto& source_spell_catalog = us_magic_spell_catalog();
    for (std::size_t index = 0U; index < source_spell_catalog.size();
         ++index) {
        const auto action_phase = static_cast<std::uint8_t>(index + 1U);
        const auto profile_id =
            std::string("motion_spell_k0b_spell0") +
            kHexDigits[action_phase];
        const auto trace = dynamic_motion_trace(
            std::string("motion_spell_profile_") +
                std::string(1U, kHexDigits[action_phase]),
            profile_id, 11U, ScriptActorActionKind::spell, action_phase,
            0x0700U, 0x0700U, 0x0E00U);
        const auto first_motion = std::find_if(
            trace.begin(), trace.end(), [](const auto& decision) {
                return decision.transition_complete &&
                       decision.ai_target_id.has_value();
            });
        const auto source_profile = static_cast<std::uint8_t>(
            source_spell_catalog[index].attribute_selector);
        const bool valid_first_motion = first_motion != trace.end() &&
            (source_profile == 0U
                 ? first_motion->movement_heading == 0x2000U &&
                       first_motion->target_motion_delay == 5U
             : source_profile == 1U || source_profile == 2U
                 ? first_motion->movement_heading == 0x6000U &&
                       first_motion->target_motion_delay == 0U
             : source_profile == 3U
                 ? first_motion->movement_heading == 0x6000U &&
                       first_motion->target_motion_delay == 5U
                 : !first_motion->movement_heading.has_value() &&
                       first_motion->target_motion_delay == 5U);
        spell_motion_catalog_matches =
            spell_motion_catalog_matches && valid_first_motion;
    }
    if (!expect(
            item_one_initial != nullptr &&
                item_one_initial->movement_heading == 0x2000U &&
                item_one_initial->target_motion_delay == 5U &&
                item_one_expired != nullptr &&
                item_one_expired->movement_heading == 0x6000U &&
                item_one_expired->movement_route_distance == 0x0E00U &&
                item_one_heading_slews_between_route_updates &&
                bear_totem_initial != nullptr &&
                bear_totem_initial->movement_heading == 0x6000U &&
                mage_spell_unresolved != mage_spell_motion.end() &&
                mage_spell_after_delay != nullptr &&
                mage_spell_after_delay->movement_heading == 0x6000U &&
                close_route_stopped && spell_motion_catalog_matches &&
                hound_close_engagement != nullptr &&
                hound_close_engagement->movement_heading == 0x2000U &&
                hound_close_engagement->ai_target_heading == 0x2000U &&
                hound_close_engagement->movement_current_heading != 0x2000U &&
                hound_mid_engagement != nullptr &&
                hound_mid_engagement->movement_current_heading == 0x2000U &&
                hound_outer_engagement != nullptr &&
                hound_outer_engagement->movement_current_heading == 0x2000U &&
                hound_far_keeps_pursuit &&
                gargoyle_engagement != nullptr &&
                gargoyle_engagement->movement_current_heading == 0x2000U &&
                wendigo_engagement != nullptr &&
                wendigo_engagement->movement_current_heading == 0x2000U &&
                generic_engagement != nullptr &&
                generic_engagement->ai_target_heading == 0x2000U &&
                c0a2_action_contract &&
                generic_boundary_rejected &&
                gargoyle_frame_two_hold != gargoyle_frame_two_motion.end(),
            "Lua actor motion failed item/spell routes, six 21-pass route updates, B574 cadence, close target ranges, C0A2 action cadence, kind-C bands, or the kind-D frame-two hold")) {
        const auto print_motion_trace = [](const char* label,
                                           const auto& trace) {
            std::cerr << label << " samples=" << trace.size();
            for (const auto& decision : trace) {
                if (decision.transition_complete &&
                    decision.ai_target_id.has_value()) {
                    std::cerr << " {delay="
                              << static_cast<unsigned>(
                                     decision.target_motion_delay)
                              << ",heading=";
                    if (decision.movement_heading.has_value()) {
                        std::cerr << "0x" << std::hex
                                  << *decision.movement_heading << std::dec;
                    } else {
                        std::cerr << "none";
                    }
                    std::cerr << '}';
                }
            }
            std::cerr << '\n';
        };
        print_motion_trace("item_one", item_one_motion);
        print_motion_trace("bear_totem", bear_totem_motion);
        print_motion_trace("mage_spell", mage_spell_motion);
        print_motion_trace("close", close_motion);
        std::cerr << scripts.failure() << '\n';
        return 29;
    }

    struct SpellRefresh {
        std::optional<ScriptActorBehaviorDecision> refresh;
        std::optional<ScriptActorBehaviorDecision> render_hold;
        std::optional<ScriptActorBehaviorDecision> completed_effect;
        std::optional<ScriptActorBehaviorDecision> next_behavior;
        std::uint8_t spell_effect_visible_heartbeats = 0U;
        std::uint8_t spell_effect_heartbeat_count = 0U;
        bool effect_completed_early = false;
        std::array<std::uint8_t, 96> spell_effect_phase_trace{};
        std::array<bool, 96> spell_effect_visible_trace{};
        std::vector<std::string> fanout_actor_ids;
        std::vector<std::uint8_t> fanout_heartbeat_counts;
        std::vector<bool> fanout_released_slot;
    };
    const auto drive_spell_refresh = [&scripts, &actor_context](
                                         const std::string& profile_id,
                                         const std::string& actor_id,
                                         const std::uint16_t entity_kind,
                                         const std::uint8_t action_code,
                                         const std::int16_t initial_delay,
                                         const std::optional<bool> projection,
                                         const ScriptActorActionLineClear& line,
                                         const ScriptCombatantFacts* caster =
                                             nullptr,
                                         const ScriptCombatantFacts* target_facts =
                                             nullptr,
                                         const bool occupy_all_effect_slots =
                                             false,
                                         const std::uint8_t world_scene_state =
                                             0U,
                                         const std::optional<std::uint8_t>
                                             caster_render_flags = std::nullopt,
                                         const std::vector<ScriptActorTargetCandidate>*
                                             additional_scan_candidates =
                                                 nullptr,
                                         const bool active_runner_alias = false)
        -> SpellRefresh {
        SpellRefresh result;
        auto context = actor_context(
            profile_id, actor_id, entity_kind, ScriptActorActionKind::spell,
            action_code, 0U, true);
        context.dynamic_world_encounter_actor = true;
        context.source_record_flags = 0x01U;
        context.world_scene_state = world_scene_state;
        context.entry_animation_frame_count = 1U;
        context.walk_animation_frame_count = 4U;
        context.initial_action_delay = initial_delay;
        context.movement_scene_id = 2U;
        context.movement_allowed = true;
        context.source_projection_active = projection;
        context.action_line_clear = line;
        context.world_effect_slot_occupancy.fill(occupy_all_effect_slots);
        context.ai_random_bound =
            [](const std::uint16_t bound) -> std::optional<std::uint16_t> {
                return bound == 0U ? std::nullopt
                                   : std::optional<std::uint16_t>{0U};
            };
        context.ai_target_candidates.push_back(ScriptActorTargetCandidate{
            "party.runner.0", 0x1234U, 0x0ABCU, 0x2000U, 0x0C00U, 0x01U, 0U,
            0U, 0U, true, false, 0x0100U, 0x0200U, 0x0C00U, 0x1800U});
        context.ai_target_candidates.back().source_record_slot = 0U;
        if (additional_scan_candidates != nullptr) {
            context.ai_target_candidates.insert(
                context.ai_target_candidates.end(),
                additional_scan_candidates->begin(),
                additional_scan_candidates->end());
        }
        if (caster != nullptr) {
            context.combatant = *caster;
        }
        if (target_facts != nullptr) {
            context.ai_target_candidates.front().combatant = *target_facts;
        }
        const std::string effect_target_actor_id = active_runner_alias
            ? "field_player_runner"
            : "party.runner.0";
        auto effect_target_context = actor_context(
            "field_runner_k03_none00", effect_target_actor_id, 3U,
            ScriptActorActionKind::none, 0U, 0U, true);
        effect_target_context.source_record_slot = 0U;
        effect_target_context.movement_scene_id = 2U;
        effect_target_context.world_scene_state = world_scene_state;
        effect_target_context.ai_target_candidates =
            context.ai_target_candidates;
        if (caster_render_flags.has_value()) {
            ScriptActorTargetCandidate caster_candidate{};
            caster_candidate.id = actor_id;
            caster_candidate.service_flags = *caster_render_flags;
            effect_target_context.ai_target_candidates.push_back(
                std::move(caster_candidate));
        }
        effect_target_context.combatant = target_facts == nullptr
            ? ScriptCombatantFacts{}
            : *target_facts;
        std::size_t combat_dice_index = 0U;
        if (caster != nullptr) {
            context.ai_random_bound = [&combat_dice_index](
                                          const std::uint16_t bound)
                -> std::optional<std::uint16_t> {
                if (bound == 0U) {
                    return std::nullopt;
                }
                if (bound == 6U) {
                    const auto face = (combat_dice_index++ & 1U) == 0U
                        ? 5U
                        : 0U;
                    return static_cast<std::uint16_t>(face);
                }
                return 0U;
            };
        }
        effect_target_context.ai_random_bound = context.ai_random_bound;
        const auto failed = [&scripts, &result]() {
            std::cerr << scripts.failure() << '\n';
            return result;
        };
        if (!scripts.actor_heartbeat(context).has_value()) {
            return failed();
        }
        if (!scripts.actor_heartbeat(effect_target_context).has_value()) {
            return failed();
        }
        context.spawn = false;
        effect_target_context.spawn = false;
        if (!scripts.actor_heartbeat(context).has_value()) {
            return failed();
        }
        context.phase = ScriptActorHeartbeatPhase::render;
        for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
            if (!scripts.actor_heartbeat(context).has_value()) {
                return failed();
            }
        }
        context.phase = ScriptActorHeartbeatPhase::behavior;
        for (std::uint8_t pass = 0U; pass < 120U; ++pass) {
            const auto decision = scripts.actor_heartbeat(context);
            if (!decision.has_value()) {
                return failed();
            }
            if (!decision->source_action_dispatch.has_value()) {
                continue;
            }
            result.refresh = decision;
            if (decision->spell_action_resolution.has_value() &&
                decision->spell_action_resolution->next_action_kind.has_value() &&
                decision->spell_action_resolution->next_action_code.has_value() &&
                decision->spell_action_resolution->next_profile_id.has_value()) {
                context.profile_id =
                    *decision->spell_action_resolution->next_profile_id;
                context.action_kind =
                    *decision->spell_action_resolution->next_action_kind;
                context.action_code =
                    *decision->spell_action_resolution->next_action_code;
            }
            if (decision->spell_action_resolution.has_value()) {
                const auto& target_resolution =
                    *decision->spell_action_resolution;
                const auto target_candidate = std::find_if(
                    context.ai_target_candidates.begin(),
                    context.ai_target_candidates.end(),
                    [&target_resolution](const auto& candidate) {
                        return candidate.id == target_resolution.target_id &&
                               candidate.source_record_slot.has_value() &&
                               *candidate.source_record_slot ==
                                   target_resolution.target_record_slot;
                    });
                if (target_candidate != context.ai_target_candidates.end() &&
                    target_candidate->combatant.has_value()) {
                    effect_target_context.instance_id =
                        active_runner_alias &&
                                target_resolution.target_record_slot == 0U
                            ? std::string_view{"field_player_runner"}
                            : std::string_view{target_candidate->id};
                    effect_target_context.source_record_slot =
                        target_resolution.target_record_slot;
                    effect_target_context.combatant =
                        *target_candidate->combatant;
                }
            }
            context.phase = ScriptActorHeartbeatPhase::render;
            result.render_hold = scripts.actor_heartbeat(context);
            if (!result.render_hold.has_value()) {
                return failed();
            }
            effect_target_context.phase =
                ScriptActorHeartbeatPhase::render;
            for (std::uint8_t frame = 0U; frame < 96U; ++frame) {
                const auto effect_heartbeat =
                    scripts.actor_heartbeat(effect_target_context);
                if (!effect_heartbeat.has_value()) {
                    return failed();
                }
                result.spell_effect_phase_trace[frame] =
                    effect_heartbeat->spell_effect_phase;
                result.spell_effect_visible_trace[frame] =
                    effect_heartbeat->spell_effect_visible;
                if (effect_heartbeat->spell_effect_visible) {
                    ++result.spell_effect_visible_heartbeats;
                }
                if (effect_heartbeat->spell_effect_completed) {
                    const auto expected_heartbeats = action_code == 1U
                        ? 12U
                        : action_code == 2U || action_code == 9U
                              ? 31U
                              : action_code == 5U ? 62U
                              : action_code == 12U ? 81U : 0U;
                    result.effect_completed_early =
                        expected_heartbeats == 0U ||
                        static_cast<std::uint16_t>(frame) + 1U !=
                            expected_heartbeats;
                    result.spell_effect_heartbeat_count =
                        static_cast<std::uint8_t>(frame + 1U);
                    result.completed_effect = effect_heartbeat;
                    break;
                }
            }
            context.phase = ScriptActorHeartbeatPhase::behavior;
            result.next_behavior = scripts.actor_heartbeat(context);
            if (action_code == 5U &&
                result.completed_effect.has_value() &&
                additional_scan_candidates != nullptr) {
                for (const auto& candidate : *additional_scan_candidates) {
                    if (!candidate.source_record_slot.has_value()) {
                        continue;
                    }
                    auto followup = actor_context(
                        "field_runner_k03_none00", candidate.id, 3U,
                        ScriptActorActionKind::none, 0U, 0U, true);
                    followup.spawn = true;
                    followup.phase = ScriptActorHeartbeatPhase::render;
                    followup.source_record_slot =
                        candidate.source_record_slot;
                    followup.movement_scene_id = 2U;
                    followup.world_scene_state = world_scene_state;
                    followup.ai_target_candidates =
                        effect_target_context.ai_target_candidates;
                    followup.combatant = candidate.combatant.value_or(
                        ScriptCombatantFacts{});
                    auto heartbeat = scripts.actor_heartbeat(followup);
                    if (!heartbeat.has_value()) {
                        return failed();
                    }
                    followup.spawn = false;
                    if (!heartbeat->spell_effect_visible) {
                        continue;
                    }
                    result.fanout_actor_ids.push_back(candidate.id);
                    std::uint8_t fanout_heartbeats = 1U;
                    for (std::uint8_t frame = 1U; frame < 96U; ++frame) {
                        heartbeat = scripts.actor_heartbeat(followup);
                        if (!heartbeat.has_value()) {
                            return failed();
                        }
                        ++fanout_heartbeats;
                        if (heartbeat->spell_effect_completed) {
                            break;
                        }
                    }
                    if (!heartbeat->spell_effect_completed) {
                        std::cerr << "spell fanout did not complete for "
                                  << candidate.id << '\n';
                        return failed();
                    }
                    result.fanout_heartbeat_counts.push_back(
                        fanout_heartbeats);
                    result.fanout_released_slot.push_back(
                        heartbeat->spell_effect_slot_released.has_value());
                }
            }
            return result;
        }
        std::cerr << "spell refresh missing for " << actor_id << '\n';
        return result;
    };
    const auto spell_cast_ok = [](const SpellRefresh& driven,
                                  const std::uint16_t selector) {
        const auto& cast = driven.refresh;
        const auto& held = driven.render_hold;
        const auto& next = driven.next_behavior;
        return cast.has_value() && held.has_value() && next.has_value() &&
               cast->source_action_dispatch.has_value() &&
               *cast->source_action_dispatch == "spell" &&
               cast->animation_id == "spell" &&
               cast->animation_frame == 0U &&
               cast->animation_selector == selector &&
               cast->direction == 1U &&
               cast->action_delay == 0x6A &&
               !cast->ai_target_engaged &&
               !cast->combat_action_requested &&
               !cast->combat_meter_update.has_value() &&
               !cast->audio_event.has_value() &&
               !cast->target_heading_audio_event.has_value() &&
               !cast->weapon_item_id.has_value() &&
               !cast->movement_step.has_value() &&
               !cast->movement_heading.has_value() &&
               held->animation_id == "spell" &&
               held->animation_frame == 0U &&
               held->animation_selector == selector &&
               !held->source_action_dispatch.has_value() &&
               !held->spell_effect_visible &&
               held->spell_effect_action_code == 0U &&
               next->animation_id == "spell" &&
               next->animation_frame == 0U &&
               next->action_delay == 0x69 &&
               !next->source_action_dispatch.has_value() &&
               !next->movement_step.has_value() &&
               !next->ai_target_engaged;
    };
    bool spell_line_called = false;
    std::uint16_t spell_line_x = 0U;
    std::uint16_t spell_line_y = 0U;
    const ScriptActorActionLineClear spell_line_true =
        [&spell_line_called, &spell_line_x, &spell_line_y](
            const std::uint16_t x, const std::uint16_t y) {
            spell_line_called = true;
            spell_line_x = x;
            spell_line_y = y;
            return true;
        };
    const ScriptActorActionLineClear spell_line_false =
        [](const std::uint16_t, const std::uint16_t) { return false; };
    const auto lonemage_cast = drive_spell_refresh(
        "lonemage_k0a_spell01", "spell_lonemage", 10U, 1U, 0, true,
        spell_line_true);
    spell_line_called = false;
    const auto gator_cast = drive_spell_refresh(
        "gator_shaman_k09_spell02", "spell_gator", 9U, 2U, 0, true,
        spell_line_true);
    spell_line_called = false;
    const auto rat_cast = drive_spell_refresh(
        "rat_shaman_k0b_spell05", "spell_rat", 11U, 5U, 0, true,
        spell_line_true);
    spell_line_called = false;
    const auto elven_cast = drive_spell_refresh(
        "elven_mage_k06_spell09", "spell_elven", 6U, 9U, 0, true,
        spell_line_true);
    const bool elven_line_hit_target =
        spell_line_called && spell_line_x == 0x1234U &&
        spell_line_y == 0x0ABCU;
    ScriptCombatantFacts spell_caster{};
    spell_caster.record_type = 7U;
    spell_caster.source_flags = 0x01U;
    spell_caster.physical_meter = 10U;
    spell_caster.mental_meter = 0U;
    spell_caster.posture = 4U;
    spell_caster.command_attribute_7c = 8U;
    spell_caster.item18_base = 8U;
    spell_caster.race_or_state = 4;
    spell_caster.equipped_action_code = 1U;
    spell_caster.action_phase_extension[9] = 20U;
    spell_caster.item_codes[0] = 6U;
    ScriptCombatantFacts spell_target{};
    spell_target.record_type = 0U;
    spell_target.source_flags = 0x01U;
    spell_target.interaction_flags = 0x80U;
    spell_target.command_attribute_77 = 8U;
    spell_target.physical_meter = 10U;
    spell_target.mental_meter = 10U;
    const auto resolved_spell_cast = drive_spell_refresh(
        "lonemage_k0a_spell01", "spell_action_resolved", 10U, 1U, 0,
        true, spell_line_true, &spell_caster, &spell_target);
    const auto flame_bolt_cast = drive_spell_refresh(
        "gator_shaman_k09_spell02", "spell_bolt_resolved", 9U, 2U, 0,
        true, spell_line_true, &spell_caster, &spell_target);
    const auto sleep_cast = drive_spell_refresh(
        "rat_shaman_k0b_spell05", "spell_sleep_resolved", 11U, 5U, 0,
        true, spell_line_true, &spell_caster, &spell_target);
    const auto mana_blast_cast = drive_spell_refresh(
        "elven_mage_k06_spell09", "spell_mana_blast_resolved", 6U, 9U, 0,
        true, spell_line_true, &spell_caster, &spell_target);
    ScriptCombatantFacts heal_lane_zero = spell_target;
    heal_lane_zero.physical_meter = 4U;
    ScriptCombatantFacts heal_lane_one = spell_target;
    heal_lane_one.physical_meter = 3U;
    heal_lane_one.mental_meter = 0U;
    ScriptCombatantFacts heal_lane_two = spell_target;
    heal_lane_two.physical_meter = 1U;
    std::vector<ScriptActorTargetCandidate> heal_lane_candidates;
    for (const auto [slot, id, combatant] : {
             std::tuple<std::uint8_t, const char*, ScriptCombatantFacts>{
                 1U, "party.runner.1", heal_lane_one},
             std::tuple<std::uint8_t, const char*, ScriptCombatantFacts>{
                 2U, "party.runner.2", heal_lane_two}}) {
        ScriptActorTargetCandidate candidate{};
        candidate.id = id;
        candidate.service_flags = 0x01U;
        candidate.has_action = true;
        candidate.world_activity_actor = false;
        candidate.source_record_slot = slot;
        candidate.combatant = combatant;
        heal_lane_candidates.push_back(std::move(candidate));
    }
    const auto heal_wounds_cast = drive_spell_refresh(
        "generic_k0a_spell0c", "spell_heal_wounds", 10U, 12U, 0,
        true, spell_line_true, &spell_caster, &heal_lane_zero, false, 0U,
        std::nullopt, &heal_lane_candidates);
    auto heal_candidates_with_absent_slot = heal_lane_candidates;
    heal_candidates_with_absent_slot.front().service_flags = 0U;
    const auto heal_skips_absent_lane = drive_spell_refresh(
        "generic_k0a_spell0c", "spell_heal_skips_absent_lane", 10U, 12U,
        0, true, spell_line_true, &spell_caster, &heal_lane_zero, false, 0U,
        std::nullopt, &heal_candidates_with_absent_slot);
    auto no_wounded_lanes = heal_lane_candidates;
    for (auto& candidate : no_wounded_lanes) {
        if (candidate.combatant.has_value()) {
            candidate.combatant->physical_meter = 4U;
        }
    }
    const auto heal_without_wounded_lane = drive_spell_refresh(
        "generic_k0a_spell0c", "spell_heal_no_wounded_lane", 10U, 12U,
        0, true, spell_line_true, &spell_caster, &heal_lane_zero, false, 0U,
        std::nullopt, &no_wounded_lanes);
    auto code2_spell_caster = spell_caster;
    code2_spell_caster.equipped_action_code = 2U;
    const auto code2_scene_state_skip = drive_spell_refresh(
        "gator_shaman_k09_spell02", "spell_code2_scene_skip", 9U, 2U, 0,
        true, spell_line_true, &code2_spell_caster, &spell_target, false, 3U,
        0x01U);
    const auto code2_scene_state_callback = drive_spell_refresh(
        "gator_shaman_k09_spell02", "spell_code2_scene_callback", 9U, 2U,
        0, true, spell_line_true, &code2_spell_caster, &spell_target, false,
        3U, 0x09U);
    std::vector<ScriptActorTargetCandidate> sleep_scan_candidates;
    const auto add_sleep_candidate = [&sleep_scan_candidates](
                                         const std::string& id,
                                         const std::uint8_t slot,
                                         const std::uint8_t flags,
                                         const std::uint16_t route_cost) {
        ScriptActorTargetCandidate candidate{};
        candidate.id = id;
        candidate.service_flags = flags;
        candidate.direct_route_step_cost = route_cost;
        candidate.source_record_slot = slot;
        sleep_scan_candidates.push_back(std::move(candidate));
    };
    add_sleep_candidate("sleep_near", 1U, 0x01U, 0x04FFU);
    add_sleep_candidate("sleep_at_range_limit", 2U, 0x01U, 0x0500U);
    add_sleep_candidate("sleep_inactive", 3U, 0x00U, 0x0100U);
    add_sleep_candidate("sleep_effect_locked", 4U, 0x41U, 0x0100U);
    add_sleep_candidate("sleep_near_world_actor", 5U, 0x01U, 0x0400U);
    auto sleep_spell_caster = spell_caster;
    sleep_spell_caster.equipped_action_code = 5U;
    const auto sleep_fanout_cast = drive_spell_refresh(
        "rat_shaman_k0b_spell05", "spell_sleep_fanout", 11U, 5U, 0,
        true, spell_line_true, &sleep_spell_caster, &spell_target, false, 0U,
        std::nullopt, &sleep_scan_candidates, true);
    const auto spell_effect_profile_matches = [](
        const SpellRefresh& driven, const std::uint8_t action_code,
        const std::uint8_t heartbeat_count, const std::uint8_t phase_count,
        const std::uint8_t visible_count) {
        const auto expected_audio_event = action_code == 1U
            ? std::optional<std::uint16_t>{0x0032U}
            : action_code == 2U
                  ? std::optional<std::uint16_t>{0x0034U}
            : action_code == 5U
                  ? std::optional<std::uint16_t>{0x0037U}
            : action_code == 9U
                  ? std::optional<std::uint16_t>{0x0035U}
            : action_code == 12U
                  ? std::optional<std::uint16_t>{0x003aU}
                  : std::nullopt;
        return driven.refresh.has_value() &&
               expected_audio_event.has_value() &&
               driven.refresh->audio_event == expected_audio_event &&
               driven.refresh->spell_action_resolution.has_value() &&
               driven.refresh->spell_action_resolution->effect_started &&
               driven.spell_effect_heartbeat_count == heartbeat_count &&
               driven.spell_effect_visible_heartbeats == visible_count &&
               !driven.effect_completed_early &&
               driven.completed_effect.has_value() &&
               driven.completed_effect->spell_effect_action_code ==
                   action_code &&
               driven.completed_effect->spell_effect_phase == phase_count &&
               driven.completed_effect->spell_effect_slot_released.has_value();
    };
    const auto spell_action_result = resolved_spell_cast.refresh.has_value()
        ? resolved_spell_cast.refresh->spell_action_resolution
        : std::nullopt;
    const bool spell_action_roll_resolved =
        resolved_spell_cast.refresh.has_value() &&
        spell_action_result.has_value() &&
        spell_action_result->action_code == 1U &&
        spell_action_result->target_id == "party.runner.0" &&
        spell_action_result->target_record_slot == 0U &&
        spell_action_result->target_record_type == 0U &&
        spell_action_result->roll_attempted &&
        spell_action_result->initial_trial_count.has_value() &&
        *spell_action_result->initial_trial_count == 8U &&
        spell_action_result->difficulty.has_value() &&
        *spell_action_result->difficulty == 6U &&
        spell_action_result->action_result == 8U &&
        spell_action_result->effect_requested &&
        resolved_spell_cast.refresh->audio_event == 0x0032U &&
        !spell_action_result->inventory_changed &&
        spell_action_result->item_codes[0] == 6U;
    const auto spell_effect_result =
        resolved_spell_cast.completed_effect.has_value()
            ? resolved_spell_cast.completed_effect->spell_effect_resolution
            : std::nullopt;
    const auto spell_target_update =
        resolved_spell_cast.completed_effect.has_value()
            ? resolved_spell_cast.completed_effect->combat_meter_update
            : std::nullopt;
    const std::array<std::uint8_t, 12> expected_spell_phase_trace{
        0U, 0U, 0U, 0U, 0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U};
    const std::array<bool, 12> expected_spell_visibility_trace{
        true, true, true, true, true, true, true, true, true, true, true,
        false};
    const bool delayed_spell_impact_applied =
        spell_action_result.has_value() &&
        spell_action_result->effect_started &&
        spell_action_result->spell_effect_slot ==
            std::optional<std::uint8_t>{0U} &&
        resolved_spell_cast.spell_effect_visible_heartbeats == 11U &&
        !resolved_spell_cast.effect_completed_early &&
        std::equal(expected_spell_phase_trace.begin(),
                   expected_spell_phase_trace.end(),
                   resolved_spell_cast.spell_effect_phase_trace.begin()) &&
        std::equal(expected_spell_visibility_trace.begin(),
                   expected_spell_visibility_trace.end(),
                   resolved_spell_cast.spell_effect_visible_trace.begin()) &&
        resolved_spell_cast.completed_effect.has_value() &&
        resolved_spell_cast.completed_effect->spell_effect_action_code == 1U &&
        resolved_spell_cast.completed_effect->spell_effect_slot_released ==
            std::optional<std::uint8_t>{0U} &&
        resolved_spell_cast.completed_effect->spell_effect_phase == 4U &&
        spell_effect_result.has_value() &&
        spell_effect_result->action_code == 1U &&
        spell_effect_result->target_id == "party.runner.0" &&
        spell_effect_result->target_trial_count == 8U &&
        spell_effect_result->difficulty == 2U &&
        spell_effect_result->target_roll_successes == 8U &&
        spell_effect_result->outcome == "effect_applied" &&
        spell_effect_result->physical && spell_effect_result->changed &&
        spell_effect_result->effect_value == 3 &&
        spell_effect_result->physical_meter == 7U &&
        spell_effect_result->mental_meter == spell_target.mental_meter &&
        spell_target_update.has_value() && spell_target_update->changed &&
        spell_target_update->physical &&
        spell_target_update->target_id == "party.runner.0" &&
        spell_target_update->physical_meter == spell_effect_result->physical_meter &&
        spell_target_update->mental_meter == spell_effect_result->mental_meter;
    const auto heal_action_result =
        heal_wounds_cast.refresh.has_value()
            ? heal_wounds_cast.refresh->spell_action_resolution
            : std::nullopt;
    const auto heal_effect_result =
        heal_wounds_cast.completed_effect.has_value()
            ? heal_wounds_cast.completed_effect->spell_effect_resolution
            : std::nullopt;
    const bool heal_wounds_source_lane_matches =
        heal_wounds_cast.refresh.has_value() &&
        heal_action_result.has_value() &&
        heal_action_result->action_code == 12U &&
        heal_action_result->target_id == "party.runner.1" &&
        heal_action_result->target_record_slot == 1U &&
        heal_effect_result.has_value() &&
        heal_effect_result->action_code == 12U &&
        heal_effect_result->target_id == "party.runner.1" &&
        heal_effect_result->target_record_slot == 1U &&
        heal_effect_result->outcome == "effect_applied" &&
        heal_effect_result->physical_meter > heal_lane_one.physical_meter &&
        heal_effect_result->mental_meter == heal_lane_one.mental_meter + 1U &&
        heal_effect_result->source_record_flags == 0x01U &&
        heal_wounds_cast.completed_effect->spell_effect_slot_released ==
            std::optional<std::uint8_t>{0U};
    const bool heal_lane_presence_and_empty_gate_match =
        heal_skips_absent_lane.refresh.has_value() &&
        heal_skips_absent_lane.refresh->spell_action_resolution.has_value() &&
        heal_skips_absent_lane.refresh->spell_action_resolution->target_id ==
            "party.runner.2" &&
        heal_skips_absent_lane.refresh->spell_action_resolution->target_record_slot ==
            2U &&
        heal_without_wounded_lane.refresh.has_value() &&
        heal_without_wounded_lane.refresh->source_action_dispatch ==
            std::optional<std::string>{"suppressed"} &&
        !heal_without_wounded_lane.refresh->spell_action_resolution.has_value() &&
        !heal_without_wounded_lane.refresh->audio_event.has_value() &&
        !heal_without_wounded_lane.completed_effect.has_value();
    const bool code2_scene_callback_gate_matches =
        code2_scene_state_skip.completed_effect.has_value() &&
        code2_scene_state_skip.completed_effect->spell_effect_completed &&
        code2_scene_state_skip.completed_effect->spell_effect_slot_released ==
            std::optional<std::uint8_t>{0U} &&
        !code2_scene_state_skip.completed_effect->spell_effect_resolution &&
        !code2_scene_state_skip.completed_effect->combat_meter_update &&
        code2_scene_state_callback.completed_effect.has_value() &&
        code2_scene_state_callback.completed_effect->spell_effect_completed &&
        code2_scene_state_callback.completed_effect->spell_effect_resolution &&
        code2_scene_state_callback.completed_effect->spell_effect_resolution->
                action_code == 2U;
    const bool sleep_scan_fanout_matches =
        sleep_fanout_cast.completed_effect.has_value() &&
        sleep_fanout_cast.completed_effect->spell_effect_completed &&
        sleep_fanout_cast.fanout_actor_ids ==
            std::vector<std::string>{"sleep_near", "sleep_near_world_actor"} &&
        sleep_fanout_cast.fanout_heartbeat_counts ==
            std::vector<std::uint8_t>{62U, 62U} &&
        sleep_fanout_cast.fanout_released_slot ==
            std::vector<bool>{false, false};
    const auto full_effect_pool_cast = drive_spell_refresh(
        "lonemage_k0a_spell01", "spell_effect_pool_full", 10U, 1U, 0,
        true, spell_line_true, &spell_caster, &spell_target, true);
    const auto full_effect_pool_result =
        full_effect_pool_cast.refresh.has_value()
            ? full_effect_pool_cast.refresh->spell_action_resolution
            : std::nullopt;
    const bool full_spell_effect_pool_blocks_spawn =
        full_effect_pool_result.has_value() &&
        full_effect_pool_result->roll_attempted &&
        full_effect_pool_result->action_result == 8U &&
        full_effect_pool_result->effect_requested &&
        !full_effect_pool_result->effect_started &&
        !full_effect_pool_result->spell_effect_slot.has_value() &&
        !full_effect_pool_cast.refresh->audio_event.has_value() &&
        !full_effect_pool_cast.completed_effect.has_value() &&
        full_effect_pool_cast.spell_effect_visible_heartbeats == 0U;
    auto gated_spell_caster = spell_caster;
    gated_spell_caster.mental_meter = 1U;
    const auto gated_spell_cast = drive_spell_refresh(
        "lonemage_k0a_spell01", "spell_action_gated", 10U, 1U, 0,
        true, spell_line_true, &gated_spell_caster, &spell_target);
    const auto gated_spell_result = gated_spell_cast.refresh.has_value()
        ? gated_spell_cast.refresh->spell_action_resolution
        : std::nullopt;
    const bool source_spell_gate_switches_action =
        gated_spell_result.has_value() &&
        gated_spell_result->outcome == "target_state_gate" &&
        !gated_spell_result->roll_attempted &&
        gated_spell_result->next_action_kind == ScriptActorActionKind::item &&
        gated_spell_result->next_action_code == 6U &&
        gated_spell_result->next_profile_id ==
            std::optional<std::string>{"lonemage_k0a_item06"} &&
        gated_spell_cast.render_hold.has_value() &&
        gated_spell_cast.next_behavior.has_value();
    if (!spell_action_roll_resolved || !source_spell_gate_switches_action) {
        const auto dump_spell_resolution = [](const char* name,
                                              const SpellRefresh& driven) {
            std::cerr << name;
            if (!driven.refresh.has_value()) {
                std::cerr << " missing refresh\n";
                return;
            }
            std::cerr << " dispatch="
                      << driven.refresh->source_action_dispatch.value_or("nil")
                      << " animation=" << driven.refresh->animation_id
                      << " render=" << driven.render_hold.has_value()
                      << " next=" << driven.next_behavior.has_value();
            if (driven.refresh->spell_action_resolution.has_value()) {
                const auto& value =
                    *driven.refresh->spell_action_resolution;
                std::cerr << " outcome=" << value.outcome
                          << " action_code="
                          << static_cast<unsigned>(value.action_code)
                          << " rolls=" << value.roll_attempted
                          << " trials="
                          << (value.initial_trial_count.has_value()
                                  ? std::to_string(*value.initial_trial_count)
                                  : "nil")
                          << " difficulty="
                          << (value.difficulty.has_value()
                                  ? std::to_string(*value.difficulty)
                                  : "nil")
                          << " successes="
                          << static_cast<unsigned>(value.action_result)
                          << " effect=" << value.effect_requested
                          << " target=" << value.target_id
                          << " slot="
                          << static_cast<unsigned>(value.target_record_slot)
                          << " type=" << value.target_record_type;
                if (value.next_action_kind.has_value()) {
                    std::cerr << " next_kind="
                              << static_cast<unsigned>(
                                     *value.next_action_kind)
                              << " next_code="
                              << static_cast<unsigned>(
                                     value.next_action_code.value_or(0U))
                              << " next_profile="
                              << value.next_profile_id.value_or("nil");
                }
                std::cerr << " inventory=" << value.inventory_changed
                          << " slot1="
                          << static_cast<unsigned>(value.item_codes[0]);
            } else {
                std::cerr << " missing typed resolution";
            }
            std::cerr << " audio="
                      << (driven.refresh->audio_event.has_value()
                              ? std::to_string(*driven.refresh->audio_event)
                              : "nil") << '\n';
            if (!driven.render_hold.has_value() ||
                !driven.next_behavior.has_value()) {
                std::cerr << name << " did not resume its subsequent heartbeat\n";
            }
        };
        dump_spell_resolution("resolved spell", resolved_spell_cast);
        dump_spell_resolution("gated spell", gated_spell_cast);
    }
    bool waiting_line_called = false;
    const auto waiting_cast = drive_spell_refresh(
        "lonemage_k0a_spell01", "spell_waiting", 10U, 1U, 400, true,
        [&waiting_line_called](const std::uint16_t, const std::uint16_t) {
            waiting_line_called = true;
            return true;
        });
    bool suppressed_line_called = false;
    const auto invisibility = drive_spell_refresh(
        "rat_shaman_k0b_spell04", "spell_invisibility", 11U, 4U, 0, true,
        [&suppressed_line_called](const std::uint16_t, const std::uint16_t) {
            suppressed_line_called = true;
            return true;
        });
    const auto rockskin = drive_spell_refresh(
        "rat_shaman_k0b_spell0b", "spell_rockskin", 11U, 11U, 0, true,
        spell_line_true);
    bool offscreen_line_called = false;
    const auto offscreen = drive_spell_refresh(
        "lonemage_k0a_spell01", "spell_offscreen", 10U, 1U, 0, false,
        [&offscreen_line_called](const std::uint16_t, const std::uint16_t) {
            offscreen_line_called = true;
            return true;
        });
    const auto unmeasured = drive_spell_refresh(
        "lonemage_k0a_spell01", "spell_unmeasured", 10U, 1U, 0, std::nullopt,
        spell_line_true);
    const auto blocked_missing = drive_spell_refresh(
        "gator_shaman_k09_spell02", "spell_blocked_missing", 9U, 2U, 0, true,
        {});
    const auto blocked_terrain = drive_spell_refresh(
        "elven_mage_k06_spell09", "spell_blocked_terrain", 6U, 9U, 0, true,
        spell_line_false);
    const auto spell_gate_contract =
        spell_cast_ok(lonemage_cast, 0x0084U) &&
        spell_cast_ok(gator_cast, 0x0084U) &&
        spell_cast_ok(rat_cast, 0x0084U) &&
        spell_cast_ok(elven_cast, 0x0084U) && elven_line_hit_target &&
        spell_action_roll_resolved && delayed_spell_impact_applied &&
        spell_effect_profile_matches(resolved_spell_cast, 1U, 12U, 4U,
                                     11U) &&
        spell_effect_profile_matches(flame_bolt_cast, 2U, 31U, 6U, 30U) &&
        spell_effect_profile_matches(sleep_cast, 5U, 62U, 9U, 61U) &&
        spell_effect_profile_matches(mana_blast_cast, 9U, 31U, 6U, 30U) &&
        spell_effect_profile_matches(heal_wounds_cast, 12U, 81U, 16U,
                                     80U) &&
        heal_wounds_source_lane_matches &&
        heal_lane_presence_and_empty_gate_match &&
        code2_scene_callback_gate_matches && sleep_scan_fanout_matches &&
        full_spell_effect_pool_blocks_spawn &&
        source_spell_gate_switches_action &&
        waiting_cast.refresh.has_value() &&
        waiting_cast.refresh->source_action_dispatch.has_value() &&
        *waiting_cast.refresh->source_action_dispatch == "waiting" &&
        waiting_cast.refresh->animation_id != "spell" &&
        waiting_cast.refresh->action_delay >= 0 &&
        !waiting_cast.refresh->combat_meter_update.has_value() &&
        !waiting_line_called &&
        invisibility.refresh.has_value() &&
        invisibility.refresh->source_action_dispatch.has_value() &&
        *invisibility.refresh->source_action_dispatch == "suppressed" &&
        invisibility.refresh->animation_id != "spell" &&
        !invisibility.refresh->ai_target_engaged &&
        !suppressed_line_called &&
        rockskin.refresh.has_value() &&
        rockskin.refresh->source_action_dispatch.has_value() &&
        *rockskin.refresh->source_action_dispatch == "suppressed" &&
        rockskin.refresh->animation_id != "spell" &&
        offscreen.refresh.has_value() &&
        offscreen.refresh->source_action_dispatch.has_value() &&
        *offscreen.refresh->source_action_dispatch == "offscreen" &&
        offscreen.refresh->animation_id != "spell" &&
        !offscreen_line_called &&
        unmeasured.refresh.has_value() &&
        unmeasured.refresh->source_action_dispatch.has_value() &&
        *unmeasured.refresh->source_action_dispatch == "offscreen" &&
        blocked_missing.refresh.has_value() &&
        blocked_missing.refresh->source_action_dispatch.has_value() &&
        *blocked_missing.refresh->source_action_dispatch == "blocked" &&
        blocked_missing.refresh->animation_id != "spell" &&
        blocked_terrain.refresh.has_value() &&
        blocked_terrain.refresh->source_action_dispatch.has_value() &&
        *blocked_terrain.refresh->source_action_dispatch == "blocked" &&
        blocked_terrain.refresh->animation_id != "spell" &&
        !blocked_terrain.refresh->combat_action_requested &&
        !blocked_terrain.refresh->weapon_item_id.has_value();
    if (!expect(spell_gate_contract,
                "Lua spell dispatch failed the 0x13FFE gates or the live "
                "0x1033E roll, target, effect, and audio result")) {
        std::cerr << "spell gates: action=" << spell_action_roll_resolved
                  << " delayed=" << delayed_spell_impact_applied
                  << " pool=" << full_spell_effect_pool_blocks_spawn
                  << " casts=" << spell_cast_ok(lonemage_cast, 0x0084U)
                  << spell_cast_ok(gator_cast, 0x0084U)
                  << spell_cast_ok(rat_cast, 0x0084U)
                  << spell_cast_ok(elven_cast, 0x0084U)
                  << " visible=" << static_cast<unsigned>(
                         resolved_spell_cast.spell_effect_visible_heartbeats)
                  << " phase=" << static_cast<unsigned>(
                         resolved_spell_cast.completed_effect.has_value()
                             ? resolved_spell_cast.completed_effect->spell_effect_phase
                             : 0U)
                  << " release=" << (resolved_spell_cast.completed_effect.has_value() &&
                         resolved_spell_cast.completed_effect->spell_effect_slot_released.has_value())
                  << " result=" << spell_effect_result.has_value()
                  << " code2=" << code2_scene_callback_gate_matches
                  << " heal_profile=" << spell_effect_profile_matches(
                         heal_wounds_cast, 12U, 81U, 16U, 80U)
                  << " heal_lane=" << heal_wounds_source_lane_matches
                  << " heal_presence=" << heal_lane_presence_and_empty_gate_match
                  << " heal_dispatch=" << (heal_action_result.has_value()
                         ? heal_wounds_cast.refresh->source_action_dispatch.value_or("nil")
                         : "missing")
                  << " heal_heartbeats=" << static_cast<unsigned>(
                         heal_wounds_cast.spell_effect_heartbeat_count)
                  << " heal_visible=" << static_cast<unsigned>(
                         heal_wounds_cast.spell_effect_visible_heartbeats)
                  << " heal_slot=" << (heal_action_result.has_value() &&
                         heal_action_result->spell_effect_slot.has_value())
                  << " heal_target=" << (heal_action_result.has_value()
                         ? heal_action_result->target_id : "missing")
                  << " heal_outcome=" << (heal_effect_result.has_value()
                         ? heal_effect_result->outcome : "missing")
                  << " code2skip=" << (code2_scene_state_skip.completed_effect.has_value() &&
                         code2_scene_state_skip.completed_effect->spell_effect_resolution.has_value())
                  << " code2callback=" << (code2_scene_state_callback.completed_effect.has_value() &&
                         code2_scene_state_callback.completed_effect->spell_effect_resolution.has_value())
                  << " sleepfan=" << sleep_scan_fanout_matches
                  << " fanout=" << sleep_fanout_cast.fanout_actor_ids.size()
                  << (spell_effect_result.has_value()
                          ? " outcome=" + spell_effect_result->outcome
                          : std::string{})
                  << '\n';
        std::cerr << scripts.failure() << '\n';
        return 30;
    }

    const auto drive_item_refresh = [&scripts, &actor_context](
                                        const std::string& profile_id,
                                        const std::string& actor_id,
                                        const std::uint8_t action_code,
                                        const std::int16_t initial_delay,
                                        const std::optional<bool> projection,
                                        const ScriptActorActionLineClear& line,
                                        const std::uint16_t distance,
                                        const bool source_heading_available)
        -> SpellRefresh {
        SpellRefresh result;
        auto context = actor_context(
            profile_id, actor_id, 7U, ScriptActorActionKind::item,
            action_code, 0U, true);
        context.dynamic_world_encounter_actor = true;
        context.source_record_flags = 0x01U;
        context.source_action_slot = 0U;
        context.source_action_ammo = 10U;
        context.source_action_counter = 0U;
        // The Lua route policy opens +0xB0.4 for a close target; the host
        // begins with the ROM record's clear flag and validates the decision.
        context.source_behavior_flags = 0U;
        context.source_animation_selector = 0U;
        context.entry_animation_frame_count = 1U;
        context.walk_animation_frame_count = 4U;
        context.initial_action_delay = initial_delay;
        context.movement_allowed = true;
        context.source_projection_active = projection;
        context.action_line_clear = line;
        context.ai_random_bound =
            [](const std::uint16_t bound) -> std::optional<std::uint16_t> {
                return bound == 0U ? std::nullopt
                                   : std::optional<std::uint16_t>{0U};
            };
        context.ai_target_candidates.push_back(ScriptActorTargetCandidate{
            "party.runner.0", 0x1234U, 0x0ABCU, 0x2000U, distance, 0x01U, 0U,
            0U, 0U, true, false, distance, distance, distance,
            static_cast<std::uint16_t>(distance * 2U), 0U});
        if (source_heading_available) {
            context.ai_target_candidates.back().source_record_heading = 0U;
        }
        const auto failed = [&scripts, &result]() {
            std::cerr << scripts.failure() << '\n';
            return result;
        };
        if (!scripts.actor_heartbeat(context).has_value()) {
            return failed();
        }
        context.spawn = false;
        if (!scripts.actor_heartbeat(context).has_value()) {
            return failed();
        }
        context.phase = ScriptActorHeartbeatPhase::render;
        for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
            if (!scripts.actor_heartbeat(context).has_value()) {
                return failed();
            }
        }
        context.phase = ScriptActorHeartbeatPhase::behavior;
        for (std::uint8_t pass = 0U; pass < 120U; ++pass) {
            const auto decision = scripts.actor_heartbeat(context);
            if (!decision.has_value()) {
                return failed();
            }
            if (!decision->source_action_dispatch.has_value()) {
                continue;
            }
            result.refresh = decision;
            return result;
        }
        std::cerr << "item refresh missing for " << actor_id << '\n';
        return result;
    };
    const auto item_request_ok = [](const SpellRefresh& driven,
                                    const char* label,
                                    const std::uint8_t weapon_id) {
        const auto& refresh = driven.refresh;
        return refresh.has_value() &&
               refresh->source_action_dispatch.has_value() &&
               *refresh->source_action_dispatch == label &&
               refresh->animation_id != "action" &&
               refresh->animation_id != "spell" &&
               !refresh->ai_target_engaged &&
               !refresh->combat_action_requested &&
               !refresh->combat_meter_update.has_value() &&
               !refresh->audio_event.has_value() &&
               refresh->weapon_item_id == weapon_id;
    };
    bool item_line_called = false;
    const ScriptActorActionLineClear item_line_true =
        [&item_line_called](const std::uint16_t x, const std::uint16_t y) {
            item_line_called = x == 0x1234U && y == 0x0ABCU;
            return true;
        };
    const auto item_spawn_request = drive_item_refresh(
        "eye5r_1_k07_item01", "item_spawn_request", 1U, 0, true,
        item_line_true, 0x0C00U, true);
    const bool item_spawn_line = item_line_called;
    item_line_called = false;
    const auto item_close_request = drive_item_refresh(
        "eye5r_1_k07_item01", "item_close_request", 1U, 0, true,
        item_line_true, 0x0100U, true);
    const bool item_close_line = item_line_called;
    item_line_called = false;
    const auto frag_request = drive_item_refresh(
        "grenade_k07_item0d", "item_frag", 13U, 0, true, item_line_true,
        0x0C00U, true);
    const bool frag_line = item_line_called;
    item_line_called = false;
    const auto concussion_request = drive_item_refresh(
        "grenade_k07_item0f", "item_concussion", 15U, 0, true, item_line_true,
        0x0C00U, true);
    item_line_called = false;
    const auto medkit_request = drive_item_refresh(
        "kit_k07_item10", "item_medkit", 16U, 0, true, item_line_true,
        0x0C00U, true);
    const bool medkit_line = item_line_called;
    bool item_waiting_line = false;
    const auto item_waiting = drive_item_refresh(
        "eye5r_1_k07_item01", "item_waiting", 1U, 400, true,
        [&item_waiting_line](const std::uint16_t, const std::uint16_t) {
            item_waiting_line = true;
            return true;
        },
        0x0C00U, true);
    bool item_offscreen_line = false;
    const auto item_offscreen = drive_item_refresh(
        "eye5r_1_k07_item01", "item_offscreen", 1U, 0, false,
        [&item_offscreen_line](const std::uint16_t, const std::uint16_t) {
            item_offscreen_line = true;
            return true;
        },
        0x0C00U, true);
    const auto item_blocked_missing = drive_item_refresh(
        "eye5r_1_k07_item01", "item_blocked_missing", 1U, 0, true, {},
        0x0C00U, true);
    const auto item_blocked_terrain = drive_item_refresh(
        "grenade_k07_item0d", "item_blocked_terrain", 13U, 0, true,
        [](const std::uint16_t, const std::uint16_t) { return false; },
        0x0C00U, true);
    const auto item_heading_unresolved = drive_item_refresh(
        "eye5r_1_k07_item01", "item_heading_unresolved", 1U, 0, true,
        item_line_true, 0x0100U, false);
    const auto item_gate_contract =
        item_request_ok(item_spawn_request, "item_gated", 1U) &&
        item_spawn_line &&
        item_request_ok(item_close_request, "item_spawn_requested", 1U) &&
        (item_close_request.refresh->source_behavior_flags & 0x10U) != 0U &&
        item_close_line &&
        item_request_ok(frag_request, "item_effect", 13U) && frag_line &&
        item_request_ok(concussion_request, "item_effect", 15U) &&
        item_request_ok(medkit_request, "item_gated", 16U) && medkit_line &&
        item_waiting.refresh.has_value() &&
        item_waiting.refresh->source_action_dispatch.has_value() &&
        *item_waiting.refresh->source_action_dispatch == "waiting" &&
        item_waiting.refresh->action_delay >= 0 &&
        !item_waiting.refresh->combat_meter_update.has_value() &&
        item_waiting.refresh->weapon_item_id == 1U &&
        !item_waiting_line &&
        item_offscreen.refresh.has_value() &&
        item_offscreen.refresh->source_action_dispatch.has_value() &&
        *item_offscreen.refresh->source_action_dispatch == "offscreen" &&
        !item_offscreen_line &&
        item_blocked_missing.refresh.has_value() &&
        item_blocked_missing.refresh->source_action_dispatch.has_value() &&
        *item_blocked_missing.refresh->source_action_dispatch == "blocked" &&
        item_blocked_terrain.refresh.has_value() &&
        item_blocked_terrain.refresh->source_action_dispatch.has_value() &&
        *item_blocked_terrain.refresh->source_action_dispatch == "blocked" &&
        item_heading_unresolved.refresh.has_value() &&
        item_heading_unresolved.refresh->source_action_dispatch.has_value() &&
        *item_heading_unresolved.refresh->source_action_dispatch ==
            "item_target_heading_unresolved" &&
        !item_blocked_terrain.refresh->combat_meter_update.has_value() &&
        item_blocked_terrain.refresh->weapon_item_id == 13U &&
        item_blocked_terrain.refresh->animation_id != "action";
    const auto drive_item_pose = [&scripts, &actor_context](
                                     const std::string_view actor_id,
                                     const ScriptActorItemSpawnResult result)
        -> std::optional<ScriptActorBehaviorDecision> {
        auto context = actor_context(
            "eye5r_1_k07_item01", actor_id, 7U,
            ScriptActorActionKind::item, 1U, 0U, true);
        context.dynamic_world_encounter_actor = true;
        context.source_record_flags = 0x01U;
        context.source_action_slot = 0U;
        context.source_behavior_flags = 0x10U;
        context.source_action_ammo = 10U;
        context.source_action_counter = 0U;
        context.source_animation_selector = 0U;
        context.entry_animation_frame_count = 1U;
        context.walk_animation_frame_count = 4U;
        context.item_action_frame_count = 1U;
        context.item_action_increment = 0x0080U;
        context.item_action_threshold = 0x1000U;
        context.initial_action_delay = 0;
        context.movement_allowed = true;
        context.source_projection_active = true;
        context.action_line_clear = [](const std::uint16_t,
                                       const std::uint16_t) {
            return true;
        };
        context.ai_random_bound =
            [](const std::uint16_t bound) -> std::optional<std::uint16_t> {
                return bound == 0U ? std::nullopt
                                   : std::optional<std::uint16_t>{0U};
            };
        context.ai_target_candidates.push_back(ScriptActorTargetCandidate{
            "party.runner.0", 0x1234U, 0x0ABCU, 0x2000U, 0x0100U,
            0x01U, 0U, 0U, 0U, true, false, 0x0100U, 0x0100U, 0x0100U,
            0x0100U, 0U});
        context.ai_target_candidates.back().source_record_heading = 0U;
        if (!scripts.actor_heartbeat(context).has_value()) {
            std::cerr << "item pose initial spawn failed: " << scripts.failure() << '\n';
            return std::nullopt;
        }
        context.spawn = false;
        if (!scripts.actor_heartbeat(context).has_value()) {
            std::cerr << "item pose entry behavior failed: " << scripts.failure() << '\n';
            return std::nullopt;
        }
        context.phase = ScriptActorHeartbeatPhase::render;
        for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
            if (!scripts.actor_heartbeat(context).has_value()) {
                std::cerr << "item pose entry render failed: " << scripts.failure() << '\n';
                return std::nullopt;
            }
        }
        context.phase = ScriptActorHeartbeatPhase::behavior;
        bool request_seen = false;
        std::string last_item_dispatch = "<none>";
        std::string last_target = "<none>";
        std::uint16_t last_selector = 0U;
        std::uint8_t last_flags = 0U;
        for (std::uint16_t pass = 0U; pass < 120U; ++pass) {
            const auto decision = scripts.actor_heartbeat(context);
            if (!decision.has_value()) {
                std::cerr << "item pose behavior failed: " << scripts.failure() << '\n';
                return std::nullopt;
            }
            last_item_dispatch = decision->source_action_dispatch.value_or("<none>");
            last_target = decision->ai_target_id.value_or("<none>");
            last_selector = decision->source_animation_selector;
            last_flags = decision->source_behavior_flags;
            if (decision->source_action_dispatch ==
                std::optional<std::string>{"item_spawn_requested"}) {
                request_seen = true;
                break;
            }
        }
        if (!request_seen) {
            std::cerr << "item pose request missing: dispatch="
                      << last_item_dispatch << " target=" << last_target
                      << " selector=0x" << std::hex << last_selector
                      << " flags=0x" << static_cast<unsigned>(last_flags)
                      << std::dec << " ammo="
                      << static_cast<unsigned>(context.source_action_ammo)
                      << " distance=" << context.ai_target_candidates[0].distance
                      << "\n";
            return std::nullopt;
        }

        context.phase = ScriptActorHeartbeatPhase::render;
        context.source_item_spawn_result = result;
        auto decision = scripts.actor_heartbeat(context);
        context.source_item_spawn_result.reset();
        if (!decision.has_value()) {
            std::cerr << "item pose result render failed: " << scripts.failure() << '\n';
            return std::nullopt;
        }
        context.source_animation_selector =
            decision->source_animation_selector;
        context.source_behavior_flags = decision->source_behavior_flags;
        if (result == ScriptActorItemSpawnResult::spawned) {
            for (std::uint8_t frame = 0U; frame < 32U; ++frame) {
                decision = scripts.actor_heartbeat(context);
                if (!decision.has_value()) {
                    std::cerr << "item pose animation render failed: " << scripts.failure() << '\n';
                    return std::nullopt;
                }
                context.source_animation_selector =
                    decision->source_animation_selector;
                context.source_behavior_flags =
                    decision->source_behavior_flags;
            }
        }
        return decision;
    };
    const auto item_pose_spawned = drive_item_pose(
        "item_pose_spawned", ScriptActorItemSpawnResult::spawned);
    const auto item_pose_blocked = drive_item_pose(
        "item_pose_blocked", ScriptActorItemSpawnResult::blocked);
    const bool item_pose_contract = item_pose_spawned.has_value() &&
        item_pose_spawned->animation_id == "weapon" &&
        item_pose_spawned->animation_frame == 0U &&
        item_pose_spawned->source_animation_selector == 0x8040U &&
        item_pose_blocked.has_value() &&
        item_pose_blocked->animation_id != "weapon" &&
        item_pose_blocked->source_animation_selector <= 0x003CU;
    if (!expect(item_gate_contract,
                "Lua item dispatch failed the 0x13FFE source action gates") ||
        !expect(item_pose_contract,
                "Lua did not start a C042 pose only after the host spawned a projectile")) {
        const auto describe_item = [](const SpellRefresh& value) {
            return value.refresh.has_value() &&
                           value.refresh->source_action_dispatch.has_value()
                ? *value.refresh->source_action_dispatch
                : std::string{"<none>"};
        };
        std::cerr << "item dispatches=" << describe_item(item_spawn_request)
                  << ',' << describe_item(item_close_request) << ','
                  << describe_item(frag_request) << ','
                  << describe_item(concussion_request) << ','
                  << describe_item(medkit_request) << ','
                  << describe_item(item_waiting) << ','
                  << describe_item(item_offscreen) << ','
                  << describe_item(item_blocked_missing) << ','
                  << describe_item(item_blocked_terrain)
                  << " lines=" << item_spawn_line << item_close_line
                  << frag_line << medkit_line << item_waiting_line
                  << item_offscreen_line << '\n';
        const auto describe_pose = [](const auto& value) {
            return value.has_value()
                ? value->animation_id + ":" +
                      std::to_string(value->animation_frame) + ":" +
                      std::to_string(value->source_animation_selector)
                : std::string{"<none>"};
        };
        std::cerr << "item poses=" << describe_pose(item_pose_spawned)
                  << ',' << describe_pose(item_pose_blocked) << '\n';
        std::cerr << scripts.failure() << '\n';
        return 31;
    }

    // Unique profile_id values in dynamic-world-activity-actors.tsv.
    // The heartbeat result is the assert; the file check only keeps this
    // list on the catalog.
    enum class CatalogDecision {
        spell,
        item_gated,
        unarmed,
        hound_effect,
    };
    struct CatalogProfile {
        const char* profile_id;
        std::uint16_t entity_kind;
        ScriptActorActionKind action_kind;
        std::uint8_t action_code;
        CatalogDecision decision;
    };
    constexpr CatalogProfile catalog_profiles[] = {
        {"hell_hound_k0c_none00", 12U, ScriptActorActionKind::none, 0U,
         CatalogDecision::hound_effect},
        {"vampire_k09_none00", 9U, ScriptActorActionKind::none, 0U,
         CatalogDecision::unarmed},
        {"eye5r_2_k07_item04", 7U, ScriptActorActionKind::item, 4U,
         CatalogDecision::item_gated},
        {"eye5r_1_k07_item01", 7U, ScriptActorActionKind::item, 1U,
         CatalogDecision::item_gated},
        {"lone11_k05_item03", 5U, ScriptActorActionKind::item, 3U,
         CatalogDecision::item_gated},
        {"lonemage_k0a_spell01", 10U, ScriptActorActionKind::spell, 1U,
         CatalogDecision::spell},
        {"ork_ganger_k04_none00", 4U, ScriptActorActionKind::none, 0U,
         CatalogDecision::unarmed},
        {"rat_shaman_k0b_spell05", 11U, ScriptActorActionKind::spell, 5U,
         CatalogDecision::spell},
        {"indian_warior_k0b_item05", 11U, ScriptActorActionKind::item, 5U,
         CatalogDecision::item_gated},
        {"wendigo_k0e_none00", 14U, ScriptActorActionKind::none, 0U,
         CatalogDecision::unarmed},
        {"bandersnatch_k04_none00", 4U, ScriptActorActionKind::none, 0U,
         CatalogDecision::unarmed},
        {"weenr1_k08_none00", 8U, ScriptActorActionKind::none, 0U,
         CatalogDecision::unarmed},
        {"weenr2_k08_item01", 8U, ScriptActorActionKind::item, 1U,
         CatalogDecision::item_gated},
        {"elven_mage_k06_spell09", 6U, ScriptActorActionKind::spell, 9U,
         CatalogDecision::spell},
        {"gator_shaman_k09_spell02", 9U, ScriptActorActionKind::spell, 2U,
         CatalogDecision::spell},
        {"gargoyle_k0d_none00", 13U, ScriptActorActionKind::none, 0U,
         CatalogDecision::unarmed},
        {"guard1_k05_item03", 5U, ScriptActorActionKind::item, 3U,
         CatalogDecision::item_gated},
        {"lone13_k05_item0a", 5U, ScriptActorActionKind::item, 10U,
         CatalogDecision::item_gated},
        {"ghoul1_k07_none00", 7U, ScriptActorActionKind::none, 0U,
         CatalogDecision::unarmed},
    };
    constexpr std::size_t catalog_profile_count =
        sizeof(catalog_profiles) / sizeof(catalog_profiles[0]);
    const auto catalog_path =
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT) /
        "field/entities/dynamic-world-activity-actors.tsv";
    std::ifstream catalog_file(catalog_path);
    std::string catalog_line;
    struct CatalogSeen {
        std::string profile_id;
        std::string action_kind;
        unsigned entity_kind = 0;
        unsigned action_code = 0;
    };
    std::vector<CatalogSeen> catalog_seen;
    bool catalog_file_ok = static_cast<bool>(catalog_file);
    if (catalog_file_ok) {
        std::getline(catalog_file, catalog_line);
    }
    const auto parse_unsigned =
        [](const std::string& text, unsigned& value) {
            if (text.empty()) {
                return false;
            }
            unsigned parsed = 0U;
            for (const char character : text) {
                if (character < '0' || character > '9') {
                    return false;
                }
                parsed = parsed * 10U +
                         static_cast<unsigned>(character - '0');
            }
            value = parsed;
            return true;
        };
    while (catalog_file_ok && std::getline(catalog_file, catalog_line)) {
        if (!catalog_line.empty() && catalog_line.back() == '\r') {
            catalog_line.pop_back();
        }
        if (catalog_line.empty()) {
            continue;
        }
        std::vector<std::string> columns;
        std::string cell;
        for (const char character : catalog_line) {
            if (character == '\t') {
                columns.push_back(cell);
                cell.clear();
            } else {
                cell.push_back(character);
            }
        }
        columns.push_back(cell);
        if (columns.size() < 12U) {
            catalog_file_ok = false;
            break;
        }
        unsigned entity_kind = 0U;
        unsigned action_code = 0U;
        if (!parse_unsigned(columns[11], entity_kind) ||
            !parse_unsigned(columns[9], action_code)) {
            catalog_file_ok = false;
            break;
        }
        const auto existing = std::find_if(
            catalog_seen.begin(), catalog_seen.end(),
            [&columns](const CatalogSeen& seen) {
                return seen.profile_id == columns[7];
            });
        if (existing == catalog_seen.end()) {
            catalog_seen.push_back(CatalogSeen{
                columns[7], columns[8], entity_kind, action_code});
        } else if (existing->action_kind != columns[8] ||
                   existing->entity_kind != entity_kind ||
                   existing->action_code != action_code) {
            catalog_file_ok = false;
            break;
        }
    }
    const auto catalog_row_for =
        [&catalog_profiles](const std::string& profile_id)
        -> const CatalogProfile* {
        for (const auto& row : catalog_profiles) {
            if (profile_id == row.profile_id) {
                return &row;
            }
        }
        return nullptr;
    };
    const auto action_name = [](const ScriptActorActionKind kind) {
        if (kind == ScriptActorActionKind::item) {
            return "item";
        }
        if (kind == ScriptActorActionKind::spell) {
            return "spell";
        }
        return "none";
    };
    bool catalog_ids_match =
        catalog_file_ok && catalog_seen.size() == catalog_profile_count &&
        catalog_profile_count == 19U;
    if (catalog_ids_match) {
        for (const auto& seen : catalog_seen) {
            const auto* row = catalog_row_for(seen.profile_id);
            if (row == nullptr ||
                action_name(row->action_kind) != seen.action_kind ||
                row->entity_kind != seen.entity_kind ||
                row->action_code != seen.action_code) {
                catalog_ids_match = false;
                std::cerr << "catalog identity mismatch for "
                          << seen.profile_id << '\n';
                break;
            }
        }
    }
    if (!expect(catalog_ids_match,
                "dynamic-world catalog no longer has the 19 "
                "profile/action pairs")) {
        std::cerr << catalog_path.string() << " unique="
                  << catalog_seen.size() << '\n';
        return 32;
    }

    ScriptCombatantFacts catalog_attacker{};
    catalog_attacker.record_type = 7U;
    catalog_attacker.source_flags = 0x25U;
    catalog_attacker.cyberware_flags = 0x0008U;
    catalog_attacker.command_attribute_77 = 2U;
    catalog_attacker.command_attribute_78 = 4U;
    catalog_attacker.command_attribute_79 = 1U;
    catalog_attacker.command_attribute_7b = 4U;
    catalog_attacker.command_attribute_7c = 4U;
    catalog_attacker.command_attribute_85 = 3U;
    catalog_attacker.race_or_state = 4;
    catalog_attacker.class_code = 7U;
    catalog_attacker.equipped_action_code = 1U;
    ScriptCombatantFacts catalog_defender{};
    catalog_defender.record_type = 0U;
    catalog_defender.source_flags = 0x01U;
    catalog_defender.interaction_flags = 0x80U;
    catalog_defender.physical_meter = 10U;
    catalog_defender.mental_meter = 10U;
    catalog_defender.command_attribute_77 = 1U;
    catalog_defender.command_attribute_78 = 4U;
    catalog_defender.command_attribute_79 = 1U;
    catalog_defender.command_attribute_7b = 4U;
    catalog_defender.command_attribute_7c = 4U;
    catalog_defender.armor_index = 0U;
    catalog_defender.class_code = 0U;
    catalog_defender.equipped_action_code = 0U;
    const auto& catalog_spell_rows = us_magic_spell_catalog();
    const auto drive_catalog_decision =
        [&](const CatalogProfile& row)
        -> std::optional<ScriptActorBehaviorDecision> {
        const std::string actor_id =
            std::string("catalog_") + row.profile_id;
        const std::uint16_t distance =
            row.decision == CatalogDecision::item_gated ? 0x0C00U : 0x0100U;
        auto context = actor_context(
            row.profile_id, actor_id, row.entity_kind, row.action_kind,
            row.action_code, 0U, true);
        context.dynamic_world_encounter_actor = true;
        context.source_record_flags = 0x01U;
        context.entry_animation_frame_count = 1U;
        context.walk_animation_frame_count = 4U;
        context.movement_allowed = true;
        context.initial_action_delay = 0;
        if (row.decision == CatalogDecision::hound_effect) {
            // Template +0x78/+0x7B. 0xD7EA with no capability bits yields 90.
            context.delay_primary_attribute = 8U;
            context.delay_secondary_attribute = 2U;
        }
        if (row.decision == CatalogDecision::unarmed) {
            context.action_animation_frame_count = 2U;
            context.action_animation_increment = 0x0200U;
            context.action_animation_threshold = 0x1000U;
            context.action_animation_audio_event = 0x004CU;
            context.combatant = catalog_attacker;
            context.action_attribute_selectors = {
                catalog_spell_rows[5U].attribute_selector,
                catalog_spell_rows[12U].attribute_selector};
        }
        if (row.decision == CatalogDecision::item_gated) {
            context.source_projection_active = true;
            context.action_line_clear =
                [](const std::uint16_t, const std::uint16_t) { return true; };
        }
        std::size_t roll_index = 0U;
        context.ai_random_bound =
            [&roll_index](const std::uint16_t bound)
            -> std::optional<std::uint16_t> {
            if (bound == 0U) {
                return std::nullopt;
            }
            if (bound == 1U) {
                return 0U;
            }
            if (bound == 6U) {
                const auto value = (roll_index++ & 1U) == 0U ? 5U : 0U;
                return static_cast<std::uint16_t>(value);
            }
            return 0U;
        };
        ScriptActorTargetCandidate target{
            "party.runner.0", 0x1234U, 0x0ABCU, 0x2000U, distance, 0x01U,
            0U, 0U, 0U, true, false, distance, distance, distance,
            static_cast<std::uint16_t>(distance * 2U)};
        target.source_record_heading = 0U;
        if (row.decision == CatalogDecision::unarmed) {
            target.source_record_slot = 0U;
            target.combatant = catalog_defender;
        }
        context.ai_target_candidates.push_back(target);
        const auto failed = [&scripts]() {
            std::cerr << scripts.failure() << '\n';
            return std::optional<ScriptActorBehaviorDecision>{};
        };
        if (!scripts.actor_heartbeat(context).has_value()) {
            return failed();
        }
        context.spawn = false;
        if (!scripts.actor_heartbeat(context).has_value()) {
            return failed();
        }
        context.phase = ScriptActorHeartbeatPhase::render;
        for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
            if (!scripts.actor_heartbeat(context).has_value()) {
                return failed();
            }
        }
        context.phase = ScriptActorHeartbeatPhase::behavior;
        for (std::uint8_t pass = 0U; pass < 120U; ++pass) {
            const auto decision = scripts.actor_heartbeat(context);
            if (!decision.has_value()) {
                return failed();
            }
            const bool found =
                (row.decision == CatalogDecision::item_gated &&
                 decision->source_action_dispatch.has_value()) ||
                (row.decision == CatalogDecision::unarmed &&
                 decision->animation_id == "action") ||
                (row.decision == CatalogDecision::hound_effect &&
                 decision->target_motion_effect_slot_request.has_value());
            if (found) {
                return decision;
            }
        }
        std::cerr << "catalog refresh missing for " << row.profile_id
                  << '\n';
        return std::nullopt;
    };
    const auto dump_catalog_decision =
        [](const ScriptActorBehaviorDecision& decision) {
            std::cerr << " anim=" << decision.animation_id
                      << " delay=" << decision.action_delay
                      << " engaged=" << decision.ai_target_engaged
                      << " motion_delay="
                      << static_cast<unsigned>(decision.target_motion_delay);
            if (decision.movement_heading.has_value()) {
                std::cerr << " heading=0x" << std::hex
                          << *decision.movement_heading << std::dec;
            }
            if (decision.source_action_dispatch.has_value()) {
                std::cerr << " dispatch=" << *decision.source_action_dispatch;
            }
            if (decision.weapon_item_id.has_value()) {
                std::cerr << " weapon="
                          << static_cast<unsigned>(*decision.weapon_item_id);
            }
            if (decision.target_motion_effect_slot_request.has_value()) {
                std::cerr << " effect="
                          << static_cast<unsigned>(
                                 *decision.target_motion_effect_slot_request);
            }
            if (decision.audio_event.has_value()) {
                std::cerr << " audio=0x" << std::hex << *decision.audio_event
                          << std::dec;
            }
            if (decision.combat_meter_update.has_value()) {
                std::cerr << " meter="
                          << static_cast<unsigned>(
                                 decision.combat_meter_update->physical_meter);
            }
            std::cerr << '\n';
        };
    bool catalog_decisions_ok = true;
    for (const auto& row : catalog_profiles) {
        bool row_ok = false;
        std::optional<ScriptActorBehaviorDecision> decision;
        if (row.decision == CatalogDecision::spell) {
            const auto driven = drive_spell_refresh(
                row.profile_id,
                std::string("catalog_") + row.profile_id, row.entity_kind,
                row.action_code, 0, true, spell_line_true);
            row_ok = spell_cast_ok(driven, 0x0084U);
            decision = driven.refresh;
        } else {
            decision = drive_catalog_decision(row);
            const bool dispatch_matches =
                decision.has_value() &&
                ((row.decision == CatalogDecision::item_gated) ==
                 decision->source_action_dispatch.has_value());
            if (dispatch_matches && !decision->combat_action_requested) {
                if (row.decision == CatalogDecision::item_gated) {
                    row_ok =
                        decision->source_action_dispatch.has_value() &&
                        *decision->source_action_dispatch == "item_gated" &&
                        decision->weapon_item_id == row.action_code &&
                        decision->animation_id != "action" &&
                        decision->animation_id != "spell" &&
                        !decision->ai_target_engaged &&
                        !decision->combat_meter_update.has_value() &&
                        !decision->audio_event.has_value() &&
                        decision->movement_heading == 0x2000U &&
                        decision->target_motion_delay == 5U;
                } else if (row.decision == CatalogDecision::hound_effect) {
                    row_ok =
                        decision->target_motion_effect_slot_request == 0U &&
                        decision->action_delay == 90 &&
                        decision->ai_target_engaged &&
                        decision->animation_id != "action" &&
                        decision->animation_id != "spell" &&
                        !decision->combat_meter_update.has_value() &&
                        !decision->weapon_item_id.has_value() &&
                        !decision->audio_event.has_value() &&
                        decision->movement_heading == 0x2000U;
                } else {
                    // Zero delay attributes, mode 1: floor(0x6A / 2) == 53.
                    // Meter 7 is the shared dice stub, not the template row.
                    row_ok =
                        decision->animation_id == "action" &&
                        decision->ai_target_engaged &&
                        decision->action_delay == 53 &&
                        decision->audio_event == 0x004CU &&
                        decision->target_heading_audio_event == 0x004DU &&
                        !decision->weapon_item_id.has_value() &&
                        decision->movement_heading == 0x2000U &&
                        decision->combat_meter_update.has_value() &&
                        decision->combat_meter_update->target_id ==
                            "party.runner.0" &&
                        decision->combat_meter_update->record_type == 0U &&
                        decision->combat_meter_update->physical &&
                        decision->combat_meter_update->changed &&
                        decision->combat_meter_update->physical_meter == 7U &&
                        decision->combat_meter_update->mental_meter == 10U;
                }
            }
        }
        if (!row_ok) {
            catalog_decisions_ok = false;
            std::cerr << "catalog decision failed for " << row.profile_id
                      << '\n';
            if (decision.has_value()) {
                dump_catalog_decision(*decision);
            }
            break;
        }
    }
    if (!expect(catalog_decisions_ok,
                "Lua catalog profiles did not publish their "
                "0x13FFE/0x14C7C decisions")) {
        std::cerr << scripts.failure() << '\n';
        return 32;
    }

    auto combat_probe_context = actor_context(
        "eye5r_1_k07_none00", "combat_reaction_probe", 7U,
        ScriptActorActionKind::none, 0U, 0U, true);
    combat_probe_context.dynamic_world_encounter_actor = true;
    combat_probe_context.source_record_flags = 0x25U;
    combat_probe_context.entry_animation_frame_count = 1U;
    combat_probe_context.walk_animation_frame_count = 4U;
    combat_probe_context.action_animation_frame_count = 2U;
    combat_probe_context.action_animation_increment = 0x0200U;
    combat_probe_context.action_animation_threshold = 0x1000U;
    combat_probe_context.action_animation_audio_event = 0x004CU;
    combat_probe_context.initial_action_delay = 0;
    combat_probe_context.delay_primary_attribute = 4U;
    combat_probe_context.delay_secondary_attribute = 6U;
    combat_probe_context.movement_allowed = true;
    const auto& combat_spell_rows = us_magic_spell_catalog();
    combat_probe_context.action_attribute_selectors = {
        combat_spell_rows[5U].attribute_selector,
        combat_spell_rows[12U].attribute_selector};
    ScriptCombatantFacts attacker_facts{};
    attacker_facts.record_type = 7U;
    attacker_facts.source_flags = 0x25U;
    attacker_facts.cyberware_flags = 0x0008U;
    attacker_facts.command_attribute_77 = 2U;
    attacker_facts.command_attribute_78 = 4U;
    attacker_facts.command_attribute_79 = 1U;
    attacker_facts.command_attribute_7b = 4U;
    attacker_facts.command_attribute_7c = 4U;
    attacker_facts.command_attribute_85 = 3U;
    attacker_facts.race_or_state = 4;
    attacker_facts.class_code = 7U;
    attacker_facts.equipped_action_code = 1U;
    combat_probe_context.combatant = attacker_facts;
    ScriptCombatantFacts target_facts{};
    target_facts.record_type = 0U;
    target_facts.source_flags = 0x01U;
    target_facts.interaction_flags = 0x80U;
    target_facts.physical_meter = 10U;
    target_facts.mental_meter = 10U;
    target_facts.command_attribute_77 = 1U;
    target_facts.command_attribute_78 = 4U;
    target_facts.command_attribute_79 = 1U;
    target_facts.command_attribute_7b = 4U;
    target_facts.command_attribute_7c = 4U;
    target_facts.armor_index = 0U;
    target_facts.class_code = 0U;
    target_facts.equipped_action_code = 0U;
    ScriptActorTargetCandidate combat_target{
        "party.runner.0", 0x1800U, 0x1800U, 0x2000U,
        0x0100U, 0x01U, 0x80U, 0U, 0U, true, false,
        0x0100U, 0x0200U, 0x0100U, 0x0200U};
    combat_target.source_record_slot = 0U;
    combat_target.combatant = target_facts;
    combat_probe_context.ai_target_candidates.push_back(combat_target);
    std::size_t combat_roll_index = 0U;
    combat_probe_context.ai_random_bound =
        [&combat_roll_index](const std::uint16_t bound)
            -> std::optional<std::uint16_t> {
        if (bound == 0U) {
            return std::nullopt;
        }
        if (bound == 1U) {
            return 0U;
        }
        if (bound == 6U) {
            const auto value = (combat_roll_index++ & 1U) == 0U ? 5U : 0U;
            return static_cast<std::uint16_t>(value);
        }
        return 0U;
    };
    const auto combat_probe_spawn =
        scripts.actor_heartbeat(combat_probe_context);
    combat_probe_context.spawn = false;
    (void)scripts.actor_heartbeat(combat_probe_context);
    combat_probe_context.phase = ScriptActorHeartbeatPhase::render;
    for (std::uint8_t frame = 0U; frame < 12U; ++frame) {
        (void)scripts.actor_heartbeat(combat_probe_context);
    }
    combat_probe_context.phase = ScriptActorHeartbeatPhase::behavior;
    std::optional<ScriptActorBehaviorDecision> combat_probe_action;
    for (std::uint8_t pass = 0U; pass < 100U; ++pass) {
        const auto decision = scripts.actor_heartbeat(combat_probe_context);
        if (!decision.has_value()) {
            break;
        }
        if (decision->animation_id == "action") {
            combat_probe_action = decision;
            break;
        }
    }
    if (!expect(
            combat_probe_spawn.has_value() &&
                combat_probe_action.has_value() &&
                combat_probe_action->ai_target_engaged &&
                combat_probe_action->combat_meter_update.has_value() &&
                combat_probe_action->combat_meter_update->target_id ==
                    "party.runner.0" &&
                combat_probe_action->combat_meter_update->record_type == 0U &&
                combat_probe_action->combat_meter_update->physical &&
                combat_probe_action->combat_meter_update->changed &&
                combat_probe_action->combat_meter_update->physical_meter == 7U &&
                combat_probe_action->combat_meter_update->mental_meter == 10U,
            "Lua did not resolve the ROM close-target reaction into the typed party meter update")) {
        std::cerr << scripts.failure() << '\n';
        return 67;
    }

    if (!expect(
            guard_wait.has_value() &&
                guard_wait->package_id ==
                    "corp_security.max_power_heavy_pistol" &&
                guard_wait->action ==
                    ScriptActorBehaviorDecision::Action::hold &&
                !guard_wait->audio_event.has_value() &&
                guard_wait_tick.has_value() &&
                guard_wait_tick->action ==
                    ScriptActorBehaviorDecision::Action::hold &&
                guard_entry_spawn.has_value() &&
                guard_entry_spawn->animation_id == "idle" &&
                guard_entry.has_value() &&
                guard_entry->action ==
                    ScriptActorBehaviorDecision::Action::
                        play_entry_animation &&
                guard_entry->package_id ==
                    "corp_security.max_power_heavy_pistol" &&
                guard_entry->animation_id == "entry" &&
                guard_entry->animation_frame == 0U &&
                guard_entry->animation_accumulator == -16 &&
                guard_entry->audio_event == 0x002EU &&
                guard_entry->weapon_item_id == 6U &&
                guard_hold.has_value() &&
                guard_hold->animation_frame == 0U &&
                guard_entry_last_frame.has_value() &&
                guard_entry_last_frame->animation_id == "entry" &&
                guard_entry_last_frame->animation_frame == 1U &&
                pedestrian_spawn.has_value() &&
                pedestrian_spawn->animation_id == "idle" &&
                pedestrian_entry.has_value() &&
                pedestrian_entry->package_id ==
                    "pedestrian.no_active_action" &&
                pedestrian_entry->audio_event == 0x002FU &&
                !pedestrian_entry->weapon_item_id.has_value() &&
                target_marker_action_0.has_value() &&
                target_marker_action_0->target_marker_visible &&
                target_marker_action_0->target_marker_style == 0U &&
                target_marker_action_0->target_marker_radius == 64U &&
                target_marker_action_3.has_value() &&
                target_marker_action_3->target_marker_style == 1U &&
                target_marker_action_6.has_value() &&
                target_marker_action_6->target_marker_style == 2U &&
                target_marker_action_9.has_value() &&
                target_marker_action_9->target_marker_style == 3U &&
                target_marker_spawn.has_value() &&
                target_marker_first.has_value() &&
                target_marker_first->target_marker_visible &&
                target_marker_first->target_marker_animating &&
                target_marker_first->target_marker_radius == 64U &&
                target_marker_last_animated.has_value() &&
                target_marker_last_animated->target_marker_animating &&
                target_marker_last_animated->target_marker_radius == 0U &&
                target_marker_static.has_value() &&
                target_marker_static->target_marker_visible &&
                !target_marker_static->target_marker_animating &&
                target_marker_cleared.has_value() &&
                !target_marker_cleared->target_marker_visible &&
                target_selection_next.has_value() &&
                target_selection_next->target_selection_processed &&
                target_selection_next->selected_world_actor_id == "next_actor" &&
                target_selection_wrap.has_value() &&
                target_selection_wrap->selected_world_actor_id ==
                    "wrapped_actor" &&
                target_selection_special.has_value() &&
                target_selection_special->selected_world_actor_id ==
                    "special_actor" &&
                guard_hold.has_value() &&
                guard_hold->action ==
                    ScriptActorBehaviorDecision::Action::hold &&
                guard_hold->weapon_item_id == 6U &&
                !guard_hold->audio_event.has_value() &&
                !unsupported_guard_action.has_value() &&
                !unsupported_profile.has_value(),
            "Lua actor packages did not resolve the ROM action/equipment state")) {
        std::cerr << scripts.failure() << '\n';
        return 15;
    }

    ScriptActorBehaviorContext player_movement{};
    player_movement.profile_id = "field_runner_k03_none00";
    player_movement.entity_kind = 3U;
    player_movement.instance_id = "field_player_runner";
    player_movement.initial_heading = 0U;
    player_movement.spawn = true;
    player_movement.movement_allowed = true;
    player_movement.controller_nibble = 6U;
    player_movement.controller_heading_table.fill(0xFFFFU);
    player_movement.controller_heading_table[6] = 0x4000U;
    const auto player_right = scripts.actor_heartbeat(player_movement);
    player_movement.spawn = false;
    player_movement.controller_nibble = 7U;
    player_movement.controller_heading_table[7] = 0x6000U;
    player_movement.movement_heading_override = 0x2000U;
    const auto player_scripted_heading =
        scripts.actor_heartbeat(player_movement);
    player_movement.movement_heading_override.reset();
    player_movement.controller_heading_table[7] = 0xFFFFU;
    const auto player_invalid_heading =
        scripts.actor_heartbeat(player_movement);
    player_movement.movement_allowed = false;
    const auto player_idle = scripts.actor_heartbeat(player_movement);
    if (!expect(
            player_right.has_value() &&
                player_right->package_id == "runner.field_controlled" &&
                player_right->movement_heading == 0x4000U &&
                player_right->movement_current_heading == 0x0800U &&
                player_scripted_heading.has_value() &&
                player_scripted_heading->movement_heading == 0x2000U &&
                player_scripted_heading->movement_current_heading == 0x1000U &&
                player_invalid_heading.has_value() &&
                !player_invalid_heading->movement_heading.has_value() &&
                player_invalid_heading->movement_current_heading == 0x1000U &&
                player_idle.has_value() &&
                !player_idle->movement_heading.has_value() &&
                player_idle->movement_current_heading == 0x1000U,
            "Lua player heartbeat did not resolve movement intent and heading slew")) {
        std::cerr << scripts.failure() << '\n';
        return 16;
    }

    player_movement.phase = ScriptActorHeartbeatPhase::render;
    player_movement.initial_heading = 0x4000U;
    player_movement.animation_mode = ScriptActorAnimationMode::walking;
    const auto player_walk_start = scripts.actor_heartbeat(player_movement);
    std::optional<ScriptActorBehaviorDecision> player_walk_seventh;
    for (std::size_t update = 0; update < 6U; ++update) {
        player_walk_seventh = scripts.actor_heartbeat(player_movement);
    }
    player_movement.animation_mode = ScriptActorAnimationMode::idle;
    player_movement.animation_direction_override = 4U;
    const auto player_reset_idle = scripts.actor_heartbeat(player_movement);
    player_movement.animation_direction_override.reset();
    player_movement.animation_mode = ScriptActorAnimationMode::attack;
    const auto player_attack_pose = scripts.actor_heartbeat(player_movement);
    player_movement.animation_mode = ScriptActorAnimationMode::walking;
    std::optional<ScriptActorBehaviorDecision> player_walk_after_attack;
    for (std::size_t update = 0; update < 7U; ++update) {
        player_walk_after_attack = scripts.actor_heartbeat(player_movement);
    }
    player_movement.phase = ScriptActorHeartbeatPhase::behavior;
    player_movement.movement_allowed = true;
    player_movement.controller_nibble = 6U;
    const auto player_behavior_after_render =
        scripts.actor_heartbeat(player_movement);
    player_movement.combat_action_available = true;
    player_movement.combat_confirm_held = true;
    player_movement.combat_action_ready = true;
    player_movement.combat_input_blocked = false;
    const auto player_combat_moving =
        scripts.actor_heartbeat(player_movement);
    player_movement.movement_allowed = false;
    player_movement.controller_nibble = 0U;
    const auto player_combat_stationary =
        scripts.actor_heartbeat(player_movement);
    player_movement.combat_action_ready = false;
    const auto player_combat_cooldown = scripts.actor_heartbeat(player_movement);
    player_movement.combat_action_ready = true;
    player_movement.combat_input_blocked = true;
    const auto player_combat_modal_block =
        scripts.actor_heartbeat(player_movement);
    if (!expect(
            player_walk_start.has_value() &&
                player_walk_start->animation_id == "walk" &&
                player_walk_start->animation_selector == 0x28U &&
                player_walk_start->direction == 2U &&
                player_walk_start->animation_frame == 0U &&
                player_walk_start->animation_accumulator == 0x0280 &&
                player_walk_seventh.has_value() &&
                player_walk_seventh->animation_frame == 1U &&
                player_reset_idle.has_value() &&
                player_reset_idle->animation_id == "idle" &&
                player_reset_idle->direction == 4U &&
                player_reset_idle->animation_selector == 0x10U &&
                player_reset_idle->animation_accumulator == 0x0400 &&
                player_attack_pose.has_value() &&
                player_attack_pose->animation_id == "attack" &&
                player_attack_pose->animation_frame == 0U &&
                player_attack_pose->animation_accumulator == 0 &&
                player_walk_after_attack.has_value() &&
                player_walk_after_attack->animation_frame == 1U &&
                player_behavior_after_render.has_value() &&
                player_behavior_after_render->animation_id == "walk" &&
                player_behavior_after_render->movement_heading == 0x4000U &&
                player_behavior_after_render->movement_current_heading ==
                    0x1800U &&
                player_combat_moving.has_value() &&
                player_combat_moving->combat_action_requested &&
                player_combat_stationary.has_value() &&
                player_combat_stationary->combat_action_requested &&
                player_combat_cooldown.has_value() &&
                !player_combat_cooldown->combat_action_requested &&
                player_combat_modal_block.has_value() &&
                !player_combat_modal_block->combat_action_requested,
            "Lua heartbeat did not own player animation, movement, and source-gated combat cadence")) {
        std::cerr << scripts.failure() << '\n';
        return 17;
    }

    auto projectile_context = actor_context(
        "weapon_projectile", "field_weapon_slot_9", 3U,
        ScriptActorActionKind::none, 0U, 0U, true);
    projectile_context.phase = ScriptActorHeartbeatPhase::render;
    projectile_context.projectile_animation =
        ScriptActorProjectileAnimation{0U, 3U, 0U, 0, 0, 0x1000, true};
    const auto projectile_frame_zero =
        scripts.actor_heartbeat(projectile_context);
    projectile_context.spawn = false;
    const auto set_projectile_input_from =
        [&projectile_context](const ScriptActorBehaviorDecision& decision) {
            auto& animation = *projectile_context.projectile_animation;
            animation.selector = decision.animation_selector.value_or(0U);
            animation.frame = decision.animation_frame;
            animation.accumulator = decision.animation_accumulator;
            animation.increment = decision.animation_increment.value_or(0);
        };
    if (projectile_frame_zero.has_value()) {
        set_projectile_input_from(*projectile_frame_zero);
    }
    const auto projectile_frame_one =
        scripts.actor_heartbeat(projectile_context);
    if (projectile_frame_one.has_value()) {
        set_projectile_input_from(*projectile_frame_one);
    }
    const auto projectile_frame_two =
        scripts.actor_heartbeat(projectile_context);
    if (projectile_frame_two.has_value()) {
        set_projectile_input_from(*projectile_frame_two);
    }
    const auto projectile_terminal =
        scripts.actor_heartbeat(projectile_context);
    if (!expect(
            projectile_frame_zero.has_value() &&
                projectile_frame_zero->package_id ==
                    "combat.weapon_projectile" &&
                projectile_frame_zero->animation_id == "projectile" &&
                projectile_frame_zero->animation_frame == 0U &&
                projectile_frame_zero->animation_increment == 0x270f &&
                projectile_frame_one.has_value() &&
                projectile_frame_one->animation_frame == 1U &&
                projectile_frame_two.has_value() &&
                projectile_frame_two->animation_frame == 2U &&
                projectile_terminal.has_value() &&
                projectile_terminal->animation_selector == 0x8000U &&
                projectile_terminal->animation_frame == 2U,
            "Lua projectile render heartbeat did not follow the finite ROM animation list")) {
        std::cerr << scripts.failure() << '\n';
        return 19;
    }

    auto special_projectile_context = actor_context(
        "weapon_projectile", "field_weapon_slot_10", 3U,
        ScriptActorActionKind::none, 0U, 0U, true);
    special_projectile_context.phase = ScriptActorHeartbeatPhase::render;
    special_projectile_context.projectile_animation =
        ScriptActorProjectileAnimation{0x20U, 11U, 0U, 0, 0, 0x1000, true};
    auto special_projectile_frame =
        scripts.actor_heartbeat(special_projectile_context);
    if (!special_projectile_frame.has_value() ||
        special_projectile_frame->animation_frame != 0U ||
        special_projectile_frame->animation_increment != 0x270f) {
        std::cerr << "Lua special projectile render heartbeat did not start at frame 0\n";
        return 20;
    }
    special_projectile_context.spawn = false;
    constexpr std::array<std::uint16_t, 19> expected_special_frames{
        1U, 2U, 3U, 4U, 4U, 4U, 4U, 4U, 5U, 6U,
        7U, 8U, 9U, 9U, 9U, 9U, 9U, 10U, 10U};
    for (std::size_t tick = 0U; tick < expected_special_frames.size(); ++tick) {
        auto& animation =
            *special_projectile_context.projectile_animation;
        animation.selector = special_projectile_frame->animation_selector
                                 .value_or(0U);
        animation.frame = special_projectile_frame->animation_frame;
        animation.accumulator =
            special_projectile_frame->animation_accumulator;
        animation.increment =
            special_projectile_frame->animation_increment.value_or(0);
        special_projectile_frame =
            scripts.actor_heartbeat(special_projectile_context);
        const bool terminal = tick + 1U == expected_special_frames.size();
        const auto expected_frame = expected_special_frames[tick];
        if (!special_projectile_frame.has_value() ||
            !special_projectile_frame->animation_selector.has_value() ||
            *special_projectile_frame->animation_selector !=
                (terminal ? 0x8020U : 0x0020U) ||
            special_projectile_frame->animation_frame !=
                (terminal ? 10U : expected_frame)) {
            std::cerr << "Lua special projectile render heartbeat did not follow the finite 0x800B list at frame "
                      << tick + 1U << '\n';
            return 20;
        }
    }

    const auto projectile_behavior_context = [&actor_context](
                                                  const std::string_view actor_id,
                                                  const ScriptActorProjectileMovement movement) {
        auto context = actor_context(
            "weapon_projectile", actor_id, 3U,
            ScriptActorActionKind::none, 0U, 0U, true);
        context.phase = ScriptActorHeartbeatPhase::behavior;
        context.projectile_animation = ScriptActorProjectileAnimation{
            movement.animation_selector, 3U, 0U, 0, 0, 0x1000, true};
        context.projectile_movement = movement;
        return context;
    };

    ScriptActorProjectileMovement moving_projectile{};
    moving_projectile.role = ScriptActorProjectileRole::primary;
    moving_projectile.active = true;
    moving_projectile.flags = 0x01U;
    moving_projectile.world_x = 10 * 0x10000;
    moving_projectile.world_y = 20 * 0x10000;
    moving_projectile.tile_offset_x = 0x0100;
    moving_projectile.retry_counter = 20U;
    std::vector<std::uint16_t> projectile_terrain_x;
    std::uint32_t projectile_collision_count = 0U;
    auto moving_projectile_context = projectile_behavior_context(
        "field_weapon_slot_move", moving_projectile);
    moving_projectile_context.projectile_terrain_probe =
        [&projectile_terrain_x](const std::uint16_t x,
                                const std::uint16_t) {
            projectile_terrain_x.push_back(x);
            return ScriptProjectileTerrainSample{0U, true, false};
        };
    moving_projectile_context.projectile_collision_search =
        [&projectile_collision_count](const std::uint16_t,
                                      const std::uint16_t,
                                      const std::uint8_t) {
            ++projectile_collision_count;
            return ScriptProjectileCollisionSearchResult{};
        };
    const auto moving_projectile_step =
        scripts.actor_heartbeat(moving_projectile_context);
    if (!expect(
            moving_projectile_step.has_value() &&
                moving_projectile_step->projectile_movement.has_value() &&
                projectile_terrain_x ==
                    std::vector<std::uint16_t>{11U, 12U, 13U, 14U} &&
                projectile_collision_count == 4U &&
                moving_projectile_step->projectile_movement->state.world_x ==
                    14 * 0x10000 &&
                moving_projectile_step->projectile_movement->state.retry_counter ==
                    16U &&
                moving_projectile_step->projectile_movement->state.active &&
                moving_projectile_step->projectile_movement->outcome ==
                    ScriptActorProjectileOutcome::none &&
                !moving_projectile_step->audio_event.has_value(),
            "Lua projectile behavior did not execute four ordered terrain/collision probes")) {
        std::cerr << scripts.failure() << '\n';
        return 21;
    }

    auto outside_projectile = moving_projectile;
    outside_projectile.tile_offset_x = 2;
    outside_projectile.retry_counter = 7U;
    auto outside_projectile_context = projectile_behavior_context(
        "field_weapon_slot_outside", outside_projectile);
    std::uint32_t outside_collision_count = 0U;
    outside_projectile_context.projectile_terrain_probe =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptProjectileTerrainSample{1U, false, true};
        };
    outside_projectile_context.projectile_collision_search =
        [&outside_collision_count](const std::uint16_t,
                                   const std::uint16_t,
                                   const std::uint8_t) {
            ++outside_collision_count;
            return ScriptProjectileCollisionSearchResult{};
        };
    const auto outside_projectile_step =
        scripts.actor_heartbeat(outside_projectile_context);
    if (!expect(
            outside_projectile_step.has_value() &&
                outside_projectile_step->projectile_movement.has_value() &&
                outside_projectile_step->projectile_movement->state.record_flags ==
                    0x09U &&
                outside_projectile_step->projectile_movement->state.flags ==
                    0U &&
                !outside_projectile_step->projectile_movement->state.active &&
                outside_projectile_step->projectile_movement->state.animation_selector ==
                    0U &&
                outside_projectile_step->projectile_movement->state.tile_offset_x ==
                    0 &&
                outside_projectile_step->projectile_movement->state.retry_counter ==
                    7U &&
                outside_projectile_step->projectile_movement->outcome ==
                    ScriptActorProjectileOutcome::retired &&
                outside_collision_count == 0U &&
                outside_projectile_step->audio_event == 0x003CU,
            "Lua projectile terminal branch did not clear outside state and request sound 0x3C")) {
        std::cerr << scripts.failure() << '\n';
        return 22;
    }

    auto exhausted_projectile = moving_projectile;
    exhausted_projectile.retry_counter = 0U;
    auto exhausted_projectile_context = projectile_behavior_context(
        "field_weapon_slot_exhausted", exhausted_projectile);
    std::uint32_t exhausted_collision_count = 0U;
    exhausted_projectile_context.projectile_terrain_probe =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptProjectileTerrainSample{0U, true, false};
        };
    exhausted_projectile_context.projectile_collision_search =
        [&exhausted_collision_count](const std::uint16_t,
                                    const std::uint16_t,
                                    const std::uint8_t) {
            ++exhausted_collision_count;
            return ScriptProjectileCollisionSearchResult{};
        };
    const auto exhausted_projectile_step =
        scripts.actor_heartbeat(exhausted_projectile_context);
    if (!expect(
            exhausted_projectile_step.has_value() &&
                exhausted_projectile_step->projectile_movement.has_value() &&
                exhausted_projectile_step->projectile_movement->state.retry_counter ==
                    0xFFU &&
                exhausted_projectile_step->projectile_movement->state.active &&
                exhausted_projectile_step->projectile_movement->outcome ==
                    ScriptActorProjectileOutcome::stopped &&
                exhausted_projectile_step->audio_event == 0x003CU &&
                exhausted_collision_count == 0U,
            "Lua projectile retry exhaustion did not stop before collision search")) {
        std::cerr << scripts.failure() << '\n';
        return 23;
    }

    auto hit_projectile = moving_projectile;
    hit_projectile.world_x = 10 * 0x10000 + 123;
    hit_projectile.tile_offset_x = 0x0100;
    auto hit_projectile_context = projectile_behavior_context(
        "field_weapon_slot_hit", hit_projectile);
    hit_projectile_context.projectile_terrain_probe =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptProjectileTerrainSample{0U, true, false};
        };
    hit_projectile_context.projectile_collision_search =
        [](const std::uint16_t, const std::uint16_t,
           const std::uint8_t) {
            return ScriptProjectileCollisionSearchResult{
                true, true, 45U, 77U,
                std::string{"field_scene_2_encounter_1_actor_0"}};
        };
    const auto hit_projectile_step =
        scripts.actor_heartbeat(hit_projectile_context);
    if (!expect(
            hit_projectile_step.has_value() &&
                hit_projectile_step->projectile_movement.has_value() &&
                hit_projectile_step->projectile_movement->outcome ==
                    ScriptActorProjectileOutcome::hit_resolution_pending &&
                hit_projectile_step->projectile_movement->state.hit_target_id ==
                    "field_scene_2_encounter_1_actor_0" &&
                hit_projectile_step->projectile_movement->state.world_x ==
                    45 * 0x10000 + 123 &&
                hit_projectile_step->audio_event == 0x003CU,
            "Lua projectile hit branch did not preserve fractional coordinates and request hit audio")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }

    auto source_weapon_hit = moving_projectile;
    source_weapon_hit.action_result = 0;
    auto source_weapon_hit_context = projectile_behavior_context(
        "field_weapon_slot_combat_hit", source_weapon_hit);
    source_weapon_hit_context.projectile_action_code = 5U;
    source_weapon_hit_context.projectile_attack_seed_pending = true;
    ScriptCombatantFacts shooter_facts{};
    shooter_facts.record_type = 0U;
    shooter_facts.source_flags = 0x01U;
    shooter_facts.physical_meter = 8U;
    shooter_facts.mental_meter = 8U;
    shooter_facts.command_attribute_77 = 4U;
    shooter_facts.command_attribute_78 = 4U;
    shooter_facts.command_attribute_7b = 4U;
    shooter_facts.command_attribute_7c = 4U;
    shooter_facts.action_skills = {10U, 10U, 10U, 10U};
    source_weapon_hit_context.combatant = shooter_facts;
    ScriptCombatantFacts victim_facts{};
    victim_facts.record_type = 3U;
    victim_facts.physical_meter = 8U;
    victim_facts.mental_meter = 8U;
    victim_facts.command_attribute_77 = 4U;
    victim_facts.command_attribute_78 = 4U;
    victim_facts.command_attribute_7b = 4U;
    victim_facts.command_attribute_7c = 4U;
    const auto make_encounter_reaction_context =
        [&actor_context](const std::string_view actor_id) {
        auto context = actor_context(
            "hell_hound_k0c_none00", actor_id, 12U,
            ScriptActorActionKind::none, 0U, 0U, true);
        context.dynamic_world_encounter_actor = true;
        context.source_record_flags = 0x25U;
        context.entry_animation_frame_count = 2U;
        context.walk_animation_frame_count = 4U;
        context.hurt_animation_frame_count = 2U;
        context.hurt_animation_increment = 0x0300U;
        context.hurt_animation_threshold = 0x1000U;
        context.hurt_audio_event = 0x0057U;
        context.down_audio_event = 0x0056U;
        return context;
    };
    const auto configure_encounter_movement = [](
        ScriptActorBehaviorContext& context,
        const std::uint16_t target_distance) {
        context.movement_allowed = true;
        context.movement_services_available = true;
        context.movement_state.position_x = 0x1800U;
        context.movement_state.position_y = 0x1800U;
        context.movement_state.heading = 0U;
        context.movement_state.animation_heading = 0U;
        context.movement_state.route_status = -1;
        context.movement_state.route_flags = 0x03U;
        context.movement_offsets = {{
            {0, -18}, {18, -18}, {18, 0}, {18, 18},
            {0, 18}, {-18, 18}, {-18, 0}, {-18, -18},
        }};
        context.movement_idle_selectors = {{
            0x20U, 0x21U, 0x22U, 0x23U,
            0x24U, 0x25U, 0x26U, 0x27U,
        }};
        context.movement_collision =
            [](const std::uint16_t, const std::uint16_t) {
                return ScriptMovementCollisionResult{0, 0U};
            };
        context.movement_route_lookup =
            [](const std::uint16_t) { return ScriptMovementRouteFacts{}; };
        context.movement_collision_recovery =
            [](const std::uint16_t, const std::uint16_t,
               const std::int16_t, const std::uint8_t) {
                return ScriptMovementCollisionRecoveryResult{false, 0U};
            };
        context.ai_random_bound =
            [](const std::uint16_t bound)
                -> std::optional<std::uint16_t> {
            return bound == 0U ? std::nullopt
                               : std::optional<std::uint16_t>{0U};
        };
        context.ai_target_candidates.push_back(
            ScriptActorTargetCandidate{
                "party.runner.0", 0x1800U, 0x1800U, 0x2000U,
                target_distance, 0x01U, 0U, 0U, 0U, true, false,
                target_distance,
                static_cast<std::uint16_t>(target_distance * 2U),
                target_distance,
                static_cast<std::uint16_t>(target_distance * 2U)});
    };
    const auto finish_encounter_entry = [&scripts](
                                            ScriptActorBehaviorContext& context) {
        context.spawn = false;
        context.phase = ScriptActorHeartbeatPhase::behavior;
        auto decision = scripts.actor_heartbeat(context);
        if (!decision.has_value()) {
            return false;
        }
        context.source_record_flags = decision->source_record_flags;
        for (std::uint16_t pass = 0U;
             pass < 128U && !decision->transition_complete; ++pass) {
            context.phase = ScriptActorHeartbeatPhase::render;
            const auto render_decision = scripts.actor_heartbeat(context);
            if (!render_decision.has_value()) {
                return false;
            }
            context.source_record_flags =
                render_decision->source_record_flags;
            context.phase = ScriptActorHeartbeatPhase::behavior;
            decision = scripts.actor_heartbeat(context);
            if (!decision.has_value()) {
                return false;
            }
            context.source_record_flags = decision->source_record_flags;
        }
        context.phase = ScriptActorHeartbeatPhase::behavior;
        return decision->transition_complete;
    };
    const auto prime_encounter_movement = [&scripts](
                                               ScriptActorBehaviorContext& context) {
        context.spawn = false;
        for (std::uint16_t pass = 0U; pass < 256U; ++pass) {
            context.phase = ScriptActorHeartbeatPhase::behavior;
            const auto behavior = scripts.actor_heartbeat(context);
            if (!behavior.has_value()) {
                return false;
            }
            context.source_record_flags = behavior->source_record_flags;
            if (behavior->movement_step.has_value()) {
                context.movement_state = behavior->movement_step->state;
                return true;
            }
            context.phase = ScriptActorHeartbeatPhase::render;
            const auto render = scripts.actor_heartbeat(context);
            if (!render.has_value()) {
                return false;
            }
            context.source_record_flags = render->source_record_flags;
        }
        return false;
    };

    for (std::uint8_t posture = 0U; posture < 9U; ++posture) {
        const std::uint16_t target_distance = posture < 3U ? 0x0700U
            : posture < 6U ? 0x0500U : 0x0300U;
        const auto posture_actor_id =
            "field_scene_2_encounter_posture_band_" +
            std::to_string(posture);
        auto posture_band_actor = make_encounter_reaction_context(
            posture_actor_id);
        posture_band_actor.profile_id = "weenr1_k08_none00";
        posture_band_actor.entity_kind = 8U;
        posture_band_actor.source_record_flags = 0x2DU;
        posture_band_actor.source_actor_posture = posture;
        posture_band_actor.entry_animation_frame_count = 2U;
        posture_band_actor.walk_animation_frame_count = 4U;
        configure_encounter_movement(posture_band_actor, target_distance);
        posture_band_actor.ai_target_candidates.front().world_activity_actor =
            true;
        posture_band_actor.ai_target_candidates.front().source_record_slot = 3U;
        const auto posture_band_spawn =
            scripts.actor_heartbeat(posture_band_actor);
        const bool posture_band_entered = posture_band_spawn.has_value() &&
            finish_encounter_entry(posture_band_actor);
        const bool posture_band_moved = posture_band_entered &&
            prime_encounter_movement(posture_band_actor);
        const auto posture_band_route = posture_band_moved
            ? scripts.actor_heartbeat(posture_band_actor)
            : std::nullopt;
        if (!expect(
                posture_band_route.has_value() &&
                    posture_band_actor.source_actor_posture ==
                        std::optional<std::uint8_t>{posture} &&
                    posture_band_actor.source_record_flags == 0x2DU &&
                    posture_band_route->movement_heading == 0x2000U &&
                    posture_band_route->movement_route_distance ==
                        target_distance,
                "out-of-range actor did not route at the linked record")) {
            std::cerr << scripts.failure() << '\n';
            std::cerr << "posture=" << static_cast<unsigned>(posture)
                      << '\n';
            return 24;
        }
    }

    auto quickness_actor = make_encounter_reaction_context(
        "field_scene_2_encounter_quickness_step");
    quickness_actor.profile_id = "weenr1_k08_none00";
    quickness_actor.entity_kind = 8U;
    quickness_actor.source_record_flags = 0x25U;
    quickness_actor.initial_heading = 0x2000U;
    quickness_actor.world_activity_active = true;
    quickness_actor.source_projection_active = true;
    ScriptCombatantFacts quickness_facts{};
    quickness_facts.command_attribute_78 = 4U;
    quickness_actor.combatant = quickness_facts;
    configure_encounter_movement(quickness_actor, 0x0700U);
    const bool quickness_entered =
        scripts.actor_heartbeat(quickness_actor).has_value() &&
        finish_encounter_entry(quickness_actor);
    const bool quickness_moved =
        quickness_entered && prime_encounter_movement(quickness_actor);
    if (!expect(
            quickness_moved &&
                quickness_actor.movement_state.position_x ==
                    static_cast<std::uint16_t>(0x1800U + 19U) &&
                quickness_actor.movement_state.position_y ==
                    static_cast<std::uint16_t>(0x1800U - 19U),
            "far actor with FFDF84 set did not step the Quickness 4 row of 0xBDF2")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }

    auto injured_actor_context = make_encounter_reaction_context(
        "field_scene_2_encounter_1_actor_0");
    const auto injured_actor_spawn =
        scripts.actor_heartbeat(injured_actor_context);
    if (!expect(injured_actor_spawn.has_value() &&
                    finish_encounter_entry(injured_actor_context),
                "Hell Hound did not finish its source C0B2 encounter-entry transition before combat")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    source_weapon_hit_context.projectile_combat_targets.push_back(
        ScriptProjectileCombatTarget{
            "field_scene_2_encounter_1_actor_0", 3U, victim_facts});
    source_weapon_hit_context.ai_random_bound =
        [](const std::uint16_t bound)
            -> std::optional<std::uint16_t> {
        return bound == 0U ? std::nullopt
                           : std::optional<std::uint16_t>{0U};
    };
    source_weapon_hit_context.projectile_terrain_probe =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptProjectileTerrainSample{0U, true, false};
        };
    source_weapon_hit_context.projectile_collision_search =
        [](const std::uint16_t, const std::uint16_t,
           const std::uint8_t) {
            return ScriptProjectileCollisionSearchResult{
                true, true, 45U, 77U,
                std::string{"field_scene_2_encounter_1_actor_0"}};
        };
    const auto source_weapon_hit_step =
        scripts.actor_heartbeat(source_weapon_hit_context);
    if (!expect(
            source_weapon_hit_step.has_value() &&
                source_weapon_hit_step->projectile_movement.has_value() &&
                source_weapon_hit_step->projectile_movement->state.action_result ==
                    0 &&
                source_weapon_hit_step->combat_meter_update.has_value() &&
                source_weapon_hit_step->combat_meter_update->target_id ==
                    "field_scene_2_encounter_1_actor_0" &&
                source_weapon_hit_step->combat_meter_update->source_record_slot ==
                    3U &&
                source_weapon_hit_step->combat_meter_update->record_type ==
                    3U &&
                source_weapon_hit_step->combat_meter_update->physical &&
                source_weapon_hit_step->combat_meter_update->changed &&
                source_weapon_hit_step->combat_meter_update->physical_meter ==
                    5U &&
                source_weapon_hit_step->combat_meter_update->mental_meter ==
                    8U,
            "Lua firearm path did not seed the projectile and apply the source Warhawk hit to its live target")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }

    injured_actor_context.spawn = false;
    injured_actor_context.phase = ScriptActorHeartbeatPhase::behavior;
    const auto injured_actor_reaction =
        scripts.actor_heartbeat(injured_actor_context);
    if (!expect(
            injured_actor_spawn.has_value() &&
                injured_actor_reaction.has_value() &&
                injured_actor_reaction->animation_id == "hurt" &&
                injured_actor_reaction->animation_frame == 0U &&
                injured_actor_reaction->source_record_flags == 0x25U &&
                !injured_actor_reaction->audio_event.has_value(),
            "stationary nonlethal firearm damage did not select directional C072")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    std::optional<ScriptActorBehaviorDecision> injured_hurt_sound;
    for (std::uint8_t pass = 0U; pass < 8U; ++pass) {
        injured_hurt_sound = scripts.actor_heartbeat(injured_actor_context);
    }
    if (!expect(injured_hurt_sound.has_value() &&
                    injured_hurt_sound->animation_id == "hurt" &&
                    injured_hurt_sound->audio_event == 0x0057U,
                "C072 hit sound did not fire after the ROM eight-service countdown")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    injured_hurt_sound = scripts.actor_heartbeat(injured_actor_context);
    if (!expect(injured_hurt_sound.has_value() &&
                    injured_hurt_sound->animation_id == "hurt" &&
                    !injured_hurt_sound->audio_event.has_value(),
                "C072 hit sound repeated after its one-shot source countdown")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    injured_actor_context.phase = ScriptActorHeartbeatPhase::render;
    std::optional<ScriptActorBehaviorDecision> injured_hurt_frame;
    for (std::uint8_t frame = 0U; frame < 5U; ++frame) {
        injured_hurt_frame = scripts.actor_heartbeat(injured_actor_context);
    }
    if (!expect(injured_hurt_frame.has_value() &&
                    injured_hurt_frame->animation_id == "hurt" &&
                    injured_hurt_frame->animation_frame == 0U,
                "C072 animation advanced before its 0x300/0x1000 cadence elapsed")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    injured_hurt_frame = scripts.actor_heartbeat(injured_actor_context);
    if (!expect(injured_hurt_frame.has_value() &&
                    injured_hurt_frame->animation_id == "hurt" &&
                    injured_hurt_frame->animation_frame == 1U,
                "C072 animation did not advance on its sixth render heartbeat")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }

    auto moving_actor_context = make_encounter_reaction_context(
        "field_scene_2_encounter_1_actor_2");
    configure_encounter_movement(moving_actor_context, 0x0700U);
    const auto moving_actor_spawn =
        scripts.actor_heartbeat(moving_actor_context);
    if (!expect(moving_actor_spawn.has_value() &&
                    finish_encounter_entry(moving_actor_context) &&
                    prime_encounter_movement(moving_actor_context),
                "moving Hell Hound did not enter and acquire its source-directed route before combat")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    auto moving_weapon_hit_context = source_weapon_hit_context;
    moving_weapon_hit_context.instance_id =
        "field_weapon_slot_combat_hit_moving_actor";
    moving_weapon_hit_context.projectile_combat_targets = {
        ScriptProjectileCombatTarget{
            "field_scene_2_encounter_1_actor_2", 3U, victim_facts}};
    moving_weapon_hit_context.projectile_collision_search =
        [](const std::uint16_t, const std::uint16_t,
           const std::uint8_t) {
            return ScriptProjectileCollisionSearchResult{
                true, true, 45U, 77U,
                std::string{"field_scene_2_encounter_1_actor_2"}};
        };
    const auto moving_weapon_hit =
        scripts.actor_heartbeat(moving_weapon_hit_context);
    moving_actor_context.phase = ScriptActorHeartbeatPhase::behavior;
    auto moving_actor_reaction =
        scripts.actor_heartbeat(moving_actor_context);
    if (moving_actor_reaction.has_value() &&
        moving_actor_reaction->movement_step.has_value()) {
        moving_actor_context.movement_state =
            moving_actor_reaction->movement_step->state;
    }
    if (!expect(moving_actor_spawn.has_value() &&
                    moving_weapon_hit.has_value() &&
                    moving_weapon_hit->combat_meter_update.has_value() &&
                    moving_weapon_hit->combat_meter_update->target_id ==
                        "field_scene_2_encounter_1_actor_2" &&
                    moving_actor_reaction.has_value() &&
                    moving_actor_reaction->animation_id == "walk" &&
                    moving_actor_reaction->movement_step.has_value() &&
                    !moving_actor_reaction->audio_event.has_value(),
                "moving Hell Hound did not continue its Lua route and select source C062 walk art after receiving C072 damage")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    std::optional<ScriptActorBehaviorDecision> moving_hurt_sound;
    for (std::uint8_t pass = 0U; pass < 8U; ++pass) {
        moving_hurt_sound = scripts.actor_heartbeat(moving_actor_context);
        if (moving_hurt_sound.has_value() &&
            moving_hurt_sound->movement_step.has_value()) {
            moving_actor_context.movement_state =
                moving_hurt_sound->movement_step->state;
        }
    }
    if (!expect(moving_hurt_sound.has_value() &&
                    moving_hurt_sound->animation_id == "walk" &&
                    moving_hurt_sound->movement_step.has_value() &&
                    moving_hurt_sound->audio_event == 0x0057U,
                "C072 callback countdown did not continue while the moving actor used its C062 walk pose")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }

    auto exhausted_actor_context = make_encounter_reaction_context(
        "field_scene_2_encounter_1_actor_1");
    const auto exhausted_actor_spawn =
        scripts.actor_heartbeat(exhausted_actor_context);
    if (!expect(exhausted_actor_spawn.has_value() &&
                    finish_encounter_entry(exhausted_actor_context),
                "lethal-hit Hell Hound did not finish its source C0B2 entry transition")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    auto lethal_hit_context = source_weapon_hit_context;
    lethal_hit_context.instance_id = "field_weapon_slot_lethal_hit";
    auto exhausted_victim_facts = victim_facts;
    exhausted_victim_facts.record_type = 4U;
    exhausted_victim_facts.physical_meter = 1U;
    lethal_hit_context.projectile_combat_targets = {
        ScriptProjectileCombatTarget{
            "field_scene_2_encounter_1_actor_1", 4U,
            exhausted_victim_facts}};
    lethal_hit_context.projectile_collision_search =
        [](const std::uint16_t, const std::uint16_t,
           const std::uint8_t) {
            return ScriptProjectileCollisionSearchResult{
                true, true, 45U, 77U,
                std::string{"field_scene_2_encounter_1_actor_1"}};
        };
    const auto lethal_hit_step = scripts.actor_heartbeat(lethal_hit_context);
    if (!expect(
            exhausted_actor_spawn.has_value() && lethal_hit_step.has_value() &&
                lethal_hit_step->combat_meter_update.has_value() &&
                lethal_hit_step->combat_meter_update->changed &&
                lethal_hit_step->combat_meter_update->physical_meter == 0U,
            "lethal firearm hit did not reduce the live encounter actor to zero physical meter")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    exhausted_actor_context.spawn = false;
    exhausted_actor_context.phase = ScriptActorHeartbeatPhase::behavior;
    const auto exhausted_actor_reaction =
        scripts.actor_heartbeat(exhausted_actor_context);
    if (!expect(
            exhausted_actor_reaction.has_value() &&
                exhausted_actor_reaction->animation_id == "down" &&
                exhausted_actor_reaction->animation_frame == 0U &&
                exhausted_actor_reaction->source_record_flags == 0x35U &&
                exhausted_actor_reaction->audio_event == 0x0056U,
            "zero physical meter did not enter the C0B2 exhausted transition and B05E cue")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    exhausted_actor_context.source_record_flags =
        exhausted_actor_reaction->source_record_flags;
    exhausted_actor_context.phase = ScriptActorHeartbeatPhase::render;
    std::optional<ScriptActorBehaviorDecision> exhausted_actor_frame;
    for (std::uint8_t frame = 0U; frame < 7U; ++frame) {
        exhausted_actor_frame =
            scripts.actor_heartbeat(exhausted_actor_context);
    }
    if (!expect(exhausted_actor_frame.has_value() &&
                    exhausted_actor_frame->animation_id == "down" &&
                    exhausted_actor_frame->animation_frame == 0U,
                "C0B2 exhausted transition advanced before its eight-render cadence")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    exhausted_actor_frame = scripts.actor_heartbeat(exhausted_actor_context);
    if (!expect(exhausted_actor_frame.has_value() &&
                    exhausted_actor_frame->animation_id == "down" &&
                    exhausted_actor_frame->animation_frame == 1U,
                "C0B2 exhausted transition did not advance on the eighth render heartbeat")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }
    for (std::uint8_t frame = 0U; frame < 8U; ++frame) {
        exhausted_actor_frame =
            scripts.actor_heartbeat(exhausted_actor_context);
    }
    if (!expect(exhausted_actor_frame.has_value() &&
                    exhausted_actor_frame->animation_id == "down" &&
                    exhausted_actor_frame->animation_frame == 1U &&
                    exhausted_actor_frame->source_record_flags == 0x35U,
                "C0B2 exhausted transition did not retain its final frame and reserved actor flag")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }

    auto unresolved_weapon_seed = moving_projectile;
    auto unresolved_weapon_context = projectile_behavior_context(
        "field_weapon_slot_unresolved_seed", unresolved_weapon_seed);
    unresolved_weapon_context.projectile_action_code = 5U;
    unresolved_weapon_context.projectile_attack_seed_pending = true;
    shooter_facts.action_profile_modifier_flags = 0x01U;
    unresolved_weapon_context.combatant = shooter_facts;
    unresolved_weapon_context.ai_random_bound =
        [](const std::uint16_t bound)
            -> std::optional<std::uint16_t> {
        return bound == 0U ? std::nullopt
                           : std::optional<std::uint16_t>{0U};
    };
    unresolved_weapon_context.projectile_terrain_probe =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptProjectileTerrainSample{0U, true, false};
        };
    unresolved_weapon_context.projectile_collision_search =
        [](const std::uint16_t x, const std::uint16_t y,
           const std::uint8_t) {
            return ScriptProjectileCollisionSearchResult{
                false, false, x, y, std::nullopt};
        };
    const auto unresolved_weapon_step =
        scripts.actor_heartbeat(unresolved_weapon_context);
    if (!expect(
            unresolved_weapon_step.has_value() &&
                unresolved_weapon_step->projectile_movement.has_value() &&
                unresolved_weapon_step->projectile_movement->state.active &&
                unresolved_weapon_step->projectile_movement->outcome ==
                    ScriptActorProjectileOutcome::none &&
                !unresolved_weapon_step->combat_meter_update.has_value(),
            "unresolved linked weapon profile retired the projectile or invented a hit result")) {
        std::cerr << scripts.failure() << '\n';
        return 24;
    }

    auto secondary_projectile = moving_projectile;
    secondary_projectile.role = ScriptActorProjectileRole::secondary;
    secondary_projectile.animation_selector = 0x8000U;
    auto secondary_projectile_context = projectile_behavior_context(
        "field_weapon_slot_secondary", secondary_projectile);
    secondary_projectile_context.projectile_terrain_probe =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptProjectileTerrainSample{};
        };
    secondary_projectile_context.projectile_collision_search =
        [](const std::uint16_t, const std::uint16_t,
           const std::uint8_t) {
            return ScriptProjectileCollisionSearchResult{};
        };
    const auto secondary_projectile_step =
        scripts.actor_heartbeat(secondary_projectile_context);
    if (!expect(
            secondary_projectile_step.has_value() &&
                secondary_projectile_step->projectile_movement.has_value() &&
                !secondary_projectile_step->projectile_movement->state.active &&
                secondary_projectile_step->projectile_movement->state.flags == 0U &&
                secondary_projectile_step->projectile_movement->outcome ==
                    ScriptActorProjectileOutcome::retired &&
                !secondary_projectile_step->audio_event.has_value(),
            "Lua secondary projectile did not retire after its finite list completed")) {
        std::cerr << scripts.failure() << '\n';
        return 25;
    }

    auto stopped_projectile = moving_projectile;
    stopped_projectile.record_flags = 0x08U;
    stopped_projectile.animation_selector = 0x8000U;
    stopped_projectile.linked_action_timer = 0;
    auto stopped_projectile_context = projectile_behavior_context(
        "field_weapon_slot_stopped", stopped_projectile);
    stopped_projectile_context.projectile_terrain_probe =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptProjectileTerrainSample{};
        };
    stopped_projectile_context.projectile_collision_search =
        [](const std::uint16_t, const std::uint16_t,
           const std::uint8_t) {
            return ScriptProjectileCollisionSearchResult{};
        };
    const auto stopped_projectile_step =
        scripts.actor_heartbeat(stopped_projectile_context);
    if (!expect(
            stopped_projectile_step.has_value() &&
                stopped_projectile_step->projectile_movement.has_value() &&
                stopped_projectile_step->projectile_movement->state.linked_action_timer ==
                    -1 &&
                !stopped_projectile_step->projectile_movement->state.active &&
                stopped_projectile_step->projectile_movement->outcome ==
                    ScriptActorProjectileOutcome::retired,
            "Lua stopped projectile did not retire after its source countdown expired")) {
        std::cerr << scripts.failure() << '\n';
        return 26;
    }

    auto burst_projectile = stopped_projectile;
    burst_projectile.record_flags = 0x09U;
    burst_projectile.linked_action_timer = 2;
    burst_projectile.heading = 0x2000U;
    burst_projectile.saved_position_x = 0x2345U;
    burst_projectile.saved_position_y = 0x3456U;
    burst_projectile.world_x = 20 * 0x10000 + 0xABCD;
    burst_projectile.world_y = 30 * 0x10000 + 0x1357;
    auto linked_projectile_context = projectile_behavior_context(
        "field_weapon_slot_linked", burst_projectile);
    linked_projectile_context.projectile_action_code = 8U;
    linked_projectile_context.combatant =
        source_weapon_hit_context.combatant;
    linked_projectile_context.projectile_animation->increment = 0x270f;
    linked_projectile_context.projectile_animation->period = 0x1000;
    std::uint32_t burst_attack_seed_rolls = 0U;
    linked_projectile_context.ai_random_bound =
        [&burst_attack_seed_rolls](const std::uint16_t bound)
            -> std::optional<std::uint16_t> {
        ++burst_attack_seed_rolls;
        return bound == 0U ? std::nullopt
                           : std::optional<std::uint16_t>{0U};
    };
    linked_projectile_context.projectile_terrain_probe =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptProjectileTerrainSample{};
        };
    linked_projectile_context.projectile_collision_search =
        [](const std::uint16_t, const std::uint16_t,
           const std::uint8_t) {
            return ScriptProjectileCollisionSearchResult{};
        };
    const auto linked_projectile_step =
        scripts.actor_heartbeat(linked_projectile_context);
    const auto first_burst = linked_projectile_step.has_value() &&
        linked_projectile_step->projectile_movement.has_value()
        ? linked_projectile_step->projectile_movement->state
        : ScriptActorProjectileMovement{};
    const auto first_burst_source_offsets =
        shadowrun::ghidra_conversion::derive_entity_heading_offsets(0x1c00U);
    if (!expect(
            linked_projectile_step.has_value() &&
                linked_projectile_step->projectile_movement.has_value() &&
                first_burst.linked_action_timer == 1 &&
                first_burst.active && first_burst.heading == 0x1c00U &&
                first_burst.world_x ==
                    static_cast<std::int32_t>(0x2345abcdU) &&
                first_burst.world_y ==
                    static_cast<std::int32_t>(0x34561357U) &&
                first_burst.tile_offset_x ==
                    first_burst_source_offsets.horizontal &&
                first_burst.tile_offset_y ==
                    first_burst_source_offsets.vertical &&
                first_burst.retry_counter == 0x20U &&
                first_burst.animation_selector == 0x14U &&
                first_burst.record_flags == 0x01U &&
                first_burst.action_result == 0 &&
                linked_projectile_step->projectile_movement->outcome ==
                    ScriptActorProjectileOutcome::burst_followup_restarted &&
                burst_attack_seed_rolls > 0U,
            "Lua did not restart the first source AK-97 burst ray at the saved origin with its -0x400 heading and fresh attack seed")) {
        std::cerr << scripts.failure() << '\n';
        return 27;
    }

    linked_projectile_context.spawn = false;
    linked_projectile_context.phase = ScriptActorHeartbeatPhase::render;
    linked_projectile_context.projectile_movement.reset();
    linked_projectile_context.projectile_animation->selector = 0x14U;
    linked_projectile_context.projectile_animation->frame = 0U;
    linked_projectile_context.projectile_animation->accumulator = 0;
    const auto burst_restart_frame =
        scripts.actor_heartbeat(linked_projectile_context);
    if (!expect(burst_restart_frame.has_value() &&
                    burst_restart_frame->animation_frame == 1U &&
                    burst_restart_frame->animation_selector == 0x14U,
                "Lua did not restart the burst animation clock from frame zero")) {
        std::cerr << scripts.failure() << '\n';
        return 27;
    }

    auto second_burst_state = first_burst;
    second_burst_state.record_flags = 0x09U;
    second_burst_state.animation_selector = 0x8014U;
    linked_projectile_context.phase = ScriptActorHeartbeatPhase::behavior;
    linked_projectile_context.projectile_movement = second_burst_state;
    const auto second_burst_step =
        scripts.actor_heartbeat(linked_projectile_context);
    const auto second_burst = second_burst_step.has_value() &&
        second_burst_step->projectile_movement.has_value()
        ? second_burst_step->projectile_movement->state
        : ScriptActorProjectileMovement{};
    const auto second_burst_source_offsets =
        shadowrun::ghidra_conversion::derive_entity_heading_offsets(0x2400U);
    if (!expect(second_burst_step.has_value() &&
                    second_burst_step->projectile_movement.has_value() &&
                    second_burst.linked_action_timer == 0 &&
                    second_burst.heading == 0x2400U &&
                    second_burst.tile_offset_x ==
                        second_burst_source_offsets.horizontal &&
                    second_burst.tile_offset_y ==
                        second_burst_source_offsets.vertical &&
                    second_burst.animation_selector == 0x14U &&
                    second_burst.record_flags == 0x01U &&
                    second_burst_step->projectile_movement->outcome ==
                        ScriptActorProjectileOutcome::burst_followup_restarted &&
                    burst_attack_seed_rolls > 1U,
                "Lua did not restart the final source AK-97 burst ray at the +0x800 heading and reroll its action pair")) {
        std::cerr << scripts.failure() << '\n';
        std::cerr << "second burst: present=" << second_burst_step.has_value()
                  << " state=" << (second_burst_step.has_value() &&
                         second_burst_step->projectile_movement.has_value())
                  << " timer=" << static_cast<int>(second_burst.linked_action_timer)
                  << " heading=0x" << std::hex << second_burst.heading
                  << " offset=" << std::dec << second_burst.tile_offset_x
                  << ',' << second_burst.tile_offset_y
                  << " selector=0x" << std::hex << second_burst.animation_selector
                  << " flags=0x" << static_cast<unsigned>(second_burst.record_flags)
                  << " outcome=" << (second_burst_step.has_value() &&
                         second_burst_step->projectile_movement.has_value()
                         ? static_cast<unsigned>(second_burst_step->projectile_movement->outcome)
                         : 999U)
                  << " rng=" << std::dec << burst_attack_seed_rolls << '\n';
        return 27;
    }

    auto final_burst_state = second_burst;
    final_burst_state.record_flags = 0x09U;
    final_burst_state.animation_selector = 0x8014U;
    linked_projectile_context.projectile_movement = final_burst_state;
    const auto final_burst_step =
        scripts.actor_heartbeat(linked_projectile_context);
    if (!expect(final_burst_step.has_value() &&
                    final_burst_step->projectile_movement.has_value() &&
                    final_burst_step->projectile_movement->state.linked_action_timer ==
                        -1 &&
                    !final_burst_step->projectile_movement->state.active &&
                    final_burst_step->projectile_movement->outcome ==
                        ScriptActorProjectileOutcome::retired,
                "Lua did not retire the AK-97 primary after exactly three source burst rays")) {
        std::cerr << scripts.failure() << '\n';
        return 27;
    }

    auto source_table_sample = stopped_projectile;
    source_table_sample.record_flags = 0x09U;
    source_table_sample.linked_action_timer = 2;
    source_table_sample.heading = 0xE200U;
    source_table_sample.saved_position_x = 0x2345U;
    source_table_sample.saved_position_y = 0x3456U;
    source_table_sample.animation_selector = 0x8000U;
    linked_projectile_context.projectile_movement = source_table_sample;
    const auto questioned_source_sample_step =
        scripts.actor_heartbeat(linked_projectile_context);
    if (!expect(questioned_source_sample_step.has_value() &&
                    questioned_source_sample_step->projectile_movement.has_value() &&
                    questioned_source_sample_step->projectile_movement->state.heading ==
                        0xDE00U &&
                    questioned_source_sample_step->projectile_movement->state.tile_offset_x ==
                        -18119 &&
                    questioned_source_sample_step->projectile_movement->state.tile_offset_y ==
                        -16503,
                "Lua heading helper did not preserve ROM table entry 111 and its companion sample")) {
        std::cerr << scripts.failure() << '\n';
        if (questioned_source_sample_step.has_value() &&
            questioned_source_sample_step->projectile_movement.has_value()) {
            const auto& state =
                questioned_source_sample_step->projectile_movement->state;
            std::cerr << "table-sample heading=0x" << std::hex << state.heading
                      << " offsets=" << std::dec << state.tile_offset_x << ','
                      << state.tile_offset_y << " active=" << state.active
                      << " timer=" << static_cast<int>(state.linked_action_timer)
                      << " outcome=" << static_cast<unsigned>(
                             questioned_source_sample_step->projectile_movement->outcome)
                      << '\n';
        } else {
            std::cerr << "table-sample decision present="
                      << questioned_source_sample_step.has_value()
                      << " movement present="
                      << (questioned_source_sample_step.has_value() &&
                          questioned_source_sample_step->projectile_movement.has_value())
                      << '\n';
        }
        return 27;
    }

    const auto player_step_context = [](
                                        const std::string_view actor_id,
                                        const std::uint16_t initial_heading,
                                        const std::uint16_t target_heading) {
        ScriptActorBehaviorContext context{};
        context.profile_id = "field_runner_k03_none00";
        context.entity_kind = 3U;
        context.instance_id = actor_id;
        context.initial_heading = initial_heading;
        context.spawn = true;
        context.movement_allowed = true;
        context.controller_nibble = 1U;
        context.controller_heading_table.fill(0xFFFFU);
        context.controller_heading_table[1] = target_heading;
        context.movement_services_available = true;
        context.movement_state.position_x = 100U;
        context.movement_state.position_y = 100U;
        context.movement_state.heading = initial_heading;
        context.movement_state.animation_heading = initial_heading;
        context.movement_state.movement_substate = 7U;
        context.movement_state.route_status = 9;
        context.movement_state.route_flags = 0x03U;
        context.movement_state.movement_flags = 0x08U;
        context.movement_state.collision_flags = 0x01U;
        context.movement_state.event_flags = 0x01U;
        context.movement_offsets = {{
            {10, 0}, {0, 10}, {20, 0}, {0, 20},
            {-10, 0}, {0, -10}, {-20, 0}, {0, -20},
        }};
        context.movement_idle_selectors = {{
            0x20U, 0x21U, 0x22U, 0x23U,
            0x24U, 0x25U, 0x26U, 0x27U,
        }};
        context.movement_collision_recovery =
            [](const std::uint16_t, const std::uint16_t,
               const std::int16_t, const std::uint8_t) {
                return ScriptMovementCollisionRecoveryResult{false, 0U};
            };
        return context;
    };

    auto clear_step_context = player_step_context(
        "movement_clear", 0U, 0U);
    clear_step_context.movement_entity_flags = 0x08U;
    clear_step_context.movement_state.route_flags = 0x23U;
    std::size_t clear_collision_queries = 0U;
    clear_step_context.movement_collision =
        [&clear_collision_queries](const std::uint16_t x,
                                   const std::uint16_t y) {
            ++clear_collision_queries;
            const auto code = static_cast<std::int16_t>(
                x == 110U && y == 100U ? 0 : 1);
            return ScriptMovementCollisionResult{
                code, static_cast<std::uint16_t>(code)};
        };
    clear_step_context.movement_route_lookup =
        [](const std::uint16_t) { return ScriptMovementRouteFacts{}; };
    const auto clear_step = scripts.actor_heartbeat(clear_step_context);

    auto route_step_context = player_step_context(
        "movement_route_fallback", 0x4000U, 0x4000U);
    std::size_t route_collision_queries = 0U;
    std::uint16_t observed_route_key = 0xFFFFU;
    route_step_context.movement_collision =
        [&route_collision_queries](const std::uint16_t x,
                                   const std::uint16_t y) {
            ++route_collision_queries;
            const auto code = static_cast<std::int16_t>(
                x == 120U && y == 100U ? 1 : 0);
            return ScriptMovementCollisionResult{
                code, static_cast<std::uint16_t>(code)};
        };
    route_step_context.movement_route_lookup =
        [&observed_route_key](const std::uint16_t key) {
            observed_route_key = key;
            return ScriptMovementRouteFacts{0, 1, 0U, 4U};
        };
    const auto route_step = scripts.actor_heartbeat(route_step_context);

    auto slide_step_context = player_step_context(
        "movement_diagonal_slide", 0x2000U, 0x2000U);
    std::size_t slide_collision_queries = 0U;
    slide_step_context.movement_collision =
        [&slide_collision_queries](const std::uint16_t x,
                                   const std::uint16_t y) {
            ++slide_collision_queries;
            if ((x == 100U && y == 110U) ||
                (x == 110U && y == 100U)) {
                return ScriptMovementCollisionResult{1, 1U};
            }
            return ScriptMovementCollisionResult{0, 0U};
        };
    slide_step_context.movement_route_lookup =
        [](const std::uint16_t) { return ScriptMovementRouteFacts{}; };
    const auto slide_step = scripts.actor_heartbeat(slide_step_context);

    auto pending_event_context = player_step_context(
        "movement_pending_world_event", 0U, 0U);
    pending_event_context.movement_entity_flags = 0x02U;
    pending_event_context.movement_state.route_flags = 0U;
    pending_event_context.movement_collision =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptMovementCollisionResult{-62, 0xFFC2U};
        };
    std::size_t pending_event_recoveries = 0U;
    pending_event_context.movement_collision_recovery =
        [&pending_event_recoveries](const std::uint16_t, const std::uint16_t,
                                    const std::int16_t,
                                    const std::uint8_t) {
            ++pending_event_recoveries;
            return ScriptMovementCollisionRecoveryResult{false, 0x20U};
        };
    pending_event_context.movement_route_lookup =
        [](const std::uint16_t) { return ScriptMovementRouteFacts{}; };
    const auto pending_event_step =
        scripts.actor_heartbeat(pending_event_context);

    auto reset_step_context = player_step_context(
        "movement_blocked_reset", 0x2000U, 0x4000U);
    std::size_t reset_collision_queries = 0U;
    reset_step_context.movement_collision =
        [&reset_collision_queries](const std::uint16_t x,
                                   const std::uint16_t y) {
            ++reset_collision_queries;
            const auto code = static_cast<std::int16_t>(
                x == 100U && y == 100U ? 0 : 1);
            return ScriptMovementCollisionResult{
                code, static_cast<std::uint16_t>(code)};
        };
    reset_step_context.movement_route_lookup =
        [](const std::uint16_t) { return ScriptMovementRouteFacts{}; };
    const auto reset_step = scripts.actor_heartbeat(reset_step_context);
    if (!expect(
            clear_step.has_value() && clear_step->movement_step.has_value() &&
                clear_step->movement_step->outcome ==
                    ScriptActorMovementStep::Outcome::accepted &&
                clear_step->movement_step->state.position_x == 110U &&
                clear_step->movement_step->state.event_flags == 0U &&
                clear_step->movement_step->state.route_flags == 0x03U &&
                clear_collision_queries == 1U &&
                route_step.has_value() && route_step->movement_step.has_value() &&
                route_step->movement_step->state.position_x == 110U &&
                route_step->movement_step->state.collision_flags == 0U &&
                observed_route_key == 4U && route_collision_queries == 2U &&
                slide_step.has_value() && slide_step->movement_step.has_value() &&
                slide_step->movement_step->state.position_x == 120U &&
                slide_step->movement_step->state.route_status == 0 &&
                slide_step->movement_step->state.movement_flags == 0x0CU &&
                slide_collision_queries == 3U &&
                pending_event_step.has_value() &&
                pending_event_step->movement_step.has_value() &&
                pending_event_step->movement_step->state.route_flags == 0x20U &&
                pending_event_step->movement_step->state.position_x == 110U &&
                pending_event_recoveries == 1U &&
                reset_step.has_value() && reset_step->movement_step.has_value() &&
                reset_step->movement_step->outcome ==
                    ScriptActorMovementStep::Outcome::reset &&
                reset_step->movement_current_heading == 0x4000U &&
                reset_step->movement_step->state.route_flags == 0x01U &&
                reset_step->movement_step->state.movement_flags == 0x09U &&
                reset_step->movement_step->state.route_status == -1 &&
                reset_step->movement_step->direction_animation == 0x22U &&
                clear_step->movement_step->direction_animation == 0x40U &&
                route_step->movement_step->direction_animation == 0x42U &&
                reset_collision_queries == 4U,
            "Lua movement policy did not preserve event retry state or resolve clear, route, slide, and reset cases")) {
        std::cerr << scripts.failure() << '\n';
        return 18;
    }

    const auto accept_attribute_step =
        [](const std::uint16_t, const std::uint16_t) {
            return ScriptMovementCollisionResult{0, 0U};
        };
    const auto attribute_move = [&](const std::string& actor_id,
                                    const std::uint8_t quickness,
                                    const bool activity,
                                    const bool on_screen) {
        auto context = player_step_context(actor_id, 0x4000U, 0x4000U);
        context.movement_entity_flags = 0x0FU;
        context.world_activity_active = activity;
        context.source_projection_active = on_screen;
        ScriptCombatantFacts facts{};
        facts.command_attribute_78 = quickness;
        context.combatant = facts;
        context.movement_collision = accept_attribute_step;
        context.movement_route_lookup =
            [](const std::uint16_t) { return ScriptMovementRouteFacts{}; };
        return scripts.actor_heartbeat(context);
    };
    const auto calm_runner = attribute_move(
        "movement_attribute_calm", 3U, false, true);
    const auto alert_quickness_3 = attribute_move(
        "movement_attribute_q3", 3U, true, true);
    const auto alert_quickness_5 = attribute_move(
        "movement_attribute_q5", 5U, true, true);
    const auto offscreen_runner = attribute_move(
        "movement_attribute_offscreen", 3U, true, false);
    const auto stepped_x = [](const std::optional<ScriptActorBehaviorDecision>&
                                  step) -> std::optional<std::uint16_t> {
        if (!step.has_value() || !step->movement_step.has_value() ||
            step->movement_step->outcome !=
                ScriptActorMovementStep::Outcome::accepted) {
            return std::nullopt;
        }
        return step->movement_step->state.position_x;
    };
    if (!expect(
            stepped_x(calm_runner) == std::optional<std::uint16_t>{130U} &&
                stepped_x(alert_quickness_3) ==
                    std::optional<std::uint16_t>{116U} &&
                stepped_x(alert_quickness_5) ==
                    std::optional<std::uint16_t>{122U} &&
                stepped_x(offscreen_runner) ==
                    std::optional<std::uint16_t>{140U},
            "0xB574 did not step the on-screen runner at 30, Quickness 3/5 at 16/22, or the off-screen runner at 40")) {
        std::cerr << scripts.failure() << '\n';
        return 18;
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

    const auto* resource_main_title =
        scripts.dialogue_text("field.activity.main_menu.title");
    const auto* resource_main_cyberspace = scripts.dialogue_text(
        "field.activity.main_menu.option.cyberspace");
    const auto* resource_main_exit =
        scripts.dialogue_text("field.activity.main_menu.option.exit");
    if (!expect(resource_main_title != nullptr &&
                    *resource_main_title == "Main Menu" &&
                    resource_main_cyberspace != nullptr &&
                    *resource_main_cyberspace == "Cyberspace" &&
                    resource_main_exit != nullptr &&
                    *resource_main_exit == "Exit",
                "Lua resource-scene main-menu labels were not preserved by extraction")) {
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
    const auto little_chiba_intro = mode_prompt_content(2U, 1U, 0);
    const auto little_chiba_handoff = mode_prompt_content(2U, 1U, -1);
    const auto little_chiba_payment = mode_prompt_content(2U, 1U, 1);
    const auto unmatched_mode_prompt = mode_prompt_content(2U, 2U, 0);
    const auto valid_mode_prompt = [&](const auto& content,
                                       const std::string_view text_key,
                                       const std::string_view portrait_alias) {
        return content.has_value() && content->size() == 3U &&
               content->at("status") == "matched" &&
               content->at("text_key") == text_key &&
               content->at("portrait_alias") == portrait_alias &&
               scripts.dialogue_text(text_key) != nullptr;
    };
    if (!expect(valid_mode_prompt(ares_intro,
                                  "field.mode.ares.store_intro",
                                  "field.mode.entry.ares.sprite") &&
                    valid_mode_prompt(ares_handoff,
                                      "field.mode.ares.package_handoff",
                                      "field.mode.entry.ares.sprite") &&
                    valid_mode_prompt(ares_payment,
                                      "field.mode.ares.package_payment",
                                      "field.mode.entry.ares.sprite") &&
                    valid_mode_prompt(
                        little_chiba_intro,
                        "field.mode.little_chiba.description",
                        "field.mode.entry.little.chiba.sprite") &&
                    valid_mode_prompt(
                        little_chiba_handoff,
                        "field.mode.little_chiba.package_handoff",
                        "field.mode.entry.little.chiba.sprite") &&
                    valid_mode_prompt(
                        little_chiba_payment,
                        "field.mode.little_chiba.package_payment",
                        "field.mode.entry.little.chiba.sprite") &&
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
    const auto startup_archetype_portraits =
        scripts.string_map("startup", "archetype_portraits");
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
                    startup_archetype_portraits.has_value() &&
                    startup_archetype_portraits->size() == 3U &&
                    startup_archetype_portraits->at("samurai") ==
                        "menu.field.runner.portrait.0" &&
                    startup_archetype_portraits->at("decker") ==
                        "menu.field.runner.portrait.1" &&
                    startup_archetype_portraits->at("gator_shaman") ==
                        "menu.field.runner.portrait.2" &&
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

    const auto maglock_intro_menu = scripts.menu(
        "field_interactions", "maglock_intro", "field.maglock", "door");
    const auto maglock_actions_menu = scripts.menu(
        "field_interactions", "maglock_actions", "field.interaction",
        "maglock_actions");
    const auto security_actions_menu = scripts.menu(
        "field_interactions", "security_terminal_actions",
        "field.interaction", "security_actions");
    if (!expect(maglock_intro_menu.has_value() &&
                    maglock_intro_menu->choices.size() == 1U &&
                    maglock_intro_menu->choices[0].id == "more" &&
                    maglock_intro_menu->choices[0].label_key ==
                        "field.maglock.option.more" &&
                    maglock_actions_menu.has_value() &&
                    maglock_actions_menu->choices.size() == 3U &&
                    maglock_actions_menu->choices[0].label_key ==
                        "field.interaction.option.electronics" &&
                    maglock_actions_menu->choices[1].label_key ==
                        "field.interaction.option.grenade" &&
                    maglock_actions_menu->choices[2].label_key ==
                        "field.interaction.option.walk-away" &&
                    security_actions_menu.has_value() &&
                    security_actions_menu->choices.size() == 2U &&
                    security_actions_menu->choices[0].label_key ==
                        "field.interaction.option.electronics" &&
                    security_actions_menu->choices[1].label_key ==
                        "field.interaction.option.walk-away",
                "field-interaction menus did not return explicit Lua text keys")) {
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
    constexpr std::array<std::string_view, 22> kClipsTextIds{{
        "header_title", "header_separator", "counter_label",
        "trade_prompt_action", "trade_prompt_clips_to",
        "trade_prompt_recipient", "exchange_complete_first",
        "exchange_complete_second", "reload_prompt_ammo_remaining",
        "reload_prompt_ammo_suffix", "reload_prompt_question",
        "reload_success_first", "reload_success_second",
        "out_of_ammo_name_suffix", "out_of_ammo_clips",
        "item_without_clips_first", "item_without_clips_second",
        "item_without_clips_third", "no_party_first", "no_party_second",
        "no_party_third", "function_aborted"}};
    constexpr std::array<std::string_view, 22> kClipsTextLabelKeys{{
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
        "clips.text.option.function_aborted",
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
    constexpr std::array<std::string_view, 2> kConfirmationLabelKeys{{
        "ui.option.yes", "ui.option.no"}};
    constexpr std::array<std::string_view, 2> kConfirmationLabels{{
        "Yes", "No"}};
    bool confirmation_rows_match = confirmation.has_value() &&
        confirmation->title_key.empty() &&
        confirmation->choices.size() == kConfirmationIds.size();
    if (confirmation_rows_match) {
        for (std::size_t index = 0; index < kConfirmationIds.size(); ++index) {
            const auto& choice = confirmation->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            confirmation_rows_match = confirmation_rows_match &&
                choice.id == kConfirmationIds[index] && label != nullptr &&
                choice.label_key == kConfirmationLabelKeys[index] &&
                *label == kConfirmationLabels[index];
        }
    }
    if (!expect(confirmation_rows_match,
                "Lua confirmation choices or ROM labels changed order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto field_metrics = scripts.menu(
        "ui", "field_metric_labels", "ui.field_metric", "catalog");
    constexpr std::array<std::string_view, 11> kFieldMetricIds{{
        "physical", "mental", "attack", "defense", "damage", "drain",
        "none", "no_armor", "weapons", "guns", "power"}};
    constexpr std::array<std::string_view, 11> kFieldMetricLabelKeys{{
        "ui.field_metric.option.physical",
        "ui.field_metric.option.mental",
        "ui.field_metric.option.attack",
        "ui.field_metric.option.defense",
        "ui.field_metric.option.damage",
        "ui.field_metric.option.drain",
        "ui.field_metric.option.none",
        "ui.field_metric.option.no_armor",
        "ui.field_metric.option.weapons",
        "ui.field_metric.option.guns",
        "ui.field_metric.option.power"}};
    constexpr std::array<std::string_view, 11> kFieldMetricLabels{{
        "Physical", "Mental", "Atk", "Def", "Damage", "Drain",
        "None", "No Armor", "Wpns", "Guns", "Power"}};
    bool field_metric_rows_match = field_metrics.has_value() &&
        field_metrics->title_key.empty() &&
        field_metrics->choices.size() == kFieldMetricIds.size();
    if (field_metric_rows_match) {
        for (std::size_t index = 0; index < kFieldMetricIds.size(); ++index) {
            const auto& choice = field_metrics->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            field_metric_rows_match = field_metric_rows_match &&
                choice.id == kFieldMetricIds[index] &&
                choice.label_key == kFieldMetricLabelKeys[index] &&
                label != nullptr && *label == kFieldMetricLabels[index];
        }
    }
    if (!expect(field_metric_rows_match,
                "Lua field metric keys or ROM labels changed source order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto attributes_skills_text = scripts.menu(
        "attributes_skills", "text_entries", "attributes_skills.text",
        "catalog");
    constexpr std::array<std::string_view, 25> kAttributesSkillsTextIds{{
        "body", "quickness", "strength", "charisma", "intelligence",
        "willpower", "essence", "magic", "sorcery", "firearms",
        "pistols", "smgs", "shotguns", "melee_combat", "throwing",
        "computer", "biotech", "electronics", "reputation", "negotiation",
        "upgrade_header", "maximum_level", "mage_only", "insufficient_karma",
        "upgrade_prompt"}};
    constexpr std::array<std::string_view, 25> kAttributesSkillsTextLabels{{
        "Body", "Quickness", "Strength", "Charisma", "Intelligence",
        "Willpower", "Essence      .", "Magic", "Sorcery", "Firearms",
        "Pistols", "SMGs", "Shotguns", "Melee Combat", "Throwing",
        "Computer", "Biotech", "Electronics", "Reputation", "Negotiation",
        "Upgrade to       Cost", "You are at the maximum level.",
        "This skill is for mages", "You do not have enough", "Upgrade?"}};
    bool attributes_skills_text_matches = attributes_skills_text.has_value() &&
        attributes_skills_text->title_key.empty() &&
        attributes_skills_text->choices.size() == kAttributesSkillsTextIds.size();
    if (attributes_skills_text_matches) {
        for (std::size_t index = 0;
             index < kAttributesSkillsTextIds.size(); ++index) {
            const auto& choice = attributes_skills_text->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            const auto expected_key = std::string(
                "attributes_skills.text.option.") +
                std::string(kAttributesSkillsTextIds[index]);
            attributes_skills_text_matches =
                attributes_skills_text_matches &&
                choice.id == kAttributesSkillsTextIds[index] &&
                choice.label_key == expected_key && label != nullptr &&
                *label == kAttributesSkillsTextLabels[index];
        }
    }
    if (!expect(attributes_skills_text_matches,
                "Lua Attributes/Skills keys or ROM text changed source order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto cyberware_text = scripts.menu(
        "cyberware", "text_entries", "cyberware.text", "catalog");
    constexpr std::array<std::string_view, 18> kCyberwareTextIds{{
        "title", "separator", "empty", "datajack", "cyber_eyes",
        "hand_razors", "spurs", "smartlink", "muscle_replace_1",
        "muscle_replace_2", "muscle_replace_3", "muscle_replace_4",
        "dermal_plating_1", "dermal_plating_2", "dermal_plating_3",
        "wired_reflexes_1", "wired_reflexes_2", "wired_reflexes_3"}};
    constexpr std::array<std::string_view, 18> kCyberwareTextLabels{{
        " CYBERWARE", "", "NONE", "    Datajack", "    CyberEyes",
        "   Hand Razors", "      Spurs", "    Smartlink",
        "Muscle Replace(1)", "Muscle Replace(2)", "Muscle Replace(3)",
        "Muscle Replace(4)", "Dermal Plating(1)", "Dermal Plating(2)",
        "Dermal Plating(3)", "Wired Reflexes(1)", "Wired Reflexes(2)",
        "Wired Reflexes(3)"}};
    bool cyberware_text_matches = cyberware_text.has_value() &&
        cyberware_text->title_key.empty() &&
        cyberware_text->choices.size() == kCyberwareTextIds.size();
    if (cyberware_text_matches) {
        for (std::size_t index = 0; index < kCyberwareTextIds.size(); ++index) {
            const auto& choice = cyberware_text->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            const auto expected_key = std::string("cyberware.text.option.") +
                std::string(kCyberwareTextIds[index]);
            const auto expected_label = index == 1U
                ? std::string("[") + std::string(10U, '\\')
                : std::string(kCyberwareTextLabels[index]);
            cyberware_text_matches = cyberware_text_matches &&
                choice.id == kCyberwareTextIds[index] &&
                choice.label_key == expected_key && label != nullptr &&
                *label == expected_label;
        }
    }
    if (!expect(cyberware_text_matches,
                "Lua Cyberware keys or ROM text changed source order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto inventory_actions =
        scripts.string_list("items", "inventory_actions");
    constexpr std::array<std::string_view, 4> kInventoryActionIds{{
        "equip_or_use", "discard", "trade", "cancel"}};
    bool inventory_action_rows_match = inventory_actions.has_value() &&
        inventory_actions->size() == kInventoryActionIds.size();
    if (inventory_action_rows_match) {
        for (std::size_t index = 0; index < kInventoryActionIds.size();
             ++index) {
            inventory_action_rows_match = inventory_action_rows_match &&
                (*inventory_actions)[index] == kInventoryActionIds[index];
        }
    }
    if (!expect(inventory_action_rows_match,
                "Lua inventory action IDs changed source order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }
    const auto inferred_inventory_menu = scripts.menu(
        "items", "inventory_actions", "items", "inventory_actions");
    if (!expect(!inferred_inventory_menu.has_value() &&
                    scripts.failure().find("explicit text keys") !=
                        std::string::npos,
                "ID-only inventory actions were accepted as a display menu")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto item_ui_text =
        scripts.menu("items", "ui_text_entries", "items.text", "catalog");
    constexpr std::array<std::string_view, 14> kItemUiTextIds{{
        "equipped_prefix", "equipped_suffix", "discard_prompt",
        "cannot_discard", "cannot_trade", "trade_recipient",
        "trade_full_inventory", "automatic_use", "first_aid_prompt",
        "first_aid_label", "first_aid_cancel", "first_aid_not_injured",
        "first_aid_slap_patch_used", "first_aid_medkit_used"}};
    constexpr std::array<std::string_view, 14> kItemUiTextKeys{{
        "items.text.option.equipped_prefix",
        "items.text.option.equipped_suffix",
        "items.text.option.discard_prompt",
        "items.text.option.cannot_discard",
        "items.text.option.cannot_trade",
        "items.text.option.trade_recipient",
        "items.text.option.trade_full_inventory",
        "items.text.option.automatic_use",
        "items.text.option.first_aid_prompt",
        "items.text.option.first_aid_label",
        "items.text.option.first_aid_cancel",
        "items.text.option.first_aid_not_injured",
        "items.text.option.first_aid_slap_patch_used",
        "items.text.option.first_aid_medkit_used"}};
    const auto item_display_text = scripts.menu(
        "items", "display_text_entries", "items.display", "catalog");
    constexpr std::array<std::string_view, 27> kItemDisplayTextIds{{
        "power_meager", "power_minor", "power_very_low", "power_low",
        "power_very_light", "power_light", "power_medium",
        "power_medium_high", "power_high", "power_brutal",
        "power_severe", "power_massive", "power_incredible",
        "armor_vest_top", "armor_vest_bottom", "armor_clothing_top",
        "armor_clothing_bottom", "armor_jacket_top", "armor_jacket_bottom",
        "armor_plates_top", "armor_plates_bottom", "lined_duster_top",
        "lined_duster_bottom", "light_combat_armor_top",
        "light_combat_armor_bottom", "heavy_combat_armor_top",
        "heavy_combat_armor_bottom"}};
    constexpr std::array<std::string_view, 27> kItemDisplayTextKeys{{
        "items.display.option.power_meager",
        "items.display.option.power_minor",
        "items.display.option.power_very_low",
        "items.display.option.power_low",
        "items.display.option.power_very_light",
        "items.display.option.power_light",
        "items.display.option.power_medium",
        "items.display.option.power_medium_high",
        "items.display.option.power_high",
        "items.display.option.power_brutal",
        "items.display.option.power_severe",
        "items.display.option.power_massive",
        "items.display.option.power_incredible",
        "items.display.option.armor_vest_top",
        "items.display.option.armor_vest_bottom",
        "items.display.option.armor_clothing_top",
        "items.display.option.armor_clothing_bottom",
        "items.display.option.armor_jacket_top",
        "items.display.option.armor_jacket_bottom",
        "items.display.option.armor_plates_top",
        "items.display.option.armor_plates_bottom",
        "items.display.option.lined_duster_top",
        "items.display.option.lined_duster_bottom",
        "items.display.option.light_combat_armor_top",
        "items.display.option.light_combat_armor_bottom",
        "items.display.option.heavy_combat_armor_top",
        "items.display.option.heavy_combat_armor_bottom"}};
    bool item_text_keys_match = item_ui_text.has_value() &&
        item_display_text.has_value() &&
        item_ui_text->choices.size() == kItemUiTextIds.size() &&
        item_display_text->choices.size() == kItemDisplayTextIds.size();
    if (item_text_keys_match) {
        for (std::size_t index = 0; index < kItemUiTextIds.size(); ++index) {
            const auto& choice = item_ui_text->choices[index];
            item_text_keys_match = item_text_keys_match &&
                choice.id == kItemUiTextIds[index] &&
                choice.label_key == kItemUiTextKeys[index] &&
                scripts.dialogue_text(choice.label_key) != nullptr;
        }
        for (std::size_t index = 0; index < kItemDisplayTextIds.size();
             ++index) {
            const auto& choice = item_display_text->choices[index];
            item_text_keys_match = item_text_keys_match &&
                choice.id == kItemDisplayTextIds[index] &&
                choice.label_key == kItemDisplayTextKeys[index] &&
                scripts.dialogue_text(choice.label_key) != nullptr;
        }
    }
    if (!expect(item_text_keys_match,
                "Lua item UI or display text keys changed source order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto magic_spells =
        scripts.menu("magic", "spells", "magic.spells", "catalog");
    constexpr std::array<std::string_view, 14> kMagicSpellIds{{
        "flame_dart", "flame_bolt", "hellblast", "invisibility", "sleep",
        "stink", "super_barrier", "mana_zap", "mana_blast", "mana_storm",
        "rockskin", "heal_wounds", "confusion", "barrier"}};
    constexpr std::array<std::string_view, 14> kMagicSpellNames{{
        "Flame Dart", "Flame Bolt", "Hellblast", "Invisibility", "Sleep",
        "Stink", "Super Barrier", "Mana Zap", "Mana Blast", "Mana Storm",
        "Rockskin", "Heal Wounds", "Confusion", "Barrier"}};
    constexpr std::array<std::string_view, 14> kMagicSpellDescriptions{{
        "A small burst of fire that\\ndelivers a nasty burn.",
        "A large ball of fire which\\ncauses a vicious burn.",
        "Creates fire to incinerate\\nall in its area of effect!",
        "Renders a person invisible.",
        "An area attack on one's\\nmental state, fatiguing them\\ninto unconciousness.",
        "An area effect spell which\\nsickens all targets with\\na foul stench.",
        "Creates a magical wall which\\ninhibits everything.",
        "A small zap of energy that\\ndelivers a nasty jolt.",
        "A burst of magical energy\\nthat causes a vicious jolt.",
        "A magical conflagration\\nengulfing all within its\\narea of effect!",
        "Hardens flesh to resist\\nthe effects of damage.",
        "Cures damage to characters.",
        "An area effect spell which\\ndistracts all enemies with\\nsensory overload.",
        "Creates a magical wall which\\ninhibits movement only."}};
    bool magic_spell_rows_match = magic_spells.has_value() &&
        magic_spells->choices.size() == kMagicSpellIds.size();
    if (magic_spell_rows_match) {
        for (std::size_t index = 0; index < kMagicSpellIds.size(); ++index) {
            const auto& choice = magic_spells->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            magic_spell_rows_match = magic_spell_rows_match &&
                choice.id == kMagicSpellIds[index] && label != nullptr &&
                choice.label_key ==
                    "magic.spells.option." +
                        std::string(kMagicSpellIds[index]) &&
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

    const auto magic_spell_descriptions = scripts.menu(
        "magic", "spell_descriptions", "magic.spells.description",
        "catalog");
    bool magic_spell_descriptions_match =
        magic_spell_descriptions.has_value() &&
        magic_spell_descriptions->choices.size() == kMagicSpellIds.size();
    if (magic_spell_descriptions_match) {
        for (std::size_t index = 0; index < kMagicSpellIds.size(); ++index) {
            const auto& choice = magic_spell_descriptions->choices[index];
            const auto* description = scripts.dialogue_text(choice.label_key);
            magic_spell_descriptions_match =
                magic_spell_descriptions_match &&
                choice.id == kMagicSpellIds[index] && description != nullptr &&
                choice.label_key ==
                    "magic.spells.description." +
                        std::string(kMagicSpellIds[index]) &&
                *description == kMagicSpellDescriptions[index];
        }
    }
    if (!expect(magic_spell_descriptions_match,
                "Lua Magic spell description keys or ROM text changed order")) {
        std::cerr << scripts.failure() << '\n';
        return 1;
    }

    const auto magic_panel_text = scripts.menu(
        "magic", "panel_text_entries", "magic.panel.text", "catalog");
    constexpr std::array<std::string_view, 7> kMagicPanelTextIds{{
        "defense_title", "force_posture_heading", "success_label",
        "force_label", "availability_label", "panel_metric_labels",
        "panel_force_posture_labels"}};
    constexpr std::array<std::string_view, 7> kMagicPanelTextLabels{{
        " Magical Defense", "Set Force & Posture.", "Success", "Force",
        "Damage",
        "Success\\n\\nDrain\\n\\nDefense\\n  vs. Phys\\n\\n  vs. Mana",
        "Force\\n\\n\\nPosture"}};
    bool magic_panel_text_matches = magic_panel_text.has_value() &&
        magic_panel_text->title_key.empty() &&
        magic_panel_text->choices.size() == kMagicPanelTextIds.size();
    if (magic_panel_text_matches) {
        for (std::size_t index = 0; index < kMagicPanelTextIds.size(); ++index) {
            const auto& choice = magic_panel_text->choices[index];
            const auto* label = scripts.dialogue_text(choice.label_key);
            magic_panel_text_matches = magic_panel_text_matches &&
                choice.id == kMagicPanelTextIds[index] && label != nullptr &&
                choice.label_key ==
                    "magic.panel.text.option." +
                        std::string(kMagicPanelTextIds[index]) &&
                *label == kMagicPanelTextLabels[index];
        }
    }
    if (!expect(magic_panel_text_matches,
                "Lua Magic panel keys or ROM text changed source order")) {
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
    constexpr std::array<std::string_view, 50U> kItemIds{{
        "streetline", "model_101t", "american_l36", "security_500",
        "warhawk", "max_power", "predator", "ak_97", "hk227_s",
        "mach_22", "allegiance", "roomsweeper", "frag", "scatter",
        "concussion", "medkit", "stim_patch", "trauma_patch", "fetish",
        "maglock_passkey", "electronic_kit", "smart_goggles",
        "enchanted_dagger", "power_focus", "flame_dart_focus",
        "flame_bolt_focus", "hellblast_focus", "invisibility_focus",
        "sleep_focus", "stink_focus", "supr_barrier_focus",
        "mana_zap_focus", "mana_blast_focus", "mana_storm_focus",
        "rockskin_focus", "heal_wounds_focus", "confusion_focus",
        "barrier_focus", "protection_talisman", "combat_sense_spell_lock",
        "rat_totem", "gator_totem", "bear_totem", "armor_vest",
        "armor_clothing", "armor_jacket", "vest_with_plates",
        "lined_duster", "light_combat_armor", "heavy_combat_armor"}};
    constexpr std::array<std::string_view, 50U> kItemKinds{{
        "firearm", "firearm", "firearm", "firearm", "firearm",
        "firearm", "firearm", "firearm", "firearm", "firearm",
        "firearm", "firearm", "grenade", "grenade", "grenade",
        "medical_kit", "stim_patch", "trauma_patch", "fetish", "passkey",
        "electronic_kit", "smart_goggles", "enchanted_dagger", "power_focus",
        "spell_focus", "spell_focus", "spell_focus", "spell_focus",
        "spell_focus", "spell_focus", "spell_focus", "spell_focus",
        "spell_focus", "spell_focus", "spell_focus", "spell_focus",
        "spell_focus", "spell_focus", "talisman", "spell_lock", "totem",
        "totem", "totem", "armor", "armor", "armor", "armor", "armor",
        "armor", "armor"}};
    constexpr std::array<std::uint16_t, 50U> kBaseCosts{{
        100U, 350U, 300U, 450U, 500U, 450U, 450U, 1000U, 1500U, 800U,
        1400U, 1000U, 300U, 180U, 180U, 200U, 300U, 300U, 500U, 10000U,
        1000U, 3000U, 50000U, 15000U, 4000U, 4000U, 4000U, 4000U, 4000U,
        4000U, 4000U, 4000U, 4000U, 4000U, 4000U, 4000U, 4000U, 4000U,
        17500U, 17500U, 2U, 65535U, 0U, 200U, 500U, 900U, 600U, 700U,
        10000U, 20000U}};
    constexpr std::array<std::uint8_t, 12U> kWeaponCapacities{{
        6U, 12U, 11U, 25U, 6U, 10U, 15U, 30U, 28U, 24U, 5U, 8U}};
    constexpr std::array<std::string_view, 12U> kWeaponModes{{
        "single_round", "single_round", "single_round", "single_round",
        "single_round", "single_round", "single_round",
        "three_round_burst", "three_round_burst", "three_round_burst",
        "single_round", "single_round"}};
    constexpr std::array<std::uint8_t, 12U> kRoundsPerAction{{
        1U, 1U, 1U, 1U, 1U, 1U, 1U, 3U, 3U, 3U, 1U, 1U}};
    constexpr std::array<std::uint8_t, 12U> kWeaponEffectIndices{{
        0U, 0U, 0U, 0U, 1U, 1U, 1U, 1U, 1U, 1U, 2U, 2U}};
    constexpr std::array<std::string_view, 12U> kReloadDelayScales{{
        "normal", "normal", "normal", "normal", "double", "normal",
        "normal", "normal", "normal", "normal", "double", "double"}};
    constexpr std::array<std::string_view, 12U> kAttackDelayScales{{
        "half", "half", "half", "half", "half", "half", "half",
        "half", "half", "half", "normal", "normal"}};
    constexpr std::array<std::string_view, 13U> kPowerProfiles{{
        "meager", "minor", "very_low", "low", "very_light", "light",
        "medium", "medium_high", "high", "brutal", "severe", "massive",
        "incredible"}};
    bool full_catalog_matches = items.has_value() && items->items.size() == 50U;
    if (full_catalog_matches) {
        for (std::size_t index = 0; index < items->items.size(); ++index) {
            const auto& item = items->items[index];
            const auto& rom_item = us_item_catalog()[index];
            const auto item_number = index + 1U < 10U
                ? "0" + std::to_string(index + 1U)
                : std::to_string(index + 1U);
            const auto* image_family = index < 15U
                ? "weapons"
                : index < 18U ? "first.aid"
                : index < 22U ? "miscellaneous"
                : index < 43U ? "magic"
                              : "armor";
            const auto expected_icon_key = std::string("items.") +
                image_family + ".icons.item." + item_number;
            const auto expected_detail_key = std::string("items.") +
                image_family + ".detail.item." + item_number;
            full_catalog_matches =
                full_catalog_matches && item.code == index + 1U &&
                item.id == kItemIds[index] && item.kind == kItemKinds[index] &&
                item.pricing.purchase_base == kBaseCosts[index] &&
                item.pricing.purchase_base == rom_item.base_cost &&
                item.pricing.resale_base == kBaseCosts[index] / 2U &&
                item.action_motion_distance_profile ==
                    rom_item.source_action_parameter_tail &&
                item.presentation.icon_resource_key == expected_icon_key &&
                item.presentation.detail_resource_key == expected_detail_key &&
                item.has_detail_art == rom_item.has_detail_art &&
                item.name_key == "items.name." + std::to_string(index + 1U) &&
                item.category_key ==
                    "items.category." + std::to_string(index + 1U) &&
                scripts.dialogue_text(item.name_key) != nullptr &&
                scripts.dialogue_text(item.category_key) != nullptr;

            if (index < kWeaponCapacities.size()) {
                const auto& weapon = item.weapon;
                const auto source_effect = find_us_action_effect(
                    kWeaponEffectIndices[index]);
                const auto power_index = static_cast<std::size_t>(
                    rom_item.detail_selector - 1U);
                full_catalog_matches =
                    full_catalog_matches && weapon.has_value() &&
                    weapon->id == item.id && weapon->item_code == item.code &&
                    weapon->magazine_capacity == kWeaponCapacities[index] &&
                    weapon->magazine_capacity ==
                        rom_item.source_combat_factor &&
                    source_effect != nullptr &&
                    weapon->source_action.effect_table_index ==
                        kWeaponEffectIndices[index] &&
                    weapon->source_action.effect_table_index ==
                        static_cast<std::uint8_t>(
                            rom_item.signed_detail_asset_index) &&
                    weapon->source_action.effect_value ==
                        source_effect->negated_source_byte &&
                    weapon->source_action.difficulty ==
                        rom_item.source_action_difficulty() &&
                    weapon->damage_bar_index == static_cast<std::uint8_t>(
                        rom_item.signed_detail_asset_index) &&
                    power_index < kPowerProfiles.size() &&
                    weapon->power_profile == kPowerProfiles[power_index] &&
                    weapon->fire_mode == kWeaponModes[index] &&
                    weapon->rounds_per_action == kRoundsPerAction[index] &&
                    weapon->attack_cadence == "runner_attribute" &&
                    weapon->trigger_behavior == "repeat_while_held" &&
                    weapon->attack_delay_scale ==
                        kAttackDelayScales[index] &&
                    weapon->reload_delay_scale ==
                        kReloadDelayScales[index] &&
                    !item.detail.has_value() && !item.armor.has_value();
            } else if (index >= 12U && index <= 18U) {
                full_catalog_matches =
                    full_catalog_matches && item.detail.has_value() &&
                    item.detail->kind == "uses" &&
                    item.detail->value == rom_item.source_combat_factor &&
                    !item.weapon.has_value() && !item.armor.has_value();
            } else if (index >= 43U) {
                const auto* armor = find_us_armor_definition_for_item_code(
                    item.code);
                full_catalog_matches =
                    full_catalog_matches && item.armor.has_value() &&
                    armor != nullptr &&
                    item.armor->item_id == item.id &&
                    item.armor->item_code == item.code &&
                    item.armor->armor_index == armor->armor_index &&
                    item.armor->primary_modifier == armor->primary_modifier &&
                    item.armor->secondary_modifier ==
                        armor->secondary_modifier &&
                    !item.detail.has_value() && !item.weapon.has_value();
            } else {
                full_catalog_matches =
                    full_catalog_matches && !item.detail.has_value() &&
                    !item.weapon.has_value() && !item.armor.has_value();
            }
        }
    }
    if (!expect(items.has_value() && items->items.size() == 50U &&
                    items->detail_text_keys.size() == 32U &&
                    items->detail_label_keys.size() == 3U &&
                    full_catalog_matches &&
                    items->detail_text_keys[31] == "items.action_detail.31",
                "Lua item catalog does not match the complete source item/weapon/armor data") ||
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
        scripts.string_list("motel", "manager_choices");
    if (!expect(manager.has_value() && manager->size() == 2U &&
                    (*manager)[0] == "crash" && (*manager)[1] == "leave",
                "ordinary manager menu does not have its two source choices")) {
        std::cerr << scripts.failure() << '\n';
        return 9;
    }
    scripts.set_story_state(repeat_visit);
    scripts.set_location_context("motel", "downtown_motel");
    const auto ordinary_motel_manager =
        scripts.string_list("motel", "manager_choices");
    if (!expect(ordinary_motel_manager.has_value() &&
                    ordinary_motel_manager->size() == 2U,
                "Michael quest hook leaked into another motel")) {
        return 10;
    }
    scripts.set_location_context("motel", "stoker_motel");
    const auto package_manager =
        scripts.string_list("motel", "manager_choices");
    if (!expect(package_manager.has_value() &&
                    package_manager->size() == 3U &&
                    (*package_manager)[2] == "package",
                "unpaid manager menu lacks the package choice")) {
        std::cerr << scripts.failure() << '\n';
        return 10;
    }
    scripts.set_story_state(paid_but_not_collected);
    const auto paid_package_manager =
        scripts.string_list("motel", "manager_choices");
    if (!expect(paid_package_manager.has_value() &&
                    paid_package_manager->size() == 3U,
                "debt payment prematurely removed the uncollected package")) {
        return 11;
    }
    scripts.set_story_state(completed);
    const auto completed_package_manager =
        scripts.string_list("motel", "manager_choices");
    if (!expect(completed_package_manager.has_value() &&
                    completed_package_manager->size() == 2U,
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

    scripts.set_location_context(
        "activity", "resource_scene", "field.activity");
    auto resource_menu = scripts.start_task(
        "field_activities", "resource_scene_main_menu");
    if (!expect(resource_menu.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    resource_menu.operation.has_value() &&
                    resource_menu.operation->type ==
                        ScriptNativeOperation::Type::ui_menu &&
                    resource_menu.operation->menu.has_value() &&
                    resource_menu.operation->menu->input_mode ==
                        ScriptMenuInputMode::cursor &&
                    resource_menu.operation->menu->selected_index == 0U &&
                    resource_menu.operation->menu->choices.size() == 2U,
                "resource Main Menu did not yield its Lua cursor state")) {
        std::cerr << scripts.failure() << '\n';
        return 15;
    }
    resource_menu = scripts.complete_native_menu_input("down");
    if (!expect(resource_menu.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    resource_menu.operation.has_value() &&
                    resource_menu.operation->menu.has_value() &&
                    resource_menu.operation->menu->selected_index == 1U,
                "Lua did not advance its cursor selection after Down")) {
        std::cerr << scripts.failure() << '\n';
        return 15;
    }
    resource_menu = scripts.complete_native_menu_input("up");
    if (!expect(resource_menu.status ==
                        ScriptTaskStep::Status::awaiting_native_operation &&
                    resource_menu.operation.has_value() &&
                    resource_menu.operation->menu.has_value() &&
                    resource_menu.operation->menu->selected_index == 0U,
                "Lua did not wrap its cursor selection after Up")) {
        std::cerr << scripts.failure() << '\n';
        return 15;
    }
    resource_menu = scripts.complete_native_menu_input("cancel");
    if (!expect(resource_menu.status == ScriptTaskStep::Status::completed &&
                    resource_menu.result_id == std::optional<std::string>{
                        "return_from_world_activity"},
                "Start did not select the Lua resource-scene Exit action")) {
        std::cerr << scripts.failure() << '\n';
        return 15;
    }

    scripts.set_story_fact_accessors({}, {});
    scripts.set_story_state({
        {"vigore_search_pending", true},
        {"vigore_search_consumed", false},
        {"vigore_search_completed", false},
    });
    scripts.set_location_context(
        "field_map", "redmond_barrens", "field.world_event",
        "field_narrator");
    const auto vigore_search_decision = scripts.call_string(
        "redmond_barrens", "should_run_vigore_search");
    if (!expect(vigore_search_decision == "run",
                "Lua did not select the pending Redmond event-7 story service")) {
        std::cerr << scripts.failure() << '\n';
        return 16;
    }
    auto vigore_search =
        scripts.start_task("redmond_barrens", "run_vigore_search");
    constexpr std::array<std::string_view, 3> vigore_search_pages{{
        "redmond.vigore-search.page-0",
        "redmond.vigore-search.page-1",
        "redmond.vigore-search.page-2",
    }};
    for (std::size_t page = 0U; page < vigore_search_pages.size(); ++page) {
        if (!expect(vigore_search.status ==
                            ScriptTaskStep::Status::awaiting_native_operation &&
                        vigore_search.operation.has_value() &&
                        vigore_search.operation->type ==
                            ScriptNativeOperation::Type::ui_say &&
                        vigore_search.operation->dialogue.has_value() &&
                        vigore_search.operation->dialogue->speaker_id ==
                            "field_narrator" &&
                        vigore_search.operation->dialogue->text_profile ==
                            "field.world_event" &&
                        vigore_search.operation->dialogue->event_id ==
                            vigore_search_pages[page],
                    "Lua Redmond event did not yield the ROM page in order")) {
            std::cerr << scripts.failure() << '\n';
            return 16;
        }
        const auto page_text = scripts.dialogue_event_text_keys(
            "field_narrator", "field.world_event",
            vigore_search_pages[page]);
        const auto* continuation_text =
            page_text.has_value() &&
                    !page_text->continuation_key.empty()
                ? scripts.dialogue_text(page_text->continuation_key)
                : nullptr;
        if (!expect(page_text.has_value() &&
                        !page_text->title_key.empty() &&
                        scripts.dialogue_text(page_text->title_key) != nullptr &&
                        ((page < 2U && page_text->continuation_key ==
                            "shared.more" && continuation_text != nullptr &&
                          continuation_text->find("MORE") !=
                              std::string_view::npos) ||
                         (page == 2U && page_text->continuation_key.empty())),
                    "Lua Redmond event page lost its ROM text or prompt state")) {
            std::cerr << scripts.failure() << '\n';
            return 16;
        }
        vigore_search = scripts.complete_native_operation(true);
    }
    if (!expect(vigore_search.status == ScriptTaskStep::Status::completed &&
                    vigore_search.result_id ==
                        std::optional<std::string>{"completed"} &&
                    scripts.story_state().at("vigore_search_completed"),
                "Lua Redmond event did not publish completion after its last page")) {
        std::cerr << scripts.failure() << '\n';
        return 16;
    }
    return 0;
}
