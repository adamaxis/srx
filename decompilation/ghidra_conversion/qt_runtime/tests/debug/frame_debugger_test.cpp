#include "shadowrun_ghidra/debug/frame_debugger.hpp"

#include <cstdint>
#include <filesystem>
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
        recorder.record_completed_frame(0x1234U);
        recorder.record_event(1U, 90, 0U, false);
        recorder.record_completed_frame(0xABCDU);
        std::string error;
        if (!recorder.finish(&error)) {
            std::cerr << error << '\n';
            return 1;
        }
    }

    KeyboardInputLog replay(
        {InputLogConfig::Mode::replay, path.string()});
    if (replay.recorded_frame_count() != 2U || !replay.has_seed_history() ||
        replay.expected_seed_after_frame(0U) != 0x1234U ||
        replay.expected_seed_after_frame(1U) != 0xABCDU) {
        std::cerr << "frame count or random-seed history did not round-trip\n";
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
    return 0;
}
