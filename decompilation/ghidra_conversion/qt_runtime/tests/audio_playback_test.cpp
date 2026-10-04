#include "audio_playback.hpp"

#include <QCoreApplication>

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);

    using shadowrun::ghidra_conversion::qt_runtime::AudioPlaybackService;
    std::string resolved_alias;
    std::size_t resolver_calls = 0U;
    AudioPlaybackService playback([&](const std::string_view alias) {
        resolved_alias.assign(alias);
        ++resolver_calls;
        return std::filesystem::path{};
    });

    const auto expected_alias = "audio.effects.resource.037";
    if (playback.alias_for_event(0x25U) != expected_alias) {
        std::cerr << "event 0x25 did not select table-2 resource 37\n";
        return 1;
    }
    if (!std::filesystem::is_regular_file(
            SHADOWRUN_GHIDRA_AUDIO_RESOURCE_PATH)) {
        std::cerr << "the extracted resource-037 FLAC is missing\n";
        return 1;
    }

    playback.play_event(0x25U);
    if (resolver_calls != 1U || resolved_alias != expected_alias) {
        std::cerr << "event 0x25 did not reach its extracted asset alias\n";
        return 1;
    }

    playback.play_event(0x2AU);
    if (resolver_calls != 1U) {
        std::cerr << "an unresolved event was assigned an audio asset\n";
        return 1;
    }

    return 0;
}
