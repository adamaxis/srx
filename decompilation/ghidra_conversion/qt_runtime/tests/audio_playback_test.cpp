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

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);

    using shadowrun::ghidra_conversion::qt_runtime::AudioPlaybackService;
    const auto menu_024 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_MENU_RESOURCE_PATH);
    const auto resource_037 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_RESOURCE_PATH);
    const auto resource_075 = std::filesystem::path(
        SHADOWRUN_GHIDRA_AUDIO_PASSKEY_RESOURCE_PATH);
    std::string resolved_alias;
    std::size_t resolver_calls = 0U;
    AudioPlaybackService playback([&](const std::string_view alias) {
        resolved_alias.assign(alias);
        ++resolver_calls;
        if (alias == "audio.menu.command-024") {
            return menu_024;
        }
        if (alias == "audio.effects.resource.037") {
            return resource_037;
        }
        if (alias == "audio.effects.resource.075") {
            return resource_075;
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
    constexpr std::array<PlaybackExpectation, 3> kMappedEvents{{
        {0x24U, "audio.menu.command-024", 2400, 2600},
        {0x25U, "audio.effects.resource.037", 2050, 2200},
        {0x4BU, "audio.effects.resource.075", 2100, 2300},
    }};
    for (const auto& expected : kMappedEvents) {
        const auto calls_before = resolver_calls;
        const auto event_started_at = std::chrono::steady_clock::now();
        playback.play_event(expected.event);
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

        playback.stop_effects();
        application.processEvents(QEventLoop::AllEvents, 20);
        if (playback.playback_status_for_event(expected.event).playing) {
            std::cerr << "stop_effects left event " << expected.event
                      << " playing\n";
            return 1;
        }
    }

    playback.play_event(0x2AU);
    playback.play_event(0x2CU);
    playback.play_event(0x30U);
    if (resolver_calls != kMappedEvents.size() ||
        !playback.alias_for_event(0x2CU).empty() ||
        !playback.alias_for_event(0x30U).empty()) {
        std::cerr << "an unresolved event was assigned an audio asset\n";
        return 1;
    }

    return 0;
}
