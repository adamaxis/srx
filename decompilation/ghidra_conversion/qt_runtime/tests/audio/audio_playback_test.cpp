#include "audio_playback.hpp"

#include <QAudioDevice>
#include <QCoreApplication>
#include <QEventLoop>
#include <QMediaDevices>
#include <QThread>

#include <chrono>
#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);

    using shadowrun::ghidra_conversion::qt_runtime::AudioPlaybackService;
    const auto menu_024 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_MENU_RESOURCE_PATH);
    const auto field_effect_016 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_FIELD_EFFECT_016_PATH);
    const auto resource_037 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_RESOURCE_PATH);
    const auto resource_075 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_PASSKEY_RESOURCE_PATH);
    const auto spawn_resource_028 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_028_PATH);
    const auto spawn_resource_029 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_029_PATH);
    const auto spawn_resource_030 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_030_PATH);
    const auto spawn_resource_063 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_063_PATH);
    const auto spawn_resource_074 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_074_PATH);
    const auto spawn_resource_081 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_SPAWN_RESOURCE_081_PATH);
    const auto boot_shock_sequence = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_BOOT_SHOCK_SEQUENCE_PATH);
    const auto title_logo_sequence = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_TITLE_LOGO_SEQUENCE_PATH);
    const auto title_theme_resource = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_TITLE_THEME_RESOURCE_PATH);
    const auto intro_stage_1 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_INTRO_STAGE_1_PATH);
    const auto intro_stage_3 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_INTRO_STAGE_3_PATH);
    std::string resolved_alias;
    std::size_t resolver_calls = 0U;
    AudioPlaybackService playback([&](const std::string_view alias) {
        resolved_alias.assign(alias);
        ++resolver_calls;
        if (alias == "audio.menu.command-024") {
            return menu_024;
        }
        if (alias == "audio.field.effect-016") {
            return field_effect_016;
        }
        if (alias == "audio.effects.resource.037") {
            return resource_037;
        }
        if (alias == "audio.effects.resource.075") {
            return resource_075;
        }
        if (alias == "audio.effects.resource.028") {
            return spawn_resource_028;
        }
        if (alias == "audio.effects.resource.029") {
            return spawn_resource_029;
        }
        if (alias == "audio.effects.resource.030") {
            return spawn_resource_030;
        }
        if (alias == "audio.effects.resource.063") {
            return spawn_resource_063;
        }
        if (alias == "audio.effects.resource.074") {
            return spawn_resource_074;
        }
        if (alias == "audio.effects.resource.081") {
            return spawn_resource_081;
        }
        if (alias == "audio.intro.sega-logo-shock-sequence") {
            return boot_shock_sequence;
        }
        if (alias == "audio.intro.shadowrun-logo-reveal-sequence") {
            return title_logo_sequence;
        }
        if (alias == "audio.music.resource.000") {
            return title_theme_resource;
        }
        if (alias == "audio.intro.title.stage-1") {
            return intro_stage_1;
        }
        if (alias == "audio.intro.title.stage-3") {
            return intro_stage_3;
        }
        return std::filesystem::path{};
    });

    const auto expected_alias = "audio.effects.resource.037";
    if (playback.alias_for_event(0x25U) != expected_alias) {
        std::cerr << "event 0x25 did not select table-2 resource 37\n";
        return 1;
    }
    if (playback.alias_for_event(0x24U) != "audio.menu.command-024") {
        std::cerr << "event 0x24 did not select its command sound\n";
        return 1;
    }
    if (!std::filesystem::is_regular_file(menu_024)) {
        std::cerr << "the extracted menu command-024 FLAC is missing\n";
        return 1;
    }
    if (!std::filesystem::is_regular_file(resource_037)) {
        std::cerr << "the extracted resource-037 FLAC is missing\n";
        return 1;
    }
    if (playback.alias_for_event(0x4BU) !=
        "audio.effects.resource.075") {
        std::cerr << "event 0x4B did not select table-2 resource 75\n";
        return 1;
    }
    if (!std::filesystem::is_regular_file(resource_075)) {
        std::cerr << "the extracted resource-075 FLAC is missing\n";
        return 1;
    }
    const std::array<std::pair<std::uint16_t, std::filesystem::path>, 5>
        spawn_sound_files{{
            {0x1CU, spawn_resource_028}, {0x1DU, spawn_resource_029},
            {0x3FU, spawn_resource_063}, {0x51U, spawn_resource_081},
            {0x1EU, spawn_resource_030}}};
    constexpr std::array<std::string_view, 5> kSpawnSoundAliases{{
        "audio.effects.resource.028", "audio.effects.resource.029",
        "audio.effects.resource.063", "audio.effects.resource.081",
        "audio.effects.resource.030"}};
    for (std::size_t index = 0; index < spawn_sound_files.size(); ++index) {
        const auto [event, path] = spawn_sound_files[index];
        if (playback.alias_for_event(event) != kSpawnSoundAliases[index] ||
            !std::filesystem::is_regular_file(path)) {
            std::cerr << "spawn action event " << event
                      << " did not select its extracted source resource\n";
            return 1;
        }
    }
    if (playback.alias_for_event(0x1EU) !=
            "audio.effects.resource.030" ||
        !std::filesystem::is_regular_file(spawn_resource_030)) {
        std::cerr << "spawn event 0x1E did not select resource 30\n";
        return 1;
    }
    if (playback.alias_for_event(0x4AU) !=
            "audio.effects.resource.074" ||
        !std::filesystem::is_regular_file(spawn_resource_074)) {
        std::cerr << "spawn event 0x4A did not select resource 74\n";
        return 1;
    }
    if (playback.alias_for_event(0x10U) != "audio.intro.title.stage-1" ||
        !std::filesystem::is_regular_file(intro_stage_1)) {
        std::cerr << "opening command 0x10 did not select the stage-1 track\n";
        return 1;
    }
    if (playback.alias_for_event(0x12U) != "audio.intro.title.stage-3" ||
        !std::filesystem::is_regular_file(intro_stage_3)) {
        std::cerr << "opening command 0x12 did not select the stage-3 track\n";
        return 1;
    }
    if (!playback.alias_for_event(0x11U).empty()) {
        std::cerr << "silent stage-2 source pair 0x11 was assigned a track\n";
        return 1;
    }
    if (playback.alias_for_event(0x5CU) !=
            "audio.intro.sega-logo-shock-sequence" ||
        !std::filesystem::is_regular_file(boot_shock_sequence)) {
        std::cerr << "pre-title command 0x5C did not select its complete "
                     "source-cadenced render\n";
        return 1;
    }
    if (playback.alias_for_event(0x5DU) !=
            "audio.intro.shadowrun-logo-reveal-sequence" ||
        !std::filesystem::is_regular_file(title_logo_sequence)) {
        std::cerr << "title logo command 0x5D did not select its complete "
                     "source-cadenced render\n";
        return 1;
    }
    if (playback.alias_for_event(0x00U) != "audio.music.resource.000" ||
        !std::filesystem::is_regular_file(title_theme_resource)) {
        std::cerr << "title theme command 0x00 did not select resource 0\n";
        return 1;
    }
    if (playback.alias_for_event(0x16U) != "audio.field.effect-016" ||
        !std::filesystem::is_regular_file(field_effect_016)) {
        std::cerr << "FF 10 command 0x16 did not select resource 22\n";
        return 1;
    }

    if (QMediaDevices::defaultAudioOutput().isNull()) {
        std::cerr << "no default audio output is available for playback\n";
        return 77;
    }

    struct PlaybackExpectation {
        std::uint16_t event;
        std::string_view alias;
        std::int64_t min_duration_ms;
        std::int64_t max_duration_ms;
    };
    constexpr std::array<PlaybackExpectation, 15> kMappedEvents{{
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
        {0x51U, "audio.effects.resource.081", 2530, 2600},
        {0x5CU, "audio.intro.sega-logo-shock-sequence", 4700, 4900},
        {0x5DU, "audio.intro.shadowrun-logo-reveal-sequence", 2680, 2720},
        {0x12U, "audio.intro.title.stage-3", 20900, 21200},
    }};
    for (const auto& expected : kMappedEvents) {
        const auto calls_before = resolver_calls;
        const auto event_started_at = std::chrono::steady_clock::now();
        if (expected.event == 0x5CU || expected.event == 0x5DU) {
            playback.begin_source_sequence();
            playback.play_event_once_in_sequence(expected.event);
        } else {
            playback.play_event(expected.event);
        }
        if (resolver_calls != calls_before + 1U ||
            resolved_alias != expected.alias) {
            std::cerr << "event " << expected.event
                      << " did not reach its mapped asset alias\n";
            return 1;
        }

        const auto deadline = std::chrono::steady_clock::now() +
                              std::chrono::seconds(3);
        auto status = playback.playback_status_for_event(expected.event);
        while (std::chrono::steady_clock::now() < deadline &&
               !status.error &&
               !(status.source_ready && status.audio_available &&
                 status.playing)) {
            application.processEvents(QEventLoop::AllEvents, 20);
            QThread::msleep(5U);
            status = playback.playback_status_for_event(expected.event);
        }
        if (!status.mapped || !status.source_ready ||
            !status.audio_available || !status.playing || status.error ||
            status.duration_ms < expected.min_duration_ms ||
            status.duration_ms > expected.max_duration_ms) {
            std::cerr << "event " << expected.event
                      << " failed to decode/play on the default output; "
                         "duration_ms="
                      << status.duration_ms
                      << " loaded=" << status.source_ready
                      << " audio=" << status.audio_available
                      << " playing=" << status.playing
                      << " error=" << status.error << '\n';
            return 1;
        }
        const auto start_latency_ms = std::chrono::duration_cast<
            std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                       event_started_at)
                                          .count();
        std::cout << "event=" << expected.event
                  << " start_latency_ms=" << start_latency_ms
                  << " duration_ms=" << status.duration_ms << '\n';

        if (expected.event == 0x5CU || expected.event == 0x5DU) {
            if (playback.playing_channel_count_for_event(expected.event) !=
                1U) {
                std::cerr << "first one-shot source request did not start one channel\n";
                return 1;
            }
            // The mapped asset already contains the ROM-timed repeated chip
            // stream: 60 starts for 0x5C, 19 starts for 0x5D. Verify that
            // each source callback does not restart the full composite file.
            const std::size_t request_count = expected.event == 0x5CU
                ? 60U
                : 19U;
            const auto request_interval = expected.event == 0x5CU
                ? std::chrono::milliseconds(33)
                : std::chrono::milliseconds(67);
            auto next_request_at = std::chrono::steady_clock::now();
            for (std::size_t request = 1U; request < request_count; ++request) {
                next_request_at += request_interval;
                while (std::chrono::steady_clock::now() < next_request_at) {
                    application.processEvents(QEventLoop::AllEvents, 5);
                    QThread::msleep(1U);
                }
                playback.play_event_once_in_sequence(expected.event);
            }
            status = playback.playback_status_for_event(expected.event);
            if (!status.audio_available || !status.playing || status.error ||
                playback.playing_channel_count_for_event(expected.event) !=
                    1U) {
                std::cerr << "repeated source callbacks restarted the "
                             "sequence render; count="
                          << playback.playing_channel_count_for_event(
                                 expected.event)
                          << " loaded=" << status.source_ready
                          << " audio=" << status.audio_available
                          << " playing=" << status.playing
                          << " error=" << status.error << '\n';
                return 1;
            }
            playback.stop_resource(expected.event);
            application.processEvents(QEventLoop::AllEvents, 20);
            if (playback.playback_status_for_event(expected.event).playing ||
                playback.playing_channel_count_for_event(expected.event) !=
                    0U) {
                std::cerr << "one-shot stop left a channel playing\n";
                return 1;
            }
            playback.begin_source_sequence();
            playback.play_event_once_in_sequence(expected.event);
            const auto replay_deadline = std::chrono::steady_clock::now() +
                                         std::chrono::seconds(3);
            status = playback.playback_status_for_event(expected.event);
            while (std::chrono::steady_clock::now() < replay_deadline &&
                   !status.error &&
                   !(status.source_ready && status.audio_available &&
                     status.playing)) {
                application.processEvents(QEventLoop::AllEvents, 20);
                QThread::msleep(5U);
                status = playback.playback_status_for_event(expected.event);
            }
            if (!status.source_ready || !status.audio_available ||
                !status.playing || status.error ||
                playback.playing_channel_count_for_event(expected.event) !=
                    1U) {
                std::cerr << "new source sequence did not replay one cue\n";
                return 1;
            }
            playback.stop_resource(expected.event);
        } else if (expected.event == 0x16U) {
            if (playback.playing_channel_count_for_event(expected.event) !=
                1U) {
                std::cerr << "FF 10 command 0x16 did not start resource 22\n";
                return 1;
            }
            playback.stop_resource(expected.event);
            application.processEvents(QEventLoop::AllEvents, 20);
            if (playback.playing_channel_count_for_event(expected.event) !=
                0U) {
                std::cerr << "FF 12 command 0x16 did not stop resource 22\n";
                return 1;
            }
        }

        if (expected.event == 0x10U) {
            playback.stop_all_channels();
            application.processEvents(QEventLoop::AllEvents, 20);
            const auto calls_before_silent_stage = resolver_calls;
            playback.play_event(0x11U);
            if (resolver_calls != calls_before_silent_stage ||
                playback.playback_status_for_event(0x10U).playing ||
                playback.playing_channel_count_for_event(0x10U) != 0U) {
                std::cerr << "FF 16 stage handoff left opening audio active\n";
                return 1;
            }
        }

        playback.stop_all_channels();
        application.processEvents(QEventLoop::AllEvents, 20);
        if (playback.playback_status_for_event(expected.event).playing) {
            std::cerr << "stop_all_channels left event " << expected.event
                      << " playing\n";
            return 1;
        }
    }

    const auto calls_before_unknown = resolver_calls;
    playback.play_event(0x11U);
    playback.play_event(0x2EU);
    playback.play_event(0x2AU);
    playback.play_event(0x2CU);
    playback.play_event(0x30U);
    if (resolver_calls != calls_before_unknown ||
        !playback.alias_for_event(0x2CU).empty() ||
        !playback.alias_for_event(0x2EU).empty() ||
        !playback.alias_for_event(0x30U).empty()) {
        std::cerr << "an unresolved event reached an audio asset\n";
        return 1;
    }

    return 0;
}
