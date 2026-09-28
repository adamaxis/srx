#include "field_scene_runtime.hpp"

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/integration/field_session_source_assets_conversion.hpp"

#include <QPainter>
#include <QColor>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::string_view kInitialFieldMapAlias = "field.maps.mode.0";
constexpr std::string_view kFieldHudAlias = "field.hud.background";
constexpr std::string_view kFieldEntityIdleAliasPrefix = "field.entities.idle.";
constexpr int kGenesisSpriteOrigin = 0x80;

std::array<std::uint8_t, 4> field_sample_state_from_result(
    const std::uint32_t packed_state) {
    return {
        static_cast<std::uint8_t>(packed_state >> 24U),
        static_cast<std::uint8_t>(packed_state >> 16U),
        static_cast<std::uint8_t>(packed_state >> 8U),
        static_cast<std::uint8_t>(packed_state)};
}

} // namespace

FieldSceneRuntime::FieldSceneRuntime(
    std::function<void(std::string_view)> source_boundary)
    : source_boundary_callback_(std::move(source_boundary)) {
    configure_callbacks();
}

void FieldSceneRuntime::source_boundary(
    const std::string_view boundary) const {
    if (source_boundary_callback_) {
        source_boundary_callback_(boundary);
    }
}

bool FieldSceneRuntime::load_image(
    const ConversionAssetCatalog& catalog,
    const std::string_view alias,
    QImage& destination) {
    const auto* record = catalog.find(std::string(alias));
    if (record == nullptr) {
        failure_ = "missing direct field asset alias: " + std::string(alias);
        return false;
    }
    destination = QImage(QString::fromStdWString(
        record->absolute_path.wstring()));
    if (destination.isNull()) {
        failure_ = "cannot load direct field asset alias: " +
                   std::string(alias);
        return false;
    }
    destination = destination.convertToFormat(
        QImage::Format_ARGB32_Premultiplied);
    return true;
}

bool FieldSceneRuntime::load_sprite_image(
    const ConversionAssetCatalog& catalog,
    const std::string_view alias,
    QImage& destination,
    int& origin_x,
    int& origin_y) {
    const auto* record = catalog.find(std::string(alias));
    if (record == nullptr) {
        failure_ = "missing direct field sprite alias: " + std::string(alias);
        return false;
    }
    if (!load_image(catalog, alias, destination)) {
        return false;
    }
    origin_x = record->origin_x;
    origin_y = record->origin_y;

    // The extracted Genesis color index zero is the sprite transparency
    // value. The direct BMP keeps the RGB palette, so restore that source
    // transparency at the presentation boundary rather than painting a
    // black rectangle behind every runner.
    for (int y = 0; y < destination.height(); ++y) {
        for (int x = 0; x < destination.width(); ++x) {
            auto pixel = destination.pixelColor(x, y);
            if (pixel.red() == 0 && pixel.green() == 0 &&
                pixel.blue() == 0) {
                pixel.setAlpha(0);
                destination.setPixelColor(x, y, pixel);
            }
        }
    }
    return true;
}

void FieldSceneRuntime::configure_callbacks() {
    callbacks_.mode_entry_audio.run_fixed_audio_transaction =
        [this](const std::uint32_t command) {
            source_boundary("field.audio.mode_entry.command");
            (void)command;
        };
    callbacks_.mode_entry_audio.wait_for_vblank_frames =
        [this](const std::uint16_t counter) {
            source_boundary("field.audio.mode_entry.wait");
            (void)counter;
        };
    callbacks_.mode_entry_audio.flush_audio_bus = [this] {
        source_boundary("field.audio.mode_entry.flush");
    };
    callbacks_.mode_entry_audio.service_audio_tick = [this] {
        source_boundary("field.audio.mode_entry.service");
    };

    callbacks_.initialize_entity_order = [this] {
        source_boundary("field.entities.initialize_order");
        state_.render_entities[0].active = field_entity_.active;
        state_.render_entities[0].id = 0U;
    };
    callbacks_.initialize_scene_profile = [this] {
        source_boundary("field.scene.initialize_profile");
    };

    callbacks_.field_resources.prepare_asset_setup = [this] {
        source_boundary("field.resources.prepare_asset_setup");
    };
    callbacks_.field_resources.upload_asset_sequence = [this] {
        source_boundary("field.resources.upload_asset_sequence");
    };
    callbacks_.field_resources.refresh_animation_assets = [this] {
        source_boundary("field.resources.refresh_animation_assets");
    };
    callbacks_.field_resources.load_font = [this] {
        source_boundary("field.resources.load_font");
    };

    world_frame_callbacks_.service_audio = [this] {
        source_boundary("field.world.service_audio");
    };
    world_frame_callbacks_.reset_effect_record = [this] {
        source_boundary("field.world.reset_effect_record");
    };
    world_frame_callbacks_.begin_effect_transition = [this] {
        source_boundary("field.world.begin_effect_transition");
    };
    world_frame_callbacks_.clear_effect_slots = [this] {
        source_boundary("field.world.clear_effect_slots");
    };
    world_frame_callbacks_.clear_entity_effects = [this] {
        source_boundary("field.world.clear_entity_effects");
    };
    world_frame_callbacks_.clear_scene_pending = [this] {
        source_boundary("field.world.clear_scene_pending");
    };
    world_frame_callbacks_.finalize_entity_templates = [this] {
        source_boundary("field.world.finalize_entity_templates");
    };
    world_frame_callbacks_.service_scene_effect = [this] {
        source_boundary("field.world.service_scene_effect");
    };
    world_frame_callbacks_.copy_terrain_layers = [this] {
        source_boundary("field.world.copy_terrain_layers");
    };
    world_frame_callbacks_.process_terrain_entries = [this] {
        source_boundary("field.world.process_terrain_entries");
    };
    world_frame_callbacks_.rebuild_collision_map = [this] {
        source_boundary("field.world.rebuild_collision_map");
    };
    world_frame_callbacks_.load_scene_resources = [this] {
        source_boundary("field.world.load_scene_resources");
    };
    world_frame_callbacks_.clear_scene_transitions = [this] {
        source_boundary("field.world.clear_scene_transitions");
    };
    world_frame_callbacks_.populate_entities = [this] {
        source_boundary("field.world.populate_entities");
    };
    world_frame_callbacks_.dispatch_entity_actions = [this] {
        source_boundary("field.world.dispatch_entity_actions");
    };
    world_frame_callbacks_.decode_entity_data = [this] {
        source_boundary("field.world.decode_entity_data");
    };
    world_frame_callbacks_.mark_entities_for_render = [this] {
        source_boundary("field.world.mark_entities_for_render");
    };
    callbacks_.run_world_frame_services = [this] {
        run_world_frame_services(world_frame_state_, world_frame_callbacks_);
    };

    callbacks_.apply_scene_profile_selection = [this] {
        source_boundary("field.scene.apply_profile_selection");
    };
    callbacks_.field_viewports.prepare_scene_resources = [this] {
        source_boundary("field.viewport.prepare_scene_resources");
    };
    callbacks_.field_viewports.refresh_mode_state = [this] {
        source_boundary("field.viewport.refresh_mode_state");
    };
    callbacks_.field_viewports.special_effects_enabled = [] {
        return false;
    };
    callbacks_.field_viewports.service_special_effects = [this] {
        source_boundary("field.viewport.service_special_effects");
    };
    callbacks_.field_viewports.render_normal_viewport = [this] {
        render_normal_field_viewport(
            viewport_state_,
            [this] { source_boundary("field.viewport.render_normal"); });
    };
    callbacks_.field_viewports.render_alternate_viewport = [this] {
        render_alternate_field_viewport(
            viewport_state_,
            [this] { source_boundary("field.viewport.render_alternate"); });
    };
    callbacks_.upload_scene_profile_block = [this] {
        source_boundary("field.scene.upload_profile_block");
    };
    callbacks_.guard_world_effect_player_spawn = [this] {
        source_boundary("field.world.guard_player_spawn");
    };

    callbacks_.entity_behavior_frames.dispatch_entity_action_candidates =
        [this] { source_boundary("field.entities.dispatch_action_candidates"); };
    callbacks_.entity_behavior_frames.noop_entity_action_hook =
        [](EntityBehaviorFrameRecord& record) { return record.action_result; };
    const auto observe_entity_boundary =
        [this](const std::string_view boundary,
               EntityBehaviorFrameRecord& record) {
            source_boundary(boundary);
            (void)record;
        };
    callbacks_.entity_behavior_frames.complete_entity_behavior_transition =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.complete_transition", record);
        };
    callbacks_.entity_behavior_frames.clear_effect_slot_state =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.clear_effect_slot", record);
        };
    callbacks_.entity_behavior_frames.handle_mode_action_event =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.handle_action_event", record);
        };
    callbacks_.entity_behavior_frames.advance_entity_behavior_action =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.advance_action", record);
        };
    callbacks_.entity_behavior_frames.enter_entity_behavior_transition =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.enter_transition", record);
        };
    callbacks_.entity_behavior_frames.dispatch_entity_collision_behavior =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.dispatch_collision", record);
        };
    callbacks_.entity_behavior_frames.apply_mode_five_boundary_motion =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.apply_boundary_motion", record);
        };
    callbacks_.entity_behavior_frames.resolve_entity_proximity_interactions =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.resolve_proximity", record);
        };
    callbacks_.entity_behavior_frames.clear_entity_effect_if_forced =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.clear_forced_effect", record);
        };
    callbacks_.entity_behavior_frames.resolve_entity_fallback_movement =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.resolve_fallback", record);
        };
    callbacks_.entity_behavior_frames.run_entity_continuation =
        [observe_entity_boundary](auto& record) {
            observe_entity_boundary("field.entities.run_continuation", record);
        };

    callbacks_.update_field_camera = [this] {
        source_boundary("field.camera.update");
    };
    callbacks_.entity_render_pass.project_screen_position =
        [this](RenderEntityState& entity) {
            source_boundary("field.entities.project_screen_position");
            if (!field_entity_.active || entity.id != 0U) {
                field_entity_.visible = false;
                return;
            }
            field_entity_.projection.world_x =
                field_entity_.world_position.x;
            field_entity_.projection.world_y =
                field_entity_.world_position.y;
            const auto projection = project_entity_screen_position_standard(
                field_entity_.projection,
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_x),
                static_cast<std::uint16_t>(
                    state_.field_scene_buffers.camera_y));
            field_entity_.visible = projection.visible;
        };
    callbacks_.entity_render_pass.advance_animation =
        [this](RenderEntityState& entity) {
            source_boundary("field.entities.advance_animation");
            (void)entity;
        };
    callbacks_.entity_render_pass.finish_entity =
        [this](RenderEntityState& entity) {
            source_boundary("field.entities.finish_entity");
            (void)entity;
        };
    callbacks_.entity_render_pass.finish_pass = [this] {
        source_boundary("field.entities.finish_render_pass");
    };
    callbacks_.wait_for_vblank = [this] {
        source_boundary("field.timing.wait_for_vblank");
    };
    callbacks_.advance_title_palette_fade = [this] {
        source_boundary("field.palette.advance_transition");
    };
    callbacks_.sample_controller_input = [this] {
        source_boundary("field.controller.sample");
    };
    callbacks_.phase_changed = [this](
                                  const integration::FieldSessionBootstrapPhase phase) {
        source_boundary(std::string("field.bootstrap.phase.") +
                        integration::field_session_bootstrap_phase_name(phase));
    };
}

bool FieldSceneRuntime::bootstrap(
    const integration::StartupIntegrationState& startup_state) {
    ready_ = false;
    failure_.clear();
    state_ = integration::FieldSessionBootstrapState{};
    world_frame_state_ = WorldFrameState{};
    viewport_state_ = FieldViewportState{};

    if (startup_state.restored_field_state.has_value()) {
        failure_ =
            "loaded-save field scene composition remains a source boundary";
        source_boundary("field.bootstrap.loaded_save_unresolved");
        return false;
    }
    if (!startup_state.field_session_source_assets.has_value()) {
        failure_ = "field-session source records are unavailable";
        source_boundary("field.bootstrap.source_records_missing");
        return false;
    }

    const auto& assets = *startup_state.field_session_source_assets;
    field_movement_direction_offsets_ = assets.field_movement_direction_offsets;
    field_controller_heading_table_ = assets.field_controller_heading_table;
    field_movement_route_sources_ = assets.field_movement_routes;
    for (std::size_t index = 0; index < field_movement_routes_.size();
         ++index) {
        const auto& source_route = assets.field_movement_routes[index];
        field_movement_routes_[index] = DirectionalMovementRoute{
            0,
            0,
            source_route.primary_fallback_slot,
            source_route.secondary_fallback_slot};
    }
    field_controller_nibble_ = 0U;
    field_target_heading_ = 0U;
    field_movement_state_ = EntityMovementStepState{};
    field_collision_record_ = EntityCollisionRecord{};
    field_terrain_sample_state_ = {};
    field_entity_source_flags_ = 0U;
    input_ = integration::FieldSessionBootstrapInput{};
    input_.action_selection_profile_source =
        assets.action_selection_profile_source;
    input_.field_mode_selection_state_source =
        assets.field_mode_selection_state_source;
    input_.field_scene =
        integration::make_initial_mode2_field_scene_input(assets);

    field_entity_ = FieldEntityPresentation{};
    field_entity_.active = startup_state.joshua_selection.has_value();
    field_entity_.world_position =
        input_.field_scene.source_metadata.initial_field_records[0];
    if (field_entity_.active) {
        field_entity_.projection.sprite_offset_y =
            startup_state.joshua_selection->typed_live_joshua_record
                .projection_word_08;
        field_entity_source_flags_ = static_cast<std::uint8_t>(
            startup_state.joshua_selection->typed_live_joshua_record.flags);
    }

    if (!load_image(startup_state.asset_catalog.catalog,
                    kInitialFieldMapAlias, field_map_) ||
        !load_image(startup_state.asset_catalog.catalog, kFieldHudAlias,
                    field_hud_)) {
        source_boundary("field.bootstrap.direct_assets_missing");
        return false;
    }
    for (std::size_t direction = 0;
         direction < field_entity_idle_images_.size(); ++direction) {
        const auto alias = std::string(kFieldEntityIdleAliasPrefix) +
                           std::to_string(direction);
        if (!load_sprite_image(startup_state.asset_catalog.catalog, alias,
                               field_entity_idle_images_[direction],
                               field_entity_idle_origin_x_[direction],
                               field_entity_idle_origin_y_[direction])) {
            source_boundary("field.bootstrap.entity_assets_missing");
            return false;
        }
    }

    const auto result = integration::bootstrap_field_session(
        input_, state_, callbacks_);
    if (result != integration::FieldSessionBootstrapResult::field_ready) {
        failure_ = std::string("field bootstrap did not reach field_ready: ") +
                   integration::field_session_bootstrap_phase_name(
                       state_.phase);
        source_boundary("field.bootstrap.incomplete");
        return false;
    }

    field_movement_state_.position_x = field_entity_.world_position.x;
    field_movement_state_.position_y = field_entity_.world_position.y;
    field_movement_state_.heading = 0U;
    field_movement_state_.movement_flags = field_entity_source_flags_;
    field_entity_.direction =
        field_movement_direction_from_heading(field_movement_state_.heading);

    ready_ = true;
    source_boundary("field.bootstrap.ready");
    return true;
}

void FieldSceneRuntime::set_controller_nibble(
    const std::uint8_t dpad_nibble) noexcept {
    field_controller_nibble_ = static_cast<std::uint8_t>(dpad_nibble & 0x0FU);
}

std::int16_t FieldSceneRuntime::evaluate_field_collision(
    const std::uint16_t candidate_x,
    const std::uint16_t candidate_y) {
    const auto collision_evaluator =
        [this](const std::uint16_t evaluated_x,
               const std::uint16_t evaluated_y) {
            const auto& metadata = state_.field_scene_buffers.source_metadata;
            const TerrainCollisionBounds bounds{
                metadata.terrain_x_inclusive_lower,
                metadata.terrain_x_exclusive_upper,
                metadata.terrain_y_inclusive_lower,
                metadata.terrain_y_exclusive_upper};
            const auto result = evaluate_terrain_collision(
                TerrainCollisionInput{evaluated_x,
                                      evaluated_y,
                                      field_entity_source_flags_,
                                      bounds,
                                      field_terrain_sample_state_},
                TerrainCollisionCallbacks{
                    [this](const std::uint16_t normalized_x,
                           const std::uint16_t normalized_y,
                           const bool expanded_probe) {
                        return sample_terrain_collision_tiles(
                            normalized_x, normalized_y, expanded_probe,
                            [this](const std::uint16_t tile_index) {
                                const auto& collision_map =
                                    state_.field_scene_buffers.source_metadata
                                        .terrain_collision_map;
                                if (tile_index >= collision_map.size()) {
                                    return static_cast<std::uint8_t>(0x7FU);
                                }
                                return collision_map[tile_index];
                            });
                    },
                    [this](const std::uint8_t object_id,
                           const std::int16_t object_x,
                           const std::int16_t object_y) {
                        const auto& rectangles =
                            state_.field_scene_buffers.source_metadata
                                .terrain_collision_rectangles;
                        if (object_id >= rectangles.size()) {
                            source_boundary(
                                "field.movement.terrain_object_record_missing");
                            return true;
                        }
                        const auto object_lookup =
                            [&rectangles](const std::uint16_t lookup_id) {
                                if (lookup_id >= rectangles.size()) {
                                    return TerrainObjectBounds{};
                                }
                                const auto& record = rectangles[lookup_id];
                                return TerrainObjectBounds{
                                    static_cast<std::int16_t>(record.left),
                                    static_cast<std::int16_t>(record.right),
                                    static_cast<std::int16_t>(record.top),
                                    static_cast<std::int16_t>(record.bottom)};
                            };
                        return contains_terrain_object_bounds(
                            object_id, object_x, object_y, object_lookup);
                    },
                    {}});
            field_terrain_sample_state_ = field_sample_state_from_result(
                result.packed_sample_state);
            return CollisionEvaluation{
                result.code, result.packed_sample_state};
        };
    return evaluate_entity_collision(field_collision_record_, candidate_x,
                                     candidate_y, collision_evaluator);
}

void FieldSceneRuntime::advance_field_movement() {
    if (!ready_ || !field_entity_.active || field_controller_nibble_ == 0U) {
        return;
    }

    const auto input = decode_field_movement_input(
        field_controller_nibble_, field_controller_heading_table_);
    if (!input.valid) {
        return;
    }

    field_target_heading_ = input.target_heading;
    EntityCoordinateTarget heading{
        field_movement_state_.heading, field_target_heading_};
    approach_entity_fixed_point_target(heading);
    field_movement_state_.heading = heading.current;

    const auto probe = [this](const std::uint16_t direction_slot) {
        const auto direction = static_cast<std::uint8_t>(
            (direction_slot >> 1U) & 0x07U);
        return probe_entity_directional_collision(
            MovementProbeInput{
                field_movement_state_.position_x,
                field_movement_state_.position_y,
                direction,
                field_entity_source_flags_},
            field_movement_direction_offsets_,
            [this](const std::uint16_t candidate_x,
                   const std::uint16_t candidate_y) {
                return evaluate_field_collision(candidate_x, candidate_y);
            });
    };
    const auto policy_probe = [probe](const std::uint16_t direction_slot) {
        return probe(direction_slot).collision_code;
    };

    const auto previous_x = field_movement_state_.position_x;
    const auto previous_y = field_movement_state_.position_y;
    const MovementResultPolicy result_policy{
        false,
        state_.field_scene_buffers.scene_reference,
        state_.field_scene_buffers.field_resource_mode};
    advance_entity_movement_step(
        field_movement_state_,
        0U,
        input.direction_slot,
        result_policy,
        probe,
        [this] {
            source_boundary("field.movement.negative_collision_recovery");
            return false;
        },
        [this, &result_policy, &policy_probe] {
            const auto route_lookup = [this](const std::uint16_t route_key) {
                if ((route_key & 0x03U) != 0U ||
                    route_key / 4U >= field_movement_routes_.size()) {
                    source_boundary("field.movement.route_key_out_of_range");
                    return DirectionalMovementRoute{};
                }
                auto route = field_movement_routes_[route_key / 4U];
                const auto& source_route =
                    field_movement_route_sources_[route_key / 4U];
                const auto sample_value = [this](
                                              const std::uint8_t sample_index) {
                    if (sample_index >= field_terrain_sample_state_.size()) {
                        source_boundary(
                            "field.movement.route_sample_out_of_range");
                        return static_cast<std::int8_t>(-1);
                    }
                    return static_cast<std::int8_t>(
                        field_terrain_sample_state_[sample_index]);
                };
                route.primary_marker =
                    sample_value(source_route.primary_sample_index);
                route.secondary_marker =
                    sample_value(source_route.secondary_sample_index);
                return route;
            };
            return evaluate_entity_movement_route(
                field_movement_state_.heading, result_policy, route_lookup,
                policy_probe);
        },
        policy_probe,
        [this] { source_boundary("field.movement.reset_blocked_step"); });

    field_entity_.world_position.x = field_movement_state_.position_x;
    field_entity_.world_position.y = field_movement_state_.position_y;
    field_entity_.direction =
        field_movement_direction_from_heading(field_movement_state_.heading);
    if (previous_x != field_movement_state_.position_x ||
        previous_y != field_movement_state_.position_y) {
        source_boundary("field.movement.accepted_step");
    }
}

void FieldSceneRuntime::advance_frame() {
    if (!ready_) {
        return;
    }
    advance_field_movement();
    update_entity_render_pass(state_.render_entities,
                              callbacks_.entity_render_pass);
}

void FieldSceneRuntime::paint_field_entity(QPainter& painter) const {
    if (!field_entity_.active || !field_entity_.visible) {
        return;
    }
    const auto direction = static_cast<std::size_t>(
        field_entity_.direction % field_entity_idle_images_.size());
    const auto& image = field_entity_idle_images_[direction];
    const auto x = static_cast<int>(field_entity_.projection.screen_x) -
                   kGenesisSpriteOrigin +
                   field_entity_idle_origin_x_[direction];
    const auto y = static_cast<int>(field_entity_.projection.screen_y) -
                   kGenesisSpriteOrigin +
                   field_entity_idle_origin_y_[direction];
    painter.drawImage(x, y, image);
}

void FieldSceneRuntime::paint(QPainter& painter) const {
    painter.fillRect(0, 0, 320, 224, QColor(4, 8, 20));
    if (!ready_ || field_map_.isNull()) {
        return;
    }

    const auto& scene = state_.field_scene_buffers;
    const auto camera_x = std::max(
        0, static_cast<int>(scene.camera_x >> 4U));
    const auto camera_y = std::max(
        0, static_cast<int>(scene.camera_y >> 4U));
    const auto max_x = std::max(0, field_map_.width() - 320);
    const auto max_y = std::max(0, field_map_.height() - 224);
    const auto viewport_x = std::min(camera_x, max_x);
    const auto viewport_y = std::min(camera_y, max_y);
    painter.drawImage(-viewport_x, -viewport_y, field_map_);
    paint_field_entity(painter);

    // This is the source HUD plate at 0x334C/0x39F2, not a Qt-drawn
    // substitute. Dynamic faces/items remain separate source callbacks until
    // their runner-record consumers are connected.
    painter.drawImage(0, 0, field_hud_);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
