#include "shadowrun_ghidra/controller_wait_conversion.hpp"

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

    return 0;
}
