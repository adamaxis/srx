#include "shadowrun_ghidra/timing_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::uint32_t> delays;
    std::vector<int> ready_order;
    wait_for_vdp_ready({
        [&delays, &ready_order](const std::uint32_t delay) {
            delays.push_back(delay);
            ready_order.push_back(1);
        },
        [&ready_order] { ready_order.push_back(2); },
        [&delays, &ready_order](const std::uint32_t delay) {
            delays.push_back(delay);
            ready_order.push_back(3);
        },
    });
    if (!expect(delays == std::vector<std::uint32_t>{45056U, 256U} &&
                ready_order == std::vector<int>{1, 2, 3})) {
        return 1;
    }

    EvenFrameState even_frame{4U};
    if (!expect(wait_for_even_frame(
                    even_frame, [](const std::uint32_t previous) {
                        return previous + 3U;
                    }) &&
                even_frame.observed_even_frame == 6U)) {
        return 2;
    }

    VblankFrameState state{0U, 10U};
    const auto waited = wait_for_vblank(
        state, [](const std::uint16_t previous) {
            return static_cast<std::uint16_t>(previous + 1U);
        });
    if (!expect(waited && state.frame_count == 1U &&
                state.observed_sample == 11U)) {
        return 3;
    }

    if (!expect(wait_for_vblank_frames(
                    state, 1U,
                    [](const std::uint16_t previous) {
                        return static_cast<std::uint16_t>(previous + 1U);
                    }) == 2U &&
                state.frame_count == 3U && state.observed_sample == 13U)) {
        return 4;
    }

    std::vector<int> controller_order;
    std::vector<std::uint16_t> samples{0x0001U, 0x1200U, 0x00FFU};
    std::size_t sample_index = 0U;
    const auto controller_iterations = wait_for_controller_event(
        state,
        {
            [&controller_order](const std::uint16_t previous) {
                controller_order.push_back(1);
                return static_cast<std::uint16_t>(previous + 1U);
            },
            [&controller_order] { controller_order.push_back(2); },
            [&controller_order, &samples, &sample_index] {
                controller_order.push_back(3);
                return samples[sample_index++];
            },
        });
    if (!expect(controller_iterations == 2U && sample_index == 3U &&
                controller_order == std::vector<int>{1, 2, 3, 1, 2, 3,
                                                      3})) {
        return 5;
    }

    std::vector<int> palette_order;
    const auto iterations = wait_for_palette_frame(
        state, 1U,
        [&palette_order](const std::uint16_t previous) {
            palette_order.push_back(1);
            return static_cast<std::uint16_t>(previous + 1U);
        },
        [&palette_order] { palette_order.push_back(2); });
    if (!expect(iterations == 2U && state.frame_count == 7U &&
                state.observed_sample == 17U &&
                palette_order == std::vector<int>{1, 2, 1, 2})) {
        return 6;
    }

    if (!expect(wait_for_palette_frame(state, 0U, {}, {}) == 1U &&
                state.frame_count == 8U)) {
        return 7;
    }

    return 0;
}
