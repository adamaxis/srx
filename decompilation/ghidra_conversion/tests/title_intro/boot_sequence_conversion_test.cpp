#include "shadowrun_ghidra/title_intro/boot_sequence_conversion.hpp"

#include <algorithm>
#include <string>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto aliases = required_boot_asset_aliases();
    AssetSetReport assets{aliases, {}, 1234U};
    BootStartupState startup{};
    std::vector<BootStartupStage> stages;
    std::vector<std::uint32_t> delays;
    std::vector<std::string> startup_events;
    BootStartupCallbacks callbacks{};
    callbacks.enter_stage = [&stages](const BootStartupStage stage) {
        stages.push_back(stage);
    };
    callbacks.initialize_display = [&startup_events] {
        startup_events.emplace_back("display");
    };
    callbacks.vdp_ready = {
        [&delays](const std::uint32_t delay) { delays.push_back(delay); },
        [&startup_events] { startup_events.emplace_back("ready"); },
        [&delays](const std::uint32_t delay) { delays.push_back(delay); },
    };
    callbacks.run_pre_title_sequence = [&startup_events] {
        startup_events.emplace_back("pre-title");
    };
    callbacks.setup_title_state = [&startup_events] {
        startup_events.emplace_back("title");
    };

    if (run_boot_startup(assets, startup, callbacks) !=
            BootStartupResult::title_ready ||
        !startup.display_ready || !startup.title_ready ||
        startup.stage != BootStartupStage::title_ready ||
        stages != std::vector<BootStartupStage>{
                      BootStartupStage::reset,
                      BootStartupStage::vdp_ready,
                      BootStartupStage::pre_title,
                      BootStartupStage::title_setup,
                      BootStartupStage::title_ready} ||
        delays != std::vector<std::uint32_t>{45056U, 256U} ||
        startup_events !=
            std::vector<std::string>{"display", "ready", "pre-title", "title"}) {
        return 1;
    }

    PreTitleSequenceState pre_title{};
    pre_title.logo_phase_countdown = 0x7FFF;
    std::size_t waits = 0;
    std::size_t sprite_builds = 0;
    std::size_t cleared_sprite_sequences = 0;
    std::size_t hscroll_updates = 0;
    std::size_t resource_stream_writes = 0;
    std::vector<std::string> resource_events;
    std::vector<PreTitleGridWriteRequest> grid_requests;
    std::vector<PreTitleLogoStrobePalette> strobe_palettes;
    std::vector<PreTitleSpriteBatch> sprite_batches;
    std::vector<PreTitleAudioRequest> audio_requests;
    std::size_t variant_audio_requests = 0;
    std::size_t enabled_service_audio_requests = 0;
    bool resource_work_area_zeroed = false;
    std::size_t palette_fades = 0;
    std::size_t controller_samples = 0;
    std::size_t controller_latches = 0;
    std::size_t palette_transition_calls = 0;
    PreTitleSequenceCallbacks pre_title_callbacks{};
    pre_title_callbacks.resources.clear_plane_a = [&resource_events] {
        resource_events.emplace_back("clear-a");
    };
    pre_title_callbacks.resources.clear_plane_b = [&resource_events] {
        resource_events.emplace_back("clear-b");
    };
    pre_title_callbacks.resources.prepare_title_resources = [&resource_events] {
        resource_events.emplace_back("prepare-title");
    };
    pre_title_callbacks.resources.write_resource_stream =
        [&resource_stream_writes, &resource_work_area_zeroed](auto& words) {
            ++resource_stream_writes;
            resource_work_area_zeroed = std::all_of(
                words.begin(), words.end(),
                [](const std::uint16_t word) { return word == 0U; });
        };
    pre_title_callbacks.resources.refresh_field_resource_streams =
        [&resource_events] { resource_events.emplace_back("refresh-field"); };
    pre_title_callbacks.resources.decode_and_upload_source_stream =
        [&resource_events] { resource_events.emplace_back("decode-upload"); };
    pre_title_callbacks.resources.advance_title_palette_fade =
        [&palette_fades] { ++palette_fades; };
    pre_title_callbacks.resources.write_pre_title_grid =
        [&resource_events, &grid_requests](const auto& request) {
        resource_events.emplace_back("write-grid");
        grid_requests.push_back(request);
    };
    pre_title_callbacks.resources.reset_vdp_upload_state = [&resource_events] {
        resource_events.emplace_back("reset-upload");
    };
    pre_title_callbacks.wait_for_vblank = [&waits] { ++waits; };
    pre_title_callbacks.animation.publish_sprite_batch =
        [&sprite_builds, &sprite_batches](const PreTitleSpriteBatch& batch) {
            ++sprite_builds;
            sprite_batches.push_back(batch);
        };
    pre_title_callbacks.animation.clear_last_sprite_sequence =
        [&cleared_sprite_sequences] { ++cleared_sprite_sequences; };
    pre_title_callbacks.animation.publish_logo_strobe_palette =
        [&strobe_palettes](const PreTitleLogoStrobePalette& palette) {
            strobe_palettes.push_back(palette);
        };
    pre_title_callbacks.animation.step_entity_color_words = [] {};
    pre_title_callbacks.write_horizontal_scroll_stream =
        [&hscroll_updates](const auto&) { ++hscroll_updates; };
    pre_title_callbacks.publish_audio_request =
        [&audio_requests, &variant_audio_requests,
         &enabled_service_audio_requests](
            const PreTitleAudioRequest& request) {
            audio_requests.push_back(request);
            if (request.service_path ==
                PreTitleAudioServicePath::enabled_variant_transaction) {
                ++variant_audio_requests;
            } else {
                ++enabled_service_audio_requests;
            }
        };
    pre_title_callbacks.sample_controller = [&controller_samples] {
        ++controller_samples;
    };
    pre_title_callbacks.latch_controller_events = [&controller_latches] {
        ++controller_latches;
    };
    pre_title_callbacks.step_palette_transition =
        [&palette_transition_calls] { ++palette_transition_calls; };
    if (run_pre_title_sequence(pre_title, pre_title_callbacks) !=
            PreTitleSequenceResult::source_complete ||
        pre_title.startup_marker != 1U || pre_title.resource_marker != 1U ||
        pre_title.figure_x != 0x0119 || pre_title.figure_y != 0x0160 ||
        waits != 560U || sprite_builds != 932U || hscroll_updates != 337U ||
        cleared_sprite_sequences != 337U || sprite_batches.size() != sprite_builds ||
        variant_audio_requests != 63U ||
            enabled_service_audio_requests != 60U ||
        audio_requests.size() !=
            variant_audio_requests + enabled_service_audio_requests ||
        resource_stream_writes != 1U || !resource_work_area_zeroed ||
        palette_fades != 2U ||
        controller_samples != 2U || controller_latches != 102U ||
        palette_transition_calls != 2U ||
        pre_title.presentation_stage !=
            PreTitleSequenceState::PresentationStage::complete ||
        pre_title.fasa_hold_frames != 0U ||
        pre_title.palette_transition_passes_completed !=
            kPreTitlePaletteTransitionPasses ||
        pre_title.palette_transition_vblank_phase != 0U ||
        resource_events != std::vector<std::string>{
            "clear-a", "clear-b", "prepare-title", "refresh-field",
            "decode-upload", "write-grid", "clear-a", "clear-b",
            "reset-upload", "write-grid"} ||
        grid_requests.size() != 2U ||
        grid_requests[0].purpose != PreTitleGridPurpose::resource_setup ||
        grid_requests[0].transfer_block_count != 3U ||
        grid_requests[0].words_per_block != 12U ||
        grid_requests[0].word_mask != 0U ||
        grid_requests[1].purpose != PreTitleGridPurpose::final_display ||
        grid_requests[1].transfer_block_count != 4U ||
        grid_requests[1].words_per_block != 16U ||
        grid_requests[1].word_mask != 0x4000U || strobe_palettes.empty() ||
        strobe_palettes.front().phase != 0U ||
        strobe_palettes.front().color_words !=
            std::array<std::uint16_t, 8>{
                0x0080U, 0x0060U, 0x0040U, 0x0020U,
                0x0000U, 0x0000U, 0x0000U, 0x0000U} ||
        sprite_batches.front().source !=
            PreTitleSpriteBatchSource::sega_guy_primary ||
        sprite_batches.front().origin_x != 0x0119 ||
        sprite_batches.front().origin_y != 0x01A8 ||
        sprite_batches.front().first_sequence != 2U ||
        sprite_batches.front().attribute_flags != 0x6000U ||
        sprite_batches.front().records.size() != 5U ||
        sprite_batches.front().records.front().x != 0x00DA ||
        sprite_batches.front().records.front().y != 0x0199 ||
        sprite_batches.front().records.front().sequence != 2U ||
        sprite_batches[1].source !=
            PreTitleSpriteBatchSource::sega_guy_secondary ||
        sprite_batches[1].first_sequence != 7U ||
        sprite_batches[1].records.size() != 5U) {
        return 2;
    }
    for (const auto& request : audio_requests) {
        if (request.service_path ==
                PreTitleAudioServicePath::enabled_variant_transaction &&
            (request.command != 0x0016U ||
             (request.trigger !=
                  PreTitleAudioTrigger::logo_effect_counter_below_six &&
              request.trigger !=
                  PreTitleAudioTrigger::logo_effect_counter_reached_zero))) {
            return 7;
        }
        if (request.service_path ==
                PreTitleAudioServicePath::enabled_service_queue &&
            (request.command != 0x005CU ||
             request.trigger !=
                 PreTitleAudioTrigger::logo_effect_counter_above_four)) {
            return 8;
        }
    }

    PreTitleSequenceState phase_state{};
    phase_state.sega_phase_delay = -1;
    phase_state.logo_phase_countdown = 0;
    phase_state.logo_phase = 5U;
    std::size_t phase_builds = 0;
    std::vector<PreTitleSpriteBatch> phase_batches;
    PreTitleSequenceCallbacks phase_callbacks{};
    phase_callbacks.animation.publish_sprite_batch =
        [&phase_builds, &phase_batches](const PreTitleSpriteBatch& batch) {
        ++phase_builds;
        phase_batches.push_back(batch);
    };
    phase_callbacks.animation.clear_last_sprite_sequence = [] {};
    advance_pre_title_animation(phase_state, phase_callbacks);
    if (phase_state.sprite_update_marker != 1U ||
        phase_state.sprite_projection_marker != 1U ||
        !phase_state.sega_lightning_batches_active ||
        phase_state.logo_phase != 0U || phase_state.logo_phase_countdown != 3 ||
        phase_builds != 4U || phase_batches.size() != 4U ||
        phase_batches[0].source !=
            PreTitleSpriteBatchSource::sega_guy_primary ||
        phase_batches[0].records.size() != 5U ||
        phase_batches[0].first_sequence != 2U ||
        phase_batches[1].source !=
            PreTitleSpriteBatchSource::sega_guy_secondary ||
        phase_batches[1].records.size() != 5U ||
        phase_batches[1].first_sequence != 7U ||
        phase_batches[2].source !=
            PreTitleSpriteBatchSource::sega_lightning_primary ||
        phase_batches[2].phase != 0U || phase_batches[2].origin_x != 0x0158 ||
        phase_batches[2].origin_y != 0x0120 ||
        phase_batches[2].first_sequence != 12U ||
        phase_batches[2].records.size() != 3U ||
        phase_batches[3].source !=
            PreTitleSpriteBatchSource::sega_lightning_secondary ||
        phase_batches[3].first_sequence != 15U ||
        phase_batches[3].records.size() != 3U ||
        phase_batches[2].records.front().x != 0x0121 ||
        phase_batches[2].records.front().y != 0x0109) {
        return 3;
    }

    PreTitleSequenceState lightning_exit_state{};
    lightning_exit_state.sega_phase_delay = -1;
    lightning_exit_state.logo_strobe_active = true;
    PreTitleSequenceCallbacks lightning_exit_callbacks{};
    lightning_exit_callbacks.animation.publish_sprite_batch =
        [](const PreTitleSpriteBatch&) {};
    lightning_exit_callbacks.animation.clear_last_sprite_sequence = [] {};
    advance_pre_title_animation(lightning_exit_state,
                                lightning_exit_callbacks);
    if (lightning_exit_state.sega_lightning_batches_active) {
        return 6;
    }

    PreTitleSequenceState fade_state{};
    fade_state.strobe_countdown = 0;
    std::size_t figure_palette_fade_calls = 0U;
    PreTitleSequenceCallbacks fade_callbacks{};
    fade_callbacks.animation.publish_logo_strobe_palette =
        [](const PreTitleLogoStrobePalette&) {};
    fade_callbacks.animation.step_entity_color_words =
        [&figure_palette_fade_calls] { ++figure_palette_fade_calls; };
    for (std::uint8_t vblank = 1U; vblank <= 38U; ++vblank) {
        advance_pre_title_logo_strobe(fade_state, fade_callbacks);
        const auto expected_calls = vblank < 3U
                                        ? 0U
                                        : static_cast<std::size_t>(
                                              1U + (vblank - 3U) / 5U);
        const auto expected_step = static_cast<std::uint8_t>(
            std::min<std::size_t>(expected_calls,
                                  kPreTitleFigurePaletteLevelsToBlack));
        if (figure_palette_fade_calls != expected_calls ||
            fade_state.figure_palette_fade_steps != expected_step) {
            return 4;
        }
    }
    if (figure_palette_fade_calls != 8U ||
        fade_state.figure_palette_fade_steps !=
            kPreTitleFigurePaletteLevelsToBlack) {
        return 5;
    }

    const std::array<std::array<std::uint16_t, 8>, 8> expected_strobes{{
        {0x0080U, 0x0060U, 0x0040U, 0x0020U,
         0x0000U, 0x0000U, 0x0000U, 0x0000U},
        {0x0060U, 0x0040U, 0x0020U, 0x0000U,
         0x0000U, 0x0000U, 0x0000U, 0x0080U},
        {0x0040U, 0x0020U, 0x0000U, 0x0000U,
         0x0000U, 0x0000U, 0x0080U, 0x0060U},
        {0x0020U, 0x0000U, 0x0000U, 0x0000U,
         0x0000U, 0x0080U, 0x0060U, 0x0040U},
        {0x0000U, 0x0000U, 0x0000U, 0x0000U,
         0x0080U, 0x0060U, 0x0040U, 0x0020U},
        {0x0000U, 0x0000U, 0x0000U, 0x0080U,
         0x0060U, 0x0040U, 0x0020U, 0x0000U},
        {0x0000U, 0x0000U, 0x0080U, 0x0060U,
         0x0040U, 0x0020U, 0x0000U, 0x0000U},
        {0x0000U, 0x0080U, 0x0060U, 0x0040U,
         0x0020U, 0x0000U, 0x0000U, 0x0000U},
    }};
    for (std::size_t previous_phase = 0; previous_phase < 8U;
         ++previous_phase) {
        PreTitleSequenceState strobe_state{};
        strobe_state.strobe_countdown = 0;
        strobe_state.strobe_phase = static_cast<std::uint8_t>(previous_phase);
        PreTitleLogoStrobePalette published{};
        PreTitleSequenceCallbacks strobe_callbacks{};
        strobe_callbacks.animation.publish_logo_strobe_palette =
            [&published](const PreTitleLogoStrobePalette& palette) {
                published = palette;
            };
        advance_pre_title_logo_strobe(strobe_state, strobe_callbacks);
        const auto next_phase = (previous_phase + 1U) % 8U;
        if (published.phase != next_phase ||
            published.color_words != expected_strobes[next_phase]) {
            return 6;
        }
    }

    return 0;
}
