#pragma once

#include "shadowrun_ghidra/core/controller_wait_conversion.hpp"
#include "shadowrun_ghidra/menus/pocket_secretary_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/magic_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/first_aid_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_state_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_camera_conversion.hpp"
#include "shadowrun_ghidra/field/field_hud_conversion.hpp"
#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"
#include "shadowrun_ghidra/field/field_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_service_target_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_transition_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_projection_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_route_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_step_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_mode_transition_conversion.hpp"
#include "shadowrun_ghidra/field/terrain_probe_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_exit_transition_finalizer_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_script_route_conversion.hpp"
#include "shadowrun_ghidra/field/field_runner_state_asset_refresh_conversion.hpp"
#include "shadowrun_ghidra/integration/field_runtime_coordinator_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_bootstrap_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"
#include "shadowrun_ghidra/menus/cyberware_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/inventory_action_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_confirmation_conversion.hpp"
#include "shadowrun_ghidra/session/runner_switch_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_operations_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"
#include "shadowrun_ghidra/world/world_entity_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_edge_conversion.hpp"
#include "shadowrun_ghidra/world/world_frame_logic_conversion.hpp"
#include "shadowrun_ghidra/world/world_scene_entity_conversion.hpp"
#include "shadowrun_ghidra/world/world_activity_routine_conversion.hpp"

#include <QImage>

#include "attributes_skills_menu_runtime.hpp"
#include "field_command_overview_bridge.hpp"
#include "field_clips_menu_runtime.hpp"
#include "field_party_roster_runtime.hpp"
#include "field_dialogue_runtime.hpp"
#include "hotel_menu_runtime.hpp"
#include "field_mode_text_runtime.hpp"
#include "field_interaction_runtime.hpp"
#include "elevator_selector_runtime.hpp"
#include "field_menu_text_catalog.hpp"
#include "shop_menu_runtime.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class FieldWorldFactFlag : std::uint8_t {
    errascoe_question_available = 1,
    run_objective_active = 9,
    redmond_vigore_search_complete = 22,
    redmond_vigore_lead_available = 26,
};

enum class FieldTitleServiceFlag : std::uint8_t {
    redmond_search_cleanup_09 = 9,
    redmond_vigore_search_consumed = 10,
    redmond_search_cleanup_12 = 12,
    redmond_vigore_search_pending = 13,
    redmond_search_cleanup_15 = 15,
    mode_command_e7_clear_15 = 15,
    mode_command_e7_clear_21 = 21,
};

enum class FieldDialogueRegisterFlag : std::uint8_t {
    wanderer_vigore_jarl_intro_seen = 6,
};

enum class FieldRenderEffectFlag : std::uint8_t {
    redmond_search_cleanup_09 = 9,
    redmond_search_cleanup_14 = 14,
    redmond_search_cleanup_19 = 19,
    redmond_search_cleanup_29 = 29,
};

enum class GroupItemFlag : std::uint8_t {
    landmark_map = 7,
};

struct FieldTargetMarkerDebugState {
    bool visible = false;
    bool animating = false;
    std::uint8_t style = 0;
    std::uint8_t radius = 0;
    std::uint16_t entity_kind = 0;
    std::uint16_t projected_x = 0;
    std::uint16_t projected_y = 0;
};

struct FieldCreatureAttackEffectDebugState {
    bool visible = false;
    std::uint8_t direction = 0;
    std::uint8_t phase = 0;
    std::optional<std::uint8_t> effect_slot;
    std::int16_t action_delay = 0;
};

constexpr std::uint32_t field_flag_mask(
    const FieldWorldFactFlag flag) noexcept {
    return std::uint32_t{1U} << static_cast<std::uint8_t>(flag);
}

constexpr std::uint32_t field_flag_mask(
    const FieldTitleServiceFlag flag) noexcept {
    return std::uint32_t{1U} << static_cast<std::uint8_t>(flag);
}

constexpr std::uint8_t field_flag_mask(
    const FieldDialogueRegisterFlag flag) noexcept {
    return static_cast<std::uint8_t>(
        std::uint8_t{1U} << static_cast<std::uint8_t>(flag));
}

constexpr std::uint32_t field_flag_mask(
    const FieldRenderEffectFlag flag) noexcept {
    return std::uint32_t{1U} << static_cast<std::uint8_t>(flag);
}

constexpr std::uint32_t group_item_flag_mask(
    const GroupItemFlag flag) noexcept {
    return std::uint32_t{1U} << static_cast<std::uint8_t>(flag);
}

enum class CommandOverviewMagicState {
    inactive,
    magical_defense_wait,
    spell_panel,
    spell_force_selection,
    spell_posture_selection,
};

enum class CommandOverviewFirstAidState {
    inactive,
    target_selection,
    result_wait,
};

enum class CommandOverviewAutomaticUseState {
    inactive,
    result_wait,
};

enum class CommandOverviewActionMessage {
    none,
    cannot_discard,
    cannot_trade,
};

// ROM 0x1A342 allocates ordinary world actors from seven live entity records.
// Keep these host identities typed so map population does not depend on RAM
// address arithmetic.
enum class FieldWorldActorSlot : std::uint8_t {
    slot_3 = 3,
    slot_4 = 4,
    slot_5 = 5,
    slot_6 = 6,
    slot_7 = 7,
    slot_8 = 8,
    slot_9 = 9,
};

struct MagicDefenseStripView {
    std::uint16_t value = 0;
    std::uint8_t bar_row = 0;
};

struct FieldOpeningAutoWalkState {
    bool active = false;
    std::uint16_t target_heading = 0;
};

struct PocketSecretaryMenuContent {
    struct Choice {
        std::string id;
        std::string label;
    };

    std::string title;
    std::string separator;
    std::array<Choice, kPocketSecretaryFieldMenuRowCount> choices{};
    std::array<std::string, kNotebookChoiceCount> notebook_choice_ids{};
};

// Composition-only adapter for the separate Ghidra-conversion Qt executable.
// It owns no ROM view and does not duplicate converted field state. The
// conversion bootstrap remains authoritative; this class supplies host
// presentation callbacks and paints the direct source assets it receives.
class FieldSceneRuntime final {
public:
    explicit FieldSceneRuntime(
        std::function<void(std::string_view)> source_boundary,
        std::function<void(std::uint16_t)> source_audio_event = {});

    bool bootstrap(
        const integration::StartupIntegrationState& startup_state);
    bool bootstrap_for_scene_dump(
        const integration::StartupIntegrationState& startup_state,
        std::uint16_t scene_reference);
    void advance_frame();
    void set_controller_nibble(std::uint8_t dpad_nibble) noexcept;
    void set_controller_port_byte(std::uint8_t port_byte) noexcept;
    void paint(QPainter& painter) const;
    void paint_field_scene_for_dump(QPainter& painter) const;
    std::size_t default_actor_count_for_dump() const noexcept;
    std::size_t visible_default_actor_count_for_dump() const;
    std::vector<SceneRunnerPosition>
    active_default_actor_positions_for_dump() const;
    std::optional<std::string>
    selected_default_actor_id_for_dump() const;
    std::optional<FieldTargetMarkerDebugState>
    selected_default_actor_target_marker_for_dump() const noexcept;
    std::optional<FieldCreatureAttackEffectDebugState>
    default_actor_creature_attack_effect_for_dump(
        std::uint16_t source_order) const noexcept;
    // Resolves injected selector rolls through Lua, then exercises the same
    // ROM-list validation, placement, Lua initialization, and renderer state
    // used by an eligible live random-encounter pass.
    std::size_t resolve_and_spawn_field_encounter_for_dump(
        std::uint8_t first_roll, std::uint8_t second_roll,
        std::uint16_t random_seed);
    std::size_t default_actor_transition_started_count_for_dump() const noexcept;
    std::size_t default_actor_transition_complete_count_for_dump() const noexcept;
    std::optional<std::uint8_t> default_actor_direction_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::uint8_t> default_actor_animation_frame_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::string> default_actor_animation_id_for_dump(
        std::uint16_t source_order) const;
    std::optional<FieldWorldActorSlot> default_actor_record_slot_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::string> default_actor_script_profile_for_dump(
        std::uint16_t source_order) const;
    std::optional<std::string> default_actor_ai_package_for_dump(
        std::uint16_t source_order) const;
    std::optional<std::string> default_actor_ai_target_id_for_dump(
        std::uint16_t source_order) const;
    std::optional<bool> default_actor_transition_complete_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<bool> default_actor_has_ai_target_pool_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::uint16_t> default_actor_ai_target_distance_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::uint8_t> default_actor_weapon_item_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::uint8_t> default_actor_physical_meter_for_dump(
        std::uint16_t source_order) const noexcept;
    // Seeds a live encounter actor for deterministic field QA only.
    bool set_default_actor_physical_meter_for_dump(
        std::uint16_t source_order, std::uint8_t physical_meter) noexcept;
    bool open_command_overview_for_dump();
    bool open_attributes_skills_for_dump();
    bool attributes_skills_menu_open() const noexcept {
        return command_overview_attributes_skills_.is_open();
    }
    const CommandMenuKarmaViewRuntime& attributes_skills_menu() const noexcept {
        return command_overview_attributes_skills_;
    }

    bool command_overview_open() const noexcept {
        return command_overview_.open();
    }
    std::uint8_t command_overview_primary_page() const noexcept {
        return command_overview_.view().source_state.primary_page;
    }
    bool dialogue_open() const noexcept { return field_dialogue_.open(); }
    const DialoguePanelView& dialogue_panel() const noexcept {
        return field_dialogue_.panel();
    }
    bool open_stoker_dialogue_for_dump(
        StokerMotelPromptId prompt) {
        // Script state reads this typed field state directly in a live
        // session, so seed the native owner for deterministic diagnostic
        // prompts instead of writing only GameScriptRuntime's fallback map.
        if (prompt == StokerMotelPromptId::manager_greeting_with_package) {
            stoker_motel_story_state_ = {true, false, false};
        } else if (prompt == StokerMotelPromptId::manager_greeting) {
            stoker_motel_story_state_ = {true, true, true};
        }
        return field_dialogue_.open_stoker_prompt_for_dump(prompt);
    }
    bool open_gunderson_interaction_for_dump();
    bool open_stoker_arrival_result_for_dump(std::uint8_t random_roll) {
        return field_dialogue_.open_stoker_arrival_result_for_dump(
            random_roll);
    }
    bool open_stoker_room_payment_for_dump(std::uint32_t funds);
    FieldDialogueAdvanceResult advance_stoker_dialogue_for_dump(
        std::uint16_t event_word);
    bool hotel_menu_open() const noexcept { return hotel_menu_.open(); }
    bool hotel_karma_screen_open() const noexcept {
        return hotel_karma_skills_.is_open();
    }
    const HotelKarmaUpgradeRuntime& hotel_karma_screen() const noexcept {
        return hotel_karma_skills_;
    }
    HotelMenuPhase hotel_menu_phase() const noexcept {
        return hotel_menu_.phase();
    }
    std::uint8_t hotel_option_selection() const noexcept {
        return static_cast<std::uint8_t>(hotel_menu_.option_selection());
    }
    std::uint8_t hotel_checkout_selection() const noexcept {
        return static_cast<std::uint8_t>(hotel_menu_.checkout_selection());
    }
    std::uint32_t hotel_animation_frames() const noexcept {
        return hotel_menu_.animation_frames();
    }
    // Deterministic source-PRNG injection for repeatable CLI screenshot QA.
    // The live field runtime still initializes its own seed.
    void set_source_random_seed_for_dump(std::uint16_t seed) noexcept {
        field_random_seed_ = seed;
    }
    HotelMenuEvent advance_hotel_menu_for_dump(std::uint16_t events);
    AttributesSkillsRuntimeEvent advance_hotel_karma_screen_for_dump(
        std::uint16_t events);
    std::optional<std::uint32_t> notebook_story_flags_for_dump() const noexcept {
        return notebook_tip_flags_resolved_
                   ? std::optional<std::uint32_t>{notebook_tip_flags_[0]}
                   : std::nullopt;
    }
    std::optional<std::uint8_t> notebook_referral_flags_for_dump() const noexcept {
        return notebook_referral_flags_;
    }
    std::optional<std::array<bool, 3>> notebook_event_gates() const noexcept {
        if (!notebook_contacts_flags_.has_value() ||
            !notebook_referral_flags_.has_value() ||
            !notebook_shadowrunner_flags_.has_value()) {
            return std::nullopt;
        }
        return std::array<bool, 3>{
            *notebook_contacts_flags_ != 0U,
            *notebook_referral_flags_ != 0U,
            *notebook_shadowrunner_flags_ != 0U};
    }
    bool open_ares_mode_text_for_dump(std::int8_t alternate_result) {
        return field_mode_text_.open(
            scripting::ScriptFieldModeContext{2U, 11U, alternate_result});
    }
    bool field_mode_text_open_for_dump() const noexcept {
        return field_mode_text_.open();
    }
    const DialoguePanelView& field_mode_text_panel_for_dump() const noexcept {
        return field_mode_text_.panel();
    }
    std::uint16_t field_mode_text_frames_remaining_for_dump() const noexcept {
        return field_mode_text_.frames_remaining();
    }
    bool field_interaction_open() const noexcept {
        return field_interaction_.open();
    }
    bool elevator_selector_open_for_dump() const noexcept {
        return field_elevator_selector_.is_open();
    }
    bool elevator_selector_animating_for_dump() const noexcept {
        return field_elevator_selector_.is_animating();
    }
    std::uint8_t elevator_selector_current_floor_for_dump() const noexcept {
        return field_elevator_selector_.current_floor();
    }
    std::uint8_t elevator_selector_selected_floor_for_dump() const noexcept {
        return field_elevator_selector_.selected_floor();
    }
    std::uint8_t elevator_selector_animation_frame_for_dump() const noexcept {
        return field_elevator_selector_.animation_frame();
    }
    ElevatorSelectorNavigationFeedback
    elevator_selector_navigation_feedback_for_dump() const noexcept {
        return field_elevator_selector_.navigation_feedback();
    }
    std::uint8_t
    elevator_selector_navigation_feedback_frames_remaining_for_dump()
        const noexcept {
        return field_elevator_selector_.navigation_feedback_frames_remaining();
    }
    std::uint16_t elevator_selector_destination_for_dump() const noexcept {
        return field_elevator_script_destination_.value_or(0U);
    }
    ElevatorSceneTransitionPhase
    elevator_scene_transition_phase_for_dump() const noexcept {
        return field_elevator_scene_transition_.phase();
    }
    std::uint8_t elevator_scene_transition_palette_step_for_dump() const noexcept {
        return field_elevator_scene_transition_.palette_step();
    }
    std::uint8_t
    elevator_scene_transition_vblanks_for_dump() const noexcept {
        return field_elevator_scene_transition_.vblanks_in_palette_step();
    }
    std::uint8_t elevator_scene_transition_fade_alpha_for_dump() const noexcept {
        return field_elevator_scene_transition_.black_overlay_alpha();
    }
    std::uint8_t field_entity_behavior_flags_for_dump() const noexcept {
        return field_entity_behavior_flags_;
    }
    const DialoguePanelView& field_interaction_panel() const noexcept {
        return field_interaction_.panel();
    }
    const WorldActivityTimeoutState& world_activity_timeout_for_dump()
        const noexcept {
        return field_world_activity_timeout_state_;
    }
    const WorldFrameLogicState& world_frame_logic_for_dump() const noexcept {
        return world_frame_logic_state_;
    }
    std::uint8_t world_activity_request_timeout_for_dump() const noexcept {
        return field_world_activity_state_.timeout;
    }
    bool dispatch_field_interaction_through_coordinator_for_dump(
        FieldInteractionPrompt prompt, std::uint8_t interaction_selection = 2U,
        bool interaction_suppressed = false,
        std::optional<std::uint8_t> security_floor_index = std::nullopt);
    bool initialize_world_activity_for_dump(std::uint8_t mode_event_code,
                                            std::uint16_t random_seed);
    bool return_from_world_activity_for_dump();
    bool dispatch_selected_security_terminal_for_dump();
    std::uint16_t field_scene_reference_for_dump() const noexcept {
        return state_.field_scene_buffers.scene_reference;
    }
    const WorldActivityRoutineState& world_activity_state_for_dump() const
        noexcept {
        return world_activity_routine_state_;
    }
    std::optional<std::uint8_t> security_floor_index_for_dump() const noexcept {
        return field_security_floor_index_;
    }
    void begin_projected_field_transition_for_dump() noexcept;
    bool field_interaction_transition_active_for_dump() const noexcept {
        return field_movement_transition_state_.interaction_transition_pending;
    }
    bool field_transition_pending_for_dump() const noexcept {
        return field_runtime_state_.field_transition_requested;
    }
    std::uint8_t field_mode_event_code_for_dump() const noexcept {
        return field_movement_transition_state_.mode_event_code;
    }
    std::uint16_t field_interaction_mode_for_dump() const noexcept {
        return field_movement_transition_state_.world_mode;
    }
    // Scene-entry QA checkpoint: 0x03074 selects the interaction mode for
    // the loaded field before ordinary movement resumes. Keep this setter on
    // the dump/test surface rather than exposing a second gameplay control.
    void set_field_interaction_mode_for_dump(std::uint16_t mode) noexcept {
        field_movement_transition_state_.world_mode = mode;
    }
    std::uint8_t field_activity_selection_for_dump() const noexcept {
        return field_movement_transition_state_.activity_selection;
    }
    const FieldMapEventSystem& field_map_event_system_for_dump() const noexcept {
        return field_map_event_system_;
    }
    const StokerMotelStoryState& stoker_motel_story_state_for_dump() const
        noexcept {
        return stoker_motel_story_state_;
    }
    std::uint16_t field_terrain_collision_code_for_dump() const noexcept {
        return field_movement_transition_state_.terrain_collision_code;
    }
    const std::array<std::int8_t, 4>&
    field_directional_probe_signs_for_dump() const noexcept {
        return field_movement_transition_state_.directional_probe_signs;
    }
    const std::array<std::uint8_t, 4>&
    field_terrain_sample_bytes_for_dump() const noexcept {
        return field_terrain_sample_state_;
    }
    std::uint16_t field_projected_code_for_dump() const noexcept {
        return field_movement_behavior_record_.projected_field_code;
    }
    const integration::FieldRuntimeCoordinatorState& field_runtime_state_for_dump()
        const noexcept {
        return field_runtime_state_;
    }
    std::int16_t field_interaction_transition_timer_for_dump() const noexcept {
        return field_movement_transition_state_.interaction_transition_timer;
    }
    std::uint8_t field_palette_fade_steps_for_dump() const noexcept {
        return field_palette_fade_steps_;
    }
    std::uint8_t field_modal_return_transition_frames_for_dump() const noexcept {
        return field_modal_return_transition_frames_remaining_;
    }
    std::uint8_t field_modal_return_blocked_inputs_for_dump() const noexcept {
        return field_modal_return_blocked_inputs_;
    }
    FieldInteractionAdvanceResult advance_field_interaction_for_dump(
        std::uint16_t event_word) {
        return advance_field_interaction(event_word);
    }
    bool begin_ares_shop_event_for_dump();
    bool shop_menu_open() const noexcept { return field_shop_menu_.is_open(); }
    ShopActionGroup shop_menu_group() const noexcept {
        return field_shop_menu_.selected_group();
    }
    std::uint8_t shop_menu_cursor() const noexcept {
        return field_shop_menu_.selected_cursor();
    }
    bool shop_menu_offers_active() const noexcept {
        return field_shop_menu_.offers_panel_active();
    }
    std::optional<std::uint8_t>
    shop_selected_weapon_capacity_for_dump() const {
        return selected_shop_weapon_capacity();
    }
    std::optional<std::uint8_t> shop_selected_item_uses_for_dump() const {
        return selected_shop_item_uses();
    }
    bool shop_transaction_confirmation_open() const noexcept {
        return field_shop_menu_.transaction_confirmation_open();
    }
    bool shop_transaction_waiting_for_event() const noexcept {
        return field_shop_menu_.transaction_waiting_for_event();
    }
    const MenuConfirmationState& shop_transaction_confirmation() const noexcept {
        return field_shop_menu_.transaction_confirmation();
    }
    ShopTransactionMessage shop_transaction_message() const noexcept {
        return field_shop_menu_.transaction_message();
    }
    FieldActionApplicationResult shop_last_purchase_result() const noexcept {
        return field_shop_menu_.last_purchase_result();
    }
    const ShopSaleOutcome& shop_last_sale_outcome() const noexcept {
        return field_shop_menu_.last_sale_outcome();
    }
    std::optional<std::uint32_t> field_funds_for_dump() const noexcept {
        return field_funds_;
    }
    void set_field_funds_for_dump(const std::uint32_t funds) noexcept {
        field_funds_ = funds;
    }
    const FieldCommandOverviewView& command_overview_view() const noexcept {
        return command_overview_.view();
    }
    RunnerRecordFields& active_runner_record() noexcept {
        return field_party_roster_.active_record();
    }
    void install_party_member_for_dump(std::size_t slot,
                                       RunnerRecordFields record,
                                       std::string name,
                                       std::uint8_t portrait_asset_family =
                                           0U) {
        field_party_roster_.install_party_member_for_dump(
            slot, std::move(record), std::move(name),
            portrait_asset_family);
        refresh_field_hud_runner_slot();
    }
    void set_active_party_slot_for_dump(std::size_t slot) {
        field_party_roster_.set_active_slot(slot);
        field_entity_selection_state_.active_index =
            field_party_roster_.active_slot();
        field_entity_behavior_flags_ = static_cast<std::uint8_t>(
            active_runner_record().flags >> 8U);
        field_movement_behavior_record_.behavior_flags =
            field_entity_behavior_flags_;
        refresh_field_hud_runner_slot();
    }
    bool field_party_slot_active_for_dump(std::size_t slot) const noexcept {
        return field_party_roster_.slot_active(slot);
    }
    std::size_t field_active_party_slot_for_dump() const noexcept {
        return field_party_roster_.active_slot();
    }
    const RunnerRecordFields& active_runner_record() const noexcept {
        return field_party_roster_.active_record();
    }
    void refresh_field_hud_for_dump() {
        refresh_field_hud_runner_slot();
    }
    FieldHudSlotState field_hud_slot_state_for_dump(
        const std::size_t slot) const {
        return field_hud_slot_state(slot);
    }
    const std::array<RunnerRecordFields,
                     FieldPartyRosterRuntime::kSlotCount>&
    field_party_records_for_dump() const noexcept {
        return field_party_roster_.records();
    }
    const EquipmentCombatBarResult& command_overview_combat_bars() const
        noexcept {
        return command_overview_combat_bars_;
    }
    bool inventory_action_open() const noexcept {
        return command_overview_inventory_action_active_;
    }
    std::uint8_t inventory_action_selection() const noexcept {
        return command_overview_inventory_action_.action_selector;
    }
    bool first_aid_open() const noexcept {
        return command_overview_first_aid_state_ !=
               CommandOverviewFirstAidState::inactive;
    }
    bool first_aid_result_waiting() const noexcept {
        return command_overview_first_aid_state_ ==
               CommandOverviewFirstAidState::result_wait;
    }
    std::size_t first_aid_target_cursor() const noexcept {
        return command_overview_first_aid_cursor_;
    }
    const FirstAidTargetList& first_aid_targets() const noexcept {
        return command_overview_first_aid_targets_;
    }
    FirstAidMessage first_aid_message() const noexcept {
        return command_overview_first_aid_message_;
    }
    bool automatic_use_result_waiting() const noexcept {
        return command_overview_automatic_use_state_ ==
               CommandOverviewAutomaticUseState::result_wait;
    }
    bool discard_confirmation_open() const noexcept {
        return command_overview_discard_confirmation_active_;
    }
    bool discard_confirmation_yes() const noexcept {
        return command_overview_discard_confirmation_.selected_option == 0U;
    }
    bool inventory_trade_open() const noexcept {
        return command_overview_inventory_trade_.open;
    }
    std::size_t inventory_trade_recipient() const noexcept {
        return command_overview_inventory_trade_.recipient_slot;
    }
    std::size_t inventory_trade_recipient_inventory_slot() const noexcept {
        return command_overview_inventory_trade_.recipient_inventory_slot;
    }
    bool inventory_trade_recipient_full() const noexcept {
        return command_overview_inventory_trade_.recipient_inventory_full;
    }
    bool inventory_trade_yes_selected() const noexcept {
        return command_overview_inventory_trade_.yes_selected;
    }
    CommandOverviewActionMessage inventory_action_message() const noexcept {
        return command_overview_action_message_;
    }
    bool clips_menu_open() const noexcept {
        return command_overview_clips_.is_open();
    }
    bool cyberware_menu_open() const noexcept {
        return command_overview_cyberware_active_;
    }
    bool magic_defense_open() const noexcept {
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::magical_defense_wait;
    }
    bool magic_spell_panel_open() const noexcept {
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_panel;
    }
    bool magic_force_selection_open() const noexcept {
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_force_selection;
    }
    bool magic_posture_selection_open() const noexcept {
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_posture_selection;
    }
    bool magic_spell_interface_open() const noexcept {
        return magic_spell_panel_open() || magic_force_selection_open() ||
               magic_posture_selection_open();
    }
    std::uint16_t magic_current_force() const noexcept {
        return command_overview_magic_force_.current_force;
    }
    std::uint16_t magic_maximum_force() const noexcept {
        return command_overview_magic_force_.maximum_force;
    }
    MagicMenuSelection magic_spell_selection() const noexcept {
        return command_overview_magic_loop_.selection;
    }
    const std::optional<MagicSpellSelection>& magic_selected_spell() const
        noexcept {
        return command_overview_magic_selected_spell_;
    }
    std::uint8_t magic_cursor_phase() const noexcept {
        return command_overview_magic_cursor_phase_;
    }
    const MagicDefenseStripView& magic_physical_defense_view() const noexcept {
        return command_overview_magic_physical_defense_;
    }
    const MagicDefenseStripView& magic_mana_defense_view() const noexcept {
        return command_overview_magic_mana_defense_;
    }
    const CyberwareMenuModel& cyberware_menu_model() const noexcept {
        return command_overview_cyberware_;
    }
    std::uint8_t clips_menu_selection() const noexcept {
        return command_overview_clips_.selection();
    }
    bool clips_reload_confirmation_open() const noexcept {
        return command_overview_clips_.reload_confirmation_open();
    }
    bool clips_reload_confirmation_yes() const noexcept {
        return command_overview_clips_.reload_confirmation_yes();
    }
    ClipsMenuMessage clips_menu_message() const noexcept {
        return command_overview_clips_.message();
    }

    bool ready() const noexcept { return ready_; }
    // Navigate the converted command loop to a specific primary page so QA
    // dumps can exercise pages other than the entry selector (6).
    bool seek_command_overview_page(std::uint8_t page);
    bool pocket_secretary_open() const noexcept {
        return pocket_secretary_.open;
    }
    bool pocket_secretary_cyberdeck_unavailable_waiting() const noexcept {
        return pocket_secretary_.cyberdeck_unavailable_wait;
    }
    bool pocket_secretary_group_items_waiting() const noexcept {
        return pocket_secretary_.group_items_wait;
    }
    bool pocket_secretary_notebook_open() const noexcept {
        return pocket_secretary_.notebook.open;
    }
    bool pocket_secretary_notebook_no_run_waiting() const noexcept {
        return pocket_secretary_.notebook.no_current_run_wait;
    }
    std::uint8_t pocket_secretary_notebook_selection() const noexcept {
        return pocket_secretary_.notebook.selection;
    }
    std::uint8_t pocket_secretary_notebook_option_count() const noexcept {
        return pocket_secretary_.notebook.option_count;
    }
    bool pocket_secretary_notebook_tips_open() const noexcept {
        return pocket_secretary_.notebook.tips.open;
    }
    std::uint8_t pocket_secretary_notebook_tips_selection() const noexcept {
        return pocket_secretary_.notebook.tips.selection;
    }
    std::uint8_t pocket_secretary_notebook_visible_tip_count() const noexcept {
        return pocket_secretary_.notebook.tips.visible_count;
    }
    std::uint8_t pocket_secretary_notebook_visible_tip_source_index(
        std::size_t page) const noexcept {
        const auto& tips = pocket_secretary_.notebook.tips;
        return page < tips.visible_count
                   ? tips.visible_source_indices[page]
                   : 0xFFU;
    }
    bool pocket_secretary_notebook_mr_johnsons_open() const noexcept {
        return pocket_secretary_.notebook.mr_johnsons.open;
    }
    bool pocket_secretary_notebook_mr_johnsons_detail_waiting() const noexcept {
        return pocket_secretary_.notebook.mr_johnsons.detail_wait;
    }
    std::uint8_t pocket_secretary_notebook_mr_johnsons_selection() const noexcept {
        return pocket_secretary_.notebook.mr_johnsons.selection;
    }
    std::uint8_t pocket_secretary_notebook_mr_johnsons_visible_count() const noexcept {
        return pocket_secretary_.notebook.mr_johnsons.visible_count;
    }
    std::uint8_t pocket_secretary_notebook_mr_johnsons_source_index(
        std::size_t entry) const noexcept {
        const auto& page = pocket_secretary_.notebook.mr_johnsons;
        return entry < page.visible_count
                   ? page.visible_source_indices[entry]
                   : 0xFFU;
    }
    bool notebook_tip_flags_resolved() const noexcept {
        return notebook_tip_flags_resolved_;
    }
    // Story event writers use the same typed source bit order as F9A4. This
    // adapter currently seeds fresh New Game; save restoration must supply
    // the full three-longword state before mutating individual tips.
    bool set_notebook_tip_unlocked(std::size_t source_index,
                                   bool unlocked) noexcept;
    // Diagnostic route coverage uses the same five FFFC27 source bits as
    // 0xF416. Loaded-save flags stay unresolved until restoration is modeled.
    bool set_notebook_referral_flags_for_dump(std::uint8_t flags) noexcept {
        if (!notebook_referral_flags_.has_value()) {
            return false;
        }
        notebook_referral_flags_ = static_cast<std::uint8_t>(flags & 0x1FU);
        return true;
    }
    // QA seam for the source FFFC22 catalog branch. Normal play obtains this
    // word from quest-item event handling, which is not connected yet.
    void set_group_item_flags_for_dump(std::uint32_t flags) noexcept {
        group_item_flags_ = flags & 0x7FFFU;
    }
    std::optional<std::uint32_t> group_item_flags_for_dump() const noexcept {
        return group_item_flags_;
    }
    void seed_vigore_search_for_dump() noexcept {
        field_mode_effect_flags_ |=
            field_flag_mask(FieldTitleServiceFlag::redmond_vigore_search_pending);
    }
    std::uint32_t field_mode_effect_flags_for_dump() const noexcept {
        return field_mode_effect_flags_;
    }
    std::uint32_t field_world_fact_flags_for_dump() const noexcept {
        return field_exit_world_flags_;
    }
    std::uint32_t field_render_effect_flags_for_dump() const noexcept {
        return field_render_effect_flags_;
    }
    std::uint8_t pocket_secretary_selection() const noexcept {
        return pocket_secretary_.selection;
    }
    bool pocket_secretary_dismiss_runner_open() const noexcept {
        return dismiss_runner_.open;
    }
    std::uint8_t dismiss_runner_selection() const noexcept {
        return dismiss_runner_.selection;
    }
    std::uint8_t dismiss_runner_option_count() const noexcept {
        return static_cast<std::uint8_t>(dismiss_runner_.candidate_count + 1U);
    }
    std::optional<std::size_t> dismiss_runner_preview_slot() const noexcept {
        if (!dismiss_runner_.open || dismiss_runner_.selection == 0U ||
            dismiss_runner_.selection > dismiss_runner_.candidate_count) {
            return std::nullopt;
        }
        return dismiss_runner_.candidate_slots[
            static_cast<std::size_t>(dismiss_runner_.selection - 1U)];
    }
    std::uint32_t dismiss_runner_effect_frames() const noexcept {
        return dismiss_runner_effect_frames_;
    }
    bool posture_dialog_open() const noexcept {
        return posture_dialog_active_;
    }
    std::uint8_t posture_dialog_value() const noexcept {
        return active_runner_record().posture;
    }
    bool field_entity_visible() const noexcept {
        return field_entity_.visible;
    }
    std::uint8_t field_controller_nibble_for_dump() const noexcept {
        return field_controller_nibble_;
    }
    bool opening_auto_walk_active_for_dump() const noexcept {
        return field_opening_auto_walk_.active;
    }
    std::uint16_t controller_event_word_for_dump() const noexcept {
        return field_controller_sample_state_.events;
    }
    const std::string& failure() const noexcept { return failure_; }
    const integration::FieldSessionBootstrapState& state() const noexcept {
        return state_;
    }
    const SceneRunnerPosition& field_entity_world_position() const noexcept {
        return field_entity_.world_position;
    }
    bool field_entity_active_for_dump() const noexcept {
        return field_entity_.active;
    }
    std::uint8_t controller_nibble_for_dump() const noexcept {
        return field_controller_nibble_;
    }
    void set_field_entity_world_position_for_dump(
        std::uint16_t x, std::uint16_t y) noexcept {
        field_entity_.world_position.x = x;
        field_entity_.world_position.y = y;
        field_movement_state_.position_x = x;
        field_movement_state_.position_y = y;
        field_entity_.projection.world_x = x;
        field_entity_.projection.world_y = y;
        field_entity_.visible = true;
    }
    bool set_default_actor_world_position_for_dump(
        std::size_t active_actor_index, std::uint16_t x,
        std::uint16_t y) noexcept;
    bool set_selected_default_actor_world_position_for_dump(
        std::uint16_t x, std::uint16_t y) noexcept;
    bool field_entity_visible_for_dump() const noexcept {
        return field_entity_.visible;
    }
    std::uint16_t field_entity_screen_x_for_dump() const noexcept {
        return field_entity_.projection.screen_x;
    }
    std::uint16_t field_entity_screen_y_for_dump() const noexcept {
        return field_entity_.projection.screen_y;
    }
    std::uint16_t field_entity_heading_for_dump() const noexcept {
        return field_movement_state_.heading;
    }
    std::uint8_t field_movement_route_flags_for_dump() const noexcept {
        return field_movement_behavior_record_.route_flags;
    }
    std::uint16_t field_entity_animation_heading_for_dump() const noexcept {
        return field_movement_state_.animation_heading;
    }
    std::uint16_t field_entity_target_heading_for_dump() const noexcept {
        return field_target_heading_;
    }
    const FieldCameraFollowState& field_camera_follow_for_dump() const noexcept {
        return field_camera_follow_;
    }
    std::uint16_t field_entity_animation_frame_for_dump() const noexcept {
        return field_entity_animation_.frame;
    }
    std::uint16_t field_entity_animation_selector_for_dump() const noexcept {
        return field_entity_animation_selector_;
    }
    std::uint8_t field_entity_direction_for_dump() const noexcept {
        return field_entity_.direction;
    }
    bool field_entity_walking_for_dump() const noexcept {
        return field_entity_walking_;
    }
    bool field_entity_attack_pose_for_dump() const noexcept {
        return field_entity_attack_pose_active_;
    }
    std::uint16_t field_entity_attack_selector_for_dump() const noexcept {
        return field_entity_animation_selector_;
    }
    std::size_t field_weapon_projectile_count_for_dump() const noexcept {
        std::size_t count = 0U;
        for (const auto& spawn : field_weapon_projectiles_) {
            count += spawn.secondary_active ? 1U : 0U;
        }
        return count;
    }
    std::size_t field_weapon_primary_count_for_dump() const noexcept;
    std::optional<SceneRunnerPosition>
    field_weapon_primary_position_for_dump() const noexcept;
    std::optional<SceneRunnerPosition>
    field_weapon_primary_spawn_position_for_dump() const noexcept;
    std::optional<EntityHeadingOffsets>
    field_weapon_primary_heading_offsets_for_dump() const noexcept;
    std::optional<std::uint8_t>
    field_weapon_primary_retry_count_for_dump() const noexcept;
    std::optional<std::uint16_t>
    field_weapon_projectile_selector_for_dump() const noexcept;
    std::optional<bool>
    field_weapon_projectile_list_finished_for_dump() const noexcept;
    std::optional<std::uint16_t>
    field_weapon_projectile_frame_for_dump() const noexcept;
    EntitySpawnCycleAction last_field_weapon_spawn_action_for_dump() const
        noexcept {
        return last_field_weapon_spawn_result_.action;
    }
    const EntityMovementModeTransitionResult&
    last_field_movement_transition_for_dump() const noexcept {
        return last_field_movement_transition_result_;
    }

private:
    struct DefaultFieldActorPresentation;
    struct DefaultActorSpriteView;
    FieldInteractionAdvanceResult advance_field_interaction(
        std::uint16_t event_word);
    void configure_callbacks();
    void configure_runtime_coordinator_callbacks();
    std::optional<scripting::ScriptActorBehaviorDecision>
    heartbeat_field_player(scripting::ScriptActorHeartbeatPhase phase);
    void advance_player_weapon_action_gate();
    void advance_field_movement();
    void advance_field_actor_animation();
    void advance_default_field_actor_animations();
    void advance_field_weapon_projectile_animations();
    void advance_field_weapon_projectiles();
    bool apply_script_combat_meter_update(
        const scripting::ScriptCombatMeterUpdate& update);
    void service_player_weapon_action(bool requested_by_lua);
    void initialize_field_entity_selection_state();
    void refresh_field_runner_state_and_assets();
    void handle_field_mode_action_event();
    void run_field_mode_exit_transition();
    bool run_field_exit_orchestrator(
        integration::FieldRuntimeCoordinatorState& runtime_state);
    bool open_gunderson_interaction();
    bool open_field_johnson_interaction();
    bool project_field_world_interaction();
    void update_source_field_camera();
    void service_pending_field_viewports();
    bool recover_blocked_field_movement(
        const MovementProbeResult& initial_probe);
    void advance_field_scene_transition_after_modal();
    void snapshot_world_activity_source_scene();
    bool restore_world_activity_source_scene(
        integration::FieldRuntimeCoordinatorState& runtime_state);
    bool resume_field_mode_entry_route();
    bool resume_jump_house_entry_transition();
    void begin_field_modal_return_transition();
    void handle_field_map_event_activations(
        const std::vector<FieldMapEventActivation>& activations);
    void handle_game_script_task_step(
        const scripting::ScriptTaskStep& step);
    bool prepare_runtime_field_scene(std::uint16_t scene_reference);
    void dispatch_field_map_scene_entry(std::uint16_t scene_id);
    std::optional<std::size_t> projected_terrain_cell() const;
    void record_field_activity_start(
        MovementModeTransitionContinuation continuation);
    std::int16_t evaluate_field_collision(std::uint16_t candidate_x,
                                          std::uint16_t candidate_y);
    std::int16_t evaluate_field_collision(
        std::uint16_t candidate_x,
        std::uint16_t candidate_y,
        EntityCollisionRecord& entity_collision_record,
        std::array<std::uint8_t, 4>& sample_state,
        std::uint8_t entity_behavior_flags,
        bool publish_movement_state,
        std::uint16_t* terrain_collision_code_output = nullptr);
    void source_boundary(std::string_view boundary) const;
    bool load_image(const ConversionAssetCatalog& catalog,
                    std::string_view alias,
                    QImage& destination);
    bool load_field_weapon_spawn_assets(
        const ConversionAssetCatalog& catalog);
    bool load_field_default_actor_assets(
        const ConversionAssetCatalog& catalog);
    bool load_dynamic_world_actor_artwork(
        const ConversionAssetCatalog& catalog);
    void initialize_default_field_actors(std::uint16_t scene_reference);
    void advance_default_field_actors();
    bool apply_scripted_field_entity_target_selection(
        const std::optional<std::string>& selected_actor_id);
    void trigger_field_world_effect_sequence();
    std::int16_t select_field_world_effect_sequence();
    void spawn_field_world_effect_sequence();
    void reset_field_entity_service_records();
    void refresh_world_entity_service_targets();
    DefaultActorSpriteView default_actor_sprite_view(
        const DefaultFieldActorPresentation& actor) const;
    const QImage* runner_preview_panel_for_record(
        const RunnerRecordFields& record) const;
    bool load_sprite_image(const ConversionAssetCatalog& catalog,
                           std::string_view alias,
                           QImage& destination,
                           int& origin_x,
                           int& origin_y);
    void paint_field_scene(QPainter& painter) const;
    void paint_field_scene_entities(QPainter& painter) const;
    void paint_field_hud_overlays(QPainter& painter) const;
    FieldHudSlotState field_hud_slot_state(std::size_t slot) const;
    void paint_command_overview_magic(QPainter& painter) const;
    void paint_attributes_skills_menu(
        QPainter& painter,
        const AttributesSkillsMenuController& menu) const;
    void paint_command_overview(QPainter& painter) const;
    void paint_command_overview_clips(QPainter& painter) const;
    void paint_field_shop(QPainter& painter) const;
    std::optional<std::uint8_t> selected_shop_item_code() const;
    std::optional<std::uint8_t> weapon_magazine_capacity_for_item(
        std::uint8_t item_code) const;
    const scripting::ScriptWeaponTemplate* weapon_template_for_item_code(
        std::uint8_t item_code) const;
    std::optional<std::uint8_t> selected_shop_weapon_capacity() const;
    std::optional<std::uint8_t> selected_shop_item_uses() const;
    FieldActionBaseCostLookup item_base_cost_lookup() const;
    FieldActionBaseCostLookup item_resale_base_lookup() const;
    void begin_hotel_menu();
    bool load_command_overview_menu_content();
    bool load_hotel_menu_content();
    bool load_pocket_secretary_menu_content();
    bool load_ares_shop_menu_content();
    bool load_item_menu_content();
    bool load_field_elevator_selector_assets(
        const ConversionAssetCatalog& catalog);
    bool open_field_elevator_selector(
        FieldCollisionEventCode event_code);
    void advance_field_elevator_selector();
    void advance_field_elevator_scene_transition();
    bool transition_to_elevator_destination(std::uint16_t scene_reference);
    HotelMenuEvent process_hotel_menu_events(std::uint16_t events);
    AttributesSkillsRuntimeEvent process_hotel_karma_screen_events(
        std::uint16_t events);
    void draw_command_overview_text(QPainter& painter, int x, int y,
                                    std::string_view text) const;
    void draw_field_menu_font_text(QPainter& painter, int x, int y,
                                   std::string_view text,
                                   const QImage& font) const;
    void draw_hotel_menu_text(QPainter& painter, int x, int y,
                              std::string_view text,
                              HotelMenuPaletteLine palette_line) const;
    void refresh_command_overview_combat_bars();
    void refresh_command_overview_item_detail();
    void refresh_field_hud_runner_slot();
    void initialize_field_party_roster(
        const integration::StartupIntegrationState& startup_state);
    void advance_command_overview_inventory_action(
        std::uint16_t controller_events);
    void begin_command_overview_first_aid();
    void advance_command_overview_first_aid(
        std::uint16_t controller_events);
    void finish_command_overview_first_aid();
    void advance_command_overview_automatic_use(
        std::uint16_t controller_events);
    void finish_command_overview_automatic_use();
    void advance_command_overview_clips(std::uint16_t controller_events);
    void begin_command_overview_inventory_action();
    void begin_command_overview_cyberware();
    void begin_command_overview_magic();
    void begin_command_overview_attributes_skills();
    void advance_command_overview_magic(std::uint16_t controller_events);
    void begin_command_overview_magic_spell_action();
    void finish_command_overview_magic_spell_action();
    void begin_command_overview_clips();
    void begin_pocket_secretary();
    void begin_pocket_secretary_dismiss_runner();
    void finish_pocket_secretary_dismiss_runner(std::size_t slot);
    void advance_pocket_secretary_dismiss_runner_effect();
    void dispatch_command_overview_inventory_action(
        const InventoryActionResult& result);
    void advance_command_overview_discard_confirmation(
        std::uint16_t controller_events);
    void advance_command_overview_inventory_trade(
        std::uint16_t controller_events);
    void open_command_overview();
    bool load_command_overview_assets(const ConversionAssetCatalog& catalog);
    bool load_field_hud_overlays(const ConversionAssetCatalog& catalog);

    struct FieldEntityPresentation {
        bool active = false;
        SceneRunnerPosition world_position{};
        EntityProjectionState projection{};
        std::uint8_t direction = 0;
        bool visible = false;
    };

    struct FieldWeaponProjectilePresentation {
        scripting::ScriptCombatantFacts shooter_facts{};
        std::size_t shooter_party_slot = 0U;
        SpawnEntityRecordState primary_record{};
        EntityCollisionRecord primary_collision_record{};
        EntityFixedPointPosition primary_fixed_position{};
        SceneRunnerPosition primary_spawn_position{};
        EntityAnimationState primary_animation{};
        bool primary_animation_script_initialized = false;
        bool primary_active = false;
        std::optional<std::uint16_t> primary_slot;
        SpawnEntityRecordState source_record{};
        EntityAnimationState animation{};
        bool secondary_animation_script_initialized = false;
        bool secondary_active = false;
        std::optional<std::uint16_t> secondary_slot;
    };

    struct DefaultFieldActorPresentation {
        bool active = false;
        std::uint16_t scene_reference = 0;
        std::uint16_t source_order = 0;
        std::optional<FieldWorldActorSlot> source_record_slot;
        std::uint16_t entity_kind = 0;
        std::string actor_instance_id;
        std::string script_profile_id;
        scripting::ScriptActorActionKind action_kind =
            scripting::ScriptActorActionKind::none;
        std::uint8_t initial_action_code = 0;
        SceneRunnerPosition world_position{};
        std::uint16_t initial_heading = 0;
        std::uint8_t palette_line = 0;
        std::uint8_t physical_meter = 0;
        std::uint8_t delay_primary_attribute = 0;
        std::uint8_t delay_secondary_attribute = 0;
        std::uint16_t delay_capability_flags = 0;
        std::string sprite_stem;
        std::uint32_t template_offset = 0;
        std::uint32_t position_offset = 0;
        std::uint32_t list_offset = 0;
        std::uint32_t selection_cell_offset = 0;
        std::uint8_t source_record_flags = 0;
        std::uint8_t source_interaction_flags = 0;
        std::uint8_t source_behavior_flags = 0;
        std::uint8_t source_spawn_state_flags = 0;
        scripting::ScriptCombatantFacts combatant{};
        std::optional<std::uint8_t> effect_slot;
        bool temporary_world_encounter = false;
        bool target_selected = false;
        bool target_marker_visible = false;
        bool target_marker_animating = false;
        std::uint8_t target_marker_style = 0;
        std::uint8_t target_marker_radius = 0;
        std::optional<std::uint8_t> target_motion_effect_slot;
        bool target_motion_effect_visible = false;
        std::uint8_t target_motion_effect_direction = 0;
        std::uint8_t target_motion_effect_phase = 0;
        std::int16_t action_delay = 0;
        bool ai_target_valid = false;
        std::uint16_t ai_target_heading = 0;
        std::uint16_t ai_target_distance = 0;
        std::string ai_target_id;
        EntityMovementStepState movement_state{};
        EntityCollisionRecord movement_collision_record{};
        std::array<std::uint8_t, 4> movement_sample_state{};
        std::uint16_t movement_terrain_collision_code = 0U;
        bool movement_state_initialized = false;
        std::string ordered_overrides;
        std::uint8_t initial_behavior_delay = 0;
        std::uint8_t entry_animation_frame_count = 2;
        std::uint8_t walk_animation_frame_count = 0;
        std::uint8_t action_animation_frame_count = 0;
        std::uint16_t action_animation_increment = 0;
        std::uint16_t action_animation_threshold = 0;
        std::uint16_t action_animation_audio_event = 0;
        std::uint8_t hurt_animation_frame_count = 0;
        std::uint16_t hurt_animation_increment = 0;
        std::uint16_t hurt_animation_threshold = 0;
        std::uint16_t hurt_audio_event = 0;
        std::uint16_t down_audio_event = 0;
        std::uint8_t animation_direction = 0;
        std::string animation_id = "idle";
        std::uint8_t animation_frame = 0;
        bool transition_started = false;
        bool transition_complete = false;
        std::string ai_package_id;
        std::optional<std::uint8_t> ai_weapon_item_id;
    };

    struct DefaultActorArtwork {
        std::array<QImage, 8> idle_images{};
        std::array<int, 8> idle_origin_x{};
        std::array<int, 8> idle_origin_y{};
        std::array<std::array<QImage, 8>, 8> entry_images{};
        std::array<std::array<int, 8>, 8> entry_origin_x{};
        std::array<std::array<int, 8>, 8> entry_origin_y{};
        std::array<std::vector<QImage>, 8> walk_images{};
        std::array<std::vector<int>, 8> walk_origin_x{};
        std::array<std::vector<int>, 8> walk_origin_y{};
        std::array<std::vector<QImage>, 8> action_images{};
        std::array<std::vector<int>, 8> action_origin_x{};
        std::array<std::vector<int>, 8> action_origin_y{};
        std::array<std::vector<QImage>, 8> hurt_images{};
        std::array<std::vector<int>, 8> hurt_origin_x{};
        std::array<std::vector<int>, 8> hurt_origin_y{};
        std::uint8_t walk_frame_count = 0;
        std::uint8_t entry_frame_count = 2;
        std::uint8_t action_frame_count = 0;
        std::uint16_t action_frame_increment = 0;
        std::uint16_t action_frame_threshold = 0;
        std::uint16_t action_audio_event = 0;
        std::uint8_t hurt_frame_count = 0;
        std::uint16_t hurt_frame_increment = 0;
        std::uint16_t hurt_frame_threshold = 0;
        std::uint16_t hurt_audio_event = 0;
        std::uint16_t down_audio_event = 0;
    };

    struct CreatureEffectArtwork {
        std::array<QImage, 4> images{};
        std::array<int, 4> origin_x{};
        std::array<int, 4> origin_y{};
    };

    // Resource-activity scenes are temporary field excursions. Preserve the
    // active typed scene and runner presentation so source 0x2F32 can rebuild
    // that scene on return without looking for an event table on the resource
    // map.
    struct WorldActivityReturnSnapshot {
        FieldSceneBuffers scene_buffers{};
        FieldSceneEntryState scene_entry{};
        FieldEntityPresentation controlled_actor{};
        EntityMovementStepState movement{};
        EntityMovementModeTransitionState movement_transition{};
        FieldCameraFollowState camera_follow{};
        std::vector<DefaultFieldActorPresentation> default_actors;
        std::array<RenderEntityState, 16> render_entities{};
        WorldActivityState activity_state{};
        WorldActivityTimeoutState activity_timeout{};
        std::array<bool, 18> world_effect_slots_occupied{};
        EntityServiceTargetOwner entity_target_owner{};
        std::vector<EntityServiceTargetRecord> entity_target_records;
        EntityAnimationState player_animation{};
        std::uint16_t player_animation_selector = 0U;
        std::string player_animation_id = "idle";
        bool player_animation_reset_pending = false;
        bool player_walking = false;
        bool player_attack_pose = false;
        std::uint8_t player_attack_direction = 0U;
        std::uint16_t target_heading = 0U;
    };

    struct DefaultActorSpriteView {
        const QImage* image = nullptr;
        int origin_x = 0;
        int origin_y = 0;
    };

    std::function<void(std::string_view)> source_boundary_callback_;
    std::function<void(std::uint16_t)> source_audio_event_callback_;
    integration::FieldSessionBootstrapInput input_;
    integration::FieldSessionBootstrapState state_;
    std::unique_ptr<integration::FieldSessionSourceAssets>
        field_session_source_assets_;
    std::unique_ptr<ConversionAssetCatalog> field_asset_catalog_;
    integration::FieldSessionBootstrapCallbacks callbacks_;
    integration::FieldRuntimeCoordinatorState field_runtime_state_;
    integration::FieldRuntimeCoordinatorCallbacks field_runtime_callbacks_;
    ControllerSampleState field_controller_sample_state_{};
    ControllerLatchState field_controller_latch_state_{};
    WorldFrameState world_frame_state_;
    WorldFrameCallbacks world_frame_callbacks_;
    WorldFrameLogicState world_frame_logic_state_;
    WorldFrameLogicCallbacks world_frame_logic_callbacks_;
    FieldViewportState viewport_state_;
    FieldCommandOverviewBridge command_overview_;
    CommandMenuKarmaViewRuntime command_overview_attributes_skills_{};
    HotelKarmaUpgradeRuntime hotel_karma_skills_{};
    scripting::GameScriptRuntime game_scripts_;
    std::optional<scripting::ScriptShopDefinition> field_shop_definition_;
    FieldDialogueRuntime field_dialogue_;
    HotelMenuRuntime hotel_menu_{};
    HotelMenuContent hotel_menu_content_{};
    PocketSecretaryMenuContent pocket_secretary_menu_content_{};
    FieldModeTextRuntime field_mode_text_{};
    FieldInteractionRuntime field_interaction_;
    ElevatorSelectorRuntime field_elevator_selector_{};
    ElevatorSceneTransitionRuntime field_elevator_scene_transition_{};
    std::optional<scripting::ScriptElevatorFloorGroup>
        field_elevator_floor_group_;
    std::optional<std::uint16_t> field_elevator_script_destination_;
    std::optional<std::uint16_t> field_elevator_pending_destination_;
    ShopMenuController field_shop_menu_{};
    FieldDialogueEventResolver field_dialogue_event_resolver_;
    StokerMotelStoryState stoker_motel_story_state_{};
    FieldPartyRosterRuntime field_party_roster_{};
    DismissRunnerMenuState dismiss_runner_{};
    RandomGridEffectState dismiss_runner_random_grid_effect_{};
    std::uint32_t dismiss_runner_effect_frames_ = 0U;
    std::array<std::int8_t, 12> dismiss_runner_random_grid_cells_{
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    std::size_t field_joshua_archetype_selection_ = 0U;
    QImage field_map_;
    QImage field_map_priority_overlay_;
    QImage field_hud_;
    QImage field_service_background_;
    std::array<QImage, kElevatorSelectorAnimationFrameCount>
        field_elevator_selector_backgrounds_{};
    std::array<QImage, kElevatorSelectorMaximumFloors>
        field_elevator_selector_current_floor_layers_{};
    std::array<QImage, kElevatorSelectorMaximumFloors>
        field_elevator_selector_selected_floor_layers_{};
    std::array<QImage, 4> field_elevator_selector_feedback_layers_{};
    QImage field_service_universal_clips_icon_;
    std::array<QImage, 6> field_service_tab_images_{};
    QImage command_overview_background_;
    QImage command_overview_inventory_boxes_;
    QImage command_overview_font_;
    QImage magic_menu_font_;
    QImage hotel_checkout_font_;
    QImage command_overview_cursor_;
    FieldMenuTextCatalog field_menu_text_{};
    std::array<QImage, 3> command_overview_portraits_{};
    std::array<QImage, 5> command_overview_focus_{};
    std::array<QImage, 5> command_overview_status_bars_{};
    std::array<QImage, 2> command_overview_selected_cells_{};
    QImage command_overview_action_background_;
    std::array<QImage, 4> command_overview_action_focus_{};
    QImage command_overview_action_confirmation_background_;
    std::array<QImage, 2> command_overview_action_confirmation_focus_{};
    QImage command_overview_clips_background_;
    QImage command_overview_clips_trade_background_;
    std::array<QImage, 4> command_overview_clips_focus_{};
    std::array<QImage, 4> command_overview_clips_trade_focus_{};
    QImage command_overview_magic_panel_background_;
    std::array<QImage, kMagicSpellTextCount> command_overview_magic_spell_icons_{};
    std::array<QImage, 4> command_overview_magic_cursor_frames_{};
    std::array<QImage, 5> command_overview_magic_posture_bars_{};
    std::array<QImage, 5> command_overview_magic_force_meter_tiles_{};
    std::array<QImage, 22> command_overview_magic_metric_bars_{};
    std::array<QImage, 4> command_overview_magic_availability_bars_{};
    std::array<QImage, 10> command_overview_magic_damage_bars_{};
    std::array<std::array<QImage, 2>, 50> command_overview_inventory_icons_{};
    std::array<QImage, 50> command_overview_item_details_{};
    std::array<QImage, 14> command_overview_damage_bars_{};
    std::array<QImage, 14> command_overview_drain_bars_{};
    std::array<QImage, 9> command_overview_armor_summary_bars_{};
    std::array<QImage, 256> command_overview_detail_values_{};
    std::array<QImage, 23> command_overview_attack_bars_{};
    std::array<QImage, 23> command_overview_defense_bars_{};
    ResourceVariantState command_overview_resource_variant_{};
    EquipmentCombatBarResult command_overview_combat_bars_{};
    ItemActionDetailResult command_overview_item_detail_{};
    InventoryActionMenuState command_overview_inventory_action_{};
    CommandOverviewFirstAidState command_overview_first_aid_state_ =
        CommandOverviewFirstAidState::inactive;
    FirstAidTargetList command_overview_first_aid_targets_{};
    std::size_t command_overview_first_aid_cursor_ = 0U;
    FirstAidMessage command_overview_first_aid_message_ =
        FirstAidMessage::none;
    CommandOverviewAutomaticUseState command_overview_automatic_use_state_ =
        CommandOverviewAutomaticUseState::inactive;
    std::uint16_t field_random_seed_ = 0U;
    std::uint16_t field_world_frame_counter_ = 0U;
    std::uint32_t field_world_encounter_sequence_ = 0U;
    std::optional<SelectedInventoryItem>
        command_overview_selected_inventory_item_;
    MenuConfirmationState command_overview_discard_confirmation_{};
    InventoryTradeState command_overview_inventory_trade_{};
    FieldClipsMenuRuntime command_overview_clips_{};
    CyberwareMenuModel command_overview_cyberware_{};
    bool command_overview_cyberware_active_ = false;
    CommandOverviewMagicState command_overview_magic_state_ =
        CommandOverviewMagicState::inactive;
    MagicDefenseStripView command_overview_magic_physical_defense_{};
    MagicDefenseStripView command_overview_magic_mana_defense_{};
    MagicMenuLoopState command_overview_magic_loop_{};
    MagicForceState command_overview_magic_force_{};
    std::optional<MagicSpellSelection> command_overview_magic_selected_spell_;
    std::uint8_t command_overview_magic_cursor_phase_ = 0U;
    std::uint8_t command_overview_magic_cursor_timer_ = 0U;
    CommandOverviewActionMessage command_overview_action_message_ =
        CommandOverviewActionMessage::none;
    bool command_overview_inventory_action_active_ = false;
    bool command_overview_inventory_selector_suppressed_ = false;
    bool command_overview_discard_confirmation_active_ = false;
    std::array<QImage, kFieldHudSlotCount> field_hud_faces_{};
    std::array<QImage, kFieldHudSlotCount> field_hud_items_{};
    std::array<QImage, kFieldHudSlotCount> field_hud_unarmed_items_{};
    std::array<std::array<QImage, kFieldHudPortraitPaletteVariantCount>,
               kFieldHudPortraitFamilyCount>
        field_hud_party_faces_{};
    std::array<std::array<QImage, kFieldHudItemPaletteVariantCount>,
               kFieldHudItemCount>
        field_hud_party_items_{};
    std::array<std::array<QImage, kFieldHudItemPaletteVariantCount>, 3U>
        field_hud_party_unarmed_items_{};
    std::array<std::array<QImage, kFieldHudMagicItemPaletteVariantCount>,
               kFieldHudMagicItemCount>
        field_hud_magic_items_{};
    std::array<QImage, kFieldHudMeterLevelCount>
        field_hud_delay_meter_counters_{};
    std::array<QImage,
               kFieldHudMeterLevelCount * kFieldHudBehaviorMeterProfileCount>
        field_hud_behavior_meter_counters_{};
    std::array<QImage, kFieldHudAmmoCounterCount> field_hud_ammo_counters_{};
    std::array<FieldHudSlotState, kFieldHudSlotCount> field_hud_slots_{};
    PocketSecretaryMenuState pocket_secretary_{};
    // FF018F bit 0 is session state. Fresh New Game starts clear; restored
    // saves remain unknown until that global is included in the save adapter.
    std::optional<bool> cyberdeck_available_ = false;
    // FFFC22 is not present in the current save adapter. New Game initializes
    // it to zero; restored saves remain an explicit source-state boundary.
    std::optional<std::uint32_t> group_item_flags_ = 0U;
    // A fresh New Game has no active run. Save restoration does not yet
    // publish the source FFFBEC run state to this presentation adapter.
    bool notebook_current_run_available_ = false;
    std::array<std::uint32_t, 3> notebook_tip_flags_{{1U, 0U, 0U}};
    bool notebook_tip_flags_resolved_ = true;
    // FFFC2C, FFFC27, and FFFC2A independently gate Contacts, Mr. Johnsons,
    // and Shadowrunners in F33E. Fresh New Game seeds Contacts to 0x1C7F at
    // 0x59C48 and clears the other two; a loaded save does not restore them.
    std::optional<std::uint32_t> notebook_contacts_flags_ = 0x1C7FU;
    std::optional<std::uint8_t> notebook_referral_flags_ = 0U;
    std::optional<std::uint16_t> notebook_shadowrunner_flags_ = 0U;
    QImage pocket_secretary_background_;
    QImage pocket_secretary_grid_;
    QImage hotel_options_cursor_;
    QImage hotel_checkout_cursor_;
    std::unordered_map<std::string, QImage> runner_preview_panels_;
    std::array<QImage, 7> dismiss_runner_random_grid_patterns_{};
    QImage command_overview_attributes_background_;
    QImage posture_focus_;
    bool posture_dialog_active_ = false;
    std::array<QImage, 8> field_entity_idle_images_{};
    std::array<int, 8> field_entity_idle_origin_x_{};
    std::array<int, 8> field_entity_idle_origin_y_{};
    std::array<QImage, 8> field_entity_attack_images_{};
    std::array<int, 8> field_entity_attack_origin_x_{};
    std::array<int, 8> field_entity_attack_origin_y_{};
    std::array<DefaultActorArtwork, 2> field_default_actor_artwork_{};
    std::unordered_map<std::string, DefaultActorArtwork>
        field_dynamic_actor_artwork_;
    std::vector<std::array<QImage, 4>>
        field_target_marker_images_by_scene_;
    std::vector<std::array<CreatureEffectArtwork, 8>>
        field_hound_close_attack_images_by_scene_;
    std::vector<DefaultFieldActorPresentation> field_default_actors_;
    std::array<bool, 18> field_world_effect_slots_occupied_{};
    EntityServiceTargetOwner field_entity_target_owner_{};
    std::vector<EntityServiceTargetRecord> field_entity_target_records_{16U};
    std::optional<scripting::ScriptFieldEncounterOutcome>
        pending_field_encounter_outcome_;
    std::array<std::vector<QImage>, 16> field_weapon_projectile_images_{};
    std::array<std::vector<int>, 16> field_weapon_projectile_origin_x_{};
    std::array<std::vector<int>, 16> field_weapon_projectile_origin_y_{};
    std::vector<FieldWeaponProjectilePresentation>
        field_weapon_projectiles_;
    // ROM 0x15A8C scans the six combat slots 9..14. These slots are separate
    // from the ten auxiliary world-map entries in field_entity_table_.
    std::array<bool, 6> field_weapon_slots_occupied_{};
    EntitySpawnCycleResult last_field_weapon_spawn_result_{};
    // Ordinary accepted movement uses four looping poses from 0xC062. C0B2's
    // eight-entry high-bit list is a separate finite behavior transition.
    std::array<std::vector<QImage>, 8> field_entity_walk_images_{};
    std::array<std::vector<int>, 8> field_entity_walk_origin_x_{};
    std::array<std::vector<int>, 8> field_entity_walk_origin_y_{};
    EntityAnimationState field_entity_animation_{};
    std::uint16_t field_entity_animation_selector_ = 0;
    std::string field_entity_animation_id_ = "idle";
    bool field_player_animation_reset_pending_ = false;
    bool field_entity_walking_ = false;
    bool field_entity_attack_pose_active_ = false;
    std::uint8_t field_entity_attack_direction_ = 0U;
    bool field_combat_input_suppressed_until_release_ = false;
    std::array<MovementOffset, integration::kFieldMovementDirectionCount>
        field_movement_direction_offsets_{};
    CollisionProbeOffsetBank field_collision_probe_offset_banks_{};
    std::array<std::uint16_t,
               integration::kFieldControllerHeadingTableCount>
        field_controller_heading_table_{};
    std::array<DirectionalMovementRoute,
               integration::kFieldMovementRouteCount>
        field_movement_routes_{};
    std::array<integration::FieldMovementRouteSourceRecord,
               integration::kFieldMovementRouteCount>
        field_movement_route_sources_{};
    EntityMovementStepState field_movement_state_{};
    EntityCollisionRecord field_collision_record_{};
    CollisionProbeState field_collision_probe_state_{};
    EntityBehaviorFrameRecord field_movement_behavior_record_{};
    RunnerSwitchState field_entity_selection_state_{};
    FieldRunnerRefreshState field_runner_refresh_state_{};
    FieldModeExitTransitionState field_mode_exit_transition_state_{};
    std::array<std::uint16_t, 10> field_entity_table_occupancy_{};
    std::array<AuxiliaryWorldEntity, 10> field_entity_table_{};
    EntityMovementModeTransitionState field_movement_transition_state_{};
    FieldMapEventSystem field_map_event_system_;
    std::uint32_t field_map_event_state_flags_ = 0U;
    // Retain the last typed result from the blocked-movement dispatcher so
    // route QA can inspect the source event and continuation after a modal
    // handler has consumed its pending state.
    EntityMovementModeTransitionResult last_field_movement_transition_result_{};
    std::optional<FieldScriptRoute> active_field_script_route_;
    // The ROM blocks inside scene-2/event-4's indexed title service. The Qt
    // modal yields across frames, so retain the selected area route and resume
    // its scene-entry tail only after the player dismisses the entry page.
    bool jump_house_entry_transition_pending_ = false;
    bool field_mode_entry_transition_pending_ = false;
    std::optional<std::uint16_t> field_mode_entry_destination_scene_;
    std::int8_t field_indexed_title_service_result_ = 0;
    std::uint8_t field_palette_fade_steps_ = 0U;
    std::uint8_t field_modal_return_transition_frames_remaining_ = 0U;
    std::uint8_t field_modal_return_blocked_inputs_ = 0U;
    bool field_world_activity_scene_entry_pending_ = false;
    WorldActivityState field_world_activity_state_{};
    WorldActivityTimeoutState field_world_activity_timeout_state_{};
    WorldActivityRoutineState world_activity_routine_state_{};
    WorldActivityRoutineRequestState world_activity_request_state_{};
    WorldActivityStartState world_activity_request_start_state_{};
    std::optional<WorldActivityReturnSnapshot>
        world_activity_return_snapshot_;
    MovementModeTransitionContinuation field_activity_continuation_ =
        MovementModeTransitionContinuation::none;
    // FFE12C selects a floor label on the successful Security Terminal file
    // branch. It is sampled by the live world-activity dispatcher.
    std::optional<std::uint8_t> field_security_floor_index_;
    std::uint8_t field_projected_terrain_mark_count_ = 0;
    bool field_terrain_render_dirty_ = false;
    std::array<std::uint8_t, 4> field_terrain_sample_state_{};
    // Current outputs of the most recent source 0xE7C8 terrain probe.
    bool field_terrain_boundary_active_ = false;
    bool field_terrain_outside_bounds_marker_ = false;
    // FFDFE4. Index of the FF1700 collision byte selected by the last 0xE8A0
    // sample. 0x1651E clears that byte; 0xE9EE may leak A2, but the index is
    // only defined for the collision table.
    std::uint16_t field_collision_marker_index_ = 0;
    FieldCameraFollowState field_camera_follow_{};
    bool field_camera_alignment_ready_ = false;
    // FFE114 when 0x1651E selects the scene-4 interaction record. 0x2B34
    // consumes it on the next scene entry.
    std::optional<std::array<std::uint16_t, 8>> field_pending_scene_profile_;
    std::uint8_t field_controller_nibble_ = 0;
    std::uint8_t field_controller_port_byte_ = 0;
    std::uint16_t field_target_heading_ = 0;
    FieldOpeningAutoWalkState field_opening_auto_walk_{};
    bool field_player_actor_script_initialized_ = false;
    // Typed source field corresponding to FF014C, consumed only by the
    // 0xC7F8 scene-runtime dispatcher.
    std::uint8_t field_action_counter_ = 0;
    std::uint8_t field_entity_behavior_flags_ = 0;
    std::int8_t field_exit_alternate_result_ = 0;
    std::uint32_t field_exit_world_flags_ = 0;
    // Typed counterparts for the field story-service registers. These remain
    // session state until the save adapter restores their source facts.
    std::uint32_t field_mode_effect_flags_ = 0;
    // FFFC27 bit 6 records the first Vigore/Jarl booth introduction.
    std::optional<std::uint8_t> field_dialogue_register_{0U};
    std::uint32_t field_render_effect_flags_ = 0;
    std::optional<std::uint32_t> field_funds_{0x14U};
    // The fresh New Game runtime initializes these typed source globals to
    // zero. The loaded-save adapter does not restore them, so paid writes stay
    // explicit for that path.
    std::optional<std::uint8_t> field_runner_feature_flags_{0U}; // FF018F
    std::optional<std::array<std::uint8_t, 3>>
        field_entity_action_phase_roll_counters_{std::array<std::uint8_t, 3>{}};
    bool field_shop_route_pending_ = false;
    FieldEntityPresentation field_entity_{};
    std::string failure_;
    bool ready_ = false;
    std::optional<std::uint16_t> field_scene_reference_override_for_dump_;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
