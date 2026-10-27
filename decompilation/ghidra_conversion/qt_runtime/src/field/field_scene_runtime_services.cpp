#include "field_scene_runtime.hpp"
#include "field_scene_runtime_internal.hpp"

#include "enums.hpp"
#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_proximity_collision_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_route_primitives_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_scene_runtime_preparation_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_progression_purchase_conversion.hpp"
#include "shadowrun_ghidra/field/field_equipment_upgrade_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_detail_text_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"
#include "shadowrun_ghidra/field/stoker_motel_followup_commands_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/session/runner_status_conversion.hpp"
#include "shadowrun_ghidra/core/armor_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/vid_phone_service_conversion.hpp"
#include "shadowrun_ghidra/core/magic_spell_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/random_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_math_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/heading_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_state_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_delay_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_combat_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_spawn_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_player_action_conversion.hpp"
#include "shadowrun_ghidra/menus/clips_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/menus/cyberware_detail_conversion.hpp"
#include "genesis_image_compositor.hpp"
#include "numeric_visual_renderer.hpp"
#include "field_map_composition.hpp"
#include "matrix_map_presenter.hpp"
#include "matrix_datastore_facts.hpp"

#include <QPainter>
#include <QColor>
#include <QRegion>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

using field_scene_detail::kInitialFieldSceneReference;
using field_scene_detail::kSceneProfileEntityPositionWord;
using field_scene_detail::kFieldModalReturnTransitionFrames;
using field_scene_detail::magic_action_record_from_runner;

void FieldSceneRuntime::provide_source_profile_patrol_movement_services(
    scripting::ScriptActorBehaviorContext& context,
    DefaultFieldActorPresentation& actor) {
    static constexpr std::array<std::uint16_t, 8> kMovementPoseBases{{
        0x00U, 0x04U, 0x08U, 0x0CU, 0x10U, 0x14U, 0x18U, 0x1CU,
    }};

    context.movement_allowed = true;
    context.movement_services_available = true;
    context.movement_state = scripting::ScriptMovementStepState{
        actor.movement_state.position_x,
        actor.movement_state.position_y,
        actor.movement_state.heading,
        actor.movement_state.animation_heading,
        actor.movement_state.collision_context,
        actor.movement_state.movement_substate,
        actor.movement_state.route_status,
        actor.source_behavior_flags,
        actor.movement_state.movement_flags,
        actor.movement_state.collision_flags,
        actor.movement_state.event_flags,
        actor.movement_state.collision_extent,
        actor.movement_terrain_collision_code};
    for (std::size_t index = 0U;
         index < field_movement_direction_offsets_.size(); ++index) {
        context.movement_offsets[index] = scripting::ScriptMovementOffset{
            field_movement_direction_offsets_[index].x,
            field_movement_direction_offsets_[index].y};
    }
    context.movement_idle_selectors = kMovementPoseBases;
    context.movement_entity_flags = actor.source_record_flags;
    context.movement_collision_mode =
        field_movement_transition_state_.world_mode;
    context.measure_destination = [&actor](
        const std::uint16_t target_x, const std::uint16_t target_y)
        -> std::optional<scripting::ScriptActorDestinationMeasurement> {
        const auto measurement = measure_entity_target_heading_and_distance(
            actor.world_position.x, actor.world_position.y,
            target_x, target_y);
        const auto route_cost = std::min<std::uint32_t>(
            compute_record_diagonal_step_cost(
                actor.world_position.x, actor.world_position.y,
                target_x, target_y),
            std::numeric_limits<std::uint16_t>::max());
        return scripting::ScriptActorDestinationMeasurement{
            measurement.heading,
            static_cast<std::uint16_t>(std::min<std::uint32_t>(
                measurement.distance,
                std::numeric_limits<std::uint16_t>::max())),
            static_cast<std::uint16_t>(route_cost)};
    };
    context.movement_collision = [this, &actor](
        const std::uint16_t x, const std::uint16_t y) {
        const auto collision_code = evaluate_field_collision(
            x, y, actor.movement_collision_record,
            actor.movement_sample_state, actor.source_record_flags, false,
            &actor.movement_terrain_collision_code);
        return scripting::ScriptMovementCollisionResult{
            collision_code, actor.movement_terrain_collision_code};
    };
    context.movement_route_lookup = [this, &actor](
        const std::uint16_t route_key) {
        if ((route_key & 0x03U) != 0U ||
            route_key / 4U >= field_movement_routes_.size()) {
            source_boundary("field.actor.movement.route_key_out_of_range");
            throw std::runtime_error(
                "Lua requested an actor route outside the source table");
        }
        const auto route_index = static_cast<std::size_t>(route_key / 4U);
        const auto& route_source = field_movement_route_sources_[route_index];
        const auto sample_value = [&actor](const std::uint8_t index) {
            if (index >= actor.movement_sample_state.size()) {
                throw std::runtime_error(
                    "actor route sample is outside its local collision state");
            }
            return static_cast<std::int8_t>(
                actor.movement_sample_state[index]);
        };
        const auto& route = field_movement_routes_[route_index];
        return scripting::ScriptMovementRouteFacts{
            sample_value(route_source.primary_sample_index),
            sample_value(route_source.secondary_sample_index),
            route.primary_fallback_slot,
            route.secondary_fallback_slot};
    };
    context.movement_collision_recovery = [&actor](
        const std::uint16_t, const std::uint16_t,
        const std::int16_t collision_code, const std::uint8_t) {
        // Default-list patrols use the actor route fallback; map-event
        // recovery remains owned by the controlled runner movement path.
        return scripting::ScriptMovementCollisionRecoveryResult{
            collision_code < 0, actor.source_behavior_flags};
    };
}

#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::open_command_overview_for_dump() {
    if (!ready_) {
        return false;
    }
    // Exercise the same converted field-coordinator Start path used by the
    // window rather than directly toggling the presentation state.
    set_controller_port_byte(static_cast<std::uint8_t>(
        field_controller_port_byte_ | 0x80U));
    advance_frame();
    set_controller_port_byte(static_cast<std::uint8_t>(
        field_controller_port_byte_ & static_cast<std::uint8_t>(~0x80U)));
    while (menu_display_blank_active_for_dump()) {
        advance_frame();
    }
    return command_overview_.open();
}

bool FieldSceneRuntime::dispatch_dialogue_service_for_dump(
    const std::string_view service_id,
    const std::string_view return_policy) {
    scripting::DialogueOutcome outcome = scripting::OpenServiceOutcome{
        std::string(service_id), std::string(return_policy)};
    return dispatch_dialogue_outcome(outcome);
}

bool FieldSceneRuntime::open_hospital_menu_for_dump(
    const std::string_view function) {
    if (!ready_ || (function != "present_options" &&
                    function != "present_leave_confirmation" &&
                    function != "present_medical_attention")) {
        return false;
    }
    if (function == "present_medical_attention") {
        auto& runner = active_runner_record();
        runner.archetype_index = 2U;
        runner.attributes[0] = 6U;
        runner.attributes[5] = 6U;
        runner.delay_meter = 3U;
        runner.behavior_meter = 3U;
        field_funds_ = 1000U;
        refresh_field_hud_runner_slot();
    }
    field_mode_entry_transition_pending_ = false;
    field_mode_entry_destination_scene_.reset();
    hospital_service_session_active_ = false;
    game_scripts_.set_location_context(
        "building", "council_island_hospital",
        "council_island_hospital", "council_island_hospital_narrator");
    game_scripts_.set_field_mode_context(
        scripting::ScriptFieldModeContext{
            4U, 4U, 0, state_.field_scene_buffers.scene_mode,
            field_movement_transition_state_.field_transition_context});
    return begin_hospital_menu_task(function);
}

bool FieldSceneRuntime::open_ordinary_field_terminal_for_dump(
    const std::uint32_t contact_flags, const std::uint32_t funds) {
    if (!ready_ || !set_notebook_contact_flags_for_dump(contact_flags)) {
        return false;
    }
    field_funds_ = funds;
    game_scripts_.set_location_context(
        "activity", "field_terminal", "field.main_menu");
    return field_dialogue_.open_gameplay_task(
        "field_activities", "ordinary_field_terminal");
}

bool FieldSceneRuntime::open_spirit_eyes_ritual_for_dump() {
    if (!ready_ || state_.field_scene_buffers.scene_reference != 4U) {
        return false;
    }
    // The field encounter that awards the scale is outside this QA route.
    // Seed its recovered source state, then let the normal Lua task, dialogue
    // runtime, controller, presenter, and typed return path run unchanged.
    field_exit_world_flags_ = (1U << 5U) | (1U << 6U) |
                              (1U << 7U) | (1U << 8U);
    group_item_flags_ = 1U << 2U;
    field_active_story_trigger_ = 8U;
    field_mode_effect_flags_ = (1U << 15U) | (1U << 18U);
    field_auxiliary_flags_ = 1U << 2U;
    game_scripts_.set_location_context(
        "site", "council_spirit_eyes_site", "salish.spirit_eyes",
        "spirit_eyes");
    game_scripts_.set_field_mode_context(scripting::ScriptFieldModeContext{
        4U, 27U, field_exit_alternate_result_,
        state_.field_scene_buffers.scene_mode,
        field_movement_transition_state_.field_transition_context});
    return field_dialogue_.open_gameplay_task(
        "salish_spirit_eyes", "present");
}

bool FieldSceneRuntime::open_tarislar_tomb_credits_for_dump() {
    if (!ready_) {
        return false;
    }
    // This hook exercises only the recovered panel sequence. It does not
    // synthesize Scene-55 fight completion or the title-screen return.
    return field_dialogue_.open_gameplay_task(
        "tarislar_tomb_credits", "present");
}

bool FieldSceneRuntime::seed_cyberspace_access_for_dump() {
    if (!ready_) {
        return false;
    }
    install_test_deck();
    // The field script checks the active runner's source Datajack bit before
    // opening Cyberspace. QA supplies that ROM-defined access condition.
    active_runner_record().cyberware_flags |= 0x0001U;
    return true;
}

bool FieldSceneRuntime::seed_cyberdeck_reference_frame_for_dump() {
    if (!ready_ || !cyberdeck_available_) {
        return false;
    }

    // Visual fixture from independent longplay frame 004220: Attack 3 is
    // selected; Attack, Medic, Deception, and Analyze are available; loaded
    // Deception + Analyze consume 2 + 3 Mp from a 15 Mp deck. Only the
    // selected Attack rating is printed in this frame; Medic uses the
    // smallest nonzero value because its exact rating is not visible.
    auto deck = field_matrix_deck_record_.value_or(CyberdeckRecord{});
    deck.memory_megapulses = 15U;
    deck.load_io_speed = 10U;
    std::array<std::uint8_t, 12U> ratings{};
    ratings[0U] = 3U;
    ratings[4U] = 1U;
    ratings[9U] = 1U;
    ratings[11U] = 1U;
    deck.program_ratings = ratings;
    deck.active_programs = std::array<std::int8_t, 5U>{
        9, 11, -1, -1, -1};
    field_matrix_deck_record_ = std::move(deck);
    field_matrix_program_slots_ = {};
    return true;
}

bool FieldSceneRuntime::seed_matrix_storage_files_for_dump() {
    if (!ready_ || !field_matrix_deck_record_.has_value()) {
        return false;
    }

    // Exercise all five ROM records, reverse display order, and three-digit
    // storage values while keeping the rest of the active deck fixture intact.
    std::array<CyberdeckDataFile, 5U> files{};
    files[0U] = CyberdeckDataFile{0, 4U, 0};
    files[1U] = CyberdeckDataFile{2, 9U, 0};
    files[2U] = CyberdeckDataFile{13, 20U, 0};
    files[3U] = CyberdeckDataFile{48, 80U, 0};
    files[4U] = CyberdeckDataFile{69, 255U, 0};
    field_matrix_deck_record_->data_files = files;
    return true;
}

bool FieldSceneRuntime::seed_cyberdeck_pull_plug_reference_for_dump() {
    if (!ready_ || !cyberdeck_available_ ||
        !field_matrix_deck_record_.has_value()) {
        return false;
    }

    // Independent longplay frame 005400 shows Deception and Attack in the
    // first two active slots while the Pull Plug prompt is open.
    field_matrix_deck_record_->active_programs =
        std::array<std::int8_t, 5U>{9, 0, -1, -1, -1};
    return true;
}

bool FieldSceneRuntime::open_matrix_traversal_for_dump(
    const std::uint8_t system_id, const std::uint8_t node_id) {
    if (!ready_ || system_id > 24U || !seed_cyberspace_access_for_dump()) {
        return false;
    }

    // Independent Section 21 traversal fixture: Medic-1 and Analyze-2 are
    // installed in the first two footer cells, both fully loaded and ready.
    auto deck = field_matrix_deck_record_.value_or(CyberdeckRecord{});
    deck.memory_megapulses = 15U;
    deck.load_io_speed = 10U;
    std::array<std::uint8_t, 12U> ratings{};
    ratings[4U] = 1U;
    ratings[11U] = 2U;
    deck.program_ratings = ratings;
    deck.active_programs =
        std::array<std::int8_t, 5U>{4, 11, -1, -1, -1};
    field_matrix_deck_record_ = std::move(deck);
    field_matrix_program_slots_.fill(
        scripting::ScriptCyberdeckProgramSlot{});

    scripting::ScriptMenuRouteStartArguments arguments{};
    arguments.matrix_system_id = system_id;
    arguments.matrix_node_id = node_id;
    arguments.matrix_node_ic_condition = 0U;
    // The Section 21 capture is a previously explored node. ROM FFFB30 keeps
    // its rank across the 0x181F8 direct Matrix entry, so seed rank 4 before
    // the entry animation rather than deriving it from the test traversal.
    arguments.matrix_node_knowledge_rank = 4U;
    arguments.return_to_field = true;
    open_command_overview(
        static_cast<std::uint16_t>(srx::MenuScreen::MATRIX_ENTRY),
        std::move(arguments));
    return menu_route_active_ &&
           menu_route_screen_ == static_cast<std::uint16_t>(
                                     srx::MenuScreen::MATRIX_ENTRY);
}

bool FieldSceneRuntime::seed_matrix_degrade_action_for_dump() {
    if (!ready_ || !field_matrix_deck_record_.has_value() ||
        !field_matrix_route_active_ ||
        menu_route_screen_ != static_cast<std::uint16_t>(
                                  srx::MenuScreen::MATRIX_NODE_ACTION)) {
        return false;
    }

    auto& deck = *field_matrix_deck_record_;
    auto ratings = deck.program_ratings.value_or(
        std::array<std::uint8_t, 12U>{});
    ratings[2U] = 3U;
    deck.program_ratings = ratings;
    auto active = deck.active_programs.value_or(
        std::array<std::int8_t, 5U>{{-1, -1, -1, -1, -1}});
    active[0U] = 2;
    deck.active_programs = active;
    field_matrix_program_slots_[0U] =
        scripting::ScriptCyberdeckProgramSlot{};
    return true;
}

bool FieldSceneRuntime::seed_run_target_for_dump(
    const std::string_view run_id, const std::uint8_t map_id,
    const std::uint8_t event_id) {
    if (!ready_) {
        return false;
    }

    std::string category;
    std::string objective;
    std::string completion_condition;
    if (run_id == "employee_recruitment") {
        category = "extraction";
        objective = "recruit_the_selected_employee";
        completion_condition = "return_to_johnson_after_the_recruitment";
    } else if (run_id == "package_recovery") {
        category = "recovery";
        objective = "recover_the_client_package";
        completion_condition = "return_the_recovered_package_to_the_johnson";
    } else if (run_id == "gang_contract") {
        const bool rom_target =
            (map_id == 1U && event_id == 9U) ||
            (map_id == 2U && event_id == 9U) ||
            (map_id == 3U && event_id == 4U);
        if (!rom_target) {
            return false;
        }
        category = "gang";
        objective = "resolve_the_selected_gang_contract";
        completion_condition =
            "return_to_the_johnson_after_the_gang_contract";
    } else {
        return false;
    }

    using PersistentValue = scripting::ScriptPersistentValue;
    const auto scalar = [](auto value) {
        PersistentValue result{};
        result.value = value;
        return result;
    };
    const PersistentValue target{PersistentValue::Object{
        {"area_id", scalar(std::int64_t{map_id})},
        {"location_id", scalar(std::int64_t{event_id})},
    }};
    const PersistentValue route{PersistentValue::Object{
        {"target", target},
    }};
    const PersistentValue active_offer{PersistentValue::Object{
        {"id", scalar(std::string(run_id))},
        {"category", scalar(category)},
        {"objective", scalar(objective)},
        {"completion_condition", scalar(completion_condition)},
        {"runner_progress_delta", scalar(std::int64_t{0})},
        {"route", route},
    }};
    const PersistentValue registry_state{PersistentValue::Object{
        {"schema_version", scalar(std::int64_t{1})},
        {"current_offer", scalar(std::monostate{})},
        {"active_offer", active_offer},
        {"last_completed_offer", scalar(std::monostate{})},
        {"last_offer_result", scalar(std::string("accepted"))},
    }};
    return game_scripts_.restore_run_registry_state(registry_state);
}

bool FieldSceneRuntime::seed_current_run_for_dump(
    const std::uint8_t source_run_type,
    const bool matrix_max_payment,
    const std::optional<std::uint8_t> employee_source_initializer) {
    if (!ready_ || source_run_type > 6U ||
        (matrix_max_payment && source_run_type != 6U) ||
        (employee_source_initializer.has_value() &&
         (source_run_type != 5U || matrix_max_payment ||
          (*employee_source_initializer != 11U &&
           *employee_source_initializer != 12U)))) {
        return false;
    }

    using PersistentValue = scripting::ScriptPersistentValue;
    const auto type = static_cast<std::size_t>(source_run_type);
    // Each case is an accepted offer row from the ROM context table. The
    // values below keep the QA seed tied to real initializer/context/payment
    // combinations while presenting each of the seven shared detail handlers.
    static constexpr std::array<std::uint8_t, 7> kContexts{
        0U, 1U, 0U, 2U, 3U, 1U, 2U};
    static constexpr std::array<std::uint8_t, 7> kInitializers{
        0U, 1U, 2U, 3U, 9U, 5U, 6U};
    static constexpr std::array<std::int64_t, 7> kProgressDeltas{
        1, 1, 1, 2, 3, 2, 2};
    static constexpr std::array<std::int64_t, 7> kBasePayments{
        10, 200, 40, 190, 1700, 500, 425};
    static constexpr std::array<std::int64_t, 7> kNegotiationScales{
        3, 2, 1, 2, 4, 5, 1};
    static constexpr std::array<std::string_view, 7> kIds{
        "ghoul_bounty", "bodyguard_escort", "package_delivery",
        "gang_contract", "package_recovery", "employee_recruitment",
        "matrix_objective"};
    static constexpr std::array<std::string_view, 7> kCategories{
        "bounty", "escort", "courier", "gang", "recovery",
        "extraction", "matrix"};
    static constexpr std::array<std::string_view, 7> kObjectives{
        "kill_ghouls_in_redmond", "escort_client_to_destination",
        "deliver_package_to_destination", "resolve_the_selected_gang_contract",
        "recover_the_client_package", "recruit_the_selected_employee",
        "complete_the_selected_matrix_objective"};
    static constexpr std::array<std::string_view, 7> kCompletionConditions{
        "return_to_johnson_for_payment",
        "client_reaches_destination_alive", "package_delivered_safely",
        "return_to_the_johnson_after_the_gang_contract",
        "return_the_recovered_package_to_the_johnson",
        "return_to_johnson_after_the_recruitment",
        "complete_the_source_matrix_objective"};

    auto context = matrix_max_payment
        ? std::uint8_t{4U}
        : kContexts[type];
    auto initializer = matrix_max_payment
        ? std::uint8_t{8U}
        : kInitializers[type];
    auto progress_delta = matrix_max_payment
        ? std::int64_t{5}
        : kProgressDeltas[type];
    auto base_payment = matrix_max_payment
        ? std::int64_t{6100}
        : kBasePayments[type];
    auto negotiation_scale = matrix_max_payment
        ? std::int64_t{10}
        : kNegotiationScales[type];
    // ROM F3EBC context 1 zero-based slot 4 is initializer 11 (delta 4,
    // 900 base, scale 5); context 4 zero-based slot 4 is initializer 12
    // (delta 6, 3500 base, scale 15). These accepted QA rows exercise field
    // publication.
    if (employee_source_initializer == std::optional<std::uint8_t>{11U}) {
        context = 1U;
        initializer = 11U;
        progress_delta = 4;
        base_payment = 900;
        negotiation_scale = 5;
    } else if (employee_source_initializer ==
               std::optional<std::uint8_t>{12U}) {
        context = 4U;
        initializer = 12U;
        progress_delta = 6;
        base_payment = 3500;
        negotiation_scale = 15;
    }

    const auto scalar = [](auto value) {
        PersistentValue result{};
        result.value = value;
        return result;
    };
    const auto location = [&scalar](const std::int64_t area_id,
                                    const std::int64_t location_id) {
        return PersistentValue{PersistentValue::Object{
            {"area_id", scalar(area_id)},
            {"location_id", scalar(location_id)},
        }};
    };
    PersistentValue::Object route_fields;
    if (source_run_type == 1U) {
        route_fields.emplace("origin", location(2, 6));
        route_fields.emplace("destination", location(5, 6));
        route_fields.emplace("client_id", scalar(std::int64_t{7}));
    } else if (source_run_type == 2U) {
        // Section 03 video route: Ares Weapon Emporium to Jackal's Lantern.
        route_fields.emplace("origin", location(2, 11));
        route_fields.emplace("destination", location(2, 5));
    } else if (source_run_type == 3U) {
        route_fields.emplace("target", location(3, 4));
    } else if (source_run_type == 4U) {
        // First site in the source two-site package-recovery pool.
        route_fields.emplace("target", location(1, 2));
    } else if (source_run_type == 5U) {
        if (initializer == 11U) {
            // First target in the context-1 two-site source pool.
            route_fields.emplace("target", location(1, 2));
        } else if (initializer == 12U) {
            // The fixed Renraku target selected by the context-4 row.
            route_fields.emplace("target", location(5, 6));
        } else {
            // First site in the shared four-site employee-recruitment pool.
            route_fields.emplace("target", location(0, 5));
        }
    }
    const PersistentValue route{std::move(route_fields)};

    const std::int64_t negotiation_skill = matrix_max_payment
        ? 12
        : (source_run_type == 0U ? 0 : 3);
    const auto negotiation_bound =
        negotiation_scale * negotiation_skill + 1;
    const auto negotiation_roll = negotiation_bound - 1;
    const PersistentValue payment{PersistentValue::Object{
        {"base_amount", scalar(base_payment)},
        {"negotiated_amount",
         scalar(base_payment + negotiation_roll * 5)},
        {"negotiation_skill", scalar(negotiation_skill)},
        {"negotiation_bound", scalar(negotiation_bound)},
        {"negotiation_roll", scalar(negotiation_roll)},
    }};
    PersistentValue::Object offer_fields{
        {"id", scalar(std::string(kIds[type]))},
        {"category", scalar(std::string(kCategories[type]))},
        {"objective", scalar(std::string(kObjectives[type]))},
        {"completion_condition",
         scalar(std::string(kCompletionConditions[type]))},
        {"runner_progress_delta", scalar(progress_delta)},
        {"source_initializer", scalar(static_cast<std::int64_t>(initializer))},
        {"source_offer_type", scalar(static_cast<std::int64_t>(initializer))},
        {"source_offer_context", scalar(static_cast<std::int64_t>(context))},
        {"source_run_type", scalar(static_cast<std::int64_t>(source_run_type))},
        {"context", scalar(static_cast<std::int64_t>(context))},
        {"route", route},
        {"payment", payment},
    };
    if (source_run_type == 0U) {
        offer_fields.emplace("area_id", scalar(std::int64_t{2}));
        offer_fields.emplace("ghoul_kill_count", scalar(std::int64_t{19}));
    } else if (source_run_type == 3U) {
        offer_fields.emplace("variant", scalar(std::int64_t{1}));
    } else if (source_run_type == 5U) {
        offer_fields.emplace("client_id", scalar(std::int64_t{0}));
    } else if (source_run_type == 6U) {
        // The regular case is a simple row. The optional maximum-payment case
        // is source Matrix Expert row 7, rank 12, roll 120, displaying 6,700.
        const auto objective_index = matrix_max_payment ? 0 : 2;
        const auto objective_class = matrix_max_payment ? 4 : 0;
        const auto objective_value = matrix_max_payment ? 20 : 1;
        const auto matrix_state = matrix_max_payment ? 1 : -1;
        const auto target_system_id = matrix_max_payment ? 7 : 0;
        offer_fields.emplace("objective_index", scalar(objective_index));
        offer_fields.emplace("target_system_id", scalar(target_system_id));
        offer_fields.emplace(
            "matrix_run_state",
            PersistentValue{PersistentValue::Object{
                {"state", scalar(matrix_state)},
                {"objective", scalar(objective_class)},
                {"value", scalar(objective_value)},
                {"size", scalar(std::int64_t{20})},
                {"fc28", scalar(std::int64_t{0})},
                {"fc1c", scalar(std::int64_t{0})},
                {"target_system_id", scalar(target_system_id)},
            }});
    }
    const PersistentValue active_offer{std::move(offer_fields)};
    const PersistentValue registry_state{PersistentValue::Object{
        {"schema_version", scalar(std::int64_t{1})},
        {"current_offer", scalar(std::monostate{})},
        {"active_offer", active_offer},
        {"last_completed_offer", scalar(std::monostate{})},
        {"last_offer_result", scalar(std::string("accepted"))},
    }};
    return game_scripts_.restore_run_registry_state(registry_state);
}

bool FieldSceneRuntime::seed_employee_recruitment_run_target_for_dump(
    const std::uint8_t map_id, const std::uint8_t event_id) {
    using PersistentValue = scripting::ScriptPersistentValue;
    if (!seed_current_run_for_dump(5U)) {
        return false;
    }
    auto registry_state = game_scripts_.save_run_registry_state();
    if (!registry_state.has_value()) {
        return false;
    }
    auto* root = std::get_if<PersistentValue::Object>(
        &registry_state->value);
    if (root == nullptr) {
        return false;
    }
    const auto active_offer = root->find("active_offer");
    if (active_offer == root->end()) {
        return false;
    }
    auto* offer = std::get_if<PersistentValue::Object>(
        &active_offer->second.value);
    if (offer == nullptr) {
        return false;
    }
    const auto route_value = offer->find("route");
    if (route_value == offer->end()) {
        return false;
    }
    auto* route = std::get_if<PersistentValue::Object>(
        &route_value->second.value);
    if (route == nullptr) {
        return false;
    }
    const auto scalar = [](const std::uint8_t value) {
        PersistentValue result{};
        result.value = static_cast<std::int64_t>(value);
        return result;
    };
    route->insert_or_assign(
        "target",
        PersistentValue{PersistentValue::Object{
            {"area_id", scalar(map_id)},
            {"location_id", scalar(event_id)},
        }});
    return game_scripts_.restore_run_registry_state(*registry_state);
}

bool FieldSceneRuntime::open_taxi_selector_for_dump(
    const std::uint8_t current_region) {
    if (!ready_ || current_region > 6U) {
        return false;
    }
    scripting::ScriptMenuRouteStartArguments arguments{};
    arguments.taxi_current_region = current_region;
    open_command_overview(
        static_cast<std::uint16_t>(srx::MenuScreen::TAXI_SELECTOR),
        std::move(arguments));
    return command_overview_.open() && field_taxi_route_active_ &&
           menu_route_screen_ == static_cast<std::uint16_t>(
                                    srx::MenuScreen::TAXI_SELECTOR);
}

void FieldSceneRuntime::set_inventory_action_overlay_probe_for_dump(
    const bool active) noexcept {
    command_overview_inventory_action_active_ = active;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::open_attributes_skills_for_dump() {
    if (!open_command_overview_for_dump() ||
        !seek_command_overview_page(3U)) {
        return false;
    }
    // Exercise the same command-row A dispatch as the focused Qt window.
    set_controller_port_byte(0x40U);
    advance_frame();
    set_controller_port_byte(0U);
    advance_frame();
    return command_overview_attributes_skills_.is_open();
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::open_stoker_room_payment_for_dump(
    const std::uint32_t funds) {
    if (!ready_) {
        return false;
    }
    field_funds_ = funds;
    if (!field_dialogue_.open_stoker_prompt_for_dump(
            StokerMotelPromptId::room_offer)) {
        return false;
    }
    return field_dialogue_.advance_controller_events(0x4000U) ==
           FieldDialogueAdvanceResult::prompt_advanced;
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
FieldDialogueAdvanceResult
FieldSceneRuntime::advance_stoker_dialogue_for_dump(
    const std::uint16_t event_word) {
    const auto result = field_dialogue_.advance_controller_events(event_word);
    if (result == FieldDialogueAdvanceResult::hotel_options_requested) {
        begin_hotel_menu();
    }
    return result;
}

bool FieldSceneRuntime::dispatch_dialogue_outcome_for_dump() {
    const auto outcome = field_dialogue_.take_gameplay_task_outcome();
    return outcome.has_value() && dispatch_dialogue_outcome(*outcome);
}
#endif // SHADOWRUN_QT_QA_DRIVER


bool FieldSceneRuntime::open_gunderson_interaction() {
    if (!ready_ || state_.field_scene_buffers.scene_reference != 13U) {
        return false;
    }
    return open_field_johnson_interaction();
}

bool FieldSceneRuntime::open_field_location_dialogue(
    const FieldScriptRoute& route) {
    if (route.kind == FieldScriptRouteKind::source_boundary) {
        source_boundary(route.boundary_reason);
        return false;
    }
    if (!ready_ || route.kind != FieldScriptRouteKind::dialogue ||
        route.script != FieldScriptId::lua_dialogue ||
        route.entry_dialogue_module.empty() ||
        route.entry_dialogue_function.empty() ||
        route.entry_location_type.empty() ||
        route.entry_location_id.empty()) {
        return false;
    }

    game_scripts_.set_location_context(
        route.entry_location_type, route.entry_location_id,
        route.entry_text_profile, route.entry_speaker_id,
        route.entry_event_id);
    game_scripts_.set_field_mode_context(scripting::ScriptFieldModeContext{
        state_.field_scene_buffers.field_resource_mode,
        route.event_code,
        field_exit_alternate_result_,
        state_.field_scene_buffers.scene_mode,
        field_movement_transition_state_.field_transition_context});

    if (route.entry_dialogue_module == "johnson") {
        const auto context = game_scripts_.johnson_interaction_context(
            route.entry_text_profile);
        if (!context.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.run.johnson_profile_unresolved");
            return false;
        }
        const auto run_return =
            game_scripts_.resolve_active_run_johnson_return(*context);
        if (!run_return.has_value()) {
            failure_ = game_scripts_.failure();
            source_boundary("field.run.johnson_return_script_failed");
            return false;
        }
        if (run_return->action ==
            scripting::ScriptRunJohnsonReturnAction::completed) {
            if (!field_funds_.has_value() ||
                !field_party_roster_.available()) {
                failure_ =
                    "completed run return has no field wallet or runner roster";
                source_boundary(
                    "field.run.johnson_return_host_state_unresolved");
                return false;
            }
            *field_funds_ += run_return->reward_nuyen;
            for (auto& runner : field_party_roster_.records()) {
                runner.auxiliary[2] = static_cast<std::uint8_t>(
                    runner.auxiliary[2] +
                    run_return->runner_progress_delta);
            }
            if (group_item_flags_.has_value()) {
                *group_item_flags_ &= ~group_item_flag_mask(
                    GroupItemFlag::run_objective_active);
            } else {
                source_boundary(
                    "field.run.objective_group_item_flag_unresolved");
            }
            refresh_field_hud_runner_slot();
            source_boundary("field.run.johnson_return_completed");
        }
    }

    if (route.return_anchor != FieldLocationReturnAnchor::none) {
        save_field_door_return_point();
    }
    if (!field_dialogue_.open_gameplay_task(
            route.entry_dialogue_module,
            route.entry_dialogue_function)) {
        source_boundary("field.location.dialogue_task_open_failed");
        return false;
    }
    field_palette_fade_steps_ = 0U;
    source_boundary("field.location.dialogue_task_opened");
    return true;
}

bool FieldSceneRuntime::open_field_johnson_interaction() {
    if (!ready_) {
        return false;
    }
    const auto& event_metadata =
        state_.field_scene_buffers.source_metadata;
    const auto route = game_scripts_.resolve_field_location_route(
        FieldScriptRouteContext{
            static_cast<std::uint8_t>(
                state_.field_scene_buffers.scene_reference),
            state_.field_scene_buffers.field_resource_mode,
            FieldCollisionEventCode::event_30,
            event_metadata.event_table_available,
            event_metadata.event_mode_table,
            field_exit_alternate_result_});
    if (!route.has_value()) {
        failure_ = game_scripts_.failure();
        return false;
    }
    active_field_script_route_ = *route;
    return open_field_location_dialogue(*route);
}

#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::open_gunderson_interaction_for_dump() {
    return open_gunderson_interaction();
}
#endif // SHADOWRUN_QT_QA_DRIVER


FieldInteractionAdvanceResult
FieldSceneRuntime::advance_field_interaction(
    const std::uint16_t event_word) {
    std::array<bool, FieldPartyRosterRuntime::kSlotCount> active_slots{};
    for (std::size_t slot = 0U; slot < active_slots.size(); ++slot) {
        active_slots[slot] = field_party_roster_.slot_active(slot);
    }
    const auto result = field_interaction_.advance_controller_events(
        event_word, field_party_roster_.records(), active_slots,
        field_movement_transition_state_.activity_selection,
        field_random_seed_);
    if (result == FieldInteractionAdvanceResult::returned_to_field) {
        // The source re-enters the field renderer after the prompt callback.
        // This host redraw replaces its full 320x224 image and consumes the
        // coordinator's display-service marker here. ROM 0x2BD2 also commits
        // a negative interaction result to the sampled field event counter;
        // passkey success uses FFE16E = 0xFF to advance the A2 door cell.
        state_.display_service_word = 0U;
        field_runtime_state_.display_service_word = 0U;
        source_boundary("field.interaction.returned_to_field");
        advance_field_scene_transition_after_modal();
        begin_field_modal_return_transition();
    }
    refresh_field_hud_runner_slot();
    return result;
}

void FieldSceneRuntime::begin_hotel_menu() {
    if (!ready_) {
        return;
    }
    std::array<std::string, FieldPartyRosterRuntime::kSlotCount> names{};
    for (std::size_t slot = 0; slot < names.size(); ++slot) {
        names[slot] = field_party_roster_.name(slot);
    }
    hotel_menu_.open(field_party_roster_.records(), names,
                     field_party_roster_.active_slot());
    refresh_field_hud_runner_slot();
    source_boundary("field.hotel.entity_status.opened");
}

void FieldSceneRuntime::register_dialogue_outcome_handlers() {
    const bool registered =
        dialogue_outcome_dispatcher_.register_type_handler<
            scripting::ContinueInPlaceOutcome>(
            [this](const scripting::ContinueInPlaceOutcome& outcome) {
                return handle_dialogue_continue_in_place(outcome);
            }) &&
        dialogue_outcome_dispatcher_.register_target_handler<
            scripting::PlayCinematicOutcome>(
            "salish.spirit_eyes.ritual_of_binding",
            [this](const scripting::PlayCinematicOutcome& outcome) {
                return handle_dialogue_play_cinematic(outcome);
            }) &&
        dialogue_outcome_dispatcher_.register_target_handler<
            scripting::PlayCinematicOutcome>(
            "salish.sinsearach.frosty_harlequin_exposition",
            [this](const scripting::PlayCinematicOutcome& outcome) {
                return handle_dialogue_play_cinematic(outcome);
            }) &&
        dialogue_outcome_dispatcher_.register_target_handler<
            scripting::PlayCinematicOutcome>(
            "tarislar.tomb.credits",
            [this](const scripting::PlayCinematicOutcome& outcome) {
                return handle_dialogue_play_cinematic(outcome);
            }) &&
        dialogue_outcome_dispatcher_.register_type_handler<
            scripting::ResumeSourceRouteOutcome>(
            [this](const scripting::ResumeSourceRouteOutcome& outcome) {
                return handle_dialogue_resume_source_route(outcome);
            }) &&
        dialogue_outcome_dispatcher_.register_type_handler<
            scripting::EnterFieldSceneOutcome>(
            [this](const scripting::EnterFieldSceneOutcome& outcome) {
                return handle_dialogue_enter_field_scene(outcome);
            }) &&
        dialogue_outcome_dispatcher_.register_target_handler<
            scripting::ReturnToOriginOutcome>(
            "source_entry",
            [this](const scripting::ReturnToOriginOutcome& outcome) {
                return handle_dialogue_return_to_source_entry(outcome);
            }) &&
        dialogue_outcome_dispatcher_.register_target_handler<
            scripting::OpenServiceOutcome>(
            "hotel.options",
            [this](const scripting::OpenServiceOutcome& outcome) {
                return open_hotel_dialogue_service(outcome);
            }) &&
        dialogue_outcome_dispatcher_.register_target_handler<
            scripting::OpenServiceOutcome>(
            "hospital.options",
            [this](const scripting::OpenServiceOutcome& outcome) {
                return open_hospital_dialogue_service(outcome);
            });

    if (!registered) {
        source_boundary(
            "field.dialogue.outcome_handler_registration_failed");
    }
}

bool FieldSceneRuntime::dispatch_dialogue_outcome(
    const scripting::DialogueOutcome& outcome) {
    if (const auto* boundary = std::get_if<scripting::SourceBoundaryOutcome>(
            &outcome)) {
        source_boundary("field.dialogue.source_boundary:" + boundary->reason);
        return field_dialogue_.present_source_boundary_for_host(
            boundary->reason);
    }

    const auto result = dialogue_outcome_dispatcher_.dispatch(outcome);
    if (result.handled()) {
        return true;
    }

    const auto status =
        result.status ==
                scripting::DialogueOutcomeDispatcher::Status::handler_rejected
            ? "handler_rejected"
            : "target_not_registered";
    const auto type_name = scripting::dialogue_outcome_type_name(
        scripting::dialogue_outcome_type(outcome));
    std::string reason = "field.dialogue.outcome." +
                         std::string(status) + ":" +
                         std::string(type_name);
    const auto target_id = scripting::dialogue_outcome_target_id(outcome);
    if (!target_id.empty()) {
        reason += ":" + std::string(target_id);
    }
    source_boundary(reason);
    return field_dialogue_.present_source_boundary_for_host(reason);
}

bool FieldSceneRuntime::handle_dialogue_continue_in_place(
    const scripting::ContinueInPlaceOutcome& /* outcome */) {
    if (!ready_) {
        return false;
    }
    if (field_contact_actor_cleanup_pending_) {
        field_contact_actor_cleanup_pending_ = false;
        if (!retire_dialogue_escort_records()) {
            return false;
        }
    }
    hospital_service_session_active_ = false;
    field_mode_entry_transition_pending_ = false;
    field_mode_entry_destination_scene_.reset();
    field_mode_entry_run_target_actor_group_.reset();
    source_boundary("field.dialogue.continued_in_place");
    advance_field_scene_transition_after_modal();
    begin_field_modal_return_transition();
    return true;
}

bool FieldSceneRuntime::handle_dialogue_play_cinematic(
    const scripting::PlayCinematicOutcome& outcome) {
    if (!ready_ || outcome.cinematic_id.empty()) {
        return false;
    }
    if (outcome.return_policy == "source_context") {
        source_boundary("field.dialogue.cinematic.completed:" +
                        outcome.cinematic_id);
        return handle_dialogue_continue_in_place(
            scripting::ContinueInPlaceOutcome{});
    }
    if (outcome.cinematic_id == "tarislar.tomb.credits" &&
        outcome.return_policy == "title") {
        if (!field_dialogue_.retain_closed_credits_page_for_title_return()) {
            return false;
        }
        if (!tarislar_credits_title_return_.begin()) {
            field_dialogue_.clear_retained_credits_page_for_title_return();
            return false;
        }
        return true;
    }
    return false;
}

bool FieldSceneRuntime::handle_dialogue_resume_source_route(
    const scripting::ResumeSourceRouteOutcome& /* outcome */) {
    if (!ready_) {
        return false;
    }
    const bool outcome_matches_route =
        active_field_script_route_.has_value() &&
        active_field_script_route_->entry_continue_outcome_type &&
        scripting::DialogueOutcomeType::resume_source_route ==
            *active_field_script_route_->entry_continue_outcome_type;
    if (!field_mode_entry_transition_pending_ || !outcome_matches_route) {
        field_mode_entry_transition_pending_ = false;
        field_mode_entry_destination_scene_.reset();
        field_mode_entry_run_target_actor_group_.reset();
        source_boundary("field.entry.typed_outcome_did_not_match_route");
        advance_field_scene_transition_after_modal();
        begin_field_modal_return_transition();
        return true;
    }

    source_boundary("field.entry.source_route_resumed");
    if (!resume_field_mode_entry_route()) {
        source_boundary("field.entry.area_event_route_resume_failed");
    }
    begin_field_modal_return_transition();
    return true;
}

bool FieldSceneRuntime::handle_dialogue_enter_field_scene(
    const scripting::EnterFieldSceneOutcome& outcome) {
    if (!ready_) {
        return false;
    }
    if (outcome.arrival_id == "salish_sinsearach_copt") {
        if (state_.field_scene_buffers.scene_reference != 9U ||
            outcome.scene_id != 6U ||
            field_dialogue_scene_pending_destination_.has_value() ||
            field_elevator_scene_transition_.is_active()) {
            source_boundary(
                "field.dialogue.scene_entry.typed_route_mismatch");
            return true;
        }
        field_dialogue_scene_pending_destination_ = outcome.scene_id;
        if (field_elevator_scene_transition_.begin_fade_out()) {
            field_dialogue_scene_transition_active_ = true;
            source_boundary(
                "field.dialogue.scene_entry.fade_out_started");
        } else {
            field_dialogue_scene_pending_destination_.reset();
            source_boundary(
                "field.dialogue.scene_entry.transition_busy");
        }
        return true;
    }
    const bool outcome_matches_service =
        outcome.arrival_id == "rom_default" &&
        field_phone_pending_destination_.has_value() &&
        outcome.scene_id == *field_phone_pending_destination_;
    if (!outcome_matches_service) {
        field_phone_pending_destination_.reset();
        source_boundary("field.vid_phone.typed_scene_outcome_mismatch");
        return true;
    }
    if (field_elevator_scene_transition_.begin_fade_out()) {
        field_phone_transition_active_ = true;
        source_boundary("field.vid_phone.passage_fade_out_started");
    } else {
        field_phone_pending_destination_.reset();
        source_boundary("field.vid_phone.passage_transition_busy");
    }
    return true;
}

bool FieldSceneRuntime::handle_dialogue_return_to_source_entry(
    const scripting::ReturnToOriginOutcome& outcome) {
    if (!ready_ || outcome.return_point_id != "source_entry" ||
        (outcome.return_policy.has_value() &&
         *outcome.return_policy != "recorded_entry") ||
        !field_door_return_point_.valid) {
        return false;
    }
    begin_field_door_return();
    refresh_field_hud_runner_slot();
    return field_door_return_active_;
}

bool FieldSceneRuntime::prepare_open_dialogue_service(
    const scripting::OpenServiceOutcome& outcome) {
    if (!ready_ ||
        outcome.return_policy != "origin") {
        return false;
    }
    field_mode_entry_transition_pending_ = false;
    field_mode_entry_destination_scene_.reset();
    field_mode_entry_run_target_actor_group_.reset();
    return true;
}

bool FieldSceneRuntime::open_hotel_dialogue_service(
    const scripting::OpenServiceOutcome& outcome) {
    if (!prepare_open_dialogue_service(outcome)) {
        return false;
    }
    begin_hotel_menu();
    return hotel_menu_.open();
}

bool FieldSceneRuntime::open_hospital_dialogue_service(
    const scripting::OpenServiceOutcome& outcome) {
    if (!prepare_open_dialogue_service(outcome)) {
        return false;
    }
    hospital_service_session_active_ = false;
    game_scripts_.set_location_context(
        "building", "council_island_hospital",
        "council_island_hospital", "council_island_hospital_narrator");
    game_scripts_.set_field_mode_context(
        scripting::ScriptFieldModeContext{
            4U, 4U, 0, state_.field_scene_buffers.scene_mode,
            field_movement_transition_state_.field_transition_context});
    return begin_hospital_menu_task("present_options");
}

bool FieldSceneRuntime::load_command_overview_menu_content() {
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.command_overview_script_invalid");
        return false;
    };
    const auto definition = game_scripts_.menu(
        "command_overview", "root", "field.command_overview", "root");
    if (!definition.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "command overview root menu script is unavailable"
                        : game_scripts_.failure());
    }
    if (definition->choices.size() != field_menu_text_.command_labels.size()) {
        return fail(
            "command overview Lua menu does not match the seven source rows");
    }
    for (std::size_t index = 0; index < definition->choices.size(); ++index) {
        const auto& choice = definition->choices[index];
        const auto* label = game_scripts_.dialogue_text(choice.label_key);
        if (label == nullptr) {
            return fail("command overview Lua choice label is missing");
        }
        field_menu_text_.command_labels[index] = *label;
    }
    source_boundary("field.command_overview.menu_content.loaded_from_lua");
    return true;
}

bool FieldSceneRuntime::load_hotel_menu_content() {
    using scripting::ScriptMenuDefinition;
    constexpr std::array<std::string_view, 2> kOptionIds{{
        "karma_screen", "check_out"}};
    constexpr std::array<std::string_view, 2> kCheckoutIds{{
        "checkout_yes", "checkout_no"}};
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.hotel_menu_script_invalid");
        return false;
    };
    const auto options = game_scripts_.menu(
        "hotel", "options", "hotel", "options");
    if (!options.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Hotel options menu script is unavailable"
                        : game_scripts_.failure());
    }
    const auto checkout = game_scripts_.menu(
        "hotel", "checkout", "hotel", "checkout");
    if (!checkout.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Hotel checkout menu script is unavailable"
                        : game_scripts_.failure());
    }
    const auto presentation =
        game_scripts_.string_map("hotel", "presentation");
    if (!presentation.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Hotel presentation script is unavailable"
                        : game_scripts_.failure());
    }
    const auto timing = [&presentation](const char* key)
        -> std::optional<std::uint32_t> {
        const auto found = presentation->find(key);
        if (found == presentation->end()) {
            return std::nullopt;
        }
        try {
            return static_cast<std::uint32_t>(std::stoul(found->second));
        } catch (...) {
            return std::nullopt;
        }
    };
    const auto status_text_delay = timing("status_text_delay_frames");
    const auto options_redraw = timing("options_redraw_frames");
    const auto karma_load = timing("karma_load_black_frames");
    if (!status_text_delay.has_value() || !options_redraw.has_value() ||
        !karma_load.has_value()) {
        return fail("Hotel presentation timing is not numeric");
    }
    hotel_karma_load_black_frames_ = *karma_load;
    const auto checkout_fade_out = timing("checkout_fade_out_start_frame");
    const auto checkout_fade_in = timing("checkout_fade_in_start_frame");
    const auto checkout_resume = timing("checkout_input_resume_frame");
    const auto checkout_camera = timing("checkout_camera_settle_start_frame");
    if (!checkout_fade_out.has_value() || !checkout_fade_in.has_value() ||
        !checkout_resume.has_value() || !checkout_camera.has_value() ||
        *checkout_camera >= *checkout_fade_in ||
        *checkout_fade_in <= *checkout_fade_out + 14U ||
        *checkout_resume <= *checkout_fade_in) {
        return fail("Hotel checkout timing is not numeric or ordered");
    }
    checkout_fade_out_start_frame_ = *checkout_fade_out;
    checkout_fade_in_start_frame_ = *checkout_fade_in;
    checkout_input_resume_frame_ = *checkout_resume;
    checkout_camera_settle_start_frame_ = *checkout_camera;
    hotel_menu_.set_presentation_timing(*status_text_delay, *options_redraw);
    const auto text_keys = game_scripts_.string_map("hotel", "text_keys");
    if (!text_keys.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Hotel text-key script is unavailable"
                        : game_scripts_.failure());
    }
    if (options->choices.size() != kOptionIds.size() ||
        checkout->choices.size() != kCheckoutIds.size()) {
        return fail("Hotel Lua menus do not match the source choice count");
    }
    HotelMenuContent content{};
    const auto copy_text = [this](const std::string_view key,
                                  std::string& destination) {
        const auto* source = game_scripts_.dialogue_text(key);
        if (source == nullptr) {
            return false;
        }
        destination = *source;
        return true;
    };
    const auto copy_named_text = [&text_keys, &copy_text](
                                    const std::string_view name,
                                    std::string& destination) {
        const auto key = text_keys->find(std::string(name));
        return key != text_keys->end() &&
               copy_text(key->second, destination);
    };
    const auto* title = game_scripts_.dialogue_text(options->title_key);
    if (title == nullptr ||
        !copy_named_text("options_separator", content.separator)) {
        return fail("Hotel options title or separator text is missing");
    }
    content.title = *title;
    for (std::size_t index = 0; index < kOptionIds.size(); ++index) {
        const auto& choice = options->choices[index];
        if (choice.id != kOptionIds[index] ||
            !copy_text(choice.label_key, content.options[index].label)) {
            return fail("Hotel options Lua choice ID or label is invalid");
        }
        content.options[index].id = choice.id;
    }
    const auto* checkout_question =
        game_scripts_.dialogue_text(checkout->title_key);
    if (checkout_question == nullptr) {
        return fail("Hotel checkout question text is missing");
    }
    const auto question_break = checkout_question->find('|');
    if (question_break == std::string::npos ||
        checkout_question->find('|', question_break + 1U) !=
            std::string::npos) {
        return fail("Hotel checkout question must contain two source rows");
    }
    content.checkout_question[0] =
        checkout_question->substr(0U, question_break);
    content.checkout_question[1] =
        checkout_question->substr(question_break + 1U);
    for (std::size_t index = 0; index < kCheckoutIds.size(); ++index) {
        const auto& choice = checkout->choices[index];
        if (choice.id != kCheckoutIds[index] ||
            !copy_text(choice.label_key,
                       content.checkout_choices[index].label)) {
            return fail("Hotel checkout Lua choice ID or label is invalid");
        }
        content.checkout_choices[index].id = choice.id;
    }
    if (!copy_named_text("medical_attention_suffix",
                         content.medical_attention_suffix)) {
        return fail("Hotel medical-attention text is missing");
    }
    constexpr std::array<std::string_view, 3> kRecoveryTextNames{{
        "recovery_2", "recovery_3", "recovery_4"}};
    for (std::size_t index = 0; index < kRecoveryTextNames.size(); ++index) {
        const auto destination_index = index + 2U;
        if (!copy_named_text(kRecoveryTextNames[index],
                             content.recovery_message[destination_index])) {
            return fail("Hotel recovery text is incomplete");
        }
    }
    hotel_menu_content_ = std::move(content);
    source_boundary("field.hotel.menu_content.loaded_from_lua");
    return true;
}

bool FieldSceneRuntime::load_pocket_secretary_menu_content() {
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.pocket_secretary_script_invalid");
        return false;
    };
    const auto definition = game_scripts_.menu(
        "pocket_secretary", "root", "pocket_secretary", "root");
    if (!definition.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Pocket Secretary root menu script is unavailable"
                        : game_scripts_.failure());
    }
    if (definition->choices.size() != kPocketSecretaryFieldMenuRowCount) {
        return fail(
            "Pocket Secretary Lua root menu has the wrong row count");
    }
    const auto root_text_definition = game_scripts_.menu(
        "pocket_secretary", "root_text_entries",
        "pocket_secretary.root.text", "catalog");
    if (!root_text_definition.has_value() ||
        root_text_definition->choices.size() != 1U) {
        return fail("Pocket Secretary root separator script entry is invalid");
    }
    const auto notebook_definition = game_scripts_.menu(
        "pocket_secretary", "notebook", "pocket_secretary.notebook",
        "options");
    if (!notebook_definition.has_value()) {
        return fail(game_scripts_.failure().empty()
                        ? "Pocket Secretary Notebook menu script is unavailable"
                        : game_scripts_.failure());
    }
    if (notebook_definition->choices.size() != kNotebookChoiceCount) {
        return fail(
            "Pocket Secretary Notebook menu has the wrong row count");
    }
    const auto* title = game_scripts_.dialogue_text(definition->title_key);
    const auto* separator = game_scripts_.dialogue_text(
        root_text_definition->choices[0].label_key);
    if (title == nullptr || separator == nullptr) {
        return fail("Pocket Secretary root title or separator text is missing");
    }
    PocketSecretaryMenuContent content{};
    content.title = *title;
    content.separator = *separator;
    for (std::size_t index = 0; index < definition->choices.size(); ++index) {
        const auto& choice = definition->choices[index];
        const auto* label = game_scripts_.dialogue_text(choice.label_key);
        if (label == nullptr) {
            return fail("Pocket Secretary Lua choice label is missing");
        }
        content.choices[index] = PocketSecretaryMenuContent::Choice{
            choice.id, *label};
    }
    for (std::size_t index = 0;
         index < notebook_definition->choices.size(); ++index) {
        const auto& choice = notebook_definition->choices[index];
        if (game_scripts_.dialogue_text(choice.label_key) == nullptr) {
            return fail(
                "Pocket Secretary Notebook choice label is missing");
        }
        content.notebook_choice_ids[index] = choice.id;
    }
    pocket_secretary_menu_content_ = std::move(content);
    source_boundary("field.pocket_secretary.menu_content.loaded_from_lua");
    return true;
}

bool FieldSceneRuntime::load_ares_shop_menu_content() {
    constexpr std::array<std::string_view, 6> kGroupIds{{
        "weapons", "armor", "other_items", "source_action_3",
        "source_action_4", "sell"}};
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.ares_shop_script_invalid");
        return false;
    };
    const auto service_titles = game_scripts_.menu(
        "shop", "services", "shop.service", "catalog");
    const auto action_headings = game_scripts_.menu(
        "shop", "action_headings", "shop.action_heading", "catalog");
    if (!field_shop_definition_.has_value() ||
        !service_titles.has_value() || !action_headings.has_value() ||
        field_shop_definition_->id != "ares_weapon_emporium" ||
        field_shop_definition_->groups.size() != kGroupIds.size() ||
        service_titles->choices.size() !=
            field_menu_text_.field_service_titles.size() ||
        action_headings->choices.size() !=
            field_menu_text_.field_service_action_titles.size()) {
        return fail("Ares shop Lua definition does not match its source page");
    }

    std::string_view ares_service_title_key;
    for (std::size_t index = 0;
         index < service_titles->choices.size(); ++index) {
        const auto& choice = service_titles->choices[index];
        const auto* title = game_scripts_.dialogue_text(choice.label_key);
        if (title == nullptr) {
            return fail("Lua shop service title text is incomplete");
        }
        field_menu_text_.field_service_titles[index] = *title;
        if (choice.id == field_shop_definition_->id) {
            ares_service_title_key = choice.label_key;
        }
    }
    if (ares_service_title_key.empty() ||
        field_shop_definition_->title_key != ares_service_title_key) {
        return fail("Ares title is not bound to its ROM service record");
    }

    for (std::size_t index = 0; index < action_headings->choices.size();
         ++index) {
        const auto& choice = action_headings->choices[index];
        const auto* title = game_scripts_.dialogue_text(choice.label_key);
        if (title == nullptr || title->size() != 20U) {
            return fail("Lua shop action-heading catalog is incomplete");
        }
        field_menu_text_.field_service_action_titles[index] = *title;
    }

    std::array<std::string_view, kGroupIds.size() * 2U>
        ares_action_heading_keys{};
    std::size_t ares_action_heading_count = 0U;
    for (std::size_t group_index = 0; group_index < kGroupIds.size();
         ++group_index) {
        const auto& group = field_shop_definition_->groups[group_index];
        if (group.id != kGroupIds[group_index]) {
            return fail("Ares shop Lua categories changed source order");
        }
        for (const auto& title_key : group.title_keys) {
            const auto heading = std::find_if(
                action_headings->choices.begin(),
                action_headings->choices.end(),
                [&title_key](const scripting::ScriptMenuChoice& choice) {
                    return choice.label_key == title_key;
                });
            if (heading == action_headings->choices.end() ||
                std::find(ares_action_heading_keys.begin(),
                          ares_action_heading_keys.begin() +
                              static_cast<std::ptrdiff_t>(
                                  ares_action_heading_count),
                          std::string_view(title_key)) !=
                    ares_action_heading_keys.begin() +
                        static_cast<std::ptrdiff_t>(
                            ares_action_heading_count)) {
                return fail("Ares shop Lua category heading is invalid");
            }
            ares_action_heading_keys[ares_action_heading_count++] = title_key;
        }
    }
    if (ares_action_heading_count != ares_action_heading_keys.size()) {
        return fail(
            "Ares shop Lua categories do not cover their heading keys");
    }
    const auto copy_shop_text = [this](const std::string& key,
                                       std::string& destination) {
        const auto* text = game_scripts_.dialogue_text(key);
        if (text == nullptr) {
            return false;
        }
        destination = *text;
        return true;
    };
    const auto& keys = field_shop_definition_->text_keys;
    if (!copy_shop_text(keys.universal_clips_title,
                        field_menu_text_.field_service_universal_clips_title) ||
        !copy_shop_text(keys.universal_clips_description,
                        field_menu_text_.field_service_universal_clips_description) ||
        !copy_shop_text(keys.transaction_prompt,
                        field_menu_text_.shop_transaction_prompt) ||
        !copy_shop_text(keys.no_inventory_slot,
                        field_menu_text_.shop_no_inventory_slot) ||
        !copy_shop_text(keys.insufficient_funds,
                        field_menu_text_.shop_insufficient_funds) ||
        !copy_shop_text(keys.special_action_limit,
                        field_menu_text_.shop_special_action_limit) ||
        !copy_shop_text(keys.protected_sale,
                        field_menu_text_.shop_protected_sale) ||
        !copy_shop_text(keys.sale_quote_heading,
                        field_menu_text_.shop_sale_quote_heading) ||
        !copy_shop_text(keys.purchase_price_heading,
                        field_menu_text_.shop_purchase_price_heading)) {
        return fail("Ares shop Lua transaction text is incomplete");
    }
    source_boundary("field.ares_shop.menu_content_loaded_from_lua");
    return true;
}

bool FieldSceneRuntime::load_item_menu_content() {
    const auto fail = [this](std::string message) {
        failure_ = std::move(message);
        source_boundary("field.bootstrap.item_script_invalid");
        return false;
    };
    const auto catalog = game_scripts_.item_catalog();
    if (!catalog.has_value() ||
        catalog->items.size() != field_menu_text_.items.size() ||
        catalog->detail_text_keys.size() !=
            field_menu_text_.field_action_detail_texts.size()) {
        return fail(game_scripts_.failure().empty()
                        ? "Lua item catalog does not match source table sizes"
                        : game_scripts_.failure());
    }
    for (std::size_t index = 0; index < catalog->items.size(); ++index) {
        const auto& item = catalog->items[index];
        if (item.code != index + 1U) {
            return fail("Lua item catalog changed source item order");
        }
        const auto* name = game_scripts_.dialogue_text(item.name_key);
        const auto* category = game_scripts_.dialogue_text(item.category_key);
        if (name == nullptr || category == nullptr) {
            return fail("Lua item name or category text is incomplete");
        }
        auto& presentation = field_menu_text_.items[index];
        presentation.definition = item;
        presentation.name = *name;
        presentation.category = *category;
        if (presentation.definition.weapon.has_value()) {
            const auto& profile =
                presentation.definition.weapon->power_profile;
            const auto* label = game_scripts_.dialogue_text(
                "items.display.option.power_" + profile);
            if (label == nullptr) {
                return fail("Lua weapon power profile has no display label");
            }
            presentation.definition.weapon->power_label = *label;
        }
    }
    for (std::size_t index = 0; index < catalog->detail_text_keys.size();
         ++index) {
        const auto* detail =
            game_scripts_.dialogue_text(catalog->detail_text_keys[index]);
        if (detail == nullptr) {
            return fail("Lua item detail text is incomplete");
        }
        field_menu_text_.field_action_detail_texts[index] = *detail;
    }
    for (std::size_t index = 0;
         index < catalog->detail_label_keys.size(); ++index) {
        const auto* label =
            game_scripts_.dialogue_text(catalog->detail_label_keys[index]);
        if (label == nullptr) {
            return fail("Lua item detail labels are incomplete");
        }
        field_menu_text_.item_detail_labels[index] = *label;
    }
    source_boundary("field.item.menu_content_loaded_from_lua");
    return true;
}

HotelMenuEvent FieldSceneRuntime::process_hotel_menu_events(
    const std::uint16_t events) {
    const auto result = hotel_menu_.advance_controller_events(events);
    switch (result) {
    case HotelMenuEvent::waiting:
        break;
    case HotelMenuEvent::status_acknowledged:
        source_boundary("field.hotel.entity_status.acknowledged");
        break;
    case HotelMenuEvent::selection_moved:
        // The Hotel loop and E660 path do not request a sound event.
        source_boundary("field.hotel.selection_moved");
        break;
    case HotelMenuEvent::checkout_opened:
        source_boundary("field.hotel.checkout.opened");
        break;
    case HotelMenuEvent::stayed_at_hotel:
        source_boundary("field.hotel.checkout.stayed");
        break;
    case HotelMenuEvent::checked_out:
        {
            scripting::DialogueOutcome outcome =
                scripting::ReturnToOriginOutcome{"source_entry", std::nullopt};
            (void)dispatch_dialogue_outcome(outcome);
        }
        refresh_field_hud_runner_slot();
        source_boundary("field.hotel.checked_out");
        break;
    case HotelMenuEvent::karma_screen_opened:
        // ROM 0xD82A rules come from Lua karma.lua; C++ keeps the grid
        // layout (cell -> record offset) and cross-checks Lua's cell.
        hotel_karma_skills_.set_upgrade_rule(
            [this](const RunnerRecordFields& record,
                   const AttributeSkillCursor cursor) {
                auto preview = preview_attribute_skill_upgrade(record, cursor);
                if (preview.status ==
                    AttributeSkillUpgradeStatus::invalid_selection) {
                    return preview;
                }
                const bool skill =
                    preview.section == AttributeSkillSection::skill;
                const auto lua = game_scripts_.call_integers(
                    "karma", "upgrade_preview_flat",
                    {cursor.column, cursor.row, preview.current_value,
                     record.species_index, record.archetype_index,
                     record.skills[1U]});
                constexpr std::array kStatus{
                    AttributeSkillUpgradeStatus::available,
                    AttributeSkillUpgradeStatus::invalid_selection,
                    AttributeSkillUpgradeStatus::attribute_not_upgradeable,
                    AttributeSkillUpgradeStatus::unknown_species,
                    AttributeSkillUpgradeStatus::class_restricted,
                    AttributeSkillUpgradeStatus::at_limit};
                if (!lua.has_value() || lua->size() != 5U ||
                    (*lua)[0] < 0 ||
                    (*lua)[0] >= static_cast<std::int64_t>(kStatus.size()) ||
                    (*lua)[1] != (skill ? 1 : 0) ||
                    (*lua)[2] != preview.value_index ||
                    (*lua)[3] < 0 || (*lua)[3] > 0xFF ||
                    (*lua)[4] < 0 || (*lua)[4] > 0xFF) {
                    source_boundary("field.hotel.karma_rule_script_failed");
                    preview.status =
                        AttributeSkillUpgradeStatus::invalid_selection;
                    return preview;
                }
                preview.status =
                    kStatus[static_cast<std::size_t>((*lua)[0])];
                preview.next_value = static_cast<std::uint8_t>((*lua)[3]);
                preview.karma_cost = static_cast<std::uint16_t>((*lua)[4]);
                return preview;
            });
        hotel_karma_skills_.open(active_runner_record());
        hotel_karma_load_frames_ = hotel_karma_load_black_frames_;
        source_boundary("field.hotel.karma_screen.opened");
        break;
    case HotelMenuEvent::mode_action:
        // 0xCFF4 calls the field C-button routine 0x391A, whose first act
        // is sound event 0x25 via 0x530DE (emulator: +1 frame after C in
        // Motel Options) [C].
        if (source_audio_event_callback_) {
            source_audio_event_callback_(static_cast<std::uint16_t>(srx::MenuSound::MOVE));
        }
        handle_field_mode_action_event();
        source_boundary("field.hotel.mode_action");
        break;
    }
    return result;
}

#ifdef SHADOWRUN_QT_QA_DRIVER
HotelMenuEvent FieldSceneRuntime::advance_hotel_menu_for_dump(
    const std::uint16_t events) {
    if (!hotel_menu_.open()) {
        return HotelMenuEvent::waiting;
    }
    if (hotel_karma_skills_.is_open()) {
        if (hotel_karma_load_frames_ > 0U) {
            --hotel_karma_load_frames_;
            return HotelMenuEvent::waiting;
        }
        (void)process_hotel_karma_screen_events(events);
        return HotelMenuEvent::waiting;
    }
    hotel_menu_.advance_animation(field_random_seed_);
    return process_hotel_menu_events(events);
}
#endif // SHADOWRUN_QT_QA_DRIVER


#ifdef SHADOWRUN_QT_QA_DRIVER
AttributesSkillsRuntimeEvent
FieldSceneRuntime::advance_hotel_karma_screen_for_dump(
    const std::uint16_t events) {
    if (!hotel_menu_.open() || !hotel_karma_skills_.is_open()) {
        return AttributesSkillsRuntimeEvent::no_change;
    }
    return process_hotel_karma_screen_events(events);
}
#endif // SHADOWRUN_QT_QA_DRIVER


AttributesSkillsRuntimeEvent
FieldSceneRuntime::process_hotel_karma_screen_events(
    const std::uint16_t events) {
    const auto result = hotel_karma_skills_.advance(
        events, active_runner_record(), [this] {
            handle_field_mode_action_event();
            refresh_field_hud_runner_slot();
            source_boundary("field.hotel.karma_screen.runner_selected");
        });
    switch (result) {
    case AttributesSkillsRuntimeEvent::no_change:
        break;
    case AttributesSkillsRuntimeEvent::selection_moved:
        source_boundary("field.hotel.karma_screen.selection_moved");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_prompt_opened:
        source_boundary("field.hotel.karma_screen.upgrade_prompt_opened");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_prompt_moved:
        source_boundary("field.hotel.karma_screen.upgrade_prompt_moved");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_applied:
        source_boundary("field.hotel.karma_screen.upgrade_applied");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_declined:
        source_boundary("field.hotel.karma_screen.upgrade_declined");
        break;
    case AttributesSkillsRuntimeEvent::upgrade_blocked:
        source_boundary("field.hotel.karma_screen.upgrade_blocked");
        break;
    case AttributesSkillsRuntimeEvent::insufficient_karma:
        source_boundary("field.hotel.karma_screen.insufficient_karma");
        break;
    case AttributesSkillsRuntimeEvent::
        active_runner_selection_requested:
        hotel_karma_skills_.refresh(active_runner_record());
        break;
    case AttributesSkillsRuntimeEvent::returned_to_parent:
        hotel_karma_skills_.close_to_parent();
        source_boundary("field.hotel.karma_screen.returned_to_options");
        break;
    }
    return result;
}

#ifdef SHADOWRUN_QT_QA_DRIVER
bool FieldSceneRuntime::begin_ares_shop_event_for_dump() {
    return begin_field_mode_service_event_for_dump(2U, 2U, 11U, 0);
}

bool FieldSceneRuntime::begin_field_mode_service_event_for_dump(
    const std::uint8_t scene_id, const std::uint8_t field_mode,
    const std::uint8_t event_code,
    const std::int8_t alternate_result) {
    if (!ready_) {
        return false;
    }
    // Inject a source-resolved event tuple into the same converted
    // orchestrator used by field movement. This test seam does not skip Lua
    // entity routing, the generic prompt, or the 0x1FA78 service selector.
    field_runtime_state_.scene_id = scene_id;
    field_runtime_state_.field_mode = field_mode;
    field_movement_transition_state_.mode_event_code = event_code;
    field_exit_alternate_result_ = alternate_result;
    field_shop_route_pending_service_id_.clear();
    field_shop_notice_active_ = false;
    field_shop_notice_service_id_.clear();
    field_shop_notice_text_key_.clear();
    field_mode_text_.close();
    (void)run_field_exit_orchestrator(field_runtime_state_);
    return field_mode_text_.open();
}
#endif // SHADOWRUN_QT_QA_DRIVER


bool FieldSceneRuntime::seek_command_overview_page(const std::uint8_t page) {
    if (!command_overview_.open() || page > 6U) {
        return false;
    }
    // The converted primary-page advance wraps the seven-page selector, so
    // a d-pad left edge moves one page toward zero. Port byte bit 2 is the
    // left direction in the sampled controller word (0x0400 in the expanded
    // events consumed by the bridge).
    const auto current = command_overview_.view().source_state.primary_page;
    const auto steps = (current + 7U - page) % 7U;
    for (std::uint8_t step = 0; step < steps; ++step) {
        set_controller_port_byte(0x04U);
        advance_frame();
        set_controller_port_byte(0U);
        advance_frame();
    }
    return command_overview_.view().source_state.primary_page == page;
}


} // namespace shadowrun::ghidra_conversion::qt_runtime
