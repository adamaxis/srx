#include "shadowrun_ghidra/debug/frame_debugger.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace shadowrun::ghidra_conversion::debug;

int main() {
    const auto path = std::filesystem::temp_directory_path() /
                      "shadowrun-frame-debugger-roundtrip.srxkey";
    std::error_code ignored;
    std::filesystem::remove(path, ignored);

    {
        KeyboardInputLog recorder(
            {InputLogConfig::Mode::record, path.string()});
        recorder.record_event(0U, 90, 0U, true);
        recorder.record_source_event(0U, "0x5AEC6.render_time_record");
        recorder.record_completed_frame(0x1234U, 0x0123456789ABCDEFULL);
        recorder.record_event(1U, 90, 0U, false);
        recorder.record_source_event(1U, "field.world.encounter.dispatch");
        recorder.record_completed_frame(0xABCDU, 0xFEDCBA9876543210ULL);
        std::string error;
        if (!recorder.finish(&error)) {
            std::cerr << error << '\n';
            return 1;
        }
    }

    KeyboardInputLog replay(
        {InputLogConfig::Mode::replay, path.string()});
    if (replay.recorded_frame_count() != 2U || !replay.has_seed_history() ||
        !replay.has_fingerprint_history() ||
        !replay.has_source_event_history() ||
        replay.source_boundary_events().size() != 2U ||
        replay.source_boundary_events()[0].frame != 0U ||
        replay.source_boundary_events()[0].name !=
            "0x5AEC6.render_time_record" ||
        replay.source_boundary_events()[1].frame != 1U ||
        replay.source_boundary_events()[1].name !=
            "field.world.encounter.dispatch" ||
        replay.expected_seed_after_frame(0U) != 0x1234U ||
        replay.expected_seed_after_frame(1U) != 0xABCDU ||
        replay.expected_fingerprint_after_frame(0U) !=
            0x0123456789ABCDEFULL ||
        replay.expected_fingerprint_after_frame(1U) !=
            0xFEDCBA9876543210ULL) {
        std::cerr << "frame count, seed, or inspection fingerprint history did not round-trip\n";
        return 1;
    }

    const auto legacy_v1_path = std::filesystem::temp_directory_path() /
                                "shadowrun-frame-debugger-v1.srxkey";
    const auto legacy_v2_path = std::filesystem::temp_directory_path() /
                                "shadowrun-frame-debugger-v2.srxkey";
    const auto legacy_v3_path = std::filesystem::temp_directory_path() /
                                "shadowrun-frame-debugger-v3.srxkey";
    for (const auto& [legacy_path, magic] :
         {std::pair{legacy_v1_path, std::string("SRXKEY1\0", 8)},
          std::pair{legacy_v2_path, std::string("SRXKEY2\0", 8)},
          std::pair{legacy_v3_path, std::string("SRXKEY3\0", 8)}}) {
        std::ofstream legacy(legacy_path, std::ios::binary | std::ios::trunc);
        legacy.write(magic.data(), static_cast<std::streamsize>(magic.size()));
        const std::uint64_t empty_count = 0U;
        for (int word = 0; word < 2; ++word) {
            for (unsigned shift = 0; shift < 64; shift += 8) {
                legacy.put(static_cast<char>((empty_count >> shift) & 0xFFU));
            }
        }
    }
    KeyboardInputLog legacy_v1(
        {InputLogConfig::Mode::replay, legacy_v1_path.string()});
    KeyboardInputLog legacy_v2(
        {InputLogConfig::Mode::replay, legacy_v2_path.string()});
    KeyboardInputLog legacy_v3(
        {InputLogConfig::Mode::replay, legacy_v3_path.string()});
    if (legacy_v1.has_seed_history() || legacy_v1.has_fingerprint_history() ||
        !legacy_v2.has_seed_history() ||
        legacy_v2.has_fingerprint_history() ||
        !legacy_v3.has_seed_history() ||
        !legacy_v3.has_fingerprint_history() ||
        legacy_v3.has_source_event_history()) {
        std::cerr << "legacy input-log versions did not retain their capabilities\n";
        return 1;
    }
    const auto first = replay.take_events_for_frame(0U);
    const auto second = replay.take_events_for_frame(1U);
    if (first.size() != 1U || first[0].key != 90 || !first[0].pressed ||
        second.size() != 1U || second[0].key != 90 || second[0].pressed) {
        std::cerr << "frame-indexed key transitions did not round-trip\n";
        return 1;
    }

    FrameStepper stepper(true);
    stepper.toggle_pause();
    stepper.queue_step(3U);
    if (!stepper.paused() || stepper.frames_to_advance(2U) != 2U ||
        stepper.frames_to_advance(2U) != 1U ||
        stepper.frames_to_advance(2U) != 0U) {
        std::cerr << "paused source-frame stepping did not consume queued frames\n";
        return 1;
    }
    stepper.toggle_pause();
    if (stepper.paused() || stepper.frames_to_advance(2U) != 2U) {
        std::cerr << "unpausing did not resume real-time frames\n";
        return 1;
    }

    std::filesystem::remove(path, ignored);
    std::filesystem::remove(legacy_v1_path, ignored);
    std::filesystem::remove(legacy_v2_path, ignored);
    std::filesystem::remove(legacy_v3_path, ignored);
    return 0;
}
