// Headless AudioPlaybackService test (formerly device-only, skipped without
// an output). Runs the null and WAV sinks against the real asset catalog.
#include "audio_playback.hpp"
#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"

#include <QCoreApplication>

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

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
    // Durations of the Timer-A regen (regen_audio_assets.py); DAC SFX tails
    // now run to their real release and 0x12 is a 60 s observation render.
    constexpr std::array<Expectation, 23> kMapped{{
        {0x00U, "audio.music.resource.000", 119900, 120100},
        {0x10U, "audio.intro.title.stage-1", 15851, 15911},
        {0x16U, "audio.field.effect-016", 2843, 2903},
        {0x24U, "audio.menu.command-024", 3358, 3418},
        {0x25U, "audio.effects.resource.037", 2050, 2200},
        {0x1CU, "audio.effects.resource.028", 2353, 2413},
        {0x1DU, "audio.effects.resource.029", 2455, 2515},
        {0x1EU, "audio.effects.resource.030", 2200, 2350},
        {0x3FU, "audio.effects.resource.063", 2036, 2096},
        {0x4AU, "audio.effects.resource.074", 2137, 2197},
        {0x4BU, "audio.effects.resource.075", 2100, 2300},
        {0x4CU, "audio.effects.resource.076", 2112, 2172},
        {0x4DU, "audio.effects.resource.077", 2035, 2095},
        {0x4EU, "audio.effects.resource.078", 2052, 2112},
        {0x32U, "audio.effects.resource.050", 2517, 2577},
        {0x34U, "audio.effects.resource.052", 3944, 4004},
        {0x35U, "audio.effects.resource.053", 5381, 5441},
        {0x37U, "audio.effects.resource.055", 2556, 2616},
        {0x3AU, "audio.effects.resource.058", 3221, 3281},
        {0x51U, "audio.effects.resource.081", 2453, 2513},
        {0x5CU, "audio.intro.sega-logo-shock-sequence", 4700, 4900},
        {0x5DU, "audio.intro.shadowrun-logo-reveal-sequence", 2680, 2720},
        {0x12U, "audio.intro.title.stage-3", 60036, 60096},
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

    // Intro music and effects have different ROM gates. The opening's SFX
    // requests 0x2E/0x46/0x2F use 0x530DE (FFFC0A); stage music uses 0x530EA
    // (FFFC08). A disabled music setting must not mute an enabled SFX request,
    // and a disabled SFX setting must not suppress field music.
    AudioPlaybackService intro_audio(
        resolver, AudioPlaybackService::Output::null);
    intro_audio.set_music_enabled(false);
    intro_audio.set_sfx_enabled(true);
    const auto effect_events_before = intro_audio.events().size();
    intro_audio.play_sfx(0x2EU);
    const auto& effect_events_after = intro_audio.events();
    check(effect_events_after.size() == effect_events_before + 1U &&
              effect_events_after.back().op == AudioOp::start &&
              effect_events_after.back().id == 0x2EU &&
              intro_audio.playing_channel_count_for_event(0x2EU) == 1U &&
              !intro_audio.mixer().music_id().has_value(),
          "opening effect 0x2E follows the SFX gate while music is disabled");
    intro_audio.set_sfx_enabled(false);
    const auto before_muted_effect = intro_audio.events().size();
    intro_audio.play_sfx(0x2FU);
    check(intro_audio.events().size() == before_muted_effect,
          "opening effect follows the SFX disable setting");
    intro_audio.set_music_enabled(true);
    intro_audio.play_music(0x0FU);
    check(intro_audio.mixer().music_id() == 0x0FU,
          "field-profile music follows the music gate while SFX is disabled");

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
    // 0x2C was silent only because the extractor lacked YM Timer A; it is a
    // DAC sample slot and now renders.
    playback.play_sfx(0x2CU);
    playback.advance_frame();
    check(!playback.alias_for_event(0x2CU).empty() &&
              playback.playing_channel_count_for_event(0x2CU) == 1U,
          "DAC slot 0x2C plays");
    playback.play_music(0x06U);
    check(playback.mixer().music_id() == 0x06U, "field music 0x06 plays");
    playback.fade_out();
    // 18 fade frames, then the 1323-sample key-off release (two frames).
    for (int i = 0; i < 21; ++i) playback.advance_frame();
    check(!playback.mixer().music_id().has_value() && playback.mixer().attenuation() == 0U,
          "0x53126 fade stops everything and restores level 0");

    // PCM-enabled backend regression: stop-all and start the same track in
    // the same game frame. QtAudioSink receives only the final mixed PCM
    // frames, so a source-level stop cannot asynchronously cancel this start.
    const auto wav = std::filesystem::temp_directory_path() /
        "srx_audio_playback_test.wav";
    constexpr std::uint16_t kSameFrameTrack = 0x06U;
    {
        AudioPlaybackService render(resolver, AudioPlaybackService::Output::wav, wav);
        render.play_music(kSameFrameTrack);
        render.advance_frame();
        render.stop_all();
        render.play_music(kSameFrameTrack);
        check(render.mixer().music_id() == kSameFrameTrack,
              "same-frame stop-all/start leaves the requested track active");
        check(render.mixer().position(kSameFrameTrack) == 0U,
              "same-frame restart begins at the track start");
        render.advance_frame();
        const auto& events = render.events();
        check(events.size() == 5U &&
                  events[2U].op == AudioOp::stop_all &&
                  events[3U].op == AudioOp::stop &&
                  events[4U].op == AudioOp::start &&
                  events[2U].frame == events[3U].frame &&
                  events[3U].frame == events[4U].frame,
              "same-frame stop-all, same-id stop, and start stay ordered");
        render.finish_output();
    }
    constexpr std::size_t kPcmFrameBytes = kAudioSamplesPerFrame * 4U;
    check(std::filesystem::file_size(wav) == 44U + 2U * kPcmFrameBytes,
          "same-frame restart WAV render size");
    std::vector<char> wav_bytes;
    {
        std::ifstream input(wav, std::ios::binary);
        wav_bytes.assign(std::istreambuf_iterator<char>(input),
                         std::istreambuf_iterator<char>());
    }
    bool restarted_frame_matches_fresh_start =
        wav_bytes.size() == 44U + 2U * kPcmFrameBytes;
    if (restarted_frame_matches_fresh_start) {
        restarted_frame_matches_fresh_start = std::equal(
            wav_bytes.begin() + 44,
            wav_bytes.begin() + 44 + static_cast<std::ptrdiff_t>(kPcmFrameBytes),
            wav_bytes.begin() + 44 + static_cast<std::ptrdiff_t>(kPcmFrameBytes));
    }
    check(restarted_frame_matches_fresh_start,
          "PCM after same-frame stop-all/start matches a fresh track frame");
    std::filesystem::remove(wav);

    if (failures != 0) {
        std::cerr << failures << " audio playback check(s) failed\n";
        return 1;
    }
    std::cout << "audio playback tests passed\n";
    return 0;
}
