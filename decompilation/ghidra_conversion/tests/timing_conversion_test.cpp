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

    VblankFrameState state{0U, 10U};
    const auto waited = wait_for_vblank(
        state, [](const std::uint16_t previous) {
            return static_cast<std::uint16_t>(previous + 1U);
        });
    if (!expect(waited && state.frame_count == 1U &&
                state.observed_sample == 11U)) {
        return 2;
    }

    std::vector<int> palette_order;
    const auto iterations = wait_for_palette_frame(
        state, 1U,
        [&palette_order](const std::uint16_t previous) {
            palette_order.push_back(1);
            return static_cast<std::uint16_t>(previous + 1U);
        },
        [&palette_order] { palette_order.push_back(2); });
    if (!expect(iterations == 2U && state.frame_count == 3U &&
                state.observed_sample == 13U &&
                palette_order == std::vector<int>{1, 2, 1, 2})) {
        return 3;
    }

    if (!expect(wait_for_palette_frame(state, 0U, {}, {}) == 1U &&
                state.frame_count == 4U)) {
        return 4;
    }

    return 0;
}
