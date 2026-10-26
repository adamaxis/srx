#pragma once

#include "shadowrun_ghidra/core/controller_wait_conversion.hpp"
#include "shadowrun_ghidra/cyberspace/cyberdeck_record.hpp"
#include "shadowrun_ghidra/menus/pocket_secretary_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/magic_menu_conversion.hpp"
#include "shadowrun_ghidra/menus/first_aid_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_state_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_camera_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_conversion.hpp"
#include "shadowrun_ghidra/field/field_hud_conversion.hpp"
#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"
#include "shadowrun_ghidra/field/field_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_animation_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_route_primitives_conversion.hpp"
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
#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"
#include "shadowrun_ghidra/world/world_entity_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_edge_conversion.hpp"
#include "shadowrun_ghidra/world/world_frame_logic_conversion.hpp"
#include "shadowrun_ghidra/world/world_scene_entity_conversion.hpp"
#include "shadowrun_ghidra/world/world_activity_routine_conversion.hpp"
#include "shadowrun_ghidra/scripting/dialogue_outcome_dispatcher.hpp"

#include <QImage>
#include <QJsonObject>

#include "attributes_skills_menu_runtime.hpp"
#include "field_command_overview_bridge.hpp"
#include "field_clips_menu_runtime.hpp"
#include "field_party_roster_runtime.hpp"
#include "field_dialogue_runtime.hpp"
#include "hotel_menu_runtime.hpp"
#include "field_mode_text_runtime.hpp"
#include "field_interaction_runtime.hpp"
#include "elevator_selector_runtime.hpp"
#include "tarislar_credits_title_return_runtime.hpp"
#include "field_menu_text_catalog.hpp"
#include "field_vdp_mirror.hpp"
#include "game_save_snapshot.hpp"
#include "game_save_store.hpp"
#include "shop_menu_runtime.hpp"
#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"
#include "enums.hpp"

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
class QColor;

#include "shadowrun_ghidra/core/random_conversion.hpp"

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class FieldSceneProfileWord : std::uint8_t {
    random_event_probability_threshold = 41,
};

constexpr std::size_t field_scene_profile_word_index(
    const FieldSceneProfileWord word) noexcept {
    return static_cast<std::size_t>(word);
}

enum class FieldWorldFactFlag : std::uint8_t {
    errascoe_question_available = 1,
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
    run_objective_active = 9, // FFFC22 bit 9, ROM 0x1EF2C/0x1F000 [C]
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

struct FieldBarrierCollisionCell {
    std::uint16_t cell_index = 0U;
    std::uint8_t code = 0U;
};

struct FieldBarrierSpriteSegment {
    std::uint8_t source_index = 0U;
    std::int16_t pixel_offset_x = 0;
    std::int16_t pixel_offset_y = 0;
};

struct FieldBarrierSpritePlan {
    std::uint16_t center_x = 0U;
    std::uint16_t center_y = 0U;
    std::uint8_t frame_index = 0U;
    std::uint8_t palette_line = 0U;
    bool low_priority = true;
    std::array<FieldBarrierSpriteSegment, 2U> segments{};
};

struct FieldBarrierPresentationOrigin {
    std::uint16_t center_x = 0U;
    std::uint16_t center_y = 0U;
    std::uint8_t octant = 0U;
};

inline constexpr std::size_t kFieldBarrierSpriteFrameCount =
    static_cast<std::size_t>(srx::v(srx::FieldBarrier::SPRITE_FRAME_COUNT));

inline constexpr std::size_t kFieldBarrierCollisionCellCount =
    static_cast<std::size_t>(srx::v(srx::FieldBarrier::CELL_COUNT));

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

// Opt-in trace for one ordinary player movement pass. The normalized sample
// coordinates and four terrain bytes make a collision decision auditable
// against ROM 0xE7C8/0xE8A0 without changing movement policy.
struct FieldTerrainProbeTrace {
    std::uint16_t candidate_x_fixed = 0;
    std::uint16_t candidate_y_fixed = 0;
    std::uint16_t normalized_x = 0;
    std::uint16_t normalized_y = 0;
    std::array<std::uint16_t, 4> corner_x{};
    std::array<std::uint16_t, 4> corner_y{};
    std::array<std::uint8_t, 4> terrain_bytes{};
    std::uint16_t selected_tile_index = 0;
    std::int16_t raw_terrain_code = 0;
    std::int16_t collision_result = 0;
    bool expanded_probe = false;
    bool outside_bounds = false;
};

// Composition-only adapter for the separate Ghidra-conversion Qt executable.
// It owns no ROM view and does not duplicate converted field state. The
// conversion bootstrap remains authoritative; this class supplies host
// presentation callbacks and paints the direct source assets it receives.
class FieldSceneRuntime final {
public:
    explicit FieldSceneRuntime(
        std::function<void(std::string_view)> source_boundary,
        std::function<void(std::uint16_t)> source_audio_event = {},
        std::function<void(std::uint16_t)> source_audio_stop = {});

    bool bootstrap(
        const integration::StartupIntegrationState& startup_state);
    bool bootstrap_for_scene_dump(
        const integration::StartupIntegrationState& startup_state,
        std::uint16_t scene_reference);
    bool reenter_field_scene_for_dump(std::uint16_t scene_reference);
    void advance_frame();
    // Combat integration may submit the source-ordered SAT pieces for the
    // next opt-in VDP-mirror frame. The mirror is the only owner of this list.
    void stage_field_sat_pieces(std::vector<VdpSpritePiece> pieces);
    void set_controller_nibble(std::uint8_t dpad_nibble) noexcept;
    // Host audio entry points beyond play_sfx/stop_sfx (constructor
    // callbacks): play_music 0x530EA, op 0x20 attenuation 0x5315E, and the
    // 0x53126 fade. Each is optional.
    void set_audio_host_callbacks(
        std::function<void(std::uint16_t)> play_music,
        std::function<void(std::uint8_t)> attenuate,
        std::function<void()> fade_out);
    const std::vector<std::uint16_t>& field_music_requests() const noexcept {
        return field_music_requests_;
    }
    const std::vector<std::uint8_t>& field_attenuation_requests() const noexcept {
        return field_attenuation_requests_;
    }
    std::size_t field_fade_requests() const noexcept {
        return field_fade_requests_;
    }
    void set_controller_port_byte(std::uint8_t port_byte) noexcept;
    void set_actor_heartbeat_observer(
        scripting::ScriptActorHeartbeatObserver observer);
    void paint(QPainter& painter) const;
    QJsonObject debug_inspection_fields() const;
    void paint_field_scene_for_dump(QPainter& painter) const;
    std::size_t default_actor_count_for_dump() const noexcept;
    std::size_t dynamic_world_actor_count_for_dump() const noexcept;
    std::size_t world_activity_script_actor_count_for_dump() const noexcept;
    std::optional<SceneRunnerPosition>
    world_activity_script_actor_position_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::string>
    world_activity_script_actor_profile_for_dump(
        std::uint16_t source_order) const;
    std::optional<FieldWorldActorSlot>
    world_activity_script_actor_record_slot_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::uint8_t>
    world_activity_script_actor_behavior_flags_for_dump(
        std::uint16_t source_order) const noexcept;
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
    std::optional<std::uint8_t> default_actor_source_action_ammo_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::uint16_t>
    default_actor_source_animation_selector_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::uint8_t> default_actor_weapon_item_for_dump(
        std::uint16_t source_order) const noexcept;
    std::optional<std::uint8_t> default_actor_physical_meter_for_dump(
        std::uint16_t source_order) const noexcept;
    // Seeds a live encounter actor for deterministic field QA only.
    bool set_default_actor_physical_meter_for_dump(
        std::uint16_t source_order, std::uint8_t physical_meter) noexcept;
    // QA stand-in for the fight: zero the +0x4C meter of every Club
    // Penumbra Mako list member so 0xB330/0xAB02 run as in combat.
    std::size_t defeat_mako_actor_list_for_qa() noexcept;
    bool open_command_overview_for_dump();
    bool menu_display_blank_active_for_dump() const noexcept {
        return command_overview_start_transition_black_frame_ ||
               command_overview_start_transition_frames_remaining_ != 0U ||
               menu_blank_reveal_line_ != 0U || !menu_lead_masks_.empty() ||
               !menu_reveal_masks_pending_.empty() || !menu_reveal_masks_.empty();
    }
#ifdef SHADOWRUN_QT_QA_DRIVER
    bool field_alarm_active_for_dump() const noexcept {
        return field_matrix_field_state_.alarm.value_or(false);
    }
    void set_field_alarm_disabled_for_dump(bool disabled) noexcept {
        field_matrix_field_state_.security_off = disabled;
    }
    std::int16_t field_alarm_timer_for_dump() const noexcept {
        return field_alarm_timer_;
    }
    void set_field_alarm_timer_for_dump(std::int16_t timer) noexcept {
        field_alarm_timer_ = timer;
    }
    bool dispatch_dialogue_service_for_dump(
        std::string_view service_id, std::string_view return_policy);
    // Start the source FFE166 party-wipe countdown for focused transition QA.
    void arm_party_wipe_countdown_for_dump() noexcept {
        field_party_wipe_countdown_ = 0x3CU; // 0xABB6 [C]
    }
    void set_world_effect_player_spawn_pending_for_dump(
        const bool pending) noexcept {
        field_world_effect_player_spawn_pending_ = pending; // 0x1DED8 FFFC3C [C]
    }
    bool world_effect_player_spawn_pending_for_dump() const noexcept {
        return field_world_effect_player_spawn_pending_;
    }
    const EffectFadeMarkerState& field_effect_fade_marker_for_dump()
        const noexcept {
        return field_effect_fade_marker_;
    }
    bool open_hospital_menu_for_dump(std::string_view function);
    bool open_ordinary_field_terminal_for_dump(
        std::uint32_t contact_flags, std::uint32_t funds);
    bool open_spirit_eyes_ritual_for_dump();
    bool open_tarislar_tomb_credits_for_dump();
    std::string_view active_credits_page_id_for_dump() const noexcept {
        return field_dialogue_.credits_page_id_for_dump();
    }
    std::string_view active_story_page_text_for_dump() const noexcept {
        return field_dialogue_.story_page_text_for_dump();
    }
    std::string_view active_story_page_text_key_for_dump() const noexcept {
        return field_dialogue_.story_page_text_key_for_dump();
    }
    bool seed_cyberspace_access_for_dump();
    bool seed_cyberdeck_reference_frame_for_dump();
    bool seed_cyberdeck_pull_plug_reference_for_dump();
    bool seed_matrix_storage_files_for_dump();
    bool open_matrix_traversal_for_dump(std::uint8_t system_id,
                                        std::uint8_t node_id);
    bool seed_matrix_degrade_action_for_dump();
    bool seed_run_target_for_dump(std::string_view run_id,
                                  std::uint8_t map_id,
                                  std::uint8_t event_id);
    bool seed_current_run_for_dump(std::uint8_t source_run_type,
                                   bool matrix_max_payment = false);
    bool field_dialogue_uses_console_layout_for_dump() const noexcept {
        return field_dialogue_.uses_field_console_menu_layout();
    }
    bool field_dialogue_uses_hospital_layout_for_dump() const noexcept {
        return field_dialogue_.uses_hospital_menu_layout();
    }
    bool field_dialogue_uses_hospital_medical_attention_layout_for_dump()
        const noexcept {
        return field_dialogue_.uses_hospital_medical_attention_layout();
    }
    bool matrix_search_route_active_for_dump() const noexcept {
        return field_matrix_route_active_ &&
            menu_route_screen_ == static_cast<std::uint16_t>(
                                      srx::MenuScreen::MATRIX_SEARCH);
    }
    std::uint16_t field_menu_route_screen_for_dump() const noexcept {
        return menu_route_screen_;
    }
    scripting::ScriptMenuRouteSnapshot
    field_menu_route_snapshot_for_dump() const noexcept {
        return game_scripts_.menu_route_snapshot();
    }
    bool open_taxi_selector_for_dump(std::uint8_t current_region);
    // Injects the stale parent action-row condition for a visual overlap probe.
    void set_inventory_action_overlay_probe_for_dump(bool active) noexcept;
    CollisionProbeState field_collision_probe_state_for_dump() const noexcept {
        return field_collision_probe_state_;
    }
    void seed_field_door_contact_for_dump(
        std::array<std::uint8_t, 3> probe_codes);
#endif
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
    bool dialogue_input_locked() const noexcept {
        return field_dialogue_.input_lockout_active();
    }
    const DialoguePanelView& dialogue_panel() const noexcept {
        return field_dialogue_.panel();
    }
    bool open_stoker_dialogue_for_dump(StokerMotelPromptId prompt) {
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
    bool dispatch_dialogue_outcome_for_dump();
    bool hotel_menu_open() const noexcept { return hotel_menu_.open(); }
    bool hotel_karma_screen_loading() const noexcept {
        return hotel_karma_skills_.is_open() && hotel_karma_load_frames_ > 0U;
    }
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
    std::uint16_t source_random_seed_for_dump() const noexcept {
        return field_random_seed_;
    }
    std::uint16_t field_world_frame_counter_for_dump() const noexcept {
        return field_world_frame_counter_;
    }
    // Places the source field tick at a deterministic cadence boundary for QA.
    void set_field_world_frame_counter_for_dump(
        std::uint16_t frame_counter) noexcept {
        field_world_frame_counter_ = frame_counter;
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
            (*notebook_contacts_flags_ & 0x1FFFU) != 0U,
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
    // R3: apply the 0x16760 terrain mark to an explicit FF2700 cell.
    bool mark_field_terrain_cell_for_dump(std::size_t cell) {
        return mark_field_terrain_cell(cell);
    }
    const QImage& field_map_for_dump() const noexcept { return field_map_; }
    const QImage& field_map_priority_overlay_for_dump() const noexcept {
        return field_map_priority_overlay_;
    }
#ifdef SHADOWRUN_QT_QA_DRIVER
    void set_field_vdp_mirror_enabled_for_qa(bool enabled);
    const VdpModel* field_vdp_model_for_dump() const noexcept {
        return field_vdp_mirror_ ? &field_vdp_mirror_->model() : nullptr;
    }
    const VdpFrameLatch* field_vdp_frame_latch_for_dump() const noexcept {
        return field_vdp_mirror_ ? &field_vdp_mirror_->frame_latch() : nullptr;
    }
    const FieldVdpSpriteLimitState*
    field_vdp_sprite_limit_state_for_dump() const noexcept {
        return field_vdp_mirror_
            ? &field_vdp_mirror_->sprite_limit_state()
            : nullptr;
    }
    std::pair<int, int> field_vdp_viewport_origin_for_dump() const noexcept {
        if (!field_vdp_mirror_) {
            return {};
        }
        const auto viewport = field_vdp_mirror_->viewport_rect();
        return {viewport.x(), viewport.y()};
    }
#endif
    std::pair<std::uint16_t, std::uint16_t> field_map_cell_words_for_dump(
        const std::size_t cell) const {
        return cell < field_map_cell_words_.size()
            ? field_map_cell_words_[cell]
            : std::pair<std::uint16_t, std::uint16_t>{};
    }
    std::size_t field_map_columns_for_dump() const noexcept {
        return field_map_cell_columns_;
    }
    const std::vector<std::uint16_t>& field_layer_a_for_dump() const noexcept {
        return state_.field_scene_buffers.layer_a;
    }
    const std::vector<std::uint16_t>& field_layer_b_for_dump() const noexcept {
        return state_.field_scene_buffers.layer_b;
    }
    // World-effect map writes (FSR apply path) with an explicit layer.
    std::size_t apply_world_effect_layer_writes_for_dump(
        const bool layer_b_target, const std::vector<MapWordWrite>& writes) {
        return apply_world_effect_layer_writes(layer_b_target, writes);
    }
    std::uint16_t field_normal_render_pending_for_dump() const noexcept {
        return viewport_state_.normal_render_pending;
    }
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
    // Seed the exact FFFBF9/FFFC10/FFFC1E and FFFC0E inputs for the mode-table
    // route QA. The following advance_frame uses the normal coordinator and
    // scene-entry callbacks; this does not create a second transition path.
    void seed_field_mode_transition_for_dump(
        std::uint8_t mode_event_code, std::int16_t transition_context,
        std::uint32_t world_activity_flags) noexcept {
        field_movement_transition_state_.mode_event_code = mode_event_code;
        field_movement_transition_state_.field_transition_context =
            transition_context;
        field_exit_world_flags_ = world_activity_flags;
        state_.field_scene_buffers.scene_mode = 1U;
        field_runtime_state_.field_transition_requested = true;
    }
    std::uint16_t field_transition_context_for_dump() const noexcept {
        return field_movement_transition_state_.field_transition_context;
    }
    std::uint8_t field_activity_selection_for_dump() const noexcept {
        return field_movement_transition_state_.activity_selection;
    }
    const FieldMapEventSystem& field_map_event_system_for_dump() const noexcept {
        return field_map_event_system_;
    }
    std::uint16_t field_terrain_collision_code_for_dump() const noexcept {
        return field_movement_transition_state_.terrain_collision_code;
    }
    void set_terrain_probe_trace_enabled_for_dump(bool enabled) noexcept {
        field_terrain_probe_trace_enabled_ = enabled;
        if (!enabled) {
            field_terrain_probe_trace_.clear();
        }
    }
    const std::vector<FieldTerrainProbeTrace>&
    terrain_probe_trace_for_dump() const noexcept {
        return field_terrain_probe_trace_;
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
    bool field_door_return_active() const noexcept {
        return field_door_return_active_;
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
    bool begin_field_mode_service_event_for_dump(
        std::uint8_t scene_id, std::uint8_t field_mode,
        std::uint8_t event_code, std::int8_t alternate_result);
    bool shop_menu_open() const noexcept { return field_shop_menu_.is_open(); }
    bool field_shop_notice_open_for_dump() const noexcept {
        return field_shop_notice_active_;
    }
    std::string_view field_shop_notice_text_key_for_dump() const noexcept {
        return field_shop_notice_text_key_;
    }
    std::uint8_t shop_menu_mode() const noexcept {
        return field_shop_menu_.menu_mode();
    }
    std::int8_t shop_menu_result() const noexcept {
        return field_shop_menu_.menu_result();
    }
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
    std::uint32_t shop_transaction_amount() const noexcept {
        return field_shop_menu_.transaction_amount();
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
    // The Boris event-6 QA starts at the information-already-paid branch;
    // New Game clears FFFC26, so seed that known byte without replaying the
    // separate $500 purchase route.
    void seed_field_dialogue_flags_for_qa(std::uint8_t flags) noexcept {
        field_dialogue_flags_ = flags;
    }
    std::optional<QJsonObject> capture_game_save_snapshot_for_dump(
        QString* error) {
        return capture_game_save_snapshot(error);
    }
    bool restore_game_save_snapshot_for_dump(
        const QJsonObject& snapshot, QString* error) {
        return restore_game_save_snapshot(snapshot, error);
    }
    const MagicSpellTable* field_magic_spell_table_for_dump(
        const std::uint16_t table_index) const noexcept {
        return field_magic_spell_table(table_index);
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
        if (menu_route_active_) {
            return menu_route_screen_ == static_cast<std::uint16_t>(
                                             srx::MenuScreen::MAGICAL_DEFENSE);
        }
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::magical_defense_wait;
    }
    bool magic_spell_panel_open() const noexcept {
        if (menu_route_active_) {
            return menu_route_screen_ == static_cast<std::uint16_t>(
                                             srx::MenuScreen::MAGIC);
        }
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_panel;
    }
    bool magic_force_selection_open() const noexcept {
        if (menu_route_active_) {
            return menu_route_screen_ == static_cast<std::uint16_t>(
                                             srx::MenuScreen::MAGIC_FORCE);
        }
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_force_selection;
    }
    bool magic_posture_selection_open() const noexcept {
        if (menu_route_active_) {
            return menu_route_screen_ == static_cast<std::uint16_t>(
                                             srx::MenuScreen::MAGIC_POSTURE);
        }
        return command_overview_magic_state_ ==
               CommandOverviewMagicState::spell_posture_selection;
    }
    bool magic_spell_interface_open() const noexcept {
        if (menu_route_active_) {
            return (magic_spell_panel_open() ||
                    magic_force_selection_open() ||
                    magic_posture_selection_open()) &&
                   menu_route_magic_view_.has_value();
        }
        return magic_spell_panel_open() || magic_force_selection_open() ||
               magic_posture_selection_open();
    }
    std::uint16_t magic_current_force() const noexcept {
        if (menu_route_active_ && menu_route_magic_view_.has_value()) {
            return static_cast<std::uint16_t>(std::clamp(
                menu_route_magic_view_->force, 0, 0xFFFF));
        }
        return command_overview_magic_force_.current_force;
    }
    std::uint16_t magic_maximum_force() const noexcept {
        if (menu_route_active_ && menu_route_magic_view_.has_value()) {
            return static_cast<std::uint16_t>(std::clamp(
                menu_route_magic_view_->maximum_force, 0, 0xFFFF));
        }
        return command_overview_magic_force_.maximum_force;
    }
    MagicMenuSelection magic_spell_selection() const noexcept {
        if (menu_route_active_ && menu_route_magic_view_.has_value()) {
            return {static_cast<std::uint8_t>(std::clamp(
                        menu_route_magic_view_->column, 0, 0xFF)),
                    static_cast<std::uint8_t>(std::clamp(
                        menu_route_magic_view_->row, 0, 0xFF))};
        }
        return command_overview_magic_loop_.selection;
    }
    const std::optional<MagicSpellSelection>& magic_selected_spell() const
        noexcept {
        return command_overview_magic_selected_spell_;
    }
    std::uint8_t magic_cursor_phase() const noexcept {
        if (menu_route_active_ && menu_route_magic_view_.has_value()) {
            return static_cast<std::uint8_t>(std::clamp(
                menu_route_magic_view_->cursor_phase, 0, 0xFF));
        }
        return command_overview_magic_cursor_phase_;
    }
    const MagicDefenseStripView& magic_physical_defense_view() const noexcept {
        if (menu_route_active_ && menu_route_magic_view_.has_value()) {
            return menu_route_magic_physical_defense_;
        }
        return command_overview_magic_physical_defense_;
    }
    const MagicDefenseStripView& magic_mana_defense_view() const noexcept {
        if (menu_route_active_ && menu_route_magic_view_.has_value()) {
            return menu_route_magic_mana_defense_;
        }
        return command_overview_magic_mana_defense_;
    }
    const CyberwareMenuModel& cyberware_menu_model() const noexcept {
        return command_overview_cyberware_;
    }
    std::uint8_t clips_menu_selection() const noexcept {
        return command_overview_clips_.selection();
    }
    bool clips_trade_open() const noexcept {
        return command_overview_clips_.trade_open();
    }
    std::uint8_t clips_trade_selection() const noexcept {
        return command_overview_clips_.trade_selection();
    }
    std::uint8_t clips_trade_quantity() const noexcept {
        return command_overview_clips_.trade_quantity();
    }
    std::size_t clips_trade_recipient_record() const noexcept {
        return command_overview_clips_.trade_recipient_record();
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
    const NotebookMenuState::CurrentRunState&
    pocket_secretary_notebook_current_run_for_dump() const noexcept {
        return pocket_secretary_.notebook.current_run;
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
    bool pocket_secretary_notebook_contacts_open() const noexcept {
        return pocket_secretary_.notebook.contacts.open;
    }
    bool pocket_secretary_notebook_contacts_detail_waiting() const noexcept {
        return pocket_secretary_.notebook.contacts.detail_wait;
    }
    std::uint8_t pocket_secretary_notebook_contacts_visible_count() const noexcept {
        return pocket_secretary_.notebook.contacts.visible_count;
    }
    std::uint8_t pocket_secretary_notebook_contacts_source_index(
        std::size_t entry) const noexcept {
        const auto& page = pocket_secretary_.notebook.contacts;
        return entry < page.visible_count
                   ? page.visible_source_indices[entry]
                   : 0xFFU;
    }
    bool pocket_secretary_notebook_shadowrunners_open() const noexcept {
        return pocket_secretary_.notebook.shadowrunners.open;
    }
    bool pocket_secretary_notebook_shadowrunners_detail_waiting() const noexcept {
        return pocket_secretary_.notebook.shadowrunners.detail_wait;
    }
    std::uint8_t pocket_secretary_notebook_shadowrunners_visible_count() const noexcept {
        return pocket_secretary_.notebook.shadowrunners.visible_count;
    }
    std::uint8_t pocket_secretary_notebook_shadowrunners_source_index(
        std::size_t entry) const noexcept {
        const auto& page = pocket_secretary_.notebook.shadowrunners;
        return entry < page.visible_count
                   ? page.visible_source_indices[entry]
                   : 0xFFU;
    }
    bool notebook_tip_flags_resolved() const noexcept {
        return notebook_tip_flags_resolved_;
    }
    // Physical-entry QA can combine a positional checkpoint with the known
    // New Game Notebook tip seed without pretending an arbitrary loaded save
    // has resolved tip data.
    void seed_notebook_tip_flags_for_qa(
        std::array<std::uint32_t, 3> flags) noexcept {
        notebook_tip_flags_ = flags;
        notebook_tip_flags_resolved_ = true;
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
    // QA can explicitly seed FFFC27 to exercise referral-gated Notebook rows.
    // Production New Game and save restoration keep their own source state.
    void seed_notebook_referral_flags_for_qa(std::uint8_t flags) noexcept {
        notebook_referral_flags_ = flags;
    }
    std::optional<std::array<std::uint8_t, 3>>
    runner_source_byte_triplet_for_dump() const noexcept {
        return field_entity_action_phase_roll_counters_;
    }
    void seed_runner_source_byte_triplet_for_qa(
        std::array<std::uint8_t, 3> source_bytes) noexcept {
        field_entity_action_phase_roll_counters_ = source_bytes;
    }
    bool set_notebook_contact_flags_for_dump(std::uint32_t flags) noexcept {
        if (!notebook_contacts_flags_.has_value()) return false;
        // FFFC2C is a full state long. Its low bits unlock phone contacts,
        // while higher bits also carry field transitions such as the
        // Scene-20 entry marker at bit 26 and Macklemann's access flag at 28.
        notebook_contacts_flags_ = flags;
        return true;
    }
    // QA can explicitly seed FFFC2C to exercise Contacts gates and Vid-Phone
    // records. Production New Game and save restoration keep their own state.
    void seed_notebook_contact_flags_for_qa(std::uint32_t flags) noexcept {
        notebook_contacts_flags_ = flags;
    }
    std::optional<std::uint32_t> notebook_contact_flags_for_dump() const noexcept {
        return notebook_contacts_flags_;
    }
    bool set_notebook_shadowrunner_flags_for_dump(std::uint16_t flags) noexcept {
        if (!notebook_shadowrunner_flags_.has_value()) return false;
        notebook_shadowrunner_flags_ =
            static_cast<std::uint16_t>(flags & 0x03FFU);
        return true;
    }
    void seed_notebook_shadowrunner_flags_for_dump(
        std::uint16_t flags) noexcept {
        // A QA map checkpoint can use New Game's runner record without
        // restoring Notebook RAM. New Game clears FFFC2A, so the harness can
        // explicitly seed that known source-zero value.
        notebook_shadowrunner_flags_ =
            static_cast<std::uint16_t>(flags & 0x03FFU);
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
    bool set_story_flag_for_dump(std::uint8_t bit, bool value) noexcept {
        if (bit >= 32U) return false;
        const auto mask = std::uint32_t{1U} << bit;
        field_exit_world_flags_ = value
            ? field_exit_world_flags_ | mask
            : field_exit_world_flags_ & ~mask;
        return true;
    }
    void set_field_exit_alternate_result_for_dump(
        std::int8_t result) noexcept {
        field_exit_alternate_result_ = result;
    }
    void set_compact_runner_markers_for_dump(
        std::uint8_t primary_marker,
        std::uint8_t secondary_marker) noexcept {
        field_party_roster_.set_compact_markers_for_dump(
            primary_marker, secondary_marker);
    }
    std::uint32_t field_mode_effect_flags_for_dump() const noexcept {
        return field_mode_effect_flags_;
    }
    bool pending_world_effect_for_dump() const noexcept {
        return field_pending_world_effect_;
    }
    std::uint32_t field_mode_flags_for_dump() const noexcept {
        return field_mode_flags_;
    }
    // Physical-entry QA seeds the typed FFFC30 fact without relying on a
    // gameplay action to create the initial test condition.
    void seed_field_mode_flags_for_qa(std::uint32_t flags) noexcept {
        field_mode_flags_ = flags;
        field_mode_flags_resolved_ = true;
    }
    std::uint32_t field_auxiliary_flags_for_dump() const noexcept {
        return field_auxiliary_flags_;
    }
    std::uint32_t field_world_fact_flags_for_dump() const noexcept {
        return field_exit_world_flags_;
    }
    std::optional<std::array<std::uint16_t, 8>>
    field_pending_scene_profile_for_dump() const noexcept {
        return field_pending_scene_profile_;
    }
    std::uint32_t field_render_effect_flags_for_dump() const noexcept {
        return field_render_effect_flags_;
    }
    // FFF101 rule, or the kDebugMenuAlwaysUnlockedForTesting override: the
    // hidden sixth (unlabelled) Pocket Secretary row is selectable.
    bool pocket_secretary_debug_row_visible() const noexcept {
        return debug_menu_unlocked_;
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
    bool field_barrier_collision_overlay_active_for_dump() const noexcept {
        return field_barrier_collision_overlay_active_;
    }
    const std::array<FieldBarrierCollisionCell,
                     kFieldBarrierCollisionCellCount>&
    field_barrier_collision_overlay_cells_for_dump() const noexcept {
        return field_barrier_collision_overlay_cells_;
    }
    const std::optional<FieldBarrierSpritePlan>&
    field_barrier_sprite_plan_for_dump() const noexcept {
        return field_barrier_sprite_plan_;
    }
    std::uint8_t controller_nibble_for_dump() const noexcept {
        return field_controller_nibble_;
    }
    bool take_requested_title_return() noexcept {
        const bool requested = title_return_requested_;
        title_return_requested_ = false;
        return requested;
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
    void set_field_camera_world_position_for_dump(
        std::uint16_t x, std::uint16_t y) noexcept {
        state_.field_scene_buffers.camera_x = x;
        state_.field_scene_buffers.camera_y = y;
        field_camera_follow_.camera_x = x;
        field_camera_follow_.camera_y = y;
        field_camera_follow_.aligned_x =
            static_cast<std::uint16_t>(x & 0xFF80U);
        field_camera_follow_.aligned_y =
            static_cast<std::uint16_t>(y & 0xFF80U);
        field_camera_alignment_ready_ = true;
    }
    bool set_default_actor_world_position_for_dump(
        std::size_t active_actor_index, std::uint16_t x,
        std::uint16_t y) noexcept;
    bool set_selected_default_actor_world_position_for_dump(
        std::uint16_t x, std::uint16_t y) noexcept;
    bool field_entity_visible_for_dump() const noexcept {
        return field_entity_.visible;
    }
    bool set_field_spell_effect_for_dump(std::uint8_t action_code,
                                         std::uint8_t phase,
                                         bool visible) noexcept;
    std::uint16_t field_entity_screen_x_for_dump() const noexcept {
        return field_entity_.projection.screen_x;
    }
    std::uint16_t field_entity_screen_y_for_dump() const noexcept {
        return field_entity_.projection.screen_y;
    }
    std::uint16_t field_entity_heading_for_dump() const noexcept {
        return field_movement_state_.heading;
    }
#ifdef SHADOWRUN_QT_QA_DRIVER
    void set_field_entity_heading_for_dump(std::uint16_t heading) noexcept {
        field_movement_state_.heading = heading;
        field_movement_state_.animation_heading = heading;
    }
    std::int16_t probe_field_collision_for_dump(
        std::uint16_t x, std::uint16_t y);
#endif // SHADOWRUN_QT_QA_DRIVER
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
    std::optional<std::uint16_t>
    field_weapon_primary_heading_for_dump() const noexcept;
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
    enum class FieldDestinationTransitionRoute : std::uint8_t {
        elevator,
        taxi,
        vid_phone,
        dialogue_scene_entry,
    };
    enum class FieldPartyWipeDialogueStage : std::uint8_t {
        inactive,
        run_failure,
        wake,
    };

    // A route actor list queued by Lua (route_actor_lists.lua data): when
    // the party enters consume_scene, the source_scene activity list
    // pending_list_<list> is published and `transitions` actor behavior
    // transitions are counted down before route_actor_lists.complete_<name>
    // runs. E.g. club_penumbra_mako: scene 17, scene 47 list 0x1E628, 4.
    struct FieldRoutePendingActorList {
        std::uint16_t consume_scene = 0U;
        std::uint16_t source_scene = 0U;
        std::uint32_t list = 0U;
        std::uint8_t transitions = 0U;
        std::string name;
    };

    struct DefaultFieldActorPresentation;
    struct DefaultActorSpriteView;
    FieldInteractionAdvanceResult advance_field_interaction(
        std::uint16_t event_word);
    void install_test_deck();
    void configure_callbacks();
    void configure_runtime_coordinator_callbacks();
    std::optional<scripting::ScriptActorBehaviorDecision>
    heartbeat_field_player(scripting::ScriptActorHeartbeatPhase phase);
    void advance_player_weapon_action_gate();
    void resolve_field_party_wipe();
    void complete_field_party_wipe();
    void start_field_party_wake_transition();
    void advance_field_movement();
    void advance_field_actor_animation();
    void advance_default_field_actor_animations();
    void advance_tarislar_tomb_outro();
    void advance_field_hit_spark_behaviors();
    void advance_field_hit_spark_animations();
    void advance_field_weapon_projectile_animations();
    void advance_field_weapon_projectiles();
    bool apply_script_combat_meter_update(
        const scripting::ScriptCombatMeterUpdate& update,
        std::string_view boundary_prefix = "field.combat.weapon_hit");
    void service_player_weapon_action(bool requested_by_lua);
    void initialize_field_entity_selection_state();
    void refresh_field_runner_state_and_assets();
    void handle_field_mode_action_event();
    void run_field_mode_exit_transition();
    bool run_field_exit_orchestrator(
        integration::FieldRuntimeCoordinatorState& runtime_state);
    bool open_gunderson_interaction();
    bool open_field_johnson_interaction();
    bool open_field_location_dialogue(
        const FieldScriptRoute& route);
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
    void begin_field_modal_return_transition();
    void suppress_field_modal_close_input();
    void handle_field_map_event_activations(
        const std::vector<FieldMapEventActivation>& activations);
    void handle_game_script_task_step(
        const scripting::ScriptTaskStep& step);
    bool prepare_runtime_field_scene(
        std::uint16_t scene_reference,
        std::optional<SceneRunnerPosition> requested_spawn = std::nullopt);
    void dispatch_field_map_scene_entry(std::uint16_t scene_id);
    std::optional<std::size_t> projected_terrain_cell() const;
    bool mark_field_terrain_cell(std::size_t cell);
    void reset_field_map_cell_words();
    bool repaint_field_map_cell(std::size_t cell);
    void rebuild_field_vdp_mirror();
    void stage_field_vdp_frame();
    std::size_t apply_world_effect_layer_writes(
        bool layer_b_target, const std::vector<MapWordWrite>& writes);
    void paint_field_console_background(QPainter& painter) const;
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
        std::uint16_t* terrain_collision_code_output = nullptr,
        bool* terrain_boundary_active_output = nullptr,
        bool* outside_bounds_output = nullptr);
    void set_field_barrier_collision_overlay(
        const std::array<FieldBarrierCollisionCell,
                         kFieldBarrierCollisionCellCount>& cells) noexcept;
    void clear_field_barrier_collision_overlay() noexcept;
    std::uint8_t terrain_collision_map_byte(std::uint16_t tile_index) const
        noexcept;
    void source_boundary(std::string_view boundary) const;
    bool load_image(const ConversionAssetCatalog& catalog,
                    std::string_view alias,
                    QImage& destination);
    bool load_field_weapon_spawn_assets(
        const ConversionAssetCatalog& catalog);
    bool load_field_hit_spark_assets(
        const ConversionAssetCatalog& catalog);
    bool load_field_default_actor_assets(
        const ConversionAssetCatalog& catalog);
    bool load_dynamic_world_actor_artwork(
        const ConversionAssetCatalog& catalog);
    void initialize_default_field_actors(std::uint16_t scene_reference);
    void advance_default_field_actors(bool source_target_list_service);
    void provide_source_profile_patrol_movement_services(
        scripting::ScriptActorBehaviorContext& context,
        DefaultFieldActorPresentation& actor);
    void resolve_default_actor_proximity(
        DefaultFieldActorPresentation& actor);
    void publish_pending_world_activity_script_actors(
        std::uint16_t scene_reference);
    void service_dynamic_actor_weapon_action(
        DefaultFieldActorPresentation& actor,
        const scripting::ScriptActorBehaviorContext& context,
        const scripting::ScriptActorBehaviorDecision& decision);
    bool apply_scripted_field_entity_target_selection(
        const std::optional<std::string>& selected_actor_id);
    void trigger_field_world_effect_sequence();
    void advance_field_alarm(bool direct_raise = false,
                             bool clear_alarm = false,
                             bool disable_alarm = false,
                             bool reset_alarm = false,
                             bool scene_exit = false);
    void apply_field_alarm_result(
        const scripting::ScriptFieldAlarmResult& result,
        std::uint16_t scene_reference);
    bool process_pending_story_world_effect_sequence();
    std::int16_t select_field_world_effect_sequence();
    void spawn_field_world_effect_sequence();
    void reset_field_entity_service_records();
    void refresh_world_entity_service_targets();
    bool probe_action_line_clear(const SpawnEntityParentState& parent,
                                 std::optional<std::size_t> protected_source_slot,
                                 std::uint16_t target_x,
                                 std::uint16_t target_y);
    void service_player_spell_cast(
        const scripting::ScriptActorBehaviorDecision& decision);
    void service_player_melee(
        const scripting::ScriptActorBehaviorDecision& decision);
    void spawn_field_hit_spark(
        const scripting::ScriptActorHitSparkSpawnRequest& request);
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
    void paint_field_scene_entities(QPainter& painter,
                                    const QRect& clip_rect = QRect()) const;
    void paint_field_hud_overlays(QPainter& painter) const;
    FieldHudSlotState field_hud_slot_state(std::size_t slot) const;
    void paint_command_overview_magic(QPainter& painter) const;
    const MagicSpellTable* field_magic_spell_table(
        std::uint16_t table_index) const noexcept;
    void paint_attributes_skills_menu(
        QPainter& painter,
        const AttributesSkillsMenuController& menu) const;
    void paint_command_overview(QPainter& painter) const;
    void paint_command_overview_clips(QPainter& painter) const;
    void paint_command_overview_posture_panel(QPainter& painter) const;
    void paint_field_shop(QPainter& painter) const;
    void paint_field_shop_notice(QPainter& painter) const;
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
    void register_dialogue_outcome_handlers();
    bool dispatch_dialogue_outcome(
        const scripting::DialogueOutcome& outcome);
    bool handle_dialogue_continue_in_place(
        const scripting::ContinueInPlaceOutcome& outcome);
    bool handle_dialogue_play_cinematic(
        const scripting::PlayCinematicOutcome& outcome);
    bool handle_dialogue_resume_source_route(
        const scripting::ResumeSourceRouteOutcome& outcome);
    bool handle_dialogue_enter_field_scene(
        const scripting::EnterFieldSceneOutcome& outcome);
    bool handle_dialogue_return_to_source_entry(
        const scripting::ReturnToOriginOutcome& outcome);
    bool prepare_open_dialogue_service(
        const scripting::OpenServiceOutcome& outcome);
    bool open_hotel_dialogue_service(
        const scripting::OpenServiceOutcome& outcome);
    bool open_hospital_dialogue_service(
        const scripting::OpenServiceOutcome& outcome);
    bool load_command_overview_menu_content();
    bool load_hotel_menu_content();
    bool load_pocket_secretary_menu_content();
    bool load_ares_shop_menu_content();
    bool load_item_menu_content();
    bool load_field_elevator_selector_assets(
        const ConversionAssetCatalog& catalog);
    bool load_field_taxi_selector_assets(
        const ConversionAssetCatalog& catalog);
    bool open_field_elevator_selector(
        FieldCollisionEventCode event_code);
    void advance_field_elevator_selector();
    void advance_field_elevator_scene_transition();
    bool begin_hospital_menu_task(std::string_view function);
    void advance_hospital_grid_effect();
    void paint_hospital_menu(QPainter& painter) const;
    bool transition_to_field_destination(
        std::uint16_t scene_reference,
        FieldDestinationTransitionRoute route,
        std::optional<SceneRunnerPosition> requested_spawn = std::nullopt);
    HotelMenuEvent process_hotel_menu_events(std::uint16_t events);
    AttributesSkillsRuntimeEvent process_hotel_karma_screen_events(
        std::uint16_t events);
    void draw_pocket_secretary_cursor(QPainter& painter, int x, int y) const;
    void draw_command_overview_text(QPainter& painter, int x, int y,
                                    std::string_view text,
                                    bool opaque_palette_pixels = false) const;
    void draw_tinted_field_text(QPainter& painter, int x, int y,
                                std::string_view text, QColor color,
                                bool opaque_palette_pixels = false) const;
    void draw_field_menu_font_text(QPainter& painter, int x, int y,
                                   std::string_view text,
                                   const QImage& font) const;
    void draw_hotel_menu_text(QPainter& painter, int x, int y,
                              std::string_view text,
                              HotelMenuPaletteLine palette_line) const;
    void refresh_command_overview_combat_bars();
    EquipmentCombatInput command_overview_combat_input() const;
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
    void configure_field_menu_route();
    void flush_menu_blank_deferred_sounds();
    void advance_field_menu_route();
    void synchronize_command_overview_return_transition(
        const scripting::ScriptMenuRouteSnapshot& route_state);
    void advance_command_overview_return_transition();
    bool save_game_slot(int slot);
    bool load_game_slot(int slot);
    int game_save_slot_status(int slot) const;
    std::optional<QJsonObject> capture_game_save_snapshot(QString* error);
    bool restore_game_save_snapshot(const QJsonObject& snapshot,
                                    QString* error);
    void synchronize_pocket_secretary_notebook_from_route(
        const scripting::ScriptMenuRouteSnapshot& route_state);
    void synchronize_command_overview_clips_from_route(
        const scripting::ScriptMenuRouteSnapshot& route_state);
    void synchronize_command_overview_inventory_from_route(
        const scripting::ScriptMenuRouteSnapshot& route_state);
    void synchronize_command_overview_magic_from_route(
        const scripting::ScriptMenuRouteSnapshot& route_state);
    void start_command_menu_legacy_screen(
        std::uint16_t screen,
        const scripting::ScriptMenuRouteArgs& args);
    bool command_menu_legacy_screen_active(std::uint16_t screen) const;
    void open_command_overview(
        std::uint16_t root_screen = static_cast<std::uint16_t>(
            srx::MenuScreen::COMMAND),
        scripting::ScriptMenuRouteStartArguments start_arguments = {});
    bool load_command_overview_assets(const ConversionAssetCatalog& catalog);
    bool load_field_hud_overlays(const ConversionAssetCatalog& catalog);

    struct FieldEntityPresentation {
        bool active = false;
        SceneRunnerPosition world_position{};
        EntityProjectionState projection{};
        std::uint8_t direction = 0;
        bool visible = false;
        bool spell_effect_visible = false;
        std::uint8_t spell_effect_action_code = 0U;
        std::uint8_t spell_effect_phase = 0U;
    };

    struct FieldWeaponProjectilePresentation {
        scripting::ScriptCombatantFacts shooter_facts{};
        std::optional<std::size_t> shooter_party_slot;
        std::string shooter_actor_id;
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

    struct FieldHitSparkPresentation {
        ReactionAuxiliaryChild child{};
        std::string actor_instance_id;
        EntityAnimationState animation{};
        bool script_initialized = false;
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
        std::uint8_t source_action_slot = 0;
        std::uint8_t source_action_ammo = 0;
        std::uint8_t source_action_counter = 0;
        std::uint16_t source_animation_selector = 0;
        std::optional<scripting::ScriptActorItemSpawnResult>
            source_item_spawn_result;
        // The static list is rebuilt from its ROM template on scene entry;
        // combatant holds the live, damageable copy for this visit.
        scripting::ScriptCombatantFacts initial_combatant{};
        scripting::ScriptCombatantFacts combatant{};
        std::optional<std::uint8_t> effect_slot;
        bool temporary_world_encounter = false;
        bool source_actor_has_waypoint_data = false;
        // ROM-selected scene script actors share the dynamic actor behavior
        // package, but their lifetime is tied to the selected scene script,
        // not to a random encounter sequence.
        bool world_activity_script_actor = false;
        // Name of the route actor list whose transition this actor owes.
        std::optional<std::string> actor_list_completion_group;
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
        bool spell_effect_visible = false;
        std::uint8_t spell_effect_action_code = 0U;
        std::uint8_t spell_effect_phase = 0U;
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
        std::array<std::vector<QImage>, 8> spell_images{};
        std::array<std::vector<int>, 8> spell_origin_x{};
        std::array<std::vector<int>, 8> spell_origin_y{};
        std::vector<QImage> spell_death_e0_images{};
        std::vector<int> spell_death_e0_origin_x{};
        std::vector<int> spell_death_e0_origin_y{};
        std::vector<QImage> spell_death_e4_images{};
        std::vector<int> spell_death_e4_origin_x{};
        std::vector<int> spell_death_e4_origin_y{};
        std::array<std::vector<QImage>, 8> entry_images{};
        std::array<std::vector<int>, 8> entry_origin_x{};
        std::array<std::vector<int>, 8> entry_origin_y{};
        std::array<std::vector<QImage>, 8> walk_images{};
        std::array<std::vector<int>, 8> walk_origin_x{};
        std::array<std::vector<int>, 8> walk_origin_y{};
        std::array<std::vector<QImage>, 8> action_images{};
        std::array<std::vector<int>, 8> action_origin_x{};
        std::array<std::vector<int>, 8> action_origin_y{};
        std::array<std::vector<QImage>, 8> item_action_images{};
        std::array<std::vector<int>, 8> item_action_origin_x{};
        std::array<std::vector<int>, 8> item_action_origin_y{};
        std::array<std::vector<QImage>, 8> hurt_images{};
        std::array<std::vector<int>, 8> hurt_origin_x{};
        std::array<std::vector<int>, 8> hurt_origin_y{};
    };

    struct CreatureEffectArtwork {
        std::array<QImage, 4> images{};
        std::array<int, 4> origin_x{};
        std::array<int, 4> origin_y{};
    };

    struct SpellEffectArtwork {
        std::array<QImage, 16> images{};
        std::array<int, 16> origin_x{};
        std::array<int, 16> origin_y{};
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
        std::array<std::string, 18> spell_effect_slot_target_ids{};
        EntityServiceTargetOwner entity_target_owner{};
        std::vector<EntityServiceTargetRecord> entity_target_records;
        EntityAnimationState player_animation{};
        std::uint16_t player_animation_selector = 0U;
        std::string player_animation_id = "idle";
        bool player_animation_reset_pending = false;
        bool player_walking = false;
        bool player_attack_pose = false;
        std::uint8_t player_attack_pose_kind = 0U;
        std::uint8_t player_attack_direction = 0U;
        std::uint16_t target_heading = 0U;
        FieldLocationReturnAnchor return_anchor =
            FieldLocationReturnAnchor::none;
        FieldLocationReturnFacing return_facing =
            FieldLocationReturnFacing::preserve_entry_heading;
    };

    struct DefaultActorSpriteView {
        const QImage* image = nullptr;
        int origin_x = 0;
        int origin_y = 0;
    };

    std::function<void(std::string_view)> source_boundary_callback_;
    std::function<void(std::uint16_t)> source_audio_event_callback_;
    std::function<void(std::uint16_t)> source_audio_stop_callback_;
    std::function<void(std::uint16_t)> source_music_callback_;
    std::function<void(std::uint8_t)> source_attenuation_callback_;
    std::function<void()> source_fade_callback_;
    std::vector<std::uint16_t> field_music_requests_;
    std::vector<std::uint8_t> field_attenuation_requests_;
    std::size_t field_fade_requests_ = 0U;
    bool start_menu_duck_active_ = false;
    bool scene_music_latch_ = false; // FFE172
    void service_scene_music_latch();
    void request_scene_field_music();
    void request_audio_attenuation(std::uint8_t level);
    void request_audio_fade();
    void restore_start_menu_duck();
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
    // The ROM disables the display while preparing the command overview.
    // Grok's emulator trace observed 27 black host frames before it returns.
    std::uint8_t command_overview_start_transition_frames_remaining_ = 0U;
    bool command_overview_start_transition_black_frame_ = false;
    std::uint16_t menu_blank_reveal_line_pending_ = 0U;
    // Lua-staged partial frames around a display-off build (E.MenuMask):
    // each entry is the list of black rects for one frame.
    std::vector<std::vector<QRect>> menu_lead_masks_;
    std::vector<std::vector<QRect>> menu_reveal_masks_pending_;
    std::vector<std::vector<QRect>> menu_reveal_masks_;
    mutable QImage menu_last_frame_;
    mutable bool menu_last_frame_is_field_ = false;
    QImage command_overview_return_image_;
    bool menu_window_sidebar_ = false;
    std::uint16_t menu_blank_reveal_line_ = 0U;
    std::vector<std::uint16_t> menu_blank_deferred_sounds_;
    CommandMenuKarmaViewRuntime command_overview_attributes_skills_{};
    HotelKarmaUpgradeRuntime hotel_karma_skills_{};
    std::uint32_t hotel_karma_load_frames_ = 0U;
    std::uint32_t hotel_karma_load_black_frames_ = 0U;
    scripting::GameScriptRuntime game_scripts_;
    GameSaveStore game_save_store_{};
    bool menu_route_active_ = false;
    std::uint16_t menu_route_screen_ = 0U;
    std::optional<scripting::ScriptMenuRouteSnapshot::MagicView>
        menu_route_magic_view_;
    std::optional<scripting::ScriptShopDefinition> field_shop_definition_;
    std::optional<scripting::ScriptShopDefinition>
        field_shop_active_definition_;
    bool field_shop_notice_active_ = false;
    std::string field_shop_notice_service_id_;
    std::string field_shop_notice_text_key_;
    FieldDialogueRuntime field_dialogue_;
    scripting::DialogueOutcomeDispatcher dialogue_outcome_dispatcher_;
    HotelMenuRuntime hotel_menu_{};
    // E660 advances while Hospital Options and its confirmation are open.
    // This state only decorates the screen; Lua owns the cursor and action.
    RandomGridEffectState hospital_random_grid_effect_{};
    std::array<std::int8_t, 12> hospital_random_grid_cells_{};
    bool hospital_service_session_active_ = false;
    HotelMenuContent hotel_menu_content_{};
    PocketSecretaryMenuContent pocket_secretary_menu_content_{};
    FieldModeTextRuntime field_mode_text_{};
    FieldInteractionRuntime field_interaction_;
    ElevatorSelectorRuntime field_elevator_selector_{};
    ElevatorSceneTransitionRuntime field_elevator_scene_transition_{};
    TarislarCreditsTitleReturnRuntime tarislar_credits_title_return_{};
    std::optional<scripting::ScriptElevatorFloorGroup>
        field_elevator_floor_group_;
    std::optional<std::uint16_t> field_elevator_script_destination_;
    std::optional<std::uint16_t> field_elevator_pending_destination_;
    ShopMenuController field_shop_menu_{};
    FieldDialogueEventResolver field_dialogue_event_resolver_;
    FieldPartyRosterRuntime field_party_roster_{};
    std::array<std::optional<MagicSpellTable>, kUsMagicSpellTableCount>
        field_magic_spell_table_overrides_{};
    DismissRunnerMenuState dismiss_runner_{};
    RandomGridEffectState dismiss_runner_random_grid_effect_{};
    std::uint32_t dismiss_runner_effect_frames_ = 0U;
    std::array<std::int8_t, 12> dismiss_runner_random_grid_cells_{
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    std::size_t field_joshua_archetype_selection_ = 0U;
    QImage field_map_;
    QImage field_map_priority_overlay_;
    bool field_vdp_mirror_enabled_ = false;
    bool field_vdp_mirror_qa_override_ = false;
    std::unique_ptr<FieldVdpMirror> field_vdp_mirror_;
    // The extracted map image is a pure function of the (FF2700, FF4700)
    // word pair of each 32x32 cell (checked on every extracted scene). Keep
    // the pair each image cell currently shows, so a changed FF2700 word can
    // be repainted from another cell that shows the same pair.
    std::vector<std::pair<std::uint16_t, std::uint16_t>> field_map_cell_words_;
    std::size_t field_map_cell_columns_ = 0U;
    QImage field_hud_;
    QImage field_hud_active_frame_;
    QImage field_service_background_;
    QImage field_taxi_map_;
    std::array<QImage, 8> field_taxi_cursor_tiles_{};
    std::array<QImage, kElevatorSelectorAnimationFrameCount>
        field_elevator_selector_backgrounds_{};
    std::array<QImage, kElevatorSelectorMaximumFloors>
        field_elevator_selector_current_floor_layers_{};
    std::array<QImage, kElevatorSelectorMaximumFloors>
        field_elevator_selector_selected_floor_layers_{};
    std::array<QImage, 4> field_elevator_selector_feedback_layers_{};
    QImage field_service_universal_clips_icon_;
    std::array<QImage, 6> field_service_tab_images_{};
    std::array<QImage, 8> field_service_cyberware_icons_{};
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
    QImage matrix_map_background_;
    QImage matrix_map_font_;
    std::array<QImage, 4> matrix_cyberdeck_backgrounds_{};
    QImage matrix_cyberdeck_panel_;
    QImage matrix_cyberdeck_runner_detail_headers_;
    std::array<QImage, 4> matrix_cyberdeck_cursors_{};
    QImage matrix_cyberdeck_empty_program_icon_;
    std::array<std::array<QImage, 4>, 5> matrix_node_backgrounds_{};
    std::array<QImage, 4> matrix_node_action_cursors_{};
    std::array<std::array<QImage, 4>, 4> matrix_cyberdeck_selector_rows_{};
    std::array<QImage, 16> matrix_cyberdeck_program_icons_{};
    std::array<QImage, 16> matrix_cyberdeck_detail_strips_{};
    std::array<QImage, 7> matrix_cyberdeck_footer_slot_status_{};
    std::array<QImage, 4> matrix_map_tile_atlases_{};
    std::array<QImage, 4> matrix_traversal_backgrounds_{};
    std::array<std::array<QImage, 5>, 6> matrix_world_models_{};
    std::array<std::vector<QImage>, 9> matrix_traversal_world_effect_modes_{};
    std::vector<QImage> matrix_traversal_world_effect_fallback_;
    QImage matrix_traversal_hud_frame_;
    QImage matrix_traversal_runner_entry_pose_;
    QImage matrix_traversal_runner_action_phase_0_;
    QImage matrix_traversal_runner_action_phase_1_;
    QImage matrix_traversal_runner_idle_;
    std::array<QImage, 4> matrix_traversal_analyze_rings_{};
    std::array<QImage, kMagicSpellTextCount> command_overview_magic_spell_icons_{};
    std::array<QImage, 4> command_overview_magic_cursor_frames_{};
    std::array<QImage, 4> command_overview_magic_header_frames_{};
    std::array<QImage, 5> command_overview_magic_posture_bars_{};
    std::array<QImage, 7> command_overview_magic_force_meter_tiles_{};
    std::array<QImage, 22> command_overview_magic_metric_bars_{};
    std::array<QImage, 4> command_overview_magic_availability_bars_{};
    std::array<QImage, 10> command_overview_magic_damage_bars_{};
    std::array<std::array<QImage, 2>, 50> command_overview_inventory_icons_{};
    std::array<QImage, 50> command_overview_item_details_{};
    std::array<QImage, 14> command_overview_damage_bars_{};
    std::array<QImage, 14> command_overview_drain_bars_{};
    std::array<QImage, 9> command_overview_armor_summary_bars_{};
    std::array<QImage, 40> command_overview_detail_values_{};
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
    // docs/24 R1: aliases the single ROM RNG word FFF9F8 shared with title/menus/Matrix (random_conversion.hpp).
    std::uint16_t& field_random_seed_ = source_random_seed();
    std::uint16_t field_world_frame_counter_ = 0U;
    // ROM FFE11E and FFE170 are transient field alarm state [C].
    std::int16_t field_alarm_timer_ = 0;
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
    MagicDefenseStripView menu_route_magic_physical_defense_{};
    MagicDefenseStripView menu_route_magic_mana_defense_{};
    MagicMenuLoopState command_overview_magic_loop_{};
    MagicForceState command_overview_magic_force_{};
    std::optional<MagicSpellSelection> command_overview_magic_selected_spell_;
    std::uint8_t command_overview_magic_cursor_phase_ = 0U;
    std::uint8_t command_overview_magic_cursor_timer_ = 0U;
    CommandOverviewActionMessage command_overview_action_message_ =
        CommandOverviewActionMessage::none;
    bool command_overview_inventory_action_active_ = false;
    bool command_overview_inventory_lua_route_active_ = false;
    bool command_overview_inventory_selector_suppressed_ = false;
    bool command_overview_inventory_detail_pending_ = false;
    bool command_overview_inventory_cursor_hidden_ = false;
    bool command_overview_inventory_picture_partial_ = false;
    std::optional<int> command_overview_inventory_ammo_count_;
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
    // Temporary development override: expose the ROM's hidden sixth row so
    // the debug screen remains reachable without entering the title code.
    bool debug_menu_unlocked_ = true;
    // FF018F bit 0 is the deck-present fact; application snapshots preserve
    // it along with the typed deck record. A Matrix session is not persisted.
    std::optional<bool> cyberdeck_available_ = false;
    // Five live slot-animation records. The deck record itself is persisted;
    // active Matrix session and per-slot animation timers are not yet saved.
    std::array<scripting::ScriptCyberdeckProgramSlot, 5>
        field_matrix_program_slots_{};
    std::int8_t field_matrix_deck_mode_ = 0;
    // Matrix node actions mutate field/world facts through Lua. The active
    // Matrix run record belongs to the Lua run registry and its save state.
    scripting::ScriptMatrixFieldState field_matrix_field_state_{};
    // FFFB30 is source RAM shared by Matrix sessions. It is transient and is
    // cleared on bootstrap; Lua updates its typed cell records through the
    // Matrix world-state bridge.
    std::optional<scripting::ScriptMatrixMapMemory>
        field_matrix_map_memory_;
    // ROM FFF0CE: shared karma rollover state. New Game clears it, and loaded
    // source state stays unknown until the save adapter restores this byte.
    std::optional<std::int8_t> field_matrix_karma_counter_;
    // ROM FFF0A3: field setup seeds 1 at 0x174FC; Attack and Slow clear it.
    std::optional<std::int8_t> field_matrix_passive_ic_counter_;
    // FFFC28 and the known fields of FFFBBE. The save adapter does not yet
    // restore these source values, so loaded sessions retain unknown state.
    std::optional<std::uint16_t> field_matrix_passcode_flags_;
    std::optional<CyberdeckRecord> field_matrix_deck_record_;
    // FFFC22 is not present in the current save adapter. New Game initializes
    // it to zero; restored saves remain an explicit source-state boundary.
    std::optional<std::uint32_t> group_item_flags_ = 0U;
    // A fresh New Game has no active run. Save restoration does not yet
    // publish the source FFFBEC run state to this presentation adapter.
    bool notebook_current_run_available_ = false;
    std::array<std::uint32_t, 3> notebook_tip_flags_{{1U, 0U, 0U}};
    bool notebook_tip_flags_resolved_ = true;
    // FFFC2C's low 13 bits gate the Contacts page, while its next 13 bits
    // record Vid-Phone calls. FFFC27 and FFFC2A gate Mr. Johnsons and
    // Shadowrunners. Fresh New Game leaves Contacts clear: 0x59C48 is the
    // debug-menu Contacts effect (table 0x1D90C0, entry 6), not New Game
    // setup. A loaded save restores FFFC2C when available.
    std::optional<std::uint32_t> notebook_contacts_flags_ = 0U;
    // FFFBFA is the numeric Lone Star heat level (0..8), read by encounters
    // and cleared by the Vid-Phone record deletion service. Older host saves
    // do not contain it, so restored sessions keep the source value unknown.
    std::optional<std::uint8_t> field_heat_level_ = std::nullopt;
    // FFFBFB advances the Strike Team escalation stage, capped at eight.
    // It is separate from the actor-transition countdown and remains unknown
    // when restored from a save that predates this field.
    std::optional<std::uint8_t> field_strike_team_counter_ = std::nullopt;
    // FFE187 arms the Fuchi cyber-heart safe; FFE191 defers its alarm
    // transition to the next source world-effect refresh. Both are transient.
    std::optional<std::uint8_t> field_fuchi_safe_context_ = std::uint8_t{0U};
    std::optional<std::uint8_t> field_alarm_transition_pending_ =
        std::uint8_t{0U};
    std::optional<std::uint8_t> notebook_referral_flags_ = 0U;
    std::optional<std::uint16_t> notebook_shadowrunner_flags_ = 0U;
    QImage pocket_secretary_background_;
    QImage pocket_secretary_grid_;
    QImage pocket_secretary_nuyen_bar_;
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
    // ROM C0A2 unarmed strike (two frames) and C082 spell pose (one frame).
    std::array<std::array<QImage, 2>, 8> field_entity_strike_images_{};
    std::array<std::array<int, 2>, 8> field_entity_strike_origin_x_{};
    std::array<std::array<int, 2>, 8> field_entity_strike_origin_y_{};
    std::array<QImage, 8> field_entity_cast_images_{};
    std::array<int, 8> field_entity_cast_origin_x_{};
    std::array<int, 8> field_entity_cast_origin_y_{};
    // ROM 0xB330 C0B2 runner-down list: entry 0 then a held entry 1.
    std::array<std::array<QImage, 2>, 8> field_entity_down_images_{};
    std::array<std::array<int, 2>, 8> field_entity_down_origin_x_{};
    std::array<std::array<int, 2>, 8> field_entity_down_origin_y_{};
    // ROM 0xAFF2 C072 party runner hurt list: two finite frames per direction.
    std::array<std::array<QImage, 2>, 8> field_entity_hurt_images_{};
    std::array<std::array<int, 2>, 8> field_entity_hurt_origin_x_{};
    std::array<std::array<int, 2>, 8> field_entity_hurt_origin_y_{};
    // FFE166: 0xABB6 arms 0x3C frames when the whole party is down; 0x1DDC6
    // counts it down and 0xC64C resolves the wipe at zero. Save restore at
    // 0x594E0 copies FFF5BE..FFF805 (end-exclusive FFF806) and does not restore
    // FFE166, so this timer is transient across ROM loads [C]; whether the
    // writer copies it is [H].
    std::uint8_t field_party_wipe_countdown_ = 0;
    bool field_world_effect_player_spawn_pending_ = false; // 0x1DED8 FFFC3C [C]
    EffectFadeMarkerState field_effect_fade_marker_{}; // FFDFDA/FFDFDC [C]
    std::array<DefaultActorArtwork, 2> field_default_actor_artwork_{};
    std::unordered_map<std::string, DefaultActorArtwork>
        field_dynamic_actor_artwork_;
    std::vector<std::array<QImage, 4>>
        field_target_marker_images_by_scene_;
    std::vector<std::array<CreatureEffectArtwork, 8>>
        field_hound_close_attack_images_by_scene_;
    std::vector<std::array<SpellEffectArtwork, 5>>
        field_spell_effect_images_by_scene_;
    std::vector<DefaultFieldActorPresentation> field_default_actors_;
    // The ROM lists hold record pointers and refresh on separate FFDF88
    // phases. Cache source record identity only; actor facts are read live.
    std::vector<std::uint8_t> runner_target_candidate_slots_;
    std::vector<std::uint8_t> world_target_candidate_slots_;
    bool target_candidate_lists_seeded_ = false;
    std::array<bool, 18> field_world_effect_slots_occupied_{};
    std::array<std::string, 18> field_spell_effect_slot_target_ids_{};
    bool field_barrier_collision_overlay_active_ = false;
    std::array<FieldBarrierCollisionCell,
               kFieldBarrierCollisionCellCount>
        field_barrier_collision_overlay_cells_{};
    std::optional<FieldBarrierSpritePlan> field_barrier_sprite_plan_;
    std::optional<FieldBarrierPresentationOrigin>
        field_barrier_presentation_origin_;
    std::array<QImage, kFieldBarrierSpriteFrameCount>
        field_barrier_images_{};
    std::array<int, kFieldBarrierSpriteFrameCount>
        field_barrier_origin_x_{};
    std::array<int, kFieldBarrierSpriteFrameCount>
        field_barrier_origin_y_{};
    EntityServiceTargetOwner field_entity_target_owner_{};
    std::vector<EntityServiceTargetRecord> field_entity_target_records_{16U};
    std::optional<scripting::ScriptFieldEncounterOutcome>
        pending_field_encounter_outcome_;
    // Story world-effect actor list queued by Lua (story_state.lua data),
    // e.g. Boris's Strike Team list 0x1D6FD0 in scene 2 (ROM 0x1F1E0).
    struct PendingStoryWorldEffect {
        std::uint16_t scene = 0U;
        std::uint32_t list = 0U;
        std::uint8_t actor_count = 0U;
        std::string name;
    };
    std::optional<PendingStoryWorldEffect> field_pending_story_world_effect_;
    std::optional<std::uint16_t>
        pending_world_activity_script_actor_scene_;
    std::vector<scripting::ScriptWorldActivityActor>
        pending_world_activity_script_actors_;
    std::array<std::vector<QImage>, 16> field_weapon_projectile_images_{};
    std::array<std::vector<int>, 16> field_weapon_projectile_origin_x_{};
    std::array<std::vector<int>, 16> field_weapon_projectile_origin_y_{};
    std::vector<FieldWeaponProjectilePresentation>
        field_weapon_projectiles_;
    std::array<QImage, 5> field_hit_spark_images_{};
    std::array<int, 5> field_hit_spark_origin_x_{};
    std::array<int, 5> field_hit_spark_origin_y_{};
    std::vector<FieldHitSparkPresentation> field_hit_sparks_;
    // ROM 0x15A8C scans the six shared child slots 9..14 used by projectile
    // records and hit sparks. These are separate from field_entity_table_.
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
    // Which list the active +0x16 pose came from: 0 firearm C042,
    // 1 unarmed strike C0A2, 2 spell C082.
    std::uint8_t field_entity_attack_pose_kind_ = 0U;
    std::uint8_t field_entity_attack_direction_ = 0U;
    bool field_combat_input_suppressed_until_release_ = false;
    std::array<MovementOffset, integration::kFieldMovementDirectionCount>
        field_movement_direction_offsets_{};
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
    FieldLocationReturnAnchor pending_field_return_anchor_ =
        FieldLocationReturnAnchor::none;
    FieldLocationReturnFacing pending_field_return_facing_ =
        FieldLocationReturnFacing::preserve_entry_heading;
    // The ROM blocks inside selected area-event entry dialogues. The Qt modal
    // yields across frames, so retain the typed destination and resume its
    // scene-entry tail only after the player dismisses the entry page.
    bool field_mode_entry_transition_pending_ = false;
    std::optional<scripting::ScriptRunFieldEventDecision>
        field_mode_entry_run_target_actor_group_;
    bool field_matrix_route_active_ = false;
    bool field_taxi_route_active_ = false;
    bool matrix_return_to_field_on_close_ = false;
    bool matrix_field_return_transition_pending_ = false;
    bool field_taxi_transition_active_ = false;
    std::optional<std::uint16_t> field_taxi_pending_destination_;
    std::optional<std::uint16_t> field_taxi_cabbie_destination_;
    bool field_phone_transition_active_ = false;
    std::optional<std::uint16_t> field_phone_pending_destination_;
    bool field_dialogue_scene_transition_active_ = false;
    std::optional<std::uint16_t>
        field_dialogue_scene_pending_destination_;
    std::optional<SceneRunnerPosition> field_dialogue_scene_pending_spawn_;
    FieldPartyWipeDialogueStage field_party_wipe_dialogue_stage_ =
        FieldPartyWipeDialogueStage::inactive;
    std::optional<std::uint16_t> field_mode_entry_destination_scene_;
    std::optional<std::uint16_t> field_pending_mode_transition_scene_;
    std::optional<SceneRunnerPosition> field_pending_mode_transition_spawn_;
    std::optional<FieldModePendingActorList>
        field_pending_mode_transition_actor_list_;
    std::optional<FieldRoutePendingActorList>
        field_pending_route_actor_list_;
    std::optional<FieldRoutePendingActorList>
        field_pending_actor_list_transition_group_;
    std::optional<std::uint8_t>
        field_route_actor_transitions_remaining_;
    // Route actor list whose transitions are being counted down.
    std::string field_active_route_actor_list_;
    std::optional<std::string> field_pending_actor_list_callback_;
    std::int8_t field_indexed_title_service_result_ = 0;
    // FFDD64: 0x53A78 stores the index of the trigger service it dispatches.
    // 0x2196C sets that bit of FFFC1E (field_exit_world_flags_). Lua reads
    // it as story_trigger_active_<n>; empty until a trigger is dispatched.
    std::optional<std::uint8_t> field_active_story_trigger_;
    std::uint8_t field_palette_fade_steps_ = 0U;
    std::uint8_t field_modal_return_transition_frames_remaining_ = 0U;
    bool command_overview_return_transition_active_ = false;
    std::uint16_t command_overview_return_transition_elapsed_frames_ = 0U;
    std::uint8_t command_overview_return_palette_step_ = 0U;
    scripting::ScriptFieldReturnTransitionPhase
        command_overview_return_transition_phase_ =
            scripting::ScriptFieldReturnTransitionPhase::none;
    // Source return point captured from the ROM-generated 0xBC36 position
    // triplet (FFDF44..4F) and camera snapshot (FFDF50/52).
    struct FieldDoorReturnPoint {
        bool valid = false;
        SceneRunnerPosition lead{};
        // FFDF44..4E: lead, runner 2, runner 3.
        std::array<SceneRunnerPosition, 3> party{};
        std::uint16_t heading = 0U;
        std::uint16_t camera_x = 0U;
        std::uint16_t camera_y = 0U;
    };
    FieldDoorReturnPoint field_door_return_point_{};
    SceneRunnerPosition field_frame_start_player_position_{};
    SceneRunnerPosition field_pre_step_player_position_{};
    std::uint32_t field_door_return_frame_ = 0U;
    bool field_door_return_active_ = false;
    std::uint32_t checkout_fade_out_start_frame_ = 0U;
    std::uint32_t checkout_fade_in_start_frame_ = 0U;
    std::uint32_t checkout_input_resume_frame_ = 0U;
    std::uint32_t checkout_camera_settle_start_frame_ = 0U;
    mutable QImage field_door_return_modal_frame_;
    void save_field_door_return_point();
    void record_field_door_contact(
        const std::array<std::uint8_t, 3>& probe_bytes);
    std::uint16_t field_door_exit_heading(std::uint16_t entry_heading);
    void begin_field_door_return();
    void place_party_at_field_door_return_point();
    bool apply_field_service_close_return();
    bool advance_field_door_return();
    std::uint8_t field_door_return_palette_level() const noexcept;
    std::uint8_t field_modal_return_blocked_inputs_ = 0U;
    bool field_world_activity_scene_entry_pending_ = false;
    WorldActivityState field_world_activity_state_{};
    WorldActivityTimeoutState field_world_activity_timeout_state_{};
    WorldActivityRoutineState world_activity_routine_state_{};
    WorldActivityRoutineRequestState world_activity_request_state_{};
    WorldActivityStartState world_activity_request_start_state_{};
    std::optional<WorldActivityReturnSnapshot>
        world_activity_return_snapshot_;
    bool field_think_token_claimed_ = false; // FFE175, one claim per frame [C]
    // FFE188: boot RAM clear (0xC3A/0xC58) leaves it 0; bootstrap seeds 0 [C].
    std::optional<std::uint8_t> field_thon_period_counter_;
    MovementModeTransitionContinuation field_activity_continuation_ =
        MovementModeTransitionContinuation::none;
    // FFE12C selects a floor label on the successful Security Terminal file
    // branch. It is sampled by the live world-activity dispatcher.
    std::optional<std::uint8_t> field_security_floor_index_;
    std::uint8_t field_projected_terrain_mark_count_ = 0;
    std::array<std::uint8_t, 4> field_terrain_sample_state_{};
    bool field_terrain_probe_trace_enabled_ = false;
    std::vector<FieldTerrainProbeTrace> field_terrain_probe_trace_;
    // Current outputs of the most recent source 0xE7C8 terrain probe.
    bool field_terrain_boundary_active_ = false;
    bool field_terrain_outside_bounds_marker_ = false;
    // FFDFE4. Index of the FF1700 collision byte selected by the last 0xE8A0
    // sample. 0x1651E clears that byte; 0xE9EE may leak A2, but the index is
    // only defined for the collision table.
    std::uint16_t field_collision_marker_index_ = 0;
    FieldCameraFollowState field_camera_follow_{};
    bool field_camera_alignment_ready_ = false;
    // FFE114 is a pending typed scene-entry placement published by map or
    // dialogue events. 0x2B34 consumes it on the next full scene entry.
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
    // Typed counterpart for source FFFC30, separate from FFFC1E and FFFC34.
    std::uint32_t field_mode_flags_ = 1U;
    // Old host saves omit FFFC30; do not expose its New Game seed as saved data.
    bool field_mode_flags_resolved_ = true;
    // Typed counterparts for the field story-service registers. These remain
    // session state until the save adapter restores their source facts.
    std::uint32_t field_mode_effect_flags_ = 0;
    // FFE182 selects the ROM's pending entity-list handoff. It is transient
    // field state and is not part of the persistent story/save flags.
    bool field_pending_world_effect_ = false;
    // FFE180 suppresses pending-list selection while a higher-priority
    // world effect is active.
    bool field_world_effect_active_ = false;
    // ROM FFFC40: separate world-activity facts, including Spirit Eyes
    // component-request markers at bits 0..2.
    std::uint32_t field_auxiliary_flags_ = 0;
    // FFFC27 bit 6 records the first Vigore/Jarl booth introduction.
    std::optional<std::uint8_t> field_dialogue_register_{0U};
    // ROM FFFC26: per-dialogue branch gates used by indexed title services.
    std::optional<std::uint8_t> field_dialogue_flags_{0U};
    std::uint32_t field_render_effect_flags_ = 0;
    std::optional<std::uint32_t> field_funds_{0x14U};
    // ROM FFE168 bit 0; set by debug-menu option 0x59C54.
    bool field_walk_fast_enabled_ = false;
    // The fresh New Game runtime initializes these typed source globals to
    // zero. The loaded-save adapter does not restore them, so paid writes stay
    // explicit for that path.
    std::optional<std::uint8_t> field_runner_feature_flags_{0U}; // FF018F
    std::optional<std::array<std::uint8_t, 3>>
        field_entity_action_phase_roll_counters_{std::array<std::uint8_t, 3>{}};
    std::string field_shop_route_pending_service_id_;
    FieldEntityPresentation field_entity_{};
    std::string failure_;
    bool ready_ = false;
    bool title_return_requested_ = false;
#ifdef SHADOWRUN_QT_QA_DRIVER
    std::optional<std::uint16_t> field_scene_reference_override_for_dump_;
#endif
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
