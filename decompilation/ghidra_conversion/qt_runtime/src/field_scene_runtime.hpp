#pragma once

#include "shadowrun_ghidra/core/controller_wait_conversion.hpp"
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
#include "shadowrun_ghidra/entities/entity_projection_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_route_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_step_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_mode_transition_conversion.hpp"
#include "shadowrun_ghidra/field/terrain_probe_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_exit_transition_finalizer_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_orchestrator_conversion.hpp"
#include "shadowrun_ghidra/field/field_exit_prompt_conversion.hpp"
#include "shadowrun_ghidra/field/field_runner_state_asset_refresh_conversion.hpp"
#include "shadowrun_ghidra/integration/field_runtime_coordinator_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_bootstrap_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"
#include "shadowrun_ghidra/menus/inventory_action_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_confirmation_conversion.hpp"
#include "shadowrun_ghidra/session/runner_switch_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"
#include "shadowrun_ghidra/world/world_entity_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"
#include "shadowrun_ghidra/world/world_frame_logic_conversion.hpp"
#include "shadowrun_ghidra/world/world_scene_entity_conversion.hpp"

#include <QImage>

#include "field_command_overview_bridge.hpp"
#include "field_clips_menu_runtime.hpp"
#include "field_party_roster_runtime.hpp"
#include "field_dialogue_runtime.hpp"
#include "field_menu_text_catalog.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

// Composition-only adapter for the separate Ghidra-conversion Qt executable.
// It owns no ROM view and does not duplicate converted field state. The
// conversion bootstrap remains authoritative; this class supplies host
// presentation callbacks and paints the direct source assets it receives.
class FieldSceneRuntime final {
public:
    explicit FieldSceneRuntime(
        std::function<void(std::string_view)> source_boundary);

    bool bootstrap(
        const integration::StartupIntegrationState& startup_state);
    void advance_frame();
    void set_controller_nibble(std::uint8_t dpad_nibble) noexcept;
    void set_controller_port_byte(std::uint8_t port_byte) noexcept;
    void paint(QPainter& painter) const;
    bool open_command_overview_for_dump();

    bool command_overview_open() const noexcept {
        return command_overview_.open();
    }
    bool dialogue_open() const noexcept { return field_dialogue_.open(); }
    const DialoguePanelView& dialogue_panel() const noexcept {
        return field_dialogue_.panel();
    }
    bool open_stoker_dialogue_for_dump(
        StokerMotelPromptId prompt) {
        return field_dialogue_.open_stoker_prompt_for_dump(prompt);
    }
    const FieldCommandOverviewView& command_overview_view() const noexcept {
        return command_overview_.view();
    }
    RunnerRecordFields& active_runner_record() noexcept {
        return field_party_roster_.active_record();
    }
    const RunnerRecordFields& active_runner_record() const noexcept {
        return field_party_roster_.active_record();
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
    bool discard_confirmation_open() const noexcept {
        return command_overview_discard_confirmation_active_;
    }
    bool discard_confirmation_yes() const noexcept {
        return command_overview_discard_confirmation_.selected_option == 0U;
    }
    bool clips_menu_open() const noexcept {
        return command_overview_clips_.is_open();
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
    bool field_entity_visible() const noexcept {
        return field_entity_.visible;
    }
    const std::string& failure() const noexcept { return failure_; }
    const integration::FieldSessionBootstrapState& state() const noexcept {
        return state_;
    }
    const SceneRunnerPosition& field_entity_world_position() const noexcept {
        return field_entity_.world_position;
    }

private:
    void configure_callbacks();
    void configure_runtime_coordinator_callbacks();
    void advance_field_movement();
    void initialize_field_entity_selection_state();
    void refresh_field_runner_state_and_assets();
    void handle_field_mode_action_event();
    void run_field_mode_exit_transition();
    bool run_field_exit_orchestrator(
        integration::FieldRuntimeCoordinatorState& runtime_state);
    bool project_field_world_interaction();
    void update_source_field_camera();
    void apply_field_entity_animation(bool walking);
    bool recover_blocked_field_movement(
        const MovementProbeResult& initial_probe);
    std::optional<std::size_t> projected_terrain_cell() const;
    void record_field_activity_start(
        MovementModeTransitionContinuation continuation);
    std::int16_t evaluate_field_collision(std::uint16_t candidate_x,
                                          std::uint16_t candidate_y);
    void source_boundary(std::string_view boundary) const;
    bool load_image(const ConversionAssetCatalog& catalog,
                    std::string_view alias,
                    QImage& destination);
    bool load_sprite_image(const ConversionAssetCatalog& catalog,
                           std::string_view alias,
                           QImage& destination,
                           int& origin_x,
                           int& origin_y);
    void paint_field_entity(QPainter& painter) const;
    void paint_field_hud_overlays(QPainter& painter) const;
    void paint_command_overview(QPainter& painter) const;
    void paint_command_overview_clips(QPainter& painter) const;
    void draw_command_overview_text(QPainter& painter, int x, int y,
                                    std::string_view text) const;
    void refresh_command_overview_combat_bars();
    void refresh_command_overview_item_detail();
    void refresh_field_hud_runner_slot();
    void initialize_field_party_roster(
        const integration::StartupIntegrationState& startup_state);
    void advance_command_overview_inventory_action(
        std::uint16_t controller_events);
    void advance_command_overview_clips(std::uint16_t controller_events);
    void begin_command_overview_inventory_action();
    void begin_command_overview_clips();
    void dispatch_command_overview_inventory_action(
        const InventoryActionResult& result);
    void advance_command_overview_discard_confirmation(
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

    std::function<void(std::string_view)> source_boundary_callback_;
    integration::FieldSessionBootstrapInput input_;
    integration::FieldSessionBootstrapState state_;
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
    FieldDialogueRuntime field_dialogue_;
    FieldPartyRosterRuntime field_party_roster_{};
    std::size_t field_joshua_archetype_selection_ = 0U;
    QImage field_map_;
    QImage field_hud_;
    QImage command_overview_background_;
    QImage command_overview_inventory_boxes_;
    QImage command_overview_font_;
    QImage command_overview_cursor_;
    FieldMenuTextCatalog field_menu_text_{};
    std::array<QImage, 3> command_overview_portraits_{};
    std::array<QImage, 5> command_overview_focus_{};
    std::array<QImage, 5> command_overview_status_bars_{};
    std::array<QImage, 2> command_overview_selected_cells_{};
    QImage command_overview_action_background_;
    std::array<QImage, 4> command_overview_action_focus_{};
    std::array<QImage, 4> command_overview_clips_focus_{};
    std::array<QImage, 4> command_overview_clips_trade_focus_{};
    std::array<std::array<QImage, 2>, 50> command_overview_inventory_icons_{};
    std::array<QImage, 50> command_overview_item_details_{};
    std::array<QImage, 14> command_overview_damage_bars_{};
    std::array<QImage, 14> command_overview_drain_bars_{};
    std::array<QImage, 256> command_overview_detail_values_{};
    std::array<QImage, 23> command_overview_attack_bars_{};
    std::array<QImage, 23> command_overview_defense_bars_{};
    ResourceVariantState command_overview_resource_variant_{};
    EquipmentCombatBarResult command_overview_combat_bars_{};
    ItemActionDetailResult command_overview_item_detail_{};
    InventoryActionMenuState command_overview_inventory_action_{};
    std::optional<SelectedInventoryItem>
        command_overview_selected_inventory_item_;
    MenuConfirmationState command_overview_discard_confirmation_{};
    FieldClipsMenuRuntime command_overview_clips_{};
    std::string command_overview_action_message_;
    bool command_overview_inventory_action_active_ = false;
    bool command_overview_discard_confirmation_active_ = false;
    std::array<QImage, kFieldHudSlotCount> field_hud_faces_{};
    std::array<QImage, kFieldHudSlotCount> field_hud_items_{};
    std::array<QImage, kFieldHudSlotCount> field_hud_unarmed_items_{};
    std::array<QImage, kFieldHudSlotCount> field_hud_vertical_counters_{};
    std::array<QImage, kFieldHudSlotCount> field_hud_horizontal_counters_{};
    std::array<QImage, kFieldHudAmmoCounterCount> field_hud_ammo_counters_{};
    std::array<FieldHudSlotState, kFieldHudSlotCount> field_hud_slots_{};
    std::array<QImage, 8> field_entity_idle_images_{};
    std::array<int, 8> field_entity_idle_origin_x_{};
    std::array<int, 8> field_entity_idle_origin_y_{};
    // The 0xC0B2 walking lists carry an 0x8008 marker (eight frames per
    // direction); the hurt lists have two and the attack pose one. Keep
    // each direction's own frame count instead of forcing a fixed width.
    std::array<std::vector<QImage>, 8> field_entity_walk_images_{};
    std::array<std::vector<int>, 8> field_entity_walk_origin_x_{};
    std::array<std::vector<int>, 8> field_entity_walk_origin_y_{};
    EntityAnimationState field_entity_animation_{};
    std::uint16_t field_entity_animation_selector_ = 0;
    bool field_entity_walking_ = false;
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
    WorldActivityState field_world_activity_state_{};
    MovementModeTransitionContinuation field_activity_continuation_ =
        MovementModeTransitionContinuation::none;
    std::uint8_t field_projected_terrain_mark_count_ = 0;
    bool field_terrain_render_dirty_ = false;
    std::array<std::uint8_t, 4> field_terrain_sample_state_{};
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
    // Typed source field corresponding to FF014C, consumed only by the
    // 0xC7F8 scene-runtime dispatcher.
    std::uint8_t field_action_counter_ = 0;
    std::uint8_t field_entity_behavior_flags_ = 0;
    std::int8_t field_exit_alternate_result_ = 0;
    std::uint32_t field_exit_world_flags_ = 0;
    FieldEntityPresentation field_entity_{};
    std::string failure_;
    bool ready_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
