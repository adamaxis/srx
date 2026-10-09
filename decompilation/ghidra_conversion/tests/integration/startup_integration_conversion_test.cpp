#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifndef SHADOWRUN_CONVERSION_ASSET_ROOT
#error "The startup integration test needs the direct asset root"
#endif

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::integration;

struct Observations {
    struct OpeningTextTimelineEntry {
        OpeningIntroTextRequest request;
        std::uint32_t source_frame = 0;
        OpeningIntroStage stage = OpeningIntroStage::schedule;
        std::uint16_t stage2_second_loop_frame = 0;
        std::uint16_t stage_loop_frame = 0;
    };

    struct OpeningAudioCommandSnapshot {
        std::uint16_t command = 0U;
        OpeningIntroStage stage = OpeningIntroStage::not_started;
        OpeningIntroScene scene = OpeningIntroScene::seattle;
        std::uint32_t source_frame = 0U;
    };

    std::vector<std::string> callbacks;
    std::vector<StartupIntegrationPhase> phases;
    std::size_t field_ready_count = 0;
    bool new_game_auto_walk_handoff_seen = false;
    bool resource_work_area_zeroed = false;
    JoshuaArchetypeChoice initialized_archetype =
        JoshuaArchetypeChoice::samurai;
    std::vector<OpeningIntroSpriteGroup> opening_sprite_groups;
    std::vector<OpeningIntroTextRequest> opening_text_requests;
    std::vector<OpeningTextTimelineEntry> opening_text_timeline;
    std::vector<OpeningAudioCommandSnapshot> opening_audio_commands;
};

struct PaletteFadeSnapshot {
    OpeningIntroStage stage = OpeningIntroStage::not_started;
    OpeningIntroScene scene = OpeningIntroScene::seattle;
    std::uint16_t vblank = 0;
    std::uint8_t step = 0;
    std::uint8_t weight = 0;
};

bool palette_fade_matches_source_steps(
    const std::vector<PaletteFadeSnapshot>& snapshots,
    const OpeningIntroStage stage, const OpeningIntroScene scene,
    const bool to_black) {
    std::size_t count = 0U;
    for (const auto& snapshot : snapshots) {
        if (snapshot.stage != stage) {
            continue;
        }
        const auto vblank = static_cast<std::uint16_t>(count);
        const auto steps_applied = static_cast<std::uint8_t>(
            std::min<std::size_t>(vblank / 2U, 7U));
        const auto expected_step = static_cast<std::uint8_t>(
            to_black ? 7U - steps_applied : steps_applied);
        const auto expected_weight = static_cast<std::uint8_t>(
            static_cast<std::uint16_t>(expected_step) * 255U / 7U);
        if (snapshot.scene != scene || snapshot.vblank != vblank ||
            snapshot.step != expected_step ||
            snapshot.weight != expected_weight) {
            return false;
        }
        ++count;
    }
    return count == kStage2PaletteTransitionVblanks;
}

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
    callbacks.pre_title.animation.publish_sprite_batch =
        [&observations](const PreTitleSpriteBatch&) {
            record(observations, "publish_pre_title_sprite_batch");
        };
    callbacks.pre_title.animation.clear_last_sprite_sequence = [&observations] {
        record(observations, "clear_last_pre_title_sprite_sequence");
    };
    callbacks.pre_title.animation.publish_logo_strobe_palette =
        [&observations](const PreTitleLogoStrobePalette&) {
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
    callbacks.pre_title.publish_audio_request = [&observations](
                                                    const PreTitleAudioRequest& request) {
        record(observations,
               request.service_path ==
                       PreTitleAudioServicePath::enabled_variant_transaction
                   ? "service_audio_variant_transaction"
                   : "service_audio_fc08_select");
    };
    callbacks.pre_title.latch_controller_events = [&observations] {
        record(observations, "latch_controller_events");
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
    callbacks.opening_intro.service_audio_command =
        [&state, &observations](const std::uint16_t command) {
        observations.opening_audio_commands.push_back(
            {command, state.opening_intro.stage,
             state.opening_intro.scene,
             state.opening_intro.source_frame});
        record(observations, "opening_service_audio_command");
    };
    callbacks.opening_intro.service_audio = [&observations] {
        record(observations, "opening_service_audio");
    };
    callbacks.opening_intro.fade_palette_to_black = [&observations] {
        record(observations, "opening_fade_palette_to_black");
    };
    callbacks.opening_intro.advance_palette_fade = [&observations] {
        record(observations, "opening_advance_palette_fade");
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
    callbacks.opening_intro.render_text =
        [&state, &observations](const OpeningIntroTextRequest& request) {
            observations.opening_text_requests.push_back(request);
            observations.opening_text_timeline.push_back(
                {request, state.opening_intro.source_frame,
                 state.opening_intro.stage,
                 state.opening_intro.stage2_second_loop_frame,
                 state.opening_intro.stage_loop_frame});
        };
    callbacks.opening_intro.source_boundary = [&observations](
                                                  const std::string_view boundary) {
        observations.callbacks.emplace_back(boundary);
    };
    callbacks.opening_intro.stop_requested = [] { return false; };

    callbacks.phase_changed = [&observations](const StartupIntegrationPhase phase) {
        observations.phases.push_back(phase);
    };
    callbacks.field_ready = [&state, &observations] {
        ++observations.field_ready_count;
        observations.new_game_auto_walk_handoff_seen =
            state.new_game_opening_completed;
    };
    return callbacks;
}

bool has_event(const StartupIntegrationState& state, const char* event) {
    return std::find(state.event_log.begin(), state.event_log.end(), event) !=
           state.event_log.end();
}

bool has_opening_text(const Observations& observations,
                      const OpeningIntroTextAsset asset,
                      const std::uint16_t destination,
                      const std::uint16_t palette_flags) {
    return std::any_of(
        observations.opening_text_requests.begin(),
        observations.opening_text_requests.end(),
        [asset, destination, palette_flags](
            const OpeningIntroTextRequest& request) {
            return opening_intro_text_asset(request.text_asset) == asset &&
                   request.destination_word == destination &&
                   request.palette_flags == palette_flags;
        });
}

std::size_t count_opening_text(const Observations& observations,
                               const OpeningIntroTextAsset asset,
                               const std::uint16_t destination,
                               const std::uint16_t palette_flags) {
    return static_cast<std::size_t>(std::count_if(
        observations.opening_text_requests.begin(),
        observations.opening_text_requests.end(),
        [asset, destination, palette_flags](
            const OpeningIntroTextRequest& request) {
            return opening_intro_text_asset(request.text_asset) == asset &&
                   request.destination_word == destination &&
                   request.palette_flags == palette_flags;
        }));
}

std::size_t count_callback(const Observations& observations,
                           const char* callback) {
    return static_cast<std::size_t>(std::count(
        observations.callbacks.begin(), observations.callbacks.end(),
        std::string(callback)));
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
        !load_state.boot.title_ready || load_state.boot.vblank_count != 560U ||
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
        load_state.phase != StartupIntegrationPhase::start_options_shell ||
        load_state.start_options.selected != StartOptionsChoice::new_game ||
        !load_state.save_roster_buffer.has_value() ||
        load_state.save_roster_buffer->compact_records.size() !=
            kSaveRosterTemplateCount ||
        !has_event(load_state, "save_roster.initialized")) {
        return 2;
    }
    if (advance_startup_integration(load_state, 0U, load_callbacks) !=
            StartupIntegrationEvent::start_options_menu_revealed ||
        load_state.phase != StartupIntegrationPhase::start_options) {
        return 26;
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
        load_state.phase != StartupIntegrationPhase::start_options_shell) {
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
        restored_state.new_game_opening_completed ||
        restored_observations.new_game_auto_walk_handoff_seen ||
        !has_event(restored_state, "save_field_restore.restored")) {
        return 11;
    }

    const auto& opening_schedule = opening_intro_source_plan().opening_schedule;
    if (opening_schedule.size() != 5U ||
        opening_schedule[3].delay_frames != 0x00F0U ||
        opening_schedule[3].action_id != 0x5B378U ||
        opening_schedule[4].delay_frames != 0x000FU ||
        opening_schedule[4].action_id != 0x49A0U ||
        kStage2PaletteTransitionVblanks != 34U) {
        return 25;
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

    // The longplay route used for the manual visual audit moves through
    // Samurai -> Decker -> Gator Shaman before confirming. Keep that captured
    // route separate from the Decker source-table check above so a generic
    // three-choice test cannot hide a wrong record/portrait binding.
    StartupIntegrationState shaman_state{};
    Observations shaman_observations;
    const auto shaman_callbacks =
        make_callbacks(shaman_state, shaman_observations);
    if (start_startup_integration(
            asset_root, shaman_state, shaman_callbacks) !=
            StartupIntegrationStartResult::title_ready ||
        enter_start_options_integration(shaman_state, shaman_callbacks) !=
            StartupIntegrationEvent::entered_start_options ||
        advance_startup_integration(
            shaman_state, kStartupMenuA, shaman_callbacks) !=
            StartupIntegrationEvent::entered_joshua_archetype ||
        advance_startup_integration(
            shaman_state, kStartupMenuDown, shaman_callbacks) !=
            StartupIntegrationEvent::selection_moved ||
        advance_startup_integration(
            shaman_state, kStartupMenuDown, shaman_callbacks) !=
            StartupIntegrationEvent::selection_moved ||
        shaman_state.joshua_archetype.selected !=
            JoshuaArchetypeChoice::gator_shaman ||
        advance_startup_integration(
            shaman_state, kStartupMenuA, shaman_callbacks) !=
            StartupIntegrationEvent::opening_started ||
        shaman_observations.initialized_archetype !=
            JoshuaArchetypeChoice::gator_shaman ||
        !shaman_state.joshua_selection.has_value() ||
        shaman_state.joshua_selection->live_joshua_record !=
            shaman_state.startup_records->joshua_archetypes[2].record ||
        !has_event(shaman_state, "joshua_archetype.initialized") ||
        !has_event(shaman_state, "opening_intro.started")) {
        return 18;
    }

    const auto& opening_plan = opening_intro_source_plan();
    const auto& ambush_patch_phases = opening_plan.sequence_phases;
    const auto ambush_patch_0 = opening_intro_sprite_group(
        ambush_patch_phases[0].sprite_descriptor);
    const auto ambush_patch_1 = opening_intro_sprite_group(
        ambush_patch_phases[1].sprite_descriptor);
    const auto ambush_patch_2 = opening_intro_sprite_group(
        ambush_patch_phases[2].sprite_descriptor);
    const auto ambush_patch_3 = opening_intro_sprite_group(
        ambush_patch_phases[3].sprite_descriptor);
    if (ambush_patch_0.family != OpeningIntroSpriteFamily::background_patch ||
        ambush_patch_1.family != OpeningIntroSpriteFamily::background_patch ||
        ambush_patch_2.family != OpeningIntroSpriteFamily::background_patch ||
        ambush_patch_3.family != OpeningIntroSpriteFamily::background_patch ||
        ambush_patch_0.variant != 0U || ambush_patch_1.variant != 1U ||
        ambush_patch_2.variant != 2U || ambush_patch_3.variant != 3U ||
        ambush_patch_phases[0].delay_frames != 0x000CU ||
        ambush_patch_phases[1].delay_frames != 0x0018U ||
        ambush_patch_phases[2].delay_frames != 0x003CU ||
        ambush_patch_phases[3].delay_frames != 0x000CU) {
        return 27;
    }
    const auto timed_phase_0 = opening_intro_sprite_group(
        opening_plan.stage_timed_phases[0].sprite_descriptor);
    const auto timed_phase_1 = opening_intro_sprite_group(
        opening_plan.stage_timed_phases[1].sprite_descriptor);
    const auto timed_phase_2 = opening_intro_sprite_group(
        opening_plan.stage_timed_phases[2].sprite_descriptor);
    const auto wilderness_map_left = opening_intro_sprite_group(
        opening_plan.wilderness_map_descriptors[0]);
    const auto wilderness_map_center = opening_intro_sprite_group(
        opening_plan.wilderness_map_descriptors[1]);
    const auto wilderness_map_right = opening_intro_sprite_group(
        opening_plan.wilderness_map_descriptors[2]);
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
        opening_plan.stage_timed_phases[2].delay_frames != 6U ||
        wilderness_map_left.family !=
            OpeningIntroSpriteFamily::wilderness_map_left ||
        wilderness_map_center.family !=
            OpeningIntroSpriteFamily::wilderness_map_center ||
        wilderness_map_right.family !=
            OpeningIntroSpriteFamily::wilderness_map_right) {
        return 9;
    }

    for (const auto* alias : {"intro.sprites.trivid.timed.0.0",
                              "intro.sprites.ambush.map.left.0.0",
                              "intro.sprites.ambush.map.center.1.0",
                              "intro.sprites.ambush.map.right.2.0",
                              "intro.sprites.wilderness.overlay.0.0",
                              "intro.sprites.wilderness.final.overlay.0.0"}) {
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
        !expect_origin("intro.sprites.ambush.map.left.0.0", 0, 96) ||
        !expect_origin("intro.sprites.ambush.map.center.1.0", 152, 128) ||
        !expect_origin("intro.sprites.ambush.map.right.2.0", 224, 56) ||
        !expect_origin("intro.sprites.wilderness.overlay.0.0", 0, 96) ||
        !expect_origin("intro.sprites.wilderness.final.overlay.0.0", 24, 120)) {
        return 10;
    }

    // New Game now follows the complete converted 0x59C5E opening chain. It
    // must not silently jump from the archetype menu to a synthetic field
    // screen or stop after the first title/tri-vid boundary.
    bool opening_completed = false;
    std::vector<PaletteFadeSnapshot> palette_fade_snapshots;
    for (std::size_t frame = 0; frame < 5000U; ++frame) {
        const auto event = advance_startup_integration(
            new_game_state, 0U, new_game_callbacks);
        const auto& intro = new_game_state.opening_intro;
        if (intro.stage == OpeningIntroStage::stage2_palette_fade_out ||
            intro.stage == OpeningIntroStage::stage2_palette_fade_in ||
            intro.stage == OpeningIntroStage::orchestrator_palette_fade_out ||
            intro.stage == OpeningIntroStage::orchestrator_palette_fade_in ||
            intro.stage == OpeningIntroStage::stage_loop_palette_fade_out) {
            palette_fade_snapshots.push_back(
                {intro.stage, intro.scene,
                 intro.scene_palette_fade_vblank,
                 intro.scene_palette_fade_step,
                 intro.scene_palette_weight});
        }
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
        !new_game_state.new_game_opening_completed ||
        !new_game_observations.new_game_auto_walk_handoff_seen ||
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
    constexpr std::array<std::uint16_t, 3> kStageAudioCommands{
        0x10U, 0x11U, 0x12U};
    std::size_t next_stage_command = 0U;
    for (const auto& snapshot :
         new_game_observations.opening_audio_commands) {
        if (next_stage_command < kStageAudioCommands.size() &&
            snapshot.command == kStageAudioCommands[next_stage_command]) {
            ++next_stage_command;
        }
    }
    const auto find_opening_audio_command =
        [&new_game_observations](const std::uint16_t command) {
            return std::find_if(
                new_game_observations.opening_audio_commands.begin(),
                new_game_observations.opening_audio_commands.end(),
                [command](const auto& snapshot) {
                    return snapshot.command == command;
                });
        };
    const auto stage2_audio = find_opening_audio_command(0x11U);
    const auto stage3_audio = find_opening_audio_command(0x12U);
    const bool later_audio_matches_stages =
        stage2_audio != new_game_observations.opening_audio_commands.end() &&
        stage2_audio->stage == OpeningIntroStage::stage2_palette_fade_out &&
        stage2_audio->scene == OpeningIntroScene::seattle &&
        stage3_audio != new_game_observations.opening_audio_commands.end() &&
        stage3_audio->stage ==
            OpeningIntroStage::orchestrator_palette_fade_out &&
        stage3_audio->scene == OpeningIntroScene::wilderness;
    if (new_game_observations.opening_audio_commands.empty() ||
        new_game_observations.opening_audio_commands.front().command !=
            0x10U ||
        new_game_observations.opening_audio_commands.front().stage !=
            OpeningIntroStage::not_started ||
        new_game_observations.opening_audio_commands.front().scene !=
            OpeningIntroScene::seattle ||
        new_game_observations.opening_audio_commands.front().source_frame !=
            0U ||
        next_stage_command != kStageAudioCommands.size() ||
        !later_audio_matches_stages) {
        std::cerr << "opening audio commands:";
        for (const auto& snapshot :
             new_game_observations.opening_audio_commands) {
            std::cerr << " 0x" << std::hex << snapshot.command;
        }
        std::cerr << '\n';
        return 27;
    }
    if (new_game_state.opening_intro.scene_palette_fade_vblank !=
            kStage2PaletteTransitionVblanks ||
        new_game_state.opening_intro.scene_palette_fade_step != 7U ||
        new_game_state.opening_intro.scene_palette_weight != 255U ||
        count_callback(new_game_observations,
                       "0x49A0.seattle_focus_hold_complete") != 1U) {
        return 26;
    }
    // The 0x131C/0x12F6 split is a source behavior boundary: resource-set
    // changes fade the previous scene to black, while only the initial
    // wilderness and tri-vid reveals advance toward a new target palette.
    // This must not regress into one transition callback on every frame.
    if (count_callback(new_game_observations,
                       "opening_fade_palette_to_black") != 6U ||
        count_callback(new_game_observations,
                       "opening_advance_palette_fade") != 2U) {
        return 19;
    }
    if (!palette_fade_matches_source_steps(
            palette_fade_snapshots,
            OpeningIntroStage::stage2_palette_fade_out,
            OpeningIntroScene::seattle, true) ||
        !palette_fade_matches_source_steps(
            palette_fade_snapshots,
            OpeningIntroStage::stage2_palette_fade_in,
            OpeningIntroScene::wilderness, false) ||
        !palette_fade_matches_source_steps(
            palette_fade_snapshots,
            OpeningIntroStage::orchestrator_palette_fade_out,
            OpeningIntroScene::wilderness, true) ||
        !palette_fade_matches_source_steps(
            palette_fade_snapshots,
            OpeningIntroStage::orchestrator_palette_fade_in,
            OpeningIntroScene::trivid, false) ||
        !palette_fade_matches_source_steps(
            palette_fade_snapshots,
            OpeningIntroStage::stage_loop_palette_fade_out,
            OpeningIntroScene::trivid, true)) {
        return 27;
    }

    // 0x5AE9E is a concrete source record writer, not a generic story label.
    // Keep the two wilderness lines and all five rewritten Seattle/team rows
    // at their ROM destinations; the Qt host decides scene visibility without
    // deleting these converted source requests.
    if (!has_opening_text(new_game_observations,
                          OpeningIntroTextAsset::without_warning, 0x2186U,
                          0xE000U) ||
        !has_opening_text(new_game_observations,
                          OpeningIntroTextAsset::ambushed, 0x2286U,
                          0xE000U) ||
        !has_opening_text(new_game_observations, OpeningIntroTextAsset::time,
                          0x210EU, 0xA000U) ||
        !has_opening_text(new_game_observations, OpeningIntroTextAsset::place,
                          0x2210U, 0xA000U) ||
        !has_opening_text(new_game_observations,
                          OpeningIntroTextAsset::team_cover, 0x2304U,
                          0xA000U) ||
        !has_opening_text(new_game_observations,
                          OpeningIntroTextAsset::team_runners, 0x2384U,
                          0xA000U) ||
        !has_opening_text(new_game_observations,
                          OpeningIntroTextAsset::team_wilderness, 0x2404U,
                          0xA000U)) {
        return 20;
    }
    // The opening schedule's 0x5AEEE entry writes both the cover and runner
    // lines before the later 0x5AE9E wilderness-story refresh writes them
    // again. Counting requests distinguishes the two source transitions.
    if (count_opening_text(new_game_observations,
                           OpeningIntroTextAsset::team_cover, 0x2304U,
                           0xA000U) < 2U ||
        count_opening_text(new_game_observations,
                           OpeningIntroTextAsset::team_runners, 0x2384U,
                           0xA000U) < 2U ||
        count_opening_text(new_game_observations,
                           OpeningIntroTextAsset::team_wilderness, 0x2404U,
                           0xA000U) < 2U) {
        return 23;
    }
    for (const auto destination : {0x2106U, 0x2186U, 0x2206U, 0x2286U,
                                   0x2306U}) {
        if (!has_opening_text(new_game_observations,
                              OpeningIntroTextAsset::blank, destination,
                              0xA000U)) {
            return 21;
        }
    }
    if (!has_opening_text(new_game_observations,
                          OpeningIntroTextAsset::massacre, 0x2106U,
                          0xE000U) ||
        !has_opening_text(new_game_observations,
                          OpeningIntroTextAsset::captured, 0x2206U,
                          0xE000U) ||
        !has_opening_text(new_game_observations,
                          OpeningIntroTextAsset::slain, 0x2306U,
                          0xE000U) ||
        !opening_intro_text(OpeningIntroTextAsset::blank).empty()) {
        return 22;
    }
    const auto massacre_transition = std::find_if(
        new_game_observations.opening_text_timeline.begin(),
        new_game_observations.opening_text_timeline.end(),
        [](const Observations::OpeningTextTimelineEntry& entry) {
            return opening_intro_text_asset(entry.request.text_asset) ==
                       OpeningIntroTextAsset::massacre &&
                   entry.request.destination_word == 0x2106U;
        });
    if (massacre_transition ==
            new_game_observations.opening_text_timeline.end() ||
        massacre_transition->stage != OpeningIntroStage::stage2_second_loop ||
        massacre_transition->stage2_second_loop_frame !=
            kTitleIntroStage2StoryTextElapsedFrameIndex) {
        return 27;
    }
    const auto landing_transition_at_rom_count = std::any_of(
        new_game_observations.opening_text_timeline.begin(),
        new_game_observations.opening_text_timeline.end(),
        [](const Observations::OpeningTextTimelineEntry& entry) {
            return opening_intro_text_asset(entry.request.text_asset) ==
                       OpeningIntroTextAsset::landing &&
                   entry.stage == OpeningIntroStage::stage_loop &&
                   entry.stage_loop_frame ==
                       kTitleIntroStageFinalTextElapsedFrameIndex;
        });
    const auto landing_text_event_count = std::count_if(
        new_game_observations.opening_text_timeline.begin(),
        new_game_observations.opening_text_timeline.end(),
        [](const Observations::OpeningTextTimelineEntry& entry) {
            return opening_intro_text_asset(entry.request.text_asset) ==
                   OpeningIntroTextAsset::landing;
        });
    constexpr std::array<std::uint16_t, 3> kCenterTextRows{{
        0x2186U, 0x2286U, 0x2386U}};
    std::array<bool, kCenterTextRows.size()> center_rows_cleared{};
    for (const auto& entry : new_game_observations.opening_text_timeline) {
        if (entry.stage != OpeningIntroStage::stage_loop ||
            opening_intro_text_asset(entry.request.text_asset) !=
                OpeningIntroTextAsset::blank ||
            entry.stage_loop_frame != 0x00D7U) {
            continue;
        }
        const auto row = std::find(kCenterTextRows.begin(),
                                   kCenterTextRows.end(),
                                   entry.request.destination_word);
        if (row != kCenterTextRows.end()) {
            center_rows_cleared[static_cast<std::size_t>(
                row - kCenterTextRows.begin())] = true;
        }
    }
    if (!landing_transition_at_rom_count) {
        return 28;
    }
    if (landing_text_event_count != 1U) {
        return 29;
    }
    if (!std::all_of(center_rows_cleared.begin(), center_rows_cleared.end(),
                     [](const bool cleared) { return cleared; })) {
        return 30;
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
        !has_sprite_family(OpeningIntroSpriteFamily::wilderness_map_left) ||
        !has_sprite_family(OpeningIntroSpriteFamily::wilderness_map_center) ||
        !has_sprite_family(OpeningIntroSpriteFamily::wilderness_map_right) ||
        !has_sprite_family(OpeningIntroSpriteFamily::wilderness_overlay) ||
        !has_sprite_family(OpeningIntroSpriteFamily::wilderness_final_overlay) ||
        !has_sprite_family(OpeningIntroSpriteFamily::ambush_actor) ||
        !has_sprite_family(OpeningIntroSpriteFamily::ambush_body) ||
        !has_sprite_family(OpeningIntroSpriteFamily::helicopter) ||
        !has_sprite_family(OpeningIntroSpriteFamily::aircraft_plane_a)) {
        return 15;
    }
    if (!new_game_state.opening_intro.seattle_plane_cleared ||
        new_game_state.opening_intro.seattle_outline_frame !=
            kSeattleOutlineAnimationFrames) {
        return 16;
    }

    const auto actor = opening_intro_sat_draw_point(0x0138U, 0x00F1U, 134, 113);
    const auto heli = opening_intro_sat_draw_point(0x01FCU, 0x00EFU, 40, 124);
    const auto plane = opening_intro_aircraft_plane_draw_point(0);
    const auto plane_scrolled = opening_intro_aircraft_plane_draw_point(-0x40);
    constexpr std::array<std::array<int, 4>, 5> kExpectedOutlineFrames{{
        {{57, 70, 193, 127}},
        {{75, 92, 154, 105}},
        {{98, 116, 116, 81}},
        {{123, 132, 66, 57}},
        {{139, 147, 19, 17}},
    }};
    if (actor.x != 190 || actor.y != 98 || heli.x != 292 || heli.y != 107 ||
        plane.x != 0x128 || plane.y != 0x70 ||
        plane_scrolled.x != 0x128 - 0x40 || plane_scrolled.y != 0x70) {
        return 17;
    }
    for (std::size_t phase = 0; phase < kExpectedOutlineFrames.size();
         ++phase) {
        const auto outline = opening_intro_seattle_outline_plan(
            static_cast<std::uint8_t>(phase));
        const auto& expected = kExpectedOutlineFrames[phase];
        if (outline.x != expected[0] || outline.y != expected[1] ||
            outline.width != expected[2] || outline.height != expected[3]) {
            return 24;
        }
    }

    return 0;
}
