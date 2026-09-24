#include "shadowrun_ghidra/audio_service_conversion.hpp"

#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<int> order;
    const AudioServiceCallbacks callbacks{
        [&order] { order.push_back(1); },
        [&order] { order.push_back(2); },
        [&order] { order.push_back(3); },
        [&order] { order.push_back(4); },
    };
    if (!expect(!service_audio_if_enabled(false, callbacks) &&
                order.empty() && service_audio_if_enabled(true, callbacks) &&
                order == std::vector<int>{1, 2, 3, 4})) {
        return 1;
    }

    if (!expect(!service_audio_command_queue({}) &&
                !service_audio_if_enabled(true, {}))) {
        return 2;
    }

    return 0;
}
