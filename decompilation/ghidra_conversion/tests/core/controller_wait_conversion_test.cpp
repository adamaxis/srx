#include "shadowrun_ghidra/core/controller_wait_conversion.hpp"

#include <cstdint>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::uint32_t frames = 0;
    std::uint32_t latches = 0;
    std::uint32_t samples = 0;
    std::uint32_t updates = 0;
    std::uint32_t reads = 0;
    const ControllerWaitHooks delayed_event{
        [&] { ++frames; },
        [&] { ++latches; },
        [&] { ++samples; },
        [&] { ++updates; },
        [&] {
            ++reads;
            return static_cast<std::uint16_t>(reads >= 3 ? 0x0100 : 0);
        },
    };

    if (!expect(wait_for_controller_event(delayed_event) == 3U)) {
        return 1;
    }
    if (!expect(frames == 3U && latches == 3U && updates == 3U)) {
        return 2;
    }
    if (!expect(samples == 4U && reads == 3U)) {
        return 3;
    }

    std::uint32_t immediate_samples = 0;
    const ControllerWaitHooks immediate_event{
        [] {},
        [] {},
        [&] { ++immediate_samples; },
        [] {},
        [] { return static_cast<std::uint16_t>(0x0200); },
    };
    if (!expect(wait_for_controller_event(immediate_event) == 1U)) {
        return 4;
    }
    if (!expect(immediate_samples == 2U)) {
        return 5;
    }

    ControllerLatchState idle{0x0012U, 0x0034U, 1, 2};
    if (!expect(latch_controller_events(idle) == 0U &&
                idle.confirm_latch == 1 && idle.start_latch == 2)) {
        return 6;
    }

    ControllerLatchState confirm{0x0100U, 0U, 0, 0};
    if (!expect(latch_controller_events(confirm) == 0x0100U &&
                confirm.confirm_latch == -1 && confirm.start_latch == 0)) {
        return 7;
    }

    ControllerLatchState start{0x8000U, 0x0100U, 0, 0};
    if (!expect(latch_controller_events(start) == 0x8100U &&
                start.confirm_latch == -1 && start.start_latch == -1)) {
        return 8;
    }

    ControllerSampleState sample{};
    sample_controller_input(sample, 0x01U);
    if (!expect(sample.held == 0x0001U && sample.events == 0x0101U &&
                sample.heading == 0x0000U)) {
        return 9;
    }
    sample_controller_input(sample, 0x01U);
    if (!expect(sample.held == 0x0001U && sample.events == 0x0001U &&
                sample.heading == 0x0000U)) {
        return 10;
    }
    sample_controller_input(sample, 0x00U);
    if (!expect(sample.held == 0U && sample.events == 0U &&
                sample.heading == 0xFFFFU)) {
        return 11;
    }
    sample_controller_input(sample, 0x08U);
    if (!expect(sample.held == 0x0008U && sample.events == 0x0808U &&
                sample.heading == 0x4000U)) {
        return 12;
    }
    sample_controller_input(sample, 0x03U);
    if (!expect(sample.held == 0x0003U && sample.events == 0x0303U &&
                sample.heading == 0xFFFFU)) {
        return 13;
    }

    return 0;
}
