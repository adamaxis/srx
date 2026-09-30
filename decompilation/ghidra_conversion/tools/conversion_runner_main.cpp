#include "shadowrun_ghidra/integration/conversion_workbench.hpp"
#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/integration/startup_integration_conversion.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

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

    integration::StartupIntegrationState startup_state{};
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
    integration::StartupIntegrationCallbacks startup_callbacks{};
    startup_callbacks.initialize_display = [] {};
    startup_callbacks.vdp_ready = {
        [](const std::uint32_t) {},
        [] {},
        [](const std::uint32_t) {},
    };
    startup_callbacks.pre_title.resources.clear_plane_a = [] {};
    startup_callbacks.pre_title.resources.clear_plane_b = [] {};
    startup_callbacks.pre_title.resources.prepare_title_resources = [] {};
    startup_callbacks.pre_title.resources.write_resource_stream =
        [&resource_stream_writes, &resource_work_area_zeroed](auto& words) {
            ++resource_stream_writes;
            resource_work_area_zeroed = std::all_of(
                words.begin(), words.end(),
                [](const std::uint16_t word) { return word == 0U; });
        };
    startup_callbacks.pre_title.resources.refresh_field_resource_streams =
        [] {};
    startup_callbacks.pre_title.resources.decode_and_upload_source_stream =
        [] {};
    startup_callbacks.pre_title.resources.advance_title_palette_fade =
        [&palette_steps] { ++palette_steps; };
    startup_callbacks.pre_title.resources.write_pre_title_grid =
        [](const auto&) {};
    startup_callbacks.pre_title.resources.reset_vdp_upload_state = [] {};
    startup_callbacks.pre_title.wait_for_vblank = [&startup_state] {
        wait_for_vblank(startup_state.frame,
                        [](const std::uint16_t previous) {
                            return static_cast<std::uint16_t>(previous + 1U);
                        });
    };
    startup_callbacks.pre_title.animation.build_title_sprite_records =
        [&sprite_builds] { ++sprite_builds; };
    startup_callbacks.pre_title.animation.publish_logo_strobe_palette =
        [&strobe_phase_updates](const PreTitleLogoStrobePalette&) {
            ++strobe_phase_updates;
        };
    startup_callbacks.pre_title.animation.step_entity_color_words = [] {};
    startup_callbacks.pre_title.write_horizontal_scroll_stream =
        [&horizontal_scroll_updates](const auto&) {
            ++horizontal_scroll_updates;
        };
    startup_callbacks.pre_title.service_audio_variant =
        [&audio_variant_calls] { ++audio_variant_calls; };
    startup_callbacks.pre_title.service_audio =
        [&audio_calls] { ++audio_calls; };
    startup_callbacks.pre_title.sample_controller =
        [&controller_samples] { ++controller_samples; };
    startup_callbacks.pre_title.step_palette_transition =
        [&palette_steps] { ++palette_steps; };

    startup_callbacks.title_setup = {
        [] {},
        [] {},
        [] {},
        [&title_palette_frames](const std::uint16_t frames) {
            title_palette_frames += frames;
        },
    };

    const auto startup_result =
        integration::start_startup_integration(
            asset_root, startup_state, startup_callbacks);
    std::cout << "startup.result="
              << (startup_result == integration::StartupIntegrationStartResult::title_ready
                      ? "title_ready"
                      : startup_result == integration::StartupIntegrationStartResult::assets_missing
                          ? "assets_missing"
                          : "callback_unresolved")
              << '\n'
              << "startup.pre_title_complete="
              << (startup_state.pre_title_complete ? "true" : "false")
              << '\n'
              << "startup.vblank_count=" << startup_state.boot.vblank_count
              << '\n'
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

    return report.passed &&
                   startup_result ==
                       integration::StartupIntegrationStartResult::title_ready &&
                   startup_state.pre_title_complete
               ? 0
               : 5;
}
