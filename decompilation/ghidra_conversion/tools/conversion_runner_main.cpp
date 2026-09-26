#include "shadowrun_ghidra/conversion_workbench.hpp"
#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/core/timing_conversion.hpp"
#include "shadowrun_ghidra/title_intro/boot_sequence_conversion.hpp"
#include "shadowrun_ghidra/title_intro/title_setup_conversion.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::string stage_name(
    const shadowrun::ghidra_conversion::BootStartupStage stage) {
    using shadowrun::ghidra_conversion::BootStartupStage;
    switch (stage) {
    case BootStartupStage::reset:
        return "reset";
    case BootStartupStage::vdp_ready:
        return "vdp_ready";
    case BootStartupStage::pre_title:
        return "pre_title";
    case BootStartupStage::title_setup:
        return "title_setup";
    case BootStartupStage::title_ready:
        return "title_ready";
    case BootStartupStage::failed:
        return "failed";
    }
    return "unknown";
}

std::filesystem::path default_asset_root() {
    const std::vector<std::filesystem::path> candidates{
        "decompilation/assets", "assets", "../assets", "../../assets"};
    for (const auto& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate / "resources.tsv")) {
            return candidate;
        }
    }
    return {};
}

} // namespace

int main(int argc, char** argv) {
    std::filesystem::path asset_root;
    for (int index = 1; index < argc; ++index) {
        const std::string argument(argv[index]);
        if (argument == "--assets" && index + 1 < argc) {
            asset_root = argv[++index];
        } else if (argument == "--help") {
            std::cout << "usage: shadowrun_ghidra_conversion_runner"
                         " [--assets <decompilation/assets>]\n";
            return 0;
        } else {
            std::cerr << "unknown or incomplete argument: " << argument
                      << '\n';
            return 2;
        }
    }
    if (asset_root.empty()) {
        asset_root = default_asset_root();
    }

    const auto report =
        shadowrun::ghidra_conversion::run_conversion_workbench();
    std::cout << shadowrun::ghidra_conversion::format_conversion_workbench_report(
        report);

    using namespace shadowrun::ghidra_conversion;
    if (asset_root.empty()) {
        std::cout << "assets.result=missing_root\n";
        return report.passed ? 3 : 1;
    }

    const auto catalog = load_conversion_asset_catalog(asset_root);
    const auto required = verify_conversion_asset_set(
        catalog.catalog, required_boot_asset_aliases());
    std::cout << "assets.root=" << asset_root.string() << '\n'
              << "assets.catalog_records=" << catalog.catalog.records.size()
              << '\n'
              << "assets.required_records=" << required.required_aliases.size()
              << '\n'
              << "assets.required_bytes=" << required.total_bytes << '\n'
              << "assets.catalog_errors=" << catalog.errors.size() << '\n'
              << "assets.missing_required=" << required.missing_aliases.size()
              << '\n';
    for (const auto& error : catalog.errors) {
        std::cout << "assets.error=" << error << '\n';
    }
    for (const auto& alias : required.missing_aliases) {
        std::cout << "assets.missing=" << alias << '\n';
    }
    if (!catalog.complete() || !required.complete()) {
        std::cout << "startup.result=assets_incomplete\n";
        return 4;
    }

    BootStartupState startup{};
    PreTitleSequenceState pre_title{};
    VblankFrameState frame_state{};
    TitleSetupState title_state{};
    std::vector<std::string> stages;
    std::size_t sprite_builds = 0;
    std::size_t horizontal_scroll_updates = 0;
    std::size_t audio_variant_calls = 0;
    std::size_t audio_calls = 0;
    std::size_t controller_samples = 0;
    std::size_t palette_steps = 0;
    std::size_t resource_stream_writes = 0;
    std::size_t strobe_phase_updates = 0;
    bool resource_work_area_zeroed = false;
    std::uint32_t title_palette_frames = 0;
    bool pre_title_completed = false;

    PreTitleSequenceCallbacks pre_title_callbacks{};
    pre_title_callbacks.resources.clear_plane_a = [&] {
        stages.emplace_back("clear_plane_a");
    };
    pre_title_callbacks.resources.clear_plane_b = [&] {
        stages.emplace_back("clear_plane_b");
    };
    pre_title_callbacks.resources.prepare_title_resources = [&] {
        stages.emplace_back("prepare_title_resources");
    };
    pre_title_callbacks.resources.write_resource_stream =
        [&resource_stream_writes, &resource_work_area_zeroed](auto& words) {
            ++resource_stream_writes;
            resource_work_area_zeroed = std::all_of(
                words.begin(), words.end(),
                [](const std::uint16_t word) { return word == 0U; });
        };
    pre_title_callbacks.resources.refresh_field_resource_streams = [&] {
        stages.emplace_back("refresh_field_resource_streams");
    };
    pre_title_callbacks.resources.decode_and_upload_source_stream = [&] {
        stages.emplace_back("decode_and_upload_source_stream");
    };
    pre_title_callbacks.resources.advance_title_palette_fade =
        [&palette_steps] { ++palette_steps; };
    pre_title_callbacks.resources.write_vdp_grid = [&] {
        stages.emplace_back("write_vdp_grid");
    };
    pre_title_callbacks.resources.reset_vdp_upload_state = [&] {
        stages.emplace_back("reset_vdp_upload_state");
    };
    pre_title_callbacks.wait_for_vblank = [&] {
        wait_for_vblank(frame_state, [](const std::uint16_t previous) {
            return static_cast<std::uint16_t>(previous + 1U);
        });
    };
    pre_title_callbacks.animation.build_title_sprite_records =
        [&sprite_builds] { ++sprite_builds; };
    pre_title_callbacks.animation.publish_logo_strobe_phase =
        [&strobe_phase_updates](const std::uint8_t) { ++strobe_phase_updates; };
    pre_title_callbacks.animation.step_entity_color_words = [] {};
    pre_title_callbacks.write_horizontal_scroll_stream =
        [&horizontal_scroll_updates](const auto&) {
            ++horizontal_scroll_updates;
        };
    pre_title_callbacks.service_audio_variant = [&] { ++audio_variant_calls; };
    pre_title_callbacks.service_audio = [&] { ++audio_calls; };
    pre_title_callbacks.sample_controller = [&] { ++controller_samples; };
    pre_title_callbacks.step_palette_transition = [&] { ++palette_steps; };

    BootStartupCallbacks startup_callbacks{};
    startup_callbacks.enter_stage = [&stages](const BootStartupStage stage) {
        stages.push_back(stage_name(stage));
    };
    startup_callbacks.initialize_display = [&stages] {
        stages.emplace_back("display_initialized");
    };
    startup_callbacks.vdp_ready = {
        [](const std::uint32_t) {},
        [] {},
        [](const std::uint32_t) {},
    };
    startup_callbacks.run_pre_title_sequence = [&] {
        pre_title_completed =
            run_pre_title_sequence(pre_title, pre_title_callbacks) ==
            PreTitleSequenceResult::source_complete;
        startup.vblank_count = frame_state.frame_count;
    };
    startup_callbacks.setup_title_state = [&] {
        setup_title_state(
            title_state,
            {[] {}, [] {}, [] {},
             [&title_palette_frames](const std::uint16_t frames) {
                 title_palette_frames += frames;
             }});
    };

    const auto startup_result =
        run_boot_startup(required, startup, startup_callbacks);
    std::cout << "startup.result="
              << (startup_result == BootStartupResult::title_ready
                      ? "title_ready"
                      : startup_result == BootStartupResult::assets_missing
                          ? "assets_missing"
                          : "callback_unresolved")
              << '\n'
              << "startup.pre_title_complete="
              << (pre_title_completed ? "true" : "false") << '\n'
              << "startup.vblank_count=" << startup.vblank_count << '\n'
              << "startup.sprite_builds=" << sprite_builds << '\n'
              << "startup.horizontal_scroll_updates="
              << horizontal_scroll_updates << '\n'
              << "startup.audio_variant_calls=" << audio_variant_calls << '\n'
              << "startup.audio_calls=" << audio_calls << '\n'
              << "startup.controller_samples=" << controller_samples << '\n'
              << "startup.palette_steps=" << palette_steps << '\n'
              << "startup.resource_stream_writes=" << resource_stream_writes
              << '\n'
              << "startup.resource_work_area_zeroed="
              << (resource_work_area_zeroed ? "true" : "false") << '\n'
              << "startup.strobe_phase_updates=" << strobe_phase_updates
              << '\n'
              << "startup.title_palette_frames=" << title_palette_frames
              << '\n';

    return report.passed && startup_result == BootStartupResult::title_ready &&
                   pre_title_completed
               ? 0
               : 5;
}
