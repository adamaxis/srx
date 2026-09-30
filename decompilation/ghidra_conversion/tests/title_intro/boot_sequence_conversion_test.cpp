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
    std::size_t hscroll_updates = 0;
    std::size_t resource_stream_writes = 0;
    std::vector<std::string> resource_events;
    std::vector<PreTitleGridWriteRequest> grid_requests;
    std::vector<PreTitleLogoStrobePalette> strobe_palettes;
    bool resource_work_area_zeroed = false;
    std::size_t palette_fades = 0;
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
    pre_title_callbacks.animation.build_title_sprite_records = [&sprite_builds] {
        ++sprite_builds;
    };
    pre_title_callbacks.animation.publish_logo_strobe_palette =
        [&strobe_palettes](const PreTitleLogoStrobePalette& palette) {
            strobe_palettes.push_back(palette);
        };
    pre_title_callbacks.animation.step_entity_color_words = [] {};
    pre_title_callbacks.write_horizontal_scroll_stream =
        [&hscroll_updates](const auto&) { ++hscroll_updates; };
    pre_title_callbacks.service_audio_variant = [] {};
    pre_title_callbacks.service_audio = [] {};
    pre_title_callbacks.sample_controller = [] {};
    pre_title_callbacks.step_palette_transition = [] {};
    if (run_pre_title_sequence(pre_title, pre_title_callbacks) !=
            PreTitleSequenceResult::source_complete ||
        pre_title.startup_marker != 1U || pre_title.resource_marker != 1U ||
        pre_title.figure_x != 0x0119 || pre_title.figure_y != 0x0160 ||
        waits != 458U || sprite_builds != 932U || hscroll_updates != 337U ||
        resource_stream_writes != 1U || !resource_work_area_zeroed ||
        palette_fades != 2U ||
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
                0x0000U, 0x0000U, 0x0000U, 0x0000U}) {
        return 2;
    }

    PreTitleSequenceState phase_state{};
    phase_state.sega_phase_delay = -1;
    phase_state.logo_phase_countdown = 0;
    phase_state.logo_phase = 5U;
    std::size_t phase_builds = 0;
    PreTitleSequenceCallbacks phase_callbacks{};
    phase_callbacks.animation.build_title_sprite_records = [&phase_builds] {
        ++phase_builds;
    };
    advance_pre_title_animation(phase_state, phase_callbacks);
    if (phase_state.sprite_update_marker != 1U ||
        phase_state.sprite_projection_marker != 1U ||
        phase_state.logo_phase != 0U || phase_state.logo_phase_countdown != 3 ||
        phase_builds != 4U) {
        return 3;
    }

    PreTitleSequenceState fade_state{};
    fade_state.strobe_countdown = 3;
    fade_state.figure_color_fade_weight = 16U;
    PreTitleSequenceCallbacks fade_callbacks{};
    fade_callbacks.animation.step_entity_color_words = [] {};
    advance_pre_title_logo_strobe(fade_state, fade_callbacks);
    if (fade_state.figure_color_fade_weight != 8U) {
        return 4;
    }
    fade_state.strobe_countdown = 3;
    advance_pre_title_logo_strobe(fade_state, fade_callbacks);
    if (fade_state.figure_color_fade_weight != 0U) {
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
