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
};

void record(Observations& observations, const char* callback) {
    observations.callbacks.emplace_back(callback);
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
    callbacks.pre_title.resources.write_vdp_grid = [&observations] {
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
        !load_observations.resource_work_area_zeroed ||
        !has_event(load_state, "assets.verified") ||
        !has_event(load_state, "boot.title_ready") ||
        !has_event(load_state, "phase.title_ready")) {
        return 1;
    }

    if (enter_start_options_integration(load_state, load_callbacks) !=
            StartupIntegrationEvent::entered_start_options ||
        load_state.phase != StartupIntegrationPhase::start_options ||
        load_state.start_options.selected != StartOptionsChoice::new_game) {
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
            StartupIntegrationEvent::field_ready ||
        new_game_state.phase != StartupIntegrationPhase::field_ready ||
        new_game_observations.field_ready_count != 1U ||
        new_game_observations.initialized_archetype !=
            JoshuaArchetypeChoice::decker ||
        !has_event(new_game_state, "phase.field_ready")) {
        return 8;
    }

    return 0;
}
