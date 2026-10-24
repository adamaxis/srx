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

    // ROM 0x0D94 waits for any new bit in FFDD22's high byte. Sweep every
    // possible controller-port byte through the typed sample/latch/wait path:
    // each nonzero mask must wake once, a held mask and its release must not
    // wake again, and a fresh press after release must wake again.
    std::uint32_t audited_masks = 0U;
    for (std::uint32_t raw_mask = 0U; raw_mask <= 0xFFU; ++raw_mask) {
        const auto mask = static_cast<std::uint8_t>(raw_mask);
        ControllerSampleState state{};
        ControllerLatchState latch{};
        std::uint32_t frames = 0U;
        std::uint32_t samples = 0U;
        const ControllerWaitHooks hooks{
            [&] { ++frames; },
            [&] {
                latch.events = state.events;
                (void)latch_controller_events(latch);
            },
            [&] {
                sample_controller_input(state, mask);
                ++samples;
            },
            [] {},
            [&] { return state.events; },
        };

        if (mask == 0U) {
            sample_controller_input(state, mask);
            if (!expect(state.events == 0U)) {
                return 14;
            }
            continue;
        }

        if (!expect(wait_for_controller_event(hooks) == 1U) ||
            !expect(frames == 1U && samples == 2U) ||
            !expect((state.events & 0xFF00U) == 0U) ||
            !expect(state.events == mask)) {
            return 15;
        }

        sample_controller_input(state, mask);
        if (!expect(state.events == mask)) {
            return 16;
        }
        latch.events = state.events;
        if (!expect(latch_controller_events(latch) == 0U)) {
            return 17;
        }

        sample_controller_input(state, 0U);
        if (!expect(state.events == 0U)) {
            return 18;
        }
        sample_controller_input(state, mask);
        const auto expected_event =
            static_cast<std::uint16_t>(raw_mask << 8U);
        if (!expect((state.events & 0xFF00U) == expected_event)) {
            return 19;
        }
        latch.events = state.events;
        latch.extra_events = 0U;
        latch.confirm_latch = 0;
        latch.start_latch = 0;
        if (!expect(latch_controller_events(latch) == expected_event) ||
            !expect(latch.confirm_latch == -1) ||
            !expect(latch.start_latch ==
                    ((mask & 0x80U) != 0U ? -1 : 0))) {
            return 20;
        }
        ++audited_masks;
    }
    if (!expect(audited_masks == 255U)) {
        return 21;
    }

    return 0;
}
