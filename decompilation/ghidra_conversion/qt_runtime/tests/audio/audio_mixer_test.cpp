// Headless unit tests for the frame-locked audio core (doc 26 phases 2-3).
#include "pcm_queue.hpp"

#include <algorithm>
#include <vector>
#include "audio_mixer.hpp"
#include "audio_resources.hpp"
#include "gems_commands.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion::qt_runtime;

int failures = 0;
void check(const bool condition, const std::string& what) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << what << '\n';
    }
}

const std::filesystem::path kAssets(SHADOWRUN_GHIDRA_ASSET_ROOT);

std::map<std::string, std::filesystem::path> catalog_paths() {
    std::map<std::string, std::filesystem::path> paths;
    std::ifstream file(kAssets / "resources.tsv");
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream row(line);
        std::string alias, path;
        std::getline(row, alias, '\t');
        std::getline(row, path, '\t');
        paths[alias] = kAssets / path;
    }
    return paths;
}

AudioResourceBank make_bank() {
    static const auto paths = catalog_paths();
    AudioResourceBank bank([](const std::string_view alias) {
        const auto found = paths.find(std::string(alias));
        return found == paths.end() ? std::filesystem::path{} : found->second;
    });
    bank.load_manifest(kAssets / "audio" / "audio_manifest.tsv");
    return bank;
}

class CaptureSink final : public AudioSink {
public:
    void submit(const std::int16_t* stereo, std::size_t frames) override {
        sizes.push_back(frames);
        samples.insert(samples.end(), stereo, stereo + frames * 2U);
    }
    std::vector<std::size_t> sizes;
    std::vector<std::int16_t> samples;
};

std::uint64_t fnv(const std::vector<std::int16_t>& samples) {
    std::uint64_t hash = 1469598103934665603ULL;
    for (const auto sample : samples) {
        hash = (hash ^ static_cast<std::uint16_t>(sample)) * 1099511628211ULL;
    }
    return hash;
}

constexpr std::uint16_t kSelect = 0x24U;   // MenuSound.SELECT / audio.menu.command-024
constexpr std::uint16_t kMove = 0x25U;
constexpr std::uint16_t kEnding = 0x13U;   // MusicId.ENDING, has loop points
constexpr std::uint16_t kMatrix = 0x03U;   // MusicId.MATRIX

// One scripted session; returns the captured PCM.
std::vector<std::int16_t> scripted_session(AudioResourceBank& bank,
                                           std::vector<AudioEvent>* events,
                                           std::vector<std::size_t>* sizes) {
    AudioMixer mixer(bank, true);
    GemsCommandLayer gems(mixer);
    CaptureSink sink;
    for (int frame = 0; frame < 40; ++frame) {
        if (frame == 0) gems.play_sfx(kSelect);
        if (frame == 3) gems.play_sfx(kMove);
        if (frame == 5) gems.play_music(kEnding);
        if (frame == 10) gems.stop_sfx(kSelect);
        if (frame == 20) gems.attenuate(0x40U);
        gems.advance_frame(&sink);
    }
    if (events) *events = mixer.events();
    if (sizes) *sizes = sink.sizes;
    return sink.samples;
}

} // namespace

int main() {
    auto bank = make_bank();
    check(bank.error().empty(), "manifest loads: " + bank.error());
    check(bank.rows().size() == 94U, "all 94 table ids render");
    for (const auto& [id, row] : bank.rows()) {
        check(bank.frame_count(id).has_value(),
              "id " + std::to_string(id) + " resolves to a 44.1 kHz FLAC (" + row.alias + ")");
        check(!row.enum_name.empty() && row.enum_name != "-",
              "id " + std::to_string(id) + " has an enum name");
    }
    // 0x11 and 0x2C wait on YM Timer A; the timer-aware extractor renders them.
    check(bank.row(0x11U) != nullptr && bank.row(0x2CU) != nullptr,
          "Timer-A slots 0x11/0x2C are mapped");

    // Determinism and the 735-sample frame.
    std::vector<AudioEvent> events_a, events_b;
    std::vector<std::size_t> sizes;
    const auto run_a = scripted_session(bank, &events_a, &sizes);
    const auto run_b = scripted_session(bank, &events_b, nullptr);
    check(run_a.size() == 40U * kAudioSamplesPerFrame * 2U, "40 frames x 735 stereo samples");
    check(fnv(run_a) == fnv(run_b) && events_a == events_b, "identical PCM and op log on rerun");
    bool all_735 = !sizes.empty();
    for (const auto size : sizes) all_735 = all_735 && size == kAudioSamplesPerFrame;
    check(all_735, "every submit is exactly 735 frames");
    const std::vector<AudioEvent> expected_log{
        {0U, AudioOp::start, kSelect}, {3U, AudioOp::start, kMove},
        {5U, AudioOp::stop, kEnding}, {5U, AudioOp::start, kEnding},
        {10U, AudioOp::stop, kSelect}, {20U, AudioOp::attenuate, 0x40U}};
    check(events_a == expected_log, "(frame, op, id) log matches the script");
    bool any_sound = false;
    for (const auto sample : run_a) any_sound = any_sound || sample != 0;
    check(any_sound, "PCM session is not silent");

    // Null mode: no decode, same voice state and log as PCM mode.
    {
        AudioMixer pcm(bank, true), null(bank, false);
        NullAudioSink sink;
        CaptureSink capture;
        bool same = true;
        for (int frame = 0; frame < 30; ++frame) {
            if (frame == 1) { pcm.start(kSelect); null.start(kSelect); }
            if (frame == 2) { pcm.start(kEnding); null.start(kEnding); }
            if (frame == 12) { pcm.stop(kSelect); null.stop(kSelect); }
            pcm.advance_frame(&capture);
            null.advance_frame(&sink);
            same = same && pcm.playing(kSelect) == null.playing(kSelect) &&
                   pcm.position(kEnding) == null.position(kEnding);
        }
        check(same && pcm.events() == null.events(), "null sink advances like the PCM mixer");
        check(sink.frames() == 30U * kAudioSamplesPerFrame, "null sink counted 30 frames");
    }

    // Stop = key-off release ramp of 1323 samples (1.8 frames).
    {
        AudioMixer mixer(bank, false);
        NullAudioSink sink;
        mixer.start(kSelect);
        for (int i = 0; i < 5; ++i) mixer.advance_frame(&sink);
        mixer.stop(kSelect);
        check(mixer.playing(kSelect), "voice still sounding at the stop frame");
        mixer.advance_frame(&sink);
        check(mixer.playing(kSelect), "release continues into the next frame");
        mixer.advance_frame(&sink);
        check(!mixer.playing(kSelect), "release ends within two frames");
    }
    // PCM release actually fades to silence.
    {
        AudioMixer mixer(bank, true);
        CaptureSink sink;
        mixer.start(kSelect);
        for (int i = 0; i < 3; ++i) mixer.advance_frame(&sink);
        mixer.stop(kSelect);
        mixer.advance_frame(&sink);
        mixer.advance_frame(&sink);
        mixer.advance_frame(&sink);
        bool silent_tail = true;
        for (std::size_t i = 5U * kAudioSamplesPerFrame * 2U; i < sink.samples.size(); ++i)
            silent_tail = silent_tail && sink.samples[i] == 0;
        check(silent_tail, "after the release ramp the output is silent");
    }

    // Loop points: ENDING wraps from loop_end to loop_start and never ends.
    {
        AudioMixer mixer(bank, false);
        NullAudioSink sink;
        const auto* row = bank.row(kEnding);
        const auto length = bank.frame_count(kEnding).value_or(0U);
        check(row && row->loop_start && row->loop_end, "ENDING has loop points");
        mixer.start(kEnding);
        const auto frames = static_cast<int>(length / kAudioSamplesPerFrame) + 50;
        bool wrapped = false;
        std::uint64_t previous = 0U;
        for (int i = 0; i < frames; ++i) {
            mixer.advance_frame(&sink);
            const auto position = mixer.position(kEnding).value_or(0U);
            wrapped = wrapped || position < previous;
            previous = position;
        }
        check(mixer.playing(kEnding) && wrapped, "looping music outlives its file length");
        check(row && previous >= *row->loop_start && previous < *row->loop_end,
              "position stays inside the loop window");
    }

    // Voice stealing and retrigger.
    {
        AudioMixer mixer(bank, false);
        const std::uint16_t ids[] = {0x14, 0x15, 0x17, 0x18, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
        for (const auto id : ids) mixer.start(id);
        check(mixer.active_sfx_voices() == kAudioSfxVoices, "8 SFX voices");
        check(!mixer.playing(0x14U) && mixer.playing(0x20U), "9th SFX steals the oldest voice");
        mixer.start(0x20U);
        check(mixer.playing_voice_count(0x20U) == 1U && mixer.position(0x20U) == 0U,
              "same-id start retriggers one voice");
        mixer.start(kEnding);
        mixer.start(kMatrix);
        check(mixer.music_id() == kMatrix && !mixer.playing(kEnding), "one music slot");
        check(mixer.active_sfx_voices() == kAudioSfxVoices, "music does not take an SFX voice");
    }

    // 0x53126 fade: 6 levels 3 frames apart, then op 0x16 and level 0 at +18;
    // operations issued during the fade run after it, in order.
    {
        AudioMixer mixer(bank, false);
        GemsCommandLayer gems(mixer);
        NullAudioSink sink;
        gems.play_music(kEnding);
        gems.advance_frame(&sink);           // frame 0 -> 1
        mixer.clear_events();
        gems.fade_out();                      // frame 1
        for (int i = 0; i < 25; ++i) {
            if (i == 4) gems.play_music(kMatrix);   // queued behind the fade
            if (i == 5) gems.play_sfx(kSelect);
            gems.advance_frame(&sink);
        }
        const std::vector<AudioEvent> expected{
            {1U, AudioOp::attenuate, 0x14U}, {4U, AudioOp::attenuate, 0x28U},
            {7U, AudioOp::attenuate, 0x3CU}, {10U, AudioOp::attenuate, 0x50U},
            {13U, AudioOp::attenuate, 0x64U}, {16U, AudioOp::attenuate, 0x78U},
            {19U, AudioOp::stop_all, 0U}, {19U, AudioOp::attenuate, 0U},
            {19U, AudioOp::stop, kMatrix}, {19U, AudioOp::start, kMatrix},
            {19U, AudioOp::start, kSelect}};
        check(mixer.events() == expected, "fade timeline matches 0x53126");
        check(!gems.fading() && mixer.music_id() == kMatrix && mixer.attenuation() == 0U,
              "queued song starts at full volume after the fade");
    }

    // wait_frames queued behind a fade delays later commands by N frames
    // after the fade completes (host stand-in for ROM setup time).
    {
        AudioMixer mixer(bank, false);
        GemsCommandLayer gems(mixer);
        NullAudioSink sink;
        gems.fade_out();                      // frame 0
        gems.wait_frames(8U);
        gems.play_music(kMatrix);
        for (int i = 0; i < 30; ++i) gems.advance_frame(&sink);
        const auto& events = mixer.events();
        check(!events.empty() && events.back().op == AudioOp::start &&
                  events.back().id == kMatrix && events.back().frame == 18U + 8U,
              "wait_frames delays the queued song 8 frames past the fade");
    }

    // Enable gates FFFC08 (music) / FFFC0A (SFX); stop_all and attenuate are ungated.
    {
        AudioMixer mixer(bank, false);
        GemsCommandLayer gems(mixer);
        gems.set_sfx_enabled(false);
        gems.play_sfx(kSelect);
        gems.stop_sfx(kSelect);
        gems.play_music(kEnding);
        gems.set_music_enabled(false);
        gems.play_music(kMatrix);
        gems.stop_all();
        gems.attenuate(kGemsStartMenuDuck);
        const std::vector<AudioEvent> expected{
            {0U, AudioOp::stop, kEnding}, {0U, AudioOp::start, kEnding},
            {0U, AudioOp::stop_all, 0U}, {0U, AudioOp::attenuate, kGemsStartMenuDuck}};
        check(mixer.events() == expected, "FFFC08/FFFC0A gates");
    }

    // Attenuation: level >= 0x7F is silent; level 0 restores.
    {
        AudioMixer mixer(bank, true);
        CaptureSink sink;
        mixer.start(kSelect);
        mixer.attenuate(kAudioAttenuationSilent);
        for (int i = 0; i < 4; ++i) mixer.advance_frame(&sink);
        bool silent = true;
        for (const auto s : sink.samples) silent = silent && s == 0;
        check(silent, "full attenuation is silent");
        mixer.attenuate(0U);
        mixer.advance_frame(&sink);
        bool loud = false;
        for (std::size_t i = 4U * kAudioSamplesPerFrame * 2U; i < sink.samples.size(); ++i)
            loud = loud || sink.samples[i] != 0;
        check(loud, "level 0 restores output");
    }

    // WAV sink.
    {
        const auto path = std::filesystem::temp_directory_path() / "srx_audio_mixer_test.wav";
        {
            AudioMixer mixer(bank, true);
            WavAudioSink wav(path);
            check(wav.ok(), "WAV sink opens");
            mixer.start(kSelect);
            for (int i = 0; i < 10; ++i) mixer.advance_frame(&wav);
        }
        check(std::filesystem::file_size(path) == 44U + 10U * kAudioSamplesPerFrame * 4U,
              "WAV holds a 44-byte header and 10 x 735 stereo frames");
        {
            std::ifstream file(path, std::ios::binary);
            char riff[4] = {};
            file.read(riff, 4);
            check(std::string(riff, 4) == "RIFF", "WAV RIFF header");
        }
        std::filesystem::remove(path);
    }

    // Playtest #35: host PCM queue keeps short writes in order, never
    // duplicates, and only drops the oldest whole frames past its cap.
    {
        using shadowrun::ghidra_conversion::qt_runtime::PcmQueue;
        PcmQueue queue(64U, 4U);
        std::vector<char> out;
        char in[40];
        for (int i = 0; i < 40; ++i) in[i] = static_cast<char>(i);
        queue.push(in, 40U);
        // Device accepts only 6 bytes per call, 10 bytes this frame.
        auto accept6 = [&](const char* d, std::size_t n) {
            const auto k = std::min<std::size_t>(n, 6U);
            out.insert(out.end(), d, d + k);
            return static_cast<long long>(k);
        };
        check(queue.flush(10U, accept6) == 6U, "PCM queue stops on a short write");
        check(queue.pending_bytes() == 34U, "PCM queue keeps the unwritten remainder");
        queue.flush(1000U, [&](const char* d, std::size_t n) {
            out.insert(out.end(), d, d + n);
            return static_cast<long long>(n);
        });
        bool ordered = out.size() == 40U;
        for (std::size_t i = 0; ordered && i < out.size(); ++i) ordered = out[i] == static_cast<char>(i);
        check(ordered, "PCM queue delivers every sample once, in order");
        queue.push(in, 40U);
        queue.push(in, 40U);
        check(queue.pending_bytes() == 64U && queue.dropped_bytes() == 16U,
              "PCM queue drops only the oldest whole frames past its cap");
        check(queue.flush(0U, accept6) == 0U, "PCM queue writes nothing with no device space");
    }

    if (failures != 0) {
        std::cerr << failures << " audio mixer check(s) failed\n";
        return 1;
    }
    std::cout << "audio mixer tests passed\n";
    return 0;
}
