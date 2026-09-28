#pragma once

#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_state_conversion.hpp"
#include "shadowrun_ghidra/entities/collision_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_camera_conversion.hpp"
#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"
#include "shadowrun_ghidra/field/field_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_projection_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_probe_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_route_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_step_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_mode_transition_conversion.hpp"
#include "shadowrun_ghidra/field/terrain_probe_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_exit_transition_finalizer_conversion.hpp"
#include "shadowrun_ghidra/field/field_runner_state_asset_refresh_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_bootstrap_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"
#include "shadowrun_ghidra/session/runner_switch_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"
#include "shadowrun_ghidra/menus/menu_transfer_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_table_conversion.hpp"
#include "shadowrun_ghidra/world/world_entity_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"
#include "shadowrun_ghidra/world/world_scene_entity_conversion.hpp"

#include <QImage>

#include <array>
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
    void paint(QPainter& painter) const;

    bool ready() const noexcept { return ready_; }
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
    void advance_field_movement();
    void initialize_field_entity_selection_state();
    void refresh_field_runner_state_and_assets();
    void handle_field_mode_action_event();
    void run_field_mode_exit_transition();
    bool project_field_world_interaction();
    void update_source_field_camera();
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
    WorldFrameState world_frame_state_;
    WorldFrameCallbacks world_frame_callbacks_;
    FieldViewportState viewport_state_;
    QImage field_map_;
    QImage field_hud_;
    std::array<QImage, 8> field_entity_idle_images_{};
    std::array<int, 8> field_entity_idle_origin_x_{};
    std::array<int, 8> field_entity_idle_origin_y_{};
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
    std::uint16_t field_target_heading_ = 0;
    // Typed source field corresponding to FF014C, consumed only by the
    // 0xC7F8 scene-runtime dispatcher.
    std::uint8_t field_action_counter_ = 0;
    std::uint8_t field_entity_behavior_flags_ = 0;
    FieldEntityPresentation field_entity_{};
    std::string failure_;
    bool ready_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
