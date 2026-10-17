// Headless AudioPlaybackService test (formerly device-only, skipped without
// an output). Runs the null and WAV sinks against the real asset catalog.
#include "audio_playback.hpp"
#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <QCoreApplication>

#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

namespace {

using shadowrun::ghidra_conversion::load_conversion_asset_catalog;
using shadowrun::ghidra_conversion::qt_runtime::AudioEvent;
using shadowrun::ghidra_conversion::qt_runtime::AudioOp;
using shadowrun::ghidra_conversion::qt_runtime::AudioPlaybackService;
using shadowrun::ghidra_conversion::qt_runtime::kAudioSamplesPerFrame;

int failures = 0;
void check(const bool condition, const std::string& what) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << what << '\n';
    }
}

} // namespace

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    const auto loaded = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_GHIDRA_ASSET_ROOT));
    check(loaded.complete(), "asset catalog loads (audio rows present)");
    const auto resolver = [&loaded](const std::string_view alias) {
        const auto* record = loaded.catalog.find(std::string(alias));
        return record == nullptr ? std::filesystem::path{} : record->absolute_path;
    };

    AudioPlaybackService playback(resolver, AudioPlaybackService::Output::null);
    check(playback.manifest_error().empty(), "manifest: " + playback.manifest_error());

    struct Expectation {
        std::uint16_t event;
        std::string_view alias;
        std::int64_t min_ms;
        std::int64_t max_ms;
    };
    // Durations carried over from the device-only test.
    constexpr std::array<Expectation, 23> kMapped{{
        {0x00U, "audio.music.resource.000", 119900, 120100},
        {0x10U, "audio.intro.title.stage-1", 14800, 14900},
        {0x16U, "audio.field.effect-016", 2700, 2800},
        {0x24U, "audio.menu.command-024", 2400, 2600},
        {0x25U, "audio.effects.resource.037", 2050, 2200},
        {0x1CU, "audio.effects.resource.028", 2450, 2510},
        {0x1DU, "audio.effects.resource.029", 2780, 2850},
        {0x1EU, "audio.effects.resource.030", 2200, 2350},
        {0x3FU, "audio.effects.resource.063", 2120, 2180},
        {0x4AU, "audio.effects.resource.074", 2200, 2350},
        {0x4BU, "audio.effects.resource.075", 2100, 2300},
        {0x4CU, "audio.effects.resource.076", 2200, 2300},
        {0x4DU, "audio.effects.resource.077", 2100, 2130},
        {0x4EU, "audio.effects.resource.078", 2100, 2130},
        {0x32U, "audio.effects.resource.050", 2440, 2490},
        {0x34U, "audio.effects.resource.052", 2490, 2540},
        {0x35U, "audio.effects.resource.053", 2410, 2460},
        {0x37U, "audio.effects.resource.055", 2790, 2850},
        {0x3AU, "audio.effects.resource.058", 2700, 2710},
        {0x51U, "audio.effects.resource.081", 2530, 2600},
        {0x5CU, "audio.intro.sega-logo-shock-sequence", 4700, 4900},
        {0x5DU, "audio.intro.shadowrun-logo-reveal-sequence", 2680, 2720},
        {0x12U, "audio.intro.title.stage-3", 20900, 21200},
    }};
    for (const auto& expected : kMapped) {
        const auto status = playback.playback_status_for_event(expected.event);
        check(playback.alias_for_event(expected.event) == expected.alias,
              "alias for " + std::to_string(expected.event));
        check(status.mapped && status.source_ready && !status.error &&
                  status.duration_ms >= expected.min_ms &&
                  status.duration_ms <= expected.max_ms,
              "duration for " + std::to_string(expected.event) + " = " +
                  std::to_string(status.duration_ms));
    }
    // The 41 rendered ids that had no alias before (doc 26 §2.3).
    constexpr std::array<std::uint16_t, 41> kNewlyMapped{{
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F, 0x13, 0x14, 0x15, 0x17, 0x18, 0x1F, 0x20,
        0x21, 0x22, 0x23, 0x26, 0x27, 0x28, 0x2A, 0x2B, 0x30, 0x31, 0x33,
        0x36, 0x38, 0x39, 0x3B, 0x3D, 0x3E, 0x50, 0x5B}};
    for (const auto id : kNewlyMapped) {
        check(playback.playback_status_for_event(id).source_ready,
              "newly mapped id " + std::to_string(id));
    }

    // Starts, stops, one-shot composites and unmapped ids, all logged.
    playback.play_sfx(0x24U);
    playback.advance_frame();
    check(playback.playing_channel_count_for_event(0x24U) == 1U, "0x24 started");
    playback.stop_resource(0x24U);
    playback.advance_frame();
    playback.advance_frame();
    check(playback.playing_channel_count_for_event(0x24U) == 0U, "0x24 stopped after release");
    playback.begin_source_sequence();
    for (int i = 0; i < 60; ++i) {
        playback.play_event_once_in_sequence(0x5CU);
        playback.advance_frame();
        playback.advance_frame();
    }
    std::size_t sega_starts = 0U;
    for (const auto& event : playback.events())
        sega_starts += event.op == AudioOp::start && event.id == 0x5CU ? 1U : 0U;
    check(sega_starts == 1U && playback.playing_channel_count_for_event(0x5CU) == 1U,
          "60 source requests start the composite once");
    playback.play_sfx(0x2CU);
    check(playback.alias_for_event(0x2CU).empty() &&
              playback.playing_channel_count_for_event(0x2CU) == 0U &&
              playback.events().back() == AudioEvent{playback.mixer().frame(), AudioOp::start, 0x2CU},
          "silent slot 0x2C is logged but plays nothing");
    playback.play_music(0x06U);
    check(playback.mixer().music_id() == 0x06U, "field music 0x06 plays");
    playback.fade_out();
    // 18 fade frames, then the 1323-sample key-off release (two frames).
    for (int i = 0; i < 21; ++i) playback.advance_frame();
    check(!playback.mixer().music_id().has_value() && playback.mixer().attenuation() == 0U,
          "0x53126 fade stops everything and restores level 0");

    // WAV render.
    const auto wav = std::filesystem::temp_directory_path() / "srx_audio_playback_test.wav";
    {
        AudioPlaybackService render(resolver, AudioPlaybackService::Output::wav, wav);
        render.play_music(0x06U);
        render.play_sfx(0x25U);
        for (int i = 0; i < 30; ++i) render.advance_frame();
        render.finish_output();
    }
    check(std::filesystem::file_size(wav) == 44U + 30U * kAudioSamplesPerFrame * 4U,
          "WAV render size");
    std::filesystem::remove(wav);

    if (failures != 0) {
        std::cerr << failures << " audio playback check(s) failed\n";
        return 1;
    }
    std::cout << "audio playback tests passed\n";
    return 0;
}
