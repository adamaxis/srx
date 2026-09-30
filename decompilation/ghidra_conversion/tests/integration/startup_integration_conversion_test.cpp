#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#ifndef SHADOWRUN_CONVERSION_ASSET_ROOT
#error "The startup integration test needs the direct asset root"
#endif

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::integration;

struct Observations {
    std::vector<std::string> callbacks;
    std::vector<StartupIntegrationPhase> phases;
    std::size_t field_ready_count = 0;
    bool resource_work_area_zeroed = false;
    JoshuaArchetypeChoice initialized_archetype =
        JoshuaArchetypeChoice::samurai;
    std::vector<OpeningIntroSpriteGroup> opening_sprite_groups;
};

void record(Observations& observations, const char* callback) {
    observations.callbacks.emplace_back(callback);
}

void write_big_endian_word(std::vector<std::uint8_t>& bytes,
                           const std::size_t offset,
                           const std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value >> 8U);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

StartupIntegrationCallbacks make_callbacks(
    StartupIntegrationState& state,
    Observations& observations) {
    StartupIntegrationCallbacks callbacks{};

    callbacks.initialize_display = [&observations] {
        record(observations, "display");
    };
    callbacks.vdp_ready = {
        [&observations](const std::uint32_t) {
            record(observations, "vdp_warmup");
        },
        [&observations] { record(observations, "vdp_ready"); },
        [&observations](const std::uint32_t) {
            record(observations, "vdp_settle");
        },
    };

    callbacks.pre_title.resources.clear_plane_a = [&observations] {
        record(observations, "clear_plane_a");
    };
    callbacks.pre_title.resources.clear_plane_b = [&observations] {
        record(observations, "clear_plane_b");
    };
    callbacks.pre_title.resources.prepare_title_resources = [&observations] {
        record(observations, "prepare_title_resources");
    };
    callbacks.pre_title.resources.write_resource_stream =
        [&observations](const std::array<
                            std::uint16_t,
                            kPreTitleResourceWorkWordCount>& words) {
            observations.resource_work_area_zeroed = std::all_of(
                words.begin(), words.end(),
                [](const std::uint16_t word) { return word == 0U; });
            record(observations, "write_resource_stream");
        };
    callbacks.pre_title.resources.refresh_field_resource_streams =
        [&observations] { record(observations, "refresh_field_resources"); };
    callbacks.pre_title.resources.decode_and_upload_source_stream =
        [&observations] { record(observations, "decode_source_stream"); };
    callbacks.pre_title.resources.advance_title_palette_fade =
        [&observations] { record(observations, "title_palette_fade"); };
    callbacks.pre_title.resources.write_pre_title_grid = [&observations](const auto&) {
        record(observations, "write_vdp_grid");
    };
    callbacks.pre_title.resources.reset_vdp_upload_state = [&observations] {
        record(observations, "reset_vdp_upload_state");
    };
    callbacks.pre_title.wait_for_vblank = [&state] {
        wait_for_vblank(state.frame, [](const std::uint16_t previous) {
            return static_cast<std::uint16_t>(previous + 1U);
        });
    };
    callbacks.pre_title.animation.build_title_sprite_records =
        [&observations] { record(observations, "build_title_sprites"); };
    callbacks.pre_title.animation.publish_logo_strobe_phase =
        [&observations](const std::uint8_t) {
            record(observations, "publish_logo_strobe");
        };
    callbacks.pre_title.animation.step_entity_color_words = [&observations] {
        record(observations, "step_entity_colors");
    };
    callbacks.pre_title.write_horizontal_scroll_stream =
        [&observations](const std::array<
                            std::uint16_t,
                            kPreTitleHorizontalScrollLines>&) {
            record(observations, "write_horizontal_scroll");
        };
    callbacks.pre_title.service_audio_variant = [&observations] {
        record(observations, "service_audio_variant");
    };
    callbacks.pre_title.service_audio = [&observations] {
        record(observations, "service_audio");
    };
    callbacks.pre_title.sample_controller = [&observations] {
        record(observations, "sample_controller");
    };
    callbacks.pre_title.step_palette_transition = [&observations] {
        record(observations, "step_palette_transition");
    };

    callbacks.title_setup.prepare_scene_resources = [&observations] {
        record(observations, "prepare_title_scene");
    };
    callbacks.title_setup.load_title_resources = [&observations] {
        record(observations, "load_title_resources");
    };
    callbacks.title_setup.load_title_font = [&observations] {
        record(observations, "load_title_font");
    };
    callbacks.title_setup.advance_palette_frames =
        [&observations](const std::uint16_t) {
            record(observations, "advance_title_palette");
        };

    callbacks.start_options.copy_start_options_resource_metadata =
        [&observations] { record(observations, "copy_start_options_metadata"); };
    callbacks.start_options.refresh_start_options_resource_metadata =
        [&observations] {
            record(observations, "refresh_start_options_metadata");
        };
    callbacks.start_options.transfer_start_options_menu_tiles =
        [&observations] { record(observations, "transfer_start_options_tiles"); };
    callbacks.start_options.load_startup_menu_font = [&observations] {
        record(observations, "load_startup_menu_font");
    };
    callbacks.start_options.draw_new_game_load_game_grid = [&observations] {
        record(observations, "draw_new_game_load_game_grid");
    };
    callbacks.start_options.draw_start_options_title = [&observations] {
        record(observations, "draw_start_options_title");
    };
    callbacks.start_options.draw_start_options_cursor = [&observations](
                                                               const StartOptionsChoice) {
        record(observations, "draw_start_options_cursor");
    };

    callbacks.save_load.draw_save_load_grid = [&observations] {
        record(observations, "draw_save_load_grid");
    };
    callbacks.save_load.draw_save_load_title = [&observations] {
        record(observations, "draw_save_load_title");
    };
    callbacks.save_load.draw_save_load_cursor = [&observations](
                                                     const SaveSlotChoice) {
        record(observations, "draw_save_load_cursor");
    };
    callbacks.save_load.try_load_save_slot = [&observations](
                                                 const SaveSlotChoice) {
        record(observations, "try_load_save_slot");
        return SaveLoadAttempt::empty_or_invalid;
    };
    callbacks.save_load.draw_save_load_message = [&observations](
                                                    const SaveLoadMessage) {
        record(observations, "draw_save_load_message");
    };
    callbacks.save_load.wait_for_controller_event = [&observations] {
        record(observations, "wait_for_controller_event");
    };

    callbacks.joshua_archetype.draw_joshua_archetype_grid = [&observations] {
        record(observations, "draw_joshua_archetype_grid");
    };
    callbacks.joshua_archetype.draw_joshua_archetype_title = [&observations] {
        record(observations, "draw_joshua_archetype_title");
    };
    callbacks.joshua_archetype.draw_joshua_archetype_cursor =
        [&observations](const JoshuaArchetypeChoice) {
            record(observations, "draw_joshua_archetype_cursor");
        };
    callbacks.joshua_archetype.preview_joshua_archetype_resources =
        [&observations](const JoshuaArchetypeChoice) {
            record(observations, "preview_joshua_archetype");
        };
    callbacks.joshua_archetype.initialize_joshua_archetype =
        [&observations](const JoshuaArchetypeChoice choice) {
            observations.initialized_archetype = choice;
            record(observations, "initialize_joshua_archetype");
        };

    callbacks.opening_intro.wait_for_vblank = [&state] {
        wait_for_vblank(state.frame, [](const std::uint16_t previous) {
            return static_cast<std::uint16_t>(previous + 1U);
        });
    };
    callbacks.opening_intro.sample_controller = [&observations] {
        record(observations, "opening_sample_controller");
    };
    callbacks.opening_intro.latch_controller_events = [&observations] {
        record(observations, "opening_latch_controller_events");
    };
    callbacks.opening_intro.prepare_mode_entry = [&observations] {
        record(observations, "opening_prepare_mode_entry");
    };
    callbacks.opening_intro.service_audio_command = [&observations](
                                                        const std::uint16_t) {
        record(observations, "opening_service_audio_command");
    };
    callbacks.opening_intro.service_audio = [&observations] {
        record(observations, "opening_service_audio");
    };
    callbacks.opening_intro.step_palette_transition = [&observations] {
        record(observations, "opening_step_palette_transition");
    };
    callbacks.opening_intro.enter_post_intro_mode = [&observations] {
        record(observations, "opening_enter_post_intro_mode");
    };
    callbacks.opening_intro.stage_changed = [&observations](
                                                const OpeningIntroStage stage) {
        observations.callbacks.emplace_back(
            std::string("opening_stage:") + opening_intro_stage_name(stage));
    };
    callbacks.opening_intro.scene_changed = [&observations](
                                                const OpeningIntroScene scene) {
        observations.callbacks.emplace_back(
            std::string("opening_scene:") + opening_intro_scene_name(scene));
    };
    callbacks.opening_intro.draw_sprite =
        [&observations](const OpeningIntroSpriteRequest& request) {
            observations.opening_sprite_groups.push_back(
                opening_intro_sprite_group(request.sprite_descriptor));
        };
    callbacks.opening_intro.source_boundary = [&observations](
                                                  const std::string_view boundary) {
        observations.callbacks.emplace_back(boundary);
    };
    callbacks.opening_intro.stop_requested = [] { return false; };

    callbacks.phase_changed = [&observations](const StartupIntegrationPhase phase) {
        observations.phases.push_back(phase);
    };
    callbacks.field_ready = [&observations] { ++observations.field_ready_count; };
    return callbacks;
}

bool has_event(const StartupIntegrationState& state, const char* event) {
    return std::find(state.event_log.begin(), state.event_log.end(), event) !=
           state.event_log.end();
}

} // namespace

int main() {
    const std::filesystem::path asset_root{
        SHADOWRUN_CONVERSION_ASSET_ROOT};

    StartupIntegrationState load_state{};
    Observations load_observations;
    const auto load_callbacks =
        make_callbacks(load_state, load_observations);
    if (start_startup_integration(asset_root, load_state, load_callbacks) !=
            StartupIntegrationStartResult::title_ready ||
        load_state.phase != StartupIntegrationPhase::title_ready ||
        !load_state.pre_title_complete || !load_state.boot.display_ready ||
        !load_state.boot.title_ready || load_state.boot.vblank_count != 458U ||
        !load_state.startup_records.has_value() ||
        !load_state.field_session_source_assets.has_value() ||
        load_state.startup_records->joshua_archetypes.size() != 3U ||
        !load_observations.resource_work_area_zeroed ||
        !has_event(load_state, "assets.verified") ||
        !has_event(load_state, "field_session_source_records.verified") ||
        !has_event(load_state, "boot.title_ready") ||
        !has_event(load_state, "phase.title_ready")) {
        return 1;
    }

    // These are small source anchors, not a second data format. They make
    // the integration test fail if the named files are replaced by arbitrary
    // same-sized records: the first ROM archetype is the Samurai template,
    // and the roster header begins with the source "GAME" signature.
    const auto& samurai =
        load_state.startup_records->joshua_archetypes[0].record;
    const auto& roster_header =
        load_state.startup_records->save_roster_header;
    if (samurai[0] != 0x07U || samurai[1] != 0x80U ||
        samurai[0x0EU] != 0x00U || samurai[0x0FU] != 0x03U ||
        samurai[0x5BU] != static_cast<std::uint8_t>('J') ||
        samurai[0x5CU] != static_cast<std::uint8_t>('o') ||
        samurai[0x5DU] != static_cast<std::uint8_t>('s') ||
        roster_header[4] != static_cast<std::uint8_t>('G') ||
        roster_header[5] != static_cast<std::uint8_t>('A') ||
        roster_header[6] != static_cast<std::uint8_t>('M') ||
        roster_header[7] != static_cast<std::uint8_t>('E')) {
        return 9;
    }

    if (enter_start_options_integration(load_state, load_callbacks) !=
            StartupIntegrationEvent::entered_start_options ||
        load_state.phase != StartupIntegrationPhase::start_options ||
        load_state.start_options.selected != StartOptionsChoice::new_game ||
        !load_state.save_roster_buffer.has_value() ||
        load_state.save_roster_buffer->compact_records.size() !=
            kSaveRosterTemplateCount ||
        !has_event(load_state, "save_roster.initialized")) {
        return 2;
    }
    if (advance_startup_integration(
            load_state, kStartupMenuDown, load_callbacks) !=
            StartupIntegrationEvent::selection_moved ||
        load_state.start_options.selected != StartOptionsChoice::load_game) {
        return 3;
    }
    if (advance_startup_integration(load_state, kStartupMenuA, load_callbacks) !=
            StartupIntegrationEvent::entered_save_load ||
        load_state.phase != StartupIntegrationPhase::save_load ||
        load_state.save_load.selected != SaveSlotChoice::slot_one) {
        return 4;
    }
    if (advance_startup_integration(load_state, kStartupMenuA, load_callbacks) !=
            StartupIntegrationEvent::no_save_data ||
        load_state.save_load.message != SaveLoadMessage::no_save_data ||
        load_state.save_load.load_flag_published ||
        load_state.phase != StartupIntegrationPhase::save_load) {
        return 5;
    }
    if (advance_startup_integration(load_state, kStartupMenuA, load_callbacks) !=
            StartupIntegrationEvent::returned_to_start_options ||
        load_state.phase != StartupIntegrationPhase::start_options) {
        return 16;
    }

    // A menu callback reporting "loaded" is not enough to enter the field:
    // the converted 0x594E0 restoration path must have a validated source
    // image. This guards the central layer against the old false-success path.
    StartupIntegrationState missing_restore_state{};
    Observations missing_restore_observations;
    auto missing_restore_callbacks =
        make_callbacks(missing_restore_state, missing_restore_observations);
    missing_restore_callbacks.save_load.try_load_save_slot =
        [](const SaveSlotChoice) { return SaveLoadAttempt::loaded; };
    if (start_startup_integration(
            asset_root, missing_restore_state, missing_restore_callbacks) !=
            StartupIntegrationStartResult::title_ready ||
        enter_start_options_integration(
            missing_restore_state, missing_restore_callbacks) !=
            StartupIntegrationEvent::entered_start_options ||
        advance_startup_integration(
            missing_restore_state, kStartupMenuDown,
            missing_restore_callbacks) !=
            StartupIntegrationEvent::selection_moved ||
        advance_startup_integration(
            missing_restore_state, kStartupMenuA,
            missing_restore_callbacks) !=
            StartupIntegrationEvent::entered_save_load ||
        advance_startup_integration(
            missing_restore_state, kStartupMenuA,
            missing_restore_callbacks) !=
            StartupIntegrationEvent::callback_unresolved ||
        missing_restore_state.phase != StartupIntegrationPhase::failed ||
        !has_event(missing_restore_state, "save_field_restore.missing")) {
        return 10;
    }

    StartupIntegrationState restored_state{};
    Observations restored_observations;
    auto restored_callbacks = make_callbacks(restored_state, restored_observations);
    restored_callbacks.save_load.try_load_save_slot =
        [](const SaveSlotChoice) { return SaveLoadAttempt::loaded; };
    LoadedSaveRestoreInput restored_input{};
    restored_input.source_work_buffer.assign(kSaveFieldImageSize, 0U);
    const auto saved_payload = kSaveFieldSourceOffset;
    write_big_endian_word(
        restored_input.source_work_buffer,
        saved_payload + kSaveFieldPayloadSize, 0x0014U);
    restored_input.source_work_buffer[saved_payload + 0x212U] = 1U;
    restored_input.source_work_buffer[saved_payload + 0x213U] = 0xFFU;
    write_big_endian_word(
        restored_input.source_work_buffer,
        saved_payload + kSaveFieldPayloadSize + 2U, 0x1234U);
    write_big_endian_word(
        restored_input.source_work_buffer,
        saved_payload + kSaveFieldPayloadSize + 4U, 0x5678U);
    restored_callbacks.read_loaded_save_for_restore =
        [restored_input](const SaveSlotChoice) { return restored_input; };
    if (start_startup_integration(
            asset_root, restored_state, restored_callbacks) !=
            StartupIntegrationStartResult::title_ready ||
        enter_start_options_integration(
            restored_state, restored_callbacks) !=
            StartupIntegrationEvent::entered_start_options ||
        advance_startup_integration(
            restored_state, kStartupMenuDown, restored_callbacks) !=
            StartupIntegrationEvent::selection_moved ||
        advance_startup_integration(
            restored_state, kStartupMenuA, restored_callbacks) !=
            StartupIntegrationEvent::entered_save_load ||
        advance_startup_integration(
            restored_state, kStartupMenuA, restored_callbacks) !=
            StartupIntegrationEvent::save_loaded ||
        restored_state.phase != StartupIntegrationPhase::field_ready ||
        !restored_state.restored_field_state.has_value() ||
        !restored_state.save_field_restore_result.has_value() ||
        restored_state.save_field_restore_result->action !=
            SaveFieldRestoreAction::restored ||
        restored_state.restored_field_state->world_mode != 0x0014U ||
        restored_state.restored_field_state->camera_x != 0x1234U ||
        restored_state.restored_field_state->camera_y != 0x5678U ||
        !has_event(restored_state, "save_field_restore.restored")) {
        return 11;
    }

    StartupIntegrationState new_game_state{};
    Observations new_game_observations;
    const auto new_game_callbacks =
        make_callbacks(new_game_state, new_game_observations);
    if (start_startup_integration(
            asset_root, new_game_state, new_game_callbacks) !=
            StartupIntegrationStartResult::title_ready ||
        enter_start_options_integration(
            new_game_state, new_game_callbacks) !=
            StartupIntegrationEvent::entered_start_options ||
        advance_startup_integration(
            new_game_state, kStartupMenuA, new_game_callbacks) !=
            StartupIntegrationEvent::entered_joshua_archetype ||
        new_game_state.phase != StartupIntegrationPhase::joshua_archetype) {
        return 6;
    }
    if (advance_startup_integration(
            new_game_state, kStartupMenuDown, new_game_callbacks) !=
            StartupIntegrationEvent::selection_moved ||
        new_game_state.joshua_archetype.selected !=
            JoshuaArchetypeChoice::decker) {
        return 7;
    }
    if (advance_startup_integration(
            new_game_state, kStartupMenuA, new_game_callbacks) !=
            StartupIntegrationEvent::opening_started ||
        new_game_state.phase != StartupIntegrationPhase::opening_intro ||
        new_game_observations.initialized_archetype !=
            JoshuaArchetypeChoice::decker ||
        !new_game_state.joshua_selection.has_value() ||
        new_game_state.joshua_selection->live_joshua_record !=
            new_game_state.startup_records->joshua_archetypes[1].record ||
        new_game_state.joshua_selection->record_selection_markers !=
            std::array<std::uint16_t, 3>{0U, 1U, 2U} ||
        !has_event(new_game_state, "joshua_archetype.initialized") ||
        !has_event(new_game_state, "opening_intro.started")) {
        return 8;
    }

    const auto& opening_plan = opening_intro_source_plan();
    const auto timed_phase_0 = opening_intro_sprite_group(
        opening_plan.stage_timed_phases[0].sprite_descriptor);
    const auto timed_phase_1 = opening_intro_sprite_group(
        opening_plan.stage_timed_phases[1].sprite_descriptor);
    const auto timed_phase_2 = opening_intro_sprite_group(
        opening_plan.stage_timed_phases[2].sprite_descriptor);
    if (opening_plan.stage_timed_phases.size() != 3U ||
        timed_phase_0.family != OpeningIntroSpriteFamily::trivid_timed ||
        timed_phase_0.variant != 0U || timed_phase_1.variant != 1U ||
        timed_phase_2.variant != 2U ||
        opening_plan.stage_timed_phases[0].x != 0x0128U ||
        opening_plan.stage_timed_phases[0].y != 0x0113U ||
        opening_plan.stage_timed_phases[1].x != 0x011FU ||
        opening_plan.stage_timed_phases[1].y != 0x0129U ||
        opening_plan.stage_timed_phases[2].x != 0x0113U ||
        opening_plan.stage_timed_phases[2].y != 0x0145U ||
        opening_plan.stage_timed_phases[0].delay_frames != 6U ||
        opening_plan.stage_timed_phases[1].delay_frames != 6U ||
        opening_plan.stage_timed_phases[2].delay_frames != 6U) {
        return 9;
    }

    for (const auto* alias : {"intro.sprites.trivid.timed.0.0",
                              "intro.sprites.trivid.overlay.0.0",
                              "intro.sprites.trivid.final.overlay.0.0"}) {
        if (new_game_state.asset_catalog.catalog.find(alias) == nullptr) {
            return 14;
        }
    }
    const auto expect_origin = [&new_game_state](const char* alias,
                                                  const int x, const int y) {
        const auto* record = new_game_state.asset_catalog.catalog.find(alias);
        return record != nullptr && record->origin_x == x && record->origin_y == y;
    };
    if (!expect_origin("intro.sprites.trivid.timed.0.0", 120, 128) ||
        !expect_origin("intro.sprites.trivid.overlay.0.0", 4, 96) ||
        !expect_origin("intro.sprites.trivid.final.overlay.0.0", 24, 120)) {
        return 10;
    }

    // New Game now follows the complete converted 0x59C5E opening chain. It
    // must not silently jump from the archetype menu to a synthetic field
    // screen or stop after the first title/tri-vid boundary.
    bool opening_completed = false;
    for (std::size_t frame = 0; frame < 5000U; ++frame) {
        const auto event = advance_startup_integration(
            new_game_state, 0U, new_game_callbacks);
        if (event == StartupIntegrationEvent::field_ready) {
            opening_completed = true;
            break;
        }
        if (event != StartupIntegrationEvent::opening_frame_advanced) {
            return 12;
        }
    }
    if (!opening_completed || new_game_state.phase !=
                                  StartupIntegrationPhase::field_ready ||
        new_game_observations.field_ready_count != 1U ||
        !has_event(new_game_state, "opening_intro.complete") ||
        !has_event(new_game_state, "phase.field_ready") ||
        new_game_state.opening_intro.orchestrator_loop_frame != 0x0115U ||
        new_game_state.opening_intro.stage_loop_frame != 0x0529U ||
        new_game_state.opening_intro.scene != OpeningIntroScene::cab_map ||
        new_game_state.opening_intro.post_intro_frame !=
            kPostIntroCabTotalFrames ||
        new_game_state.opening_intro.post_intro_mode != 2U ||
        new_game_state.opening_intro.post_intro_phase != 4U) {
        return 13;
    }
    const auto has_sprite_family =
        [&new_game_observations](const OpeningIntroSpriteFamily family) {
            return std::any_of(
                new_game_observations.opening_sprite_groups.begin(),
                new_game_observations.opening_sprite_groups.end(),
                [family](const OpeningIntroSpriteGroup group) {
                    return group.family == family;
                });
        };
    if (!has_sprite_family(OpeningIntroSpriteFamily::trivid_timed) ||
        !has_sprite_family(OpeningIntroSpriteFamily::trivid_overlay) ||
        !has_sprite_family(OpeningIntroSpriteFamily::trivid_final_overlay) ||
        !has_sprite_family(OpeningIntroSpriteFamily::ambush_actor) ||
        !has_sprite_family(OpeningIntroSpriteFamily::ambush_body) ||
        !has_sprite_family(OpeningIntroSpriteFamily::helicopter) ||
        !has_sprite_family(OpeningIntroSpriteFamily::aircraft_plane_a)) {
        return 15;
    }
    if (!new_game_state.opening_intro.seattle_plane_cleared ||
        new_game_state.opening_intro.seattle_outline_frame !=
            kSeattleOutlineHoldFrames) {
        return 16;
    }

    const auto actor = opening_intro_sat_draw_point(0x0138U, 0x00F1U, 134, 113);
    const auto heli = opening_intro_sat_draw_point(0x01FCU, 0x00EFU, 40, 124);
    const auto plane = opening_intro_aircraft_plane_draw_point(0);
    const auto plane_scrolled = opening_intro_aircraft_plane_draw_point(-0x40);
    const auto outline = opening_intro_seattle_outline_plan(0);
    if (actor.x != 190 || actor.y != 98 || heli.x != 292 || heli.y != 107 ||
        plane.x != 0x128 || plane.y != 0x70 ||
        plane_scrolled.x != 0x128 - 0x40 || plane_scrolled.y != 0x70 ||
        outline.x != 56 || outline.y != 64 || outline.width != 192 ||
        outline.height != 120) {
        return 17;
    }

    return 0;
}
